/* Internet LPD Server
 *   written by David Johnson (dave@cs.olemiss.edu)
 *
 * This code is in the public domain.
 *
 * Revision History:
 *
 * Revision 1.10  91/11/24  dave
 * New jobs were accepted when spooling was disabled for a queue
 * Added new secure mode selected by -i option.  This mode checks permissions
 *     based on IP addresses rather than domain names.  HOST field in
 *     control file must use IP addresses as well or permissions file should
 *     contain identical permissions for those names as the IP addresses.
 *
 * Revision 1.9  91/10/23  dave
 * Obtain a new sequence number for each job in a single connection
 *
 * Revision 1.8  91/10/21  dave
 * Added support for DEVD, a device server
 *
 * Revision 1.7  91/10/12  dave
 * Added -P option to lpd to allow specification of the permissions file
 * Free the SD, LF, and LF entries in the printcap
 * If printer is "down" for a reason, show this in the lpq
 *
 * Revision 1.6  91/10/03  dave (from Hans-Juergen Knobloch)
 * Changed class[32] definition in validate_control_file() to class[64]
 *     to support long class names from VAX/VMS systems
 *
 * Revision 1.5  91/09/28  dave
 * Separate filenames in lpq -l with commas
 * Log permission rejections to the queue error log file
 *
 * Revision 1.4  91/09/26  dave (from Hans-Juergen Knobloch)
 * Reset secure flag on startup so the security features may be enabled
 *     and disabled without leaving net
 * Changed definition of priority to char
 * Changed definition of job size in send_queue() to a long
 * Changed formatting of date in send_queue()
 *
 * Revision 1.3  91/09/19  dave
 * Had to update is_job_specified() since additional letter sequence
 *
 * Revision 1.2  91/09/17  dave
 * Job number reported to lpq included part which was not numeric
 * Long hostname in send_queue() would overflow line buffer
 * Date in send_queue() was not terminated properly
 *
 * Revision 1.1  91/09/14  dave
 * Finished permission checking
 *
 * Revision 1.0  91/09/04  dave
 * Initial Release
 *
 */
#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "lp.h"
#include "lpd.h"
#include "prnqueue.h"
#include "prnport.h"
#include "httpd.h"
#include "joblog.h"

#if defined(IPPD) && defined(LPD_TXT)
#include "ippd.h"
#endif

struct lpd_transf {
	int     mode;
	struct LPDtrans	*trans; 
};

#define  BLOCK_SIZE				( 1460 )

//from psmain.c
extern uint32 rdclock();
//extern cyg_sem_t UNIX_SIGNAL_NO;    //615wu::no psmain

//615wu::no psmain
#define UnixHolded        0x01
#define UnixHoldAbort     0x02
uint8 UnixHold[NUM_OF_PRN_PORT] = {0};
#define PrnIsUnixHold(Port)			(UnixHold[Port] & UnixHolded)
#define PrnSetUnixHold(Port)		(UnixHold[Port] |= UnixHolded)
#define PrnSetUnixUnHold(Port)		(UnixHold[Port] &= (~(UnixHolded|UnixHoldAbort)))
#define PrnIsUnixHoldAbort(Port)    (UnixHold[Port] & UnixHoldAbort)
#define PrnSetUnixHoldAbort(Port) 	 (UnixHold[Port] |= UnixHoldAbort)

//from httpd.c
extern ZOT_FILE* zot_fopen( int handle );
extern int zot_fclose( ZOT_FILE* fp);
extern char *zot_fgets(char *buf,int len, ZOT_FILE *fp);
extern int zot_fputs (int8 *buf ,ZOT_FILE *fp );
#define fdopen(x,y)		zot_fopen(x)
#define fclose(x) 		zot_fclose(x)
#define fgets(x,y,z)	zot_fgets(x,y,z)
#define	fputs(x,y) 		zot_fputs(x,y)
#define FILE 			ZOT_FILE
extern int availmem();
extern void rip(char *s);
extern char 			Hostname[];
extern int	sendack(int s);
//extern int recv(int s, void *buf, size_t len, int flags);
//extern int send(int s, const void *buf, size_t len, int flags);

//from http_constant.c
extern const char *PrnUsedMessage[];

//from ippd.c
extern void ProcessCRLF(BYTE *KeepCR, struct prnbuf *pbuf,BYTE *Tmpbuf, int16 size);


#define BUFFER_SIZE 1024		// 512 is too short in Windows 2000

static int  receive_control_file(unsigned long filesize,char *buf, int s);

#if defined(IPPD) && defined(LPD_TXT)
static int  receive_data_file(int TXTMode, int PortNumber, unsigned long filesize, int s,int NumOfFile);
static int receive_bin_data_file(int PortNumber,unsigned long filesize, int s);
static int receive_txt_data_file(int PortNumber,unsigned long filesize, int s);
#else
static int  receive_data_file(int PortNumber, unsigned long filesize, int s,int NumOfFile);
#endif

static void service_job(int Mode, void *lpd, char *buf);
static void receive_job(cyg_addrword_t data);
static void abort_job(struct LPDtrans *LPD,char *buf);
static void send_queue(struct LPDtrans *LPD);
static int  CheckServiceMode(int remote, FILE* network,int *Port, int *Mode, char *buf);
static int  hold_job(int PortNumber, int NumOfFile, int s);
static void GetQueueID(int PortNumber, char *pbuf);	//6/4/98
static int  CheckRemoveJobID(char *buf,int PortNumber);

