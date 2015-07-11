#include <stdlib.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "prnqueue.h"
#include "prnport.h"
#include "ipx.h"
#include "ntps.h"
#include "ntudp.h"
#include "nps.h"
#include "joblog.h"

extern BYTE ReadPortStatus(BYTE port);
extern int urandom(unsigned int n);
extern uint16 NGET16( uint8 *pSrc );
extern void NSET16( uint8 *pDest, uint16 value );

extern NT_PORT_BLOCK	NTPortInfo[NUM_OF_PRN_PORT];
extern BYTE             IsNTEndPrint[NUM_OF_PRN_PORT];
extern struct parport 	PortIO[NUM_OF_PRN_PORT];

extern void UtilGetListViewData(LIST_VIEW *BoxInfo);
extern void UtilSetListViewData(LIST_VIEW *BoxInfo);
extern void UtilRemoveBusyStatus(uint8 *PrintStatus);
extern void UtilSetListViewData_Adv(LIST_VIEW_EXT *BoxInfo_E);

//from eeprom.c
extern uint8   			NTMaxRecvPacket;

uint16 	t_tran_byte_udp = 0;
BYTE 	Previous_BNo_UDP = 0;
int		util_socket;						//615wu //lwip socket number is 'int' type
struct  sockaddr_in  printsock;				//615wu //lwip sendto need 'from'

static Utility_Rec NTUDPUtilData;
static NTQueryAckData NTUDPSendAckData;
int16 peer_socket[NUM_OF_PRN_PORT];

static void NTStartPrintUDP(BYTE Version, struct sockaddr_in *fromsock, BYTE MaxPackets);
static int16 NTUDPOpenDataPort(BYTE PortNumber,struct sockaddr_in * fromsock);
static void NTEndPrintUDP(BYTE Version, struct sockaddr_in *fromsock);
static void NTQueryAckUDP (BYTE PortNumber,BYTE BlockCount,struct sockaddr_in * fromsock);
static void NTSendAckStartPrintUDP (BYTE Version, BYTE PrinterID, BYTE NTStatus,int16 Socket, struct sockaddr_in *fromsock);
static void NTSendAckEndPrintUDP (BYTE Version, BYTE PrinterID, struct sockaddr_in *fromsock);
static void NTDoCommandUDP (struct sockaddr_in *fromsock);

static void UtilConfigBoxUDP (struct sockaddr_in *fromsock);						// (81)
static void UtilViewBoxUDP (BYTE BroadCastMode, struct sockaddr_in *fromsock);		// (84)
static void UtilPrintStatusUDP (struct sockaddr_in *fromsock);						// (88)
static void UtilSendDeviceInfoUDP (BYTE PrinterID,struct sockaddr_in *fromsock);	// (90)
static void UtilReadPrinterDataUDP(BYTE,WORD,struct sockaddr_in *);					// (91)
static void UtilWritePrinterDataUDP(BYTE,WORD,struct sockaddr_in *);				// (92)
static void UtilLoadDefaultUDP(struct sockaddr_in *fromsock);						// (99)
static void UtilConfigBoxUDP_Adv (struct sockaddr_in *fromsock);          			// (A1)
static void UtilViewBoxUDP_Adv (BYTE BroadCastMode, struct sockaddr_in *fromsock);  // (A4)
#ifdef SECURITY_CONFIG
static void UtilSecuritySet (char *buf, struct sockaddr_in *fromsock); // (A5)

extern unsigned int Security_key_setting(char *buf); //Ron 1/30/05
#endif
void my_send_view_box( void *dataptr, int size, struct sockaddr_in *fromsock); //615wu

//for PACK_DATA_EXT
BYTE rec_buffer_udp[sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1+6];
BYTE *recbuffer_udp = rec_buffer_udp;

//615wu::No PSMain
extern cyg_sem_t NT_SIGNAL_PORT_1;

