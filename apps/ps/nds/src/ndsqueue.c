#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "nps.h"
#include "nds.h"
#include "ndscrypt.h"
#include "nwcrypt.h"
#include "mpilib.h"
#include "ndsext.h"
#include "ndsmain.h"
#include "ndslogin.h"
#include "ndsqueue.h"
#include "ndsmacro.h"
#include "prnqueue.h"

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
extern WORD ServiceNetWareQueue (WORD Socket, PortPCB  *PortPCBPointer);
static void kwait(int a){  cyg_thread_yield();}
extern PrnInfo          PortInfo[NUM_OF_PRN_PORT];
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#ifdef NDS_PS

BYTE NetWareReEntry = 0, NDSConnectFlag = 0;

FSInfo  *NDSFSInfo;
PortPCB *NDSPCBInfo[NUM_OF_PRN_PORT];

BYTE  NDSbNoActiveFSInfo; //still not ative File Server (ring)

int32 NDSReadGetAttrCount(BYTE **buf)
{
	//Interaction Handle (-1 or 0)
	NDSget_dword_lh(buf);

	//Entry Info  (1)
	if(NDSget_dword_lh(buf) !=  1) return (-1);

	return NDSget_dword_lh(buf);      //Name of attributes (1)
}

int32 NDSReadEntryCount(BYTE **buf, uint32 SyntaxID)
{
	uint32 skiplen;

	if(NDSget_dword_lh(buf) != SyntaxID) return (-1); //Syntax ID
	skiplen = NDSget_dword_lh(buf);    //Length of Attribute Name
	*buf += (skiplen+3) & (~3);         //Skip attribute name
	return NDSget_dword_lh(buf);       //Entries
}

//SyntaxID = 1
BYTE *NDSReadGetHostName(BYTE **buf)
{
	uint32 skiplen;
	BYTE *EntryName;

	skiplen = NDSget_dword_lh(buf);    //Length of Entry Item
	EntryName = *buf;
	*buf += (skiplen+3) & (~3);         //Skip to next item

	return EntryName;
}

//SyntaxID = 12
uint32 NDSReadGetNetworkAddress(BYTE **buf)
{
	NDSget_dword_lh(buf);  //Item Length
	NDSget_dword_lh(buf);  //Address Type (0 = IPX)
	if(NDSget_dword_lh(buf) != 12) return NDS_NCP_PACKET_LENGTH;

	return OKAY;
}

//SyntaxID = 25
BYTE *NDSReadGetEntryItem(BYTE **buf, uint32 *Level)
{
	uint32 skiplen;
	BYTE *EntryName;

	skiplen = NDSget_dword_lh(buf);    //Length of Entry Item
	if(Level) *Level = NGET32(*buf);
	EntryName = *buf + 12;
	*buf += (skiplen+3) & (~3);         //Skip to next item

	return EntryName;
}

void NDSAttachQueueServer(WORD Socket, PortPCB *PCBInfo)
{
	WORD j, i = 0;

	while(i < PCBInfo->TotalQueue)	{
		if(AttachQueueServer(Socket, PCBInfo->pFSInfo,
		                  PCBInfo->PCBQueueInfo[i].QueueObjectID) != OKAY)
		{
#ifdef PC_OUTPUT
				AtSaySpace(0,15,80);
				printf("(NDS) Has object but QUEUE ID not same !!");
#endif
			//Can not Attach this Queue, remove this Queue ID
			for(j = i+1; j < PCBInfo->TotalQueue; j++) {
				PCBInfo->PCBQueueInfo[j-1] = PCBInfo->PCBQueueInfo[j];
			}
			if(--PCBInfo->TotalQueue <= 0)
				NSET32(PCBInfo->PCBQueueInfo[0].QueueObjectID, 0);
			continue;
		}
		i++;
	}
}

