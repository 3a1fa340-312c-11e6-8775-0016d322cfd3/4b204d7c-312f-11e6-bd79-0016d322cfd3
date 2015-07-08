#ifndef _NDSMAIN_H
#define _NDSMAIN_H

WORD NDSAttachToFS(FSInfo *FSInfoPointer);
void NDSListenWatchDog (void);
WORD RequestConnect2Server(WORD Socket,FSInfo *FSInfoPointer);
WORD SendNotifyMessage(WORD Socket,BYTE PortNo, PortPCB *pPortPCB);

extern BYTE NDStatus;  //for display in HTTP

#endif  _NDSMAIN_H
