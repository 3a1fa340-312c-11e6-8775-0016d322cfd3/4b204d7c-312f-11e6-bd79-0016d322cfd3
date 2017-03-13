#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "prnqueue.h"
#include "psdefine.h"
#include "prnport.h"
#if defined(IPPD) && defined(LPD_TXT)
#include "ippd.h"
#endif

extern char * strdup(const char *str);
extern void restore( int is );
extern int dirps();

static PrnBuf *pNoUseBuf[NUM_OF_PRN_PORT];
static PrnBuf *PrnGetOutQueueBuf(int PrnPort);
static struct printstatus PrinterList[NUM_OF_PRN_PORT];
cyg_mutex_t NoUseBuf_mutex;

#ifdef CONST_DATA
//move this table to constant.c 3/24/98
#else
const char *PrnUsedMessage[]= {
	 " is available !"                   //0
	," is removing current job !"        //1
	," is used by Windows !"             //2  --+--- Swap 5/13/99
	," is used by NetWare !"             //3  --+
	," is used by Unix (LPD), job#=%s"   //4 9/7/98
	," is used by Mac (ATALK) !"         //5 3/25/99
	," is used by IPPD, job#=%d(%d) !"   //6 8/8/2000
	," is used by SMB !"				 //7 8/18/2000 added
	," is used by RawTcp !"              //8 10/31/2001 added
	," is used by Ftp !"                 //9 10/31/2001 added
	," is used by Upgrade !"             //10 10/31/2001 added
	," is used by Test Pag !"            //11 
};
#endif

///////////////////////////
// Initial Printer Queue //
///////////////////////////
cyg_mutex_t pq_mutex[NUM_OF_PRN_PORT];

void PrnQueueInit(void)
{
	char *pBuf = PRINT_QUEUE_ADDRESS;
	int i, j;

#ifndef USB_ZERO_CPY
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++)
	{
		for(j = 0 ; j < PRNQUEUELEN ;j++)
		{
			PrinterList[i].buf[j].data = pBuf;
			pBuf += ( BLOCKSIZE + 20 );
		}
//ZOTIPS		PrinterList[i].reverse.data = pBuf;
//ZOTIPS		pBuf += ( BLOCKSIZE + 20 );

	}
#else
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++)
	{
		for(j = 0 ; j < PRNQUEUELEN ;j++)
		{
			PrinterList[i].buf[j].data = pBuf;
            pBuf += (1460*10);
            // pBuf += 8192;
		}
	}
#endif		
}

/////////////////////
// Printer initial //
/////////////////////
void PrnPrinterInit( void )
{
	int PrnPort,i;
	char *PrinterName= G_PRN_PORT_NAME;

	for(PrnPort = 0;PrnPort < NUM_OF_PRN_PORT;PrnPort++) {
		PrinterName[G_PRN_PORT_POS] = PrnPort + '1';
		PrinterList[PrnPort].PrinterName  = strdup(PrinterName);
		PrinterList[PrnPort].OutQueueHead = NULL_BYTE;
		PrinterList[PrnPort].OutQueueTail = NULL_BYTE;
		PrinterList[PrnPort].InUse        = PrnNoUsed;
		PrinterList[PrnPort].HoldMode     = NoJobHolded;
		PrinterList[PrnPort].AvailQueueNO = PRNQUEUELEN;
		PrinterList[PrnPort].InQueue 	  = 0;
		//DMAPrinting used index queue
		pNoUseBuf[PrnPort] 				  = 0;
		
		for(i = 0 ; i < PRNQUEUELEN-1; i++) {
			PrinterList[PrnPort].buf[i].Nextbuf = i+1;
		}
		PrinterList[PrnPort].buf[i].Nextbuf = NULL_BYTE;
#ifdef PQ_USE_MUTEX
        cyg_mutex_init(&pq_mutex[PrnPort]);
#endif /* PQ_USE_MUTEX */
	}

    cyg_mutex_init(&NoUseBuf_mutex);
}