uint32 NDSGetPrinterQueue(FSInfo *ConnInfo, uint32 PrintServerID)
{
	int16 i,j,k;
	BYTE *p, *buf, *bufend;
	BYTE *p1,*buf1,*buf1end;
	int16 outlen;
	int16 NumOfPrinter, NumOfQueue;
	BYTE *uniItemName;
	uint32 CurItemID;
	BYTE  FSCount;
	BYTE  PCBCount[NUM_OF_PRN_PORT];
	BYTE  ActiveFSIndex;
	BYTE *pPCBIndex;

	struct TmpFSInfo {
		uint32 FileServerID;
		BYTE   PrePrnNO;
#ifdef PC_OUTPUT
		BYTE   FileServerName[48];   // Queue Object Name
#endif PC_OUTPUT
	} *FileServerInfo;

	struct TmpQInfo {
		uint32 QueueID;
		BYTE   FSIndex; //link to FileServerID[index]
		BYTE   *QueueName;   // Queue Object Name
	} *QueueInfo[NUM_OF_PRN_PORT];

	uint32 rc;


	FileServerInfo = NULL;
	NDSFSInfo = NULL;
	buf = buf1 = NULL;

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		QueueInfo[i] = NULL;
		NDSPCBInfo[i] = NULL;
	}

	rc = OKAY;

	if (!(buf = malloc(2048)) || !(buf1 = malloc(2048)) ||
		!(FileServerInfo = (struct TmpFSInfo *) calloc(MAX_NDS_FS,sizeof(struct TmpFSInfo)))
	) {
		rc = NDS_NOMEM;
		goto GetQErrExit;
	}

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		PCBCount[i] = 0;
		if(!(QueueInfo[i] = (struct TmpQInfo *) calloc(MAX_NDS_QUEUE_PER_PRN+1,sizeof(struct TmpQInfo)))) {
			rc = NDS_NOMEM;
			goto GetQErrExit;
		}
	}

	//Read (Printer) ////////////////////////////////
	if( (outlen = NDSRead(buf, 2048, ConnInfo,PrintServerID, "Printer")) <= 0) {
		if(outlen == (int16) NDS_BUFFER_OVERFLOW) {
			NDStatus = NDS_E_TOO_MANY_PRINTERS;
		}
		else NDStatus = NDS_E_CAN_NOT_GET_PRINTER_OBJECT;
		rc = NDS_INVALID_RESPONSE;
		goto GetQErrExit;
	}
	bufend = (p = buf) + outlen;

	if( NDSReadGetAttrCount(&p) <= 0 || (p > bufend) ||
		(NumOfPrinter = NDSReadEntryCount(&p,25)) <= 0 || (p > bufend) ) {
		rc = NDS_NCP_PACKET_LENGTH;
		goto GetQErrExit;
	}

	for (k = 0 ;k < NumOfPrinter; k++) {
		//Get uni-printer name , CurItemID = Printer Number	(0 , 1 , 2)

		uniItemName = NDSReadGetEntryItem(&p,&CurItemID);
		if(k < (NumOfPrinter-1) && p > bufend) {
			rc = NDS_NCP_PACKET_LENGTH;
			goto GetQErrExit;
		}

		if(CurItemID >= NUM_OF_PRN_PORT) continue;
		i = (int16) CurItemID; //printer number

		//Resolve printer name ==> Get Printer ID
		if( (rc = NDSResolveName(ConnInfo, 0x62, (uni_char *) uniItemName,
		                    &CurItemID, NULL, NULL)) != OKAY)
		{
			if(rc == NDS_NO_SUCH_ENTRY) {
				NDStatus = NDS_E_CAN_NOT_GET_PRINTER_ENTRY;
			}
			goto GetQErrExit;
		}

		//Read (Queue) ////////////////////////////////////////////
		if( (outlen = NDSRead(buf1, 2048, ConnInfo,CurItemID, "Queue")) <= 0) {
			if(outlen == NDS_NO_SUCH_OBJECT) {
#ifdef PC_OUTPUT
				AtSaySpace(0,20,80);
				printf("No queue for Port%d\n");
#endif PC_OUTPUT
				continue;
			}
			if(outlen == (int16) NDS_BUFFER_OVERFLOW) {
				NDStatus = NDS_E_TOO_MANY_QUEUE;
			}
			rc = NDS_INVALID_RESPONSE;
			goto GetQErrExit;
		}

		buf1end = (p1 = buf1) + outlen;

		if( NDSReadGetAttrCount(&p1) <= 0 || (p1 > buf1end) ||
			(NumOfQueue = NDSReadEntryCount(&p1,25)) <= 0 || (p1 > buf1end) ) {
			rc = NDS_NCP_PACKET_LENGTH;
			goto GetQErrExit;
		}

		if(NumOfQueue > MAX_NDS_QUEUE_PER_PRN) NumOfQueue = MAX_NDS_QUEUE_PER_PRN;

		for (j = 0 ; j < NumOfQueue; j++) {

			//Get uni-queue name
			uniItemName = NDSReadGetEntryItem(&p1,NULL);
			if(j < (NumOfQueue - 1) && p1 > buf1end) {
				rc = NDS_NCP_PACKET_LENGTH;
				goto GetQErrExit;
			}

			//Resolve printer queue name ==> Get Queue ID
			if( (rc = NDSResolveName(ConnInfo, 0x62, (uni_char *) uniItemName,
			                         &CurItemID, NULL, NULL)) != OKAY)
			{
				if(rc == NDS_NO_SUCH_ENTRY) {
					NDStatus = NDS_E_CAN_NOT_GET_QUEUE_ENTRY;
				}
				goto GetQErrExit;
			}

			QueueInfo[i][j].QueueID = CurItemID;
			if((QueueInfo[i][j].QueueName = (BYTE *)malloc(NDSuni_strlen((const uni_char*) uniItemName) + 1)) == NULL) {
				rc = NDS_NOMEM;
				goto GetQErrExit;
			}
			NDSu2c_strcpy(QueueInfo[i][j].QueueName, (const uni_char *) uniItemName);
		}
	}
	free(buf1);
	buf1 = NULL;

	//Get Build QueueInfo & FileServerInfo
	//
	//          +--------+-------------+     +--------+------------+
	// LPT3     |QueueID | FSIndex(FF) |==>  |                     |==>
	//        +--------+------------+ +   +--------+------------+ +
	// LPT2   |QueueID | FSIndex(2) |==>  |                     |==>
	//      +--------+------------+ +   +--------+------------+ +
	// LPT1	|QueueID | FSIndex(0) |==>  |QueueID | FSIndex(1) |==> .........
	//      |        |      *     |     |        |      *     |
	//      +--------+------º-----+     +--------+------º-----+
	//                      º                           º
    //           É==========¼       É===================¼
    //  +-----------------+----------------+-----------------+
    //  | FileServerID (0) | FileServerID(1) | FileServerID(2) | ........
    //  +------------------+-----------------+-----------------+

	FSCount = 0;
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		j = 0;
		while(QueueInfo[i][j].QueueID) {
			//Read Host Server //////////////////////////////////////////////
			if( (outlen = NDSRead(buf, 2048, ConnInfo,QueueInfo[i][j].QueueID, "Host Server")) <= 0) {
				rc = NDS_INVALID_RESPONSE;
				NDStatus = NDS_E_CAN_NOT_ATTACH_BINDERY_QUEUE;

				///////// 12/22/99 ///////////////
				free(QueueInfo[i][j].QueueName);
				QueueInfo[i][j].QueueName = NULL;
				k = j+1;

				while(QueueInfo[i][k].QueueID) {
					QueueInfo[i][k-1] = QueueInfo[i][k++];
				}
				QueueInfo[i][k-1].QueueID = 0;
				continue;
				////////// 12/22/99 //////////////
				// goto GetQErrExit;
			}
			bufend = (p = buf) + outlen;

			if( NDSReadGetAttrCount(&p) <= 0 || (p > bufend) ||
			    NDSReadEntryCount(&p,1) <= 0 || (p > bufend) ) {
				rc = NDS_NCP_PACKET_LENGTH;
				goto GetQErrExit;
			}

			//Get uni-file-server name
			uniItemName = NDSReadGetHostName(&p);
//			if(p > bufend) {
//				rc = NDS_NCP_PACKET_LENGTH;
//			    goto GetQErrExit;
//			}

			//Resolve file server name ==> Get File Server ID
			if( (rc = NDSResolveName(ConnInfo, 0x62, (uni_char *) uniItemName,
			                         &CurItemID, NULL, NULL)) != OKAY)
			{
				if(rc == NDS_NO_SUCH_ENTRY) {
					NDStatus = NDS_E_CAN_NOT_GET_FS_ENTRY;
				}
				goto GetQErrExit;
			}

			//Insert File Server ID to List	///////////////////////////////
			// i : Printer NO, j : Queue num of current printer
			// k : FileServer NO
			for(k = 0 ; k < FSCount ; k++) {
				if(FileServerInfo[k].FileServerID == CurItemID) break;
			}

			if(k < MAX_NDS_FS) {
				if(k >= FSCount) {
					//Add new FS
					FileServerInfo[k].FileServerID = CurItemID;
					FSCount++;
					PCBCount[i]++;
#ifdef PC_OUTPUT
					if(NDSuni_strlen((const uni_char *) uniItemName) >= 47) uniItemName[47*2] = '\0';
					NDSu2c_strcpy(FileServerInfo[k].FileServerName, (const uni_char *) uniItemName);
#endif PC_OUTPUT
				} else {
					//FS Already exist
					if (FileServerInfo[k].PrePrnNO != i) {
						//if previous Printer# not same as current printer#
						PCBCount[i]++;
					}
				}
				FileServerInfo[k].PrePrnNO = i;
				QueueInfo[i][j].FSIndex = k;
			} else {
				//Too many Fiie Servers discard this queue !!!
				QueueInfo[i][j].FSIndex = NULL_BYTE;
			}
			//////////////////////////////////////////////////////////////
			j++;
		}  // while(QueueInfo[i][j].QueueID) { .....
	} // for(i = 0 ; i < NUM_OF_PRN_PORT; i++) ......

	ActiveFSIndex = NDSbNoActiveFSInfo = NULL_BYTE;

	if(FSCount) {
		//Build NDS File Server Info ///////////////////////////
		if( !(NDSFSInfo = (FSInfo *)calloc(FSCount,sizeof(FSInfo))) ) {
			rc = NDS_NOMEM;
			goto GetQErrExit;
		}

		////////////////////////////////////////
		////Set File server network address ////
		////////////////////////////////////////
		for( i = 0 ; i < FSCount; i++) {
			//Read Network Address ///////////////////////////////////////
			if( (outlen = NDSRead(buf, 2048, ConnInfo, FileServerInfo[i].FileServerID, "Network Address")) <= 0)
			{
				rc = NDS_INVALID_RESPONSE;
				goto GetQErrExit;
			}

			bufend = (p = buf) + outlen;

			if( NDSReadGetAttrCount(&p)  <= 0 || (buf > bufend) ||
			    NDSReadEntryCount(&p,12) <= 0 || (buf > bufend) )
			{
				rc = NDS_NCP_PACKET_LENGTH;
				goto GetQErrExit;
			}

			if((rc = NDSReadGetNetworkAddress(&p)) != OKAY) goto GetQErrExit;

			if(memcmp(p,ConnInfo->PCBNetworkNumber,12) == 0) {
				//This File Server = Current Attached File Server
				ActiveFSIndex = i;
			} else {
				if(NDSbNoActiveFSInfo == NULL_BYTE) NDSbNoActiveFSInfo = i;
				NDSFSInfo[i].NextFSInfo = NDSFSInfo[NDSbNoActiveFSInfo].NextFSInfo;
				NDSFSInfo[NDSbNoActiveFSInfo].NextFSInfo = i;
				NDSbNoActiveFSInfo = i;
				memcpy(NDSFSInfo[i].PCBNetworkNumber,p,12);
			}

#ifdef PC_OUTPUT
			NDSFSInfo[i].NDSTmpFileServerName = strdup(FileServerInfo[i].FileServerName);
#endif PC_OUTPUT
		}
		free(buf);
		buf = NULL;

		//Build PCB Info for NDS Queue

		for( i = 0; i < NUM_OF_PRN_PORT; i++) {
			if(PCBCount[i] == 0) continue;
			if( !(NDSPCBInfo[i] = (PortPCB *)calloc(PCBCount[i],sizeof(PortPCB)))  ) {
				rc = NDS_NOMEM;
				goto GetQErrExit;
			}
			for(j = 0 ; j < FSCount; j++) FileServerInfo[j].PrePrnNO = 0;

			for(j = 0 ; QueueInfo[i][j].QueueID ; j++) {

				if(QueueInfo[i][j].FSIndex == NULL_BYTE) {
					//Too many FS, discard this queue
					continue;
				}


				if(QueueInfo[i][j].FSIndex == ActiveFSIndex) {
					pPCBIndex =  &_NDSbActPort(i);
				} else {
					pPCBIndex =  &_NDSbFreePort(i);
				}

				if(FileServerInfo[QueueInfo[i][j].FSIndex].PrePrnNO++ != 0) {
					//Insert Q to exist PCB
					while(*pPCBIndex != NULL_BYTE) {
						if(NDSPCBInfo[i][*pPCBIndex].pFSInfo == &NDSFSInfo[QueueInfo[i][j].FSIndex]) break;
						*pPCBIndex = NDSPCBInfo[i][*pPCBIndex].NextPortPCB;
					}
#ifdef PC_OUTPUT
					while(*pPCBIndex == NULL_BYTE) {
						printf("NDS Insert Queue Design Error (1) !\n");
					}
					while(NGET32(NDSPCBInfo[i][*pPCBIndex].PCBQueueInfo[NDSPCBInfo[i][*pPCBIndex].TotalQueue].QueueObjectID) != 0) {
						printf("NDS Insert Queue Design Error (2) !\n");
					}
#endif PC_OUTPUT
				}
				else {
					//Add New PCB for New Q
#ifdef PC_OUTPUT
					while(PCBCount[i] == 0) {
						printf("(NDS) PCB Count design error (1) !\n");
					}
#endif PC_OUTPUT
					PCBCount[i]--;
					if(*pPCBIndex != NULL_BYTE) {
						NDSPCBInfo[i][PCBCount[i]].NextPortPCB = *pPCBIndex;
					} else {
						NDSPCBInfo[i][PCBCount[i]].NextPortPCB = NULL_BYTE;
					}

					*pPCBIndex = PCBCount[i];
					NDSPCBInfo[i][*pPCBIndex].pFSInfo = &NDSFSInfo[QueueInfo[i][j].FSIndex];
					NDSPCBInfo[i][*pPCBIndex].ProcessRoutine = ServiceNetWareQueue;
				}
#ifdef PC_OUTPUT
				memcpy(NDSPCBInfo[i][*pPCBIndex].PCBQueueInfo[_NDSvPCB(i,*pPCBIndex).TotalQueue].QueueName,QueueInfo[i][j].QueueName,MAX_NAME_LEN);
				NDSPCBInfo[i][*pPCBIndex].PCBQueueInfo[_NDSvPCB(i,*pPCBIndex).TotalQueue].QueueName[MAX_NAME_LEN-1] = '\0';
#endif PC_OUTPUT
				if(QueueInfo[i][j].FSIndex == ActiveFSIndex) {
					//This is a active FS just service QueueID
					NSET32(NDSPCBInfo[i][*pPCBIndex].PCBQueueInfo[_NDSvPCB(i,*pPCBIndex).TotalQueue++].QueueObjectID,
						QueueInfo[i][j].QueueID);
					free(QueueInfo[i][j].QueueName);
					QueueInfo[i][j].QueueName = NULL;
				} else {
					//Keep the Queue Name for Get real Queue ID
					NSET32((NDSPCBInfo[i][*pPCBIndex].PCBQueueInfo[_NDSvPCB(i,*pPCBIndex).TotalQueue++].QueueObjectID),
						(uint32)QueueInfo[i][j].QueueName);
				}
			}
#ifdef PC_OUTPUT
			while(PCBCount[i]) {
				printf("(NDS) PCB Count design error  (2) !\n");
			}
#endif PC_OUTPUT
			_NDSbCurPort(i) = _NDSbActPort(i);
		}

		if(ActiveFSIndex != NULL_BYTE) {

			//Copy this server info to FSInfo block ///
#ifdef PC_OUTPUT
			BYTE *TmpFileServerName =  NDSFSInfo[ActiveFSIndex].NDSTmpFileServerName;
#endif PC_OUTPUT

			memcpy(&NDSFSInfo[ActiveFSIndex],ConnInfo,sizeof(FSInfo));
			NDSFSInfo[ActiveFSIndex].NextFSInfo = NULL_BYTE;

#ifdef PC_OUTPUT
			NDSFSInfo[ActiveFSIndex].NDSTmpFileServerName = TmpFileServerName;
#endif PC_OUTPUT
			//(NDS Only)
			//When (PCBFileServerName != NULL) means
			//the Queue ID of this server is real ID
#ifdef PC_OUTPUT
			while(sizeof(NDSFSInfo[ActiveFSIndex].PCBFileServerName) != 4) {
				printf("(NDS) Pointer Size of PCBFileServerName Error (1) !\n");
			}
#endif PC_OUTPUT
			NDSFSInfo[ActiveFSIndex].PCBFileServerName = 0xFFFFFFFF;

			//Attach Active PCB to this server ///////
			for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
				if(_NDSbActPort(i) != NULL_BYTE) {
					NDSAttachQueueServer(NDSPsSocket,&NDSPCBInfo[i][_NDSbActPort(i)]);
				}
			}
			//Connect Flag ON
			NDSConnectFlag = 1;
		} else {
			//No any queue need service on Attach's FS !
			DisConnection(NDSPsSocket,ConnInfo);
		}

		rc = OKAY;
		goto GetQOKExit;
	}
