#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "nds.h"
#include "ndscrypt.h"
#include "eeprom.h"
#include "led.h"
#include "ledcode.h"
#include "nds.h"
#include "ndsext.h"
#include "ndslogin.h"
#include "ndsqueue.h"
#include "ndsmain.h"

//615wu-----------------------------------------------------------------------
static void kwait(int a){ cyg_thread_yield();}	//615wu //must be, could not use 'yeild' to replace
extern int memicmp(const void * first, const void * last, unsigned int count);
extern char Hostname[];

#ifdef NDS_PS

#ifdef _PC
#define NDS_DELAY_TIME (1000L)
#else
#define NDS_DELAY_TIME (30000L)
#endif

NCP_NDS_STRUCT *SendNDSData;
BYTE    NDStatus;

BYTE GetTreeServer(BYTE *TreeName,FSInfo *ConnInfo);
BYTE QueryNearestDirectoryServer(BYTE *TreeName,FSInfo *FSInfoPointer);
BYTE ScanBinderyDSName(BYTE *TreeName, FSInfo *ConnInfo,BYTE *DSName);
WORD NDSReqGetBigNCPMaxPacket(WORD Socket, FSInfo  *FSInfoPointer);

ECB           NDSReceiveWDogECB;
IPXHeader     NDSReceiveWDogIPXHeader;
WatchDog_Rec  NDSReceiveWDogData;

