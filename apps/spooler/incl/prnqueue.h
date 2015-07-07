#ifndef _LPDPRN_H
#define _LPDPRN_H

//#define NUM_OF_PRN_PORT  3 //10/28/98 Simon remarked

#define NULL_BYTE        0xFF

//printer queue buffer status
#define PRN_Q_NORMAL     (0)	   //Normal case
#define PRN_Q_EOF        (1)	   //End of file for printing
#define PRN_Q_ABORT      (2)	   //Abort printing
#define PRN_Q_HOLD       (3)	   //Hold printing

typedef struct prnbuf {
	int    size;				/* Size of associated data buffer */
	uint8  *data;		        /* working data */
	uint8  flags;				//PRN_Q_NORMAL | PRN_Q_EOF | PRN_Q_ABORT
	uint8  Nextbuf;
} PrnBuf;

typedef struct printstatus {
	char  *PrinterName;
	PrnBuf buf[PRNQUEUELEN];
	PrnBuf reverse;
	uint8  InUse;
	uint8  HoldMode;
	uint8  OutQueueHead;
	uint8  OutQueueTail;
	uint8  InQueue;
	uint8  AvailQueueNO;
	uint8  IsPrinting;		 //9/3/98
	DWORD  IsPrintStartTime; //9/3/98
} PrintStatus;

//--------------------------
//printer queue InUse status
//(crrent used 0x00 - 0x0F )
#define PrnNoUsed       0x00
#define PrnAbortUsed    0x01
#define NTUsed          0x02
#define NetwareUsed     0x03
#define UnixUsed        0x04
#define AtalkUsed   	0x05 //3/10/99 for APPLETALK
#define IppUsed         0x06 //7/20/2000 added
#define SMBUsed         0x07 //8/18/2000 added
#define RawTcpUsed		0x08 // 2001/10/31 added
#define FtpUsed			0x09 // 2001/10/31 added
#define UpgradeUsed		0x0A // 2002/01/31 added 
#define TestPageUsed	0x0B 
#define DeviceIDUsed    0x10 //8/8/2000 change id value, 3/17/2000 for IEEE1284 GET DEVICE ID
#define NTReUsed        0x80 //for NTUDP.C only

//(hold mode 0x01, 0x02, 0x04, 0x08 )
#define NoJobHolded       0x00
#define NetwareHolded     0x01
#define NTHolded          0x02
#define UnixHolded        0x04
#define AtalkHolded       0x08
#define NetwareHoldAbort  0x10
#define NTHoldAbort       0x20
#define UnixHoldAbort     0x40
#define AtalkHoldAbort	  0x80
//sometime = UNIXHOLD + UNIXHOLDABORT (0x44)
//sometime = UNIXHOLD + NTHOLD (0x22)
//sometime = UNIXHOLD + NTHOLD + NTHOLDABORT + UNIXHOLDABORT (0x66)
//------------------------------------------------------------------

#define PrnSetNoUse(Port)          PrnSetQueueInUse(Port,PrnNoUsed);
#define PrnSetAbortUse(Port)       PrnSetQueueInUse(Port,PrnAbortUsed);
#define PrnSetNTInUse(Port)        PrnSetQueueInUse(Port,NTUsed);
#define PrnSetNetwareInUse(Port)   PrnSetQueueInUse(Port,NetwareUsed);
#define PrnSetUnixInUse(Port) 	   PrnSetQueueInUse(Port,UnixUsed);
#define PrnSetAtalkInUse(Port)     PrnSetQueueInUse(Port,AtalkUsed);
#define PrnSetDeviceIDInUse(Port)  PrnSetQueueInUse(Port,DeviceIDUsed);
#define PrnSetIppInUse(Port)       PrnSetQueueInUse(Port,IppUsed);
#define PrnSetRawTcpInUse(Port)	   PrnSetQueueInUse(Port,RawTcpUsed);
#define PrnSetFtpInUse(Port)	   PrnSetQueueInUse(Port,FtpUsed);
#define PrnSetSMBInUse(Port)       PrnSetQueueInUse(Port,SMBUsed); //6/28/2001 by ron
#define PrnSetUpgradeInUse(Port)   PrnSetQueueInUse(Port,UpgradeUsed);
#define PrnSetTestPageInUse(Port)  PrnSetQueueInUse(Port,TestPageUsed);
#if 0	//615wu::No PSMain
#define PrnSetNTHold(Port) 	       PrnSetJobHold(Port,NTHolded);
#define PrnSetNetwareHold(Port)    PrnSetJobHold(Port,NetwareHolded);
#define PrnSetUnixHold(Port)	   PrnSetJobHold(Port,UnixHolded);
#define PrnSetAtalkHold(Port)      PrnSetJobHold(Port,AtalkHolded);