#ifdef PC_OUTPUT
	printf("\aNo File Server for NDS !\n");
#endif PC_OUTPUT

	if(NDStatus != NDS_E_CAN_NOT_ATTACH_BINDERY_QUEUE)
		NDStatus = NDS_E_NO_ANY_QUEUE_ATTACHED;
	rc = NDS_NO_QUEUE_ATTACH;

GetQErrExit:

	DisConnection(NDSPsSocket,ConnInfo); //Fail
	if(NDSFSInfo) free(NDSFSInfo);
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		if(NDSPCBInfo[i]) free(NDSPCBInfo[i]);
		if(QueueInfo[i]) {
			for(j = 0 ; j < MAX_NDS_QUEUE_PER_PRN; j++) {
				if(QueueInfo[i][j].QueueName) free(QueueInfo[i][j].QueueName);
			}
		}
	}

GetQOKExit:
	if(buf) free(buf);
	if(buf1) free(buf1);
	if(FileServerInfo) free(FileServerInfo);
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		if(QueueInfo[i]) free(QueueInfo[i]);
	}

	return rc;
}

void NDSGetRealQueueID(PortPCB *PCBInfo)
{
	uni_char uniQueueContext[256];
	BYTE  *QueueContext;
	uint32 QueueID;
	int16  i = 0, j;

	while ( i < PCBInfo->TotalQueue) {
		QueueContext = *(BYTE**)PCBInfo->PCBQueueInfo[i].QueueObjectID;
		NDSc2u_strcpy(uniQueueContext,QueueContext);
		//Resolve printer queue name ==> Get Queue ID
		if(NDSResolveName(PCBInfo->pFSInfo, 0x62,uniQueueContext, &QueueID, NULL, NULL) != OKAY)
		{
			//Can not get this Queue ID, remove this Queue ID
			for(j = i+1; j < PCBInfo->TotalQueue; j++) {
				PCBInfo->PCBQueueInfo[j-1] = PCBInfo->PCBQueueInfo[j];
			}
			if(--PCBInfo->TotalQueue <= 0)
				NSET32(PCBInfo->PCBQueueInfo[0].QueueObjectID, 0);
			free(QueueContext);
			continue;
		}
		else {
			NSET32(PCBInfo->PCBQueueInfo[i].QueueObjectID, QueueID);
		}
		free(QueueContext);
		i++;
	}
}

