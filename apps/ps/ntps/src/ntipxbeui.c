#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include <stdlib.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "prnqueue.h"
#include "prnport.h"
#include "nps.h"
#include "ipx.h"
#include "netbeui.h"
#include "ntps.h"
#include "led.h"
#include "joblog.h"

//IPXUpgrade Thread initiation information
#define IPXUpgrade_TASK_PRI         	20	//ZOT716u2
#define IPXUpgrade_TASK_STACK_SIZE  	4096 //ZOT716u2 3072
static	uint8			IPXUpgrade_Stack[IPXUpgrade_TASK_STACK_SIZE];
static  cyg_thread		IPXUpgrade_Task;
static  cyg_handle_t	IPXUpgrade_TaskHdl;

cyg_sem_t SIGNAL_IPX_UPGRADE_DATA;
cyg_sem_t SIGNAL_IPX_UPGRADE_FINISH;

//615wu::No PSMain
extern cyg_sem_t NT_SIGNAL_PORT_1;

//****************IPX Request Data queue*****************

struct ipx_mbuf
{
	struct ipx_mbuf *anext;	/* Links packets on queues */
	NTRequestData	*data;
};

struct ipx_usock 
{
	struct ipx_mbuf *rcvq;	//Queue of pending IPX packet
	cyg_sem_t sem_f;	
};

struct ipx_usock IPXUSOCK;
//*******************************************************

//*********************for IPXUpgrade***************
ECB				 	SendUtilUpgradeECB, RecvUtilUpgradeECB;
IPXHeader        	SendUtilUpgradeIPXHeader, RecvUtilUpgradeIPXHeader;
IPX_Upgrade_Rec  	SendUtilUpgradeData, RecvUtilUpgradeData;
uint16				TotalUpgradeBlock;

#define _SendRequestBlock(BlockNo) 		SendUpgradeCmd(0x86,0x00,BlockNo)
#define _SendRequestBlockFail(BlockNo)  SendUpgradeCmd(0x86,0xFF,BlockNo)
#define _SendRequestFinish()            SendUpgradeCmd(0x87,0x00,0x0000)

//from eeprom.c
extern uint8   PSUpgradeMode;

void ListenUpgradePack(void);
void UtilUpgradeESR(void);
//*************************************************

ECB              	SendUtilECB, ReceiveUtilECB;
ECB           		NTSendPsECB, NTReceivePsECB;
IPXHeader        	SendUtilIPXHeader, ReceiveUtilIPXHeader;
IPXHeader      		NTSendPsIPXHeader, NTReceivePsIPXHeader;	
Utility_Rec      	SendUtilData, ReceiveUtilData;

NTQueryAckData 		NTSendQueryAckData;
//NTRequestData  		*NTReceiveRequestData;	//queue ipx

DWORD          		IPXTimeOut,IPXPktErr,IPXBlockErr;

#ifdef NDS_PS
#include "ndsqueue.h"
#else
BYTE	NDSConnectFlag = 0;	//615wu //disable
#endif

//from eeprom.c
extern uint8   			NTMaxRecvPacket;

//from ntps.c
extern NT_PORT_BLOCK	NTPortInfo[NUM_OF_PRN_PORT];
extern BYTE             IsNTEndPrint[NUM_OF_PRN_PORT];

//from proport.c
extern struct parport 	PortIO[NUM_OF_PRN_PORT];

//from psmain.c
extern int urandom(unsigned int n);
extern uint16 NGET16( uint8 *pSrc );
extern void NSET16( uint8 *pDest, uint16 value );

//from ntutil.c
extern void UtilGetListViewData(LIST_VIEW *BoxInfo);
extern void UtilSetListViewData(LIST_VIEW *BoxInfo);
extern void UtilRemoveBusyStatus(uint8 *PrintStatus);
extern void UtilSetListViewData_Adv(LIST_VIEW_EXT *BoxInfo_E);

//from wlanif.c
//temp extern uint8 mvWDomain;

//from hub.c
extern int usb_hub_link_devices( int port );

void ListenUtilPack(void);
void NTWaitforDataECB (void);
void NTQueryAckECB (BYTE PortNumber,BYTE BlockCount);

void UtilSendEEPROM(void);				// (80)
void UtilConfigBox(void);				// (81)
void UtilViewBox(void);					// (84)
void UtilStartUpgrade(void);			// (85)
void UtilPrintStatus(void);				// (88)
void UtilRecvEEPROM(void);				// (8D)
void UtilPrintPattern(void);			// (8E)
void UtilSendDeviceInfo(BYTE);			// (90)
void UtilReadPrinterData(BYTE,WORD);	// (91)
void UtilWritePrinterData(BYTE,WORD);	// (92)
void UtilGetFirmwareData(void);			// (98)
void UtilLoadDefault(void);				// (99)
void UtilPrintLoopback(void);			// (9B)
#ifdef WIRELESS_CARD
void UtilWirelessLoopback1(void);		// (9C)
void UtilWirelessLoopback2(void);		// (9D)
#endif
void UtilUSBLoopback(void);				// (9E)
void UtilSendEEPROM_Adv(void);			// (A0)
void UtilConfigBox_Adv(void);			// (21)
void UtilViewBox_Adv(void);				// (24)
void NTStartUSBTest(BYTE);				// (AA)
void NTUSBTestDataReply(BYTE);			// (AB)
void NTEndUSBTest(BYTE);				// (AC)
void UtilRecvEEPROM_Adv(void);			// (AD)

//BYTE	rec_buffer_ipx[sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1];
BYTE	rec_buffer_ipx[3*MAX_PS_READ_LEN];
BYTE	*recbuffer_ipx = rec_buffer_ipx;
uint16 	t_tran_byte_ipx = 0;
BYTE 	Previous_BNo_IPX = 0;

BYTE 	Previous_BNo = 0;

//****************IPX Request Data queue*****************
static void ipx_enqueue(struct ipx_mbuf **q,struct ipx_mbuf **bpp)
{
	struct ipx_mbuf *p;

	if(q == NULL || bpp == NULL || *bpp == NULL)
		return;
	
	if(*q == NULL){
		/* List is empty, stick at front */
		*q = *bpp;
	} else {
		for(p = *q ; p->anext != NULL ; p = p->anext)
			;
		p->anext = *bpp;
	}
	*bpp = NULL;	/* We've consumed it */		
}

/* Unlink a packet from the head of the queue */
static struct ipx_mbuf *ipx_dequeue(struct ipx_mbuf **q)
{
	struct ipx_mbuf *bp;
	int i_state;

	if(q == NULL)
		return NULL;
	i_state = dirps();

	if((bp = *q) != NULL){
		*q = bp->anext;
		bp->anext = NULL;
	}
	
	restore(i_state);
	return bp;
}

int32 ipx_recvfrom(struct ipx_usock *ipx_usock, void *Data, int PortNumber)
{
	NTReqData *RecvReqData;
	struct ipx_mbuf *buf;
	int ErrTmp;
	uint32 len = 0;
	
    if (cyg_semaphore_timed_wait (&ipx_usock->sem_f, cyg_current_time() + (TICKS_PER_SEC/3))) {
        buf = ipx_dequeue(&ipx_usock->rcvq);
        CYG_ASSERT(buf != NULL, "ipx_dequeue NULL");
    } 
    else
        return -1;
	// while((buf = ipx_dequeue(&ipx_usock->rcvq)) == NULL)
	// {			
	//     cyg_semaphore_init(&ipx_usock->sem_f,0);
	//     ErrTmp = (int)cyg_semaphore_timed_wait(&ipx_usock->sem_f,cyg_current_time() + (TICKS_PER_SEC/3));
    // 
	//     if(ErrTmp == 0)
	//         return -1;
	// }
	
	RecvReqData = (NTReqData *) &(buf->data)->Nt2.Cmd;	//queue ipx
#ifdef USE_PS_LIBS	
	if(RecvReqData->BlockNumber != NTPortInfo[PortNumber].BlockNumber)
	{
		free(buf->data);
		free(buf);
		return -2;	//Block Number error
	}
#endif /* USE_PS_LIBS */	
	if((BYTE)RecvReqData->DataLength)
		len = RecvReqData->DataLength - 0x100;
	else
		len = RecvReqData->DataLength;
	
	memcpy(Data, RecvReqData->Data, len);
	
	free(buf->data);
	free(buf);
	
	return (len);
	
}

//*******************************************************