void NTStartPrintUDP(BYTE Version, struct sockaddr_in *fromsock, BYTE MaxPackets)
{
#ifdef USE_PS_LIBS
	BYTE NTUDPStatus, PortStatus;
	BYTE PortNumber;
	BYTE *PortName;
	int16 SocketNumber;

//ZOTIPS	armond_printf("Start UDP Printing \n");
	if(Version == NT_VERSION_4) { //9/8/99 added
		PortNumber = (NTUDPUtilData.Nt2.Data[0])-1;
		PortName   = (BYTE*)&NTUDPUtilData.Nt2.Data[1];
		
		PortStatus = ReadPortStatus(PortNumber);
		
		if(PortStatus == PORT_PRINTING) PortStatus = 0; //6/23/99
	} else {
		PortNumber = (NTUDPUtilData.Nt1.Data[0])-1;
		PortName   = (BYTE*)&NTUDPUtilData.Nt1.Data[1];
		PortStatus = 0;
	}

	if(PortNumber >= NUM_OF_PRN_PORT) {
	    return;
	}

	cyg_scheduler_lock();	//615wu::No PSMain

	NTUDPStatus = PrnGetPrinterStatus(PortNumber);
#ifndef SUPPORT_JOB_LOG
	PortName[NT_MAX_PORT_NAME_LENGTH] = '\0';
#endif //SUPPORT_JOB_LOG
	
//	if(PrnIsUnixHold(PortNumber)) NTUDPStatus = PrnAbortUsed;        //615wu::No PSMain
	if(!PortStatus) { //6/23/99
		switch(NTUDPStatus) {
			case PrnNoUsed:	//Start print
				PrnSetNTInUse(PortNumber);
	
				NTPortInfo[PortNumber].BlockNumber = 0x00;
#ifndef SUPPORT_JOB_LOG
			//Port Name
				memcpy(NTPortInfo[PortNumber].PortName,PortName,NT_MAX_PORT_NAME_LENGTH);
#endif SUPPORT_JOB_LOG
				NTPortInfo[PortNumber].RemoteSocket = *fromsock;
				NTPortInfo[PortNumber].IPXFrameType = TCPUDP;	//3/16/98
	            NTPortInfo[PortNumber].Version      = Version;	//9/8/99
	
				if( Version == NT_VERSION_4 )
				{
					if( MaxPackets == 0 )
						MaxPackets = 5;
				}
				else
					MaxPackets = 1;

				NTPortInfo[PortNumber].MaxRecvPackets = MaxPackets;
	
				IsNTEndPrint[PortNumber] = NTPortInfo[PortNumber].HaveRecvPacket = 0;
				
#ifdef SUPPORT_JOB_LOG
//			memcpy(&job_log->LoginUser, NTUDPUtilData.Nt2.Data + 1, 32);	// HostName
			JL_PutList(0, PortNumber, NTUDPUtilData.Nt2.Data + 33, 32);
#endif //SUPPORT_JOB_LOG

				break;
			case NTUsed:
				if(!NTPortInfo[PortNumber].HaveRecvPacket &&
				   NTPortInfo[PortNumber].RemoteSocket.sin_port == fromsock->sin_port &&
				   NTPortInfo[PortNumber].RemoteSocket.sin_addr.s_addr == fromsock->sin_addr.s_addr &&
				   !strcmp(NTPortInfo[PortNumber].PortName,PortName) 
#ifdef SUPPORT_JOB_LOG
			   && !strcmp(NTPortInfo[PortNumber].PortName,PortName)
#endif //SUPPORT_JOB_LOG
			    )
				{
					//request 2 times , send ACK again
				    NTUDPStatus = NTReUsed;  //12/31/98 added
					//Retry Start Printer !
					//Send Ack Start Printer lost, so PrintMonitor retry it !
				}
				break;
			case PrnAbortUsed: //start connect just hold it !
			default:
				break;
		}
	
		cyg_scheduler_unlock();	//615wu::No PSMain
	
		switch(NTUDPStatus) {
		case PrnNoUsed:
			if(NTUDPOpenDataPort(PortNumber, fromsock)) {
				return;
			}
			SocketNumber = peer_socket[PortNumber];
			NTUDPStatus = 0x00;
			break;
		case NTReUsed:	//12/31/98 added
			SocketNumber = peer_socket[PortNumber];
			NTUDPStatus = 0x00;
			break;
		default:
			SocketNumber = util_socket;
			NTUDPStatus = 0x10;
			break;
		}
	}
	else { //6/23/99
	
		cyg_scheduler_unlock();	//615wu::No PSMain
	
		SocketNumber = util_socket;
	
		if(NTUDPStatus == PrnNoUsed) NTUDPStatus = 0x00;
		else NTUDPStatus = 0x10;
	
		NTUDPStatus |= PortStatus;
	}
	//NTUDPStatus : 00: OK, 01:PaperOut, 02:OffLine, 1X: Busy

	NTSendAckStartPrintUDP(Version,PortNumber+1,NTUDPStatus,SocketNumber,&printsock);

//615wu::No PSMain	
	if(NTUDPStatus == PrnNoUsed)
		cyg_semaphore_post( &NT_SIGNAL_PORT_1 );
#endif /* USE_PS_LIBS */
}

int16 NTUDPOpenDataPort(BYTE PortNumber,struct sockaddr_in * fromsock)
{
#ifdef USE_PS_LIBS 
	struct timeval rcv_timeout;
	
	if(peer_socket[PortNumber] != 0)
		close(peer_socket[PortNumber]);

	if((peer_socket[PortNumber] = socket( AF_INET, SOCK_DGRAM, 0 )) < 0) {
			return -1;
	}
	rcv_timeout.tv_usec = 500;
	rcv_timeout.tv_sec = 1;
	setsockopt (peer_socket[PortNumber], 
				SOL_SOCKET, 
				SO_RCVTIMEO,
				&rcv_timeout,
				sizeof(struct timeval));
				
	memcpy(&printsock, fromsock, sizeof(struct sockaddr_in));			
#endif
	return 0;
}

void NTEndPrintUDP (BYTE Version, struct sockaddr_in *fromsock)
{
#ifdef USE_PS_LIBS
	BYTE PrinterID;

//ZOTIPS	armond_printf("UDP Printing end\n");
	if(Version == NT_VERSION_4) {
		PrinterID = NTUDPUtilData.Nt2.Data[0];
	} else {
		PrinterID = NTUDPUtilData.Nt1.Data[0];
	}



	if(PrinterID > NUM_OF_PRN_PORT ||
	   NTPortInfo[PrinterID-1].RemoteSocket.sin_addr.s_addr != fromsock->sin_addr.s_addr
	)  {
		return;
	}

	if( ( Version==NT_VERSION_4
	      ?strcmp(&NTUDPUtilData.Nt2.Data[1], NT_END_PRINT)==0
	      :strcmp(&NTUDPUtilData.Nt1.Data[1], NT_END_PRINT)==0 )
    ){
		NTPortInfo[PrinterID-1].HaveRecvPacket = 1;
		IsNTEndPrint[PrinterID-1] = 1;
		NTSendAckEndPrintUDP(Version,PrinterID,fromsock);
	}

#ifdef SUPPORT_JOB_LOG
	JL_EndList(PrinterID-1, 0);		// Completed. George Add January 26, 2007
#endif //SUPPORT_JOB_LOG
#endif /* USE_PS_LIBS */
}