void NDSMoveFreePortToActivePort(FSInfo *NDSpCurrentFSInfo)
{
	int16 i;

	BYTE NDSbPreFreePCB, NDSbCurFreePCB,NDSbTmpFreePCB;

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		NDSbPreFreePCB = NULL_BYTE;
		NDSbCurFreePCB = _NDSbFreePort(i);
		while(NDSbCurFreePCB != NULL_BYTE) {
			NDSbTmpFreePCB = NDSbCurFreePCB;
			NDSbCurFreePCB = NDSPCBInfo[i][NDSbCurFreePCB].NextPortPCB;

			if(NDSPCBInfo[i][NDSbTmpFreePCB].pFSInfo == NDSpCurrentFSInfo) {
				//Initial PCB data
				NDSPCBInfo[i][NDSbTmpFreePCB].QCount = 0;

				//Remove this PCB from FreePCB Link List //////////////////

				if(NDSbPreFreePCB == NULL_BYTE) {
					_NDSbFreePort(i) = NDSbCurFreePCB;
				} else {
					NDSPCBInfo[i][NDSbPreFreePCB].NextPortPCB = NDSbCurFreePCB;
				}

				//Add this PCB to ActivePCB Link List /////////////////////
				NDSPCBInfo[i][NDSbTmpFreePCB].NextPortPCB = _NDSbActPort(i);

				if(_NDSbActPort(i) == NULL_BYTE) {
					//If no any Active port before, start service it !
				    _NDSbCurPort(i) = NDSbTmpFreePCB;;
				}
				_NDSbActPort(i) = NDSbTmpFreePCB;

				if(NDSpCurrentFSInfo->PCBFileServerName == NULL) {
					NDSGetRealQueueID(&NDSPCBInfo[i][NDSbTmpFreePCB]);
#ifdef PC_OUTPUT
					while(sizeof(NDSpCurrentFSInfo->PCBFileServerName) != 4) {
						printf("(NDS) Pointer Size of PCBFileServerName Error (2) !\n");
					}
#endif PC_OUTPUT
				}
				NDSAttachQueueServer(NDSPsSocket,&NDSPCBInfo[i][NDSbTmpFreePCB]);
				break;
			}
			NDSbPreFreePCB = NDSbTmpFreePCB;
		} //while(NDSbCurFreePCB != NULL_BYTE) .......
	} //for(i = 0 ; i < NUM ......

	NDSpCurrentFSInfo->PCBFileServerName  = 0xFFFFFFFF;
}

