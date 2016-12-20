#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"  //print server configuration file
#include "psdefine.h"
#include "eeprom.h"
#include "led.h"
#include "ledcode.h"
#include "banner.h"
#include "prnqueue.h"
#include "prnport.h"
#include "ipx.h"

//615wu------------------------------------------------------------------------
extern uint32	PollingTime;	
extern BYTE  PSMode;
extern void starSAPThread(void);
static void kwait(int a){cyg_thread_yield(); }	//615wu //must be, could not use 'yeild' to replace

#include "nps.h"
#ifdef NDS_PS
#include "ndsmacro.h"
#include "ndsqueue.h"
#include "nds.h"
#include "ndsmain.h"
#include "nwcrypt.h"
#endif NDS_PS
#ifdef SNMPIPX
//#include "snmpd\snmp_ipx.h"
#endif SNMPIPX

// Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
extern BYTE WebLangVersion[];
extern int diag_flag;

//from proport.c
extern struct parport 	PortIO[NUM_OF_PRN_PORT];
#define _PortMask(Port) 		(0x03<<(Port<<1))
#define _PrintPortStatus(Port)	((ReadPrintStatus()&_PortMask(Port)) >> (Port<<1))

extern unsigned char       RSAPublicHMAC_MD5[16];

//ZOTIPS extern struct netif *WLanface;
extern UINT8	Hostname[LENGTH_OF_BOX_NAME+1];
extern UINT8    mvAPPTLKEn;
extern UINT8    mvATPortName[ATALK_PORT_NAME];
extern UINT8    mvRENVEnable;
extern UINT8    mvRENVServiceName[64];

#ifdef WIRELESS_CARD
extern UINT8    mvBSSType;
extern UINT8    mvWEPType;
extern UINT8    mvAuthenticationType;
extern UINT8    mvTxMode;
extern UINT	   mvWPAType;
#define WLAN_BSSID_LEN		6

#endif

//------ define for PortPCB Block -----------------------------------

#define _pPrnPortPCB(n,offset)  (LPTPort[n]+offset)

#define _bActPort(nPort)        (PortInfo[nPort].ActivePortPCB)
#define _bCurPort(nPort)        (PortInfo[nPort].CurrentPortPCB)
#define _bFreePort(nPort)       (PortInfo[nPort].FreePortPCB)

#define _pActivePortPCB(nPort)   (_pPrnPortPCB(nPort,_bActPort(nPort)))
#define _pCurrentPortPCB(nPort)  (_pPrnPortPCB(nPort,_bCurPort(nPort)))
#define _pFreePortPCB(nPort)     (_pPrnPortPCB(nPort,_bFreePort(nPort)))

#ifdef NDS_PS
#define _Set2Bmode(nPort)        (PortInfo[nPort].CurServiceMode = PS_NETWARE_MODE)
#define _IsBmode(nPort)          (PortInfo[nPort].CurServiceMode & PS_NETWARE_MODE)
#define _IsDSmode(nPort)         (PortInfo[nPort].CurServiceMode & PS_NDS_MODE)
#define _ModeToggle(nPort)       (PortInfo[nPort].CurServiceMode ^= (PS_NETWARE_MODE|PS_NDS_MODE))
#else
#define _IsBmode(nPort)          (1)
#endif NDS_PS


//---------------------- Global Variable Definitions -------------------------


ECB               SendPsECB,         ReceivePsECB;
ECB               SendSAPECB,        ReceiveSAPECB;

BYTE              IntoNPS3main;	  //8/26/98
BYTE			  IntoAttachFS;	  //8/26/98


#if defined(HTTPD) && !defined(CODE1)
BYTE HttpdUsed;
#endif HTTPD

#ifndef CODE1
ECB               SendWDogECB,       ReceiveWDogECB;
#endif !CODE1

IPXHeader         SendPsIPXHeader,   ReceivePsIPXHeader;
IPXHeader         SendSAPIPXHeader;

#ifndef CODE1
IPXHeader         SendWDogIPXHeader, ReceiveWDogIPXHeader;
WatchDog_Rec      SendWDogData,      ReceiveWDogData;
#endif !CODE1

BYTE              ReceivePsIPXData[MAX_PS_RECEIVE_LEN];
BYTE              SendPsIPXData[MAX_PS_SEND_LEN];

SAPResponseData   SendSAPIPXData;

NCPResponseData  *ReceiveNCPData;
NCPQueryData     *SendNCPData;
BYTE             *ReceiveNCPSubData;
BYTE             *SendNCPSubData;

WORD             CSIP;      // for Trace.

#ifdef CODE1
#ifdef _PC
WORD             ServiceFSCount;
#endif
#else

WORD             ServiceFSCount;
uint32           StartByteOffset;
WORD             ReadByte;	//for call ReadFileData function()
BYTE             PrintQueueName[MAX_NAME_LEN];

BYTE             OpenQueueBuffer[] = {"SYS:SYSTEM/00000000/QUEUE.000"};

#endif CODE1

BYTE             BrocastNetwork[4] = { 0x00, 0x00, 0x00, 0x00 };
BYTE             BrocastNode[6]    = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#ifndef CODE1
PrnInfo          PortInfo[NUM_OF_PRN_PORT];
PortPCB          *LPTPort[NUM_OF_PRN_PORT];
#endif !CODE1

BYTE             SAPName[13];

#ifndef CODE1

BYTE             LoginKeyID[8];
BYTE             BinderyObjectID[4];

//BYTE             FileHandle[6];
//BYTE             JobNumber[2];
//BYTE             ClientIDNumber[4];

#endif !CODE1

BYTE             NovellConnectFlag;
WORD             status;

#ifndef CODE1
FSInfo           *FSInfoBlock;
BYTE             bNoActiveFSInfo; //still not ative File Server Block (Ring)
BYTE             CurServicePort;
BYTE             *PrintData;  //point to ReadFileData's read data

#endif !CODE1

int16            CurrentFrameType; //Current Netware Frame Type Number


#ifdef PC_OUTPUT
extern void ErrorBeep(void);
extern void AtSaySpace(int X, int Y,int SpaceLen) ; //unknow.c
extern void At(int,int); //unknow.c
#endif

#ifdef _PC
extern long NovellDelayTime; //main.c
#endif


static void InitSAP (void);
static void ListenWatchDog (void);
       WORD HasAnyFileServerConnect(void); //4/30/98
static WORD AttachToFileServer (WORD SocketNumber, FSInfo *FSInfoPointer);
       WORD QueryNearestFileServer (WORD SocketNumber, FSInfo *FSInfoPointer);
       WORD RequestCreateServiceConnect (WORD Socket, FSInfo *FSinfoPointer); //8/23/99 remove static
       WORD RequestNegotiateBufferSize (WORD Socket, FSInfo  *FSInfoPointer); //8/23/99 remove static
       WORD ReadPropertyValue (WORD Socket, FSInfo *ConnectedFSInfoPointer, BYTE *FileServerName,WORD ObjectType, BYTE *NetworkAddress);
       WORD DisConnection (WORD Socket, FSInfo *FSInfoPointer);	//8/23/99 remove static
       WORD RequestRIP (WORD Socket, FSInfo *FSInfoPointer);  //8/23/99 remove static
static WORD LoginPSToFileServer (WORD Socket, FSInfo *FSInfoPointer);
static WORD GetPSQueueID (WORD Socket, FSInfo *FSInfoPointer);
static WORD ServiceQueueJob (WORD Socket,PortPCB  *PortPCBPointer, BYTE *QObjectID);
       WORD ReadQueueJobEntry (WORD Socket, PortPCB  *PortPCBPointer, BYTE *QObjectID, BYTE **TextScript);
static WORD GetBinderyObjectName (WORD Socket, FSInfo *FSInfoPointer, BYTE *QueueObjectID);
static WORD ReadFileData (WORD Socket, FSInfo  *FSInfoPointer, BYTE *FileHandle, BYTE *StartByteOffset, BYTE *ReadByte);
static WORD FinishServiceQueueJob (WORD Socket, FSInfo  *FSInfoPointer, BYTE *JobNumber, BYTE *QObjectID);
static WORD GetLoginKey (WORD Socket, FSInfo *FSInfoPointer);
static WORD GetBinderyObjectID(WORD Socket,FSInfo *FSInfoPointer,WORD ObjectType,int16 ObjectNameLength,BYTE *ObjectNameValue);
static WORD LoginToFS (WORD Socket, FSInfo *FSInfoPointer);
static WORD OpenQueueInfoFile (WORD Socket, FSInfo  *FSInfoPointer,BYTE *FileHandle);
       WORD AttachQueueServer (WORD Socket, FSInfo *FSInfoPointer, BYTE *QueueID);
static WORD CloseQueueInfoFile (WORD Socket,FSInfo *FSInfoPointer, BYTE *FileHandle);
       WORD NCPRequest (WORD Socket, FSInfo *FSInfoPointer); //8/23/99 remove static
static void ProcessLoginKey (BYTE * GetBinObjectID);
static void EnCode_063A (BYTE *IDOrNum, BYTE *SearchPointer, WORD Counter, BYTE *PointerLast);
static void Encode_05B9 (BYTE *SourceBuff, BYTE *ScanBuf, BYTE *DestBuff);
static void ClearMemory (BYTE *CMpointer, WORD CMcounter);
static void EnCodeAgain_0743 (int8 *MakeBuffer, int8 *PointerLast);
static WORD CheckInUseFlag (BYTE WaitResponseTime);
static WORD CheckConnection (FSInfo  *FSInfoPointer);
static void WatchDogESR (void);
static void MoveAttachPortToFreePort (void);
static void TryConnectNoAttachFS (void);
#ifdef NOVELL_PS
void NovellPrintServerInit (void)
{
	int16 i;

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		_bFreePort(i) = _bCurPort(i) = _bActPort(i) = NULL_BYTE;
		PortInfo[i].StartTime = rdclock();

#ifdef NDS_PS
		_Set2Bmode(i);
		_NDSbActPort(i) = _NDSbCurPort(i) = _NDSbFreePort(i) = NULL_BYTE;
#endif NDS_PS

	}
}
#endif NOVELL_PS

#ifdef NOVELL_PS
void BinderyPrintServerInit (void)
{
	int16 i,j;

	CSIP = 0 ;

	FSInfoBlock = (FSInfo *)calloc(ServiceFSCount,sizeof(FSInfo));
#ifdef PC_OUTPUT
	if(FSInfoBlock == NULL) {
		printf("Error! Not enough memory (ConnectNetware()) !\n");
		exit(0);
	}
#endif

	for (i=0 ; i < NUM_OF_PRN_PORT; i++)
	{
		LPTPort[i] = (PortPCB *)calloc(ServiceFSCount,sizeof(PortPCB));

#ifdef PC_OUTPUT
		if(LPTPort[i] == NULL) {
			printf("Error ! Not enough memory (NovellPrintServerInit()) !\n");
			exit(0);
		}
#endif
		_bFreePort(i) = 0;  // Assign Buffer Pointer
		for (j=0 ; j < (ServiceFSCount-1) ; j++) {
			_pPrnPortPCB(i,j)->NextPortPCB = j+1;
		}
		_pPrnPortPCB(i,j)->NextPortPCB = NULL_BYTE;

	} // end for
}
#endif NOVELL_PS