BYTE NTRequestUDP (BYTE PortNumber,BYTE *Data,int16 *DataLength)
{
#ifdef USE_PS_LIBS
	int16 RecvBytes;
	BYTE  RecvRetryCount,i;
	WORD  RecvLen; //6/25/99 ONE PACKET ONLY

//9/8/99 remark	NTRequestData  *NTUDPRecvData;  //12/3/98 added
//	BYTE  RecvPackets,VersionOffset; //Ron
	BYTE  RecvPackets;
	int   EmptyDataCount = 0;
	int fromlen = sizeof(struct sockaddr);
	char  *recvbuf = (char *)Data - sizeof(NTReqCmdHeader); //Ron
	NTReqCmdHeader *NTReqcmdheader = (NTReqCmdHeader *)recvbuf; //Ron
	char  tempbuf[11]; //Ron ... sizeof((NTReqCmdHeader) = 11

//		VersionOffset = sizeof(NEW_NTRequestData) - sizeof(NTReqData); //Ron

	if( NTPortInfo[PortNumber].Version == NT_VERSION_4 )
		RecvPackets = NTPortInfo[PortNumber].MaxRecvPackets;
	else
		RecvPackets = 1;
	
	*DataLength = 0;  //6/25/99 ONE PACKET ONLY
	
	do {
		NTQueryAckUDP(PortNumber,1,&printsock);
		for(i = 0 ; i < RecvPackets; i++) {
//			NTUDPRecvData = (NTReqData *) (Data+VersionOffset);  //Ron
			RecvRetryCount = 0;

			/* Temp the first bytes( lenth sizeof(NTReqCmdHeader)) to 
			   receive struct NTReqCmdHeader data. Recover it at last ...Ron*/	
			memcpy(tempbuf, recvbuf, sizeof(NTReqCmdHeader)); //Ron
			
			do {	        	    	     

//Ron				RecvBytes = recv(peer_socket[PortNumber],
//Ron	    	    	         recvbuf,
//Ron	        	    	     sizeof(NTRequestData), 0 );
				
	        	RecvBytes = recvfrom(peer_socket[PortNumber], recvbuf, sizeof(NTRequestData), 0,	//Ron
		      		(struct sockaddr *)&printsock, &fromlen);   	     							//Ron

				if(IsNTEndPrint[PortNumber]) {	//move from above	12/6/98
					*DataLength = 0;  //6/25/99 ONE PACKET ONLY
					/* Recover data in PrintQueue ... Ron */
					memcpy(recvbuf, tempbuf, sizeof(NTReqCmdHeader)); //Ron
					
					return (PRN_Q_EOF);
				}

				if(RecvBytes < 0 || ++RecvRetryCount > NTUDP_RETRY_COUNT ) {
					/* Recover data in PrintQueue ... Ron */
					memcpy(recvbuf, tempbuf, sizeof(NTReqCmdHeader)); //Ron
					
					if(*DataLength)  return (PRN_Q_NORMAL); //7/05/99 ONE PACKET ONLY
					else return (PRN_Q_ABORT);              //6/05/99 ONE PACKET ONLY
				}

#ifdef SUPPORT_JOB_LOG
				JL_AddSize(PortNumber, RecvBytes);
#endif SUPPORT_JOB_LOG

			} while(
//Ron			  (NTUDPRecvData->BlockNumber != NTPortInfo[PortNumber].BlockNumber ||
//Ron			  (NTUDPRecvData->BlockNumber != NTPortInfo[PortNumber].BlockNumber || 
//Ron			  NTUDPRecvData->PrinterID != (PortNumber+1))
			  NTReqcmdheader->BlockNumber != NTPortInfo[PortNumber].BlockNumber || 			  		
			  NTReqcmdheader->PrinterID != (PortNumber+1)
			);

//Ron			RecvLen = NTUDPRecvData->DataLength;  //6/25/99	ONE PACKET ONLY
			RecvLen = NTReqcmdheader->DataLength;  //6/25/99	ONE PACKET ONLY
			if((BYTE)RecvLen) RecvLen -= 0x100;	  //6/25/99 ONE PACKET ONLY
			*DataLength += RecvLen;	  //6/25/99 ONE PACKET ONLY
//Ron			memcpy(Data,NTUDPRecvData->Data,RecvLen);  //12/3/98 remarked 6/25/99 ONE PACKET ONLY

			/* Recover data in PrintQueue ... Ron */
			memcpy(recvbuf, tempbuf, sizeof(NTReqCmdHeader)); //Ron
			
			Data +=	RecvLen;		  //6/25/99 ONE PACKET ONLY
			recvbuf += RecvLen; //Ron
			NTReqcmdheader = (NTReqCmdHeader *)recvbuf; //Ron
			
			NTPortInfo[PortNumber].BlockNumber++;
			NTPortInfo[PortNumber].HaveRecvPacket = 1;
		} //for( i = 0 ...........	//6/25/99 ONE PACKET ONLY

		// Charles 2001/11/20, wait for print data
		if( *DataLength == 0 )
		{
			EmptyDataCount++;
			if(EmptyDataCount>20)	return(PRN_Q_HOLD);			
			if( EmptyDataCount > 3 ) ppause( EmptyDataCount );
			if( EmptyDataCount > 20 ) EmptyDataCount = 20;
			
			cyg_thread_yield();
			
		}

	} while(*DataLength == 0);
#endif /* USE_PS_LIBS */
	return (PRN_Q_NORMAL);
}

