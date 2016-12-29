#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "prnqueue.h"
#include "psdefine.h"
#include "prnport.h"

#define SNMP_MAX_STRING   512

//PortNWrite Thread initiation information
#define PortNWrite_TASK_PRI         	20	//ZOT716u2
#define PortNWrite_TASK_STACK_SIZE  	2048
static	uint8			PortNWrite_Stack[PortNWrite_TASK_STACK_SIZE];
static  cyg_thread		PortNWrite_Task;
static  cyg_handle_t	PortNWrite_TaskHdl;


#define PRNTEST
#undef PRNTEST


//Spooler Thread initiation information
#define PrnSpooler_TASK_PRI				20	//ZOT716u2
#define PrnSpooler_TASK_STACK_SIZE    	2048
static	uint8			PrnSpooler_Stack[PrnSpooler_TASK_STACK_SIZE];
static  cyg_thread		PrnSpooler_Task;
static  cyg_handle_t	PrnSpooler_TaskHdl;

//ksign
cyg_sem_t	 SP_SIGNAL_PORT_1;

#define PRN_EXIT_PRINT_MODE_TIME			7000

//PrnStateSpooler Thread initiation information
#define PrnStateSpooler_TASK_PRI			20	//ZOT716u2
#define PrnStateSpooler_TASK_STACK_SIZE    	4096
static	uint8			PrnStateSpooler_Stack[PrnStateSpooler_TASK_STACK_SIZE];
static  cyg_thread		PrnStateSpooler_Task;
static  cyg_handle_t	PrnStateSpooler_TaskHdl;

//extern int usbprn_write(int nPort, uint8 *pBuf, int nLength);
//extern int usbprn_open(int nPort);
//extern int usbprn_close(int nPort);
//extern uint8 usbprn_read_status( int nPort );

//global variable
BYTE G_PortReady;

#ifdef SUPPORT_PRN_COUNT
uint32			PageCount[NUM_OF_PRN_PORT] = {0};
#endif

struct parport PortIO[NUM_OF_PRN_PORT];

BYTE PrnReadPortStatus( BYTE port );

extern uint32 rdclock();

//ZOTIPS void PrnStartPrintNegotiate( int port )
int PrnStartPrintNegotiate( int port )
{
//ZOTIPS	usbprn_open(port);
//ZOTIPS	return;
	return usbprn_open(port, 1);
}

void PrnEndPrintNegotiate( int port )
{
//ZOTIPS	usbprn_close(port);
//ZOTIPS	return;
	return usbprn_close(port, 1);
}

void PrnDeviceInfo(BYTE Port, BYTE *Buf)
{
	// Charles 2001/06/27
	if( PortIO[Port].Manufacture != NULL && PortIO[Port].Model != NULL )
	{
		sprintf( Buf, "MANUFACTURER:%s;MODEL:%s;CLASS:PRINTER", 
			PortIO[Port].Manufacture, PortIO[Port].Model );
		if( PortIO[Port].CommandSet ) {
			if( ( strlen(Buf) + strlen(PortIO[Port].CommandSet) + 20 ) < SNMP_MAX_STRING )
				sprintf( &Buf[strlen(Buf)], ";COMMAND SET:%s", PortIO[Port].CommandSet );
		}
		if( PortIO[Port].Description ) {
			if( ( strlen(Buf) + strlen(PortIO[Port].Description) + 20 ) < SNMP_MAX_STRING )
				sprintf( &Buf[strlen(Buf)], ";DESCRIPTION:%s", PortIO[Port].Description );
		}
	}
}

void PrnFreeDeviceInfo(BYTE port)
{
	if(PortIO[port].Manufacture) {
		free(PortIO[port].Manufacture);
		PortIO[port].Manufacture = NULL;
	}

	if(PortIO[port].Model) {
		free(PortIO[port].Model);
		PortIO[port].Model = NULL;
	}

	if(PortIO[port].CommandSet) {
		free(PortIO[port].CommandSet);
		PortIO[port].CommandSet = NULL;
	}

	if(PortIO[port].Description) {
		free(PortIO[port].Description);
		PortIO[port].Description = NULL;
	}
}