void NovellPrintSocketInit(void)
{
	WORD SocketData;

	CSIP = 0 ;

#ifndef CODE1
	ReceiveNCPData    = (NCPResponseData  *)ReceivePsIPXData;
	ReceiveNCPSubData = ReceivePsIPXData + sizeof(NCPResponseData);

	SendNCPData = (NCPQueryData  *)SendPsIPXData;
    SendNCPSubData = SendPsIPXData + 6;
#endif CODE1

	//Open Print Servre Socket
	SocketData=PsSocket;
	if(IPXOpenSocket((BYTE *)&SocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
		printf(" Can't Open Ps Socket\n");
		exit(0);
#endif
#ifndef _PC
		LightOnForever(LED_PS_SOCKET);
#endif
	}

	//Open SAP Socket
	SocketData=SAPSocket;
	if(IPXOpenSocket((BYTE*)&SocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
		printf(" Can't Open SAP Socket\n");
		exit(0);
#endif
#ifndef _PC
		LightOnForever(LED_SAP_SOCKET);
#endif
	}

#ifndef CODE1
	//Open WatchDog Socket
	SocketData=WatchDogSocket;
	if(IPXOpenSocket((BYTE*)&SocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
		printf (" Can't Open WDog Socket\n");
		exit(0);
#endif
#ifndef _PC
		LightOnForever(LED_WDOG_SOCKET);
#endif
    }

#if defined(HTTPD) && !defined(CODE1)
	//Open HTTPD Socket for Search FS
	SocketData=SearchFSocket;
	if(IPXOpenSocket((BYTE*)&SocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
		printf (" Can't Open HTTP-Search FSg Socket\n");
		exit(0);
#endif
#ifndef _PC
		LightOnForever(LED_SEARCH_FS_SOCKET);
#endif
	}
#endif HTTPD

#endif !CODE1

	NovellConnectFlag = 0; //disconnect
	sprintf(SAPName,"ZOT-PS%02X%02X%02X",MyPhysNodeAddress[3] ,MyPhysNodeAddress[4] ,MyPhysNodeAddress[5]);

	InitSAP();           // Initialize SAP

#ifdef SNMPIPX
//	HPJetAdminSAPInit(); //6/21/2000
#endif SNMPIPX

#ifndef CODE1
	ListenWatchDog();    // Listen Watch Dog packet
#endif !CODE1

//	QueryNearestFileServer(PsSocket,NULL); //for Get MyNetworkAddress 5/11/98	//eCos move to ConnectNetware
}


int PrimaryConnectFinish = 0;
#ifdef NOVELL_PS

//615wu

void ConnectNetware(cyg_addrword_t data)
{
	int16 i;

	bNoActiveFSInfo = NULL_BYTE;

    ppause(10000);	//>300ticks	//615wu

	//Start SAP Thread		//eCos
	QueryNearestFileServer(PsSocket,NULL);
	starSAPThread();	

	for (i=0 ; i < ServiceFSCount ; i++)
	{
		FSInfoBlock[i].PCBFileServerName = _FileServerName(i);
		FSInfoBlock[i].PCBSequenceNumber = 0x00;
#ifdef PC_OUTPUT
		AtSaySpace(0,15,80);
		printf("?(BIND) FServer:[%s] Try Connection... ",FSInfoBlock[i].PCBFileServerName);
#endif

		if(AttachToFileServer(PsSocket, &FSInfoBlock[i]) != OKAY) {
			//No Connect !! add as a ring

			if(bNoActiveFSInfo == NULL_BYTE) bNoActiveFSInfo = i;

			FSInfoBlock[i].NextFSInfo = FSInfoBlock[bNoActiveFSInfo].NextFSInfo;
			FSInfoBlock[bNoActiveFSInfo].NextFSInfo = i;
			bNoActiveFSInfo = i;
#ifdef PC_OUTPUT
			putchar(0x07);
#endif
		}
		else {
			// Connect is OK !! //
		}
	}
	//615wu
	PrimaryConnectFinish = 1;
    sys_check_stack();
}
#endif NOVELL_PS

void ListenPsECB (WORD SockNumber, WORD ReceiveSize)
{
	// InitialSetup receive PS ECB
	ReceivePsECB.ESRAddress = 0x00;
	ReceivePsECB.inUseFlag  = 0;
	ReceivePsECB.socketNumber = SockNumber;
	ReceivePsECB.fragmentCount = 2;
	ReceivePsECB.fragmentDescriptor[0].address = &ReceivePsIPXHeader;
	ReceivePsECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	ReceivePsECB.fragmentDescriptor[1].address = ReceivePsIPXData;
	ReceivePsECB.fragmentDescriptor[1].size    = ReceiveSize;

	// Receive IPX Packet
	IPXListenForPacket(&ReceivePsECB);
}

void SendPsECBInit (WORD SocketNumber, BYTE *TargetAddress,WORD SendSize)
{
	// Initial send PS ECB
	SendPsECB.ESRAddress = 0x00;
	SendPsECB.inUseFlag  = 0;
	SendPsECB.socketNumber  = SocketNumber;
	DataCopy(SendPsECB.immediateAddress, TargetAddress, 6);

	SendPsECB.IPXFrameType = GetIPXFrameType(CurrentFrameType); //3/16/98

	SendPsECB.fragmentCount = 2;
	SendPsECB.fragmentDescriptor[0].address = &SendPsIPXHeader;
	SendPsECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	SendPsECB.fragmentDescriptor[1].address = SendPsIPXData;
	SendPsECB.fragmentDescriptor[1].size    = SendSize;

#ifdef PC_OUTPUT
	if(SendSize > MAX_PS_SEND_LEN) {
		At(30,13);
		printf("\a \a \a SEND SIZE > BUFFER SIZE (SendPsECBInit) \a \a \a");
		ErrorBeep();
	}
#endif
}

WORD QueryNearestFileServer (WORD SocketNumber, FSInfo *FSInfoPointer)
{
	int16 FrameTimeOut;
	uint32 startime;
	SAPResponseData *ReceiveSAPData;
	SAPQueryData    *SendSAPData;


#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_QUERY_NEAREST);
#endif NDS_PS

ReListenSAP:

#ifdef PC_OUTPUT
	AtSaySpace(0,15,80);
	printf ("? Query Nearest File Server....");
//	ppause(3000L);
#endif

	//Listen Ps ECB Block
	ReceiveSAPData = (SAPResponseData *)ReceivePsIPXData;
	ListenPsECB(SocketNumber, 66);
	//Initial Send Ps ECB Block
	SendSAPData = (SAPQueryData *)SendPsIPXData;
	SendPsECBInit(SocketNumber, BrocastNode,sizeof(SAPQueryData));

	// Setup IPX Header
//	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	SendPsIPXHeader.packetType = 0x04;   // IPX Packet Type 8/3/99 changed
	DataCopy (SendPsIPXHeader.destination.network, BrocastNetwork, 4);
	DataCopy (SendPsIPXHeader.destination.node, BrocastNode, 6);
	NSET16(SendPsIPXHeader.destination.socket, SAPSocket);

	// Setup SAP Protocol
	SendSAPData->PacketType = 0x0300;    // Query Nearest File Server
	SendSAPData->ServerType = 0x0400;    // File Server

	CurrentFrameType = 0;

ReSendOtherFrameType:

	SendPsECB.IPXFrameType = GetIPXFrameType(CurrentFrameType);

	// Send (Query Nearst File Server with Auto-Detect FrameType) Packet
	do {
		IPXSendPacket(&SendPsECB);
	} while (SendPsECB.inUseFlag) ;

	startime = rdclock();

	while(ReceivePsECB.inUseFlag || ReceivePsECB.completionCode) {
		kwait(0);
		if((rdclock() - startime) > SAP_RETRY_TIME) {
			if(++CurrentFrameType >= MaxFrameType) {
				CurrentFrameType = 0;
				status = IPXCancelEvent(&ReceivePsECB);
#ifdef PC_OUTPUT
				if(status != SUCCESSFUL) {
					AtSaySpace(40,20,35);
					printf("(Netware) Cancel ECB Fail (1)");
					ErrorBeep();
				}
#endif
#ifdef NDS_PS
				END_NETWARE_CRITICAL();
#endif NDS_PS
				return(FAILURE);
			}
			goto ReSendOtherFrameType;
		}
	}

	if(NGET16(ReceivePsIPXHeader.source.socket) != SAPSocket) {
#ifdef PC_OUTPUT
		AtSaySpace(0,14,80);
		printf("Receive ECB is not a SAPSocket ");
		printf("Node->%s ", ReceivePsIPXHeader.source.node);
		printf("Socket->%x ", ReceivePsIPXHeader.source.socket);
#endif
		goto ReListenSAP;
	}

	if(FSInfoPointer != NULL) {	//5/11/98 add this line !

		//Get the Nearest FileServer Network Address
		DataCopy(FSInfoPointer->PCBNetworkNumber, ReceiveSAPData->Network, 12);

		//Get the Nearest FileServer Physical Node Addsess
		DataCopy(FSInfoPointer->PCBPhysicalID, ReceivePsECB.immediateAddress, 6);
	}

	//This Network Address
	DataCopy(MyNetworkAddress, ReceivePsIPXHeader.destination.network, 4);

	//Set SAP NetworkAddress as this Network Address
	//Before set it the SAP will use BrocastNetwork as network address
	DataCopy(SendSAPIPXData.Network, MyNetworkAddress, 4);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (OKAY);
}

#ifdef NOVELL_PS //--//--//--//--//--//--//--//--//--//--//--//--//--//--//
//return Attach to server port (0 - 2)
// if return value >= NUM_OF_PRN_PORT then no any file server attached
WORD HasAnyFileServerConnect(void)
{
	WORD i;

	for (i=0 ; i < NUM_OF_PRN_PORT ; i++) {
		if(_bActPort(i) != NULL_BYTE) {
			break;
		}
	}

	return i;
}


WORD AttachToFileServer (WORD SocketNumber, FSInfo *FSInfoPointer)
{
	WORD RetCode;
	WORD AttachPort;
	FSInfo *ConnectedFSInfo;
	BYTE NetworkAddress[12];  //want connected NetworkAddress	

	AttachPort = HasAnyFileServerConnect();

	if(AttachPort >= NUM_OF_PRN_PORT) {
		//No File Server Attached, Try connect to nearest File Server

		NovellConnectFlag = 0; //disconnect

		//------ Query Nearest File Server (SAP) -----------------------
		//Get nearest FS's NetworkNumber, PhysicalAddress and This NetworkAddress
		if(QueryNearestFileServer(SocketNumber, FSInfoPointer) != OKAY)
			return (FAILURE);

#ifdef NDS_PS
		//------ Request Connect to Server ----------------
		if(RequestConnect2Server(SocketNumber, FSInfoPointer) != OKAY)
			return (FAILURE);
#else
		//------ Request Create Service Connection (NCP)----------------
		//For Get Connect Number (0)
		if(RequestCreateServiceConnect(SocketNumber, FSInfoPointer) != OKAY)
			return (FAILURE);

		//------ Request Negotiate Buffer Size (NCP)--------------------
		//Get negotiate buffer Size
		if(RequestNegotiateBufferSize(SocketNumber, FSInfoPointer) != OKAY)
			return (FAILURE);
#endif NDS_PS

		ConnectedFSInfo = FSInfoPointer;
	}
	else {
		//Get any connected FS from ActivePCB
		ConnectedFSInfo = _pActivePortPCB(AttachPort)->pFSInfo;
	}

#ifdef PC_OUTPUT
	AtSaySpace(0,14,80);
	printf ("F Server:[%s] Try Connection... ",FSInfoPointer->PCBFileServerName);
#endif

	//------- Read Property Value (NCP) ----------------------------------
	//Get FS'S DATA	(NetworkNumber and Socketnumber)
	RetCode = ReadPropertyValue(SocketNumber,
	                            ConnectedFSInfo,
	                            FSInfoPointer->PCBFileServerName,
	                            FILE_SERVER_OBJECT,
	                            NetworkAddress);

	if(AttachPort >= NUM_OF_PRN_PORT) {
		// Destroy Nearest File Service Connection
		DisConnection(SocketNumber, ConnectedFSInfo);
	}

	//move NetworkAddress to FSInfo Block
	DataCopy(FSInfoPointer->PCBNetworkNumber,NetworkAddress,12);

	if(RetCode != OKAY) {
#ifdef PC_OUTPUT
		AtSaySpace(40,14,80);
		printf (" FServer:[%s] No such object... ",FSInfoPointer->PCBFileServerName);
#endif
	    return (FAILURE);
	}

	//------- Request RIP (RIP) ------------------------------------------
	//Get FS's Physical Node Address
	if(RequestRIP(SocketNumber,FSInfoPointer) != OKAY) return (FAILURE);


	if(LoginPSToFileServer(SocketNumber,FSInfoPointer) != OKAY) {
		DisConnection(SocketNumber, FSInfoPointer);
		return (FAILURE);
	}

	if(GetPSQueueID(SocketNumber,FSInfoPointer) != OKAY) {
		DisConnection(SocketNumber, FSInfoPointer);
		return (FAILURE);
	}

#ifdef PC_OUTPUT
	AtSaySpace(40,15,80);
	printf ("# The FServer:[%s] is Connect ! ",FSInfoPointer->PCBFileServerName);
#endif
#if (NUM_OF_STATUS_LED == 2)
	Light_Off(G_Lite|R_Lite);
	Light_On(G_Lite);
#endif
	NovellConnectFlag= 1;
	return(OKAY);
}

//-----------------	Get Queue Job and want to service it ----------
//Return Code :
// 0: Next time need Service again !
// TIME_OUT     : NCP call fail
// NO_QUEUE_JOB : No Job in Queue !(SERVICE_NEXT_QUEUE)
// SERVICE_DATA : Found Queue then want to service it !
// SERVICE_AGAIN: Next time need service again !
// Other        : No connect !
//------------------------------------------------------------------
WORD ServiceNetWareQueue (WORD Socket, PortPCB  *PortPCBPointer)
{
	BYTE   *QObjectID;
	BYTE   PortStatus = ReadPortStatus(CurServicePort);	  //5/12/99

	CSIP = 3;

	if(PrnPrinterPortIsBusy(CurServicePort) ||
	   PortStatus == PORT_PAPER_OUT         ||	//5/12/99
	   PortStatus == PORT_OFF_LINE )			//5/12/99
	{
#ifdef PC_OUTPUT
		AtSaySpace(0,CurServicePort*4+3,40);
		printf("(Netware) Port%d is Busy",CurServicePort);
#endif
		return (OKAY);	//next time need service again
	}

	if(PrnGetAvailQueueNO(CurServicePort) < 2) {
		//need at least 2 InQueue buffer for print banner
#ifdef PC_OUTPUT
		AtSaySpace(0,CurServicePort*4+3,40);
		printf("(Netware) Port%d:Queue Buffer < 2",CurServicePort);
#endif
		return (OKAY); //next time need service again
	}

	if((rdclock() - PortInfo[CurServicePort].StartTime) < PollingTime) {
#ifdef PC_OUTPUT
		AtSaySpace(0,CurServicePort*4+3,40);
		printf("(Netware) Port%d:Polling Time < %d",CurServicePort,PollingTime/TICKS_PER_SEC);
#endif
		return (SERVICE_AGAIN); //next time need service again
	}

    PortInfo[CurServicePort].StartTime = rdclock(); //5/14/98 Simon

#ifdef PC_OUTPUT
	while(PrnGetPrinterStatus(CurServicePort) != PrnNoUsed) {
		AtSaySpace(0,CurServicePort*4+3,40);
		printf("(Netware) Two Thread want to use same Port%d",CurServicePort);
		ErrorBeep();
	}
#endif
    PrnSetNetwareInUse(CurServicePort);
/*
#ifdef SUPPORT_JOB_LOG
			JL_PutList(6, CurServicePort, PortPCBPointer->pFSInfo->PCBFileServerName, 32);
#endif //SUPPORT_JOB_LOG
*/
	//-------------------- Service Queue Job (NCP) -----------------------
	QObjectID = PortPCBPointer->PCBQueueInfo[PortPCBPointer->QCount].QueueObjectID;

#ifdef PC_OUTPUT
	AtSaySpace(10,CurServicePort*4+2,70);
	printf ("(Netware) Service [%s] --> Queue:%d (%s) TotalQueue:%d ",
	        PortPCBPointer->pFSInfo->PCBFileServerName,
	        PortPCBPointer->QCount,
			PortPCBPointer->PCBQueueInfo[PortPCBPointer->QCount].QueueName,
	        PortPCBPointer->TotalQueue);
//	ppause(1000);
#endif

	//Get Job's ClientIDNumber, JobNumber, FileHandle
	if((status = ServiceQueueJob(Socket, PortPCBPointer ,QObjectID)) != OKAY) {
#ifdef PC_OUTPUT
		AtSaySpace(0,CurServicePort*4+3,40);
		printf ("(Netware) (No Queue Job !)");
#endif
		PrnSetNoUse(CurServicePort);
/*
#ifdef SUPPORT_JOB_LOG 			
 		JL_EndList(CurServicePort, 0);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG		
*/
		//If Print-Server is removed, status = 0x01 <<Bad Service Connection>>
		return (status);
	}

#ifdef PC_OUTPUT
	AtSaySpace(0,CurServicePort*4+3,40);
	printf("(Netware) Read QueueJOB Entry... ");
#endif
	//------------------- Read Queue Job Entry (NCP) ---------------------
	//Read Queue Job Entry and output Banner if need !
	if((status = ReadQueueJobEntry(Socket ,PortPCBPointer, QObjectID, NULL)) != OKAY) {
		PrnSetNoUse(CurServicePort);
/*
#ifdef SUPPORT_JOB_LOG 			
 		JL_EndList(CurServicePort, 0);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG		
*/		
		return (status);
	}

#ifdef PC_OUTPUT
	AtSaySpace(0,CurServicePort*4+3,40);
	printf("(Netware) Get Bindery Object Name ");
#endif

/*////////// Simon 2/25/98 //////////
	//------------------- Get Bindery Object Name (NCP) ----------------
	if((status = GetBinderyObjectName(PsSocket ,PortPCBPointer->pFSInfo ,ClientIDNumber)) != OKAY) {
		PrnSetNoUse(CurServicePort);
		return (status);
	}
*////////////////////////////////////

	CSIP = 4 ;

	PortPCBPointer->ReadByteOffset = 0x00;
	PortInfo[CurServicePort].StartTime = rdclock();

#ifdef SUPPORT_JOB_LOG
	//JL_PutList(7, CurServicePort, PortPCBPointer->pFSInfo->PCBFileServerName, 32);
	JL_PutList(7, CurServicePort, "User", 5);
#endif //SUPPORT_JOB_LOG

	return (SERVICE_DATA);
}

// ---------------- Read Job data then output to Port FIFO -------------
//return code :
//  (1) TIME_OUT           ==> Read data Fail !
//  (2) SERVICE_NEXT_QUEUE ==> Finish this Job !
//  (3) SERVICE_DATA       ==> Still need to read data from this queue !
//-----------------------------------------------------------------------
WORD QueueToPort (WORD Socket, PortPCB  *PortPCBPointer)
{
	BYTE *QObjectID;
	BYTE *JOBNum;
	PrnBuf *PrintBuffer;
	WORD PrintFlag;
	WORD MaxReadByte;

	//No InQueue buffer
	if(PrnGetAvailQueueNO(CurServicePort) < 1) {
#ifdef PC_OUTPUT
		AtSaySpace(0,CurServicePort*4+3,40);
		printf("(Netware) NO Queue Buffer !");
#endif
	    return (OKAY);
	}

#if (NUM_OF_STATUS_LED == 2)
	Light_Off(G_Lite);
#endif

#ifdef PC_OUTPUT
	AtSaySpace(0,CurServicePort*4+3,40);
	printf("(Netware) Read Queue:%d (%s) ......",
	        PortPCBPointer->QCount,
			PortPCBPointer->PCBQueueInfo[PortPCBPointer->QCount].QueueName);
#endif

#ifdef NDS_PS
	ReadByte = MaxReadByte = PortPCBPointer->pFSInfo->QueueSize;
#else
	ReadByte = MaxReadByte = MAX_JOB_READ_BYTE;               // 1024 Byte
#endif NDS_PS
	// Restore StartByteOffset
	StartByteOffset = PortPCBPointer->ReadByteOffset;

	status = ReadFileData(Socket,PortPCBPointer->pFSInfo,PortPCBPointer->FileHandle,(BYTE *)&StartByteOffset,(BYTE *)&ReadByte);
	if(status != OKAY) {
		PrnAbortSpooler(CurServicePort,NULL);
		PrnSetNoUse(CurServicePort);

#ifdef SUPPORT_JOB_LOG 			
 			JL_EndList(CurServicePort, 0);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG
		
		if(status == INVALID_FILE_HANDLE) {
#if (NUM_OF_STATUS_LED == 2)
			Light_On(G_Lite);
#endif
			return (SERVICE_NEXT_QUEUE); //this job delete by user ??
		}
		else {
#ifdef SUPPORT_JOB_LOG 			
 			JL_EndList(CurServicePort, 3);	    // George Add January 26, 2007
#endif 
			return (TIME_OUT);
       }
	}

#ifdef PC_OUTPUT
//	PrintData[ReadByte] = '\0';
//	printf ("%s \r\n",PrintData);
#endif

	//--------------- Output data ----------------------------
	PrintBuffer = PrnGetInQueueBuf(CurServicePort);
#ifdef PC_OUTPUT
	while(PrintBuffer == NULL) {
		AtSaySpace(0,20,40);
		printf("\a \a \a (Netware) Queue3 Design Error \a \a \a");
		ErrorBeep();
	}
#endif
	DataCopy(PrintBuffer->data,PrintData,ReadByte);
	PrintBuffer->size = ReadByte;

#ifdef SUPPORT_JOB_LOG
	JL_AddSize(CurServicePort, ReadByte);
#endif SUPPORT_JOB_LOG

	if(ReadByte < MaxReadByte) PrintFlag = PRN_Q_EOF;
	else PrintFlag = PRN_Q_NORMAL;

	PrnPutOutQueueBuf(CurServicePort,PrintBuffer,PrintFlag);
	//---------------------------------------------------------

	PortPCBPointer->ReadByteOffset += ReadByte;     // Queue Count

#if (NUM_OF_STATUS_LED == 2)
	Light_On(G_Lite);
#endif

	if(ReadByte < MaxReadByte) {
#ifdef NDS_PS	//eCos
		if(--PortPCBPointer->Copies) {
			//Still need service it
			PortPCBPointer->ReadByteOffset = 0;
		} else {
#endif NDS_PS	//eCos
			// Finish Job !!
			//------------Finish Servicing Queue Job (NCP)------------------------
#ifdef PC_OUTPUT
			AtSaySpace(0,CurServicePort*4+4,40);
			printf ("(Netware) Finish Queue Job....");
#endif
#ifdef NDS_PS
			if(PortPCBPointer->ClientStation != 0)
				SendNotifyMessage(Socket,CurServicePort,PortPCBPointer);
#endif NDS_PS

			QObjectID = PortPCBPointer->PCBQueueInfo[PortPCBPointer->QCount].QueueObjectID;
			JOBNum = PortPCBPointer->JOBNumber;

			status = FinishServiceQueueJob(Socket ,PortPCBPointer->pFSInfo ,JOBNum ,QObjectID);
			if(status !=OKAY ) {
				PrnSetNoUse(CurServicePort);
				
#ifdef SUPPORT_JOB_LOG 			
 			JL_EndList(CurServicePort, 0);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG				

				if(status == NO_QUEUE_JOB)
					return (SERVICE_NEXT_QUEUE); //this job delete by user ??
				else {
#ifdef SUPPORT_JOB_LOG 			
 			        JL_EndList(CurServicePort, 3);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG				
					return (TIME_OUT);
                }
			}
			PrnSetNoUse(CurServicePort);
			
#ifdef SUPPORT_JOB_LOG 			
 			JL_EndList(CurServicePort, 0);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG

			return (SERVICE_NEXT_QUEUE);
#ifdef NDS_PS	//eCos
		}
#endif NDS_PS	//eCos
	}
	return (SERVICE_DATA);
}

WORD LoginPSToFileServer (WORD Socket, FSInfo *FSInfoPointer)
{
	CSIP = 5 ;

#ifdef NDS_PS
	//------ Request Connect to Server ----------------
	if(RequestConnect2Server(Socket, FSInfoPointer) != OKAY)
		return (FAILURE);
#else
	//--------------- Request Service Connection (NCP) -------------------
	if(RequestCreateServiceConnect(Socket, FSInfoPointer) != OKAY)
		return (FAILURE);

	//----------------- Request Buffer Size (NCP) ------------------------
	if(RequestNegotiateBufferSize(Socket, FSInfoPointer) != OKAY)
		return (FAILURE);
#endif NDS_PS

	//----------------------Get Login Key (NCP)---------------------------
	//Put the login Key To LoginKeyID
	if(GetLoginKey(Socket, FSInfoPointer) != OKAY) {
		return (FAILURE);
	}

	//----------------Get PServer Bindery Object ID (NCP)-----------------
	//Get Print Server Bindery Object ID --> BinderyObjectID
	if(GetBinderyObjectID(Socket,
	                  FSInfoPointer,
	                  0x0007,
	                  strlen(_PrintServerName),
	                  _PrintServerName) != OKAY)
	{
		return (FAILURE);
	}

	//Set Print Server Bindery Object ID as Queue file path
	sprintf(OpenQueueBuffer+11,
	        "%02X%02X%02X%02X",
	        BinderyObjectID[0],
	        BinderyObjectID[1],
	        BinderyObjectID[2],
	        BinderyObjectID[3]
	);
	OpenQueueBuffer[19] = '/';

	//----------------------Login To File Server (NCP)---------------------
	if(LoginToFS(Socket, FSInfoPointer) != OKAY)
		return (FAILURE);

	return (OKAY);
}

//Get PS Queue ID from FileServer/SYS:SYSTEM/PrintServerID/QUEUE.00X
WORD GetPSQueueID (WORD Socket, FSInfo *FSInfoPointer)
{
	int16 TotalQueue;
	int16 xPort,i;
	PortPCB *PortInfoTMP;
	BYTE InsertToActivePortPCB, AttachQueueOK = 0;
	BYTE FileHandle[6];

	for(xPort=0 ; xPort < NUM_OF_PRN_PORT ; xPort++) {
		if(_bFreePort(xPort) == NULL_BYTE) {
#ifdef PC_OUTPUT
			AtSaySpace(0,15,40);
			printf ("No FreePortPCB Buffer");
#endif
			continue;
		}
		OpenQueueBuffer[28] = xPort + 0x30; // fill QUEUE.00x ; x=0,1,2
		//----------------------Open Queue Info File (NCP)----------------------
		//Get QUEUE Info File -> FileHandle
		status = OpenQueueInfoFile(Socket, FSInfoPointer,FileHandle);

		if(status == FAILURE) {
#ifdef PC_OUTPUT
			AtSaySpace(0,15,40);
			printf ("Open %s Failure !! ", OpenQueueBuffer);
#endif
			continue;
		}

		if(status != OKAY) return(FAILURE);
#ifdef PC_OUTPUT
		AtSaySpace(0,15,80);
		printf ("Read %s ...", OpenQueueBuffer);
#endif

		PortInfoTMP= _pFreePortPCB(xPort);
		PortInfoTMP->TotalQueue = 0;
		PortInfoTMP->QCount = 0;
		StartByteOffset = 0x00000000;

		for(TotalQueue=0; TotalQueue < MAX_QUEUE; TotalQueue++)	{
			ReadByte = 0x0031;              // <----- Read 49 Bytes.
			//------------- Read Queue Info File (NCP) ------------------------
			if(ReadFileData(Socket,
			                FSInfoPointer,
			                FileHandle,
			                (BYTE *)&StartByteOffset,
			                (BYTE *)&ReadByte) != OKAY
			) return (FAILURE);

//			if((*(ReceiveNCPSubData + 0) || *(ReceiveNCPSubData + 1)) == 0x00) break;
			if(ReadByte == 0) break; //End of Read Queue Info File

			StartByteOffset += ReadByte;

			if(TotalQueue%2==0)
				strcpy (PrintQueueName, ReceiveNCPSubData + 2);   // even
			else
				strcpy (PrintQueueName, ReceiveNCPSubData + 3);   // odd

#ifdef PC_OUTPUT
			AtSaySpace(0,15,80);
			printf("Queue:%s ReadByte= %d.", PrintQueueName,ReadByte);
#endif
			//-------------- Get Queue Bindery Object ID (NCP) ------------------
			//Get Print Queue Bindery Object ID --> BinderyObjectID
			status = GetBinderyObjectID(Socket,
			                      FSInfoPointer,
			                      0x0003,
			                      strlen(PrintQueueName),
			                      PrintQueueName);

			//When delete print queue but does not
			//remove queue name from Printer assigned queue list.
			if(status == NO_SUCH_OBJECT) {
#ifdef PC_OUTPUT
				AtSaySpace(0,15,80);
    			printf("NO Queue Object !!!");
#endif
			    continue;
			}

			if(status != OKAY) return(FAILURE);

			//----------- Attach Queue Server To Queue (NCP) ---------------
			//Use BinderyObjectID to Attach to Queue Server
			status = AttachQueueServer(Socket, FSInfoPointer,BinderyObjectID);
//?????
			//When delete print queue then add new same queue name;
			//at this time, QUEUE ID is not same as previous deleted.
			if(status == NO_SUCH_QUEUE) {
#ifdef PC_OUTPUT
				AtSaySpace(0,15,80);
				printf("Has object but QUEUE ID not same !!");
#endif
			    continue ; //When No Such Queue ID Member
			}
			if(status != OKAY) return(FAILURE);

			//Only keep successful Queue ID
#ifdef PC_OUTPUT
			strcpy(PortInfoTMP->PCBQueueInfo[PortInfoTMP->TotalQueue].QueueName, PrintQueueName);
#endif
			DataCopy(PortInfoTMP->PCBQueueInfo[PortInfoTMP->TotalQueue++].QueueObjectID, BinderyObjectID, 4);

		} //for(TotalQueue=0;TotalQueue < MAX_QUEUE ;TotalQueue++)...

		//------------------- Close Queue Info File (NCP) -----------------------
		if(CloseQueueInfoFile(Socket, FSInfoPointer, FileHandle) != OKAY)
			return (FAILURE);

		//(1) No any queue assign to Printer.
		//(2) Fail to attach to all Printer assigned queue.
		if(PortInfoTMP->TotalQueue == 0) continue;

		AttachQueueOK++;

		PortInfoTMP->pFSInfo = FSInfoPointer; //point to FSInfo block
		PortInfoTMP->ProcessRoutine = ServiceNetWareQueue;

		//Remove PortInfoTMP from FreePortPCB Link List
		InsertToActivePortPCB = _bFreePort(xPort);
		_bFreePort(xPort) = PortInfoTMP->NextPortPCB;

		//Add PortInfoTmp to ActivePortPCB
		PortInfoTMP->NextPortPCB = _bActPort(xPort);
		_bActPort(xPort) = InsertToActivePortPCB;
		if(_bCurPort(xPort) == NULL_BYTE)
			_bCurPort(xPort) = InsertToActivePortPCB;
	} // for(xPort=0 ; xPort < NUM_OF_PRN_PORT ; xPort++).....

	//No any Queue Attached
	if(!AttachQueueOK) return (FAILURE);

	return(OKAY);
}




//--------------- Request Routine Information Protocol (RIP) ----------------
WORD RequestRIP (WORD Socket, FSInfo *FSInfoPointer)
{
	WORD     RetryCount = 0;
	uint32   startime;
	RIPData *SendRIPData;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_REQUEST_RIP);
#endif NDS_PS

	do {

#ifdef PC_OUTPUT
		AtSaySpace(0,15,80);
		printf (" ## SendRIP ......");
#endif

//		ReceiveRIPData = (RIPData *)ReceivePsIPXData;

		//----------- Listen Ps ECB -----------------------------
//12/7/99		ListenPsECB(Socket, sizeof(RIPData));
		ListenPsECB(Socket, sizeof(ReceivePsIPXData));

		//------------ Send Ps ECB ------------------------------
		SendRIPData = (RIPData *)SendPsIPXData;

		// Setup ECB Block
		DataCopy (SendPsECB.immediateAddress, BrocastNode, 6);
        SendPsECBInit(Socket,BrocastNode,10);
		// Setup IPX Header
		SendPsIPXHeader.packetType = 0x01;   // RIP Packet Type
		DataCopy(SendPsIPXHeader.destination.network, BrocastNetwork, 4);
		DataCopy(SendPsIPXHeader.destination.node, BrocastNode, 6);
		NSET16(SendPsIPXHeader.destination.socket, RIPSocket);
		DataCopy(SendPsIPXHeader.source.network, BrocastNetwork, 4);

		// Setup RIP Protocol
		SendRIPData->Operation = 0x0100;     // Request
		DataCopy(SendRIPData->Network,FSInfoPointer->PCBNetworkNumber, 4);
		SendRIPData->Hops = 0xffff;
		SendRIPData->Tick = 0xffff;

		// Send IPX Packet
		do {
			IPXSendPacket(&SendPsECB);
		} while (SendPsECB.inUseFlag) ;

		startime = rdclock();
		while(ReceivePsECB.inUseFlag || ReceivePsECB.completionCode) {
			kwait(0);
			if((rdclock()-startime) > RIP_RETRY_TIME) {
				if(++RetryCount < MAX_RETRY_COUNT) {
					do {
						IPXSendPacket(&SendPsECB);
					} while (SendPsECB.inUseFlag) ;
					startime = rdclock();
				}
				else {
					status = IPXCancelEvent(&ReceivePsECB);
#ifdef PC_OUTPUT
					if(status != SUCCESSFUL) {
						AtSaySpace(40,20,35);
						printf("(Netware) Cancel ECB Fail (2)");
						ErrorBeep();
					}
#endif
#ifdef NDS_PS
					END_NETWARE_CRITICAL();
#endif NDS_PS
					return (TIME_OUT);
				}
			}
		}
	}while(NGET16(ReceivePsIPXHeader.source.socket) != RIPSocket);

	DataCopy(FSInfoPointer->PCBPhysicalID, ReceivePsECB.immediateAddress, 6);
//???? need set it or not 	DataCopy(SourceNetworkAddress, ReceivePsIPXHeader.Destination.Network, 4);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (OKAY);
}

//----------------------Read Property Value (NCP)----------------------------
WORD ReadPropertyValue (WORD Socket, FSInfo *ConnectedFSInfoPointer, BYTE *FileServerName,WORD ObjectType, BYTE *NetworkAddress)
{
	WORD  NCPRetCode;
	int16 FileServerNameLength = strlen(FileServerName);

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_READ_PROPERTY);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(
		Socket,
		ConnectedFSInfoPointer->PCBPhysicalID,
		6+ 3 + FileServerNameLength +
		LEN_OF_PROPERTY_NAME + 6
	);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, ConnectedFSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = ConnectedFSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = ConnectedFSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x00;
	SendNCPData->ConnectionNumberHigh = ConnectedFSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length [Word]
	*(SendNCPSubData + 2) = FileServerNameLength + LEN_OF_PROPERTY_NAME + 6;
	*(SendNCPSubData + 3) = 0x3d;        // SubFunction Code 61

	NSET16((SendNCPSubData + 4), ObjectType); // Object Type (File Server) [Lo-Hi]
	*(SendNCPSubData + 6) = FileServerNameLength;
	strcpy(SendNCPSubData + 7, FileServerName);
	*(SendNCPSubData + 7 + FileServerNameLength + 0) = 0x01;
	*(SendNCPSubData + 7 + FileServerNameLength + 1) = LEN_OF_PROPERTY_NAME;
	strcpy(SendNCPSubData + 7 + FileServerNameLength + 2, PROPERTY_NAME);
	NCPRetCode=NCPRequest(Socket,ConnectedFSInfoPointer);
	DataCopy(NetworkAddress, ReceiveNCPSubData, 12);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return(NCPRetCode);
}

//--------------------- Get Login Key (NCP) -----------------------------
WORD GetLoginKey (WORD Socket, FSInfo *FSInfoPointer)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_GET_LOGIN_KEY);
#endif NDS_PS

	// Setup ECB Block
    SendPsECBInit(Socket,
              FSInfoPointer->PCBPhysicalID,
              6 + 4
	            );	

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup SAP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // Subfunction Length
	*(SendNCPSubData + 2) = 0x01;
	*(SendNCPSubData + 3) = 0x17;        // Subfunction Code 23

	NCPRetCode=NCPRequest(Socket,FSInfoPointer);
	DataCopy(LoginKeyID, ReceiveNCPSubData, 8);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return(NCPRetCode);
}