void NTQueryAckUDP (BYTE PortNumber,BYTE BlockCount,struct sockaddr_in * fromsock)
{
#ifdef USE_PS_LIBS
	if(NTPortInfo[PortNumber].Version == NT_VERSION_4) {
		memcpy(NTUDPSendAckData.Nt2.Mark,NT_CHECK_MARK2, NT_CHECK_MARK2_LEN);
		NTUDPSendAckData.Nt2.Cmd[0]  = 0x8B;
		NTUDPSendAckData.Nt2.Cmd[1]  = 0x00;
		NTUDPSendAckData.Nt2.PrinterID   = PortNumber+1;
		NTUDPSendAckData.Nt2.BlockNumber = NTPortInfo[PortNumber].BlockNumber;
		NTUDPSendAckData.Nt2.Mode        = NORMAL_MODE;	//0:Normal 1:Burst
		NTUDPSendAckData.Nt2.BlockCount  = BlockCount;   //how many block want to receive !
	} else {
		memcpy(NTUDPSendAckData.Nt1.Mark,NT_CHECK_MARK1, NT_CHECK_MARK1_LEN);
		NTUDPSendAckData.Nt1.Cmd[0]  = 0x8B;
		NTUDPSendAckData.Nt1.Cmd[1]  = 0x00;
		NTUDPSendAckData.Nt1.PrinterID   = PortNumber+1;
		NTUDPSendAckData.Nt1.BlockNumber = NTPortInfo[PortNumber].BlockNumber;
		NTUDPSendAckData.Nt1.Mode        = NORMAL_MODE;	//0:Normal 1:Burst
		NTUDPSendAckData.Nt1.BlockCount  = BlockCount;   //how many block want to receive !
	}

	sendto(peer_socket[PortNumber],&NTUDPSendAckData,sizeof(NTUDPSendAckData), 0, 
		(struct sockaddr *)fromsock,sizeof(struct sockaddr));
#endif /* USE_PS_LIBS */
}

void NTSendAckStartPrintUDP (BYTE Version, BYTE PrinterID, BYTE NTStatus,int16 SocketNumber, struct sockaddr_in *fromsock)
{
#ifdef USE_PS_LIBS
	NTStartRespData	*RespData;

	if(Version == NT_VERSION_4) {

		RespData =  (NTStartRespData *) NTUDPUtilData.Nt2.Data;
		// Setup Utility Protocol
		NTUDPUtilData.Nt2.Cmd[0] = 0x8A;
		NTUDPUtilData.Nt2.Cmd[1] = 0x00;

		RespData->MaxDataLen = NT_MAX_RECV_LEN;

		if( NTPortInfo[PrinterID-1].MaxRecvPackets > NTMaxRecvPacket )
			NTPortInfo[PrinterID-1].MaxRecvPackets = NTMaxRecvPacket;


		RespData->MAXPacket  = NTPortInfo[PrinterID-1].MaxRecvPackets;

	} else {

		RespData =  (NTStartRespData *) NTUDPUtilData.Nt1.Data;
		// Setup Utility Protocol
		NTUDPUtilData.Nt1.Cmd[0] = 0x8A;
		NTUDPUtilData.Nt1.Cmd[1] = 0x00;
	}

	RespData->PrinterID  = PrinterID;	 //(1 - N)
	RespData->Status	 = NTStatus;      //Status
	RespData->Mode       = NORMAL_MODE;  //mode ;

	sendto(SocketNumber,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
		(struct sockaddr *)fromsock,sizeof(struct sockaddr));
#endif /* USE_PS_LIBS */
}

void NTSendAckEndPrintUDP (BYTE Version, BYTE PrinterID, struct sockaddr_in *fromsock)
{
	if(Version == NT_VERSION_4) {
		// Setup Utility Protocol
		NTUDPUtilData.Nt2.Cmd[0] = 0x8C;
		NTUDPUtilData.Nt2.Cmd[1] = 0x00;
		NTUDPUtilData.Nt2.Data[0] = PrinterID;	//Printer ID

	} else {
		// Setup Utility Protocol
		NTUDPUtilData.Nt1.Cmd[0] = 0x8C;
		NTUDPUtilData.Nt1.Cmd[1] = 0x00;
		NTUDPUtilData.Nt1.Data[0] = PrinterID;	//Printer ID
	}

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
	(struct sockaddr *)fromsock,sizeof(struct sockaddr));
}

