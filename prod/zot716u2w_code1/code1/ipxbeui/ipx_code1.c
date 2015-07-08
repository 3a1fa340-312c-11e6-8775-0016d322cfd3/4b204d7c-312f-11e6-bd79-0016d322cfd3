#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>

#include "pstarget.h"
#include "psglobal.h"
#include "ipx.h"

BYTE MyNetworkAddress[4];
BYTE MyPhysNodeAddress[6];
extern WORD MyIntNO;

//*******************************************************************
//*
//*  Module   Name : IPXInitialize();
//*  Created  Date : JULY        30,1991
//*  Author        : ZeroOne Corp.
//*  Modified Date : 1/23/98, Simon
//*  Modified By   : ZeroOne Corp.
//*  Purpose       : Initialize IPX and LANDriver
//*  Globals       : socketNumberTable   socketTable
//*
//*******************************************************************
SocketNumberTable     socketTable[SOCKET_TABLE_LENGTH];
WORD socketTablePointer;
static WORD startSocket;	//Low-High
static PKTSENDBUFFER PKTBuf;    //ethernet send buffer length = 1514

extern uint16 NGET16( uint8 *pSrc );
extern void NSET16( uint8 *pDest, uint16 value );
extern int send_pkt(int intno,uint8 *buffer,unsigned int length);


#ifdef CONST_DATA
//move this table to constant.c	3/24/98
extern const WORD far FrameTypeSequence[];
#else
const WORD FrameTypeSequence[MaxFrameType]={
     IPX8022,
     IPX8023,
     IPXENII,
     IPXSNAP
};
#endif CONST_DATA

WORD GetIPXFrameType(WORD i)
{
	if(i > MaxFrameType) i = 0;
	return FrameTypeSequence[i];
}

int IPXInitialize(void)
{
	int	i;

	startSocket = 0x4000;
	socketTablePointer = 0;

	for( i = 0 ; i < SOCKET_TABLE_LENGTH ; i ++) {
		socketTable[i].socketNumber = 0x0000;
		socketTable[i].ECBLinkList = NULL;
	}
	return (0);
}

void IPXGetInternetworkAddress(BYTE *Address)
{
	memcpy(Address,MyNetworkAddress,4);
	memcpy(Address+4,MyPhysNodeAddress,6);
}

//*******************************************************************
//*
//*  Module   Name : IPXOpenSocket (BYTE *, BYTE)
//*  Created  Date : JULY     26,1991
//*  Author        : ZeroOne Corp.
//*  Modified Date : 1/23/98
//*  Modified By   : ZeroOne Corp.
//*  Purpose       : Open a socket number for later use
//*  Input         : socketNumber *     high-low
//*                  socketType
//*  Output        : ccode           0x00    SUCCESSFUL
//*                                  0xFF    SOCKET_ALREADY_OPEN
//*                                  0xFE    SOCKET_TABLE_FULL
//*
//*******************************************************************
WORD IPXOpenSocket(BYTE *socketNumber,BYTE socketType)
{
	WORD i = 0;
	WORD ccode = SUCCESSFUL;
	WORD TmpSocketNumber; //high-low

	cli();	//eCos

	if(socketTablePointer >= SOCKET_TABLE_LENGTH) {
		// if socketTable is full than set
		// the ccode to SOCKET_TABLE_FULL
	    ccode = SOCKET_TABLE_FULL;
	}
	else {
		if( NGET16(socketNumber) == ANY ) {
			// if sockeNumber is ANY than find a
			// free socketNumber to open

			TmpSocketNumber = WordSwap(startSocket); //high-low
			for(i = 0; i < socketTablePointer; i++) {
				if(socketTable[i].socketNumber == TmpSocketNumber) {
					startSocket++;
					TmpSocketNumber = WordSwap(startSocket); //high-low
					i = 0;
				}
			}
			*socketNumber = TmpSocketNumber; //high-low
			socketType = SHORT_LIVED;
		}
		else {
			// if the socketNumber is specified
			// check if this number had been opend
			// if No, open this new socket

			TmpSocketNumber = NGET16(socketNumber); //high-low
			for(i = 0; i < socketTablePointer; i++) {
				if(socketTable[i].socketNumber == TmpSocketNumber) {
					ccode = SOCKET_ALREADY_OPEN;
					sti();	//eCos
					return (ccode);
				}
			}
		}
		socketTable[socketTablePointer].socketNumber = TmpSocketNumber;
		socketTable[socketTablePointer].socketType = socketType;
		socketTablePointer++;
	}
	sti();	//eCos
	return (ccode);
}