BYTE NTRequestECB (BYTE PortNumber,BYTE *Data,int *DataLength)
{
#ifdef USE_PS_LIBS
	BYTE   ReceivePacket;
	BYTE  i;
    #if defined(NDWP2020)
    BYTE  j;
    #endif
	int  EmptyDataCount = 0;
	BYTE  RecvRetryCount;
	int32 RecvBytes;
	// BYTE  VersionOffset;
    // 
	// VersionOffset = sizeof(NEW_NTRequestData) - sizeof(NTReqData);

	if( NTPortInfo[PortNumber].Version == NT_VERSION_4 )
		ReceivePacket = NTPortInfo[PortNumber].MaxRecvPackets;
	else
		ReceivePacket = 1;

	*DataLength = 0;

	do {
		NTQueryAckECB(PortNumber,1);

		for(i = 0 ; i < ReceivePacket; i++) {

			RecvRetryCount = 0;
			
			do{
                #if defined(NDWP2020)
                for (j = 0; j < 30; j++) {
                #endif

				RecvBytes = ipx_recvfrom(&IPXUSOCK, Data, PortNumber); 
					
				if(IsNTEndPrint[PortNumber]) {
					//end of printing
					return (PRN_Q_EOF);
				}

                #if defined(NDWP2020)
                if(RecvBytes > 0)
                    break;
                }
                #endif
                
//				if(RecvBytes ==-1 || ++RecvRetryCount > NTPortInfo[PortNumber].MaxRecvPackets ) {
				if(RecvBytes ==-1  ) {
					if(*DataLength)  return (PRN_Q_NORMAL); //7/05/99 ONE PACKET ONLY
					else return (PRN_Q_ABORT);              //6/05/99 ONE PACKET ONLY
				}
			}while(RecvBytes < 0);
			
			NTPortInfo[PortNumber].BlockSize = RecvBytes;
			NTPortInfo[PortNumber].BlockNumber++;
			NTPortInfo[PortNumber].HaveRecvPacket = 1;
			
			*DataLength += NTPortInfo[PortNumber].BlockSize;
			Data += NTPortInfo[PortNumber].BlockSize;

#ifdef SUPPORT_JOB_LOG
			JL_AddSize(PortNumber, NTPortInfo[PortNumber].BlockSize);
#endif //SUPPORT_JOB_LOG			
			
		} //for( i = 0 ...
		// Charles 2001/11/20, wait for print data
		if( *DataLength == 0 )
		{
			EmptyDataCount++;
			if (EmptyDataCount > 20)  return (PRN_Q_HOLD);	//07/14/2003 wuidy reciver to much data 0 packet 
			if( EmptyDataCount > 3 ) ppause( EmptyDataCount );
			if( EmptyDataCount > 20 ) EmptyDataCount = 20;
		
			cyg_thread_yield();
			
		}

	} while(*DataLength == 0);
	NTPortInfo[PortNumber].IsFirst = 1;		//10/27/99
#endif /* USE_PS_LIBS */
	return (PRN_Q_NORMAL);
}

//Send IPX Upgrade Packet
void SendUpgradePacket(void)
{
	SendUtilUpgradeECB.ESRAddress = 0x00;
	SendUtilUpgradeECB.inUseFlag  = 0;
	SendUtilUpgradeECB.socketNumber  = UpgradeSocket;
	memcpy(SendUtilUpgradeECB.immediateAddress,RecvUtilUpgradeECB.immediateAddress, 6);

	SendUtilUpgradeECB.IPXFrameType = RecvUtilUpgradeECB.IPXFrameType;

	SendUtilUpgradeECB.fragmentCount = 2;
	SendUtilUpgradeECB.fragmentDescriptor[0].address = &SendUtilUpgradeIPXHeader;
	SendUtilUpgradeECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	SendUtilUpgradeECB.fragmentDescriptor[1].address = &SendUtilUpgradeData;
	SendUtilUpgradeECB.fragmentDescriptor[1].size    = sizeof(IPX_Upgrade_Rec) - UPGRADE_BLOCK_SIZE;

	// Setup IPX Header
	SendUtilUpgradeIPXHeader.packetType = 0x5a;  // Experimental Packet Type
	memcpy(SendUtilUpgradeIPXHeader.destination.network,RecvUtilUpgradeIPXHeader.source.network,12);

	// Setup Utility Protocol
	memcpy(SendUtilUpgradeData.Mark,NT_CHECK_MARK2,NT_CHECK_MARK2_LEN);

	// Send IPX Packet
	do {
		IPXSendPacket(&SendUtilUpgradeECB);
	} while(SendUtilUpgradeECB.inUseFlag);
}

//Send IPX Upgrade Cmd Packet
void SendUpgradeCmd(uint8 Cmd, uint8 Status, uint16 BlockNumber)
{
	SendUtilUpgradeData.Cmd[0] = Cmd;
	SendUtilUpgradeData.Cmd[1] = Status;
	SendUtilUpgradeData.BlockNumber = BlockNumber;

	SendUpgradePacket();
}

//Send IPX Utility Packet (5050)
void SendUtilityPacket(void)
{
	SendUtilECB.ESRAddress = 0x00;
	SendUtilECB.inUseFlag  = 0;
	SendUtilECB.socketNumber  = UtilitySocket;
	memcpy (SendUtilECB.immediateAddress,ReceiveUtilECB.immediateAddress, 6);

	SendUtilECB.IPXFrameType = ReceiveUtilECB.IPXFrameType;	//3/16/98

#ifdef NETBEUI
	if(ReceiveUtilECB.IPXFrameType == IPXBEUI) {
		memcpy(SendUtilECB.IPXSrcNETBEUIName,ReceiveUtilECB.IPXSrcNETBEUIName, 16 );
		//ListView
		if(SendUtilData.Nt2.Cmd[0] == 0x84 || SendUtilData.Nt1.Cmd[0] == 0x84) {
			SendUtilECB.socketNumber  = NETBEUI_ListViewSocket;
		}
	}
#endif NETBEUI

	SendUtilECB.fragmentCount = 2;
	SendUtilECB.fragmentDescriptor[0].address = &SendUtilIPXHeader;
	SendUtilECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	SendUtilECB.fragmentDescriptor[1].address = &SendUtilData;

	if(SendUtilData.Nt2.Cmd[0] == 0x80)
		SendUtilECB.fragmentDescriptor[1].size  = sizeof(NAME_Utility_Rec);
	else
		SendUtilECB.fragmentDescriptor[1].size  = sizeof(NEW_NT_Utility_Rec);

//for usb test
	if(SendUtilData.Nt2.Cmd[0] == 0xAB)
	{
		NTReqData *RespData;
		WORD 	  DataLength;
		
		RespData = (NTStartRespData *) SendUtilData.Nt2.Cmd;
		DataLength = WordSwap(RespData->DataLength);
		SendUtilECB.fragmentDescriptor[1].size  = DataLength+11;
	}

	// Setup IPX Header
	SendUtilIPXHeader.packetType = 0x5a;  // Experimental Packet Type
	memcpy (SendUtilIPXHeader.destination.network,ReceiveUtilIPXHeader.source.network,12);

	// Send IPX Packet
	do {
		IPXSendPacket(&SendUtilECB);
	} while(SendUtilECB.inUseFlag);
}