void parse_device_id( int port, BYTE *p )
{
	BYTE *q,*sep;
	BYTE *u;

	while (p) {
		q = strchr(p, ';');
		if (q) *q = 0;			  //+----->	p (u)
		                          //|        +--------> q
		                          //V	     V
		sep = strchr(p, ':');	  //MFC: HP	 ;
		if (sep) {
			u = p;
			*(sep++) = 0;
			while (*u) {
				*u = toupper(*u);
				u++;
			}
			if (strstr(p, "MFG") || strstr(p, "MANUFACTURER")) {
				if((PortIO[port].Manufacture = malloc(strlen(sep)+1)) != NULL) {
					memset(PortIO[port].Manufacture, 0, strlen(sep));
					strcpy(PortIO[port].Manufacture, sep);
				}
			} else if (!strcmp(p, "MDL") || !strcmp(p, "MODEL")) {
				if((PortIO[port].Model = malloc(strlen(sep)+1)) != NULL) {
					memset(PortIO[port].Model, 0, strlen(sep));
					strcpy(PortIO[port].Model, sep);
				}
			} else if (strstr(p, "CMD") || strstr(p, "COMMAND SET")) {
				if((PortIO[port].CommandSet = malloc(strlen(sep)+1)) != NULL) {
					memset(PortIO[port].CommandSet, 0, strlen(sep));
					strcpy(PortIO[port].CommandSet, sep);
				}

				u = sep;
				while (*u) {
					*u = toupper(*u);
					u++;
				}

				PortIO[port].SupportLang = 0;
				if(strstr(sep, "PJL")) PortIO[port].SupportLang |= P1284_PJL;
				if(strstr(sep, "PCL")) PortIO[port].SupportLang |= P1284_PCL;
				if(strstr(sep, "POSTSCRIPT")) PortIO[port].SupportLang |= P1284_POSTSCRIPT;
			} else if (strstr(p, "DESCRIPTION")) {    // Charles 2001/06/27
				if((PortIO[port].Description = mallocw(strlen(sep))) != NULL) {
					strcpy(PortIO[port].Description, sep);
				}
			}
		}
		if (q) p = q+1; else p=NULL;
	}
}

BYTE PrnLangSupport(BYTE port, BYTE Lang)
{
	return ( PortIO[port].SupportLang & Lang );
}

BYTE RealPortStatus(BYTE port)
{
	// check if it is IEEE1284 or USB printer device
	return usbprn_read_status( port );
}

BYTE StartDMAIO(BYTE *SrcBuf, WORD DataSize, BYTE Port)
{
	PortIO[Port].DataSize = DataSize;
	PortIO[Port].DataPtr = SrcBuf;

	if( G_PortReady & ( 1 << Port ) )
		cyg_semaphore_post (&SP_SIGNAL_PORT_1);

	return 1;
}

uint32 Printing_StartNum[NUM_OF_PRN_PORT] = {0};	//is printing to longer
#define ALERT_RECOVERY		1
#define ALERT_ERROR			2
uint8 	PortType[NUM_OF_PRN_PORT] = {0};		//1:recover,2:offline
uint8   adjPortType[NUM_OF_PRN_PORT] = {0};
uint32	Printing_MarkNum[NUM_OF_PRN_PORT] = {0};
uint32 StateSpooler_Interval = 2000; //1s

void PrnStateSpooler(cyg_addrword_t data)
{
	BYTE 	port, status;
	uint32  time = 0;
			
	while (1){
		for (port = 0; port < NUM_OF_PRN_PORT ; port++)
		{	
			if(PrnGetPrinterStatus(port) != PrnNoUsed)
			{
				status = PrnReadPortStatus( port );

				if ( (status == PORT_PAPER_OUT) || (status == PORT_OFF_LINE) ){
					adjPortType[port] = status;
					PortType[port] = ALERT_ERROR;
				} else {
					if ( (Printing_StartNum[port] > 0) && (Printing_MarkNum[port] == Printing_StartNum[port]) ){
						time = ( time >= 45) ? 45: (time + 1);
					} else	{
						adjPortType[port] = PORT_READY;
						PortType[port] = ALERT_RECOVERY;
						time = 0;
					}
						
					if (Printing_StartNum[port] > 0)
						Printing_MarkNum[port] = Printing_StartNum[port];
					else
						Printing_MarkNum[port] = 0;
					
					if( time == 45)
					{
						adjPortType[port] = PORT_PAPER_OUT;
						PortType[port] = ALERT_ERROR;
					}	
				}		
			}
		}
	    sys_check_stack();	
		ppause(StateSpooler_Interval);		//90sec
	}

}