//********************************************************************
//*
//*  Module   Name : IPXSendPacket (ECB *)
//*  Created  Date : JULY        26,1994
//*  Author        : Zero One Corp.
//*  Modified Date : 1/23/98
//*  Modified By   : Zero One Corp.
//*  Purpose       : To send a packet through IPX layer
//*
//********************************************************************


void IPXSendPacket(ECB *sendECB)
{
	WORD  i          = 0;
	WORD  packetSize = 0;
	WORD  SendToPktDrvSize = 0;

	IPXHeader  *ipxHead;
	WORD  FrameHeadSize;

	BYTE *tmpipxHead;
	WORD tmpSize;
	int16 rc;  //12/11/98

	if(sendECB->inUseFlag != SENDING) {
		if(sendECB->fragmentCount == 0)  //low-high
		{
          	// if packetsize is equal to zero
			// set the completion code to
			// MALFORM_PACKET
			sendECB->inUseFlag      = FREEZE;           // Clear InUseFlag
			sendECB->completionCode = MALFORM_PACKET;   // Set CompletionCode is 0xFD (253)
		}  // illegal packet
		else {
#ifdef NETBEUI
			if(sendECB->IPXFrameType == IPXBEUI) {
				NT_IPXtoNETBEUIHeader(sendECB);
			}
#endif NETBEUI
			// if the packet size in less than the
			// allowed IPX packet size sets the
			// completion code to MALFORM_PACKET
			if(sendECB->fragmentDescriptor[0].size < MIN_IPX_PACKET_SIZE) {
				sendECB->inUseFlag      = FREEZE;
				sendECB->completionCode = MALFORM_PACKET;
			}
			else {
				for(i = 0; i < sendECB->fragmentCount; i ++) {
					packetSize = packetSize + sendECB->fragmentDescriptor[i].size;
				}
				if(packetSize > MAX_IPX_PACKET_SIZE) {
					sendECB->inUseFlag      = FREEZE;
					sendECB->completionCode = MALFORM_PACKET;
				}
				else {
					sendECB->inUseFlag = SENDING;
					//Destination address
					memcpy(PKTBuf.Etherh.dest,sendECB->immediateAddress,6);
					//Source address
					memcpy(PKTBuf.Etherh.me,MyPhysNodeAddress,6);

					switch (sendECB->IPXFrameType)
					{
						case IPX8023:
							//Length (high-low)
							PKTBuf.IPX_8023.Etherh.lentyp[0] = packetSize >> 8;
							PKTBuf.IPX_8023.Etherh.lentyp[1]  = (BYTE)packetSize;
							FrameHeadSize = 14;
							break;
						case IPX8022:
							//Length (high - low)
							PKTBuf.IPX_8022.Etherh.lentyp[0]  = (packetSize+3) >> 8;
							PKTBuf.IPX_8022.Etherh.lentyp[1]   = (BYTE) (packetSize+3);

							PKTBuf.IPX_8022.NOVELL_DSAP = NOVELL_8022_DSAP;
							PKTBuf.IPX_8022.NOVELL_SSAP = NOVELL_8022_SSAP;
							PKTBuf.IPX_8022.CONTROL_8022_UI  = CONTROL_UI;
							FrameHeadSize = 17;
							break;
						case IPXENII:
							//type
							PKTBuf.IPX_ENII.Etherh.lentyp[0] = (BYTE)(NOVELL_ETHN_TYPE & 0x00ff );
							PKTBuf.IPX_ENII.Etherh.lentyp[1] = (BYTE)((NOVELL_ETHN_TYPE & 0xff00 ) >> 8 );
							FrameHeadSize = 14;
							break;
						case IPXSNAP:
							PKTBuf.IPX_SNAP.Etherh.lentyp[0]  = (packetSize+8) >> 8;
							PKTBuf.IPX_SNAP.Etherh.lentyp[1]   = (BYTE) (packetSize+8);

							PKTBuf.IPX_SNAP.NOVELL_DSAP = NOVELL_SNAP_DSAP;
							PKTBuf.IPX_SNAP.NOVELL_SSAP = NOVELL_SNAP_SSAP;
							PKTBuf.IPX_SNAP.CONTROL_SNAP_UI	 = CONTROL_UI;
							PKTBuf.IPX_SNAP.ORGANIZATION_CODE[0] = VENDERID1;
							PKTBuf.IPX_SNAP.ORGANIZATION_CODE[1] = VENDERID2;
							PKTBuf.IPX_SNAP.ORGANIZATION_CODE[2] = VENDERID3;
							PKTBuf.IPX_SNAP.ETHERNETTYPE = NOVELL_ETHN_TYPE;
							FrameHeadSize = 22;
							break;
#ifdef NETBEUI
						case IPXBEUI:
							//Length (high - low)
							PKTBuf.IPX_8022.Etherh.lentyp[0]  = (packetSize+3) >> 8;
							PKTBuf.IPX_8022.Etherh.lentyp[1]   = (BYTE) (packetSize+3);

							PKTBuf.IPX_8022.NOVELL_DSAP = NETBEUI_8022_DSAP;
							PKTBuf.IPX_8022.NOVELL_SSAP = NETBEUI_8022_SSAP;
							PKTBuf.IPX_8022.CONTROL_8022_UI  = CONTROL_UI;
							FrameHeadSize = 17;
							break;
#endif NETBEUI

						default:
							break;
					}
					SendToPktDrvSize = packetSize + FrameHeadSize;
					if(SendToPktDrvSize < 60 ) SendToPktDrvSize = 60;

					tmpipxHead = ((BYTE*)&PKTBuf+FrameHeadSize);
					ipxHead = (IPXHeader *)tmpipxHead;
					for(i = 0 ; i < sendECB->fragmentCount; i++) {
						tmpSize = sendECB->fragmentDescriptor[i].size;
						memcpy(tmpipxHead,
						       sendECB->fragmentDescriptor[i].address,
						       tmpSize);
						tmpipxHead += tmpSize;
					}

					if(sendECB->IPXFrameType != IPXBEUI) {
						// update checksum field
						ipxHead->checkSum = 0xFFFF;

						// packet length
						ipxHead->length = WordSwap(packetSize);

						// update transport control field
						ipxHead->transportControl = 0x00;

						memcpy(ipxHead->source.network,MyNetworkAddress,4);
						memcpy(ipxHead->source.node,MyPhysNodeAddress,6);

						NSET16( ipxHead->source.socket, sendECB->socketNumber );
					}

                    ///////////* Pass this ECB to Next layer        *//////////

					for(i = 0; i < 2 ; i++)	//12/11/98 changed
						if((rc = send_pkt(MyIntNO,PKTBuf.PKT_BUFFER,SendToPktDrvSize)) == 0)
							break;

					if(rc !=0) {
						sendECB->completionCode = SOCKET_SEND_PACKET_ERROR;
					}
					sendECB->inUseFlag = OKAY;
				}
			}
		}
	}
}