//------------------------- NT UDP UTILITY -------------------------------------
/*void NTUDPNeedReBroadCast(struct ether *hdr, struct mbuf *bp)
{
	BYTE *data = bp->data;

	// Must be a broadcast udp packet
	if(memcmp(hdr->dest, Ether_bdcst, 6)) return;


	if( ((data[0x00] >> 4) != IPVERSION)          || //Check IP version
	     (data[0x09] != UDP_PTCL)                 || //UDP Packet
	     (NGET16(data+0x16) != IPPORT_NTPS_UTIL))    //NTUDP UTILITY PORT NO
		return;

	//NT_VERSION_4
	//1C 1D 1E = NET
	//1F       = TYPE
	//20 21    = 0x04 0x00

	//NT_VERSION_2
	//1C 1D    = ZO
	//1E 1F	   = 0x00 0x00

	if( ((memcmp(data+0x1C,NT_CHECK_MARK2,NT_CHECK_MARK2_LEN) == 0) &&
	     (NGET16(data+0x20) == 0x0004) ) // Cmd[0] = 04, Cmd[1] = 00
	     ||
	    ((memcmp(data+0x1C,NT_CHECK_MARK1,NT_CHECK_MARK1_LEN) == 0) &&
	     (NGET16(data+0x1E) == 0x0000) ) // Cmd[0] = 00, Cmd[1] = 00
	) {
		arp_add(DWordSwap(NGET32(data+0x0C)),ARP_ETHER,hdr->source,0);
	}
}
*/

extern int Network_TCPIP_ON;

void NTUtilityUDP(cyg_addrword_t data)
{
	struct sockaddr_in util_sockaddr;
	struct sockaddr_in fromsock;
	int fromlen;
	BYTE Version;
	
	while( Network_TCPIP_ON == 0 )
		ppause(100);
	
	util_sockaddr.sin_family = AF_INET;
	util_sockaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	util_sockaddr.sin_port = htons (IPPORT_NTPS_UTIL);
	util_sockaddr.sin_len = sizeof(util_sockaddr);

	if((util_socket = socket( AF_INET, SOCK_DGRAM, 0 )) < 0) {
		return;
	}
	
//ZOTIPS	armond_printf("open socket cnt = %d \n",util_socket);
	
	if(bind(util_socket, (struct sockaddr *)&util_sockaddr,sizeof(util_sockaddr)) < 0) {
		return;
	}

	for (;;) {
		fromlen = sizeof(fromsock);
		if(recvfrom(util_socket, &NTUDPUtilData, sizeof(NTUDPUtilData), 0,
		      (struct sockaddr *)&fromsock, &fromlen) < 1) 
		      		continue;
		

	
	
		if(memcmp(NTUDPUtilData.Nt2.Mark,NT_CHECK_MARK2,NT_CHECK_MARK2_LEN) == 0) {
			Version = NT_VERSION_4;
		} else {
			if(memcmp(NTUDPUtilData.Nt1.Mark,NT_CHECK_MARK1,NT_CHECK_MARK1_LEN) == 0) {
				Version = NT_VERSION_2;
			} else
				continue;
		}
		if(Version == NT_VERSION_4) NTDoCommandUDP(&fromsock);
		
	}
}

void NTDoCommandUDP (struct sockaddr_in *fromsock)
{
	switch(NTUDPUtilData.Nt2.Cmd[0]) {
		case CMD01: // Config Box
//#ifndef PACK_DATA_EXT		
#ifndef O_AXIS
			UtilConfigBoxUDP(fromsock);
#endif			
//#endif			
			break;
		case CMD03:	// ReBoot
			if(!memcmp(MyPhysNodeAddress,NTUDPUtilData.Nt2.Data,6))
				Reset();	//615wu //eCos
			break;
		case CMD04: //Send List View Box
			UtilViewBoxUDP(NTUDPUtilData.Nt2.Cmd[1], fromsock);
			break;
		case CMD08: // Request Printer's Status
			UtilPrintStatusUDP(fromsock);
			break;
		case CMD0A:	// Start Print
			NTStartPrintUDP(NT_VERSION_4, fromsock, NTUDPUtilData.Nt2.Cmd[1] );
			break;
		case CMD0C: // End PRINT
			NTEndPrintUDP(NT_VERSION_4, fromsock);
			break;
		case CMD10: //Send Device Info 4/18/2000
			UtilSendDeviceInfoUDP(NTUDPUtilData.Nt2.Data[0],fromsock);
			break;
		case CMD11:
			UtilReadPrinterDataUDP( NTUDPUtilData.Nt2.Data[0], NGET16( &NTUDPUtilData.Nt2.Data[1] ) ,fromsock);
			break;
		case CMD12:
			UtilWritePrinterDataUDP( NTUDPUtilData.Nt2.Data[0], NGET16( &NTUDPUtilData.Nt2.Data[1] ) ,fromsock);
			break;
		case CMD19:
			UtilLoadDefaultUDP(fromsock);
			break;
#ifdef PACK_DATA_EXT
		case CMD21: // Config Box
		
#ifndef SECURITY_CONFIG		
			UtilConfigBoxUDP_Adv(fromsock);
#endif			
			break;
		case CMD24: //Send List View Box
			UtilViewBoxUDP_Adv(NTUDPUtilData.Nt2.Cmd[1], fromsock);
			break;
#endif //PACK_DATA_EXT			

#ifdef SECURITY_CONFIG
		case CMD25: //pre-share key security setting ... Ron 1/30/05
			UtilSecuritySet((char *)&NTUDPUtilData, fromsock);
			break;			
#endif
		default: // If the CMD is UnKnown , Give Up the Packet!
			break;
	} //switch (CMD) {
}

