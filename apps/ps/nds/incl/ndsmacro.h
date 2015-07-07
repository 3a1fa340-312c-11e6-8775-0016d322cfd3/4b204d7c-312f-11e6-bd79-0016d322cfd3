#ifndef NDSMACRO_H
#define NDSMACRO_H

//for NDS
//-------------------------------------------------------------------------
#define _NDSbActPort(nPort)        (PortInfo[nPort].NDSActivePortPCB)
#define _NDSbCurPort(nPort)        (PortInfo[nPort].NDSCurrentPortPCB)
#define _NDSbFreePort(nPort)       (PortInfo[nPort].NDSFreePortPCB)

#define _NDSpCurrentPortPCB(nPort) (&NDSPCBInfo[nPort][_NDSbCurPort(nPort)])

#define _NDSvPCB(nPort,Num)        (NDSPCBInfo[nPort][Num])
//-------------------------------------------------------------------------

#endif  NDSMACRO_H
