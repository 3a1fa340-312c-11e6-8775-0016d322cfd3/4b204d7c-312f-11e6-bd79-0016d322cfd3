/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Main SMB server routines
   Copyright (C) Andrew Tridgell 1992-1998
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
    Foundation, Inc   ., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "smbinc.h"
#include "smb.h"

//for pipes_struct must include
#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"



//SMBD Thread initiation information
#define SMBD_TASK_PRI         	20	//ZOT716u2
#define SMBD_TASK_STACK_SIZE  	3072 //ZOT716u2 3072
static	uint8			SMBD_Stack[SMBD_TASK_STACK_SIZE];
static  cyg_thread		SMBD_Task;
static  cyg_handle_t	SMBD_TaskHdl;

//SMB-Proc Thread initiation information
#define SMB_Proc_TASK_PRI         	20	//ZOT716u2
#define SMB_Proc_TASK_STACK_SIZE  	8192 //ZOT716u2 8192
static	uint8			SMB_Proc_Stack[NUM_SMBTHREAD][SMB_Proc_TASK_STACK_SIZE];
static  cyg_thread		SMB_Proc_Task[NUM_SMBTHREAD];
static  cyg_handle_t	SMB_Proc_TaskHdl[NUM_SMBTHREAD];


int smbthread[NUM_SMBTHREAD];
//form reply.c
extern int max_send[];
extern struct Tempdata *Gtempdata[];
extern int preEndoffset[NUM_SMBTHREAD];
extern uint16 printing_conn_tid[NUM_OF_PRN_PORT];
extern int timeout_paperout[NUM_SMBTHREAD];
//form smbutil.c
extern int Protocol[]; 
//Jesse extern fstring global_myname;
extern fstring remote_machine;
//Jesse extern unsigned long Availmem;      /* Heap memory, ABLKSIZE units */

//from conn.c
extern connection_struct *Connections[NUM_SMBTHREAD];
extern int num_open[NUM_SMBTHREAD];
//from srvpipe.c
extern pipes_struct *Pipes[NUM_SMBTHREAD];
//from process.c
extern uint8 _forceclosethread[NUM_SMBTHREAD]; 



//from httpd.c
extern  EEPROM	 		EEPROM_Data;

//from utilsock.c
extern int open_socket_in(int type, int port, int dlevel,uint32 socket_addr, int rebind);

//from process.c
extern smbd_process(cyg_addrword_t data);

static void Smb_GlobalVariables_init(void){
	int i;
	
	for (i =0; i< NUM_SMBTHREAD; i++){
		smbthread[i] = -1;
		preEndoffset[i] = -1;
		_forceclosethread[i] =0;
		timeout_paperout[i] =0;
		num_open[i]=0;
    	Gtempdata[i] = NULL;		
		max_send[i] = MAX_SEND_SIZE;
		Protocol[i] = PROTOCOL_COREPLUS;
		Connections[i] =NULL;
		Pipes[i] =NULL;
    }
    for (i =0; i< NUM_OF_PRN_PORT; i++){
    	printing_conn_tid[i] = 0;
    }
    for (i =0; i < 3; i++){  
    	if (strlen(EEPROM_Data.ServiceName[i]))
	    	strcpy(Smbprinterserver[i], EEPROM_Data.ServiceName[i]);
    }
}