//(81) Write config data into EEPROM.
void UtilConfigBoxUDP (struct sockaddr_in *fromsock)
{
	BYTE Result = 0x00;

	//check this data for myself or not (broadcast) !
	if(memcmp(MyPhysNodeAddress,((LIST_VIEW *)NTUDPUtilData.Nt2.Data)->NodeID,6) != 0) return;

	UtilGetListViewData((LIST_VIEW *)NTUDPUtilData.Nt2.Data);


	if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xff;

	// Setup Utility Protocol
	NTUDPUtilData.Nt2.Cmd[0] = 0x81;
	NTUDPUtilData.Nt2.Cmd[1] = Result;

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
		(struct sockaddr *)fromsock,sizeof(struct sockaddr));
}


extern struct netif *Lanface; 
//ZOTIPS extern struct netif *WLanface; 
extern err_t SendPacket(struct netif *netif, struct pbuf *p);
extern err_t ethernetif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
//(84) Send Box data for utility view & config it.
void UtilViewBoxUDP (BYTE BroadCastMode, struct sockaddr_in *fromsock)
{
	int i;
	
	NTUDPUtilData.Nt2.Cmd[0]=0x84;
	NTUDPUtilData.Nt2.Cmd[1]=0x00;
	
	memset(NTUDPUtilData.Nt2.Data, 0, 1024);
	
	UtilSetListViewData((LIST_VIEW *)NTUDPUtilData.Nt2.Data);
	
	if( BroadCastMode == 0 )
	{	
		my_send_view_box( &NTUDPUtilData, sizeof(NEW_NT_Utility_Rec),(struct sockaddr *)fromsock);
	}
	else{
		sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
	}
	
}

//(88) Send Print HW Status.
void UtilPrintStatusUDP (struct sockaddr_in *fromsock)
{
#ifdef USE_PS_LIBS
	BYTE PortNumber;

	NTUDPUtilData.Nt2.Cmd[0] = 0x88;
	NTUDPUtilData.Nt2.Cmd[1] = 0x00;


	//print status
	NTUDPUtilData.Nt2.Data[0] = ReadPrintStatus();	//HW status

	UtilRemoveBusyStatus(&NTUDPUtilData.Nt2.Data[0]);
#ifdef NOVELL_PS
	NTUDPUtilData.Nt2.Data[1] = NovellConnectFlag;
#endif
	//Using Status
	for(PortNumber = 0; PortNumber < NUM_OF_PRN_PORT; PortNumber++) {
		NTUDPUtilData.Nt2.Data[PortNumber+2] = PrnGetPrinterStatus(PortNumber);
		if(NTUDPUtilData.Nt2.Data[PortNumber+2]) NTUDPUtilData.Nt2.Data[PortNumber+2]--;
	}
	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
#endif /* USE_PS_LIBS */
}

//-------------------------------------------------------------------
//(90) Send Device Info. 4/18/2000 added
void UtilSendDeviceInfoUDP (BYTE PrinterID,struct sockaddr_in *fromsock)
{
#ifdef USE_PS_LIBS
	BYTE *p;
	WORD len;

	NTUDPUtilData.Nt2.Cmd[0] = 0x90;
	NTUDPUtilData.Nt2.Cmd[1] = 0x00;

	//Printer Status
	NTUDPUtilData.Nt2.Data[0] = PrinterID;
	if(PrinterID > NUM_OF_PRN_PORT) {
		memset(&NTUDPUtilData.Nt2.Data[1],'\0',10);
	} else {
		memset(&NTUDPUtilData.Nt2.Data[1],'\0',10);

		p = &NTUDPUtilData.Nt2.Data[1];

		if( PortIO[PrinterID-1].Manufacture != NULL
			&& isprint( PortIO[PrinterID-1].Manufacture[0] ) )
		{
			len = strlen(PortIO[PrinterID-1].Manufacture);
			memcpy(p,PortIO[PrinterID-1].Manufacture,len+1);
		} else {
			len = 0;
			p[0] = '\0';
		}	
		p += (len+1);

		if( PortIO[PrinterID-1].Model != NULL
			&& isprint( PortIO[PrinterID-1].Model[0] ) )
		{
	    	len = strlen(PortIO[PrinterID-1].Model);
			memcpy(p,PortIO[PrinterID-1].Model,len+1);
		} else {
			len = 0;
			p[0] = '\0';
		}	
		p += (len+1);

		if( PortIO[PrinterID-1].CommandSet != NULL
			&& isprint( PortIO[PrinterID-1].CommandSet[0] ) )
		{
			len = strlen(PortIO[PrinterID-1].CommandSet);
			memcpy(p,PortIO[PrinterID-1].CommandSet, len+1);
		} else {
			len = 0;
			p[0] = '\0';
		}	
		p += (len+1);

		*p = '\0';
	}

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
#endif /* USE_PS_LIBS */
}