WORD NDSHasAnyFileServerConnect(void)
{
	WORD i;

	if(NDStatus == NDS_E_BEGIN_LOGIN_TO_SERVER ||
	   NDStatus == NDS_E_PRE_CONNECTED )
	{
		//Before begin attach
		return (0);
	}

	NDSConnectFlag = 0;
	for (i=0 ; i < NUM_OF_PRN_PORT ; i++) {
		if(_NDSbActPort(i) != NULL_BYTE) {
			NDSConnectFlag = 1;
			break;
		}
	}

	return i;
}

void NDSTryConnectNoAttachFS (void)
{
	BYTE NDSbCurFSInfo,NDSbPreFSInfo;
	FSInfo *NDSpCurrentFSInfo;
	int32  rc;
	uint32 PrintServerID;
	WORD   FScount, IsFirst;

	IsFirst = FScount = 1;
	//First Time Try Attach to ALL FS
	NDSbPreFSInfo = NDSbNoActiveFSInfo;

	if(NDSbNoActiveFSInfo != NULL_BYTE) {
		//at least one server no connect.
		while((NDSFSInfo+NDSbPreFSInfo)->NextFSInfo != NDSbNoActiveFSInfo) {
			FScount++;
			NDSbPreFSInfo = (NDSFSInfo+NDSbPreFSInfo)->NextFSInfo;
		}
	} else {
		//All server connect.
#if (NUM_OF_STATUS_LED == 2)
		Light_Off(R_Lite|G_Lite);
		Light_On(G_Lite);
#endif
	}

	while (1) {
		if(NDSbNoActiveFSInfo != NULL_BYTE) {
			for(;FScount;FScount--) {
#ifdef PC_OUTPUT
				while(NDSbNoActiveFSInfo == NULL_BYTE) {
					printf("NDSTryConnectNoAttachFS Error !\n");
				}
#endif PC_OUTPUT
				if(!IsFirst) SetNetwareLED();

				NDSpCurrentFSInfo = NDSFSInfo+NDSbNoActiveFSInfo;
				if(NDSAttachToFS(NDSpCurrentFSInfo) != OKAY) {
#ifdef PC_OUTPUT
					AtSaySpace(0,18,80);
					printf("(NDS) Retry again : No Connect (%02X-%02X-%02X-%02X) !! ",
				    	               NDSpCurrentFSInfo->PCBNetworkNumber[0],
					                   NDSpCurrentFSInfo->PCBNetworkNumber[1],
					                   NDSpCurrentFSInfo->PCBNetworkNumber[2],
				    	               NDSpCurrentFSInfo->PCBNetworkNumber[3]
					);
#endif PC_OUTPUT
					goto TryConnectNextFS;
				}

				if( (rc = NDSLoginAuth(NDSpCurrentFSInfo,_PrintServerName,_NDSContext
				         ,_NovellPassword,&PrintServerID)) != OKAY)
				{
#ifdef PC_OUTPUT
					AtSaySpace(0,18,80);
					if (rc != NDS_PASSWORD_EXPIRED) {
						printf("(NDS) LoginAuth Fail %02x-%02x-%02x-%02x !",
				    		            NDSpCurrentFSInfo->PCBNetworkNumber[0],
				        		        NDSpCurrentFSInfo->PCBNetworkNumber[1],
				            		    NDSpCurrentFSInfo->PCBNetworkNumber[2],
			    	            		NDSpCurrentFSInfo->PCBNetworkNumber[3]
						);
					} else {
						printf("(NDS) LoginAuth Password Expired %02x-%02x-%02x-%02x !",
					    	            NDSpCurrentFSInfo->PCBNetworkNumber[0],
				   		    	        NDSpCurrentFSInfo->PCBNetworkNumber[1],
		    		    	    	    NDSpCurrentFSInfo->PCBNetworkNumber[2],
			    	        	    	NDSpCurrentFSInfo->PCBNetworkNumber[3]
						);
					}
#endif PC_OUTPUT
					goto TryConnectNextFS;
				}

#if (NUM_OF_STATUS_LED == 2)
				Light_Off(G_Lite|R_Lite);
				Light_On(G_Lite);
#endif
				// NDS login OK !!
				NDSConnectFlag = 1;

				//-- Remove this node from NDSNOActiveFSInfo Ring -------------
				NDSbPreFSInfo = NDSbNoActiveFSInfo;
				while((NDSFSInfo+NDSbPreFSInfo)->NextFSInfo != NDSbNoActiveFSInfo) {
					NDSbPreFSInfo = (NDSFSInfo+NDSbPreFSInfo)->NextFSInfo;
				}
				if(NDSbPreFSInfo == NDSbNoActiveFSInfo) {
					//Only one node in ring
					NDSbNoActiveFSInfo = NULL_BYTE;
				} else {
					//remove this node from ring
					NDSFSInfo[NDSbPreFSInfo].NextFSInfo=NDSpCurrentFSInfo->NextFSInfo;
				}
				NDSMoveFreePortToActivePort (NDSpCurrentFSInfo);

TryConnectNextFS:
				//shift bNoActiveFSInfo to next
				if(NDSbNoActiveFSInfo != NULL_BYTE)
					NDSbNoActiveFSInfo = NDSpCurrentFSInfo->NextFSInfo;
			} // for(;FScount;FScount--) { ....
		}  //if(NDSbNoActiveFSInfo != NULL_BYTE) ......

		if(IsFirst) {
			NDStatus = NDS_E_CONNECTED_OK;
			IsFirst = 0;
		}
		else {
			ppause(15000L);
		}
		FScount = 1;
	} //while (1) ...
}

