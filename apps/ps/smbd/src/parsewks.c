
/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-1997,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1997,
 *  Copyright (C) Paul Ashton                       1997.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
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

#include "rpcwksvc.h"

//from parsemis.c
extern void init_buf_unistr2(UNISTR2 *str, uint32 *ptr, char *buf);
extern int smb_io_unistr2(char *desc, UNISTR2 *uni2, uint32 buffer, prs_struct *ps, int depth);

//from parseprs.c
extern int prs_uint16(char *name, prs_struct *ps, int depth, uint16 *data16);
extern int prs_uint32(char *name, prs_struct *ps, int depth, uint32 *data32);
extern int prs_align(prs_struct *ps);

//0509extern int DEBUGLEVEL;

/*******************************************************************
 Init
 ********************************************************************/

void init_wks_q_query_info(WKS_Q_QUERY_INFO *q_u,
				char *server, uint16 switch_value)  
{
//0509	DEBUG(5,("init_wks_q_query_info\n"));

	init_buf_unistr2(&q_u->uni_srv_name, &q_u->ptr_srv_name, server);
	q_u->switch_value = switch_value;
}

/*******************************************************************
 Reads or writes a WKS_Q_QUERY_INFO structure.
********************************************************************/

int wks_io_q_query_info(char *desc, WKS_Q_QUERY_INFO *q_u, prs_struct *ps, int depth)
{
	if (q_u == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "wks_io_q_query_info");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_srv_name", ps, depth, &q_u->ptr_srv_name))
		return False;
	if(!smb_io_unistr2("", &q_u->uni_srv_name, q_u->ptr_srv_name, ps, depth))
		return False;
	if(!prs_align(ps))
		return False;
/* since prs_align(ps) above can not add 2 to ps->data_offset 
	(since ps ->align ==0) <<why????>>
	for testing, add ps->data_offset at this by ron on 5/14/2001 */
//	ps->data_offset+=2; //since srvpipe.c line 195 command //0605/2001
	if(!prs_uint16("switch_value", ps, depth, &q_u->switch_value))
		return False;
	if(!prs_align(ps))
		return False;

	return True;
}

/*******************************************************************
 wks_info_100
 ********************************************************************/

void init_wks_info_100(WKS_INFO_100 *inf,
				uint32 platform_id, uint32 ver_major, uint32 ver_minor,
				char *my_name, char *domain_name)
{
//0509	DEBUG(5,("Init WKS_INFO_100: %d\n", __LINE__));

	inf->platform_id = platform_id; /* 0x0000 01f4 - unknown */
	inf->ver_major   = ver_major;   /* os major version */
	inf->ver_minor   = ver_minor;   /* os minor version */

	init_buf_unistr2(&inf->uni_compname, &inf->ptr_compname, my_name    );
	init_buf_unistr2(&inf->uni_lan_grp, &inf->ptr_lan_grp, domain_name);
}

/*******************************************************************
 Reads or writes a WKS_INFO_100 structure.
********************************************************************/

static int wks_io_wks_info_100(char *desc, WKS_INFO_100 *inf, prs_struct *ps, int depth)
{
	if (inf == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "wks_io_wks_info_100");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("platform_id ", ps, depth, &inf->platform_id)) /* 0x0000 01f4 - unknown */
		return False;
	if(!prs_uint32("ptr_compname", ps, depth, &inf->ptr_compname)) /* pointer to computer name */
		return False;
	if(!prs_uint32("ptr_lan_grp ", ps, depth, &inf->ptr_lan_grp)) /* pointer to LAN group name */
		return False;
	if(!prs_uint32("ver_major   ", ps, depth, &inf->ver_major)) /* 4 - major os version */
		return False;
	if(!prs_uint32("ver_minor   ", ps, depth, &inf->ver_minor)) /* 0 - minor os version */
		return False;

	if(!smb_io_unistr2("", &inf->uni_compname, inf->ptr_compname, ps, depth))
		return False;
	if(!prs_align(ps))
		return False;

	if(!smb_io_unistr2("", &inf->uni_lan_grp, inf->ptr_lan_grp , ps, depth))
		return False;
	if(!prs_align(ps))
		return False;

	return True;
}

/*******************************************************************
 Inits WKS_R_QUERY_INFO.

 only supports info level 100 at the moment.

 ********************************************************************/

void init_wks_r_query_info(WKS_R_QUERY_INFO *r_u,
				uint32 switch_value, WKS_INFO_100 *wks100,
				int status)  
{
//0509	DEBUG(5,("init_wks_r_unknown_0: %d\n", __LINE__));

	r_u->switch_value = switch_value;  /* same as in request */
//0514	r_u->switch_value = 0x64;  /* same as in request */ //0514

	r_u->ptr_1     = 1;          /* pointer 1 */
	r_u->wks100    = wks100;

	r_u->status    = status;
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

int wks_io_r_query_info(char *desc, WKS_R_QUERY_INFO *r_u, prs_struct *ps, int depth)
{
	if (r_u == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "wks_io_r_query_info");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint16("switch_value", ps, depth, &r_u->switch_value)) /* level 100 (0x64) */
		return False;
	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_1       ", ps, depth, &r_u->ptr_1)) /* pointer 1 */
		return False;
	if(!wks_io_wks_info_100("inf", r_u->wks100, ps, depth))
		return False;

	if(!prs_uint32("status      ", ps, depth, &r_u->status))
		return False;

	return True;
}