#define PrnSetNTHoldAbort(Port) 	 PrnSetJobHold(Port,NTHoldAbort);
#define PrnSetNetwareHoldAbort(Port) PrnSetJobHold(Port,NetwareHoldAbort);
#define PrnSetUnixHoldAbort(Port) 	 PrnSetJobHold(Port,UnixHoldAbort);

#define PrnSetNTUnHold(Port) 	   PrnSetJobUnHold(Port,(NTHolded|NTHoldAbort));
#define PrnSetNetwareUnHold(Port)  PrnSetJobUnHold(Port,(NetwareHolded|NetwareHoldAbort));
#define PrnSetUnixUnHold(Port) 	   PrnSetJobUnHold(Port,(UnixHolded|UnixHoldAbort));
#define PrnSetAtalkUnHold(Port)    PrnSetJobUnHold(Port,(AtalkHolded|AtalkHoldAbort));

#define PrnIsNTHold(Port)         (PrnGetJobHoldStatus(Port) & NTHolded)
#define PrnIsNetwareHold(Port)    (PrnGetJobHoldStatus(Port) & NetwareHolded)
#define PrnIsUnixHold(Port)       (PrnGetJobHoldStatus(Port) & UnixHolded)

#define PrnIsNTHoldAbort(Port)      (PrnGetJobHoldStatus(Port) & NTHoldAbort)
#define PrnIsNetwareHoldAbort(Port) (PrnGetJobHoldStatus(Port) & NetwareHoldAbort)
#define PrnIsUnixHoldAbort(Port)    (PrnGetJobHoldStatus(Port) & UnixHoldAbort)
#endif
#define PrnAbortMessage  			"\n /* abort */"

#define PrnPrinterPortIsBusy(Port) \
        (PrnGetPrinterStatus((Port)) !=	PrnNoUsed)

//12/22/98 changed
#define PrnOutQueueIsEmpty(Port)  \
         ( (PrnGetPrinterStatus((Port)) == PrnNoUsed) && \
           (PrnGetAvailQueueNO((Port)) == PRNQUEUELEN) )

void            PrnQueueInit(void);
void 			PrnPrinterInit(void);
int 		    PrnCheckPrinter(char *name);
char           *PrnGetPrinterName(int Prnport);
short 			PrnGetPrinterStatus(int PrnPort);
short           PrnGetAvailQueueNO(int PrnPort);
void 			PrnAbortSpooler(int PrnPort,PrnBuf *);
void 			PrnSetQueueInUse(int PrnPort,int value);
PrnBuf         *PrnGetInQueueBuf(int Port);
void 			PrnPutOutQueueBuf(int PrnPort,PrnBuf *Buf, int flags);
void 			PrnPutInQueueBuf(int PrnPort,PrnBuf *pbuf);  //4/21/99
PrnBuf		   *PrnGetReverseQueueBuf(int PrnPort);
void 			PrnStartSpooler(cyg_addrword_t data);
BYTE            DMAPrinting(int PrnPort);
BYTE			FastDMAPrepare(int PrnPort);
BYTE			FastDMAPrinting(int PrnPort);
short           PrnGetJobHoldStatus(int PrnPort);
void            PrnSetJobUnHold(int PrnPort,int value);
void            PrnSetJobHold(int PrnPort,int value);
BYTE            PrnPrintPattern(BYTE PrnPort,BYTE *Data, WORD wSize);// 2001/6/22 changed
void            PrnSetPrinting(void);
short           PrnIsPrinting(int PrnPort);

#endif _LPDPRN_H