//-------------------Get Bindery Object ID (NCP)-------------------------
WORD GetBinderyObjectID(
WORD   Socket,
FSInfo *FSInfoPointer,
WORD   ObjectType,
int16  ObjectNameLength,
BYTE   *ObjectNameValue)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_GET_BINDERY_OBJ);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket,
	              FSInfoPointer->PCBPhysicalID,
	              6 + 7 + ObjectNameLength
	            );


	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;    // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = ObjectNameLength + 4 >> 8;

	// Subfunction Length
	*(SendNCPSubData + 2) = ObjectNameLength + 4;
	*(SendNCPSubData + 3) = 0x35;        // Subfunction Code 53
	*(SendNCPSubData + 4) = ObjectType >> 8;

	// Object Type (Word)
	*(SendNCPSubData + 5) = ObjectType;
	*(SendNCPSubData + 6) = ObjectNameLength;
	DataCopy (SendNCPSubData + 7, ObjectNameValue, ObjectNameLength);

	NCPRetCode=NCPRequest(Socket,FSInfoPointer);
	DataCopy (BinderyObjectID, ReceiveNCPSubData, 4);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return(NCPRetCode);
}

//---------------------Login To File Server (NCP)-------------------------
WORD LoginToFS (WORD Socket, FSInfo *FSInfoPointer)
{
	WORD NCPRetCode;

	//Put the result to LoginKeyID
	ProcessLoginKey(BinderyObjectID);

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_LOGIN_FS);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket,
	              FSInfoPointer->PCBPhysicalID,
	              6+12+strlen(_PrintServerName)+1+2
	);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup SAP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // Subfunction Length
	*(SendNCPSubData + 2) = 12 + strlen (_PrintServerName);
	*(SendNCPSubData + 3) = 0x18;        // Subfunction Code 24
	DataCopy (SendNCPSubData + 4, LoginKeyID, 8);
	*(SendNCPSubData + 12) = 0x00;       // Object Type (Print Server)
	*(SendNCPSubData + 13) = 0x07;
	*(SendNCPSubData + 14) = strlen (_PrintServerName);
	strcpy (SendNCPSubData + 15, _PrintServerName);

	NCPRetCode  = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}