/****************************************************************************
  open the socket communication
****************************************************************************/
//os static int open_sockets(int nouse, void *nouse1, void *nouse2)
void open_sockets(cyg_addrword_t data)
{
//	int Client[NUM_SMB_SOCKID];
	int Client;
	int s;
	struct timeval rcv_timeout;
	struct linger ling;

		/* open an incoming socket */
	s = open_socket_in(SOCK_STREAM, SMB_PORT, 0, 0, TRUE);
	if (s == -1)
//Jesse		return(False);
		return;

//0711	setsocketopt(s,SO_RCV_NOBLOCK);
/*
	rcv_timeout.tv_usec = 0;
	rcv_timeout.tv_sec = 3;

	setsockopt (s, 
		SOL_SOCKET, 
		SO_RCVTIMEO,
		(char *)&rcv_timeout,
		sizeof(rcv_timeout));
		
	ling.l_onoff = 1;
	ling.l_linger = 0;
		
	setsockopt (s, 
		SOL_SOCKET,
		SO_LINGER,
		(char *)&ling,
		sizeof(ling));  	
*/		
		/* ready to listen */
//Jesse	if (listen(s, 5) == -1) {
	if (listen(s, 128) == -1) {
//Jesse		close_s(s);
		close(s);
//Jesse		return False;
		return;
	}
	
	Smb_GlobalVariables_init(); //add on 6/29/2001

	while (1) {
			struct sockaddr addr;
			int in_addrlen = sizeof(addr);
        	int16 exist_smbthread = 0;  
			int i = 0;

			while (!exist_smbthread){
				for (i =0; i < NUM_SMBTHREAD; i++){
			    	if (smbthread[i] == -1){
			    		if(SMB_Proc_TaskHdl[i]){
			    			cyg_thread_delete(SMB_Proc_TaskHdl[i]);
			    			SMB_Proc_TaskHdl[i] = 0;	
			    		}
						exist_smbthread =1;
						break;
					}                       
				}	  
//os				kwait(NULL);
				cyg_thread_yield();		
			}						
			exist_smbthread = 0;   			  
			
			Client = accept(s,&addr,&in_addrlen);
			
//ZOTIPS			armond_printf("open socket cnt = %d \n",Client);
/*			
			rcv_timeout.tv_usec = 0;
			rcv_timeout.tv_sec = 3;
	
			setsockopt (Client, 
				SOL_SOCKET, 
				SO_RCVTIMEO,
				(char *)&rcv_timeout,
				sizeof(rcv_timeout));
				
			ling.l_onoff = 1;
			ling.l_linger = 0;
				
			setsockopt (Client, 
				SOL_SOCKET,
				SO_LINGER,
				(char *)&ling,
				sizeof(ling));  
*/		  
			if (Client != -1) { 
		  		smbthread[i] = Client;	
//os			if (!newproc("SMB-Proc",2532,(void (*)())smbd_process
//os			  			,Client,NULL,NULL,0)){
//os				smbthread[i] = -1;
//os			  	close_s(Client);   	
//os			}
				memset(SMB_Proc_Stack[i], 0x00, SMB_Proc_TASK_STACK_SIZE);
				//Create SMB-Proc Thread
				cyg_thread_create(SMB_Proc_TASK_PRI,
									smbd_process,
									Client,
									"SMB-Proc",
									(void *) (SMB_Proc_Stack[i]),
									SMB_Proc_TASK_STACK_SIZE,
									&SMB_Proc_TaskHdl[i],
									&SMB_Proc_Task[i]);
								
				//Start SMB-Proc Thread
				cyg_thread_resume(SMB_Proc_TaskHdl[i]);	
					
		 }
	} /* end while 1 */
}


/****************************************************************************
  main program
****************************************************************************/
int smbmain()
{
	int opt;
	extern char *optarg;

#ifdef HAVE_SETLUID
	/* needed for SecureWare on SCO */
	setluid(0);
#endif

	pstrcpy(remote_machine, "smb");
//os	newproc( "SMBD",768,(void (*)())open_sockets,0,NULL,NULL,0);
	//Create SMBD Thread
	cyg_thread_create(SMBD_TASK_PRI,
						open_sockets,
						0,
						"SMBD",
						(void *) (SMBD_Stack),
						SMBD_TASK_STACK_SIZE,
						&SMBD_TaskHdl,
						&SMBD_Task);
					
	//Start SMBD Thread
	cyg_thread_resume(SMBD_TaskHdl);
	return(0);
}