#ifdef IPX_DEBUG
void IPX_SEND_PKT(ECB *sendECB, BYTE* PktBuf, WORD PktSize)
{
	BYTE *DBG_Buf;
	WORD DBG_Size;

	IPXSendPacket1((BYTE*)sendECB,&DBG_Buf,&DBG_Size);

	if(PktSize != DBG_Size)
	    printf("Size Error (IPX_SEND_PKT)\n");

	if(memcmp(PktBuf,DBG_Buf,PktSize) != 0)
	    printf("Data Error (IPX_SEND_PKT)\n");
}
#endif

//********************************************************************
//*
//*  Module   Name : IPXListenForPacket(ECB *)
//*  Created  Date : JULY     26,1991
//*  Author        : ZeroOne Corp.
//*  Modified Date : 1/23/98, Simon
//*  Author        : ZeroOne Corp.
//*  Purpose       : Prepare link list of listening ECB
//*
//********************************************************************
void IPXListenForPacket(ECB *listenECB)
{
	WORD i;
	WORD SocketNumberFound;

	SocketNumberFound = 0;
	if(socketTablePointer < SOCKET_TABLE_LENGTH) {
    	for(i = 0 ; i < socketTablePointer && !SocketNumberFound; i++) {
			if(listenECB->socketNumber == socketTable[i].socketNumber) {
				if(listenECB->inUseFlag != RECEIVING) {
					//add ECB to socket table (link list)
					listenECB->linkAddress = socketTable[i].ECBLinkList;
					socketTable[i].ECBLinkList = listenECB;
					listenECB->inUseFlag = RECEIVING;
				}
				SocketNumberFound = 1;
			}
		}
	}

	if(!SocketNumberFound) {
	    // update ECB completion code
		listenECB->completionCode = SOCKET_NOT_OPEN;
#ifdef PC_OUTPUT
		printf("Not Socket Open\n");
#endif
	}
}