int	   PNW_Statu[NUM_OF_PRN_PORT] = {0}; 			//for debug
int	UsedPortNumber = 0;		//615WU //Only port 1 thread defined
void PortNWrite(cyg_addrword_t data)
{
	uint32 start;
	int    written;
	int port = UsedPortNumber;

#ifdef SUPPORT_PRN_COUNT
	BYTE	reqbuf[40] = {0x1B,0x25,0x2D,0x31,0x32,0x33,0x34,0x35,0x58,
						   0x40,0x50,0x4A,0x4C,0x20,0x49,0x4E,0x46,0x4F,0x20,
						   0x50,0x41,0x47,0x45,0x43,0x4F,0x55,0x4E,0x54,0x20,0x0D,0x0A,
						   0x1B,0x25,0x2D,0x31,0x32,0x33,0x34,0x35,0x58};
	BYTE	respbuf[40] = {0};
#endif	
	
	
	while( 1 )
	{
		cyg_semaphore_wait (&SP_SIGNAL_PORT_1);
		
		PNW_Statu[port] = 1;	//for dbg
		
//ZOTIPS	 	PrnStartPrintNegotiate( port );

	 	while(PrnStartPrintNegotiate( port )!=0)	
	 		cyg_thread_yield();
	 	
#ifdef SUPPORT_PRN_COUNT

		// check if it is IEEE1284 or USB printer device
//		if( port >= NUM_OF_1284_PORT ){
			if(PrnLangSupport(port,P1284_PJL))
				usbprn_write( port, reqbuf, 40 );
//		}

#endif //SUPPORT_PRN_COUNT
	 	
	 	
//ZOTIPS	 	armond_printf("Start writing, Data size: %d\n",PortIO[port].DataSize);
	 	
		start = jiffies;				
		
		while( jiffies - start < PRN_EXIT_PRINT_MODE_TIME )		
		{
			if( PortIO[port].DataSize )
			{
				written = 0;
				PortIO[port].TotalSize = PortIO[port].DataSize;
				
				//for simonsay.htm to debug
				Printing_StartNum[port] = (jiffies ? jiffies : 1);
				
				do {
#ifndef PRNTEST
					// check if it is IEEE1284 or USB printer device
					written = usbprn_write( port, PortIO[port].DataPtr, PortIO[port].DataSize );
#else					
					written = PortIO[port].DataSize;			//615wu-spooler-temp
#endif					
					PortIO[port].DataSize = PortIO[port].TotalSize - written;
					if( PortIO[port].DataSize ) 
						PortIO[port].PortTimeout++;
					else
						PortIO[port].PortTimeout = 0;
				} while( written < PortIO[port].TotalSize );

				start = jiffies;
				PortIO[port].DataSize = 0;
				
				//for simonsay.htm to debug
				Printing_StartNum[port] = 0;
			}
			
//			cyg_thread_yield();
			
			if( DMAPrinting( port ) == 0 ) {
//				ppause( 20 );
				ppause( 10 );
				if( PrnGetPrinterStatus( port ) == PrnNoUsed )
					ppause( 100 );
			}
		}							
		
//ZOTIPS		armond_printf("End writing\n");
									
		PrnEndPrintNegotiate( port );

		adjPortType[port] = PrnReadPortStatus( port );
		
#ifdef SUPPORT_PRN_COUNT
		//read pagecount info from printer
		if(PrnLangSupport(port,P1284_PJL)){
			memset(respbuf,0,40);
			usbprn_read( port, respbuf, 30 );
			PageCount[port] = atol(memchr(respbuf,0x0A,30));
		}
#endif		
		
		PNW_Statu[port] = 0;	//for dbg
		
		G_PortReady |= ( 1 << port );
        sys_check_stack();
	}
}

