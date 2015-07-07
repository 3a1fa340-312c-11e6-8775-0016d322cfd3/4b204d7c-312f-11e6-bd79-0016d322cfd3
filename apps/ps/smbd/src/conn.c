/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Manage connections_struct structures
   Copyright (C) Andrew Tridgell 1998
   
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

#include "prnqueue.h"
#include "smbinc.h"
#include "smb.h"

#include "dlinlist.h"

/* set these to define the limits of the server. NOTE These are on a
   per-client basis. Thus any one machine can't connect to more than
   MAX_CONNECTIONS services, but any number of machines may connect at
   one time. */
//0706 static int num_open = 0;
int num_open[NUM_SMBTHREAD];
connection_struct *Connections[NUM_SMBTHREAD];
#define MAX_CONNECTIONS 128
extern uint16 printing_conn_tid[];
extern struct Tempdata *Gtempdata[];
extern int InSMBPrinting[NUM_OF_PRN_PORT];
extern int StartSMBPrinting[NUM_OF_PRN_PORT];
extern uint8 _forceclosethread[NUM_SMBTHREAD]; 

/****************************************************************************
find a conn given a cnum
****************************************************************************/
connection_struct *conn_find(int cnum, int threadid)
{
	int count=0;
	connection_struct *conn;

	for (conn=Connections[threadid];conn;conn=conn->next,count++) {
		if (conn->cnum == cnum) {
			if (count > NUM_SMBTHREAD) {
				DLIST_PROMOTE(Connections[threadid], conn);
			}
			return conn;
		}
	}

	return NULL;
}


/****************************************************************************
  find first available connection slot, starting from a random position.
The randomisation stops problems with the server dieing and clients
thinking the server is still available.
****************************************************************************/
connection_struct *conn_new(pstring *dev, int threadid)
{
	connection_struct *conn;
	static uint16 treeid =1;
	if (num_open[threadid]++ > MAX_CONNECTIONS)
		return NULL;

    conn = (connection_struct *)malloc(sizeof(connection_struct));
	if (!conn) return NULL;

//2/8/2002	ZERO_STRUCTP(conn);
	memset((char *)conn ,0, sizeof(connection_struct));
	conn->cnum = treeid++;
	conn->lastused = (time_t)(msclock() /1000);
	conn->smbprnportid = -1;
 	conn->printer = (strncmp(dev,"LPT",3) == 0);
	conn->ipc = (strncmp(dev,"IPC",3) == 0);
	conn->prnpaperout =0;
	conn->ClientOs = -1;
	
	DLIST_ADD(Connections[threadid], conn);

	return conn;
}

/****************************************************************************
close all conn structures
****************************************************************************/
 
void conn_close_all(int threadid)
{
	connection_struct *conn, *next;
	int i;
	
	for (conn=Connections[threadid];conn;conn=next) {
		for ( i =0; i< 3 ; i++){
			if (printing_conn_tid[i] == conn->cnum){
    			if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed )
				    PrnSetNoUse(conn->smbprnportid);
			}	
		}
		next=conn->next;
		close_cnum(conn, threadid);
	}
}

/****************************************************************************
idle inactive connections
****************************************************************************/
int conn_idle_all(time_t t, int deadtime, int threadid)
{
	int allidle = True;
	connection_struct *conn, *next;

	for (conn=Connections[threadid];conn;conn=next) {
		next=conn->next;

		if ((t - conn->lastused) < deadtime){  
			allidle = False;
		}	
	}
	return allidle;
}
 

/****************************************************************************
free a conn structure
****************************************************************************/

void conn_free(connection_struct *conn, int threadid)
{
	connection_struct *Conn_prev, *Conn_next;
	
	Conn_prev = conn->prev;
	Conn_next = conn->next;
	DLIST_REMOVE(Connections[threadid], conn);
//2/8/2002	ZERO_STRUCTP(conn);
	memset((char *)conn ,0, sizeof(connection_struct));
	num_open[threadid]--;
	free(conn);
	if ((Conn_prev == NULL) && (Conn_next == NULL))  //Ron Add 3/6/2002
		Connections[threadid] = NULL;
}

int conn_num_open(int threadid){

	return num_open[threadid];

}	

/* copy from service.c 0413/2001 */
void close_cnum(connection_struct *conn, int threadid)
{
	conn_free(conn, threadid);
}

/* Add by Ron 3/8/2002 */
void release_conn_resource(connection_struct *conn, int threadid)
{
	struct Tempdata *ptr = NULL, *next =NULL;
	
    if (conn->smbprnportid > -1){  // It's printing connect
	  for (ptr = Gtempdata[threadid]; ptr ; ptr = next){
	    next = ptr->next;
	    if (ptr->treeid != conn->cnum)
	    	continue;
	    if ((ptr->data != NULL))
	      free(ptr->data);
        memset(ptr, 0 , sizeof(struct Tempdata));  
	    free(ptr);  
	    if (next == NULL){ // there is no Data Link, reset global pointer Gtempdata[threadid] to NULL
	      Gtempdata[threadid] = NULL;	
	      break;	
	    }  
	  }	
    if (printing_conn_tid[conn->smbprnportid] == conn->cnum){
	  if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed ) // If dose not reset PrnPort to NoUse
		PrnSetNoUse(conn->smbprnportid);
	 }		  
  }	
}	

void check_idle_conn(int deadtime, int threadid)
{
	connection_struct *conn, *next;
	time_t t = (msclock()/1000);

	for (conn=Connections[threadid];conn;conn=next) {
		next=conn->next;

		if (((t - conn->lastused) > deadtime) && (conn->smbprnportid > -1) ){
		 	release_conn_resource(conn , threadid);
		 	close_cnum(conn, threadid);
/* if User cannceled the Client printing job (maybe paper out or prnqueue full), Windows "Won't" send the 
   tree disconnect SMB command to disconnect this connection. And he will use the pervious treeID to printing 
   in the further. Now I closed the connection by timeout( 60 seconds ), but I can't let Client closed this 
   connection at the same time. Thus I can not service hhe Client in his printing job. So I just only closed this
   thread and Socket, let Client closed his Socket used in this connection. When he open new Socket and tree ID
   to request Printing, I will service him with new Socket and Tree ID.    .......By Ron 3/11 /2002 */
		 	_forceclosethread[threadid] =1;  
		} 	
    }	
}
