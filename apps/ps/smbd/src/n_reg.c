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
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"

//n_name.c
extern void standard_success_register(struct subnet_record *subrec, 
                             struct userdata_struct *userdata,
//0420                             struct nmb_name *nmbname, uint16 nb_flags, int ttl,
//Jesse                             char *nmbname, uint16 nb_flags, int ttl,
                             struct nmb_name *nmbname, uint16 nb_flags, int ttl,
                             struct in_addr registered_ip);

/****************************************************************************
 Deal with a timeout when registering one of our names.
****************************************************************************/

//0420 static void register_name_timeout_response(struct subnet_record *subrec,
void register_name_timeout_response(struct subnet_record *subrec,
                       struct response_record *rrec)
{
	uint16 nb_flags = 0;
	int ttl = 0;
	struct in_addr registered_ip;
//	char temp[20];
	struct nmb_name temp;
/*
//Jesse
	memset(&temp.name[0], 0, sizeof(struct nmb_name));
	sprintf( &temp.name[0], "%s%c", _MyWorkgroup, 0x1E );
//	memcpy(&temp.name[0],_MyWorkgroup,16);
	temp.scope[0] = 0x1E;
	temp.name_type = 0x1E;
	standard_success_register(subrec, rrec->userdata, &temp, NB_GROUP, ttl, registered_ip);

//Jesse
	memset(&temp.name[0], 0, sizeof(struct nmb_name));
	sprintf( &temp.name[0], "%s%c", _MyWorkgroup, 0x00 );
//	memcpy(&temp.name[0],_MyWorkgroup,16);
	temp.name_type = 0x00;
	standard_success_register(subrec, rrec->userdata, &temp, NB_GROUP, ttl, registered_ip);
*/
//Jesse	memset(temp, 0, 20);
	memset(&temp.name[0], 0, sizeof(struct nmb_name));
	sprintf( &temp.name[0], "%s%c", my_netbios_names[0], 0x20 );
//	memcpy(&temp.name[0],my_netbios_names[0],16);
	temp.name_type = 0x20;
	standard_success_register(subrec, rrec->userdata, &temp, nb_flags, ttl, registered_ip);

	memset(&temp.name[0], 0, sizeof(struct nmb_name));
	sprintf( &temp.name[0], "%s%c", my_netbios_names[0], 0x03 );
//	memcpy(&temp.name[0],my_netbios_names[0],16);
	temp.name_type = 0x03;
	standard_success_register(subrec, rrec->userdata, &temp, nb_flags, ttl, registered_ip);

	memset(&temp.name[0], 0, sizeof(struct nmb_name));
	sprintf( &temp.name[0], "%s%c", my_netbios_names[0], 0x00 );
//	memcpy(&temp.name[0],my_netbios_names[0],16);
	temp.name_type = 0x00;
	standard_success_register(subrec, rrec->userdata, &temp, nb_flags, ttl, registered_ip);

//Jesse
	memset(&temp.name[0], 0, sizeof(struct nmb_name));
	temp.name[0]='*';
	temp.name_type = 0x00;
	standard_success_register(subrec, rrec->userdata, &temp, nb_flags, ttl, registered_ip);
	
}