// void IPXUpgrade(int unused, void *unused1, void *unused2)
void IPXUpgrade(cyg_addrword_t data)
{
	uint16 RequestBlock = 0;
//Jesse	uint8  *CurTempAddress = NEW_CODE2_TEMP_ADDRESS;
//	uint8  *CurTempAddress = (char *)mallocw(512*1024);
	uint8  *CurTempAddress = UPGRADE_TEMP_ADDRESS;
	uint8  *TempAddress;  
	int 	upgrade_type = 0;

	int		retry, no_timeout = 1, UpgradeOK = 0;
	uint32	bsize=0;
	
	PSUpgradeMode = IPX_UPGRADE_MODE;

	RecvUtilUpgradeECB.IPXFrameType = ReceiveUtilECB.IPXFrameType;
	memcpy(RecvUtilUpgradeECB.immediateAddress,ReceiveUtilECB.immediateAddress, 6);
	memcpy(RecvUtilUpgradeIPXHeader.source.network,ReceiveUtilIPXHeader.source.network,12);
	
	TempAddress = CurTempAddress;
	
	while( TotalUpgradeBlock-- )
	{
		retry = 0;
		do {
			cyg_semaphore_init(&SIGNAL_IPX_UPGRADE_DATA, 0);
			
			_SendRequestBlock(RequestBlock);

			//// wait for recv data block ///////////////
			no_timeout = cyg_semaphore_timed_wait( &SIGNAL_IPX_UPGRADE_DATA, cyg_current_time() + (UPGRADE_TIME_OUT_SEC/MSPTICK));
/*Jesse			kalarm(UPGRADE_TIME_OUT_SEC);
			timeout = kwait(SIGNAL_IPX_UPGRADE_DATA);
			kalarm(0L);
*/			/////////////////////////////////////////////
			if(RecvUtilUpgradeData.BlockNumber != RequestBlock)
				no_timeout = 0;
		} while( !no_timeout && ++retry < UPGRADE_MAX_RETRY_TIMES);

		if( !no_timeout ) break;

		memcpy( CurTempAddress, RecvUtilUpgradeData.Data, UPGRADE_BLOCK_SIZE );

//Jesse		GreenLightToggle = 5;

/*Jesse		if( RequestBlock == 0 ) // verify firmware version
		{
			upgrade_type = CheckBIN( CurTempAddress );
			if (upgrade_type==1){
				if( CurTempAddress[BIN_ID_OFFSET] != 'X' ||
					CurTempAddress[BIN_ID_OFFSET+1] != 'L' ||
					CurTempAddress[BIN_ID_OFFSET+2] != 'Z' ||
					CurTempAddress[BIN_MODEL_OFFSET] != CURRENT_PS_MODEL )
					break;
			}else if (upgrade_type == 2){
				if( CurTempAddress[BIN_ID_OFFSET] != 'X' ||
					CurTempAddress[BIN_ID_OFFSET+1] != 'G' ||
					CurTempAddress[BIN_ID_OFFSET+2] != 'Z' ||
					CurTempAddress[BIN_MODEL_OFFSET] != CURRENT_PS_MODEL )
					break;
			}	
			else
				break;
		}	
*/		
		bsize += UPGRADE_BLOCK_SIZE;
		CurTempAddress += UPGRADE_BLOCK_SIZE;

		RequestBlock++;
	}

	if( no_timeout )
	{
		if( TotalUpgradeBlock == 0xFFFF )
			UpgradeOK = ApUpgradeFirmware( TempAddress , bsize );	

/*Jesse			if (upgrade_type == 1)
				error = vPCode1( NEW_CODE2_TEMP_ADDRESS );
			else if (upgrade_type == 2)
				error = vProgramCode2( NEW_CODE2_TEMP_ADDRESS );
			else
				error = 1;
*/
		else
			UpgradeOK = 0;

		if(UpgradeOK)
		{
			retry = 0;
			do {
				_SendRequestFinish();
				cyg_semaphore_init(&SIGNAL_IPX_UPGRADE_FINISH, 0);
				no_timeout = cyg_semaphore_timed_wait( &SIGNAL_IPX_UPGRADE_FINISH, cyg_current_time() + (UPGRADE_TIME_OUT_SEC/MSPTICK));
/*				kalarm(UPGRADE_TIME_OUT_SEC);
				timeout = kwait(SIGNAL_IPX_UPGRADE_FINISH);
				kalarm(0L);
*/			} while(no_timeout && ++retry < UPGRADE_MAX_RETRY_TIMES);

			ppause(20);

			Reset(); //615wu //eCos
		}
		else
		{
			retry = 0;
			do {
				_SendRequestBlockFail(RequestBlock);
				cyg_semaphore_init(&SIGNAL_IPX_UPGRADE_DATA, 0);
				no_timeout = cyg_semaphore_timed_wait( &SIGNAL_IPX_UPGRADE_DATA, cyg_current_time() + (UPGRADE_TIME_OUT_SEC/MSPTICK));
/*Jesse				kalarm(UPGRADE_TIME_OUT_SEC);
				timeout = kwait(SIGNAL_IPX_UPGRADE_DATA);
				kalarm(0L);
*/			} while(no_timeout && ++retry < UPGRADE_MAX_RETRY_TIMES);
		}

	}
	
	vReleaseCode2Memory();

	PSUpgradeMode = WAIT_UPGRADE_MODE;
	
	// for exit thread
	cyg_thread_exit();
}


//(8A).2 Send Start Printing data.
void NTSendAckStartPrintECB(BYTE Version, BYTE PrinterID, BYTE NTStatus)
{
#ifdef USE_PS_LIBS
	NTStartRespData	*RespData;

	if(Version == NT_VERSION_4) {
		RespData = (NTStartRespData *) SendUtilData.Nt2.Data;

		// Setup Utility Protocol
		SendUtilData.Nt2.Cmd[0] = 0x8A;
		SendUtilData.Nt2.Cmd[1] = 0x00;

		RespData->MaxDataLen = NT_MAX_RECV_LEN;

		if( NTPortInfo[PrinterID-1].MaxRecvPackets > NTMaxRecvPacket )
			NTPortInfo[PrinterID-1].MaxRecvPackets = NTMaxRecvPacket;

		RespData->MAXPacket = NTPortInfo[PrinterID-1].MaxRecvPackets;

	} else {
		RespData = (NTStartRespData *) SendUtilData.Nt1.Data;

		// Setup Utility Protocol
		SendUtilData.Nt1.Cmd[0] = 0x8A;
		SendUtilData.Nt1.Cmd[1] = 0x00;
	}

	RespData->PrinterID  = PrinterID;	 //(1 - N)
	RespData->Status	 = NTStatus;      //Status
	RespData->Mode       = NORMAL_MODE;  //mode ;

	cli();    //11/25/99
	SendUtilityPacket();
	sti();   //11/25/99
#endif /* USE_PS_LIBS */
}
//(8A).1 Start Printing.
void NTStartPrint(BYTE Version, BYTE PrinterID, BYTE MaxPackets)
{
#ifdef USE_PS_LIBS
	BYTE NTStatus, PortStatus = 0;
	BYTE PortNumber = PrinterID-1;

//ZOTIPS	armond_printf("Start IPX Printing \n");
	if(PrinterID > NUM_OF_PRN_PORT) {
		return;
	}

	cyg_scheduler_lock();	//615wu::No PSMain

	NTStatus = PrnGetPrinterStatus(PortNumber);

	if(Version == NT_VERSION_4) { //9/6/99
		PortStatus = ReadPortStatus(PortNumber);  //5/12/99
		if(PortStatus == PORT_PRINTING)	PortStatus = 0;	 //6/23/99
	}

//	if(PrnIsUnixHold(PortNumber)) NTStatus = PrnAbortUsed;        //615wu::No PSMain
	if(!PortStatus) { //6/23/99
		switch(NTStatus) {
			case PrnNoUsed:
				PrnSetNTInUse(PortNumber);
	
				//save destination node info
				NTPortInfo[PortNumber].BlockNumber = 0x00;
				NTPortInfo[PortNumber].IsFirst = 1;			  //10/27/99
				NTPortInfo[PortNumber].BlockData   = NULL;
				NTPortInfo[PortNumber].Version     = Version; //9/6/99
	
				if( Version == NT_VERSION_4 )
				{
					if( MaxPackets == 0 )
						MaxPackets = 5;
					if( ReceiveUtilECB.IPXFrameType == IPXBEUI )
						MaxPackets = 3;
				}
				else
					MaxPackets = 1;
	
				NTPortInfo[PortNumber].MaxRecvPackets = MaxPackets;
	
				NTPortInfo[PortNumber].PortName[NT_MAX_PORT_NAME_LENGTH] = '\0';
				//Netware address
				NTPortInfo[PortNumber].Destination = ReceiveUtilIPXHeader.source;
				//Immediate Address
				memcpy(NTPortInfo[PortNumber].ImmediateAddress, ReceiveUtilECB.immediateAddress, 6);
				//Last node address
	            NTPortInfo[PortNumber].HaveRecvPacket = 0;
				//Destination Frame Type
				NTPortInfo[PortNumber].IPXFrameType = ReceiveUtilECB.IPXFrameType;	//3/16/98
#ifdef NETBEUI
				if(ReceiveUtilECB.IPXFrameType == IPXBEUI)
					memcpy(NTPortInfo[PortNumber].SrcNETBEUIName, ReceiveUtilECB.IPXSrcNETBEUIName, 16);
#endif NETBEUI
	
				IsNTEndPrint[PortNumber] = 0;

#ifdef SUPPORT_JOB_LOG
//			memcpy(&job_log->LoginUser, ReceiveUtilData.Nt2.Data + 1, 32);	// HostName
			JL_PutList(2, PortNumber, ReceiveUtilData.Nt2.Data + 33, 32);
#endif //SUPPORT_JOB_LOG				
				
				break;
	  		case NTUsed:
	  			if(Version == NT_VERSION_4) {
					ReceiveUtilData.Nt2.Data[1+NT_MAX_PORT_NAME_LENGTH] = '\0';
				} else {
					ReceiveUtilData.Nt1.Data[1+NT_MAX_PORT_NAME_LENGTH] = '\0';
				}
				if(!NTPortInfo[PortNumber].HaveRecvPacket &&
				   !memcmp(ReceiveUtilIPXHeader.source.node,NTPortInfo[PortNumber].Destination.node,6) &&
				   (Version == NT_VERSION_4?
				     !strcmp(NTPortInfo[PortNumber].PortName,&ReceiveUtilData.Nt2.Data[1])
				    :!strcmp(NTPortInfo[PortNumber].PortName,&ReceiveUtilData.Nt1.Data[1])
				   ) && Version == NTPortInfo[PortNumber].Version
				){
				    NTStatus = PrnNoUsed;
				    //Retry Start Printer !
					//Send Ack Start Printer lost, so PrintMonitor retry it !
				}
				break;
			case PrnAbortUsed:
			default:
				break;
		}
	}//6/23/99

	cyg_scheduler_unlock();	//615wu::No PSMain

	if(NTStatus == PrnNoUsed) 
		NTStatus = 0x00;
	else                      
		NTStatus = 0x10;

	NTStatus |= PortStatus;	 //6/23/99

	NTSendAckStartPrintECB(Version,PrinterID,NTStatus);
	
//615wu::No PSMain	
	if(NTStatus == PrnNoUsed)
		cyg_semaphore_post( &NT_SIGNAL_PORT_1 );
#endif /* USE_PS_LIBS */
}