static int Slpd = -1;             // Prototype socket for service
static int ServiceProcIsBusy = 0; //Service_job is busy or not !
int CurJobQueueID[NUM_OF_PRN_PORT]; //6/4/98 added, 12/22/98 set as a global

//615wu::no psmain
#define LPRTask	2
struct Lpr_Task {
	int     num;
	BYTE	flag; 
	int     needrelase;
};
struct Lpr_Task LPRTaskInUse[LPRTask] = {0};

//615wu::no psmain
	struct LPDtrans LPD[LPRTask][3];	//for port 0 - 2

//LpdRecv Thread initiation information	//615wu::no psmain
#define LpdRecv_TASK_PRI         	20	//ZOT716u2
#define LpdRecv_TASK_STACK_SIZE  	2048 //ZOT716u2 4096
static	uint8			LpdRecv_Stack[LPRTask][LpdRecv_TASK_STACK_SIZE];
static  cyg_thread		LpdRecv_Task[LPRTask];
static  cyg_handle_t	LpdRecv_TaskHdl[LPRTask];
#ifdef SUPPORT_JOB_LOG
uint8	HadLogged = 0;
#endif SUPPORT_JOB_LOG

extern cyg_sem_t network_tcpip_on;

// Start up LPD service
void lpdstart(cyg_addrword_t data)
{
	struct sockaddr_in lsocket;
	struct sockaddr_in from_addr;
	int c, s, from_len;
	FILE *network;
	int i,rc;
	char *ServerProcName="LpdServ1";
#if 0	//615wu::no psmain
	struct LPDtrans LPD[4];	//for port 0 - 2 & service proc
#else
	struct LPDtrans LPD_SRV;
#endif
#ifdef SUPPORT_JOB_LOG
//	char computername[32];
	char person[32];
#endif SUPPORT_JOB_LOG
	int Port, Mode;
	char *buf;

#if 0	//615wu::no psmain
	struct lpd_transf *LPD_TRANS;
	LPD_TRANS = (struct lpd_transf *)malloc( sizeof(struct lpd_transf) );
#else
	int task_index = 0;
	struct lpd_transf *LPD_TRANS[LPRTask];
	for(i = 0; i <LPRTask; i++)	
		LPD_TRANS[i] = (struct lpd_transf *)malloc( sizeof(struct lpd_transf) );
#endif
	
	if( Slpd != -1 ) {
		// Already running!
		return ;
	}

    cyg_semaphore_wait(&network_tcpip_on);
	
	buf = malloc( BUFFER_SIZE );
	if( buf == NULL )
		return ;

	ServiceProcIsBusy = 0;

	// Wait for incoming jobs
	lsocket.sin_family = AF_INET;
	lsocket.sin_addr.s_addr = htonl (INADDR_ANY);
	lsocket.sin_port = htons(IPPORT_LPD);

	Slpd = socket( AF_INET, SOCK_STREAM, 0 );
	
//ZOTIPS	armond_printf("open socket cnt = %d \n",Slpd);
	
	bind( Slpd,(struct sockaddr *) &lsocket, sizeof(lsocket) );
	listen( Slpd, 1 );

	i = 0;
	for( ;; ) {
		from_len = sizeof(from_addr);  //4/8/98 Simon
		if( (s = accept( Slpd,(struct sockaddr *) &from_addr, &from_len )) == -1 )
		{
			continue;	// Service is shutting down
		}
		network = fdopen(s,"r+b");

//ZOTIPS		armond_printf("open socket cnt = %d \n",s);
		
		// check service mode
		switch(rc = CheckServiceMode(s,network,&Port,&Mode,buf)) {
			case LPD_RECEIVE_MODE:
				if( availmem() != 0 ) {

//Jesse					WarnLightOn(LED_LOW_MEMORY,2);
					RESPOND_ERR( s );
					shutdown(s,1);
					fclose(network);
					break;
				}
#if 0	//615wu::no psmain
				LPD[Port].network = (FILE *) network;
				LPD[Port].remote = s;
				LPD[Port].PortNumber = Port;
				ServerProcName[7] = (++i) + '0';
#if defined(IPPD) && defined(LPD_TXT)
//os				newproc(ServerProcName, 768 + (BUFFER_SIZE/2), receive_job,Mode,(void *) &LPD[Port], NULL, 0 );
				
				LPD_TRANS->mode = Mode;
				LPD_TRANS->trans = (void *) &LPD[Port];
				
				if( LpdRecv_TaskHdl != 0 )
					cyg_thread_delete(LpdRecv_TaskHdl);
				
				//Create LpdRecv Thread
				cyg_thread_create(LpdRecv_TASK_PRI,
									receive_job,
									LPD_TRANS,
									ServerProcName,
									(void *) (LpdRecv_Stack),
									LpdRecv_TASK_STACK_SIZE,
									&LpdRecv_TaskHdl,
									&LpdRecv_Task);
	
				//Start LpdRecv Thread
				cyg_thread_resume(LpdRecv_TaskHdl);
													
#else
//os				newproc(ServerProcName, 768 + (BUFFER_SIZE/2), receive_job,rc,(void *) &LPD[Port], NULL, 0 );
				LPD_TRANS->mode = rc;
				LPD_TRANS->trans = (void *) &LPD[Port];
				//Create LpdRecv Thread
				cyg_thread_create(LpdRecv_TASK_PRI,
									receive_job,
									&LPD_TRANS,
									ServerProcName,
									(void *) (LpdRecv_Stack),
									LpdRecv_TASK_STACK_SIZE,
									&LpdRecv_TaskHdl,
									&LpdRecv_Task);
	
				//Start LpdRecv Thread
				cyg_thread_resume(LpdRecv_TaskHdl);
#endif
#else
				for(task_index=0;task_index<LPRTask;task_index++)
				{
					if(LPRTaskInUse[task_index].needrelase	== 1 )
					{
						cyg_thread_delete(LpdRecv_TaskHdl[task_index]);
						LPRTaskInUse[task_index].needrelase = 0;
					}
				}
				
				for(task_index=0;task_index<LPRTask;task_index++)
				{
					if(LPRTaskInUse[task_index].flag == 0 )
					{
					
						LPRTaskInUse[task_index].num = s;
						LPRTaskInUse[task_index].flag = 1;
						
						LPD[task_index][Port].network = (FILE *) network;
						LPD[task_index][Port].remote = s;
						LPD[task_index][Port].PortNumber = Port;
						ServerProcName[7] = (++i) + '0';
						
						LPD_TRANS[task_index]->mode = Mode;
						LPD_TRANS[task_index]->trans = (void *) &(LPD[task_index][Port]);			
						
						//Create LpdRecv Thread
						cyg_thread_create(LpdRecv_TASK_PRI,
											receive_job,
											LPD_TRANS[task_index],
											ServerProcName,
											(void *) (LpdRecv_Stack[task_index]),
											LpdRecv_TASK_STACK_SIZE,
											&LpdRecv_TaskHdl[task_index],
											&LpdRecv_Task[task_index]);
			
						//Start LpdRecv Thread
						cyg_thread_resume(LpdRecv_TaskHdl[task_index]);
						
				break;
					}
				}
#endif
				break;
			case LPD_SERVICE_MODE:
#if 0	//615wu::no psmain
				LPD[3].network = (FILE *) network;
				LPD[3].remote = s;
				LPD[3].PortNumber = Port;
				service_job(Mode,(void *)&LPD[3],buf);
#else
				LPD_SRV.network = (FILE *) network;
				LPD_SRV.remote = s;
				LPD_SRV.PortNumber = Port;
				service_job(Mode,(void *)&LPD_SRV,buf);
#endif
				break;
			case LPD_ERROR_MODE:
				fclose(network);
				break;

		}
	}

	free( buf );
//	return 0;
}