void NDSmain(cyg_addrword_t data)
{
	WORD   NDSSocketData;
	FSInfo NDSRootTreeInfo;
	int32  rc;
	uint32 PrintServerID;

    if(_NDSTreeName[0] == '\0' || _NDSContext[0] == '\0') return;

	NDSSocketData=NDSPsSocket;
	if(IPXOpenSocket((BYTE *)&NDSSocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
		printf(" Can't Open NDS Ps Socket\n");
		exit(0);
#endif
#ifndef _PC
		LightOnForever(LED_PS_SOCKET);
#endif
	}

	NDSSocketData=NDSWatchDogSocket;
	if(IPXOpenSocket((BYTE *)&NDSSocketData,LONG_LIVED) != OKAY) {
#ifdef PC_OUTPUT
		printf(" Can't Open NDS Ps+1 Socket\n");
		exit(0);
#endif
#ifndef _PC
		LightOnForever(LED_PS_SOCKET);
#endif
	}

	NDSListenWatchDog();  // Listen NDS Watch Dog packet

	NDStatus = NDS_E_BEGIN_LOGIN_TO_SERVER;
	SendNDSData = (NCP_NDS_STRUCT *) SendNCPSubData;
	do {
		ppause(NDS_DELAY_TIME);

		if(GetTreeServer(_NDSTreeName,&NDSRootTreeInfo) != OKAY) {
			NDStatus = NDS_E_CAN_NOT_GET_TREE_SERVER;
			continue;
		}

		if(NDSAttachToFS(&NDSRootTreeInfo) != OKAY) {
			NDStatus = NDS_E_CAN_NOT_ATTACH_TO_SERVER;
			continue; //Attach to Root Server
		}

		PrintServerID = 0;
		if( (rc = NDSLoginAuth(&NDSRootTreeInfo,_PrintServerName,_NDSContext
		         ,_NovellPassword,&PrintServerID)) != OKAY)
		{
			if (rc != NDS_PASSWORD_EXPIRED) {
#ifdef _PC
				printf("(NDS) LoginAuth Fail \n");
#endif _PC
				NDStatus = NDS_E_CAN_NOT_LOGIN_TO_SERVER;
			} else {
#ifdef _PC
				printf("(NDS) Password has expired\n");
#endif _PC
				NDStatus = NDS_E_PASSWORD_HAS_EXPIRED;
			}
		}
		else if( (rc = NDSGetPrinterQueue(&NDSRootTreeInfo,PrintServerID)) == OKAY)
		{
			NDStatus = NDS_E_PRE_CONNECTED;
			break;
		}

		if(NDStatus == NDS_E_BEGIN_LOGIN_TO_SERVER) {
			if(rc == NDS_NOMEM) NDStatus = NDS_E_INTERNAL_ERROR;
			else NDStatus = NDS_E_GENERAL_SERVER_ERROR;
		}
        sys_check_stack();
	} while (SetNetwareLED(),1);

	if(rc == OKAY) {
		//infinite loop for connect to no attached file server
		NDSTryConnectNoAttachFS();
	}
}

#define NDS_MAX_LAN	128


BYTE GetTreeServer(BYTE *TreeName,FSInfo *ConnInfo)
{
	DWORD DSObjectID;
	BYTE NetworkAddress[12];
	BYTE DSName[48];  //Directory Server Name

	memset(ConnInfo,'\0',sizeof(FSInfo));
	if(QueryNearestDirectoryServer(TreeName,ConnInfo) != OKAY) {

		if(NGET16(ConnInfo->PCBNetworkNumber+10) == 0) {
			//Directory Server not found , Query nearest Bindery server
			if(QueryNearestFileServer (NDSPsSocket,ConnInfo) != OKAY)
				return (FAILURE);
		}

		if( (RequestCreateServiceConnect(NDSPsSocket, ConnInfo) != OKAY) ||
		    (RequestNegotiateBufferSize(NDSPsSocket, ConnInfo) != OKAY) )
		{
			return (FAILURE);
		}

		if( (ScanBinderyDSName(TreeName,ConnInfo,DSName) != OKAY) ||
		    (ReadPropertyValue(NDSPsSocket, ConnInfo,DSName, DS_SERVER_OBJECT,NetworkAddress) != OKAY) )
		{
			DisConnection(NDSPsSocket,ConnInfo);
			return (FAILURE);
		}
		DisConnection(NDSPsSocket,ConnInfo);
		DataCopy(ConnInfo->PCBNetworkNumber,NetworkAddress,10);
		NSET16(ConnInfo->PCBNetworkNumber+10, 0x5104) ;  //0x0451 (NCP)
	}

	return (OKAY);
}

BYTE ScanBinderyDSName(BYTE *TreeName, FSInfo *ConnInfo,BYTE *DSName)
{
	WORD NCPRetCode;
	BYTE TreeLen;

#ifdef NDS_PS
	BEGIN_NETWARE_CRITICAL(INTO_SCAN_BIND_DS_OBJECT);
#endif NDS_PS

	// Setup ECB Block
	SendPsECBInit(NDSPsSocket,ConnInfo->PCBPhysicalID,6 + 45);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, ConnInfo->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;    // Service Request
	SendNCPData->SequenceNumber       = ConnInfo->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = ConnInfo->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x13;
	SendNCPData->ConnectionNumberHigh = ConnInfo->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 0x17;        // Function Code 23
	*(SendNCPSubData + 1) = 0x00;

	// Subfunction Length
	*(SendNCPSubData + 2) = 0x09;
	*(SendNCPSubData + 3) = 0x37;        // Subfunction Code 55
	NSET32(SendNCPSubData + 4, 0xFFFFFFFF);

	// Object Type (Word)
	*(SendNCPSubData + 8) = 0x02;	//Search Object Type = Directory Server
	*(SendNCPSubData + 9) = 0x78;

	*(SendNCPSubData + 10) = 0x21;	//Search Object Name: Length = 33
	TreeLen = strlen(TreeName);
	memset((SendNCPSubData + 11),0x5F,32);
	memcpy((SendNCPSubData + 11),TreeName,TreeLen);
	*(SendNCPSubData + 11+ 0x20) = 0x2A;

	NCPRetCode=NCPRequest(NDSPsSocket,ConnInfo);

//	*ObjectID = NGET32(ReceiveNCPSubData);	//DS Object ID
	memcpy(DSName,ReceiveNCPSubData+6,48);

#ifdef NDS_PS
	END_NETWARE_CRITICAL();
#endif NDS_PS

	return(NCPRetCode);
}

BYTE QueryNearestDirectoryServer(BYTE *TreeName,FSInfo *FSInfoPointer)
{
	uint32 startime;
	SAPResponseData *ReceiveSAPData;
	SAPQueryData    *SendSAPData;
	WORD   RetryCount = 0;
	BYTE   FirstListen = 1;
	BYTE  *p;

#ifdef PC_OUTPUT
	AtSaySpace(0,15,80);
	printf ("(NDS) ? Query General Directory Server....");
#endif

	BEGIN_NETWARE_CRITICAL(INTO_NEAR_DIR_SERVER); //////////

	//Listen Ps ECB Block
	ReceiveSAPData = (SAPResponseData *)ReceivePsIPXData;
	ListenPsECB(NDSWatchDogSocket, 66);
	//Initial Send Ps ECB Block
	SendSAPData = (SAPQueryData *)SendPsIPXData;

	SendPsECBInit(NDSWatchDogSocket, BrocastNode,sizeof(SAPQueryData));

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x04;   // IPX Packet Type
	DataCopy (SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 4);
	DataCopy (SendPsIPXHeader.destination.node, BrocastNode, 6);
	NSET16(SendPsIPXHeader.destination.socket, SAPSocket);

	// Setup SAP Protocol
	SendSAPData->PacketType = 0x0300;    // Nearest Service Query
	SendSAPData->ServerType = 0x7802;    // Directory Server

	SendPsECB.IPXFrameType = GetIPXFrameType(CurrentFrameType);

	// Send (Query Nearst File Server with Auto-Detect FrameType) Packet
	do {
		IPXSendPacket(&SendPsECB);
	} while (SendPsECB.inUseFlag) ;

ReListenDS:

	startime = rdclock();

	while(ReceivePsECB.inUseFlag || ReceivePsECB.completionCode) {
		kwait(0);
		if((rdclock()-startime) > NDS_MAX_RETRY_TIME) {
			if(++RetryCount < NDS_SAP_RETRY_COUNT) {
				if(FirstListen){
					do {
						IPXSendPacket(&SendPsECB);
					} while (SendPsECB.inUseFlag) ;
				}
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
				END_NETWARE_CRITICAL();
				return (TIME_OUT);
			}
		}
	}

	//Get the Tree Directory Server's Network Address
	DataCopy(FSInfoPointer->PCBNetworkNumber, ReceiveSAPData->Network, 10);
	NSET16(FSInfoPointer->PCBNetworkNumber+10,0x5104);  //0x0451 (NCP)

	//Get the Tree Directory Server's Physical Node Addsess
	DataCopy(FSInfoPointer->PCBPhysicalID, ReceivePsECB.immediateAddress, 6);

	p = ReceiveSAPData->ServerName + 31; //Tree Name Length <= 32
	while(p > ReceiveSAPData->ServerName && *p == '_') p--;

	if(((p - ReceiveSAPData->ServerName+1) != strlen(TreeName) ) ||
		memicmp(ReceiveSAPData->ServerName,TreeName,strlen(TreeName)) != 0) {
		ListenPsECB(NDSWatchDogSocket, 66);		
		FirstListen = 0;
		goto ReListenDS;
	}

	END_NETWARE_CRITICAL();	//////////

	return (OKAY);
}

WORD NDSAttachToFS(FSInfo *FSInfoPointer)
{
	WORD rc = OKAY;

#ifdef PC_OUTPUT
	AtSaySpace(0,14,80);
	printf ("(NDS) Try Connection.... ",FSInfoPointer->PCBFileServerName);
#endif

	//------- Request RIP (RIP) ----------------------------------------
	//Get FS's Physical Node Address
	if((rc = RequestRIP(NDSPsSocket,FSInfoPointer)) != OKAY) {
		goto AttFSErrExit;
	}

	//------ Request connect to server --------------------------------
	if((rc=RequestConnect2Server(NDSPsSocket, FSInfoPointer)) != OKAY) {
		goto AttFSErrExit;
	}

AttFSErrExit:

	return rc;
}

WORD RequestConnect2Server(WORD Socket,FSInfo *FSInfoPointer)
{
	WORD rc;

	//------ Request Create Service Connection (NCP)--------------------
	//For Get Connect Number (0)
	if((rc=RequestCreateServiceConnect(Socket, FSInfoPointer)) != OKAY) {
		return rc;
	}

	FSInfoPointer->QueueSize = NDS_MAX_PACKET_SIZE - IPX_WITH_NCP_SIZE;
	//------ Request Get Big Packet NCP Max Packet Size
	if((rc=NDSReqGetBigNCPMaxPacket(Socket,FSInfoPointer)) != OKAY) {
		//------ Request Negotiate Buffer Size (NCP)------------------------
		//Get negotiate buffer Size
		if((rc=RequestNegotiateBufferSize(Socket, FSInfoPointer)) != OKAY) {
			return rc;
		}
		FSInfoPointer->QueueSize = 1024;
	}

	if(FSInfoPointer->QueueSize > 1400) FSInfoPointer->QueueSize = 1400;

	return OKAY;
}

WORD NDSReqGetBigNCPMaxPacket(WORD Socket, FSInfo  *FSInfoPointer)
{
	WORD rc;

	BEGIN_NETWARE_CRITICAL(INTO_GET_MAX_PACKET); //////////

	// Setup ECB Block
	SendPsECBInit(Socket,
	              FSInfoPointer->PCBPhysicalID,
	              6 + 4
	);

	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;    // NCP Packet Type
	DataCopy(SendPsIPXHeader.destination.network, FSInfoPointer->PCBNetworkNumber, 12);

	// Setup SAP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = FSInfoPointer->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = FSInfoPointer->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x00;
	SendNCPData->ConnectionNumberHigh = FSInfoPointer->PCBConnectionNumberHigh;

	*(SendNCPSubData + 0) = 97;		//Get Big Packet NCP Max Packet Size
	NSET16((SendNCPSubData + 1), WordSwap(NDS_MAX_PACKET_SIZE));
	*(SendNCPSubData + 3) = 0;      //Security flag

	rc = NCPRequest(Socket,FSInfoPointer);

	END_NETWARE_CRITICAL();	//////////

	return rc;
}

void NDSWatchDogESR (void)
{
	if(NDSReceiveWDogData.SignatureChar==0x3f) {
		// if true then Response WDog

		SendWDogECB.ESRAddress = 0x00;
		SendWDogECB.inUseFlag  = 0;
		SendWDogECB.socketNumber = NDSWatchDogSocket;
		DataCopy(SendWDogECB.immediateAddress,NDSReceiveWDogECB.immediateAddress, 6);

		SendWDogECB.IPXFrameType = NDSReceiveWDogECB.IPXFrameType;
		SendWDogECB.fragmentCount = 2;
		SendWDogECB.fragmentDescriptor[0].address = &SendWDogIPXHeader;
		SendWDogECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
		SendWDogECB.fragmentDescriptor[1].address = &SendWDogData;
		SendWDogECB.fragmentDescriptor[1].size    = 2;

		// Setup IPX Header
		SendWDogIPXHeader.packetType = 0;  // UnKnown Type
		DataCopy(SendWDogIPXHeader.destination.network,NDSReceiveWDogIPXHeader.source.network,12);

		// Setup WatchDog Protocol
		SendWDogData.ConnectionNumber = NDSReceiveWDogData.ConnectionNumber;
		SendWDogData.SignatureChar = 0x59; // Response 'Y' (Session is valid)

		// Send IPX Packet
		do{
			IPXSendPacket(&SendWDogECB);
		}while(SendWDogECB.inUseFlag) ;
	}

	NDSListenWatchDog();  //ReListen WatchDog Packet
#ifdef PC_OUTPUT
	AtSaySpace(20,0,59);
	printf ("\a-------- (NDS) Reply a WDog Packet ------------");
	kwait(0);
	AtSaySpace(20,0,59);
	printf ("\-------- (NDS) Reply a WDog Packet ------------");
#endif
}

void NDSListenWatchDog (void)
{
	// Setup Receive-ECB Block
	NDSReceiveWDogECB.ESRAddress = NDSWatchDogESR;   // Should be fill WatchDog ESR Address
	NDSReceiveWDogECB.inUseFlag  = 0;
	NDSReceiveWDogECB.socketNumber = NDSWatchDogSocket;
	NDSReceiveWDogECB.fragmentCount = 2;
	NDSReceiveWDogECB.fragmentDescriptor[0].address = &NDSReceiveWDogIPXHeader;
	NDSReceiveWDogECB.fragmentDescriptor[0].size    = sizeof(IPXHeader);
	NDSReceiveWDogECB.fragmentDescriptor[1].address = &NDSReceiveWDogData;
	NDSReceiveWDogECB.fragmentDescriptor[1].size    = 2;
	IPXListenForPacket (&NDSReceiveWDogECB);
}

WORD SendNotifyMessage(WORD Socket, BYTE PortNo, PortPCB *pPortPCB)
{
	WORD NCPRetCode;
	BYTE *TextScript, *SendData;
	BYTE ScriptLen,TmpLen;

	if((NCPRetCode = ReadQueueJobEntry(Socket ,pPortPCB, pPortPCB->PCBQueueInfo[pPortPCB->QCount].QueueObjectID, &TextScript)) != OKAY) {
		return NCPRetCode;
	}
	ScriptLen = strlen(TextScript);

	BEGIN_NETWARE_CRITICAL(INTO_SEND_NOTIFY); //////////

	// Setup ECB Block
	SendPsECBInit(Socket,pPortPCB->pFSInfo->PCBPhysicalID,6 + 255);
	// Setup IPX Header
	SendPsIPXHeader.packetType = 0x11;   // NCP Packet Type
	DataCopy (SendPsIPXHeader.destination.network,pPortPCB->pFSInfo->PCBNetworkNumber, 12);

	// Setup NCP Protocol
	SendNCPData->RequestType          = 0x2222;   // Service Request
	SendNCPData->SequenceNumber       = pPortPCB->pFSInfo->PCBSequenceNumber;
	SendNCPData->ConnectionNumberLow  = pPortPCB->pFSInfo->PCBConnectionNumberLow;
	SendNCPData->TaskNumber           = 0x02;
	SendNCPData->ConnectionNumberHigh = pPortPCB->pFSInfo->PCBConnectionNumberHigh;
	*(SendNCPSubData + 0) = 21;                      // Function Code 21
	*(SendNCPSubData + 1) = 00;                      // SubFunction Length (Hi)
	*(SendNCPSubData + 3) = 0x00;                    // SubFunction = 00
	*(SendNCPSubData + 4) = 1;                       // Client List #
	*(SendNCPSubData + 5) = pPortPCB->ClientStation; // ClientStation

	SendData = SendNCPSubData + 7;
	//-------------- Message ---------------------------------------
	*(SendData++) = '[';
	TmpLen = strlen(Hostname);
	DataCopy(SendData, Hostname, TmpLen);
    SendData += TmpLen;
#if (NUM_OF_PRN_PORT > 1)
	DataCopy(SendData,"-P",2);
	SendData += 2;
	*(SendData++) = PortNo+'1';
#endif
	DataCopy(SendData,"]: File ", 8);
	SendData += 8;
	DataCopy (SendData, TextScript, ScriptLen);
	SendData += ScriptLen;
	DataCopy (SendData, " was printed.", 14);
	SendData += 14;
	//--------------------------------------------------------------

	*(SendNCPSubData + 6) = (SendData - SendNCPSubData); // Subfunction Code
	*(SendNCPSubData + 2) = 3 + *(SendNCPSubData + 6);   // SubFunction Length (Low)


	NCPRetCode = NCPRequest(Socket,pPortPCB->pFSInfo);

	END_NETWARE_CRITICAL();	//////////

	return (NCPRetCode);
}

#endif NDS_PS