//(8B) Send Request Printer data. (6060)
uint32 ruery_ecb=0;
void NTQueryAckECB (BYTE PortNumber,BYTE BlockCount)
{
#ifdef USE_PS_LIBS
	NTSendPsECB.ESRAddress   = 0x00;
	NTSendPsECB.inUseFlag    = 0;
	NTSendPsECB.socketNumber = NTDataSocket;

	memcpy(NTSendPsECB.immediateAddress,NTPortInfo[PortNumber].ImmediateAddress, 6 );

	NTSendPsECB.IPXFrameType = NTPortInfo[PortNumber].IPXFrameType; //3/18/98
#ifdef NETBEUI
	if(NTPortInfo[PortNumber].IPXFrameType == IPXBEUI)
		memcpy(NTSendPsECB.IPXSrcNETBEUIName,NTPortInfo[PortNumber].SrcNETBEUIName, 16 );
#endif NETBEUI

	NTSendPsECB.fragmentCount = 2;
	NTSendPsECB.fragmentDescriptor[0].address = &NTSendPsIPXHeader;
	NTSendPsECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	NTSendPsECB.fragmentDescriptor[1].address = &NTSendQueryAckData;
	NTSendPsECB.fragmentDescriptor[1].size    = sizeof(NTQueryAckData);

	NTSendPsIPXHeader.packetType = 0x5a;

	NTSendPsIPXHeader.destination = NTPortInfo[PortNumber].Destination;

	if(NTPortInfo[PortNumber].Version == NT_VERSION_4) {

		memcpy(NTSendQueryAckData.Nt2.Mark,NT_CHECK_MARK2, NT_CHECK_MARK2_LEN );
		NTSendQueryAckData.Nt2.Type = (BYTE)NTDataSocket;  //NT30
		NTSendQueryAckData.Nt2.Cmd[0]  = 0x8B;
		NTSendQueryAckData.Nt2.Cmd[1]  = 0x00;

		NTSendQueryAckData.Nt2.PrinterID   = PortNumber+1;
		NTSendQueryAckData.Nt2.BlockNumber = NTPortInfo[PortNumber].BlockNumber;
		NTSendQueryAckData.Nt2.Mode        = NORMAL_MODE;	//0:Normal 1:Burst
		NTSendQueryAckData.Nt2.BlockCount  = BlockCount;   //how many block want to receive !

	} else {
		if(NTPortInfo[PortNumber].IPXFrameType == IPXBEUI)
			NSET16(&NTSendQueryAckData.Nt1.Mark, NTDataSocket);
		else
			memcpy(NTSendQueryAckData.Nt1.Mark,NT_CHECK_MARK1,NT_CHECK_MARK1_LEN );

		NTSendQueryAckData.Nt1.Cmd[0]  = 0x8B;
		NTSendQueryAckData.Nt1.Cmd[1]  = 0x00;

		NTSendQueryAckData.Nt1.PrinterID   = PortNumber+1;
		NTSendQueryAckData.Nt1.BlockNumber = NTPortInfo[PortNumber].BlockNumber;
		NTSendQueryAckData.Nt1.Mode        = NORMAL_MODE;	//0:Normal 1:Burst
		NTSendQueryAckData.Nt1.BlockCount  = BlockCount;   //how many block want to receive !
	}

	do {
		++ruery_ecb;
		IPXSendPacket(&NTSendPsECB);
	} while(NTSendPsECB.inUseFlag);
	ruery_ecb = 0;
#endif /* USE_PS_LIBS */
}

//(8C).2 Send End Printing data.
void NTSendAckEndPrintECB (BYTE Version, BYTE PrinterID)
{
	if(Version == NT_VERSION_4) {
		//new version
		SendUtilData.Nt2.Cmd[0] = 0x8C;
		SendUtilData.Nt2.Cmd[1] = 0x00;
		SendUtilData.Nt2.Data[0] = PrinterID;	//Printer ID
	} else {
		//old version
		SendUtilData.Nt1.Cmd[0] = 0x8C;
		SendUtilData.Nt1.Cmd[1] = 0x00;
		SendUtilData.Nt1.Data[0] = PrinterID;	//Printer ID
	}

	SendUtilityPacket();
}
//(8C).1 End Printing.
void NTEndPrint (BYTE Version, BYTE PrinterID)
{
#ifdef USE_PS_LIBS
//ZOTIPS	armond_printf("IPX Printing end\n");
	if(PrinterID > NUM_OF_PRN_PORT) {
		return;
	}

	NTPortInfo[PrinterID-1].HaveRecvPacket = 1;

	if( (Version == NT_VERSION_4?
	       strcmp(&ReceiveUtilData.Nt2.Data[1], NT_END_PRINT)==0
	     : strcmp(&ReceiveUtilData.Nt1.Data[1], NT_END_PRINT)==0
	    ) && NTPortInfo[PrinterID-1].Version == Version
	) {
		NTSendAckEndPrintECB(Version, PrinterID);
		IsNTEndPrint[PrinterID-1] = 1;
	}

#ifdef SUPPORT_JOB_LOG
	JL_EndList(PrinterID, 0);		// Completed. George Add January 26, 2007
#endif //SUPPORT_JOB_LOG
#endif /* USE_PS_LIBS */	
}

//(80) Send EEPROM data.
void UtilSendEEPROM(void)
{

	SendUtilData.Nt2.Cmd[0]=0x80;
	SendUtilData.Nt2.Cmd[1]=0x01; //0x00: 512K, 0x01: 1024K
	DataCopy(SendUtilData.Nt2.Data,&EEPROM_Data,sizeof(EEPROM_Data));

	SendUtilityPacket();
}

//(81) Write config data into EEPROM.
void UtilConfigBox(void)
{
	uint8 Result = 0x00;

	UtilGetListViewData((LIST_VIEW *)ReceiveUtilData.Nt2.Data);

	if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xff;

	// Setup Utility Protocol
	SendUtilData.Nt2.Cmd[0] = 0x81;
	SendUtilData.Nt2.Cmd[1] = Result;

	SendUtilityPacket();
}

//(84) Send Box data for utility view & config it.
void UtilViewBox(void)
{
	SendUtilData.Nt2.Cmd[0]=0x84;
	SendUtilData.Nt2.Cmd[1]=0x00;  //OK
	
	memset(SendUtilData.Nt2.Data, 0, 1024);
	
	UtilSetListViewData((LIST_VIEW *)SendUtilData.Nt2.Data);

	ppause(urandom(10)+1);	//delay 1 - 10 ms 9/9/99

	SendUtilityPacket();
}