int CheckServiceMode(int remote, FILE* network,int *Port, int *Mode, char* buf)
{
	char *printer;

#if defined(IPPD) && defined(LPD_TXT)
	int  PrintMode = 0;
#endif

	while(1) {
		if(recv( remote ,buf , BUFFER_SIZE , 0  ) < 0){
			// He closed on us
			return LPD_ERROR_MODE;
		}

		if(strlen(buf) == 0) {
			// Can't be a legal LPD command
			RESPOND_ERR(remote);
		}
		else
			break;
	}

    rip( buf );

	//Get Printer (Queue) name
////	for( sp = buf+1, dp = printer; *sp && *sp != ' '; *dp++ = *sp++ );
	for(printer = buf+1; *printer && *printer != ' '; printer++);
	*printer = NULL;
	printer = buf+1;

	if( START_CMD <= *buf && *buf <= REMOVE_CMD ) {
		if( (*Port = PrnCheckPrinter( printer )) < 0 ) {
//			fprintf( LPD->network , "Printer %s not found(%s).\n",printer, Hostname);

			fputs("Printer ",network);
			fputs(printer,network);
			fputs(" not found (",network);
			fputs(Hostname,network);
			fputs(").\n",network);
//			RESPOND_ERR(remote );	//OK
			return LPD_ERROR_MODE;
		}
#if defined(IPPD) && defined(LPD_TXT)
		PrintMode = (*Port & 0xF0)?1:0;	  //Text Mode or not
		*Port &= 0x0F;
#endif
	} else {

		fputs("Bad command\n",network);
//		RESPOND_ERR(remote); //OK
		return LPD_ERROR_MODE;
	}

	*Mode = (int) *buf;
	if(*buf != RECEIVE_CMD) {
		//Check service proc status
		if(ServiceProcIsBusy) {

			fputs("Server is busy !\n",network );
//			RESPOND_ERR(remote);	//OK
			return LPD_ERROR_MODE;
		}

		ServiceProcIsBusy = 1;
		return LPD_SERVICE_MODE;
	}
	else {
#if defined(IPPD) && defined(LPD_TXT)
		*Mode = PrintMode;	  //Txt mode or not
#endif
		//Check printer status
#if 0	//615wu::no psmain
		switch(PrnGetPrinterStatus(*Port)) {
//			case PrnNoUsed:
//				PrnSetUnixInUse(*Port);
//				return LPD_RECEIVE_MODE;
			case UnixUsed:
				// printer is busy

				break;
			default:
				if(PrnIsUnixHold(*Port)) {

					break;
				}
				PrnSetUnixHold(*Port);
				return LPD_RECEIVE_MODE;
		}
#else	

#ifdef O_AXIS
		if( ReadPortStatus(*Port) == PORT_OFF_LINE )
		{
			fputs(PrnGetPrinterName(*Port),network);
			fputs(" is offline !\n",network);
			RESPOND_ERR(remote);	//OK
			return LPD_ERROR_MODE;
		}
#endif		
		if(!PrnIsUnixHold(*Port)) {	//615wu::no psmain (no one hold, accept this connection.permit 'one' hold only)
			PrnSetUnixHold(*Port);
			return LPD_RECEIVE_MODE;
		}
#endif
		
//		fprintf( LPD->network, "%c: is busy.\n", *Port+'1');
		fputs(PrnGetPrinterName(*Port),network);
		fputs(" is busy !\n",network);
		RESPOND_ERR(remote);	//OK
		return LPD_ERROR_MODE;
	}
}

