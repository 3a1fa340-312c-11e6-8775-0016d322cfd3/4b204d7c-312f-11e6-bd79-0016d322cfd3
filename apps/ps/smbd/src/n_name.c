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

#include "dlinlist.h"

uint16 samba_nb_type = 0; /* samba's NetBIOS name type */


//form psmain.c
extern char * strupr ( char * string );

//from nmblib.c
extern void make_nmb_name( struct nmb_name *n, const char *name, int type );

/* ************************************************************************** **
 * Set Samba's NetBIOS name type.
 * ************************************************************************** **
 */
void set_samba_nb_type(void)
{
    samba_nb_type = NB_BFLAG;           /* samba is broadcast-only node type. */
} /* set_samba_nb_type */

/* ************************************************************************** **
 * Convert a NetBIOS name to upper case.
 * ************************************************************************** **
 */
static void upcase_name( struct nmb_name *target, struct nmb_name *source )
{
	int i;

	if( NULL != source )
		(void)memcpy( target, source, sizeof( struct nmb_name ) );

	strupr( target->name );
	strupr( target->scope );

	/* fudge... We're using a byte-by-byte compare, so we must be sure that
	 * unused space doesn't have garbage in it.
	 */
	for( i = strlen( target->name ); i < sizeof( target->name ); i++ )
		target->name[i] = '\0';
	for( i = strlen( target->scope ); i < sizeof( target->scope ); i++ )
		target->scope[i] = '\0';
} /* upcase_name */

/* ************************************************************************** **
 * Add a new or overwrite an existing namelist entry.
 * ************************************************************************** **
 */
static void update_name_in_namelist( struct subnet_record *subrec,
                              struct name_record *namerec )
{
	struct name_record *n;

	for (n = subrec->namelist; n; n = n->next)
	{
		if( memcmp( &n->name, &namerec->name, sizeof(struct nmb_name) ) == 0 )
		{
			DLIST_REMOVE( subrec->namelist, n );
			free( n );
			break;
		}	
	}

	DLIST_ADD(subrec->namelist, namerec);

} /* update_name_in_namelist */

/* ************************************************************************** **
 * Find a name in a subnet.
 * ************************************************************************** **
 */
struct name_record *find_name_on_subnet( struct subnet_record *subrec,
                                         struct nmb_name      *nmbname,
                                         int                  self_only )
{
	struct name_record *name_ret = NULL;
	struct name_record *n;
	struct nmb_name    uc_name[1];
	int i = 0;

	upcase_name( uc_name, nmbname );
	for (n = subrec->namelist; n; n = n->next)
	{
		if( !strcmp( &n->name, uc_name ) )
		{
			name_ret = n;
			break;
		}
	}	
	if( name_ret )
	{
		/* Self names only - these include permanent names. */
		if( self_only
			&& (name_ret->data.source != SELF_NAME)
			&& (name_ret->data.source != PERMANENT_NAME) )
		{
			return( NULL );
		}
		return( name_ret );
	}
	return( NULL );
} /* find_name_on_subnet */

/* ************************************************************************** **
 * Find a name over all known broadcast subnets.
 * ************************************************************************** **
 */
struct name_record *find_name_for_remote_broadcast_subnet(
                                                   struct nmb_name *nmbname,
                                                   int             self_only )
  {
//0316  struct subnet_record *subrec;
  struct subnet_record *subrec;
  struct name_record   *namerec = NULL;
  int i;

  for( i = 0; i < MAX_SUBNETS; i++ )
	{  
  	  if( _subnetlist[i].bInUsed && (&_subnetlist[i] != unicast_subnet) )
		{
			subrec = &_subnetlist[i];
			if( NULL != (namerec = find_name_on_subnet(subrec, nmbname, self_only)) )
     		 	break;	     
   	   	}
	}
  return( namerec );
  } /* find_name_for_remote_broadcast_subnet */
  
/* ************************************************************************** **
 * Update the ttl of an entry in a subnet name list.
 * ************************************************************************** **
 */
#if 0 //0424
void update_name_ttl( struct name_record *namerec, int ttl )
{
	time_t time_now = (msclock()/10);

	if( namerec->data.death_time != PERMANENT_TTL )
		namerec->data.death_time = time_now + ttl;

	namerec->data.refresh_time = time_now + MIN((ttl/2), MAX_REFRESH_TIME);

	namerec->subnet->namelist_changed = True;
} /* update_name_ttl */
#endif //0
/* ************************************************************************** **
 * Add an entry to a subnet name list.
 * ************************************************************************** **
 */
struct name_record *add_name_to_subnet( struct subnet_record *subrec,
                                        char                 *name,
                                        int                   type,
                                        uint16                nb_flags,
                                        int                   ttl,
                                        enum name_source      source,
                                        int                   num_ips,
                                        struct in_addr       *iplist)
{
	struct name_record *namerec;

	namerec = (struct name_record *)malloc( sizeof(struct name_record) );
	if( NULL == namerec )
	{
		return( NULL );
	}

	memset( (char *)namerec, '\0', sizeof(struct name_record) );
	namerec->data.ip = &namerec->data.iplst;

	if( num_ips != 1 )
	{
		free( (char *)namerec );
		return NULL;
	}

	namerec->subnet = subrec;

//Jesse	make_nmb_name( &namerec->name, name, 1 );
	make_nmb_name( &namerec->name, name, type );
	upcase_name( &namerec->name, NULL );

	/* Enter the name as active. */
	namerec->data.nb_flags = nb_flags | NB_ACTIVE;

	/* If it's our primary name, flag it as so. */
	if( !strcmp( my_netbios_names[0], name ) )
		namerec->data.nb_flags |= NB_PERM;

	/* Copy the IPs. */
	namerec->data.num_ips = num_ips;
	memcpy( (namerec->data.ip), iplist, num_ips * sizeof(struct in_addr) );

	/* Data source. */
	namerec->data.source = source;

	/* Setup the death_time and refresh_time. */
	if( ttl == PERMANENT_TTL )
		namerec->data.death_time = PERMANENT_TTL;
	else
		namerec->data.death_time = (msclock()/10) + ttl;

	namerec->data.refresh_time = (msclock()/10) + MIN((ttl/2), MAX_REFRESH_TIME);

	/* Now add the record to the name list. */    
	update_name_in_namelist( subrec, namerec );

	subrec->namelist_changed = True;

	return(namerec);
}

/*******************************************************************
 Utility function automatically called when a name refresh or register 
 succeeds. By definition this is a SELF_NAME (or we wouldn't be registering
 it).
 ******************************************************************/

void standard_success_register(struct subnet_record *subrec, 
                             struct userdata_struct *userdata,
//0420                             struct nmb_name *nmbname, uint16 nb_flags, int ttl,
//Jesse                             char *nmbname, uint16 nb_flags, int ttl,
							struct nmb_name *nmbname, uint16 nb_flags, int ttl,
                             struct in_addr registered_ip)
{
	struct name_record *namerec;

	registered_ip.s_addr = htonl(lan_ip.s_addr);

	namerec = find_name_on_subnet( subrec, nmbname, FIND_SELF_NAME );
//Jesse
namerec = NULL;

	if( NULL == namerec )
//0420		add_name_to_subnet( subrec, nmbname->name, nmbname->name_type,
//Jesse		add_name_to_subnet( subrec, nmbname, 1,
			add_name_to_subnet( subrec, nmbname->name, nmbname->name_type,
                              nb_flags, ttl, SELF_NAME, 1, &registered_ip );
//0424	else
//0424		update_name_ttl( namerec, ttl );
}