//-------------------Open Queue Info File (NCP)----------------------------
WORD OpenQueueInfoFile (WORD Socket, FSInfo  *FSInfoPointer,BYTE *FileHandle)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_OPEN_QUEUEINFO_FILE);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID,(6 + 34));
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x4c;        // Function Code 76
	*(SendNCPSubData + 1) = 0x00;        // Directory Handle
	*(SendNCPSubData + 2) = 0x06;        // Search Attributes
	*(SendNCPSubData + 3) = 0x11;        // Desired Access Rights
	*(SendNCPSubData + 4) = 0x1d;        // Length
	strcpy (SendNCPSubData + 5, OpenQueueBuffer);

	NCPRetCode=NCPRequest(Socket,FSInfoPointer);
	DataCopy (FileHandle, ReceiveNCPSubData, 6);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return(NCPRetCode);
}

//------------------------Read File Data (NCP)-----------------------------
WORD ReadFileData (WORD Socket, FSInfo  *FSInfoPointer, BYTE *FileHandle, BYTE *StartByteOffset, BYTE *ReadByte)
{
	int Retcode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_READ_FILE_DATA);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID,(6 + 14));
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x48;        // Function Code 72
	*(SendNCPSubData + 1) = 0x00;        // ????

	DataCopy ((SendNCPSubData + 2), FileHandle, 6);
	*(SendNCPSubData + 8) = *(StartByteOffset + 3);
	// Start Read Offset
	*(SendNCPSubData + 9) = *(StartByteOffset + 2);
	*(SendNCPSubData + 10) = *(StartByteOffset + 1);
	*(SendNCPSubData + 11) = *(StartByteOffset + 0);
	*(SendNCPSubData + 12) = *(ReadByte + 1);
	*(SendNCPSubData + 13) = *(ReadByte + 0);

	Retcode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	if(Retcode != OKAY) return(Retcode);

	*ReadByte     = *(ReceiveNCPSubData + 1);
	*(ReadByte+1) = *(ReceiveNCPSubData + 0); // Bytes Actually Read
	PrintData = ReceiveNCPSubData +2;         // Real Data of Receive
	return (OKAY);
}

//--------------------Attach Queue Server To Queue (NCP)--------------------
WORD AttachQueueServer (WORD Socket, FSInfo *FSInfoPointer, BYTE *QueueID)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_ATTACH_QUEUE_SERVER);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID, 6+8);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length
	*(SendNCPSubData + 2) = 0x05;
	*(SendNCPSubData + 3) = 0x6f;        // Subfunction Code 111
	DataCopy (SendNCPSubData + 4, QueueID, 4);
	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}

//-------------------Close Queue Info File (NCP)-------------------------
WORD CloseQueueInfoFile (WORD Socket,FSInfo *FSInfoPointer, BYTE *FileHandle)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_CLOSE_QUEUEINFO_FILE);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket,FSInfoPointer->PCBPhysicalID,6 + 8);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x42;        // Function Code 66
	*(SendNCPSubData + 1) = 0x00;        // ????
	DataCopy ((SendNCPSubData + 2), FileHandle, 6);

	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);

}

//----------------- Service Queue Job (NCP) ------------------------------
WORD ServiceQueueJob (WORD Socket, PortPCB  *PortPCBPointer, BYTE *QObjectID)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_SERVICE_QUEUE_JOB);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket, PortPCBPointer->pFSInfo->PCBPhysicalID, 6 + 10);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network,PortPCBPointer->pFSInfo->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = PortPCBPointer->pFSInfo->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = PortPCBPointer->pFSInfo->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = PortPCBPointer->pFSInfo->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length
	*(SendNCPSubData + 2) = 0x07;
	*(SendNCPSubData + 3) = 0x71;        // SubFunction Code 113
	DataCopy ((SendNCPSubData + 4), QObjectID, 4);
	*(SendNCPSubData + 8) = 0xff;        // Target Service Type
	*(SendNCPSubData + 9) = 0xff;

	NCPRetCode=NCPRequest(Socket,PortPCBPointer->pFSInfo);
#ifdef NDS_PS
	PortPCBPointer->ClientStation = *ReceiveNCPSubData;
#endif NDS_PS
	DataCopy (PortPCBPointer->JOBNumber, ReceiveNCPSubData + 22, 2);
	DataCopy (PortPCBPointer->FileHandle, ReceiveNCPSubData + 42, 6);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}

//----------------------Read Queue Job Entry (NCP)--------------------------
WORD ReadQueueJobEntry (WORD Socket, PortPCB  *PortPCBPointer, BYTE *QObjectID,BYTE **TextScript)
{
	WORD   NCPRetCode;
	int16  Year;
	BYTE   APM[3];
	BYTE   BannerFlag;
	BYTE   *BannerName, *JobDescription, *JobEntryTime, *FormName;
	PrnBuf *PrintBuffer;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_READ_QUEUE_JOB_ENTRY);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket, PortPCBPointer->pFSInfo->PCBPhysicalID,6 + 10);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, PortPCBPointer->pFSInfo->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = PortPCBPointer->pFSInfo->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = PortPCBPointer->pFSInfo->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = PortPCBPointer->pFSInfo->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length
	*(SendNCPSubData + 2) = 0x07;
	*(SendNCPSubData + 3) = 0x6c;        // SubFunction Code 108
	DataCopy ((SendNCPSubData + 4), QObjectID, 4);
	DataCopy ((SendNCPSubData + 8), PortPCBPointer->JOBNumber, 2);  // Job Number

	NCPRetCode = NCPRequest(Socket,PortPCBPointer->pFSInfo);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	if(NCPRetCode != OKAY) return (NCPRetCode);

	//12/8/99 added
	if(TextScript != NULL && *TextScript != NULL) {
		*TextScript = (ReceiveNCPSubData + 54);
		return (OKAY);
	}


	//0x08: NWPS_SUPPRESS_FF
	//0x10: NWPS_NOTIFY_USER
	//0x40: NWPS_TEXT_MODE
	//0x80: NWPS_PRINT_BANNER
	BannerFlag = (BYTE) *(ReceiveNCPSubData + 104 + 5); // Get Banner Control Flag

#ifdef NDS_PS
	PortPCBPointer->Copies = WordSwap(NGET16(ReceiveNCPSubData + 104 + 2));
	if(!(BannerFlag&0x10)) PortPCBPointer->ClientStation = 0x00;
#endif NDS_PS

	if((BannerFlag & 0x80)) {

		JobEntryTime = (ReceiveNCPSubData + 16);
		BannerName   = (ReceiveNCPSubData + 104 + 32);
		FormName     = (ReceiveNCPSubData + 104 + 10);
		JobDescription = (ReceiveNCPSubData + 54);

		//JobEntryTime[0]: year       , JobEntryTime[1]: month (1-12)
		//JobEntryTime[2]: day  (1-31), JobEntryTime[3]: hour  (0-23)
		//JobEntryTime[4]: min  (0-59), JobEntryTime[5]: sec   (0-59)
		if(JobEntryTime[0] > 90) // Year
			Year = 1900 + JobEntryTime[0];
		else
			Year = 2000 + JobEntryTime[0];

		APM[1] = 'M';
		APM[2] = '\0';
		if(JobEntryTime[3] >= 12 ) {
			// Hour & A/P M
			JobEntryTime[3] -= 12 ;	//Hour
			if(JobEntryTime[3] == 0) JobEntryTime[3] = 12;
			APM[0] = 'P'; //APM[] = "PM";
		}
		else {
			//Hour = JobEntryTime[3] ; //Hour
			if(JobEntryTime[3] == 0) JobEntryTime[3] = 12;
			APM[0] = 'A'; //APM[] = "AM";
		}

#ifdef PC_OUTPUT
		if(PrnGetAvailQueueNO(CurServicePort) < 2) {
			AtSaySpace(0,20,40);
			printf("\a \a \a(Netware) Queue < 2 Design Error \a \a \a");
			ErrorBeep();
		}
#endif
		//Print Banner Title
		PrintBuffer = PrnGetInQueueBuf(CurServicePort);
#ifdef PC_OUTPUT
		while(PrintBuffer == NULL) {
			AtSaySpace(0,20,40);
			printf("\a \a \a(Netware) Queue1 Design Error \a \a \a");
			ErrorBeep();
		}
#endif
		//month/day/Year, Hour:Min:Sec, (PM|AM)
		sprintf(PrintBuffer->data, BannerData,
		        BannerName, FormName, JobDescription,
		        JobEntryTime[1], JobEntryTime[2],
		        Year, JobEntryTime[3], JobEntryTime[4],
		        JobEntryTime[5], APM );
		PrintBuffer->size = strlen(PrintBuffer->data);
		PrnPutOutQueueBuf(CurServicePort,PrintBuffer,PRN_Q_NORMAL);

		//Print Large Banner Name
		PrintBuffer = PrnGetInQueueBuf(CurServicePort);
#ifdef PC_OUTPUT
		while(PrintBuffer == NULL) {
			AtSaySpace(0,20,40);
			printf("\a \a \a (Netware) Queue2 Design Error \a \a \a");
			ErrorBeep();
		}
#endif
		print_large(PrintBuffer->data,BannerName,1); //5/14/99 change
		PrintBuffer->size = strlen(PrintBuffer->data);
		PrnPutOutQueueBuf(CurServicePort,PrintBuffer,PRN_Q_NORMAL);

		kwait(0);
	}
	return(NCPRetCode);
}

/*///////// Simon 2/25/98 //////////
//--------------------Get Bindery Object Name (NCP)------------------------
WORD GetBinderyObjectName (WORD Socket, FSInfo *FSInfoPointer, BYTE *QueueObjectID)
{
	WORD   NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_GET_BINDERY_OBJ_NAME);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID,sizeof(NCPQueryData) + 8);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length
	*(SendNCPSubData + 2) = 0x05;
	*(SendNCPSubData + 3) = 0x36;        // SubFunction Code 54
	DataCopy((SendNCPSubData + 4), QueueObjectID, 4);

	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}
*////////// Simon 2/25/98 //////////

/*
//-----------------------Get Queue Job File Size (NCP)-----------------------
WORD GetQueueJobFileSize (WORD Socket, PortPCB *PortPCBPointer, BYTE *JobNumber)
{
	WORD   NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_GET_QUEUE_FILE_SIZE);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket,
	              PortPCBPointer->pFSInfo->PCBPhysicalID,
	              (sizeof(NCPQueryData) + 10)
	);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, PortPCBPointer->pFSInfo->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = PortPCBPointer->pFSInfo->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = PortPCBPointer->pFSInfo->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = PortPCBPointer->pFSInfo->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;        // SubFunction Length
	*(SendNCPSubData + 2) = 0x07;
	*(SendNCPSubData + 3) = 0x78;        // SubFunction Code 120

	DataCopy ((SendNCPSubData + 4), PortPCBPointer->PCBQueueInfo[0].QueueObjectID, 4);
	DataCopy ((SendNCPSubData + 8), JobNumber, 2); // Job Number

	NCPRetCode = NCPRequest(Socket,PortPCBPointer->pFSInfo);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}
*/

//--------------------Finish Servicing Queue Job (NCP)----------------------
WORD FinishServiceQueueJob (WORD Socket, FSInfo  *FSInfoPointer, BYTE *JobNumber, BYTE *QObjectID)
{
	WORD   NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_FINISH_QUEUE_JOB);
#endif NDS_PS

  	// Setup ECB Block
	SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID,6 + 14);
	// Setup IPX Header
	SendPsIPXHeader.packetType        = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17; // Function Code 23
	*(SendNCPSubData + 1) = 0x00; // SubFunction Length
	*(SendNCPSubData + 2) = 0x0b;
	*(SendNCPSubData + 3) = 0x72; // SubFunction Code 114

	DataCopy((SendNCPSubData + 4), QObjectID, 4);
	DataCopy((SendNCPSubData + 8), JobNumber, 2);

	// Job Number
	*(SendNCPSubData + 10) = 0x00;       // Charge Informatiom (Word)
	*(SendNCPSubData + 11) = 0x00;
	*(SendNCPSubData + 12) = 0x00;
	*(SendNCPSubData + 13) = 0x00;

	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}

#ifdef NDS_PS

void ProcessLoginKey (BYTE * BinderyObjectID)
{
	BYTE *buf = SendPsIPXData;

	shuffle(BinderyObjectID,_NovellPassword, strlen(_NovellPassword), buf);
	nw_encrypt(LoginKeyID, buf, LoginKeyID);
}

#else

//---------------- Process Login Key Number --------------------------
BYTE Table1ACC[]=
{
	0x07,0x08,0x00,0x08,0x06,0x04,0x0E,0x04,0x05,0x0C,0x01,0x07,0x0B,0x0F,0x0A,0x08,
	0x0F,0x08,0x0C,0x0C,0x09,0x04,0x01,0x0E,0x04,0x06,0x02,0x04,0x00,0x0A,0x0B,0x09,
	0x02,0x0F,0x0B,0x01,0x0D,0x02,0x01,0x09,0x05,0x0E,0x07,0x00,0x00,0x02,0x06,0x06,
	0x00,0x07,0x03,0x08,0x02,0x09,0x03,0x0F,0x07,0x0F,0x0C,0x0F,0x06,0x04,0x0A,0x00,
	0x02,0x03,0x0A,0x0B,0x0D,0x08,0x03,0x0A,0x01,0x07,0x0C,0x0F,0x01,0x08,0x09,0x0D,
	0x09,0x01,0x09,0x04,0x0E,0x04,0x0C,0x05,0x05,0x0C,0x08,0x0B,0x02,0x03,0x09,0x0E,
	0x07,0x07,0x06,0x09,0x0E,0x0F,0x0C,0x08,0x0D,0x01,0x0A,0x06,0x0E,0x0D,0x00,0x07,
	0x07,0x0A,0x00,0x01,0x0F,0x05,0x04,0x0B,0x07,0x0B,0x0E,0x0C,0x09,0x05,0x0D,0x01,
	0x0B,0x0D,0x01,0x03,0x05,0x0D,0x0E,0x06,0x03,0x00,0x0B,0x0B,0x0F,0x03,0x06,0x04,
	0x09,0x0D,0x0A,0x03,0x01,0x04,0x09,0x04,0x08,0x03,0x0B,0x0E,0x05,0x00,0x05,0x02,
	0x0C,0x0B,0x0D,0x05,0x0D,0x05,0x0D,0x02,0x0D,0x09,0x0A,0x0C,0x0A,0x00,0x0B,0x03,
	0x05,0x03,0x06,0x09,0x05,0x01,0x0E,0x0E,0x00,0x0E,0x08,0x02,0x0D,0x02,0x02,0x00,
	0x04,0x0F,0x08,0x05,0x09,0x06,0x08,0x06,0x0B,0x0A,0x0B,0x0F,0x00,0x07,0x02,0x08,
	0x0C,0x07,0x03,0x0A,0x01,0x04,0x02,0x05,0x0F,0x07,0x0A,0x0C,0x0E,0x05,0x09,0x03,
	0x0E,0x07,0x01,0x02,0x0E,0x01,0x0F,0x04,0x0A,0x06,0x0C,0x06,0x0F,0x04,0x03,0x00,
	0x0C,0x00,0x03,0x06,0x0F,0x08,0x07,0x0B,0x02,0x0D,0x0C,0x06,0x0A,0x0A,0x08,0x0D
};

BYTE Table1BCC[]=
{
	0x48,0x93,0x46,0x67,0x98,0x3D,0xE6,0x8D,0xB7,0x10,0x7A,0x26,0x5A,0xB9,0xB1,0x35,
	0x6B,0x0F,0xD5,0x70,0xAE,0xFB,0xAD,0x11,0xF4,0x47,0xDC,0xA7,0xEC,0xCF,0x50,0xC0,
	0x0F,0x08,0x05,0x07,0x0C,0x02,0x0E,0x09,0x00,0x01,0x06,0x0D,0x03,0x04,0x0B,0x0A,
	0x02,0x0C,0x0E,0x06,0x0F,0x00,0x01,0x08,0x0D,0x03,0x0A,0x04,0x09,0x0B,0x05,0x07,
	0x05,0x02,0x09,0x0F,0x0C,0x04,0x0D,0x00,0x0E,0x0A,0x06,0x08,0x0B,0x01,0x03,0x07,
	0x0F,0x0D,0x02,0x06,0x07,0x08,0x05,0x09,0x00,0x04,0x0C,0x03,0x01,0x0A,0x0B,0x0E,
	0x05,0x0E,0x02,0x0B,0x0D,0x0A,0x07,0x00,0x08,0x06,0x04,0x01,0x0F,0x0C,0x03,0x09,
	0x08,0x02,0x0F,0x0A,0x05,0x09,0x06,0x0C,0x00,0x0B,0x01,0x0D,0x07,0x03,0x04,0x0E,
	0x0E,0x08,0x00,0x09,0x04,0x0B,0x02,0x07,0x0C,0x03,0x0A,0x05,0x0D,0x01,0x06,0x0F,
	0x01,0x04,0x08,0x0A,0x0D,0x0B,0x07,0x0E,0x05,0x0F,0x03,0x09,0x00,0x02,0x06,0x0C,
	0x05,0x03,0x0C,0x08,0x0B,0x02,0x0E,0x0A,0x04,0x01,0x0D,0x00,0x06,0x07,0x0F,0x09,
	0x06,0x00,0x0B,0x0E,0x0D,0x04,0x0C,0x0F,0x07,0x02,0x08,0x0A,0x01,0x05,0x03,0x09,
	0x0B,0x05,0x0A,0x0E,0x0F,0x01,0x0C,0x00,0x06,0x04,0x02,0x09,0x03,0x0D,0x07,0x08,
	0x07,0x02,0x0A,0x00,0x0E,0x08,0x0F,0x04,0x0C,0x0B,0x09,0x01,0x05,0x0D,0x03,0x06,
	0x07,0x04,0x0F,0x09,0x05,0x01,0x0C,0x0B,0x00,0x03,0x08,0x0E,0x02,0x0A,0x06,0x0D,
	0x09,0x04,0x08,0x00,0x0A,0x03,0x01,0x0C,0x05,0x0F,0x07,0x02,0x0B,0x0E,0x06,0x0D
};

