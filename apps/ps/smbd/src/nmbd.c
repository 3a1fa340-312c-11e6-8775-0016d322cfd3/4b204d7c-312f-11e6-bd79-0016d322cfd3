/*
   Unix SMB/Netbios implementation.
   Version 1.9.
   NBT netbios routines and daemon - version 2
   Copyright (C) Andrew Tridgell 1994-1998
   
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
   
   Revision History:

   14 jan 96: lkcl@pires.co.uk
   added multiple workgroup domain master support

*/
#include <cyg/kernel/kapi.h>
#include "network.h"
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "smbinc.h"
#include "smb.h"

typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
}MIB_DHCP;
extern MIB_DHCP                   *mib_DHCP_p;

//psmain.c
extern  uint32 msclock();

//httpd.c
extern  EEPROM	 		EEPROM_Data;
//EEPROM_Data.WorkGroupName = "WORKGROUP";

//n_packet.c
extern  void retransmit_or_expire_resp_records(time_t t);
extern  int listen_for_packets();
extern  void run_packet_queue(void);

//n_ann.c
extern void announce_my_server_names(time_t t);

//utilsock.c
extern int open_socket_in(int type, int port, int dlevel,uint32 socket_addr, int rebind);

//n_subnet.c
extern int create_subnets(void);

//extern  Node2Num(BYTE *buf,BYTE offset);


//NMBD Thread initiation information
#define NMBD_TASK_PRI         	20	//ZOT716u2
#define NMBD_TASK_STACK_SIZE  	8192 //ZOT716u2 8192
static	uint8			NMBD_Stack[NMBD_TASK_STACK_SIZE];
static  cyg_thread		NMBD_Task;
static  cyg_handle_t	NMBD_TaskHdl;


/* have we found LanMan clients yet? */
//0419 int found_lm_clients = FALSE;

int  ClientNMB       = -1;
int  ClientDGRAM     = -1;

char _MyServerName[NCBNAMSZ];
char _MyWorkgroup[NCBNAMSZ]= "WORKGROUP";
char _MyNetBIOSName[NCBNAMSZ];
char  my_netbios_names[MAX_NETBIOS_NAMES][NCBNAMSZ];


/* Convert Internet address in ascii dotted-decimal format (44.0.0.1) to
 * binary IP address
 */
int32
aton(char *s)
{
    int32 n;

    register int i;

    n = 0;
    if(s == NULL)
        return 0;
    for(i=24;i>=0;i -= 8){
    /* Skip any leading stuff (e.g., spaces, '[') */
        while(*s != '\0' && !isdigit(*s))
            s++;
        if(*s == '\0')
            break;
        n |= (int32)atoi(s) << i;
        if((s = strchr(s,'.')) == NULL)
            break;
        s++;
    }
    return n;
}

//form netbeui.c
extern ode2Num(BYTE *buf,BYTE offset);
/*
void Node2Num(BYTE *buf,BYTE offset)	 
{
	BYTE i;

	for(i = offset; i < 6 ; i++) {
		if((*buf = (MyPhysNodeAddress[i]>>4)) > 9) *buf += ('A'-10);
		else *buf += '0';
		buf++;

		if((*buf = (MyPhysNodeAddress[i]&0x0F)) > 9) *buf += ('A'-10);
		else *buf += '0';
		buf++;
	}
}
*/

/**************************************************************************** **
 The main select loop.
 **************************************************************************** */
//os void nb_proc(int nouse, void *nouse1, void *nouse2)
void nb_proc (cyg_addrword_t data)
{
	time_t last;
	time_t t;

	last = (msclock()/1000);

	// 2002/03/22, we only run once
	retransmit_or_expire_resp_records(last);
	
	while( TRUE )
	{
		t = (msclock()/1000);  //per second
		
		/*
		 * Check all broadcast subnets to see if
		 * we need to run an election on any of them.
		 * (nmbd_elections.c)
		 */
		//0205	  run_election = check_elections();
		
		/*
		* Read incoming UDP packets.
		* (nmbd_packets.c)
		*/
		listen_for_packets();
		
		/*
		 * Process all incoming packets
		 * read above. This calls the success and
		 * failure functions registered when response
		 * packets arrrive, and also deals with request
		 * packets from other sources.
		 * (nmbd_packets.c)
		 */

		run_packet_queue();

		/*
		 * If we are configured as a logon server, attempt to
		 * register the special NetBIOS names to become such
		 * (WORKGROUP<1c> name) on all broadcast subnets and
		 * with the WINS server (if used). If we are configured
		 * to become a domain master browser, attempt to register
		 * the special NetBIOS name (WORKGROUP<1b> name) to
		 * become such.
		 * (nmbd_become_dmb.c)
		 */
//0625		 add_domain_names(t);

		if( last != t )
		{

		/*
		 * Send out any broadcast announcements
		 * of our server names. This also announces
		 * the workgroup name if we are a local
		 * master browser.
		 * (nmbd_sendannounce.c)
		 */
			announce_my_server_names(t);
				
		/*
		 * Go through the response record queue and time out or re-transmit
		 * and expired entries.
		 * (nmbd_packets.c)
		 */
//			retransmit_or_expire_resp_records(t); // move out the loop

			last = t;
		}	
		
        sys_check_stack();
		ppause( 50 );
	}
} /* process */