static void
service_job(
int Mode,
void *lpd,
char *buf )
{
//Jesse	union sp sockp;
//Jesse	struct sockaddr sc;
//Jesse	struct sockaddr_in from_addr;
	struct LPDtrans *LPD = (struct LPDtrans *)lpd;


	switch( Mode ) {
		case START_CMD:			// Check queue and print
			//					(print any waiting jobs of the queue)
			// Line format: \1printer\n
			//
			RESPOND_OK( LPD->remote );
//			close_s( s );			// close connection
//			start_unspooler( LPD.name );
			break;
		case RECEIVE_CMD:		// Submit a print job
			//
			// Line format: \2printer\n
			//

//			receive_job(LPD);
			break;
		case SQUEUE_CMD:		// Display Queue (short form)
			//*
			//* Line format: \3printer [users ...] [jobs ...]\n
			//*
		case QUEUE_CMD:			// Display Queue (long form)
			//*
			//* Line format: \4printer [users ...] [jobs ...]\n
			//*
			send_queue(LPD);
			break;
		case REMOVE_CMD:		// Remove a job from Queue
			//*
			//* Line format: \5printer person [users ...] [job ...]\n
			//*
			abort_job(LPD,buf);
			break;
		default:			// Invalid command

			break;
//			fprintf( LPD.network , "Bad command\n" );
//			fputs("Bad command\n",LPD.network);
//			RESPOND_ERR( LPD.remote );
	}
	// Clean up
	fclose(LPD->network);
	ServiceProcIsBusy = 0;
}

#ifdef SUPPORT_JOB_LOG
	int buffercount = 0;
	char tbuffer[32]={0};
#endif //SUPPORT_JOB_LOG