void DumpPrintQueue ()
{
    int PrnPort, i;

	for(PrnPort = 0;PrnPort < NUM_OF_PRN_PORT;PrnPort++) {
        diag_printf("=================================================\n");
        diag_printf("PrinterName:%s\n", PrinterList[PrnPort].PrinterName);
        diag_printf("OutQueueHead:%d\n", PrinterList[PrnPort].OutQueueHead);
        diag_printf("OutQueueTail:%d\n", PrinterList[PrnPort].OutQueueTail);
        diag_printf("Inuse       :%x\n", PrinterList[PrnPort].InUse);
        diag_printf("HoldMode    :%x\n", PrinterList[PrnPort].HoldMode);
        diag_printf("AvailOueueNO:%d\n", PrinterList[PrnPort].AvailQueueNO);
        diag_printf("InQueue     :%d\n", PrinterList[PrnPort].InQueue);
        diag_printf("pNoUseBuf   :%x\n", (unsigned int)pNoUseBuf[PrnPort]);
        diag_printf("=================================================\n");
    }

    for(i = 0 ; i < PRNQUEUELEN-1; i++) {
        diag_printf("------- PrnQueue %d -------\n", i);
        diag_printf("buf addr:%x\n", (unsigned int)PrinterList[0].buf[i].data);
        diag_printf("buf flag:%x\n", (unsigned int)PrinterList[0].buf[i].flags);
        diag_printf("---------------------------\n");
    }
}

//////////////////////////////////////////////////////////////////
// Check printer , if found return port number else return < 0	//
//////////////////////////////////////////////////////////////////
int
PrnCheckPrinter( char *name )
{
	int i;

#if (NUM_OF_PRN_PORT == 1)	//12/30/98
#if defined(IPPD) && defined(LPD_TXT)
	if((i = strlen(name)) >= (sizeof(IPP_TXT_PRINTER_EXT_FORMAT)-1)) {
		if(!stricmp(&name[i+1-sizeof(IPP_TXT_PRINTER_EXT_FORMAT)],IPP_TXT_PRINTER_EXT_FORMAT)) {
			return (0x10);	//High Nibble = TXT mode or not !
		}
	}
#endif
	return (0); //Don't check LP name, always true !
#else
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		//Case insensitive 10/28/98 Simon
		if(stricmp(PrinterList[i].PrinterName, name) == 0 ) {
			return(i);
		}
#if defined(IPPD) && defined(LPD_TXT)
		if(!memicmp(PrinterList[i].PrinterName,name,strlen(PrinterList[i].PrinterName)) &&
		   !stricmp(IPP_TXT_PRINTER_EXT_FORMAT,name+strlen(PrinterList[i].PrinterName))
		) {
			return (0x10+i);  //High Nibble = TXT mode or not !
		}
#endif
	}
	return (-1);
#endif
}
//////////////////////
// Get printer name //
//////////////////////
char *PrnGetPrinterName( int Prnport )
{
	return( PrinterList[Prnport].PrinterName );
}

/////////////////////////////
// Get printer hold status //
/////////////////////////////
short PrnGetJobHoldStatus( int PrnPort )
{
	return ( PrinterList[PrnPort].HoldMode );
}

////////////////////////
// Get Printer Status //
////////////////////////
short PrnGetPrinterStatus( int PrnPort )
{
	return ( PrinterList[PrnPort].InUse );
}

////////////////////////////////////
// Get Number of Available Queue  //
////////////////////////////////////
short
PrnGetAvailQueueNO( int PrnPort )
{
	return ( PrinterList[PrnPort].AvailQueueNO );
}

///////////////////////////////////////////////////////
// Abort printing, set abort status for queue buffer //
///////////////////////////////////////////////////////
void PrnAbortSpooler( int PrnPort, PrnBuf *pbuf )
{
	if(PrinterList[PrnPort].InUse == PrnNoUsed) return ;

	if(pbuf == NULL)
		while((pbuf = PrnGetInQueueBuf(PrnPort)) == NULL);
	
	pbuf->size = 0;
	PrnPutOutQueueBuf(PrnPort,pbuf,PRN_Q_ABORT);
}

///////////////////////////////////////////////////////
// Set Print UnHold by NT, Netware, Unix ....        //
///////////////////////////////////////////////////////
void PrnSetJobUnHold( int PrnPort, int value )
{
	int i_state;

#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#else
	i_state = dirps();
#endif /* PQ_USE_MUTEX */

	PrinterList[PrnPort].HoldMode &= (~value);

#ifdef PQ_USE_MTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
	restore(i_state);
#endif /* PQ_USE_MUTEX */
}

/////////////////////////////////////////////////////
// Set Print Hold by NT, Netware, Unix ....        //
/////////////////////////////////////////////////////
void PrnSetJobHold( int PrnPort, int value )
{
	int i_state;

#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#else
	i_state = dirps();
#endif /* PQ_USE_MUTEX */
	
	PrinterList[PrnPort].HoldMode |= value;
	
#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
	restore(i_state);
#endif /* PQ_USE_MUTEX */
}