//(85) Switch to Code1 for Download Flash
void UtilStartUpgrade(void)
{

	if( PSUpgradeMode == WAIT_UPGRADE_MODE )
	{
		if( !vAllocCode2Memory() )
		{
			//Reject D/L Request when Print Busy or in Upgrade Mode
			SendUtilData.Nt2.Cmd[0] = 0x85;
			SendUtilData.Nt2.Cmd[1] = 0xFF;
			SendUtilityPacket();
		}
		else
		{
			SendUtilData.Nt2.Cmd[0] = 0x85;
			SendUtilData.Nt2.Cmd[1] = 0x00;
			SendUtilityPacket();

			TotalUpgradeBlock = NGET16(ReceiveUtilData.Nt2.Data) + 1;
			
			if( IPXUpgrade_TaskHdl != 0 )
				cyg_thread_delete(IPXUpgrade_TaskHdl);
			
//			newproc("IPX-UPGRADE",1024,IPXUpgrade,0,NULL,NULL,0);
			//Create IPXUpgrade Thread
			cyg_thread_create(IPXUpgrade_TASK_PRI,
								IPXUpgrade,
								0,
								"IPX-UPGRADE",
								(void *) (IPXUpgrade_Stack),
								IPXUpgrade_TASK_STACK_SIZE,
								&IPXUpgrade_TaskHdl,
								&IPXUpgrade_Task);
			
			//Start IPXUpgrade Thread
			cyg_thread_resume(IPXUpgrade_TaskHdl);
			
			
		}	
	}
}


//(88) Send Print HW Status.
void UtilPrintStatus(void)
{
#ifdef USE_PS_LIBS
	uint8 PortNumber;

	SendUtilData.Nt2.Cmd[0] = 0x88;
	SendUtilData.Nt2.Cmd[1] = 0x00;

	//Printer Status
	SendUtilData.Nt2.Data[0] = ReadPrintStatus();  //HW Status

	UtilRemoveBusyStatus(&SendUtilData.Nt2.Data[0]);

	//4/26/2000 changed
#ifdef NOVELL_PS 
	SendUtilData.Nt2.Data[1] = NovellConnectFlag & (NDSConnectFlag << 1);
#endif

	//Using Status
	for(PortNumber = 0; PortNumber < NUM_OF_PRN_PORT; PortNumber++) {
		SendUtilData.Nt2.Data[PortNumber+2] = PrnGetPrinterStatus(PortNumber) & 0x0F;
		if(SendUtilData.Nt2.Data[PortNumber+2]) SendUtilData.Nt2.Data[PortNumber+2]--;
	}

	SendUtilityPacket();
#endif /* USE_PS_LIBS */
}

//(8D) Receive EEPROM Data.
void UtilRecvEEPROM(void)
{
	uint8 Result = 0xFF;

	// Charles 2001/08/27
	if( !strcmp(((EEPROM *)ReceiveUtilData.Nt4.Data)->ZOT_Mark,"ZOT") && 
		((EEPROM *)ReceiveUtilData.Nt4.Data)->Model == CURRENT_PS_MODEL )
	{
		if( ReceiveUtilData.Nt4.Cmd[1] == 0x01 )
	    {

			if( !memcmp(((EEPROM *)ReceiveUtilData.Nt4.Data)->EthernetID,MyPhysNodeAddress,6) )
			{
				Result = 0x00;
				DataCopy((uint8 *)&EEPROM_Data,ReceiveUtilData.Nt4.Data, sizeof(EEPROM_Data));

				if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xFF;
			}
		}
		else if( ReceiveUtilData.Nt4.Cmd[1] == 0x02 )
		{
			Result = 0x00;
			DataCopy((uint8 *)&EEPROM_Data,ReceiveUtilData.Nt4.Data, sizeof(EEPROM_Data));

			memcpy( &DEFAULT_Data, &EEPROM_Data, sizeof(EEPROM) );
			memcpy( &QC0_Defualt_EEPROM, &EEPROM_Data, sizeof(EEPROM) );
			
			if(WriteToQC0_Default(&QC0_Defualt_EEPROM) != 0)  
				Result = 0xFF;
			else if(WriteToDefault(&DEFAULT_Data) != 0)  
				Result = 0xFF;
			else if(WriteToEEPROM(&EEPROM_Data) != 0)  
				Result = 0xFF;	
		}
	}

	SendUtilData.Nt4.Cmd[0] = 0x8D;
	SendUtilData.Nt4.Cmd[1] = Result;

	SendUtilityPacket();
}


//(8E) QC -- Output pattern to print port
void UtilPrintPattern(void)
{
#ifdef USE_PS_LIBS
	uint8 Result = PrnPrintPattern(ReceiveUtilData.Nt2.Cmd[1],ReceiveUtilData.Nt2.Data, 256 );

	// Setup Utility Protocol
	SendUtilData.Nt2.Cmd[0] = 0x8E;
	SendUtilData.Nt2.Cmd[1] = Result;

	SendUtilityPacket();
#endif /* USE_PS_LIBS */
}