//***************************************************************************
//*
//*  IPXInput( p )
//*
//*   Called by the packet input to get a new ip packet.  Checks the
//*   validity of the packet (checksum, flags) and then passes it on to the
//*   appropriate protocol handler.
//*
//***************************************************************************
WORD IPXInput(WORD InputFrameType, BYTE *SrcAddress,IPXHeader *p,int16 DataLen)
{
	WORD  i = 0,j;
	BYTE  *PKTDataPoint;
	BYTE  ECBFound = 0;
	ECB   *listenECB;
	WORD CopyLen;
	int i_state;

	i_state = dirps();	//eCos

#ifdef SNMPIPX
	if(NGET16(p->destination.socket) == 0x5204 && NGET32(&(p[1])) != 0x0C030100) {
		return 0;
	}
#endif SNMPIPX

		for(i = 0 ; (!ECBFound && i < socketTablePointer) ; i++ ) {
			if( NGET16(p->destination.socket) == socketTable[i].socketNumber) {
				if(socketTable[i].ECBLinkList == NULL) {
					break;
				}
				listenECB = socketTable[i].ECBLinkList;
				socketTable[i].ECBLinkList = listenECB->linkAddress;

				if( listenECB->inUseFlag == RECEIVING ) {
					memcpy(listenECB->immediateAddress, SrcAddress, 6);
					PKTDataPoint = (BYTE *) p;

					for(j=0 ; j < listenECB->fragmentCount ; j++ ) {
						if(DataLen <= 0) break;
						if((listenECB->fragmentDescriptor[j].size != 0) && (listenECB->fragmentDescriptor[j].address != NULL)){		//queue ipx
						CopyLen = listenECB->fragmentDescriptor[j].recv_size
						        = min(DataLen,listenECB->fragmentDescriptor[j].size);
						memcpy(listenECB->fragmentDescriptor[j].address, PKTDataPoint, CopyLen);
						PKTDataPoint = PKTDataPoint + listenECB->fragmentDescriptor[j].size;
						DataLen -= CopyLen;
    		        }
    		        }
					listenECB->inUseFlag = OKAY;
					listenECB->completionCode = OKAY;
					listenECB->IPXFrameType = InputFrameType; //3/16/98
					if(listenECB->ESRAddress != NULL) {
						(listenECB->ESRAddress)(); // Go To Service ESR Routine
					}
				}
				ECBFound = 1;
			}
		}
	restore(i_state);	//eCos
	
	return(0);
}

//********************************************************************
//*
//*  Module   Name : IPXCancelEvent(ECB *)
//*  Created  Date : JULY     26,1991
//*  Author        : ZeroOne Corp.
//*  Modified Date : 2/3/98, Simon
//*  Author        : ZeroOne Corp.
//*  Purpose       : Remove ECB from socket link list
//*
//********************************************************************
int IPXCancelEvent(ECB *listenECB)
{
	WORD  i;
	WORD  ccode;

	ccode = SOCKET_NOT_OPEN;
	for(i = 0; i < socketTablePointer; i++) {
		if(listenECB->socketNumber == socketTable[i].socketNumber) {
			//remove one ECB from socket table (First In Last Out)
			socketTable[i].ECBLinkList = listenECB->linkAddress;
			ccode = SUCCESSFUL;
			break;
		}
	}
	return (ccode);
}

//extern int msi_wl_FreeUsrBuf(USR_BUF_DESC *);
extern int msi_wl_FreePktBuf(char *);