char search_buf_014B[5];
BYTE PSID[4];

void ProcessLoginKey (BYTE * GetBinObjectID)
{
	BYTE RandomBuf_1E6A[0x10];  // 1e6a-1e79
	BYTE RandomBuf_1E7A[8];     // 1e6a-1e79
	WORD Asearch_counter;
	BYTE *ScanPoint;

	ScanPoint = search_buf_014B + 1;
	Asearch_counter = 0;
	EnCode_063A(GetBinObjectID, ScanPoint, Asearch_counter, RandomBuf_1E6A);
	Encode_05B9(LoginKeyID/*(in)*/, RandomBuf_1E6A, LoginKeyID/*(out)*/);
}

void EnCode_063A (BYTE *IDOrNum, BYTE *SearchPointer, WORD Counter, BYTE *PointerLast)
{
	BYTE *NotZero_bp_26;
	BYTE *Local_IDOrNum;
	BYTE *CreatNum;
	BYTE TempBuffer[0x26];
	WORD LoopCount_02,Index;

	Local_IDOrNum = IDOrNum;
	NotZero_bp_26 = SearchPointer + Counter;

	NotZero_bp_26--;
	while (*NotZero_bp_26 == 0)
	{
		if (Counter == 0)
			break;
		else
			Counter--;
		NotZero_bp_26--;
	}
	CreatNum = TempBuffer;
	ClearMemory(TempBuffer, 0x20);
	while (Counter >= 20) {
		// Counter =[BP+0E]
		// 067A
		LoopCount_02 = 0x00;
		while (LoopCount_02 < 0x20) {
			*(TempBuffer + LoopCount_02) = (*SearchPointer)^*(TempBuffer + LoopCount_02);
			LoopCount_02++;
			SearchPointer++;
		}
		break;
	}
	// 06AC
	NotZero_bp_26 = SearchPointer;
	if(Counter > 0) {
		// 06B5
		LoopCount_02 = 0x00;
		while(LoopCount_02 < 0x20) {
			// 06BC
			if((SearchPointer + Counter) == NotZero_bp_26) {
				// 06CF
				NotZero_bp_26 = SearchPointer;
				*(CreatNum + LoopCount_02) =
				  (*(CreatNum + LoopCount_02) ^ Table1BCC[LoopCount_02]);
			}
			else {
				// 06EA
				*(CreatNum + LoopCount_02) =
				  (*(CreatNum + LoopCount_02) ^ *NotZero_bp_26);
				NotZero_bp_26++;
			}
			LoopCount_02++;
		}// while ( LoopCount_02<0x20)
	}// if (Counter>0) end

	LoopCount_02 = 0x00;    // begin 0705
	while (LoopCount_02 < 0x20) {
		// 070C
		Index = LoopCount_02;
		Index = (Index & 0x0003);
		*(CreatNum + LoopCount_02) =
		             (*(CreatNum + LoopCount_02) ^ *(Local_IDOrNum + Index));
		LoopCount_02++;
	}//while ( LoopCount_02<0x20)

	EnCodeAgain_0743(TempBuffer, PointerLast);
	// begin 072D
}

void EnCodeAgain_0743 (int8 *MakeBuffer, int8 *PointerLast)
{
	BYTE TempIndex_01,TableIndex,Index,PointerLastIndex;
	BYTE i;
	BYTE XORNum,testnum0,testnum1,testnum2,testnum3,testnumx;

	TempIndex_01 = 0;
	for (i = 0 ;i < 2;i++) {
		for (Index = 0;Index < 0x20;Index++) {
#ifdef DEBUG
			testnum0 = *(MakeBuffer + Index) + TempIndex_01;
			testnum1 = (TempIndex_01 + Index)&0x1F;
			testnum2 = *(MakeBuffer + testnum1);
			testnum3 = testnum2 - Table1BCC[Index];
			testnumx = testnum0^testnum3;
#endif
			XORNum = ((*(MakeBuffer + Index) + TempIndex_01)^(*(MakeBuffer + ((TempIndex_01 + Index)&0x1F)) - Table1BCC[Index]));

			TempIndex_01 = TempIndex_01 + XORNum;
			*(MakeBuffer + Index) = XORNum;
		}
	}
	//---------------------------------------------------------------------
	ClearMemory(PointerLast, 0x10);
	for (Index = 0;Index < 0x20;Index++) {
		TableIndex = *(MakeBuffer + Index);
		PointerLastIndex = Index >> 1;
		if(Index & 0x01) //ODD
			*(PointerLast + PointerLastIndex) = ((*(PointerLast + PointerLastIndex)) | (Table1ACC[TableIndex] << 4));
		else			 //EVEN
			*(PointerLast + PointerLastIndex) = ((*(PointerLast + PointerLastIndex)) | Table1ACC[TableIndex]);
	}
}

void Encode_05B9 (BYTE *SourceBuff, BYTE *ScanBuf, BYTE *DestBuff)
{
	BYTE RandomBuf_1E34[0x20];  // 1e34-1e53
	int16 i,Asearch_counter;

	Asearch_counter = 0x10;
	EnCode_063A(SourceBuff,   ScanBuf, Asearch_counter, RandomBuf_1E34);
	EnCode_063A(SourceBuff+4, ScanBuf, Asearch_counter, RandomBuf_1E34+0x10);
	for(i = 0;i < 0x10;i++)
		RandomBuf_1E34[i] = (RandomBuf_1E34[i]^RandomBuf_1E34[0x1F - i]);
	for (i = 0;i < 8;i++)
		*(DestBuff + i) = (RandomBuf_1E34[i]^RandomBuf_1E34[0xF - i]);
}

void ClearMemory (BYTE *CMpointer, WORD CMcounter)
{
	WORD i;

	for (i = 0;i < CMcounter;i++) {
		*CMpointer = 0;
		CMpointer++;
	}
}

#endif !NDS_PS

#endif NOVELL_PS //--//--//--//--//--//--//--//--//--//--//--//--//--//--//

//-------------------Request Service Connection (NCP)----------------------
WORD RequestCreateServiceConnect (WORD Socket, FSInfo *FSInfoPointer)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_CREATE_SERVICE_CONN);
#endif NDS_PS

	FSInfoPointer->PCBSequenceNumber=0;

	// Setup ECB Block
    SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID,6);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType = 0x1111;   // Create a Service Connection
	SendNCPData->SequenceNumber = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow = 0xff;

	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

	FSInfoPointer->PCBConnectionNumberLow = ReceiveNCPData->ConnectionNumberLow;
	FSInfoPointer->PCBConnectionNumberHigh= ReceiveNCPData->ConnectionNumberHigh;

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return(NCPRetCode);
}

//---------------------Request Buffer Size (NCP)--------------------------
WORD RequestNegotiateBufferSize (WORD Socket, FSInfo  *FSInfoPointer)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_NEGO_BUFFER_SIZE);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(Socket,
	              FSInfoPointer->PCBPhysicalID,
	              6 + 3
	);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;    // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup SAP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x00;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x21;
	*(SendNCPSubData + 1) = 0x04;		//1024 bytes 8/3/99 changed
	*(SendNCPSubData + 2) = 0x00;

	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}

//-------- NCP REQUEST ----------------------------------------------------
WORD NCPRequest (WORD Socket, FSInfo *FSInfoPointer)
{
	WORD   ccode = OKAY;
	BYTE   RetryCount;
	BYTE   ServerResponse;
	BYTE   WaitResponseTime = NCP_RETRY_TIME;

	ListenPsECB(Socket, sizeof(ReceivePsIPXData));

	RetryCount = 0;
	ServerResponse = TIME_OUT;

	while(ServerResponse == TIME_OUT &&
	      RetryCount < MAX_RETRY_COUNT)
	{
		do{
			IPXSendPacket(&SendPsECB);
		} while(SendPsECB.inUseFlag) ;

		do {
			if((ServerResponse = CheckInUseFlag(WaitResponseTime)) == CODE_FREE
			    && ReceivePsECB.completionCode == SUCCESSFUL )
			{
				ServerResponse = CheckConnection(FSInfoPointer);
				switch(ServerResponse) {
				case SERVER_OK:
					// check the connection status field
					if((ReceiveNCPData->CompletionStatus != SUCCESSFUL )
					    &&(ReceiveNCPData->CompletionStatus != 0x40))
					{
						ccode = ReceiveNCPData->CompletionStatus;
/*//////////////////////////////////////
						// if status indicator is set to
						// DISCONNECT sets the g_LANFlag to
						// SERVER_ABORT
						if(ccode == NCPSTATUS_DISCONNECT) {
							;
						}
						else
							// if status indicator is set to
							// SHUTDOWN_BAD sets the g_LANFlag to
							// SHUT_DOWN
							if(ccode == NCPSTATUS_SHUTDOWN_BAD) {
								;
							}
*///////////////////////////////////////
					}
					else {
						ccode = ReceiveNCPData->CompletionCode;
					}
					break;
				case SERVER_BUSY:        //(try listen again)
					// retry after the delay time count
					if(RetryCount != 0) RetryCount --;
					ppause(100);
					++WaitResponseTime;
					// update the receiveTimeOut field
					// in connectionIDTable
					ServerResponse = SERVER_LISTEN_NEXT;
				case SERVER_LISTEN_NEXT: //(try listen again)
					IPXListenForPacket(&ReceivePsECB);
					break;
				case SERVER_ABORT:
					// received SERVER_ABORT packet
					ccode = NCPSTATUS_DISCONNECT;
					break;
				} // switch (result).....
			}
			else {
				//TIME OUT
				ccode = TIME_OUT;
				break; //retry send packet again
			}//if(CheckInUseFlag ....
			kwait(0);
		}while(ServerResponse == SERVER_LISTEN_NEXT);
		RetryCount++;
		kwait(0);
	}//while(... RetryCount < MAX_RETRY_COUNT)....

	if(ccode == TIME_OUT) {
		status = IPXCancelEvent(&ReceivePsECB);
#ifdef PC_OUTPUT
		if(status != SUCCESSFUL) {
			AtSaySpace(40,20,35);
			printf("(Netware) Cancel ECB Fail (3)");
			ErrorBeep();
		}
#endif
	}
	FSInfoPointer->PCBSequenceNumber++;
	return(ccode);
}

//------------------- Destory Server Connection -----------------------
WORD DisConnection (WORD Socket, FSInfo *FSInfoPointer)
{
	WORD  NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_DISCONNECTION);
#endif NDS_PS

	// Setup ECB Block
    SendPsECBInit(Socket, FSInfoPointer->PCBPhysicalID,6);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11; // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType         = 0x5555; // Destory a Service Connection
	SendNCPData->SequenceNumber      = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow = FSInfoPointer->PCBConnectionNumberLow;

	NCPRetCode = NCPRequest(Socket,FSInfoPointer);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return (NCPRetCode);
}

WORD CheckInUseFlag (BYTE WaitResponseTime)
{
	uint32 startime;

	startime = rdclock();
	while(ReceivePsECB.inUseFlag) {
		if((rdclock()-startime) > WaitResponseTime) return(TIME_OUT);
		kwait(0);
	}
	return (CODE_FREE);
}

WORD CheckConnection (FSInfo  *FSInfoPointer)
{
	BYTE i ;
	BYTE NodeMatch = 1; // check if the node address is matched

	for( i = 0 ; i < 6 ; i ++) {
		if(ReceivePsIPXHeader.source.node[i] !=
		   FSInfoPointer->PCBNetworkNumber[i+4]) NodeMatch = 0;
	}

	//Node address, PacketType and socketnumber must Match
	if( NodeMatch && ReceivePsIPXHeader.packetType == NCP_REQUEST &&
		NGET16( ReceivePsIPXHeader.source.socket ) ==
		NGET16( &FSInfoPointer->PCBNetworkNumber[10] ) )
	{
		switch(ReceiveNCPData->ReplyType) {
		case NCP_FILE_SERVER_BUSY:
			return (SERVER_BUSY);
		case NCP_ABORT_CONNECTION:
			return(SERVER_ABORT);
		case NCP_REPLY:
			if(ReceiveNCPData->SequenceNumber ==
			   FSInfoPointer->PCBSequenceNumber)
			{
				return (SERVER_OK);
			}
		}
	}
	return (SERVER_LISTEN_NEXT);
}

void InitSAP (void)
{
	//- Service Advertising Procotol

	// Setup Send-ECB Block
	SendSAPECB.ESRAddress = 0x00;
	SendSAPECB.inUseFlag  = 0;
	SendSAPECB.socketNumber = SAPSocket;
	DataCopy (SendSAPECB.immediateAddress, BrocastNode, 6);

	SendSAPECB.IPXFrameType = GetIPXFrameType(CurrentFrameType); //3/16/98

	SendSAPECB.fragmentCount = 2;
	SendSAPECB.fragmentDescriptor[0].address = &SendSAPIPXHeader;
	SendSAPECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	SendSAPECB.fragmentDescriptor[1].address = &SendSAPIPXData;
	SendSAPECB.fragmentDescriptor[1].size    = 66;

	// Setup IPX Header
	SendSAPIPXHeader.packetType = 4;                // IPX Packet Type
	DataCopy (SendSAPIPXHeader.destination.network, BrocastNetwork, 4);
	DataCopy (SendSAPIPXHeader.destination.node, BrocastNode, 6);
	NSET16( SendSAPIPXHeader.destination.socket, SAPSocket );

	// Setup SAP Protocol
	SendSAPIPXData.ResponseType = 0x0200; // 2 - Type for periodic broadcast
	SendSAPIPXData.ServerType   = MY_SERVER_TYPE; // EP Print Server
	strcpy(SendSAPIPXData.ServerName, SAPName);
	DataCopy(SendSAPIPXData.Node ,MyPhysNodeAddress ,6);

	SendSAPIPXData.Socket = PsSocket;
	SendSAPIPXData.IntermediateNetworks = 0x0100; //  Set to 1
}

#ifndef CODE1
//------------------------ Watch Dog ESR ----------------------------
void ListenWatchDog (void)
{
	// Setup Receive-ECB Block
	ReceiveWDogECB.ESRAddress = WatchDogESR;   // Should be fill WatchDog ESR Address
	ReceiveWDogECB.inUseFlag  = 0;
	ReceiveWDogECB.socketNumber = WatchDogSocket;
	ReceiveWDogECB.fragmentCount = 2;
	ReceiveWDogECB.fragmentDescriptor[0].address = &ReceiveWDogIPXHeader;
	ReceiveWDogECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	ReceiveWDogECB.fragmentDescriptor[1].address = &ReceiveWDogData;
	ReceiveWDogECB.fragmentDescriptor[1].size    = 2;
	IPXListenForPacket (&ReceiveWDogECB);
}

void WatchDogESR (void)
{
	if(ReceiveWDogData.SignatureChar==0x3f) {
		// if true then Response WDog

		SendWDogECB.ESRAddress = 0x00;
		SendWDogECB.inUseFlag  = 0;
		SendWDogECB.socketNumber = WatchDogSocket;
		DataCopy(SendWDogECB.immediateAddress,ReceiveWDogECB.immediateAddress, 6);

		SendWDogECB.IPXFrameType = ReceiveWDogECB.IPXFrameType; //3/16/98

		SendWDogECB.fragmentCount = 2;
		SendWDogECB.fragmentDescriptor[0].address = &SendWDogIPXHeader;
		SendWDogECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
		SendWDogECB.fragmentDescriptor[1].address = &SendWDogData;
		SendWDogECB.fragmentDescriptor[1].size    = 2;
		// Setup IPX Header
		SendWDogIPXHeader.packetType = 0;  // UnKnown Type
		DataCopy(SendWDogIPXHeader.destination.network,ReceiveWDogIPXHeader.source.network,12);

		// Setup WatchDog Protocol
		SendWDogData.ConnectionNumber = ReceiveWDogData.ConnectionNumber;
		SendWDogData.SignatureChar = 0x59; // Response 'Y' (Session is valid)

		// Send IPX Packet
		do{
			IPXSendPacket(&SendWDogECB);
		}while(SendWDogECB.inUseFlag) ;
	}
	ListenWatchDog();  //ReListen WatchDog Packet
#ifdef PC_OUTPUT
	AtSaySpace(20,0,59);
	printf ("\a-------- Reply a WDog Packet ------------");
	kwait(0);
	AtSaySpace(20,0,59);
	printf ("\a-------- Reply a WDog Packet ------------");
#endif
}