//
//File Server TIME_OUT , move all active port of this FS to free port
//
void NDSMoveActivePortToFreePort (FSInfo *NDSpTimeOutFSInfo)
{
	BYTE NDSbPrePortPCB, NDSbCurPortPCB;
	PortPCB *NDSpCurPortPCB;
	BYTE NDSbCurFSInfo;
	BYTE nPort,SearchPortOk;

	SearchPortOk = 0;

	for(nPort=0 ; nPort<NUM_OF_PRN_PORT ; nPort++) {
		NDSbPrePortPCB = NULL_BYTE;
		NDSbCurPortPCB = _NDSbActPort(nPort);
		while(NDSbCurPortPCB != NULL_BYTE) {
			NDSpCurPortPCB= &NDSPCBInfo[nPort][NDSbCurPortPCB];
			if(NDSpTimeOutFSInfo == NDSpCurPortPCB->pFSInfo) {
				//-- Remove Queue from ActivePortPCB -------------

				if(NDSbPrePortPCB == NULL_BYTE) {
					//Remove first PCB
					_NDSbActPort(nPort) = NDSpCurPortPCB->NextPortPCB;
				}
				else {
					//Remove PCB
					NDSPCBInfo[nPort][NDSbPrePortPCB].NextPortPCB = NDSpCurPortPCB->NextPortPCB;
				}

				//--- Add Queue to FreePortPCB --------------------
				NDSpCurPortPCB->NextPortPCB = _NDSbFreePort(nPort);
				_NDSbFreePort(nPort) = NDSbCurPortPCB;

				SearchPortOk++;

				//TimeOut Port's FS = Current Port's FS

				if(NDSbCurPortPCB == _NDSbCurPort(nPort))
				{
					//Reset CurrentPortPCB
					_NDSbCurPort(nPort) = _NDSbActPort(nPort);

					//If TimeOut Port is printing ....
					if(PrnGetPrinterStatus(nPort) == NetwareUsed) {
						PrnAbortSpooler(nPort,NULL);
						PrnSetNoUse(nPort);
					}
				}

				break;
			}//if(strcmp()).......
			//--- Shift to next ActivePortPCB ------------------
#ifdef PC_OUTPUT
			if(NDSbCurPortPCB == NULL_BYTE) {
				AtSaySpace(0,20,80);
				printf("\a \a \a(NDS) NDSbCurPortPCB == NULL_BYTE, design error (NDSMoveActivePortToFreePort) !\a \a \a");
				ErrorBeep();
			}
#endif
			NDSbPrePortPCB = NDSbCurPortPCB;
			NDSbCurPortPCB = NDSpCurPortPCB->NextPortPCB;
		}//while( )...
	}//for(nPort= .....

	//---------- Add TimeOut's FS to NoActiveFS ---------------------
	if( SearchPortOk != 0 ) {
		//bNOActiveFSInfo is a ring
		NDSbCurFSInfo = NDSpTimeOutFSInfo - NDSFSInfo;	//get the Index of Time Out FS
		if(NDSbNoActiveFSInfo == NULL_BYTE) NDSbNoActiveFSInfo = NDSbCurFSInfo;

		NDSpTimeOutFSInfo->NextFSInfo = NDSFSInfo[NDSbNoActiveFSInfo].NextFSInfo;
		NDSFSInfo[NDSbNoActiveFSInfo].NextFSInfo = NDSbCurFSInfo;
		NDSbNoActiveFSInfo = NDSbCurFSInfo;
	}
}

void SetNetwareLED(void)
{

#if (NUM_OF_STATUS_LED == 2)
	Light_On(R_Lite);
	ppause(10);
	if(HasAnyFileServerConnect() < NUM_OF_PRN_PORT ||
		NDSHasAnyFileServerConnect() < NUM_OF_PRN_PORT )
	{
		Light_On(G_Lite);
		Light_Off(R_Lite);
	} else {
		Light_Off(G_Lite);
	}
#endif
}

void BEGIN_NETWARE_CRITICAL(BYTE value)
{
#ifdef PC_OUTPUT
	while(!value) printf("BEGIN_NETWARE_CRITICAL error !\n");
#endif PC_OUTPUT

    while(NetWareReEntry){
//615wu    kwait(NULL);
		cyg_thread_yield();
	}
	NetWareReEntry = value;
}

void END_NETWARE_CRITICAL(void)
{
#ifdef PC_OUTPUT
	while(!NetWareReEntry) printf("END_NETWARE_CRITICAL error !\n");
#endif PC_OUTPUT

	NetWareReEntry = 0;
}


#endif NDS_PS