static void
receive_job(cyg_addrword_t data)
{
	struct lpd_transf *LPD_TRANS = data; //Jesse
	int Mode = LPD_TRANS->mode; //Jesse
	int iReceiveTimeout;
	int rc;
	unsigned long filesize;
	char buffer[BUFFER_SIZE];
	int  PortNumber;
//Jesse	struct LPDtrans *LPD = (struct LPDtrans *)lpd;
	struct LPDtrans *LPD = (struct LPDtrans *)LPD_TRANS->trans; //Jesse
	int  NumOfFile = 0;  //How many files want to printing ? //5/20/98
	int i = 0;	//615wu::no psmain

	PortNumber = LPD->PortNumber;



  	RESPOND_OK( LPD->remote );

/* move to receive data file 3/23/98
	hold_job(PortNumber);

	if( PrnIsUnixHoldAbort(PortNumber) ) {
			//abort by user
			fgets( buffer, sizeof(buffer) , LPD->network );

			RESPOND_ERR( LPD->remote );
			fclose(LPD->network);
			PrnSetUnixUnHold(PortNumber);
			return;
	}
*/

	rc = 0;
	while( rc == 0 ) {
//Jesse		if(fgets( buffer, sizeof(buffer) , LPD->network ) == NULL ){
		if(recv( LPD->remote, buffer , sizeof(buffer) , 0  ) < 0){
			// He closed on us
			break;
		}

		if(strlen(buffer) == 0)	{
			// Can't be a legal LPD command

//			fprintf( LPD->network, "%c: illegal LPD command\n",PortNumber+'1');
			fputs(PrnGetPrinterName(PortNumber),LPD->network);
			fputs(" illegal LPD command\n",LPD->network);
			RESPOND_ERR( LPD->remote );
			continue;
		}
		rip(buffer);

		switch( *buffer ) {
			case ABORT_SCMD:	// flush current files

				abort_job(LPD,NULL);
				RESPOND_OK( LPD->remote );
				break;	//ignore !
			case RECEIVE_CF_SCMD:	// control file

				filesize = atol( buffer+1 );

				if(receive_control_file(filesize, buffer, LPD->remote ) < 0 ) {

//					fprintf( LPD->network, "%c: can not receive control file\n",PortNumber+'1');
					fputs(PrnGetPrinterName(PortNumber),LPD->network);
					fputs(" can not receive control file\n",LPD->network);
//					RESPOND_ERR( LPD->remote );	//add by Simon 12/4/97
					rc = -1;
					break;
				}

#ifdef SUPPORT_JOB_LOG
				// RECEIVE_JOB_INFORMATION
//----------------------------------------------------------------------------
				memset(tbuffer,0,32 );
				buffercount=0;				
				while(1){
					buffercount++;
					if((BYTE)buffer[buffercount]== 0x0a)	//
						if((BYTE)buffer[buffercount+1] == 0x50){	//'P'
							memcpy(tbuffer,buffer+buffercount+2,32);
							break;
						}
					if ( buffercount == BUFFER_SIZE) break;
				}
				if ( buffercount < BUFFER_SIZE){
					buffercount=0;
					while(1){
						buffercount++;
						if((BYTE)tbuffer[buffercount]== 0x0a)
							if((BYTE)tbuffer[buffercount+1] == 0x4A)	//'J'
								break;
						if( buffercount == 32)
							break;		
					}
				}
//----------------------------------------------------------------------------				

#endif //SUPPORT_JOB_LOG
				RESPOND_OK( LPD->remote );
				break;
			case RECEIVE_DF_SCMD:

				filesize = atol( buffer+1 );
				//6/4/98 Get Printer Queue ID
				GetQueueID(PortNumber,buffer);

#if defined(IPPD) && defined(LPD_TXT)
				if( (iReceiveTimeout = receive_data_file(Mode, PortNumber, filesize, LPD->remote,NumOfFile++)) < 0 ) {
#else
				if( (iReceiveTimeout = receive_data_file( PortNumber, filesize, LPD->remote,NumOfFile++)) < 0 ) {
#endif


//					fprintf( LPD->network, "%c: can not receive data file\n", PortNumber+'1');
					fputs(PrnGetPrinterName(PortNumber),LPD->network);
					fputs(" can not receive data file\n",LPD->network);
//					RESPOND_ERR( LPD->remote );	//add by Simon 12/4/97
///////////////////////////////////
					//Add by Simon 12/23/97	abort the connect for (ACITSLPR95+LPM, DIGILPR95 + LPM)
//os					kwait(0); //must wait until TCPIN release the TCB block !!!
					cyg_thread_yield();
					shutdown(LPD->remote,2);
///////////////////////////////////
					if(iReceiveTimeout == -3)
						rc = -3;
					else
					rc = -1;
					break;
				}
				RESPOND_OK( LPD->remote );
				break;
		} //switch()...
	}//while (rc == 0) ...
	PrnSetNoUse(PortNumber);
//615wu::no psmain	PrnSetUnixUnHold(PortNumber);  //4/8/98

#ifdef SUPPORT_JOB_LOG
	if(rc == -3)
	{
		JL_EndList(PortNumber, 3);		// Timeout. George Add January 31, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_TPLINA) && !defined(O_LS)
		SendEOF(PortNumber);			// Send the EOF page. George Add January 10, 2008
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
	}
	else
		JL_EndList(PortNumber, 0);		// Completed. George Add January 26, 2007

	HadLogged = 0;
#endif //SUPPORT_JOB_LOG
	
	fclose(LPD->network);
	
	for(i=0;i<LPRTask;i++)
	{
		if(LPRTaskInUse[i].num	== LPD->remote  )
		{
			LPRTaskInUse[i].num = 0;
			LPRTaskInUse[i].flag = 0;
			LPRTaskInUse[i].needrelase = 1;
			break;
		}
	}
	
	cyg_thread_exit();

}

void GetQueueID(int PortNumber,	char *pbuf)
{
	while(*pbuf != 'd' && *pbuf != 'D' && *pbuf != '\n') pbuf++;

	if(*pbuf == '\n') {
		//Queue Name not found
		CurJobQueueID[PortNumber] = -1;
		return;
	}

	while((*pbuf < '0' || *pbuf > '9') && *pbuf != '\n' ) pbuf++;

	if(*pbuf == '\n') {
		//Queue Name not found
		CurJobQueueID[PortNumber] = -1;
		return;
	}
	pbuf[3] = '\0';
	CurJobQueueID[PortNumber] = atoi(pbuf);
}


#if defined(IPPD) && defined(LPD_TXT)
static int receive_data_file(int TXTMode, int PortNumber, unsigned long filesize, int s, int NumOfFile )
#else
static int receive_data_file(int PortNumber, unsigned long filesize, int s, int NumOfFile )
#endif
{
#if defined(IPPD) && defined(LPD_TXT)

#else
	int bytes, blocksize;
	unsigned long total;
	char status = STATUS_OK;
	struct prnbuf *pbuf;
	uint32 startime;  //10/11/99

	total = 0;
	filesize++;	// add in status byte
	blocksize = filesize > BLOCKSIZE ? BLOCKSIZE : filesize;
#endif


	RESPOND_OK( s );

	hold_job(PortNumber,NumOfFile,s);

#ifdef SUPPORT_JOB_LOG
				if(HadLogged == 0){
					JL_PutList(1, PortNumber, tbuffer, buffercount);
				
					HadLogged = 1;
				}
#endif //SUPPORT_JOB_LOG

	if( PrnIsUnixHoldAbort(PortNumber) ) {
		//abort by user

		PrnAbortSpooler(PortNumber,NULL);
		PrnSetUnixUnHold(PortNumber);
		return (-1);
	}

#if defined(IPPD) && defined(LPD_TXT)
	if(TXTMode) 
        return receive_txt_data_file(PortNumber,filesize, s);
	else        
        return receive_bin_data_file(PortNumber,filesize, s);
#else

	do {
		startime = 0;  //10/11/99
		while((pbuf = PrnGetInQueueBuf(PortNumber)) == NULL) {

			//10/11/99 Send Ack When Buffer is empty *****************
			if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
				if(startime) sendack(s);
				startime = rdclock();
			}
			//********************************************************

//os			kwait(0);
			cyg_thread_yield();
			//queue buffer is full, waiting for printing
			//printf("Warning: Can not get In buffer !!!\n");
		}

		if(PrnGetPrinterStatus(PortNumber) == PrnAbortUsed) {

			status = STATUS_ERR;
			break;
		}


		if( (bytes = recv( s, pbuf->data, blocksize, 0 )) < 1 ) {

			status = STATUS_ERR;
			break;
		}


		if( total + bytes == filesize ) {
			bytes--;	// don't save status byte
			status = *(pbuf->data + bytes);
			total++;
		}

		pbuf->size = bytes;
		total += bytes;
		if( filesize - total < BLOCKSIZE ) // handle last portion
			blocksize = filesize - total;

		if(total < filesize) {
			PrnPutOutQueueBuf(PortNumber,pbuf,PRN_Q_NORMAL);
		}
		else {
			PrnPutOutQueueBuf(PortNumber,pbuf,PRN_Q_EOF); //end of printing
		}
	} while( total < filesize );

	if( status != STATUS_OK ) {
		PrnAbortSpooler(PortNumber,pbuf);
		return -1;
	}
	return 0;
#endif
}

