#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "ntudp.h"
#include "nps.h"
#include "ipx.h"
#include "netbeui.h"
#include "ntps.h"

DWORD IPXSocketErr;	//10/20/99


static NETBEUIHeader NTSendNETBEUIHeader;
static BYTE MyNetBEUIName[16];

void Node2Num(BYTE *buf,BYTE offset)
{
	BYTE i;
	for(i = offset; i < 6 ; i++) {
		if((*buf = (MyPhysNodeAddress[i]>>4)) > 9) *buf += ('A'-10);
		else *buf += '0';
		buf++;

		if((*buf = (MyPhysNodeAddress[i]&0x0F)) > 9) *buf += ('A'-10);
		else *buf += '0';
		buf++;
	}
}

void NETBEUInit(void)
{
	BYTE NameHeadSize = sizeof(MY_NETBEUI_NAME) -1 ;
	BYTE *pName = MyNetBEUIName;
	BYTE i;

	memcpy(pName,MY_NETBEUI_NAME,NameHeadSize);
	pName += NameHeadSize;
	Node2Num(pName,3);	//3/24/99 changed
}

WORD NT_NETBEUItoIPXInput(WORD InputFrameType, BYTE *SrcAddress,NETBEUIHeader *p,int16 DataLen)
{
	WORD  i,j;
	BYTE  *PKTDataPoint;
	BYTE  ECBFound = 0;
	ECB   *listenECB;
	int i_state;
	WORD CopyLen;
	WORD NETBEUISocket;
	IPXHeader *pIPXHdr;

	if(memcmp((BYTE*)(p+1),NT_CHECK_MARK2,NT_CHECK_MARK2_LEN) == 0) {
		//NT_VERSION_4
		PKTDataPoint = (BYTE*)&NETBEUISocket;  //NT30
		PKTDataPoint[0]=PKTDataPoint[1] = *((BYTE *)(p+1)+3); //(0x50, 0x60 or 0x70), (N-E-T-50, N-E-T-60,N-E-T-70)
	} else {
		//NT_VERSION_2
		NETBEUISocket = NGET16(p+1);
		memcpy((void*)(p+1),NT_CHECK_MARK1,NT_CHECK_MARK1_LEN);
	}

	if(NETBEUISocket == NETBEUI_ListViewSocket) NETBEUISocket = UtilitySocket;
	else
	{
		if(memcmp(p->DestName,MyNetBEUIName,sizeof(MY_NETBEUI_NAME)+5) != 0) {
			return (0);
		}
	}

	i_state = dirps();

	for(i = 0 ; (!ECBFound && i < socketTablePointer) ; i++ ) {
		if(NETBEUISocket == socketTable[i].socketNumber) {
			if(socketTable[i].ECBLinkList == NULL) {
				IPXSocketErr++;
				break;
			}
			listenECB = socketTable[i].ECBLinkList;
			socketTable[i].ECBLinkList = listenECB->linkAddress;

			if( listenECB->inUseFlag == RECEIVING ) {
				memcpy(listenECB->immediateAddress, SrcAddress, 6);
				memcpy(listenECB->IPXSrcNETBEUIName,p->SrcName,16);
				pIPXHdr = (IPXHeader *)listenECB->fragmentDescriptor[0].address;
				memcpy(pIPXHdr->source.node, SrcAddress, 6);
				NSET16( &pIPXHdr->source.socket, NETBEUISocket );

				PKTDataPoint = (BYTE *)(p+1);
				DataLen -= sizeof(NETBEUIHeader);

				for(j=1 ; j < listenECB->fragmentCount ; j++ ) {
					if(DataLen <= 0) break;
					CopyLen = min(DataLen,listenECB->fragmentDescriptor[j].size);
					memcpy(listenECB->fragmentDescriptor[j].address, PKTDataPoint, CopyLen);
					PKTDataPoint = PKTDataPoint + listenECB->fragmentDescriptor[j].size;
					DataLen -= CopyLen;
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

	restore(i_state);
	return(0);
}

void NT_IPXtoNETBEUIHeader(ECB *sendECB)
{
	memset(&NTSendNETBEUIHeader,'\0',sizeof(NTSendNETBEUIHeader));
	NTSendNETBEUIHeader.Length[0] = 0x2C;
	NTSendNETBEUIHeader.Length[1] = 0x00;
	NTSendNETBEUIHeader.Delimiter[0] = 0xFF;
	NTSendNETBEUIHeader.Delimiter[1] = 0xEF;
	NTSendNETBEUIHeader.Command = 0x08;	 //SEND.DATAGRAM
	memcpy(NTSendNETBEUIHeader.DestName,sendECB->IPXSrcNETBEUIName,16);
	memcpy(NTSendNETBEUIHeader.SrcName,MyNetBEUIName,16);

	sendECB->fragmentDescriptor[0].size = sizeof(NETBEUIHeader);
	sendECB->fragmentDescriptor[0].address = &NTSendNETBEUIHeader;

	//NT_VERSION_2
	if(memcmp(sendECB->fragmentDescriptor[1].address,NT_CHECK_MARK1,NT_CHECK_MARK1_LEN) == 0)
		NSET16( sendECB->fragmentDescriptor[1].address, sendECB->socketNumber );
}