//-------------------------------------------------------------------
//(91) Read Printer Data. 2001/06/27 added
void UtilReadPrinterDataUDP( BYTE PrinterID, WORD DataSize ,struct sockaddr_in *fromsock)
{
#ifdef USE_PS_LIBS
	uint8  PrnPort = PrinterID - 1;
	uint16 ReadSize = 0;

	NTUDPUtilData.Nt2.Cmd[0] = 0x91;

	if( ( G_PortReady & (0x01 << PrnPort) )
		&& PrnGetAvailQueueNO(PrnPort) == PRNQUEUELEN
		&& PortReady(PrnPort) )
	{
		NTUDPUtilData.Nt2.Cmd[1] = 0x00;
	}
	else
	{
		NTUDPUtilData.Nt2.Cmd[1] = 0x01;
	}

	NTUDPUtilData.Nt2.Data[0] = PrinterID;

	NSET16( &NTUDPUtilData.Nt2.Data[1], ReadSize );

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
#endif
}

//-------------------------------------------------------------------
//(92) Write Printer Data. 2001/06/27 added
void UtilWritePrinterDataUDP( BYTE PrinterID, WORD DataSize ,struct sockaddr_in *fromsock)
{
#ifdef USE_PS_LIBS
	uint16 WriteSize = 0;
	uint8 result;

	result = PrnPrintPattern( PrinterID - 1, &NTUDPUtilData.Nt2.Data[3], DataSize );

	if( result == 0 ) WriteSize = DataSize;

	NTUDPUtilData.Nt2.Cmd[0] = 0x92;
	NTUDPUtilData.Nt2.Cmd[1] = result;
	NTUDPUtilData.Nt2.Data[0] = PrinterID;
	NSET16( &NTUDPUtilData.Nt2.Data[1], WriteSize );

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
#endif
}

//-------------------------------------------------------------------
//(99) Load Default Settings. 2001/07/19 added
void UtilLoadDefaultUDP(struct sockaddr_in *fromsock)
{
	NTUDPUtilData.Nt2.Cmd[0] = 0x99;
	NTUDPUtilData.Nt2.Cmd[1] = 0x00;

	ResetToDefalutFlash(0,1,0);

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
}

//-------------------------------------------------------------------
//(A1) Write config data into EEPROM.
void UtilConfigBoxUDP_Adv (struct sockaddr_in *fromsock)
{
	uint8 Result = 0x00;
	uint8 Block_No = NTUDPUtilData.Nt2.Data[0];
	uint8 total_block = NTUDPUtilData.Nt2.Data[1];
	uint16 Block_Size = NTUDPUtilData.Nt2.Data[2] + 256 * NTUDPUtilData.Nt2.Data[3];
	
	// Check this data for myself or not (broadcast) !
	if( memcmp(MyPhysNodeAddress, NTUDPUtilData.Nt2.Data + 4, 6) != 0 ) 
		return;

	if ( (Block_No > Previous_BNo_UDP+1) || (Block_No < Previous_BNo_UDP ) ) 
		return;

	if ( Block_No ==1 ) {
		t_tran_byte_udp = 0;	// Jesse added this in 716U2W at build0005 on December 10, 2010.
		memset(rec_buffer_udp,0,sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1+6);
	}
	Previous_BNo_UDP = Block_No;
	
	memcpy( recbuffer_udp + t_tran_byte_udp, NTUDPUtilData.Nt2.Data + 4 + 6, Block_Size);
	t_tran_byte_udp += Block_Size;
	memset(NTUDPUtilData.Nt2.Data,0,sizeof(NTUDPUtilData.Nt2.Data));
	
	if (Block_No == total_block){
		t_tran_byte_udp = 0;
		Previous_BNo_UDP = 0;
		UtilGetListViewData( (LIST_VIEW *)recbuffer_udp );
		recbuffer_udp += ( sizeof(LIST_VIEW) - 1 );
		UtilGetListViewData_Adv( (LIST_VIEW_EXT *)recbuffer_udp );
		recbuffer_udp = rec_buffer_udp;

#ifndef _PC
		if(WriteToEEPROM(&EEPROM_Data) != 0)
			Result = 0xff;
#endif

		// Setup Utility Protocol
		NTUDPUtilData.Nt2.Cmd[0] = 0xA1;
		NTUDPUtilData.Nt2.Cmd[1] = Result;
		NTUDPUtilData.Nt2.Data[0] = Block_No;
		NTUDPUtilData.Nt2.Data[1] = total_block;

		sendto(util_socket, &NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0,
				(struct sockaddr *)fromsock,sizeof(struct sockaddr));
	}
}