#endif !CODE1


#ifdef NOVELL_PS

WORD NPS3main (BYTE nPort)
{
	int16 i;
	BYTE RetCode;
	PortPCB *CurPortPCB;
	WORD CurSocket;

//615wu
	if((!NovellConnectFlag)
#ifdef NDS_PS
		& (!NDSConnectFlag)
#endif
		)
		return (SERVICE_NEXT_PS);

	CurServicePort = nPort;
	if(_IsBmode(nPort) && _bCurPort(nPort) == NULL_BYTE) {
		//This port is available need service other PS (NT or UNIX)
#ifdef PC_OUTPUT
		AtSaySpace(10,nPort*4+2,70);
		printf("(Netware-BIND) No Queue Need Service !");
		AtSaySpace(0,nPort*4+3,40);
		printf("(Netware-BIND) No Queue Need Service !");
#endif
#ifdef NDS_PS
		_ModeToggle(nPort);
#else
		return (SERVICE_NEXT_PS);
#endif NDS_PS
	}

#ifdef NDS_PS
	if(_IsDSmode(nPort)	&& _NDSbCurPort(nPort) == NULL_BYTE) {
#ifdef PC_OUTPUT
		AtSaySpace(10,nPort*4+2,70);
		printf("(Netware-NDS) No Queue Need Service !");
		AtSaySpace(0,nPort*4+3,40);
		printf("(Netware-NDS) No Queue Need Service !");
#endif PC_OUTPUT

		_ModeToggle(nPort);
		return (SERVICE_NEXT_PS);
	}

#ifdef PC_OUTPUT
	if(!_IsDSmode(nPort) && !_IsBmode(nPort)) {
    	while(1) printf("BIND-NDS Mode design Error !\n");
	}
#endif PC_OUTPUT
#endif NDS_PS

#ifdef PC_OUTPUT
	At(0,nPort*4+2);
	printf ("<LPT%d(%04s)>",nPort,_IsBmode(nPort)?"BIND":"NDS");
#endif

	//ProcessRoutine : (1) ServiceNetWareQueue;
	//                 (2) QueueToPort
	if(_IsBmode(nPort))	{
		CurPortPCB= _pCurrentPortPCB(nPort);
		CurSocket = PsSocket;
	}
#ifdef NDS_PS
	else {
		CurPortPCB= _NDSpCurrentPortPCB(nPort);
		CurSocket = NDSPsSocket;
	}
#endif NDS_PS

	if((RetCode = (*CurPortPCB->ProcessRoutine)(CurSocket,CurPortPCB)) != 0) {
		//if RetCode == 0 , still need service this Queue
		switch(RetCode) {
		default:     // NoConnect
#ifdef PC_OUTPUT
			AtSaySpace(0,nPort*4+4,1);
			printf ("(Netware:NPS3main) No Connect!! ");
#endif
		case TIME_OUT:
#ifdef PC_OUTPUT
			AtSaySpace(0,nPort*4+4,1);
			printf ("(Netware-%s) TimeOut !! ",_IsBmode(nPort)?"BIND":"NDS");
#endif
			//This port TimeOut
			//1. Disbale all port with same FS.
			//2. Add this FS to NOActiveFS linklist
			if(_IsBmode(nPort)) {
			    MoveAttachPortToFreePort();
			}
#ifdef NDS_PS
			else {
			    NDSMoveActivePortToFreePort(CurPortPCB->pFSInfo);
			}
#endif NDS_PS

			break;
		case SERVICE_DATA:
			//(1) return from QueueToPort : Job not finish continue service it !
			//(2) return from ServiceNetwareQueue : Get Queue Job OK !
			CurPortPCB->ProcessRoutine = QueueToPort;
            return (SERVICE_DATA);
			break;
		case SERVICE_NEXT_QUEUE:	//return from QueueToPort (Job finish)
		case NO_QUEUE_JOB:		//return from ServiceNetwareQueue (This Queue No Job)
			//queue finish, service next PS
			//(1) Service next queue of Current PCB
			CurPortPCB->ProcessRoutine = ServiceNetWareQueue;
			CurPortPCB->QCount++;
			if(CurPortPCB->QCount >= CurPortPCB->TotalQueue) {
				//All queue of CurrentPortPCB service finish !
				CurPortPCB->QCount = 0;

				//(2) Service Next ActivePCB
				if(CurPortPCB->NextPortPCB == NULL_BYTE) {
					if(_IsBmode(nPort))
						_bCurPort(nPort) = _bActPort(nPort);
#ifdef NDS_PS
					else
					    _NDSbCurPort(nPort) = _NDSbActPort(nPort);

					_ModeToggle(nPort);	//Bindery <--> NDS
#endif NDS_PS
				} else {
					if(_IsBmode(nPort))
						_bCurPort(nPort) = CurPortPCB->NextPortPCB;
#ifdef NDS_PS
					else
					    _NDSbCurPort(nPort) = CurPortPCB->NextPortPCB;
#endif NDS_PS

				}
			}
			return (SERVICE_NEXT_PS);
		case SERVICE_AGAIN:  //5/14/98  query time < polling time
			return (SERVICE_NEXT_PS);
		} //switch
	}//if((RetCode = (*ProcessRoutine) .....
	return (0);
}

void TryConnectNoAttachFS (void)
{
	BYTE bCurFSInfo,bPreFSInfo;
	FSInfo *pCurrentFSInfo;

	//once try connect to one FS
	if(bNoActiveFSInfo != NULL_BYTE) {
#if 0 //615wu
#if (NUM_OF_STATUS_LED == 2)
		Light_On(R_Lite);
#endif
#endif
		ppause(10);
#if 0 //615wu
#if (NUM_OF_STATUS_LED == 2)
		if(HasAnyFileServerConnect() < NUM_OF_PRN_PORT
#ifdef NDS_PS
		   || NDSHasAnyFileServerConnect() < NUM_OF_PRN_PORT
#endif NDS_PS
		) {
			Light_On(G_Lite); //1/10/99
			Light_Off(R_Lite);
		}
		else {
			Light_Off(G_Lite);
		}
#endif
#endif
		pCurrentFSInfo = FSInfoBlock+bNoActiveFSInfo;
		if(AttachToFileServer(PsSocket, pCurrentFSInfo) != OKAY) {
			// Trying Attach Server
#ifdef PC_OUTPUT
			AtSaySpace(0,15,80);
			printf(" Retry again : No Connect !! (TryConnectNoAttachFS)");
//			putchar(0x07);
#endif
		}
		else {
			// Connect OK !!
			//-- Remove this node from NOActiveFSInfo Ring -------------
			bPreFSInfo = bNoActiveFSInfo;
			while((FSInfoBlock+bPreFSInfo)->NextFSInfo != bNoActiveFSInfo)
				bPreFSInfo = (FSInfoBlock+bPreFSInfo)->NextFSInfo;

			if(bPreFSInfo == bNoActiveFSInfo) {
				//Only one node in ring
				bNoActiveFSInfo = NULL_BYTE;
			}
			else {
				//remove this node from ring
				FSInfoBlock[bPreFSInfo].NextFSInfo=pCurrentFSInfo->NextFSInfo;
			}
		}
		//shift bNoActiveFSInfo to next
		if(bNoActiveFSInfo != NULL_BYTE)
			bNoActiveFSInfo = pCurrentFSInfo->NextFSInfo;
	}// if(bCurFSInfo != NULL_BYTE).....
}
//
//File Server TIME_OUT , move all active port of this FS to free port
//
void MoveAttachPortToFreePort (void)
{
//	BYTE *FSName; //Discoonect File Server name	, 11/24/99 remarked no used !!
	BYTE bPrePortPCB, bCurPortPCB;
	PortPCB *pCurPortPCB;
//	FSInfo  *FileServerInfo;  //11/24/99 remarked no used !!
	BYTE nPort,SearchPortOk;
	FSInfo *pTimeOutFSInfo;
	BYTE bCurFSInfo;

	pTimeOutFSInfo = _pCurrentPortPCB(CurServicePort)->pFSInfo;
	SearchPortOk = 0;

	for(nPort=0 ; nPort<NUM_OF_PRN_PORT ; nPort++) {
		bPrePortPCB = NULL_BYTE;
		bCurPortPCB = _bActPort(nPort);
		while(bCurPortPCB != NULL_BYTE) {
			pCurPortPCB= _pPrnPortPCB(nPort,bCurPortPCB);
			if(pTimeOutFSInfo == pCurPortPCB->pFSInfo) {
				//-- Remove Queue from ActivePortPCB -------------
				if(bPrePortPCB == NULL_BYTE) {
					//Remove first PCB
					_bActPort(nPort) = pCurPortPCB->NextPortPCB;
				}
				else {
					//Remove PCB
					_pPrnPortPCB(nPort,bPrePortPCB)->NextPortPCB = pCurPortPCB->NextPortPCB;
				}

				//--- Add Queue to FreePortPCB --------------------
				pCurPortPCB->NextPortPCB = _bFreePort(nPort);
				_bFreePort(nPort) = bCurPortPCB;

				SearchPortOk++;

				//TimeOut Port's FS = Current Port's FS
				if(bCurPortPCB == _bCurPort(nPort))
				{
					//Reset CurrentPortPCB
					_bCurPort(nPort) = _bActPort(nPort);

					//If TimeOut Port is printing ....
					if(PrnGetPrinterStatus(nPort) == NetwareUsed) {
						PrnAbortSpooler(nPort,NULL);
						PrnSetNoUse(nPort);

#ifdef SUPPORT_JOB_LOG 			
 						JL_EndList(CurServicePort, 0);		// George Add January 26, 2007
#endif SUPPORT_JOB_LOG						
					}
				}
				break;
			}//if(strcmp()).......
			//--- Shift to next ActivePortPCB ------------------
#ifdef PC_OUTPUT
			if(bCurPortPCB == NULL_BYTE) {
				AtSaySpace(0,20,80);
				printf("\a \a \abCurPortPCB == NULL_BYTE, design error (MoveAttachPortToFreePort) !\a \a \a");
				ErrorBeep();
			}
#endif
			bPrePortPCB = bCurPortPCB;
			bCurPortPCB = pCurPortPCB->NextPortPCB;
		}//while( )...
	}//for(nPort= .....

	//---------- Add TimeOut's FS to NoActiveFS ---------------------
	if( SearchPortOk != 0 ) {
		//bNOActiveFSInfo is a ring
		bCurFSInfo = pTimeOutFSInfo - FSInfoBlock;
		if(bNoActiveFSInfo == NULL_BYTE) bNoActiveFSInfo = bCurFSInfo;

		pTimeOutFSInfo->NextFSInfo = FSInfoBlock[bNoActiveFSInfo].NextFSInfo;
		FSInfoBlock[bNoActiveFSInfo].NextFSInfo = bCurFSInfo;
		bNoActiveFSInfo = bCurFSInfo;
	}
}
#endif NOVELL_PS

//--------------------- Send PS SAP per 60 second --------------------------
void SendNovellSAP(cyg_addrword_t data)
{
  	for(;;) {
		// Send SAP Packet
#ifdef PC_OUTPUT
{
    	int i;

		AtSaySpace(0,0,80);
		printf("\a");
		for (i = 2 ; i < 20;i++) {
			At(i-1,0);
			printf(" ");
			At(i,0);
			printf(">>>>-- Send SAP Packet ------->>");
			ppause(1L);
		}
}
#endif
		SendSAPECB.IPXFrameType = GetIPXFrameType(CurrentFrameType); //3/16/98

		do {
			IPXSendPacket (&SendSAPECB);
		} while (SendSAPECB.inUseFlag) ;
#ifdef SNMPIPX
//		SendJetAdminSAP();	//6/21/2000
#endif SNMPIPX
        sys_check_stack();
		ppause(60000L);  //sleep 60 second.
	}
}

#ifdef DO_STATUS_PRINT
//---------------Utility Printing Status Data ----------------------
#if defined(O_ELEC)
const BYTE PrintServerData[] ="\
                  ****************************************                     \r\n\
                  *        Print Server Test Page        *                     \r\n\
                  ****************************************                     \r\n\
\r\n\
\r\n\
     IP Address: %u.%u.%u.%u                                                   \r\n\
\r\n\
     Subnet Mask: %u.%u.%u.%u                                                  \r\n\
\r\n\
     Gateway IP: %u.%u.%u.%u                                                   \r\n\
\r\n\
\r\n\
     Protocol: TCP/IP                                                          \r\n\
\r\n\
         LPR Printing: %-9s                                                    \r\n\
\r\n\
         Print Monitor Printing: %-9s                                          \r\n\
\r\n\
         IPP Printing: %-9s                                                    \r\n\
\r\n\
         SMB: %-9s                                                             \r\n\
\r\n\
             Workgroup Name: %-17s                                             \r\n\
\r\n\
             Shared Printer Name: %-14s                                        \r\n\
\r\n\
\r\n\
     Print Speed: %-7s                                                         \r\n\
\r\n\
     Device Name: %-19s                                                        \r\n\
\r\n\
     MAC Address: %02X-%02X-%02X-%02X-%02X-%02X                                \r\n\
\r\n\
     Firmware Version: %02X.%02X.%02d%s build %04x                             \r\n\
\r\n\r\n%c";
#else
#ifdef WIRELESS_CARD  // for wireless 
const BYTE PrintServerData[] ="\
                  ****************************************                     \r\n\
                  *        Print Server Test Page        *                     \r\n\
                  ****************************************                     \r\n\
\r\n\
\r\n\
     Device Name: %-19s                    									\r\n\
\r\n\
     Firmware Version: %-20s												\r\n\
\r\n\
     MAC Address: %02X-%02X-%02X-%02X-%02X-%02X 							\r\n\
\r\n\
     IP Address: %u.%u.%u.%u 												\r\n\
\r\n\
     Subnet Mask: %u.%u.%u.%u 												\r\n\
\r\n\
     Gateway IP: %u.%u.%u.%u 												\r\n\
\r\n\
     AppleTalk Status: %-9s                               					\r\n\
\r\n\
     AppleTalk Port Name: %-13s                            					\r\n\
\r\n\
     Wireless Status														\r\n\
\r\n\
     	Mode:                 %s\r\n\
\r\n\
        ESSID:                %s\r\n\
\r\n\
        Channel:              %d\r\n\
\r\n\
        AP MAC Address:       %02X-%02X-%02X-%02X-%02X-%02X\r\n\
\r\n\
        Authentication:       %s\r\n\
\r\n\
        Encryption:           %s\r\n\
\r\n\r\n%c";
#else
const BYTE PrintServerData[] ="\
                  ****************************************                     \r\n\
                  *        Print Server Test Page        *                     \r\n\
                  ****************************************                     \r\n\
\r\n\
\r\n\
     Device Name: %-19s                    									\r\n\
\r\n\
     Firmware Version: %-20s												\r\n\
\r\n\
     MAC Address: %02X-%02X-%02X-%02X-%02X-%02X 							\r\n\
\r\n\
     IP Address: %u.%u.%u.%u 												\r\n\
\r\n\
     Subnet Mask: %u.%u.%u.%u 												\r\n\
\r\n\
     Gateway IP: %u.%u.%u.%u 												\r\n\
\r\n\
     AppleTalk Status: %-9s                               					\r\n\
\r\n\
     AppleTalk Port Name: %-13s                            					\r\n\
\r\n\r\n%c";
#endif  // defined(RT8188)
#endif	// !defined(O_ELEC) && !defined(O_ZOTCH) && !defined(O_INTELB)

