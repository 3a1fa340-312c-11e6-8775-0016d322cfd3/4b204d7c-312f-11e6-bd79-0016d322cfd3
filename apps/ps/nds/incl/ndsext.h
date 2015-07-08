#ifndef _NDSEXT_H
#define _NDSEXT_H

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "ipx.h"
#include "nps.h"

//define in NDSmain.c
extern NCP_NDS_STRUCT *SendNDSData;

//define in NOVELL\NPS3.C
extern BYTE  BrocastNetwork[];
extern BYTE  BrocastNode[];

extern IPXHeader     SendPsIPXHeader,   ReceivePsIPXHeader;
extern BYTE         *SendNCPSubData;
extern BYTE         *ReceiveNCPSubData;
extern BYTE          ReceivePsIPXData[];
extern BYTE          SendPsIPXData[];
extern ECB           SendPsECB,         ReceivePsECB;
extern NCPQueryData *SendNCPData;

extern int16 CurrentFrameType; //Current Netware Frame Type Number
extern WORD  status;

extern ECB           SendWDogECB;
extern IPXHeader     SendWDogIPXHeader;
extern WatchDog_Rec  SendWDogData;

void ListenPsECB (WORD SockNumber, WORD ReceiveSize);
void SendPsECBInit (WORD SocketNumber, BYTE *TargetAddress,WORD SendSize);
WORD RequestRIP (WORD Socket, FSInfo *FSInfoPointer);
WORD RequestCreateServiceConnect (WORD Socket, FSInfo *FSinfoPointer);
WORD NCPRequest (WORD Socket, FSInfo *FSInfoPointer);
WORD RequestNegotiateBufferSize (WORD Socket, FSInfo  *FSInfoPointer);
WORD DisConnection (WORD Socket, FSInfo *FSInfoPointer);

#endif  _NDSEXT_H