//  0: wait for job, 1: paper out, 2:off line, 3:busy (is printing)
BYTE PrnReadPortStatus( BYTE port )
{
	BYTE status;
	BYTE rc;
	int  mode;

	status = usbprn_read_status(port);
	
	if( status == 0x7F ) {
		//NO CONNECTED
		if( PrnGetPrinterStatus( port ) == PrnNoUsed && PortIO[port].PrnMode != PRN_NO_PRINTER ) {
			if( ( G_PortReady & ( 0x01 << port ) ) && PrnOutQueueIsEmpty( port ) ) {
				//Previous connected, but now no connected
				if( port < NUM_OF_1284_PORT )
				{
//					parport_negotiate( &PortIO[port], IEEE1284_MODE_COMPAT );
					PortIO[port].PrnMode = PRN_NO_PRINTER;
					PortIO[port].HasGetDeviceID = 0;
					PortIO[port].SupportLang = 0;
					PrnFreeDeviceInfo( port );
				}	
			}
		}
		rc = PORT_OFF_LINE;
	} else {
		//CONNECTED
		if( status & PARPORT_STATUS_PAPEROUT ) {
			//PE
			rc = PORT_PAPER_OUT;
		} else {
			if( status & PARPORT_STATUS_ERROR ) {
				//nFAULT
				if( status & PARPORT_STATUS_BUSY ) {
					//nBUSY
					if( ( status & PARPORT_CONTROL_SELECT ) && ( status & PARPORT_STATUS_ACK ) &&
					    ( G_PortReady & (0x01 << port ) ) && PrnOutQueueIsEmpty( port ) )
					{
						rc = PORT_READY;
					}
					else
						rc = PORT_PRINTING;
				} else {
					//BUSY
					rc = PORT_PRINTING;
				}
			} else {
				rc = PORT_OFF_LINE;
			}
		}
	}

	if(  rc == PORT_PRINTING )
	{
		if( ( G_PortReady & (0x01 << port) ) && PrnOutQueueIsEmpty( port ) )
			rc = PORT_READY;
	}	
#ifdef PRNTEST
	return 0;
#else	
	return rc;
#endif	
}

BYTE ReadPortStatus(BYTE port)
{
#ifdef PRNTEST
	return 0;
#else	
	return( PortIO[port].HasGetDeviceID ? PrnReadPortStatus(port) : PORT_OFF_LINE );
#endif	
}

int GetDeviceID( int port )
{
	uint8 status = PrnReadPortStatus( port );

	if( ( status == PORT_READY )
		&& PrnGetPrinterStatus( port ) == PrnNoUsed )
	{
		PrnSetDeviceIDInUse( port );

		PrnSetNoUse( port );

		return 1;
	}
	return 0;
}

// Print Port Status:
//  bit 0-1 => port 0, bit 2-3 => port 1, bit 4-5 => port 2
//  0: wait for job, 1: paper out, 2:off line, 3:busy (is printing)
//
BYTE ReadPrintStatus( void )
{
	BYTE i, rc = 0;

	for( i = 0 ; i < NUM_OF_PRN_PORT; i++ ) {
		rc |= ( ReadPortStatus(i) << ( 2 * i ) );
	}
	return rc;
}

void PrnPortInit(void)
{
	memset( PortIO, 0, sizeof(PortIO) );
	G_PortReady = ~(0xFF << NUM_OF_PRN_PORT);
}

void Spooler_init(void)
{	
	
//...........Print Port Virtual Driver Function...........//

	//Print Port Initialization
	PrnPortInit();

//...........Print Queue Buffer Allocate Function...........//
	
	PrnQueueInit();
	
//...........Print Queue Initialize Function...........//
	
	PrnPrinterInit();
	
	//Clear SP_SIGNAL_PORT_1 count
	cyg_semaphore_init(&SP_SIGNAL_PORT_1,0);

//...........Spooler Function...........//

	//Create PrnSpooler Thread
	cyg_thread_create(PrnSpooler_TASK_PRI,
                  PrnStartSpooler,
                  0,
                  "PrnSpooler",
                  (void *) (PrnSpooler_Stack),
                  PrnSpooler_TASK_STACK_SIZE,
                  &PrnSpooler_TaskHdl,
                  &PrnSpooler_Task);
	
	//Start PrnSpooler Thread
	cyg_thread_resume(PrnSpooler_TaskHdl);
	
	//Create PortNWrite Thread
	cyg_thread_create(PortNWrite_TASK_PRI,
                  PortNWrite,
                  0,
                  "Port1Write",
                  (void *) (PortNWrite_Stack),
                  PortNWrite_TASK_STACK_SIZE,
                  &PortNWrite_TaskHdl,
                  &PortNWrite_Task);
	
	//Start PortNWrite Thread
	cyg_thread_resume(PortNWrite_TaskHdl);
	
//...........Read Print Port State For Paper_out Function...........//	
	
	//Create PrnStateSpooler Thread
	cyg_thread_create(PrnStateSpooler_TASK_PRI,
                  PrnStateSpooler,
                  0,
                  "PrnStateSpooler",
                  (void *) (PrnStateSpooler_Stack),
                  PrnStateSpooler_TASK_STACK_SIZE,
                  &PrnStateSpooler_TaskHdl,
                  &PrnStateSpooler_Task);
	
	//Start PrnStateSpooler Thread
	cyg_thread_resume(PrnStateSpooler_TaskHdl);

}
