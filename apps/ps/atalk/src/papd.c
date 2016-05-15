/*
 * Copyright (c) 1990,1995 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */

#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "prnqueue.h"
#include "prnport.h"
#include "socket2.h"
#include "atalkd.h"
#include "atp.h"
#include "atprn.h"
#include "session.h"
#include "pap.h"
#include "papd.h"

struct pap_sock pap_socksum[PAP_SOCKSUM]={0};

//PAPConn1 Thread initiation information
#define PAPConn1_TASK_PRI         	20	//ZOT716u2
#define PAPConn1_TASK_STACK_SIZE  	4096 //ZOT716u2 8192
static	uint8			PAPConn1_Stack[PAP_SOCKSUM][PAPConn1_TASK_STACK_SIZE];
static  cyg_thread		PAPConn1_Task[PAP_SOCKSUM];
static  cyg_handle_t	PAPConn1_TaskHdl[PAP_SOCKSUM];

int 	ApplePortNumber = 0;		//615WU //Only port 1 thread defined
cyg_sem_t ATALK_SIGNAL_NO_1;

static int
availmem()
{
	return 0;	//615wu //always ok
}

#define PreStatus   "status: "
#define PreLen      (sizeof(PreStatus)-1)
uint8 *PrStatus[] = { "idle", "out of paper", "off line", "busy"};
struct atprn pr[NUM_OF_PRN_PORT];

//          +----- Max string size of PrStatus[]
//          |
//          V
char rbuf[ 20 + PreLen + 1 + 8 ];

void PAPGetPPDName(void);
int getstatus(int PrnPort, uint8 *buf);

void papd_init(void)
{
	int i;

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		if (( pr[i].p_atp = atp_open(0)) == NULL ) {
#ifdef PC_OUTPUT
			printf("ATP_OPEN() error (PAPD.C)\n");
#endif PC_OUTPUT
			return;
		}
	}
#ifdef _PC
	PAPGetPPDName();
#endif _PC
	nbp_rgstr();
}

#ifdef _PC
void PAPGetPPDName(void)
{
	BYTE   *String,i;

	if(ConfigOpenFile("{PPD}")) {
		printf("Read {PPD} Error (papd.c) \n");
	    exit(0);
	}

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		if((String = ConfigGetString()) == NULL ) {
			printf("Read {PPD-%d} Error (papd.c) \n",i);
		    exit(0);
		}
		pr[i].p_ppdfile = strdup(String);
	}

	ConfigCloseFile();
}
#endif _PC