extern UINT8   mvESSID[33];
const BYTE EPSONData[]="\x00\x00\x00\x1b\x01@EJL 1284.4\x0a@EJL     \x0a\x1b@\x1b@\x1b";
//Send Print Wireless status data to print queue !
BYTE SendStatusData (int Port)
{
	PrnBuf *PrintBuffer;
	BYTE PrintStatus;
	BYTE *pTemp;
	BYTE *IPAddr;
	BYTE *Mask;
	BYTE *GatewayIP;
	BYTE Protocol;
	BYTE PrintSpeed;
	BYTE dataOffset = 0;
	uint16 dataLeng = 0;
	BYTE EPSON_String[5] = {0x45, 0x50, 0x53, 0x4F, 0x4E};	// EPSON

	static char *TestPageStrings[5] = {"Disabled", "Enabled",
								"Fast", "Normal", "Slow"};

	BYTE szBoxName[LENGTH_OF_BOX_NAME+1];	// George added this at build0005 of DWP2020 on June 21, 2012.
	BYTE szVersion[20];

#ifdef WIRELESS_CARD	 // for wireless
	BYTE tempbuffer[64]={0};
	unsigned char bssid[WLAN_BSSID_LEN]={0};
	char *cp=NULL, *auth_type_str, *encryption_str;
#endif 

	memset(szBoxName, 0x00, sizeof(szBoxName));
	memset(szVersion, 0x20, sizeof(szVersion));

//WIRELESS_CARD
//	BYTE tempbuffer[64]={0};
//	knownbss_t *curr;
//	char *cp,*cap_str;
//-----------
//	curr = wlan_get_scanlist();
	
	if((PrintStatus = _PrintPortStatus(Port)) != 0) return (PrintStatus);

	if(PrnGetPrinterStatus(Port) != PrnNoUsed) return (PRINTER_BUSY);
	if(PrnGetAvailQueueNO(Port)  < 1) return (PRINTER_BUSY);
	
	PrnSetTestPageInUse(Port);

	pTemp = (BYTE *)malloc(2048);
	if(pTemp == NULL)
		return 0;
	memset(pTemp, 0, 2048);
	//Print Test Data 1
	PrintBuffer = PrnGetInQueueBuf(Port);
	
//	if (PortIO[Port].Manufacture == "EPSON"){
	if ( memcmp(PortIO[Port].Manufacture, EPSON_String, 5) == 0 ){
		memcpy(PrintBuffer->data, EPSONData,32);
		dataOffset = 32;	//for epson print
	} else
		dataOffset = 0;

	strcpy(szBoxName, EEPROM_Data.BoxName);
	szBoxName[LENGTH_OF_BOX_NAME] = 0x00;

	IPAddr = _BoxIPAddress;
	Mask = _BoxSubNetMask;
	GatewayIP = _BoxGatewayAddress;
	Protocol = EEPROM_Data.PrintServerMode;
	PrintSpeed = EEPROM_Data.PrinterSpeed;

	//sprintf(szVersion,"%d.%02X.%02d%c build %04d",
	//		CURRENT_MAJOR_VER, CURRENT_MINOR_VER, CURRENT_PS_MODEL, WebLangVersion, CURRENT_BUILD_VER);

	// Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
	sprintf(szVersion,"%d.%02X.%02d%s build %04d",
			CURRENT_MAJOR_VER, CURRENT_MINOR_VER, CURRENT_PS_MODEL, &WebLangVersion, CURRENT_BUILD_VER);

#ifdef WIRELESS_CARD // for wireless
	
	wlan_get_currbssid(bssid);
	//wlan_get_currssid(tempbuffer);
	memcpy(tempbuffer, mvESSID, 32 );
	switch(mvAuthenticationType)
	{
		case 1:
			auth_type_str = "Open system";
			break;
		case 2:
			auth_type_str = "Share Key";
			break;
		case 3:
			auth_type_str = "Both use";
			break;
		case 4:
			auth_type_str = "WPA-PSK";
			break;
		case 5:
			auth_type_str = "WPA2-PSK";
			break;
		default:
			auth_type_str = "Open system";
			break;
	}
	if( (mvAuthenticationType >= 4) && (mvAuthenticationType <= 5) )
	{
		if( mvWPAType == 1)
			encryption_str = "AES_CCMP";
		else 	
			encryption_str = "TKIP";
	}
	else
	{
		if( mvWEPType  == 1)
			encryption_str = "WEP-64bit";
		else if( mvWEPType  == 2)
			encryption_str = "WEP-128bit";
		else
			encryption_str = "Disable";		
	}
#endif //defined(RT8188)

#if defined(O_ELEC)
	sprintf(pTemp, PrintServerData,
			IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3],
			Mask[0],Mask[1],Mask[2],Mask[3],
			GatewayIP[0],GatewayIP[1],GatewayIP[2],GatewayIP[3],
			((Protocol & PS_UNIX_MODE) == PS_UNIX_MODE)?TestPageStrings[1]:TestPageStrings[0],
			((Protocol & PS_WINDOWS_MODE) == PS_WINDOWS_MODE)?TestPageStrings[1]:TestPageStrings[0],
			((Protocol & PS_IPP_MODE) == PS_IPP_MODE)?TestPageStrings[1]:TestPageStrings[0],
			((Protocol & PS_SMB_MODE) == PS_SMB_MODE)?TestPageStrings[1]:TestPageStrings[0],
			EEPROM_Data.WorkGroupName,
			EEPROM_Data.ServiceName[0],
			(PrintSpeed == 0x00)?TestPageStrings[2]:
			(PrintSpeed == 0x01)?TestPageStrings[3]:TestPageStrings[4],
			szBoxName,				//Device Name
			MyPhysNodeAddress[0],MyPhysNodeAddress[1],MyPhysNodeAddress[2],	//Node ID
	        MyPhysNodeAddress[3],MyPhysNodeAddress[4],MyPhysNodeAddress[5],
	        //CURRENT_MAJOR_VER, CURRENT_MINOR_VER,CURRENT_PS_MODEL,WebLangVersion,CURRENT_BUILD_VER, //Version
	        // Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
	        CURRENT_MAJOR_VER, CURRENT_MINOR_VER,CURRENT_PS_MODEL,&WebLangVersion,CURRENT_BUILD_VER, //Version
			0x0C);
#else
	sprintf(pTemp, PrintServerData,
 	        szBoxName,				//Device Name
	        szVersion,		//Version
	        MyPhysNodeAddress[0],MyPhysNodeAddress[1],MyPhysNodeAddress[2],	//Node ID
	        MyPhysNodeAddress[3],MyPhysNodeAddress[4],MyPhysNodeAddress[5],
	        IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3],
			Mask[0],Mask[1],Mask[2],Mask[3],
			GatewayIP[0],GatewayIP[1],GatewayIP[2],GatewayIP[3],
			((Protocol & PS_ATALK_MODE) == PS_ATALK_MODE)?TestPageStrings[1]:TestPageStrings[0],
			_ATalkPortName,
#ifdef WIRELESS_CARD // for wireless 
			(EEPROM_Data.WLMode == 0)?"Infrastructure":"Ad-Hoc",
			tempbuffer,
			wlan_get_channel(),
			bssid[0], bssid[1], bssid[2],
			bssid[3], bssid[4], bssid[5],
			auth_type_str,
			encryption_str,
#endif 
			0x0C);
#endif	// defined(O_ELEC)

	dataLeng = strlen(pTemp);
	memcpy(PrintBuffer->data + dataOffset, pTemp, dataLeng);
//	pTemp = print_large(PrintBuffer->data+strlen(PrintBuffer->data),"P R I N T",0);
//	print_large(pTemp,"T E S T",1);
	free(pTemp);
	
	PrintBuffer->size = dataOffset + dataLeng;
	PrnPutOutQueueBuf(Port,PrintBuffer,PRN_Q_NORMAL);
	
	PrnSetNoUse(Port);
	
	return(PRINTER_FREE);
}

#if defined(O_INTELB)
const BYTE SendEOFData[] ="\
                  *******************************************                  \r\n\
\r\n\
\r\n\
     Nome do Dispositivo: %-19s                                                \r\n\
\r\n\
     Rede de dados indisponivel.                                               \r\n\
\r\n\
     O servidor de impressao nao consegue receber nenhum dado para imprimir.   \r\n\
\r\n\
     Por favor verifique as condicoess da rede ou contate o administrador.     \r\n\
\r\n\r\n%c%c";
#else
const BYTE SendEOFData[] ="\
                  *******************************************                  \r\n\
\r\n\
\r\n\
     Device Name: %-19s                                                        \r\n\
\r\n\
     The network is unstable.                                                  \r\n\
\r\n\
     The print server cannot receive any printing data.                        \r\n\
\r\n\
     Please check your network connection and the environment.                 \r\n\
\r\n\r\n%c%c";
#endif	// defined(O_INTELB)

// Send the EOF character!
BYTE SendEOF (int Port)
{
	PrnBuf *PrintBuffer;
	BYTE PrintStatus;
	BYTE *pTemp;
	BYTE dataOffset = 0;
	uint16 dataLeng = 0;
	BYTE EPSON_String[5] = {0x45, 0x50, 0x53, 0x4F, 0x4E};	// EPSON
	
	BYTE szBoxName[LENGTH_OF_BOX_NAME+1];	// George added this at build0005 of DWP2020 on June 21, 2012.

	memset(szBoxName, 0x00, sizeof(szBoxName));

//	if((PrintStatus = _PrintPortStatus(Port)) != 0) return (PrintStatus);

//	if(PrnGetPrinterStatus(Port) != PrnNoUsed) return (PRINTER_BUSY);
//	if(PrnGetAvailQueueNO(Port)  < 1) return (PRINTER_BUSY);
	
	if ((PrintStatus = PrnReadPortStatus( Port )) == PORT_OFF_LINE)
		return (PRINTER_FREE);
		
	//Print Test Data 1
	while( (PrintBuffer = PrnGetInQueueBuf(Port)) == NULL){
		cyg_thread_yield();
	}

	if(PrnGetPrinterStatus(Port) == PrnNoUsed)	
		PrnSetTestPageInUse(Port);

	pTemp = (BYTE *)malloc(2048);

	if(pTemp == NULL)
		return 0;

	memset(pTemp, 0, 2048);
	
//	if (PortIO[Port].Manufacture == "EPSON"){
	if ( memcmp(PortIO[Port].Manufacture, EPSON_String, 5) == 0 ){
		memcpy(PrintBuffer->data, EPSONData,32);
		dataOffset = 32;	//for epson print
	} else
		dataOffset = 0;

	strcpy(szBoxName, EEPROM_Data.BoxName);
	szBoxName[LENGTH_OF_BOX_NAME] = 0x00;

	sprintf(pTemp, SendEOFData, szBoxName, 0x0D,0x0C);

// ----------------------------------------------------------------------------
	dataLeng = strlen(pTemp);
	memcpy(PrintBuffer->data + dataOffset, pTemp, dataLeng);
//	pTemp = print_large(PrintBuffer->data+strlen(PrintBuffer->data),"P R I N T",0);
//	print_large(pTemp,"T E S T",1);
	free(pTemp);

//	PrintBuffer->size = strlen(PrintBuffer->data);
	PrintBuffer->size = dataOffset + dataLeng;
	PrnPutOutQueueBuf(Port,PrintBuffer,PRN_Q_NORMAL);
	
	if(PrnGetPrinterStatus(Port) == TestPageUsed)		
	PrnSetNoUse(Port);
	
	return(PRINTER_FREE);
}

#endif//DO_STATUS_PRINT

#ifndef CODE1

#if defined(WIRELESS_STATUS_PRINT)
//---------------Utility Printing Wireless Status Data ----------------------
const BYTE WirelessData[] ="\
******************************************************************************\r\n\
     Device Name: %-18s       Node ID: %02X-%02X-%02X-%02X-%02X-%02X\r\n\
\r\n\
     Flash Version: %02X.%02X                  Release Version: %04d\r\n\
\r\n\
\r\n\
     ESSID: %-32s                       							\r\n\
\r\n\
     BSSID: %02X-%02X-%02X-%02X-%02X-%02X               			\r\n\
\r\n\
     Network Type: %16s                       Channel: %02d		\r\n\
******************************************************************************\r\n\r\n%c";

const BYTE WirelessTestPageData[] ="\
                  ***************************************                    \r\n\
                  *      Test Page                      *                    \r\n\
                  ***************************************                    \r\n\
\r\n\
\r\n\
 ________________________________________________________________________ \r\n";

#ifdef O_DLINK
const BYTE PrintServerData[] ="\
                  ***************************************                     \r\n\
                  *       Print  Server Test Page       *                     \r\n\
                  ***************************************                     \r\n\
\r\n\
\r\n\
     Device Name: %-18s                    \r\n\
\r\n\
   Firmware Version: %02X.%02X      \r\n\
\r\n\
     MAC Address: %02X-%02X-%02X-%02X-%02X-%02X \r\n\
\r\n\
      IP Address: %d.%d.%d.%d \r\n\
\r\n\
     Subnet Mask: %d.%d.%d.%d \r\n\
\r\n\r\n%c";
#else
#ifdef O_AXIS
const BYTE PrintServerData[] ="\
                  *********************************************                   \r\n\
                  * AXIS OfficeBasic USB Wireless G Test Page *                   \r\n\
                  *********************************************                   \r\n\
\r\n\
\r\n\
\tDevice Name: %-18s                    \r\n\
\r\n\
\tFirmware Version: %02X.%02X.%02d%s build %04d     \r\n\
\r\n\
\tMAC Address: %02X-%02X-%02X-%02X-%02X-%02X \r\n\
\r\n\
\t%-24s		\r\n\
\r\n\
\t	IP Address: %d.%d.%d.%d \r\n\
\r\n\
\t	Subnet Mask: %d.%d.%d.%d \r\n\
\r\n\
\t	Default Router: %d.%d.%d.%d \r\n\
\r\n\
\t%-24s		\r\n\
\r\n\
\t	Operation Mode: %s     \r\n\
\r\n\
\t	ESSID: %-32s     \r\n\
\r\n\
\t	Wireless Domain: %-28s     \r\n\
\r\n\
\t	Current Channel: %d     \r\n\
\r\n\
\t	Transmit Mode : %s     \r\n\
\r\n\
\t	Signal Strength : %d%%     \r\n\
\r\n\
\t	Link Quality : %d%%     \r\n\
\r\n\
\t	WEP: %s     \r\n\
\r\n\
\t	WPA-PSK: %s     \r\n\
\r\n\
\t	%s: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X     \r\n\
\r\n\
\t%-24s		\r\n\
\r\n\
\t	AppleTalk: %s     \r\n\
\r\n\
\t	AppleTalk Port Name: %-13s \r\n\
\r\n\
\t%-24s		\r\n\
\r\n\
\t	Rendezvous: %s     \r\n\
\r\n\
\t	Rendezvous Service Name: %-64s \r\n\
\r\n\
\tFor assistance, contact your local dealer/distributor.                        \r\n\
\r\n\
\tIn addition to the user documentation found in the product           			\r\n\
\tpackaging, you can find on-line manuals, technical support,           		\r\n\
\tfirmware updates and application software on http://www.axis.com/              \r\n\
\r\n\%c
                  *********************************************                   \r\n\
                  * AXIS OfficeBasic USB Wireless G Test Page *                   \r\n\
                  *********************************************                   \r\n\
\r\n\
\r\n\
 ___________________________________________________________________________ \r\n\
|SSID                             |AP MAC Address    |Chan|WPA-PSK|B/G| RSSI|\r\n\
|                                 |                  | nel|  /WEP |   |     |\r\n\
|_________________________________|__________________|____|_______|___|_____|\r\n\
";
#else
const BYTE PrintServerData[] ="\
                  *******************************************                     \r\n\
                  *         Print  Server Test Page         *                     \r\n\
                  *******************************************                     \r\n\
\r\n\
\r\n\
\tDevice Name: %-18s                    \r\n\
\r\n\
\tFirmware Version: %02X.%02X.%02d build %04x     \r\n\
\r\n\
\tMAC Address: %02X-%02X-%02X-%02X-%02X-%02X \r\n\
\r\n\
\tESSID: %-32s     \r\n\
\r\n\
\tChannel: %d     \r\n\
\r\n\
\tWEP: %s     \r\n\
\r\n\
\tIP Address: %d.%d.%d.%d \r\n\
\r\n\
\tSubnet Mask: %d.%d.%d.%d \r\n\
\r\n\r\n%c";
#endif	// defined(O_AXIS)
#endif O_DLINK