int first_flag = 0;
int err_flag = 0;
//#define print_debug

#if defined(IPPD) && defined(LPD_TXT)
static int receive_bin_data_file(int PortNumber,unsigned long filesize, int s)
{
	char test_data;
	int bytes, blocksize;
	unsigned long total = 0;
	char status = STATUS_OK;
	struct prnbuf *pbuf;
	uint32 startime;
	int recv_bytes;
#ifdef USB_ZERO_CPY	
	int retrycnt, dataremain;
#endif
	char *pdata = 0x1B9000;
	unsigned int datalen = 0;
	uint32	startime_bin;

#ifdef print_debug
	
	if( first_flag == 0)
		memset(pdata, 0, 156*1024);
#endif //print_debug
	
	filesize++;	// add in status byte
#ifndef USB_ZERO_CPY	
	blocksize = filesize > BLOCKSIZE ? BLOCKSIZE : filesize;
#else	
	blocksize = 8192;
#endif
	do {
		startime = 0;
		while((pbuf = PrnGetInQueueBuf(PortNumber)) == NULL) {
			
			//10/11/99 Send Ack When Buffer is empty *****************
//			if((( rdclock()-startime)  > ((uint32)TICKS_PER_SEC*5) )) {
			if((( rdclock()-startime)  > ((uint32)TICKS_PER_SEC*10) )) {
				//if(startime)	//2003Nov28
				sendack(s);
				startime = rdclock();
			}
			//********************************************************
//os			kwait(0);
			cyg_thread_yield();
		}

		if(PrnGetPrinterStatus(PortNumber) == PrnAbortUsed) {

			status = STATUS_ERR;
			break;
		}

//		if( (bytes = recv( s, pbuf->data, blocksize, 0 )) < 1 ) {
//			status = STATUS_ERR;
//			break;
//		}
		
		// George Add April 3, 2007
		startime_bin = rdclock();
		
///*
		bytes = 0;
#ifndef USB_ZERO_CPY		
		while( ( recv_bytes = recv( s, &pbuf->data[bytes], BLOCK_SIZE, 0 ) ) > 0 )
#else		
		retrycnt =0;
		dataremain = blocksize;
		while( ( recv_bytes = recv( s, &pbuf->data[bytes], dataremain, 0 ) ) > 0 || retrycnt++ < 2)
#endif		
		{
#ifdef USB_ZERO_CPY			
			if (recv_bytes <= 0){
				cyg_thread_yield();
				continue;	
			}
			retrycnt =0;
#endif			
			bytes += recv_bytes;
			
#ifndef USB_ZERO_CPY			
			if( bytes + BLOCK_SIZE > blocksize )
#else		
			dataremain -= recv_bytes;	
			if( !dataremain )
#endif						
				break;
				
			if( get_socket_rcevent(s) == 0 )
				break;
				
		}
		
		if( recv_bytes <= 0 )
		{
			if ( bytes )
			{
				pbuf->size = bytes;
				
				test_data = *(pbuf->data + pbuf->size);
				
				PrnPutOutQueueBuf(PortNumber,pbuf,PRN_Q_EOF);
				status = STATUS_OK;
			}	
			else		
			{
				pbuf->size = 0;
				status = STATUS_ERR;
			}
			break;
		}
//*/
		if( total + bytes == filesize ) {
			bytes--;	// don't save status byte
			status = *(pbuf->data + bytes);
			total++;
		}

#ifdef SUPPORT_JOB_LOG
		JL_AddSize(PortNumber, bytes);
#endif //SUPPORT_JOB_LOG
		
		pbuf->size = bytes;
		total += bytes;
		if( filesize - total < BLOCKSIZE ) // handle last portion
			blocksize = filesize - total;

#ifdef print_debug		
		if( first_flag == 0)
		{
			memcpy( pdata+datalen ,pbuf->data,pbuf->size);
		}
		else
		{
			if(memcmp( pdata+datalen,pbuf->data,pbuf->size)!=0)
			{
				err_flag = 1;
				return -1;
			}
		}
		datalen+=bytes;
#endif //print_debug
		
		if(total < filesize) {
			PrnPutOutQueueBuf(PortNumber,pbuf,PRN_Q_NORMAL);
		}
		else {
			PrnPutOutQueueBuf(PortNumber,pbuf,PRN_Q_EOF); //end of printing
#ifdef print_debug
			first_flag = 1;
#endif //print_debug			
		}
				
	} while( total < filesize );

	if( status != STATUS_OK ) {
		PrnAbortSpooler(PortNumber,pbuf);
#ifdef SUPPORT_JOB_LOG
		// George Add April 3, 2007
		if((rdclock()-startime_bin) > ((uint32)(EEPROM_Data.TimeOutValue))*10)
			return -3;
		else
#endif //SUPPORT_JOB_LOG
		return -1;
	}
	return 0;
}

