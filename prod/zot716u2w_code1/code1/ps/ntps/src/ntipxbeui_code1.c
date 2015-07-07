#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include <stdlib.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "nps.h"
#include "ipx.h"
#include "netbeui.h"
#include "ntps.h"
#include "led.h"



//IPXUpgrade Thread initiation information
#define IPXUpgrade_TASK_PRI         	20	//ZOT716u2
#define IPXUpgrade_TASK_STACK_SIZE  	2048 //ZOT716u2 3072
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
NTRequestData  		*NTReceiveRequestData;	//queue ipx

DWORD          		IPXTimeOut,IPXPktErr,IPXBlockErr;

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


void ListenUtilPack(void);


void UtilSendEEPROM(void);				// (80)
void UtilConfigBox(void);				// (81)
void UtilViewBox(void);					// (84)
void UtilStartUpgrade(void);			// (85)
void UtilPrintStatus(void);				// (88)
void UtilRecvEEPROM(void);				// (8D)

void UtilSendDeviceInfo(BYTE);			// (90)
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
	
	while((buf = ipx_dequeue(&ipx_usock->rcvq)) == NULL)
	{			
		cyg_semaphore_init(&ipx_usock->sem_f,0);
		ErrTmp = (int)cyg_semaphore_timed_wait(&ipx_usock->sem_f,cyg_current_time() + (TICKS_PER_SEC/3));

		if(ErrTmp == 0)
			return -1;	//time out
	}
	
	RecvReqData = (NTReqData *) &(buf->data)->Nt2.Cmd;	//queue ipx
	
	if(RecvReqData->BlockNumber != NTPortInfo[PortNumber].BlockNumber)
	{
		free(buf->data);
		free(buf);
		return -2;	//Block Number error
	}
	
	if((BYTE)RecvReqData->DataLength)
		len = RecvReqData->DataLength - 0x100;
	else
		len = RecvReqData->DataLength;
	
	memcpy(Data, RecvReqData->Data, len);
	
	free(buf->data);
	free(buf);
	
	return (len);

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


//(80) Send EEPROM data.
void UtilSendEEPROM(void)
{
	uint8 huge* Co2RVer;

	SendUtilData.Nt2.Cmd[0]=0x80;
	SendUtilData.Nt2.Cmd[1]=0x01; //0x00: 512K, 0x01: 1024K
	DataCopy(SendUtilData.Nt2.Data,&EEPROM_Data,sizeof(EEPROM_Data));
/*Jesse
	// Charles
	memcpy(SendUtilData.Nt3.ExtMark,"Nver",4);  //Name version
	memcpy(SendUtilData.Nt3.CODE1Ver,(uint8 *)(CODE1_START_ADDRESS+MAJOR_VER_OFFSET),sizeof(SendUtilData.Nt3.CODE1Ver));
	SendUtilData.Nt3.CODE1Ver[sizeof(SendUtilData.Nt3.CODE1Ver)-1] = '\0';

	//Charles
	Co2RVer = (RAM_CODE_START_ADDRESS+MAJOR_VER_OFFSET);

	memcpy(SendUtilData.Nt3.CODE2Ver, Co2RVer, sizeof(SendUtilData.Nt3.CODE2Ver));
	SendUtilData.Nt3.CODE2Ver[sizeof(SendUtilData.Nt3.CODE2Ver)-1] = '\0';

	UtilGetVersionString(SendUtilData.Nt3.ModelName);
	strcat(SendUtilData.Nt3.ModelName," (SC4510-50)");
*/
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
	SendUtilData.Nt2.Cmd[1]=0xFF;  //Waiting for D/L
	
	memset(SendUtilData.Nt2.Data, 0, 1024);
	
	UtilSetListViewData((LIST_VIEW *)SendUtilData.Nt2.Data);

	ppause(urandom(10)+1);	//delay 1 - 10 ms 9/9/99

	SendUtilityPacket();
}

//(85) Switch to Code1 for Download Flash
void UtilStartUpgrade(void)
{

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
	uint8 PortNumber;

	SendUtilData.Nt2.Cmd[0] = 0x88;
	SendUtilData.Nt2.Cmd[1] = 0xFF;      // Download mode

	//Printer Status

	SendUtilData.Nt2.Data[0] = 0;


	UtilRemoveBusyStatus(&SendUtilData.Nt2.Data[0]);

	//4/26/2000 changed
	SendUtilData.Nt2.Data[1] =  0;

	//Using Status
	for(PortNumber = 0; PortNumber < NUM_OF_PRN_PORT; PortNumber++) {
		SendUtilData.Nt2.Data[PortNumber+2] = 0;
	}

	SendUtilityPacket();
}

//(8D) Receive EEPROM Data.
void UtilRecvEEPROM(void)
{
	uint8 Result = 0xFF;

	// Charles 2001/08/27
	if( !strcmp(((EEPROM *)ReceiveUtilData.Nt4.Data)->ZOT_Mark,"ZOT") && 
		((EEPROM *)ReceiveUtilData.Nt4.Data)->Model == CODE1_PS_MODEL )
	{
		if( ReceiveUtilData.Nt4.Cmd[1] == 0x01 )
	    {

			if( !memcmp(((EEPROM *)ReceiveUtilData.Nt4.Data)->EthernetID,MyPhysNodeAddress,6) )
			{
				Result = 0x00;
				DataCopy((uint8 *)&EEPROM_Data,ReceiveUtilData.Nt4.Data, sizeof(EEPROM_Data));

				memcpy( &DEFAULT_Data, &EEPROM_Data, sizeof(EEPROM) );

				if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xFF;
			}
		}
		else if( ReceiveUtilData.Nt4.Cmd[1] == 0x02 )
		{
			Result = 0x00;
			DataCopy((uint8 *)&EEPROM_Data,ReceiveUtilData.Nt4.Data, sizeof(EEPROM_Data));

			memcpy( &DEFAULT_Data, &EEPROM_Data, sizeof(EEPROM) );

			if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xFF;
		}
	}

	SendUtilData.Nt4.Cmd[0] = 0x8D;
	SendUtilData.Nt4.Cmd[1] = Result;

	SendUtilityPacket();
}




//(90) Send Device Info. 4/18/2000 added
void UtilSendDeviceInfo(uint8 PrinterID)
{
	uint16 len;
	uint8  *p;

	SendUtilData.Nt2.Cmd[0] = 0x90;
	SendUtilData.Nt2.Cmd[1] = 0x00;

	//Printer Status
	SendUtilData.Nt2.Data[0] = PrinterID;
	memset(&SendUtilData.Nt2.Data[1],'\0',10);

	SendUtilityPacket();
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
			(((EEPROM *)recbuffer_ipx)->Model == CODE1_PS_MODEL) )
		{
			Result = 0x00;
			DataCopy((uint8 *)&EEPROM_Data, recbuffer_ipx, sizeof(EEPROM_Data));

			memcpy( &DEFAULT_Data, &EEPROM_Data, sizeof(EEPROM) );

			if(WriteToDefault(&DEFAULT_Data) != 0)  Result = 0xFF;
			else if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xFF;

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
//code1 don't save data.			
//				UtilConfigBox();
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
			case CMD0D: //Write EEPROM
				UtilRecvEEPROM();
				break;
			case CMD10: //Send Device Info 4/18/2000
				UtilSendDeviceInfo(ReceiveUtilData.Nt2.Data[0]);
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

void IPXUtilityInit(void)
{
	uint16 SocketData= UtilitySocket;;

	//Open Utility Socket
	IPXOpenSocket((uint8 *)&SocketData,LONG_LIVED);
	ListenUtilPack();    // Listen Utility Packet
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