/**************************************************************************** **
 open the socket communication
 **************************************************************************** */
static int open_smb_sockets(int port)
{
	/* The sockets opened here will be used to receive broadcast
	 * packets *only*. Interface specific sockets are opened in
	 * make_subnet() in namedbsubnet.c. Thus we bind to the
	 * address "0.0.0.0". The parameter 'socket address' is
	 * now deprecated.
	 */
	struct timeval rcv_timeout,rcv_timeout1;
	int  broadcastFlag = 1;
	
	ClientNMB = open_socket_in(SOCK_DGRAM, port,0,0,TRUE);
	 
	ClientDGRAM = open_socket_in(SOCK_DGRAM,DGRAM_PORT,3,0,TRUE);
	 
	if ( ClientNMB == -1 )
		return( FALSE );

//Jesse	setsocketopt(ClientNMB,SO_RCV_NOBLOCK);
//Jesse	setsocketopt(ClientDGRAM,SO_RCV_NOBLOCK);

	rcv_timeout.tv_sec = 0;
	rcv_timeout.tv_usec =  10;//65536 * 2;
	
	setsockopt (ClientNMB, 
				SOL_SOCKET, 
				SO_RCVTIMEO,
				&rcv_timeout,
				sizeof(rcv_timeout));

	setsockopt (ClientDGRAM, 
			SOL_SOCKET, 
			SO_RCVTIMEO,
			(char *)&rcv_timeout,
			sizeof(rcv_timeout));
/*	
	setsockopt(ClientNMB, SOL_SOCKET, SO_BROADCAST,
				(char *) &broadcastFlag, sizeof(broadcastFlag));
	setsockopt(ClientDGRAM, SOL_SOCKET, SO_BROADCAST,
				(char *) &broadcastFlag, sizeof(broadcastFlag));			
*/	
	//set_socket_options( ClientNMB,   "SO_BROADCAST" );
	//set_socket_options( ClientDGRAM, "SO_BROADCAST" );	 

	return( TRUE );
} /* open_sockets */

void nb_init()
{
	BYTE NameHeadSize = sizeof(MY_NETBEUI_NAME) -1 ;
	BYTE *pName = _MyNetBIOSName;
	int  i;
	int j;
     
	if( strlen( EEPROM_Data.BoxName ) )
	{
		strncpy( _MyNetBIOSName, EEPROM_Data.BoxName, NCBNAMSZ );
		_MyNetBIOSName[NCBNAMSZ - 1] = 0;
	}
	else
	{
		memcpy(pName,"PS",2);
		pName += 2;
		Node2Num(pName,3);
	}	
	
	for( i = 0; i < MAX_NETBIOS_NAMES; i++ )
		strcpy(  my_netbios_names[i], "" );

//	strcpy( _MyServerName, "Samba 2.0.7" );
    strcpy( _MyServerName, "" );
//	strcpy( _MyWorkgroup, "WIN98" );  //for test
    if( strlen(EEPROM_Data.WorkGroupName) )
	{
		strncpy(_MyWorkgroup, EEPROM_Data.WorkGroupName, NCBNAMSZ );
		_MyWorkgroup[NCBNAMSZ-1] = 0;
	}	
	strcpy( my_netbios_names[0], _MyNetBIOSName );

	while( mib_DHCP_p->IPAddr == 0x0)
		ppause(100);
//Jesse	lan_ip.s_addr = Lanface->addr;
	lan_ip.s_addr = mib_DHCP_p->IPAddr;
	ipzero.s_addr = aton( "0.0.0.0" );
	allones_ip.s_addr = aton( "255.255.255.255" );
	loopback_ip.s_addr = aton( "127.0.0.1" );

	if( open_smb_sockets( NMB_PORT ) )
	{
		if( create_subnets() )
		{
//0420			if( register_my_workgroup_and_names() )
				//
//os				newproc( "NMBD",2312,(void (*)())nb_proc,0,NULL,NULL,0);
			//Create NMBD Thread
			cyg_thread_create(NMBD_TASK_PRI,
								nb_proc,
								0,
								"NMBD",
								(void *) (NMBD_Stack),
								NMBD_TASK_STACK_SIZE,
								&NMBD_TaskHdl,
								&NMBD_Task);
					
			//Start NMBD Thread
			cyg_thread_resume(NMBD_TaskHdl);
		}
	}
}

