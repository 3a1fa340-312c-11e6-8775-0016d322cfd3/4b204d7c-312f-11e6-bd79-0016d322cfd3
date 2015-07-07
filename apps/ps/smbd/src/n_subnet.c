/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   NBT netbios routines and daemon - version 2
   Copyright (C) Andrew Tridgell 1994-1998
   Copyright (C) Luke Kenneth Casson Leighton 1994-1998
   Copyright (C) Jeremy Allison 1994-1998
   
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

#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"


typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
}MIB_DHCP;
extern MIB_DHCP                   *mib_DHCP_p;

extern unsigned long inet_address(unsigned long addr);

/* This is the broadcast subnets database. */
struct subnet_record _subnetlist[MAX_SUBNETS];

/* Extra subnets - keep these separate so enumeration code doesn't
   run onto it by mistake. */

struct subnet_record *unicast_subnet = NULL;
struct subnet_record *remote_broadcast_subnet = NULL;

/****************************************************************************
  Add a subnet into the list.
  **************************************************************************/

static void add_subnet(struct subnet_record *subrec)
{
	subrec->bInUsed = TRUE;
}

/****************************************************************************
  Create a subnet entry.
  ****************************************************************************/

static struct subnet_record *make_subnet(char *name, enum subnet_type type,
					 struct in_addr myip, struct in_addr bcast_ip, 
					 struct in_addr mask_ip)
{
	struct subnet_record *subrec = NULL;
	int nmb_sock =-1 , dgram_sock=-1, i;
	int broadcastFlag = 1; 
	
	/* Check if we are creating a non broadcast subnet - if so don't create
	   sockets.
	*/

	if(type != NORMAL_SUBNET)
	{
		nmb_sock = -1;
		dgram_sock = -1;
	}
	else
	{
		/*
		 * Attempt to open the sockets on port 137/138 for this interface
		 * and bind them.
		 * Fail the subnet creation if this fails.
		 */
// Ron 7/21/2003
/*		if( ( nmb_sock = open_socket_in(SOCK_DGRAM,NMB_PORT,0,myip.s_addr,TRUE)) == -1)
		{
			return NULL;
		}

		if( ( dgram_sock = open_socket_in(SOCK_DGRAM,DGRAM_PORT,3,myip.s_addr,TRUE)) == -1)
		{
			return NULL;
		}

		// Make sure we can broadcast from these sockets. 
		setsockopt(nmb_sock, SOL_SOCKET, SO_BROADCAST,
				(char *) &broadcastFlag, sizeof(broadcastFlag));
		setsockopt(dgram_sock, SOL_SOCKET, SO_BROADCAST,
				(char *) &broadcastFlag, sizeof(broadcastFlag));		

		//set_socket_options(nmb_sock,"SO_BROADCAST");
		//set_socket_options(dgram_sock,"SO_BROADCAST");
*/
	}

	for( i = 0; i < MAX_SUBNETS; i++ )
	{
		if( _subnetlist[i].bInUsed == FALSE )
		{
			subrec = &_subnetlist[i];
			break;
		}
	}
  
	memset( (char *)subrec, 0, sizeof(struct subnet_record) );

	strcpy( subrec->subnet_name, name );
 
	subrec->namelist_changed = FALSE;
	subrec->work_changed = FALSE;
 
	subrec->bcast_ip = bcast_ip;
	subrec->mask_ip  = mask_ip;
	subrec->myip = myip;
	subrec->type = type;
	subrec->nmb_sock = nmb_sock;
	subrec->dgram_sock = dgram_sock;
  
	return subrec;
}

/****************************************************************************
  Create a normal subnet
**************************************************************************/
//Jesse struct subnet_record *make_normal_subnet( struct iface *iface )
struct subnet_record *make_normal_subnet( struct MIB_DHCP_s *iface )
{
	struct subnet_record *subrec;
	struct name_record *namerec;
	struct in_addr myip;
	struct in_addr bcast_ip;
	struct in_addr mask_ip;
	struct in_addr myip_temp;

//Jesse	myip.s_addr = iface->addr;
//Jesse	bcast_ip.s_addr = iface->broadcast;
//Jesse	mask_ip.s_addr = iface->netmask;
	myip.s_addr = iface->IPAddr;
	bcast_ip.s_addr = INADDR_BROADCAST;
	mask_ip.s_addr = iface->SubnetMask;
	myip_temp.s_addr = inet_address(iface->IPAddr);
	
//Jesse	subrec = make_subnet( (char *)inet_ntoa(iface->addr), NORMAL_SUBNET,
//Jesse			     myip, bcast_ip, mask_ip );
	subrec = make_subnet( (char *)inet_ntoa( myip_temp ), NORMAL_SUBNET,
							myip, bcast_ip, mask_ip );
	if(subrec)
		add_subnet(subrec);
	return subrec;
}


/****************************************************************************
  Create subnet entries.
**************************************************************************/

int create_subnets(void)
{
	struct in_addr unicast_ip;
//Jesse	struct iface *ifp;
	struct MIB_DHCP_s  *ifp;
	int i;
	
	// initialize subnetlsit
	for( i = 0; i < MAX_SUBNETS; i++ )
	{
		memset( &_subnetlist[i], 0, sizeof(_subnetlist[i]) );
		_subnetlist[i].bInUsed = FALSE;
	}

	/* 
	 * Create subnets from all the local interfaces and thread them onto
	 * the linked list. 
	 */

//Jesse	ifp = Lanface;
	ifp = mib_DHCP_p;
	if( !make_normal_subnet(ifp) )
		return FALSE;

	/* We should not be using a WINS server at all. Set the
		ip address of the subnet to be zero. */
	unicast_ip = ipzero;

	/*
	 * Create the unicast and remote broadcast subnets.
	 * Don't put these onto the linked list.
	 * The ip address of the unicast subnet is set to be
	 * the WINS server address, if it exists, or ipzero if not.
	 */

	unicast_subnet = make_subnet( "UNICAST_SUBNET", UNICAST_SUBNET, 
								   unicast_ip, unicast_ip, unicast_ip);

	remote_broadcast_subnet = make_subnet( "REMOTE_BROADCAST_SUBNET",
										REMOTE_BROADCAST_SUBNET,
										ipzero, ipzero, ipzero);

	if((unicast_subnet == NULL) || (remote_broadcast_subnet == NULL))
		return FALSE;

	return TRUE;
}