//(90) Send Device Info. 4/18/2000 added
void UtilSendDeviceInfo(uint8 PrinterID)
{
#ifdef USE_PS_LIBS
	uint16 len;
	uint8  *p;

	SendUtilData.Nt2.Cmd[0] = 0x90;
	SendUtilData.Nt2.Cmd[1] = 0x00;

	//Printer Status
	SendUtilData.Nt2.Data[0] = PrinterID;

	if(PrinterID > NUM_OF_PRN_PORT) {
		memset(&SendUtilData.Nt2.Data[1],'\0',10);
	} else {
		memset(&SendUtilData.Nt2.Data[1],'\0',10);

		p = &SendUtilData.Nt2.Data[1];

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

	SendUtilityPacket();
#endif /* USE_PS_LIBS */
}

//(91) Read Printer Data
void UtilReadPrinterData( BYTE PrinterID, WORD DataSize )
{
#ifdef USE_PS_LIBS
	uint8  PrnPort = PrinterID - 1;
	uint16 ReadSize = 0;

	SendUtilData.Nt2.Cmd[0] = 0x91;

	if( ( G_PortReady & (0x01 << PrnPort) )
		&& PrnGetAvailQueueNO(PrnPort) == PRNQUEUELEN
		&& PortReady(PrnPort) )
	{
		SendUtilData.Nt2.Cmd[1] = 0x00;

		//PrnEndNegotiate(PrnPort); // Charles 2001/10/03
		//ReadSize = PrnReadBack(PrnPort, &SendUtilData.Nt2.Data[3], DataSize);
		//PrnStartNegotiate(PrnPort); // Charles 2001/10/03
	}
	else
	{
		SendUtilData.Nt2.Cmd[1] = 0x01;
	}		

	SendUtilData.Nt2.Data[0] = PrinterID;

	NSET16( &SendUtilData.Nt2.Data[1], ReadSize );

	SendUtilityPacket();
#endif /* USE_PS_LIBS */
}

//(92) Write Printer Data
void UtilWritePrinterData( BYTE PrinterID, WORD DataSize )
{
#ifdef USE_PS_LIBS
	uint16 WriteSize = 0;
	uint8  result;

	result = PrnPrintPattern( PrinterID - 1, &ReceiveUtilData.Nt2.Data[3], DataSize );

	if( result == 0 ) WriteSize = DataSize;

	SendUtilData.Nt2.Cmd[0] = 0x92;
	SendUtilData.Nt2.Cmd[1] = result;
	SendUtilData.Nt2.Data[0] = PrinterID;
	NSET16( &SendUtilData.Nt2.Data[1], WriteSize );

	SendUtilityPacket();
#endif /* USE_PS_LIBS */
}

//(98) Get Firmwave Data
void UtilGetFirmwareData()
{
	SendUtilData.Nt2.Cmd[0] = 0x98;
	SendUtilData.Nt2.Cmd[1] = 0x00;

	memset(SendUtilData.Nt2.Data, 0, sizeof(VIEW_DATA));
	UtilGetFWData((VIEW_DATA *)SendUtilData.Nt2.Data);

	SendUtilityPacket();
}

//(99) Load Default Data
void UtilLoadDefault()
{
	int result =0;
	SendUtilData.Nt2.Cmd[0] = 0x99;
	SendUtilData.Nt2.Cmd[1] = 0x00;

	result = ResetToDefalutFlash(0,1,0);
	
	memcpy(&SendUtilData.Nt2.Data[0], MyPhysNodeAddress, 6);
	
	SendUtilData.Nt2.Data[6] = result;
	

	SendUtilityPacket();
}

//(9B)Print Loopback Test
void UtilPrintLoopback()
{
	SendUtilData.Nt2.Cmd[0] = 0x9B;
	SendUtilData.Nt2.Cmd[1] = 0x00;
/*Jesse
	SendUtilData.Nt2.Data[0] = PrnLoopbackTest(0);

#if (NUM_OF_1284_PORT > 1)
	SendUtilData.Nt2.Data[1] = PrnLoopbackTest(1);
#else
	SendUtilData.Nt2.Data[1] = 0;
#endif

#if (NUM_OF_1284_PORT > 2)
	SendUtilData.Nt2.Data[2] = PrnLoopbackTest(2);
#else
	SendUtilData.Nt2.Data[2] = 0;
#endif
*/
	SendUtilityPacket();
}

//(9C)Wireless Loopback1 Test
void UtilWirelessLoopback1()
{
	uint8 result;
	
	SendUtilData.Nt2.Cmd[0] = 0x9C;
	SendUtilData.Nt2.Cmd[1] = 0x00;
   	SendUtilData.Nt2.Data[0] = 0xFF;

	ppause(1000);//After lpbk, initial mac again and must wait network connect ... Ron 3/26/2003
	if (!result)
		SendUtilData.Nt2.Data[0] = 0;
	else if (result == 2)
		SendUtilData.Nt2.Data[0] = 2;	//2:635u2pw Write Image to flash error
	SendUtilData.Nt2.Data[1] = 0xAA;
	SendUtilData.Nt2.Data[2] = 38;
//temp	SendUtilData.Nt2.Data[3] = mvWDomain;
//Jesse	SendUtilData.Nt2.Data[4] = EEPROM_Data.WLVersion;
	SendUtilData.Nt2.Data[4] = 0;
	SendUtilData.Nt2.Data[5] = EEPROM_Data.WLZone;
	SendUtilData.Nt2.Data[6] = EEPROM_Data.WLAPMode;
	SendUtilData.Nt2.Data[7] = EEPROM_Data.WLMode;
	memcpy( &SendUtilData.Nt2.Data[8], EEPROM_Data.WLESSID, 32 );
	SendUtilData.Nt2.Data[40] = EEPROM_Data.WLChannel;


	SendUtilityPacket();
}

//(9D)Wireless Loopback2 Test
void UtilWirelessLoopback2()
{

	// Charles 2002/1/4
	// T : product
    // E : end-point
	// +-----------------------------------------------------------------+
	// |  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F  |
	// +------------------------+-----------------------+----------------+
	// |  Sender MAC Backup (T) |     Dest MAC (T,E)    |  Dest Addr (T) |
	// +------------------------+-------+---------------+----------------+
	// |         Dest Addr (T)          |
	// +--------------------------------+

	SendUtilData.Nt2.Cmd[0] = 0x9D;
	SendUtilData.Nt2.Cmd[1] = 0x00;

	if( memcmp( &ReceiveUtilData.Nt2.Data[6], MyPhysNodeAddress, 6 ) == 0 )
	{
		memcpy( &SendUtilData.Nt2.Data[0], &ReceiveUtilData.Nt2.Data[6], 6 );
		memcpy( &SendUtilData.Nt2.Data[6], &ReceiveUtilData.Nt2.Data[0], 6 );
		memcpy( &SendUtilData.Nt2.Data[12], &ReceiveUtilData.Nt2.Data[12], 12 );
	}
	else
	{
		// backup sender, include network number, node id and socket
		memcpy( &SendUtilData.Nt2.Data[0], ReceiveUtilECB.immediateAddress, 6 );
		memcpy( &SendUtilData.Nt2.Data[12], ReceiveUtilIPXHeader.source.network, 12 );

		memcpy( ReceiveUtilECB.immediateAddress, &ReceiveUtilData.Nt2.Data[6], 6 );
		memcpy( ReceiveUtilIPXHeader.source.network, &ReceiveUtilData.Nt2.Data[12], 12 );

		memcpy( &SendUtilData.Nt2.Data[6], &ReceiveUtilData.Nt2.Data[6], 6 );
	}

	SendUtilityPacket();

}

//(9E)USB Loopback Test
void UtilUSBLoopback()
{

	int				i;

	SendUtilData.Nt2.Cmd[0] = 0x9E;
	SendUtilData.Nt2.Cmd[1] = 0x00;

 	for( i = 0; i < 4; i++ )
		SendUtilData.Nt2.Data[i] = usb_hub_link_devices( i );

	SendUtilityPacket();

}

//(A0) Send EEPROM data to the computer of PSQC2
void UtilSendEEPROM_Adv(void)
{
	uint8 total_block;
	uint16 Block_Size, Block_Offset = 0;
	uint16 packsize = 1194 - 4 - 6;	// 4 bytes: Block number, Total block number, Block size x 2
									// 6 bytes: Source Node ID
	uint16 datasize = sizeof(EEPROM_Data);
	uint8 Block_No = ReceiveUtilData.Nt2.Data[0];
	
	if( Block_No != 1 ) return;

	total_block = datasize / packsize;
	if ( ( datasize % packsize )!=0 )
		total_block++;
	
	ppause(urandom(10)+1);	//delay 1 - 10 ms 9/9/99

	while (Block_No <= total_block)
	{
		if (Block_No == total_block)
			Block_Size = (datasize % packsize == 0 )? packsize:(datasize % packsize);
		else
			Block_Size = packsize;
			
		Block_Offset = (Block_No - 1) * packsize;
		
		SendUtilData.Nt2.Cmd[0]=0xA0;
		SendUtilData.Nt2.Cmd[1]=0x00;	// OK
		SendUtilData.Nt2.Data[0]= Block_No;		// Block number
		SendUtilData.Nt2.Data[1]= total_block;	// Total block number
		NSET16( &SendUtilData.Nt2.Data[2], Block_Size );	// Block size x 2
		memcpy(SendUtilData.Nt2.Data + 4, MyPhysNodeAddress, 6);		// Source Node ID
		memcpy(SendUtilData.Nt2.Data + 10, (char *)&EEPROM_Data + Block_Offset, Block_Size);	// Data

		SendUtilityPacket();
		
		ppause(5);
		Block_No++;
	}
}

//(A1) Write config data into EEPROM.
void UtilConfigBox_Adv(void)
{
	uint8 Result = 0x00;
	uint8 Block_No = ReceiveUtilData.Nt2.Data[0];
	uint8 total_block = ReceiveUtilData.Nt2.Data[1];
	uint16 Block_Size = ReceiveUtilData.Nt2.Data[2] + 256 * ReceiveUtilData.Nt2.Data[3];
	
	// Check this data for myself or not (broadcast) !
	if( memcmp(MyPhysNodeAddress, ReceiveUtilData.Nt2.Data + 4, 6) != 0 ) 
		return;
	
	if ( (Block_No > Previous_BNo_IPX+1) || (Block_No < Previous_BNo_IPX ) ) 
		return;

	if ( Block_No ==1 ) {
		t_tran_byte_ipx = 0;	// Jesse added this in 716U2W at build0005 on December 10, 2010.
		memset(rec_buffer_ipx,0,sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1);
	}
	Previous_BNo_IPX = Block_No;
	
	memcpy( recbuffer_ipx + t_tran_byte_ipx, ReceiveUtilData.Nt2.Data + 4 + 6, Block_Size);
	t_tran_byte_ipx += Block_Size;
	memset( ReceiveUtilData.Nt2.Data, 0, sizeof(ReceiveUtilData.Nt2.Data) );
	
	if (Block_No == total_block){
		t_tran_byte_ipx = 0;
		UtilGetListViewData( (LIST_VIEW *)recbuffer_ipx );
		recbuffer_ipx += ( sizeof(LIST_VIEW) - 1 );
		UtilGetListViewData_Adv( (LIST_VIEW_EXT *)recbuffer_ipx );
		recbuffer_ipx = rec_buffer_ipx;
		Previous_BNo_IPX = 0;

#ifndef _PC
		if(WriteToEEPROM(&EEPROM_Data) != 0)
			Result = 0xff;
#endif

		// Setup Utility Protocol
		SendUtilData.Nt2.Cmd[0] = 0xA1;
		SendUtilData.Nt2.Cmd[1] = Result;
		SendUtilData.Nt2.Data[0] = Block_No;
		SendUtilData.Nt2.Data[1] = total_block;
		
		SendUtilityPacket();
	}
}

//(A4) Send Box data for utility view & config it.
void UtilViewBox_Adv(void)
{
	BYTE *p,*init_p;
	uint8 total_block;
	uint16 Block_Size, Block_Offset = 0;
	uint16 packsize = 1024 - 4 - 6;	// 4 bytes: Block number, Total block number, Block size x 2
									// 6 bytes: Source Node ID
	uint16 datasize = sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1;
	uint8 Block_No = ReceiveUtilData.Nt2.Data[0];
	
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
		
		SendUtilData.Nt2.Cmd[0]=0xA4;
		SendUtilData.Nt2.Cmd[1]=0x00;	// OK
		SendUtilData.Nt2.Data[0]= Block_No;		// Block number
		SendUtilData.Nt2.Data[1]= total_block;	// Total block number
		NSET16( &SendUtilData.Nt2.Data[2], Block_Size );	// Block size x 2
		memcpy(SendUtilData.Nt2.Data + 4, init_p, 6);		// Source Node ID
		memcpy(SendUtilData.Nt2.Data + 10, init_p + Block_Offset, Block_Size);	// Data

		SendUtilityPacket();
		
		ppause(5);
		Block_No++;
	}
	
	free(init_p);
}