//-------------------------------------------------------------------
//(A4) Send Box data for utility view & config it.
void UtilViewBoxUDP_Adv (BYTE BroadCastMode, struct sockaddr_in *fromsock)
{
	BYTE *p,*init_p;
	uint8 total_block;
	uint16 Block_Size, Block_Offset = 0;
	uint16 packsize = 1024 - 4 - 6;	// 4 bytes: Block number, Total block number, Block size x 2
									// 6 bytes: Source Node ID
	uint16 datasize = sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1;
	uint8 Block_No = NTUDPUtilData.Nt2.Data[0];
	
	if( Block_No != 1 ) return;
	
	p = mallocw(datasize);
	init_p = p;
	memset(p, 0, datasize);
	total_block = datasize / packsize;
	if ( ( datasize % packsize )!=0 )
		total_block++;
		
	UtilSetListViewData( (LIST_VIEW *)p );
	p += (sizeof(LIST_VIEW) - 1);
	UtilSetListViewData_Adv( (LIST_VIEW_EXT *)p );
	
	ppause(urandom(10)+1);	//delay 1 - 10 ms 9/9/99
	
	while (Block_No <= total_block)
	{
		if (Block_No == total_block)
			Block_Size = (datasize % packsize == 0 )? packsize:(datasize % packsize);
		else
			Block_Size = packsize;
			
		Block_Offset = (Block_No - 1) * packsize;
		
		NTUDPUtilData.Nt2.Cmd[0]=0xA4;
		NTUDPUtilData.Nt2.Cmd[1]=0x00;	// OK
		NTUDPUtilData.Nt2.Data[0]= Block_No;		// Block number
		NTUDPUtilData.Nt2.Data[1]= total_block;		// Total block number
		NSET16( &NTUDPUtilData.Nt2.Data[2], Block_Size );	// Block size x 2
		memcpy(NTUDPUtilData.Nt2.Data + 4, init_p, 6);		// Source Node ID
		memcpy(NTUDPUtilData.Nt2.Data + 10, init_p + Block_Offset, Block_Size);	// Data

		if(BroadCastMode == 0)
		{
		 	my_send_view_box( &NTUDPUtilData, sizeof(NEW_NT_Utility_Rec) - 1024 + Block_Size + 10,
		 		(struct sockaddr *)fromsock);
		}
	//	send(util_socket, &NTUDPUtilData,sizeof(NTUDPUtilData), 0);
		else{
			sendto(util_socket, &NTUDPUtilData,sizeof(NEW_NT_Utility_Rec) - 1024 + Block_Size + 10, 0,
				(struct sockaddr *)fromsock,sizeof(struct sockaddr)); //7/18/2000 changed
		}
		ppause(5);
		Block_No++;
	}
	
	free(init_p);
}

/* For Pre-share key security setting ... Ron 1/30/05 */
#ifdef SECURITY_CONFIG
void UtilSecuritySet (char *buf, struct sockaddr_in *fromsock)
{
	unsigned int result =0;
	int BroadCastMode = ((NEW_NT_Utility_Rec *)buf)->Cmd[1];
	char *Data = ((NEW_NT_Utility_Rec *)buf)->Data;
	
	result = Security_key_setting(Data);
		
	((NEW_NT_Utility_Rec *)buf)->Cmd[0] = 0xA5;
	((NEW_NT_Utility_Rec *)buf)->Cmd[1] = 0x00;
		
	if (result == 1){
		if(BroadCastMode == 0)
		{
		 	my_send_view_box( &NTUDPUtilData, sizeof(NEW_NT_Utility_Rec),
		 		(struct sockaddr *)fromsock);
		}
		else{
		sendto(util_socket, &NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
		}
	}
}
#endif

void my_send_view_box( void *dataptr, int size, struct sockaddr_in *fromsock)
{
	struct pbuf *p=NULL,*q=NULL;
	struct ip_addr *ipaddr=NULL;
	struct netif *temp_netif=NULL;
	struct udp_hdr *udphdr=NULL;
	int i=0;
	
	// Creat a temp IP:1.2.3.4
	ipaddr = (struct ip_addr *)malloc(sizeof(struct ip_addr));
	if( ipaddr == NULL )
		goto end;
	ipaddr->addr = htonl(0x01020304);
	
	// alloc a new pbuf
	p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_REF);
	if( p == NULL)
		goto end;

	p->payload = dataptr;
	p->len = p->tot_len = size;

    /* allocate udp header in a seperate new pbuf */
    q = pbuf_alloc(PBUF_IP, UDP_HLEN, PBUF_RAM);
    /* new header pbuf could not be allocated? */
    if (q == NULL) {
    	goto end;
     }
    /* chain header q in front of given pbuf p */
    pbuf_chain(q, p);
	
	// fill udp header 	
	udphdr = q->payload;
	udphdr->src = htons(0x5050);
	udphdr->dest = (fromsock->sin_port);
	/* in UDP, 0 checksum means 'no checksum' */
	udphdr->chksum = 0x0000; 
	udphdr->len = htons(q->tot_len);
	  		
	udphdr->chksum = inet_chksum_pseudo(q, ipaddr, &(fromsock->sin_addr),
    	 IP_PROTO_UDP, q->tot_len);
	
	if (udphdr->chksum == 0x0000) udphdr->chksum = 0xffff;
	
	// Creat a temp netif for IP:1.2.3.4 MASK:0.0.0.0		
	temp_netif = (struct netif *)malloc(sizeof(struct netif));
	if( temp_netif == NULL )
		goto end;			
	memset( temp_netif, 0, sizeof(struct netif));
	temp_netif->ip_addr.addr = 0x01020304;
	temp_netif->netmask.addr = 0x0;
	temp_netif->output = ethernetif_output;
	temp_netif->linkoutput = SendPacket;
	
	/* set MAC hardware address length */
	temp_netif->hwaddr_len = 6;
	
	/* set MAC hardware address */
	for ( i = 0; i < 6; ++i)
    {
        temp_netif->hwaddr[i] = MyPhysNodeAddress[i];
    }
	
	/* maximum transfer unit */
	temp_netif->mtu = 1500;	
	
	// send to ip layer
	ip_output_if (q, ipaddr, &(fromsock->sin_addr), 255, 0, 17, temp_netif);
		
end:		
	pbuf_free(q);
	pbuf_free(p);
	free(temp_netif);
	free(ipaddr);
	

}
