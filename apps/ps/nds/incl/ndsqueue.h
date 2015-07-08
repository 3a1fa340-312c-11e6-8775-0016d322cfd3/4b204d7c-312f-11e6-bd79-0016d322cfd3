#ifndef _NDSQUEUE_H
#define _NDSQUEUE_H

extern PortPCB *NDSPCBInfo[NUM_OF_PRN_PORT];
extern BYTE NDSbNoActiveFSInfo; //still not ative File Server (ring)
extern BYTE NetWareReEntry, NDSConnectFlag;

uint32 NDSGetPrinterQueue(FSInfo *ConnInfo, uint32 PrintServerID);
void   NDSTryConnectNoAttachFS(void);
void   NDSMoveActivePortToFreePort(FSInfo *NDSpTimeOutFSInfo);
WORD   NDSHasAnyFileServerConnect(void);
void   SetNetwareLED(void);

/*
#define BEGIN_NETWARE_CRITICAL()  \
{	\
	while(NetWareReEntry) kwait(NULL); \
	NetWareReEntry = 1; \
}

#define END_NETWARE_CRITICAL()  { NetWareReEntry = 0;}
*/

void BEGIN_NETWARE_CRITICAL(BYTE value);
void END_NETWARE_CRITICAL(void);

//NDSMAIN.C
#define INTO_NEAR_DIR_SERVER        1   //QueryNearestDirectoryServer
#define INTO_GET_MAX_PACKET         2   //NDSReqGetBigNCPMaxPacket
#define INTO_SEND_NOTIFY            3   //SendNotifyMessage
#define INTO_SCAN_BIND_DS_OBJECT    4   //ScanBinaryDSObject

//NDSLOGIN.C
#define INTO_NDS_RESOLVE           10   //NDSResolveName
#define INTO_NDS_GET_SERVER_NAME   11   //NDSGetServerName
#define INTO_NDS_READ              12   //NDSRead
#define INTO_NDS_LOGIN_1           13   //NDSLogin-1
#define INTO_NDS_LOGIN_2           14   //NDSLogin-2
#define INTO_NDS_BEGIN_AUTH        15   //NDSBeginAuth
#define INTO_NDS_READ_ENTRY_NAME   16   //NDSReadEntryName
#define INTO_NDS_CHANGE_STATE      17   //NDSNCPChangeConnState
#define INTO_NDS_AUTH              18   //NDSAuthenticate

//NPS3.C
#define INTO_QUERY_NEAREST         30   //QueryNearestFileServer
#define INTO_CREATE_SERVICE_CONN   31   //RequestCreateServiceConnect
#define INTO_NEGO_BUFFER_SIZE	   32   //RequestNegotiateBufferSize
#define INTO_REQUEST_RIP           33   //RequestRIP
#define INTO_READ_PROPERTY         34   //ReadPropertyValue
#define INTO_DISCONNECTION         35   //DisConnection
#define	INTO_GET_LOGIN_KEY         36   //GetLoginKey
#define INTO_GET_BINDERY_OBJ       37   //GetBinderyObjectID
#define INTO_LOGIN_FS              38   //LoginToFS
#define INTO_OPEN_QUEUEINFO_FILE   39   //OpenQueueInfoFile
#define INTO_READ_FILE_DATA        40   //ReadFileData
#define INTO_ATTACH_QUEUE_SERVER   41   //AttachQueueServer
#define INTO_CLOSE_QUEUEINFO_FILE  42   //CloseQueueInfoFile
#define INTO_SERVICE_QUEUE_JOB     43   //ServiceQueueJob
#define INTO_READ_QUEUE_JOB_ENTRY  44   //ReadQueueJobEntry
#define INTO_GET_BINDERY_OBJ_NAME  45   //GetBinderyObjectName
#define INTO_GET_QUEUE_FILE_SIZE   46   //GetQueueJobFileSize
#define INTO_FINISH_QUEUE_JOB	   47   //FinishServiceQueueJob
#define INTO_SEARCH_FS_BIND_OBJ    48   //SearchBinaryFSObject

#endif  _NDSQUEUE_H