extern int usb_usb_send;
extern struct netif *ULanface;
//(AA) Send Start USB ETHERNET test reply(IPX only)
void NTStartUSBTest(BYTE PrinterID)
{
	int				i;
	NTStartRespData *RespData;
	
	RespData = (NTStartRespData *) SendUtilData.Nt2.Data;
	SendUtilData.Nt2.Cmd[0] = 0xAA;
	SendUtilData.Nt2.Cmd[1] = 0x00;
	
	memset(RespData, 0 , sizeof(NTStartRespData));
	
	RespData->PrinterID = PrinterID;
	if( ULanface != NULL )
		RespData->Status = 0x0;
	else
		RespData->Status = 0x1;
	
	RespData->MaxDataLen = NT_MAX_RECV_LEN;

	SendUtilityPacket();
	
	if( ULanface != NULL )
		usb_usb_send = 1;	
}

//(AB) Send USB ETHERNET test data reply(IPX only)
void NTUSBTestDataReply(BYTE PrinterID)
{
	int				i;
	WORD 			DataLength;
	
	NTReqData *RecvReqData,*RespData;
	
	RecvReqData = (NTReqData *) ReceiveUtilData.Nt2.Cmd;
	RespData = (NTStartRespData *) SendUtilData.Nt2.Cmd;
	SendUtilData.Nt2.Cmd[0] = 0xAB;
	SendUtilData.Nt2.Cmd[1] = 0x00;
	
	RespData->PrinterID = RecvReqData->PrinterID;
	RespData->BlockNumber = RecvReqData->BlockNumber;
	RespData->Mode = RecvReqData->Mode;
	RespData->DataLength = RecvReqData->DataLength;
	DataLength = WordSwap(RecvReqData->DataLength);
	memcpy(RespData->Data, RecvReqData->Data, DataLength);
		
	SendUtilityPacket();
}

//(AC) Send End USB ETHERNET test reply (IPX only)
void NTEndUSBTest(BYTE PrinterID)
{
	int				i;
	NTStartRespData *RespData;
	
	RespData = (NTStartRespData *) SendUtilData.Nt2.Data;
	SendUtilData.Nt2.Cmd[0] = 0xAC;
	SendUtilData.Nt2.Cmd[1] = 0x00;
	
	memset(RespData, 0 , sizeof(NTStartRespData));	
	RespData->PrinterID = PrinterID;
	
	SendUtilityPacket();
	
	//usb_usb_send = 0;
}

//(AD) Receive EEPROM Data > 1408.
void UtilRecvEEPROM_Adv(void)
{
	uint8 Result = 0xFF;
	uint8 Block_No = ReceiveUtilData.Nt4.Data[0];
	uint8 total_block = ReceiveUtilData.Nt4.Data[1];
	uint16 Block_Size = ReceiveUtilData.Nt4.Data[2] + 256 * ReceiveUtilData.Nt4.Data[3];
	
	// Check this data for myself or not (broadcast) !
	if(memcmp(MyPhysNodeAddress,ReceiveUtilData.Nt4.Data + 4, 6) != 0) return;
	
	if((Block_No > Previous_BNo+1) || (Block_No < Previous_BNo )) return;
	
	Previous_BNo = Block_No;
	
	memcpy( recbuffer_ipx + t_tran_byte_ipx, ReceiveUtilData.Nt4.Data + 4 + 6, Block_Size);
	t_tran_byte_ipx += Block_Size;
	memset( ReceiveUtilData.Nt4.Data, 0, sizeof(ReceiveUtilData.Nt4.Data) );
	
	if (Block_No == total_block)
	{
		if( !strcmp(((EEPROM *)recbuffer_ipx)->ZOT_Mark,"ZOT") &&
			(((EEPROM *)recbuffer_ipx)->Model == CURRENT_PS_MODEL) )
		{
			Result = 0x00;
			DataCopy((uint8 *)&EEPROM_Data, recbuffer_ipx, sizeof(EEPROM_Data));

			memcpy( &DEFAULT_Data, &EEPROM_Data, sizeof(EEPROM) );
			memcpy( &QC0_Defualt_EEPROM, &EEPROM_Data, sizeof(EEPROM) );
			
			if(WriteToQC0_Default(&QC0_Defualt_EEPROM) != 0)  
				Result = 0xFF;
			else if(WriteToDefault(&DEFAULT_Data) != 0)  
				Result = 0xFF;
			else if(WriteToEEPROM(&EEPROM_Data) != 0)  
				Result = 0xFF;			

		}
		
		t_tran_byte_ipx = 0;
		recbuffer_ipx = rec_buffer_ipx;
		Previous_BNo = 0;

		SendUtilData.Nt4.Cmd[0] = 0xAD;
		SendUtilData.Nt4.Cmd[1] = Result;
		SendUtilData.Nt4.Data[0] = Block_No;
		SendUtilData.Nt4.Data[1] = total_block;
	
		SendUtilityPacket();
	}
}