///////////////////////////////
// Set printer InUse status  //
///////////////////////////////
void
PrnSetQueueInUse( int PrnPort, int value )
{
	int i_state;

#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#else
	i_state = dirps();
#endif /* PQ_USE_MUTEX */

	PrinterList[PrnPort].InUse = value;

#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
	restore(i_state);
#endif /* PQ_USE_MUTEX */
}

////////////////////////////////////////////////////////
// Get available queue buffer for input data,		  //
// Return   : Queue buffer (or NULL)                  //
//            (if no more free buffer,                //
//             it will return NULL    )               //
// Warning : After save the data into queue buffer    //
//           MUST call PrnPutOutQueueBuf() to print   //
//           this buffer out                          //
////////////////////////////////////////////////////////
PrnBuf * PrnGetInQueueBuf( int PrnPort )
{
#ifndef PQ_USE_MUTEX
	int i_state = dirps(); //12/7/98
#endif /* ! PQ_USE_MUTEX */
		
	uint8 InQueueNo;
	
	if(PrinterList[PrnPort].InQueue == NULL_BYTE) {
#ifndef PQ_USE_MUTEX
		restore(i_state);  //12/7/98
#endif /* ! PQ_USE_MUTEX */
		return NULL;
	}
#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#endif /* PQ_USE_MUTEX */
	PrinterList[PrnPort].AvailQueueNO--;
	InQueueNo = PrinterList[PrnPort].InQueue;
	PrinterList[PrnPort].InQueue = PrinterList[PrnPort].buf[InQueueNo].Nextbuf;
#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
//	armond_printf("Queue Buffer AvailQueueNO:%d\n",PrinterList[PrnPort].AvailQueueNO);
	restore(i_state);  //12/7/98
#endif /* PQ_USE_MUTEX */
	return((PrinterList[PrnPort].buf)+InQueueNo);
}

////////////////////////////////////////////////////////
// Put available queue buffer for output data,		  //
// this buffer get from PrnGetInQueueBuf()            //
////////////////////////////////////////////////////////
void PrnPutOutQueueBuf( int PrnPort, PrnBuf *Buf, int flags )
{
    uint8 OutQueueTail;
    uint8 CurQueueNO;
#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#else
	int i_state = dirps(); //12/7/98
#endif /* PQ_USE_MUTEX */

	OutQueueTail = PrinterList[PrnPort].OutQueueTail;
	CurQueueNO = Buf - PrinterList[PrnPort].buf;

	Buf->Nextbuf = NULL_BYTE;
	Buf->flags = flags;
	PrinterList[PrnPort].OutQueueTail = CurQueueNO;

	if(OutQueueTail == NULL_BYTE) {
		PrinterList[PrnPort].OutQueueHead = CurQueueNO;
	} else {
		PrinterList[PrnPort].buf[OutQueueTail].Nextbuf = CurQueueNO;
	}
#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
	restore(i_state);  //12/7/98
#endif /* PQ_USE_MUTEX */
}

////////////////////////////////////////////////////////
// Get output queue data for output data		      //
// Return   : Queue buffer (or NULL)                  //
//            (if no any data need output)            //
// Warning : After output queue buffer                //
//           MUST call PrnPutInQueueBuf() to release  //
//           this buffer                              //
////////////////////////////////////////////////////////
static PrnBuf * PrnGetOutQueueBuf( int PrnPort )
{

	uint8 OutQueueNo;
#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#else
	int i_state = dirps(); //12/7/98
#endif /* PQ_USE_MUTEX */

	if(PrinterList[PrnPort].OutQueueHead == NULL_BYTE)
	{
#ifdef PQ_USE_MUTEX
        cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
		restore(i_state);  //12/7/98
#endif /* PQ_USE_MUTEX */
		return NULL;
	}

	OutQueueNo = PrinterList[PrnPort].OutQueueHead;
	PrinterList[PrnPort].OutQueueHead = PrinterList[PrnPort].buf[OutQueueNo].Nextbuf;
	if( PrinterList[PrnPort].OutQueueHead == NULL_BYTE )
	    PrinterList[PrnPort].OutQueueTail = NULL_BYTE;

#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
	restore(i_state);  //12/7/98
#endif /* PQ_USE_MUTEX */
	return((PrinterList[PrnPort].buf)+OutQueueNo);
}

