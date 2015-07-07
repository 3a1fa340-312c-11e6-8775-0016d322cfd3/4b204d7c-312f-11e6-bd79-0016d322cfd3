#ifndef _PRNPORT_H
#define _PRNPORT_H

//--- Printer Status ----------------------------------------
#define	PORT_READY      0x00   //READY
#define PORT_PAPER_OUT  0x01   //PAPEROUT
#define PORT_OFF_LINE   0x02   //OFFLINE
#define PORT_PRINTING   0x03   //BUSY
#define PORT_FAULT      0x04   //FAULT
#define PORT_NO_CONNECT 0x05   //NOCONNECT

//--- Printer support language (IEEE1284) -------------------
#define P1284_PJL				0x01
#define P1284_PCL				0x02
#define P1284_POSTSCRIPT		0x04
#define P1284_START_PJL 		0x08

//--- Current Printer initial mode --------------------------
#define PRN_NO_PRINTER			0x00
#define PRN_SPP_PRINTER 		0x01
#define PRN_ECP_PRINTER 		0x02
#define PRN_USB_PRINTER			0x03

//Device Control Register (DCR)
#define PARPORT_CONTROL_STROBE    		0x1
#define PARPORT_CONTROL_AUTOFD    		0x2
#define PARPORT_CONTROL_INIT      		0x4
#define PARPORT_CONTROL_SELECT    		0x8
#define PARPORT_CONTROL_ACKINT    		0x10
#define PARPORT_CONTROL_DIRECTION 		0x20

//Device Status Register (DSR)
#define PARPORT_STATUS_ERROR      		0x8
#define PARPORT_STATUS_SELECT     		0x10
#define PARPORT_STATUS_PAPEROUT   		0x20
#define PARPORT_STATUS_ACK        		0x40
#define PARPORT_STATUS_BUSY       		0x80

//--- for Bidirectional and IEEE1284mode -------------------
#define P1284_ITEM_ECP			0x02
#define P1284_ITEM_SPP			0x00
#define P1284_ITEM_AUTO 		0x01
#define P1284_ITEM_DISABLE		0x00

extern BYTE G_PortReady;

// A parallel port
struct parport {
//PortInfo:
	UINT   base;	  // base address
	BYTE   ctr; 	  // Device Control Register
//	UINT   ieee1284_mode;
//	enum   ieee1284_phase ieee1284_phase;
	BYTE   PrnMode; 		//Current printing mode :(NO, SPP printing, ECP printing)
	BYTE   PrnReadBackMode; //Current printing mode :(NO, SPP printing, ECP printing)
//DeviceNofo:
	BYTE   HasGetDeviceID; //0: not yet , 1: at least get once
	BYTE   SupportLang;    //Printer support language (PJL, PCL, POSTSCRIPT)
	BYTE  *Manufacture;    //MANUFACTURER
	BYTE  *Model;		   //MODEL
	BYTE  *CommandSet;	   //COMMAND SET
	BYTE  *Description;	   //DESCRIPTION

//	BYTE   InECPMode;
	BYTE   PortStatus;
	DWORD  BlockCount;
// for paper-out 
	int    PortTimeout;
	int	   TotalSize;
//	int    (*PrnWrite)(int nPort, uint8 *pBuf, int nLength);
	int    DataSize;
	BYTE  *DataPtr;
};

BYTE PrnLangSupport(BYTE port, BYTE Lang);
BYTE ReadPortStatus(BYTE port);
BYTE ReadPrintStatus (void);
BYTE StartDMAIO(BYTE *SrcBuf, WORD DataSize ,BYTE Port);

#define PortReady(port) (RealPortStatus((port)) == 0xDF)
BYTE RealPortStatus(BYTE port);

#endif  _PRNPORT_H