void UtilityESR(void)
{
	if(memcmp(ReceiveUtilData.Nt2.Mark, NT_CHECK_MARK2, NT_CHECK_MARK2_LEN ) == 0) {
		//NT_VERSION_4
		memcpy(SendUtilData.Nt2.Mark,NT_CHECK_MARK2,NT_CHECK_MARK2_LEN);
		SendUtilData.Nt2.Type = ReceiveUtilData.Nt2.Type;  //4/20/2000 changed

		switch(ReceiveUtilData.Nt2.Cmd[0]) {
			case CMD00: // Read EEPROM
//os				UtilSendEEPROM();
				break;
			case CMD01: // Config Box
//#ifndef PACK_DATA_EXT
#ifndef O_AXIS
				UtilConfigBox();
#endif
//#endif
				break;
			case CMD03: // Reset Box
				Reset(); //615wu //eCos
				break;
			case CMD04: // Send List View Box
				UtilViewBox();
				break;
			case CMD05:	// Start Upgrade
				UtilStartUpgrade();
				break;
			case CMD08: // Request Printer's Status
				UtilPrintStatus();
				break;
			case CMD0A:	// Start Print
				NTStartPrint(NT_VERSION_4, ReceiveUtilData.Nt2.Data[0], ReceiveUtilData.Nt2.Cmd[1]);
				break;
			case CMD0C: // End PRINT
				NTEndPrint(NT_VERSION_4,ReceiveUtilData.Nt2.Data[0]);
				break;
			case CMD0D: //Write EEPROM
				UtilRecvEEPROM();
				break;
			case CMD0E: //QC-- Output Pattern
				UtilPrintPattern();
				break;
			case CMD10: //Send Device Info 4/18/2000
				UtilSendDeviceInfo(ReceiveUtilData.Nt2.Data[0]);
				break;
			case CMD11:
				UtilReadPrinterData( ReceiveUtilData.Nt2.Data[0], NGET16( &ReceiveUtilData.Nt2.Data[1] ) );
				break;
			case CMD12:
				UtilWritePrinterData( ReceiveUtilData.Nt2.Data[0], NGET16( &ReceiveUtilData.Nt2.Data[1] ) );
				break;
			case CMD18:		//QC-- get F/W inf data
				UtilGetFirmwareData();
				break;	
		    case CMD19:
				UtilLoadDefault();
				break;
			case CMD1A:
                #ifdef WIRELESS_CARD
				WirelessLightToggle = 5;
                #endif
				StatusLightToggle = 5;				
//ZOT716u2				LanLightToggle = 5;
#ifdef  USB_LED
				USBTestLightToggle = 5;	//ZOT716u2
#endif
				break;
			case CMD1B:
//os				UtilPrintLoopback();
				break;
			case CMD1C:
                #ifdef WIRELESS_CARD
				UtilWirelessLoopback1();
                #endif
				break;
			case CMD1D:
			case CMD9D:
                #ifdef WIRELESS_CARD
				UtilWirelessLoopback2();
                #endif
				break;
			case CMD1E:
				UtilUSBLoopback();
				break;
			case CMD20:		// Send EEPROM data to the computer of PSQC2.
				UtilSendEEPROM_Adv();
				break;
#ifdef PACK_DATA_EXT
			case CMD21: // Config Box
#ifndef SECURITY_CONFIG
				UtilConfigBox_Adv();
#endif
				break;
			case CMD24: // Send List View Box
				UtilViewBox_Adv();
				break;
#endif //PACK_DATA_EXT
			case CMD2A:		// Start USB ETHERNET test (IPX only)
				NTStartUSBTest(ReceiveUtilData.Nt2.Data[0]);
				break;
			case CMD2B:		// Request USB ETHERNET test data (IPX only)
				NTUSBTestDataReply(ReceiveUtilData.Nt2.Data[0]);
				break;
			case CMD2C:		// End USB ETHERNET test (IPX only)
				NTEndUSBTest(ReceiveUtilData.Nt2.Data[0]);
				break;
			case CMD2D:	// Write EEPROM > 1408
				UtilRecvEEPROM_Adv();
				break;	

			default: // If the CMD is UnKnown , Give Up the Packet!
				break;
			} //switch (CMD) {
	}//if(strncmp(ReceiveUtil........

	ListenUtilPack();                    // ReListen Utility ESR Packet !
}
void NTReceiveData (void)
{
	NTReqData *RecvReqData;
	BYTE PortNumber, Version;
	struct ipx_mbuf *mbuf;		//queue ipx
	NTRequestData	*NTReceiveRequestData = NTReceivePsECB.fragmentDescriptor[1].address;	//queue ipx
	
	if(NTReceiveRequestData == NULL)					//queue ipx
		goto RecvDataExit;
	
	mbuf = malloc(sizeof(struct ipx_mbuf));				//queue ipx
	if (mbuf  == NULL){									//queue ipx
		free(NTReceiveRequestData);
		goto RecvDataExit;
	}
	memset(mbuf, 0x00, sizeof(struct ipx_mbuf));		//queue ipx
	
	if(memcmp(NT_CHECK_MARK2,NTReceiveRequestData->Nt2.Mark,NT_CHECK_MARK2_LEN) == 0) {	//queue ipx
		RecvReqData = (NTReqData *) &(NTReceiveRequestData->Nt2.Cmd);	//queue ipx
		Version = NT_VERSION_4;
	} else {
		if(memcmp(NT_CHECK_MARK1,NTReceiveRequestData->Nt1.Mark,NT_CHECK_MARK1_LEN) == 0) {	//queue ipx
			RecvReqData = (NTReqData *) &(NTReceiveRequestData->Nt1.Cmd);	//queue ipx
			Version = NT_VERSION_2;	
		} else {
			IPXPktErr++; //10/20/99
			free(mbuf);
			free(NTReceiveRequestData);		//drop packet
			goto RecvDataExit;
		}
	}

	PortNumber = RecvReqData->PrinterID-1;
	
#ifdef USE_PS_LIBS
	if(PortNumber < NUM_OF_PRN_PORT                                       &&
	   NTReceivePsECB.IPXFrameType == NTPortInfo[PortNumber].IPXFrameType &&
	   Version == NTPortInfo[PortNumber].Version                          &&
	   RecvReqData->Cmd[0]  == 0x0B)
	{
		mbuf->data = NTReceiveRequestData;		//queue ipx
		ipx_enqueue(&(IPXUSOCK.rcvq),&mbuf);		//queue ipx
		cyg_semaphore_post(&(IPXUSOCK.sem_f));
	}
	else
#endif /* USE_PS_LIBS */
	{	
		free(mbuf);
	 	free(NTReceiveRequestData);				//drop packet
	}
RecvDataExit:
	NTWaitforDataECB();
	return;

}
void ListenUtilPack(void)
{
	// Setup Receive Utility-ECB Block
	ReceiveUtilECB.ESRAddress = UtilityESR;   // Should be fill Utility ESR Address
	ReceiveUtilECB.inUseFlag  = 0;
	ReceiveUtilECB.socketNumber  = UtilitySocket;
	ReceiveUtilECB.fragmentCount = 2;
	ReceiveUtilECB.fragmentDescriptor[0].address = &ReceiveUtilIPXHeader;
	ReceiveUtilECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	ReceiveUtilECB.fragmentDescriptor[1].address = &ReceiveUtilData;
	ReceiveUtilECB.fragmentDescriptor[1].size    = sizeof(Utility_Rec);

	IPXListenForPacket(&ReceiveUtilECB);
}

void NTWaitforDataECB (void)
{	
	NTRequestData  		*NTReceiveRequestData = 0;	//queue ipx	
	
	NTReceiveRequestData = malloc(sizeof(NTRequestData));					//queue ipx
	if (NTReceiveRequestData != NULL)
	    memset(NTReceiveRequestData, 0x00, sizeof(NTRequestData));			//queue ipx
	
	// Setup Receive DATA-ECB Block
	NTReceivePsECB.ESRAddress    = NTReceiveData;
	NTReceivePsECB.inUseFlag     = 0;
	NTReceivePsECB.socketNumber  = NTDataSocket;
	NTReceivePsECB.fragmentCount = 2;

	NTReceivePsECB.fragmentDescriptor[0].address = &NTReceivePsIPXHeader;	//queue ipx
	NTReceivePsECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	if (NTReceiveRequestData != NULL){
	NTReceivePsECB.fragmentDescriptor[1].address = NTReceiveRequestData;	//queue ipx
	NTReceivePsECB.fragmentDescriptor[1].size    = sizeof(NTRequestData);
	}else{
		NTReceivePsECB.fragmentDescriptor[1].address = NULL;				//queue ipx
		NTReceivePsECB.fragmentDescriptor[1].size    = 0;	
	}

	IPXListenForPacket(&NTReceivePsECB);
//ZOTIPS	armond_printf("Start Listen '0B' IPX Packet.\n");
}

void IPXUtilityInit(void)
{
	uint16 SocketData= UtilitySocket;;

	//Open Utility Socket
	IPXOpenSocket((uint8 *)&SocketData,LONG_LIVED);
	ListenUtilPack();    // Listen Utility Packet
}

void IPXReceiveInit (void)
{
	uint16 SocketData = NTDataSocket;
	
	memset(&IPXUSOCK, 0x00, sizeof(struct ipx_usock));	//queue ipx
	cyg_semaphore_init(&IPXUSOCK.sem_f,0);				//queue ipx

	//Open Receive Socket
	IPXOpenSocket((BYTE*)&SocketData,LONG_LIVED);
	NTWaitforDataECB();
}

void ListenUpgradePack(void)
{
	// Setup Download-ECB Block
	RecvUtilUpgradeECB.ESRAddress = UtilUpgradeESR;
	RecvUtilUpgradeECB.inUseFlag  = 0;
	RecvUtilUpgradeECB.socketNumber  = UpgradeSocket;
	RecvUtilUpgradeECB.fragmentCount = 2;
	RecvUtilUpgradeECB.fragmentDescriptor[0].address = &RecvUtilUpgradeIPXHeader;
	RecvUtilUpgradeECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	RecvUtilUpgradeECB.fragmentDescriptor[1].address = &RecvUtilUpgradeData;
	RecvUtilUpgradeECB.fragmentDescriptor[1].size    = sizeof(IPX_Upgrade_Rec);

	IPXListenForPacket(&RecvUtilUpgradeECB);
}

void UtilUpgradeESR(void)
{

	if(memcmp(RecvUtilUpgradeData.Mark, NT_CHECK_MARK2,NT_CHECK_MARK2_LEN) == 0) {

		switch (RecvUtilUpgradeData.Cmd[0]) {
		case CMD06: // Recv Ack Block Data !
			cyg_semaphore_post(&SIGNAL_IPX_UPGRADE_DATA);
			break;
		case CMD07: // Recv Ack Upgrade Complete !
			cyg_semaphore_post(&SIGNAL_IPX_UPGRADE_FINISH);
			break;
		}
	}
	ListenUpgradePack();

}

void UpgradeSocketInit(void)
{
	uint16 SocketData;

	//Open Upgrade Socket
	SocketData = UpgradeSocket;
	IPXOpenSocket((uint8 *)&SocketData,LONG_LIVED); 
		
	ListenUpgradePack(); //Listen Upgrade Packet
}