////////////////////////////////////////////////////////
// Release queue buffer for input data,		          //
// the release buffer get from PrnGetOutQueueBuf()    //
////////////////////////////////////////////////////////
void PrnPutInQueueBuf( int PrnPort, PrnBuf *Buf )
{
	
	uint8 InQueue;
	uint8 CurQueueNO = Buf - PrinterList[PrnPort].buf;
#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#else
	int i_state = dirps(); //12/7/98
#endif /* PQ_USE_MUTEX */

	InQueue = PrinterList[PrnPort].InQueue;
	PrinterList[PrnPort].InQueue = CurQueueNO;
	PrinterList[PrnPort].buf[CurQueueNO].Nextbuf = InQueue;
	PrinterList[PrnPort].AvailQueueNO++;

#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#else
	restore(i_state);  //12/7/98
#endif /* PQ_USE_MUTEX */
}

PrnBuf * PrnGetReverseQueueBuf( int PrnPort )
{
	return &PrinterList[PrnPort].reverse;
}

////////////////////////////////
// Start Print buffer spooler //
////////////////////////////////

void PrnStartSpooler(cyg_addrword_t data)
{
	BYTE PortMask;
	int PrnPort;
	
	while(1){
		for(PrnPort = 0; PrnPort < NUM_OF_PRN_PORT; PrnPort++)
		{
			PortMask = 1 << PrnPort; //1/22/99
			if( G_PortReady & PortMask )
			{
				if( DMAPrinting(PrnPort) )
					G_PortReady &= ~PortMask;
			}
		}
//ZOT716u2		cyg_thread_yield();
        sys_check_stack();
		cyg_thread_delay(100);	//ZOT716u2
	}
	
}

BYTE DMAPrinting(int PrnPort)
{
	PrnBuf *pBuf;

    cyg_mutex_lock(&NoUseBuf_mutex);
	if(pNoUseBuf[PrnPort]) PrnPutInQueueBuf(PrnPort,pNoUseBuf[PrnPort]);
    cyg_mutex_unlock(&NoUseBuf_mutex);

	if((pBuf = PrnGetOutQueueBuf(PrnPort)) == NULL || pBuf->size == 0) {
		if(pBuf) {
            diag_printf("%s(%d) pbuf size is 0\n", __func__, __LINE__);
			PrnPutInQueueBuf(PrnPort,pBuf); //size = 0			
		}
        cyg_mutex_lock(&NoUseBuf_mutex);
		pNoUseBuf[PrnPort] = NULL;
        cyg_mutex_unlock(&NoUseBuf_mutex);
		return (0);
	}

    cyg_mutex_lock(&NoUseBuf_mutex);
	pNoUseBuf[PrnPort] = pBuf;
    cyg_mutex_unlock(&NoUseBuf_mutex);
	
    return StartDMAIO(pBuf->data, pBuf->size, PrnPort);
	
}

///////////////////////////////////
// Direct Output Pattern to DMA  //
///////////////////////////////////

BYTE PrnPrintPattern(BYTE PrnPort, BYTE *Data, WORD wSize)
{
	BYTE PortMask = 1 << PrnPort;
	PrnBuf *pBuf;
	int i_state;

	if(!(G_PortReady & PortMask)) return (0xFF);

	if(PrnGetPrinterStatus(PrnPort) != PrnNoUsed ||
	   PrnGetAvailQueueNO(PrnPort) < 1 ) return (0xFF);

	while( (pBuf = PrnGetInQueueBuf(PrnPort)) == NULL);
	memcpy( pBuf->data,Data, wSize );
	pBuf->size = wSize;

	//  i_state = dirps();
//	cyg_scheduler_lock();
	
#ifdef PQ_USE_MUTEX
    cyg_mutex_lock(&pq_mutex[PrnPort]);
#endif /* PQ_USE_MUTEX */
	G_PortReady &= ~PortMask;
#ifdef PQ_USE_MUTEX
    cyg_mutex_unlock(&pq_mutex[PrnPort]);
#endif /* PQ_USE_MUTEX */

	StartDMAIO(pBuf->data, pBuf->size, PrnPort);

//	cyg_scheduler_unlock();
	//  restore(i_state);

	PrnPutInQueueBuf(PrnPort,pBuf);

	return (0);
}

////////////////////////////////
// Set Printing Status 9/3/98 //
////////////////////////////////
void PrnSetPrinting(void)
{
	int PrnPort;

	for(PrnPort = 0 ; PrnPort < NUM_OF_PRN_PORT; PrnPort++) {
		if((rdclock() - PrinterList[PrnPort].IsPrintStartTime) > TICKS_PER_SECOND){
			PrinterList[PrnPort].IsPrinting = 0;
		}
	}
}

////////////////////////////////
// Get Printing Status 9/3/98 //
////////////////////////////////
short PrnIsPrinting(int PrnPort)
{
	return(PrinterList[PrnPort].IsPrinting);
}

