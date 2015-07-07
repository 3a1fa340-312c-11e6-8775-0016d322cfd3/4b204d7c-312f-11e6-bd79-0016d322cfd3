/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   multiple interface handling
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cyg/kernel/kapi.h>
#include "network.h"

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



struct in_addr ipzero;
struct in_addr allones_ip;
struct in_addr loopback_ip;
struct in_addr lan_ip;

/****************************************************************************
  check if an IP is one of mine
  **************************************************************************/
int ismyip(struct in_addr ip)
{
	/*
	struct interface *i;
	for (i=local_interfaces;i;i=i->next)
		if (ip_equal(i->ip,ip)) return True;
	return False;
	*/
//Jesse	if( Lanface->addr == ip.s_addr )
	if( mib_DHCP_p->IPAddr == ip.s_addr )
		return True;
	return False;
}

/****************************************************************************
  check if a packet is from a local (known) net
  **************************************************************************/
int is_local_net(struct in_addr from)
{
/*Jesse
	register struct iface *ifp;

	for(ifp = Ifaces; ifp != NULL; ifp = ifp->next)
	{
		if((from.s_addr & ifp->netmask) == (ifp->addr & ifp->netmask) )
			return True;
	}	
	return False;
*/	
	if((from.s_addr & mib_DHCP_p->SubnetMask) == (mib_DHCP_p->IPAddr & mib_DHCP_p->SubnetMask) ){
			return True;
	}	
	return False;
}

/****************************************************************************
 True if we have two or more interfaces.
  **************************************************************************/
int we_are_multihomed(void)
{
	return False;
}

struct in_addr *iface_ip(struct in_addr ip)
{
	return &lan_ip;
}