static int receive_txt_data_file(int PortNumber,unsigned long filesize, int s)
{
	int bytes, blocksize;
	unsigned long total = 0;
	char status = STATUS_OK;
	struct prnbuf *pbuf[3];
	uint32 startime = 0;

	int  i;
	uint8 KeepCR = 0;

	filesize++;	// add in status byte
	blocksize = filesize > BLOCKSIZE ? BLOCKSIZE : filesize;

	while((pbuf[2] = PrnGetInQueueBuf(PortNumber)) == NULL) {

		//********************************************************
		if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
			if(startime)	sendack(s);
			startime = rdclock();
		}
		//********************************************************

//os		kwait(0);
		cyg_thread_yield();
	}
	pbuf[2]->size = 0;

	do {
		startime = 0;  //10/11/99
		for(i = 0 ; i < 2; i++) {
			while((pbuf[i] = PrnGetInQueueBuf(PortNumber)) == NULL) {

				//********************************************************
				if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
					//if(startime)	//2003Nov28
					sendack(s);
					startime = rdclock();
				}
				//********************************************************

//os				kwait(0);
				cyg_thread_yield();
			}
			pbuf[i]->size = 0;
		}

		if(PrnGetPrinterStatus(PortNumber) == PrnAbortUsed) {
			status = STATUS_ERR;
			break;
		}


		if( (bytes = recv( s, pbuf[2]->data, blocksize, 0 )) < 1 ) {
			status = STATUS_ERR;
			break;
		}


		if( total + bytes == filesize ) {
			bytes--;	// don't save status byte
			status = *(pbuf[2]->data + bytes);
			total++;
		}

#ifdef SUPPORT_JOB_LOG
		JL_AddSize(PortNumber, bytes);
#endif //SUPPORT_JOB_LOG

		pbuf[2]->size = bytes;
		total += bytes;
		if( filesize - total < BLOCKSIZE ) // handle last portion
			blocksize = filesize - total;

		bytes = (pbuf[2]->size > (BLOCKSIZE/2))?(BLOCKSIZE/2):pbuf[2]->size;
		ProcessCRLF(&KeepCR, pbuf[0],pbuf[2]->data,bytes);
		PrnPutOutQueueBuf(PortNumber,pbuf[0],PRN_Q_NORMAL);

		if((bytes = (pbuf[2]->size - (BLOCKSIZE/2))) > 0) {
			ProcessCRLF(&KeepCR, pbuf[1],pbuf[2]->data+(BLOCKSIZE/2),bytes);
			PrnPutOutQueueBuf(PortNumber,pbuf[1],PRN_Q_NORMAL);
		} else {
			PrnPutInQueueBuf(PortNumber, pbuf[1]);
		}
	} while( total < filesize );

	if( status != STATUS_OK ) {
		PrnPutInQueueBuf(PortNumber, pbuf[0]);
		PrnPutInQueueBuf(PortNumber, pbuf[1]);
		PrnAbortSpooler(PortNumber,pbuf[2]);
		return -1;
	} else {
		pbuf[2]->size = 0;
		PrnPutOutQueueBuf(PortNumber,pbuf[2],PRN_Q_EOF);
	}
	return 0;
}
#endif


static int
receive_control_file(
unsigned long filesize,
char *buf,
int s
)
{
	int c, bytes;
	unsigned long total = 0;
	char status;
	int  blocksize = BUFFER_SIZE;
	char *buffer = buf;

	filesize++;	// add in status byte

	RESPOND_OK( s );

	do {
		if( (bytes = recv( s, buffer, blocksize, 0 )) < 1 ) {
			return -1;
		}
		if( total + bytes == filesize ) {
			bytes--;	// don't save status byte
			status = *(buffer + bytes);
			total++;
		}
		buffer += bytes;
		total += bytes;
		if( filesize - total < BLOCKSIZE ) // handle last portion
			blocksize = filesize - total;
	} while( total < filesize );

	if( status != STATUS_OK ) {
		return -1;
	}
	return 0;
}

void
send_queue(
struct LPDtrans *LPD
)
{
	int PortNumber = LPD->PortNumber;
	char JobNO[6];
	char SysResoure[110];
	int PrintStatus;
	char *Co1RVer, *Co2RVer;

#ifndef ATALKD
	int atpNumBufs = 0;
#endif  ATALKD


	fputs(PrnGetPrinterName(PortNumber),LPD->network);
	PrintStatus = PrnGetPrinterStatus(PortNumber);

	// 6/4/98 Add display Job ID !
	if(PrintStatus != UnixUsed)
		fputs(PrnUsedMessage[PrintStatus], LPD->network);
	else {
		sprintf(JobNO,"%d",CurJobQueueID[PortNumber]);
		sprintf(SysResoure,PrnUsedMessage[PrintStatus],
 			(CurJobQueueID[PortNumber] == -1?"unknow":JobNO) );
		fputs(SysResoure,LPD->network);
	}
/*Jesse
	Co1RVer = (char *)(CODE1_START_ADDRESS+MAJOR_VER_OFFSET);    //Code1 release version 6/5/01
	Co2RVer = (char *)(RAM_CODE_START_ADDRESS+MAJOR_VER_OFFSET); //Code2 release version 6/5/01

	sprintf(SysResoure," (%02d,%02d,%02d,%06lu|%02d|%06lu|%06lu|%02d,%02d,%02d|%d.%d,%d.%d)\n",
	    MIntbufNO[0], MIntbufNO[1],MIntbufNO[2], MbufFail,atpNumBufs,
	    (Availmem*12),(mini_Availmem*12),
	    MbufNO[0], MbufNO[1], MbufNO[2],
	    Co1RVer[0],Co1RVer[1],Co2RVer[0],Co2RVer[1]);
*/	    
//	sprintf(SysResoure," (%02d.%02d,%02d.%02d,%06lu|%06lu|%02d.%02d,%02d.%02d,%02d.%02d)\n",
//	    MIntbufNO[0],MIntbufUsed[0], MIntbufNO[1],MIntbufUsed[1], MbufFail,
//	    Availmem*12,
//	    MbufNO[0],MbufUsed[0],MbufNO[1],MbufUsed[1],MbufNO[2],MbufUsed[2]);
	fputs(SysResoure, LPD->network);

}