void papd(cyg_addrword_t data)
{
//	ATP			atp;
	struct iovec	iov;
	struct atp_block	atpb;
	struct sockaddr_at	sat;
	int err = 0;
	char cbuf[8];
	char *PrnProcName="PAPConn1";
	uint8 PrinterStatus;
	int i =0;
//	uint8 Connected;
	int PrnPort = ApplePortNumber;	//615wu

#ifdef _PC
	//Get PPD File Name
#endif _PC
    while (!ATD_INIT_OK) 
        cyg_thread_yield();

	//*
	//* Begin accepting connections.
	//*
	for (;;) {
//		Connected = 0;
		memset( &sat,'\0',sizeof( struct sockaddr_at ));
		sat.sat_family = AF_APPLETALK;
		sat.sat_addr.s_net = ATADDR_ANYNET;
		sat.sat_addr.s_node = ATADDR_ANYNODE;
		sat.sat_port = ATADDR_ANYPORT;
		// do an atp_rsel(), to prevent hangs
		if (atp_rsel( pr[PrnPort].p_atp, &sat, ATP_TREQ ) != ATP_TREQ ) {
			continue;
		}
		atpb.atp_saddr = &sat;
		atpb.atp_rreqdata = cbuf;
		atpb.atp_rreqdlen = sizeof( cbuf );
		if ( atp_rreq( pr[PrnPort].p_atp, &atpb ) < 0 ) {
#ifdef _PC
			printf("atp_rreq() error (papd.c) port: %d\n",PrnPort);
#endif _PC
			continue;
		}

		// should check length of req buf

		switch( cbuf[ 1 ] ) { //Command
		case PAP_OPEN :
//			if(pr[PrnPort].cur_atp != NULL &&
//			   pr[PrnPort].connid == (uint8)cbuf[ 0 ] &&
//			   pr[PrnPort].sat.sat_addr.s_net  ==  sat.sat_addr.s_net &&
//			   pr[PrnPort].sat.sat_addr.s_node ==  sat.sat_addr.s_node
//			) {
//				Connected = 1;
#ifdef PC_OUTPUT
//				printf("(PAPD.c) Connect again !\n");
#endif PC_OUTPUT
//			}

			pr[PrnPort].connid       = (uint8)cbuf[ 0 ];
			pr[PrnPort].sat.sat_port = (uint8) cbuf[ 4 ];
			pr[PrnPort].quantum      = (uint8)cbuf[ 5 ];
			rbuf[ 0 ] = cbuf[ 0 ];	   //ConnID
			rbuf[ 1 ] = PAP_OPENREPLY; //Command
			rbuf[ 2 ] = rbuf[ 3 ] = 0;
			rbuf[ 6 ] = rbuf[ 7 ] = 0; //Result

			//  0: wait for job, 1: paper out, 2:off line, 3:is printing
			PrinterStatus = ReadPortStatus(PrnPort);

//			if(!Connected && (PrnGetPrinterStatus(PrnPort) != PrnNoUsed ||
			if((PrnGetPrinterStatus(PrnPort) != PrnNoUsed ||
				PrinterStatus == PORT_PAPER_OUT           ||
				PrinterStatus == PORT_OFF_LINE            ||
				PrinterStatus == PORT_PRINTING            //5/16/2000
//				||        PrnIsUnixHold(PrnPort)	//615wu::No PSMain
			)){
				//busy or offline
#ifdef PC_OUTPUT
				AtSaySpace(0,22,79);
				printf("\a!!! BUSY !!! (papd.c)\a\n");
#endif PC_OUTPUT
				rbuf[ 6 ] = rbuf[ 7 ] = 0xff;
				err = 1;
			}
		    //*
		    //* If this fails, we've run out of sockets. Rather than
		    //* just die(), let's try to continue. Maybe some sockets
		    //* will close, and we can continue;
		    //*
			if (availmem() != 0 ||
//			   (!Connected && (pr[PrnPort].cur_atp = atp_open( 0 )) == NULL) )
			   ((pr[PrnPort].cur_atp = atp_open( 0 )) == NULL) )
			{
				//not enough memory
#ifdef PC_OUTPUT
				AtSaySpace(0,22,79);
				printf("\a(PAPD.C) Not enough memory !!!\a\n");
#endif PC_OUTPUT
				rbuf[ 6 ] = rbuf[ 7 ] = 0xff;
				err = 2;
			}

			rbuf[ 4 ] = atp_sockaddr( pr[PrnPort].cur_atp )->sat_port; //socket number

//#if NUM_OF_PRN_PORT >= 3
//			rbuf[ 5 ] = 8;	   //flow quantum  5/21/99
//#else
			rbuf[ 5 ] = PAP_MAXQUANTUM;	   //flow quantum
//#endif

			iov.iov_base = rbuf;
			iov.iov_len = 8 + getstatus( PrnPort, &rbuf[ 8 ] );
			atpb.atp_sresiov = &iov;
			atpb.atp_sresiovcnt = 1;

		    //*
		    //* This may error out if we lose a route, so we won't die().
			//*
			if ( atp_sresp( pr[PrnPort].p_atp, &atpb ) < 0 ) {
#ifdef _PC
				printf("Error : atp_sresp(1) (papd.c)\n");
#endif _PC
				atp_close(pr[PrnPort].cur_atp);
				pr[PrnPort].cur_atp = NULL;
				continue;
			}

			if ( err ) {
				err = 0;
				atp_close(pr[PrnPort].cur_atp);
				pr[PrnPort].cur_atp = NULL;
				continue;
			}

//			if(Connected) continue;

			cyg_scheduler_lock();	//615wu::No PSMain

			if(PrnGetPrinterStatus(PrnPort) != PrnNoUsed){
				
				cyg_scheduler_unlock();
				err = 0;
				atp_close(pr[PrnPort].cur_atp);
				pr[PrnPort].cur_atp = NULL;
				continue;
			}

			PrnSetAtalkInUse(PrnPort);
	
			cyg_scheduler_unlock();	//615wu::No PSMain

			PrnProcName[7] = PrnPort + '1';

			pr[PrnPort].sat.sat_family = AF_APPLETALK;
			pr[PrnPort].sat.sat_addr = sat.sat_addr;

			//!!! may be out of memory !!!
#ifdef PC_OUTPUT
			sound(300);
			delay(100);
			nosound();
#endif PC_OUTPUT
//			newproc(PrnProcName, 3072, pap_session, PrnPort,NULL,NULL,0);
			
			for(i=0;i<PAP_SOCKSUM;i++)
			{
				if(pap_socksum[i].needrelase == 1 )
				{
					cyg_thread_delete(PAPConn1_TaskHdl[i]);
					pap_socksum[i].needrelase = 0;
				}
			}
			
			cyg_semaphore_init(&ATALK_SIGNAL_NO_1,0);
			
			for(i=0;i<PAP_SOCKSUM;i++)
			{
				if(pap_socksum[i].flag == 0 )
				{
					pap_socksum[i].flag = 1;	
									
					//Create PAPConn1 Thread
					cyg_thread_create(PAPConn1_TASK_PRI,
				                  pap_session,
				                  i,
				                  "PAPConn1",
				                  (void *) (PAPConn1_Stack[i]),
				                  PAPConn1_TASK_STACK_SIZE,
				                  &PAPConn1_TaskHdl[i],
				                  &PAPConn1_Task[i]);
					
					//Start PAPConn1 Thread
					cyg_thread_resume(PAPConn1_TaskHdl[i]);

					cyg_semaphore_wait (&ATALK_SIGNAL_NO_1);
					
					break;
				}
			}
			break;
		case PAP_SENDSTATUS :
	    	rbuf[ 0 ] = 0;
		    rbuf[ 1 ] = PAP_STATUS;
		    rbuf[ 2 ] = rbuf[ 3 ] = 0;
		    rbuf[ 4 ] = rbuf[ 5 ] = 0;
	    	rbuf[ 6 ] = rbuf[ 7 ] = 0;
			iov.iov_base = rbuf;
			iov.iov_len = 8 + getstatus( PrnPort, &rbuf[ 8 ] );
			atpb.atp_sresiov = &iov;
			atpb.atp_sresiovcnt = 1;
			//*
			//* This may error out if we lose a route, so we won't die().
			//*
			if ( atp_sresp( pr[PrnPort].p_atp, &atpb ) < 0 ) {
#ifdef _PC
				printf("Error : atp_sresp(2) (papd.c)\n");
#endif _PC
			}
			break;
		default :
#ifdef _PC
			printf("Bad request from %u.%u!\n",
			        ntohs( sat.sat_addr.s_net ),
			        sat.sat_addr.s_node );
#endif _PC
			break;
		}
	} //for (;;) {
}

int getstatus(int PrnPort, uint8 *buf)
{
	int cnt = 0;
	uint8 *len = buf++;

	memcpy(buf,PreStatus,PreLen);
	buf += PreLen;

#ifdef _PC
	switch(urandom(4)) {
#else
	switch(ReadPortStatus(PrnPort)) {
#endif
		case 0:
			cnt = strlen(PrStatus[0]);
			memcpy(buf,PrStatus[0],cnt+1);
			break;
		case 1:
			cnt = strlen(PrStatus[1]);
			memcpy(buf,PrStatus[1],cnt+1);
			break;
		case 2:
			cnt = strlen(PrStatus[2]);
			memcpy(buf,PrStatus[2],cnt+1);
			break;
		case 3:
			cnt = strlen(PrStatus[3]);
			memcpy(buf,PrStatus[3],cnt+1);
			break;
	}
#ifdef _PC
	if(cnt+PreLen+1 > (sizeof(rbuf)-9)) {
		printf("\agetstatus() design error \n");
	}
#endif _PC
	*len = (cnt+PreLen+1);
	return (int)(*len+1);
}
