#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "prnqueue.h"
#include "prnport.h"
#include "ipx.h"
#include "ntps.h"
#ifdef NTUDP
#include "ntudp.h"
#endif
#include "nps.h"

#ifndef USE_PS_LIBS
#undef NOVELL_PS
#undef WINDOWS_PS
#endif

#if defined(WINDOWS_PS)
#define MAX_U32_VALUE	4294967295

//615wu::No PSMain
cyg_sem_t NT_SIGNAL_PORT_1;

NT_PORT_BLOCK	NTPortInfo[NUM_OF_PRN_PORT];
BYTE			IsNTEndPrint[NUM_OF_PRN_PORT];
BYTE			NTimeOutCount[NUM_OF_PRN_PORT];
#if defined(NDWP2020)
BYTE			NTimeOutRetryCount;  //1/25/99
#else
int             NTimeOutRetryCount;
#endif
uint32			NTHoldTimer[];
uint32			NTHoldTimerOut = 10*60*1000; //about 10 Mins
int 			CurPortNumber = 0;		//615WU //Only port 1 thread defined

//return status
// (1) OK (2) END_OF_PRINTING (3) TIME_OUT
BYTE NTrecv (BYTE PortNumber, PrnBuf *PrintBuffer)
{
#ifdef NTUDP
	if(NTPortInfo[PortNumber].IPXFrameType == TCPUDP)
		return NTRequestUDP(PortNumber,PrintBuffer->data,&PrintBuffer->size);
	else
#endif
		return NTRequestECB(PortNumber,PrintBuffer->data,&PrintBuffer->size);
}



void NTPrinterServerInit (void)
{
//615wu::No PSMain
	cyg_semaphore_init( &NT_SIGNAL_PORT_1, 0 );
	
	NTimeOutRetryCount = EEPROM_Data.TimeOutValue * 2 / NT_TIME_OUT;  //518/99
}

void NT3main(cyg_addrword_t data)
{
	PrnBuf *PrintBuffer;
	BYTE    RecvFlag;
	BYTE	i,hold_long_abort = 0;
	uint32	ltimerdiff = 0;
    #if defined(NDWP2020)
    int     TimeOutRetryCount = 0;
    #endif
	
	for(;;) {
		cyg_semaphore_wait( &NT_SIGNAL_PORT_1 );
//		PrnSetNTHold(CurPortNumber);        //615wu::No PSMain
		
//ZOTIPS		armond_printf("Start NT3main \n");
        #if defined(NDWP2020)
		//eason for DWP2020
		if(NTPortInfo[CurPortNumber].IPXFrameType == TCPUDP)
			TimeOutRetryCount = NTimeOutRetryCount*8;
		else 
			TimeOutRetryCount = NTimeOutRetryCount;
        #endif

		PrintBuffer = NULL;
		NTimeOutCount[CurPortNumber] = 0;
		for(;;) {
			if(PrintBuffer == NULL && (PrintBuffer = PrnGetInQueueBuf(CurPortNumber)) == NULL) {//4/26/99 changed
				if(IsNTEndPrint[CurPortNumber]) break; //6/23/99 , ABORT
				cyg_thread_yield();
				continue;
			}
            #if defined(NDWP2020)
			if ( (NTimeOutCount[CurPortNumber] > TimeOutRetryCount) || (hold_long_abort) ) {	//1/25/99
            #else
			if ( (NTimeOutCount[CurPortNumber] > NTimeOutRetryCount) || (hold_long_abort) ) {	//1/25/99
            #endif
				//TIME OUT
				PrnAbortSpooler(CurPortNumber,PrintBuffer);	//4/26/99 changed
				hold_long_abort = 0;
				NTimeOutCount[CurPortNumber] = 0;
//ZOTIPS				armond_printf("NT3main timeout\n");
				//continue;	//12/2/98 marked

#ifdef SUPPORT_JOB_LOG
				JL_EndList(CurPortNumber, 3);	// Timeout. George Add February 26, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINA) && !defined(O_TPLINS) && !defined(O_LS)
#ifdef NOVELL_PS
				SendEOF(CurPortNumber);	        // Send the EOF page. George Add January 10, 2008
#endif
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
#endif //SUPPORT_JOB_LOG				
				
				break;	//time out
			}
//ZOTIPS			armond_printf("NT3main NTrecv \n");
			RecvFlag = NTrecv(CurPortNumber,PrintBuffer);

			if(RecvFlag == PRN_Q_ABORT) {
				NTimeOutCount[CurPortNumber]++;				
				continue;
			}

			if(RecvFlag == PRN_Q_HOLD){
				if(NTHoldTimer[CurPortNumber] == 0)
					NTHoldTimer[CurPortNumber] = jiffies;
				if(jiffies < NTHoldTimer[CurPortNumber])
					ltimerdiff = MAX_U32_VALUE - NTHoldTimer[CurPortNumber]+jiffies;
				else
					ltimerdiff = jiffies - NTHoldTimer[CurPortNumber];
				if(ltimerdiff>NTHoldTimerOut){
					hold_long_abort = 1;
					NTHoldTimer[CurPortNumber] = 0;
				}
				continue;
			}
			NTHoldTimer[CurPortNumber] = 0;
				
			NTimeOutCount[CurPortNumber] = 0;
			PrnPutOutQueueBuf(CurPortNumber,PrintBuffer,RecvFlag);
			PrintBuffer = NULL;

			if(RecvFlag == PRN_Q_EOF) {
				break;	//end of printing
			}
			
			cyg_thread_yield();
			
		}//for(;;).....

//#ifdef SUPPORT_JOB_LOG
//		JL_EndList(PrinterID, 0);		// Completed. George Add January 26, 2007
//#endif //SUPPORT_JOB_LOG

		PrnSetNoUse(CurPortNumber);
		NTPortInfo[CurPortNumber].HaveRecvPacket = 1;
//		PrnSetNTUnHold(CurPortNumber);        //615wu::No PSMain
	} //for(;;)....
}

#endif WIDNOWS_PS