static void
abort_job(
struct LPDtrans *LPD,
char *buf
)
{
	int PrnStatus = PrnGetPrinterStatus(LPD->PortNumber);
	int PortNumber = LPD->PortNumber;
	int IsUnixHold = PrnIsUnixHold(PortNumber);
	int NeedRemoveJob;

	//6/4/98 Check Need remove Job or not ! /////////////////////
	if(buf == NULL) NeedRemoveJob = 1;
	else NeedRemoveJob = CheckRemoveJobID(buf,LPD->PortNumber);

	if(!NeedRemoveJob) {
		send_queue(LPD);
		return;
	}
	/////////////////////////////////////////////////////////////

	if(!IsUnixHold) {
		switch(PrnStatus) {
			case PrnNoUsed:
			case PrnAbortUsed:
			case NetwareUsed:
			case NTUsed:
				send_queue(LPD);
				return;
			case UnixUsed:
				break;
		}
		PrnSetAbortUse(PortNumber);
	}
	else {
		PrnSetUnixHoldAbort(PortNumber);
//os		ksignal(UNIX_SIGNAL_NO(PortNumber),1);
//615wu::no psmain		cyg_semaphore_post( &UNIX_SIGNAL_NO );
//os		kwait(0);
//615wu::no psmain		cyg_thread_yield();
	}
	fputs(PrnGetPrinterName(PortNumber),LPD->network);
	fputs(PrnUsedMessage[PrnAbortUsed], LPD->network);


}

//6/4/98 Check remove job ID
int CheckRemoveJobID(char *buf,int PortNumber)
{
	int i, Result;
	int JobID;

// Remove Jobs
//      +----+-------+----+-------+----+------+----+
//      | 05 | Queue | SP | Agent | SP | List | LF |
//      +----+-------+----+-------+----+------+----+
//
//      Command code - 5
//      Operand 1 - Printer queue name
//      Operand 2 - User name making request (the agent)
//      Other operands - User names or job numbers

// buf : the 'LF' has been replease with  '\0'


	//Skip Queue and Agent !
	while(*(buf++) != '\0' );

	Result = 1;

	do {
		while(*buf != ' ' && *buf != '\0' ) buf++;
		if(*buf == '\0') return (Result);
		buf++;
	}while(*buf	< '0' || *buf > '9');

	Result = 0;
	do {
		JobID = atoi(buf);
		if(JobID == CurJobQueueID[PortNumber]) {
			Result = 1;
			break;
		}
		while(*buf != ' ' && *buf != '\0' ) buf++;
		if(*buf == '\0') break;
		buf++;
	}while(*buf	< '0' || *buf > '9');

	return (Result);
}


// return (0) OK
// return (1) Abort by user
int hold_job(int PortNumber, int NumOfFile, int s)
{
	uint32 startime;  	
#if 0	//615wu::no psmain
	do {
		if(NumOfFile) {
			if(PrnGetPrinterStatus(PortNumber) == UnixUsed)
			{
				PrnSetUnixHold(PortNumber);
				PrnSetNoUse(PortNumber);
            }
			else {
//Jesse				WarnLightOn(LED_HOLD_JOB_ERROR,0); //light flash forever !
			}
		}
//os		kwait(UNIX_SIGNAL_NO(PortNumber));
		cyg_semaphore_wait( &UNIX_SIGNAL_NO );
		if(PrnIsUnixHoldAbort(PortNumber)) {
			return (1);	//abort by user
		}
		//ServiceNextPS(PortNumber)

	}while(PrnGetPrinterStatus(PortNumber) != PrnNoUsed);
#else

	if(NumOfFile)	//615wu::no psmain ('UnixInUse' already, no need to hold job, printing directly)
		return 0;
	
	cyg_scheduler_lock();	//615wu::No PSMain
	
	startime = rdclock();
	
	while( ReadPortStatus(PortNumber) != PORT_READY ||
		   PrnGetPrinterStatus(PortNumber) != PrnNoUsed){
		cyg_scheduler_unlock();	//615wu::No PSMain
		
		if((( rdclock()-startime)  > ((uint32)TICKS_PER_SEC*10) )) {
			
			sendack(s);
			startime = rdclock();
		}
		cyg_thread_yield();		
//		ppause(100);
		
		cyg_scheduler_lock();	//615wu::No PSMain
		
		if(PrnIsUnixHoldAbort(PortNumber)) {
			cyg_scheduler_unlock();	//615wu::No PSMain
			return (1);	//abort by user
		}
	}
#endif

	PrnSetUnixInUse(PortNumber);
	PrnSetUnixUnHold(PortNumber);
	cyg_scheduler_unlock();	//615wu::No PSMain
	
	return (0);
}