extern uint8    rxStatsRSSI;             /* RF Signal Strength Indicator */
extern uint8    linkQuality;              /* Link Quality */
//temp extern UINT8   mvWDomain; //Ron Add 9/23/04
//Send Print Wireless status data to print queue !
BYTE SendWirelessStatusData (int Port)
{
	PrnBuf *PrintBuffer;
	BYTE PrintStatus;
	BYTE *pTemp;
	BYTE EndText = 0;
	BYTE i = 0, j = 0;
	WORD x = 0, m = 0;
	BYTE *IPAddr;
	BYTE *Mask;
	BYTE *GatewayAddr;
	BYTE WLZone;

	static char *TestPageCaption[4] = {"TCP/IP Settings:", "Wireless Settings:",
								"AppleTalk Settings:", "Rendezvous Settings:"};

#if defined(WIRELESS_CARD)
	BYTE tempbuffer[64]={0},temp_ssid[33]="SSID broadcast disabled";
	BYTE Public_Key[17]={0};
//	knownbss_t *curr;
	APICMDBUF_GET_SURVEY_INFO curr;
	API_SURVEY_ENTRY *ssEntry_p;
	char *cp,*cap_str,*wep_str,*trx_str,*str;
	int RSSI_TEMP = 0;
	
	memset( &curr, 0, sizeof(APICMDBUF_GET_SURVEY_INFO));
	wlan_get_scanlist(&curr);	
#endif		
	
	if((PrintStatus = _PrintPortStatus(Port)) != 0) return (PrintStatus);

	if(PrnGetPrinterStatus(Port) != PrnNoUsed) return (PRINTER_BUSY);
	if(PrnGetAvailQueueNO(Port)  < 1) return (PRINTER_BUSY);
	
	PrnSetTestPageInUse(Port);
	
	pTemp = (BYTE *)malloc(8192);
	if(pTemp == NULL)
		return 0;
	memset(pTemp, 0, 8192);
	//Print Test Data 1
	PrintBuffer = PrnGetInQueueBuf(Port);

	if(EEPROM_Data.SPECIAL_OEM != 0x02){
		
#if !defined(O_AXIS)
		if (diag_flag == 1)
		{	
			// Diagnostic mode

			sprintf(PrintBuffer->data, WirelessTestPageData);
			
			for(j=0; j < curr.maxEntry; )
			{
				ssEntry_p = curr.infoAddr + (j * sizeof(API_SURVEY_ENTRY));
					
				if(ssEntry_p->dirty && ( ssEntry_p->channel > 0) && (ssEntry_p->channel <= 14) )
				{	
					cp = ssEntry_p->SsId;
							
					/* Refine RSSI value ... Ron 9/8/04 */
					//sEntry_p->RSSI = (ssEntry_p->RSSI * 100)/30;
					ssEntry_p->RSSI = ssEntry_p->RSSI * 2;
			 		if (ssEntry_p->RSSI > 100)
			 			ssEntry_p->RSSI = 100;
							
					if( ssEntry_p->IBSS == 0)
						cap_str = "Infrastructure";
			 		else
			 			cap_str = "802.11AdHoc";
			    			
			  		if (i == 0)
						x = 0;
					else 
						x = strlen(pTemp);
					sprintf( pTemp+ x,"|ESSID:%-32s  BSSID:%02X-%02X-%02X-%02X-%02X-%02X         |\r\n|Network Type:%-16s     Channel: %02d		Signal: %3d      |\r\n ________________________________________________________________________ \r\n", cp,
								ssEntry_p->BssId[0], ssEntry_p->BssId[1], ssEntry_p->BssId[2],
								ssEntry_p->BssId[3], ssEntry_p->BssId[4], ssEntry_p->BssId[5],	
								cap_str, ssEntry_p->channel, ssEntry_p->RSSI );
					i++;
				}
					
				j++;
				
			}	
			
			for(;i<=15;i++)
				{
				x = strlen(pTemp);
				sprintf( pTemp+ x,"|                                                                        |\r\n|                                                                        |\r\n ________________________________________________________________________ \r\n");
			}

					sprintf(pTemp+strlen(pTemp),"\r\n%c", 0x0C);
			
//			sprintf(PrintBuffer->data, pTemp);
			sprintf((PrintBuffer->data + strlen(PrintBuffer->data)), pTemp);

		}
		else
#endif	// !defined(O_AXIS)
		{
			// Normal mode
			//IPAddr = _BoxIPAddress;
			//Mask = _BoxSubNetMask;
			//GatewayAddr = _BoxGatewayAddress;
    		IPAddr = &(Lanface->ip_addr);
    		Mask = &(Lanface->netmask);
    		GatewayAddr = &(Lanface->gw);
			WLZone = _WLZone;
			wlan_get_currssid(tempbuffer);
			memcpy(Public_Key, RSAPublicHMAC_MD5, 16);
			
			for(j=0; j < curr.maxEntry; )
			{
				ssEntry_p = curr.infoAddr + (j * sizeof(API_SURVEY_ENTRY));
										
				if(ssEntry_p->dirty && (ssEntry_p->IBSS == 0)
					&& ( ssEntry_p->channel > 0) && (ssEntry_p->channel <= 14) )
				{	
				
					if( !strcmp(ssEntry_p->SsId, ""))
	        			cp = temp_ssid ;
	        		else		
						cp = ssEntry_p->SsId;																										
			
					if( ssEntry_p->IBSS == 0)
						cap_str = "Infrastructure";
			 		else
			 			cap_str = "802.11AdHoc";
			    	
			    	if(ssEntry_p->wpa2Enabled)
	        				wep_str = "WPA2";
					else if(ssEntry_p->wpaEnabled)
   							wep_str = "WPA";
   						else
   						{
   							if(ssEntry_p->wepEnabled)
   								wep_str="WEP";
   							else 
   								wep_str="NO";
   						}
			    	
			    	if(ssEntry_p->B_Support == 1)
	        				trx_str = "B";
   						if(ssEntry_p->G_Support == 1)
	        				trx_str = "G";
			    	
			    	/* Refine RSSI value ... Ron 9/8/04 */
					//ssEntry_p->RSSI = (ssEntry_p->RSSI * 100)/30;
					RSSI_TEMP = ssEntry_p->RSSI * 2;
			 		if (RSSI_TEMP > 100)
			 			RSSI_TEMP = 100;
			    			
			  		if (i == 0)
						x = 0;
					else 
						x = strlen(pTemp);
					sprintf( pTemp+ x,"|%-32s |%02X-%02X-%02X-%02X-%02X-%02X | %02d | %4s  | %s | %3d%%%%|\r\n|_________________________________|__________________|____|_______|___|_____| \r\n"
								,cp,
								ssEntry_p->BssId[0], ssEntry_p->BssId[1], ssEntry_p->BssId[2],
								ssEntry_p->BssId[3], ssEntry_p->BssId[4], ssEntry_p->BssId[5],	
								ssEntry_p->channel, wep_str, trx_str, RSSI_TEMP);
					i++;
				}
					
				j++;
				if( j == curr.maxEntry)
				{
					m=strlen(pTemp);
					sprintf(pTemp+strlen(pTemp),"\r\n%c", 0x0C);
				}
			}	

			sprintf(PrintBuffer->data, PrintServerData,
//14/03/2005 mark	 	        _BoxName,														//Device Name
				Hostname,		//Device Name
		        //CURRENT_MAJOR_VER, CURRENT_MINOR_VER,CURRENT_PS_MODEL,WebLangVersion,CURRENT_BUILD_VER,		//Version
		        // Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
		        CURRENT_MAJOR_VER, CURRENT_MINOR_VER,CURRENT_PS_MODEL,&WebLangVersion,CURRENT_BUILD_VER,		//Version
		        MyPhysNodeAddress[0],MyPhysNodeAddress[1],MyPhysNodeAddress[2],	// Node ID
		        MyPhysNodeAddress[3],MyPhysNodeAddress[4],MyPhysNodeAddress[5],
		        TestPageCaption[0],										// TCP/IP Settings
		        IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3],						// IP address
				Mask[0],Mask[1],Mask[2],Mask[3],								// Subnet mask
				GatewayAddr[0],GatewayAddr[1],GatewayAddr[2],GatewayAddr[3],	// Gateway
				TestPageCaption[1],										// Wireless Settings
				(diag_flag == 0)?((EEPROM_Data.WLMode == 0)?"Infrastructure (Normal)":
	        	(EEPROM_Data.WLMode == 1)?"Ad-Hoc (Normal)":"802.11b Ad-Hoc (Normal)"):
		        	"Diagnostic",											// Operation mode
		        tempbuffer,														// Wireless ESSID
		        (WLZone==0xA1)?"Channel 1-11, USA":							// Wireless zone
		        (WLZone==0xA2)?"Channel 1-13, Europe":
		        (WLZone==0xA3)?"Channel 10-11, France":
		        (WLZone==0xA4)?"Channel 1-14, Japan":
		        (WLZone==0xA5)?"Channel 10-13, Spain":"Channel 1-11, USA",
		        wlan_get_channel(),
		        ((mvTxMode == 1)?" B ONLY":((mvTxMode == 2)?" G ONLY":((mvTxMode == 3)?" B/G/N Mixed":((mvTxMode == 4)?" N ONLY":" B/G Mixed")))),
		        (((rxStatsRSSI*3)>=100)?100:(rxStatsRSSI*3)),
		        linkQuality,												// Wireless channel
		        (mvWEPType==0)?"Disabled":(mvWEPType==1)?"64 bit":"128 bit",		// WEP
		        ((mvAuthenticationType==0x5)&&(mvWPAType==0x1))?"WPA2-PSK(CCMP)":
		        ((mvAuthenticationType==0x5)&&(mvWPAType==0x0))?"WPA2-PSK(TKIP)":
		        ((mvAuthenticationType==0x4)&&(mvWPAType==0x1))?"WPA-PSK(CCMP)":
		        ((mvAuthenticationType==0x4)&&(mvWPAType==0x0))?"WPA-PSK(TKIP)":"Disabled", // WPA-PSK
		        "Public Key",
		        Public_Key[0],Public_Key[1],Public_Key[2],Public_Key[3],Public_Key[4],Public_Key[5],
		        Public_Key[6],Public_Key[7],Public_Key[8],Public_Key[9],Public_Key[10],Public_Key[11],
		        Public_Key[12],Public_Key[13],Public_Key[14],Public_Key[15],
		        TestPageCaption[2],										// AppleTalk Settings
		        (mvAPPTLKEn==0x01)?"Enabled":"Disabled",							// AppleTalk
				mvATPortName,													// AppleTalk port name
				TestPageCaption[3],										// Rendezvous Settings
				(mvRENVEnable==0x01)?"Enabled":"Disabled",						// Rendezvous
				mvRENVServiceName,														// Rendezvous service name
				0x0C);
			
			sprintf((PrintBuffer->data + strlen(PrintBuffer->data)), pTemp);	
			
		}
	}
	else
	{
		// I am D-Link.
		IPAddr = _BoxIPAddress;
		Mask = _BoxSubNetMask;
		GatewayAddr = _BoxGatewayAddress;
		sprintf(PrintBuffer->data, PrintServerData,
	 	        _BoxName,				//Device Name
		        CURRENT_MAJOR_VER, CURRENT_MINOR_VER,		//Version
		        CURRENT_MAJOR_VER, CURRENT_MINOR_VER,		//Version
		        MyPhysNodeAddress[0],MyPhysNodeAddress[1],MyPhysNodeAddress[2],	//Node ID
		        MyPhysNodeAddress[3],MyPhysNodeAddress[4],MyPhysNodeAddress[5],
		        IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3],
				Mask[0],Mask[1],Mask[2],Mask[3],
				GatewayAddr[0],GatewayAddr[1],GatewayAddr[2],GatewayAddr[3],
				EEPROM_Data.ATPortName,
				0x0C);
	}		
		
//	pTemp = print_large(PrintBuffer->data+strlen(PrintBuffer->data),"P R I N T",0);
//	print_large(pTemp,"T E S T",1);
	
	free(pTemp);
	
	PrintBuffer->size = strlen(PrintBuffer->data);
	PrnPutOutQueueBuf(Port,PrintBuffer,PRN_Q_NORMAL);
	
	PrnSetNoUse(Port);
	
	return(PRINTER_FREE);
}
#endif //N535WP || N535WPI || N535WUI || N535WPID

#endif !CODE1

#if defined(HTTPD) && !defined(CODE1)
static FSInfo SearchFSBlock, *SearchFSInfo;
//static BYTE   SearchFSAnyFileAttached;
BYTE SearchObjectInit(void)
{
	BYTE i;
	int32 StartTimer = rdclock();

	while(HttpdUsed) return (2);

	HttpdUsed = 1;

	if( (PSMode & PS_NETWARE_MODE)
#ifdef NDS_PS
	   || (PSMode & PS_NDS_MODE)
#endif NDS_PS
	  ) {
		while((IntoNPS3main || IntoAttachFS) && ((rdclock() - StartTimer) < (5 * TICKS_PER_SEC)))
			kwait(0);
		if(IntoNPS3main || IntoAttachFS) {
			HttpdUsed = 0;
			return (2);
		}
	}

	SearchFSInfo = &SearchFSBlock;
	memset(SearchFSInfo,'\0',sizeof(FSInfo));

	if(QueryNearestFileServer(SearchFSocket, SearchFSInfo) == OKAY)	{
		if(RequestCreateServiceConnect(SearchFSocket, SearchFSInfo) == OKAY){
			if(RequestNegotiateBufferSize(SearchFSocket, SearchFSInfo) == OKAY) {
				return (OKAY);
			}
		}
	}
	HttpdUsed = 0;
	return (1);	 //Fail
}

BYTE SearchBinaryObjectName(BYTE *ObjectName,DWORD *LastObjectSeed,WORD ObjectType)
{
	WORD NCPRetCode;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_SCAN_BIND_DS_OBJECT);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(SearchFSocket,
	              SearchFSInfo->PCBPhysicalID,
	              6 + 12
	);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, SearchFSInfo->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;    // Service Request
	SendNCPData->SequenceNumber       = SearchFSInfo->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = SearchFSInfo->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x13;
	SendNCPData->ConnectionNumberHigh = SearchFSInfo->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;

	// Subfunction Length
	*(SendNCPSubData + 2) = 0x09;
	*(SendNCPSubData + 3) = 0x37;        // Subfunction Code 55
	NSET32((SendNCPSubData + 4), *LastObjectSeed);

	// Object Type (Word)
//12/22/99 marked *(SendNCPSubData + 8) = 0x00;	//Search Object Type = File server
//12/22/99 marked *(SendNCPSubData + 9) = 0x04;
	NSET16((SendNCPSubData + 8), ObjectType); // Object Type [Lo-Hi]

	*(SendNCPSubData + 10) = 0x01;	//Search Object Name: Length = 1
	*(SendNCPSubData + 11) = 0x2A;	//Search Object Name: value = *

	NCPRetCode=NCPRequest(SearchFSocket,SearchFSInfo);

	*LastObjectSeed = NGET32(ReceiveNCPSubData);	//Last Object Seed
	memcpy(ObjectName,ReceiveNCPSubData+6,48);			//File Server Name

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	if(NCPRetCode) {
//	     && !SearchFSAnyFileAttached)		//No Object Found

		DisConnection(SearchFSocket, SearchFSInfo);
		HttpdUsed = 0;
	}

	return(NCPRetCode);
}

#if defined(N716U2W) || defined(N716U2)
//615wu::No PSMain
void NovellPSmain(cyg_addrword_t data)
{
	BYTE Port;
	BYTE i;
	DWORD startime = rdclock();
	WORD PrinterStatus;

	while(1) {
		for(Port = 0 ; Port < NUM_OF_PRN_PORT ; Port++) {
			PrinterStatus = PrnGetPrinterStatus(Port);
			if(PrinterStatus == PrnNoUsed || PrinterStatus == NetwareUsed)
			{
				IntoNPS3main = 1;

				NPS3main(Port);

				IntoNPS3main = 0;
			}
			if((PSMode & PS_NETWARE_MODE) &&
#if defined(HTTPD) 
				    !HttpdUsed && PrimaryConnectFinish &&
#endif
			(rdclock() - startime) > TICKS_PER_SEC*10 )
			{
				IntoAttachFS=1;
		    	TryConnectNoAttachFS();
				IntoAttachFS=0;
				startime = rdclock();
			}	
			ppause(PollingTime / 10);
		}
	    sys_check_stack();	
	}
}
#endif // defined(N716U2W)

#if defined(NDWP2020)
void NovellPSmain(cyg_addrword_t data)
{
	BYTE Port;
	BYTE i,delay_type;
	DWORD startime = rdclock();
	WORD PrinterStatus;

	while(1) {
		for(Port = 0 ; Port < NUM_OF_PRN_PORT ; Port++) {
			PrinterStatus = PrnGetPrinterStatus(Port);
			for(i = 0 ; i < 3;i++) {
			if(PrinterStatus == PrnNoUsed || PrinterStatus == NetwareUsed)
			{
				IntoNPS3main = 1;

					if(NPS3main(Port) == SERVICE_DATA)	//eason
						delay_type = 1;
					else
						delay_type = 0;		
					//NPS3main(Port);

				IntoNPS3main = 0;
			}
				cyg_thread_yield();
			}	
			if((PSMode & PS_NETWARE_MODE) &&
#if defined(HTTPD) 
				    !HttpdUsed && PrimaryConnectFinish &&
#endif
			(rdclock() - startime) > TICKS_PER_SEC*10 )
			{
				IntoAttachFS=1;
		    	TryConnectNoAttachFS();
				IntoAttachFS=0;
				startime = rdclock();
			}	
			if(delay_type == 1)	//eason
				cyg_thread_yield();
			else	
			ppause(PollingTime / 10);
		}
	    sys_check_stack();	
	}
}
#endif // defined(DWP2020)

#endif
