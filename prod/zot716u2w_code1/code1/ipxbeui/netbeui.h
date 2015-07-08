#ifndef _NETBEUI_H
#define _NETBEUI_H

#include "ipx.h"

extern SocketNumberTable     socketTable[SOCKET_TABLE_LENGTH]; //in ipx.c
extern WORD socketTablePointer;	 //in ipx.c


#define NETBEUI_ListViewSocket              0x7070
#define NETBEUI_MULTICAST  "\x03\x00\x00\x00\x00\x01"
#define MY_NETBEUI_NAME	  "ZOT-N63-"

typedef struct {
	BYTE        Length[2];
	BYTE        Delimiter[2];
	BYTE        Command;
	BYTE        Data1;
	BYTE        Data2[2];
	BYTE        Xmit[2];
	BYTE        Resp[2];
	BYTE        DestName[16];
	BYTE        SrcName[16];
}PACK NETBEUIHeader;

#endif _NETBEUI_H
