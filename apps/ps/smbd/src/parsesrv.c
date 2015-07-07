
/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-1997,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1997,
 *  Copyright (C) Paul Ashton                       1997.
 *  Copyright (C) Jeremy Allison                    1999.
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

#include "rpc_srv.h"


//from parsemis.c
extern void init_unistr2(UNISTR2 *str, char *buf, int len);
extern int smb_io_unistr2(char *desc, UNISTR2 *uni2, uint32 buffer, prs_struct *ps, int depth);
extern int prs_uint32(char *name, prs_struct *ps, int depth, uint32 *data32);
extern int smb_io_enum_hnd(char *desc, ENUM_HND *hnd, prs_struct *ps, int depth);
extern void init_buf_unistr2(UNISTR2 *str, uint32 *ptr, char *buf);

//from parseprs.c
extern int prs_align(prs_struct *ps);

//0514extern int DEBUGLEVEL;

/*******************************************************************
 Inits a SH_INFO_1_STR structure
********************************************************************/

void init_srv_share_info1_str(SH_INFO_1_STR *sh1, char *net_name, char *remark)
{
//	DEBUG(5,("init_srv_share_info1_str\n"));

	init_unistr2(&sh1->uni_netname, net_name, strlen(net_name)+1);
	init_unistr2(&sh1->uni_remark, remark, strlen(remark)+1);
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

static int srv_io_share_info1_str(char *desc, SH_INFO_1_STR *sh1, prs_struct *ps, int depth)
{
	if (sh1 == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_share_info1_str");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!smb_io_unistr2("", &sh1->uni_netname, True, ps, depth))
		return False;
	if(!smb_io_unistr2("", &sh1->uni_remark, True, ps, depth))
		return False;

	return True;
}

/*******************************************************************
 makes a SH_INFO_1 structure
********************************************************************/

void init_srv_share_info1(SH_INFO_1 *sh1, char *net_name, uint32 type, char *remark)
{
//	DEBUG(5,("init_srv_share_info1: %s %8x %s\n", net_name, type, remark));

	sh1->ptr_netname = (net_name != NULL) ? 1 : 0;
	sh1->type        = type;
	sh1->ptr_remark  = (remark != NULL) ? 1 : 0;
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

static int srv_io_share_info1(char *desc, SH_INFO_1 *sh1, prs_struct *ps, int depth)
{
	if (sh1 == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_share_info1");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_netname", ps, depth, &sh1->ptr_netname))
		return False;
	if(!prs_uint32("type       ", ps, depth, &sh1->type))
		return False;
	if(!prs_uint32("ptr_remark ", ps, depth, &sh1->ptr_remark))
		return False;

	return True;
}

/*******************************************************************
 Inits a SH_INFO_2_STR structure
********************************************************************/

void init_srv_share_info2_str(SH_INFO_2_STR *sh2,
				char *net_name, char *remark,
				char *path)
{
//	DEBUG(5,("init_srv_share_info2_str\n"));

	init_unistr2(&sh2->uni_netname, net_name, strlen(net_name)+1);
	init_unistr2(&sh2->uni_remark, remark, strlen(remark)+1);
	init_unistr2(&sh2->uni_path, path, strlen(path)+1);
//0609	init_unistr2(&sh2->uni_passwd, passwd, strlen(passwd)+1);
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

static int srv_io_share_info2_str(char *desc, SH_INFO_2_STR *sh2, prs_struct *ps, int depth)
{
	if (sh2 == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_share_info2_str");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!smb_io_unistr2("", &sh2->uni_netname, True, ps, depth))
		return False;
	if(!smb_io_unistr2("", &sh2->uni_remark, True, ps, depth))
		return False;
	if(!smb_io_unistr2("", &sh2->uni_path, True, ps, depth))
		return False;
	if(!smb_io_unistr2("", &sh2->uni_passwd, True, ps, depth))
		return False;

	return True;
}

/*******************************************************************
 Inits a SH_INFO_2 structure
********************************************************************/

void init_srv_share_info2(SH_INFO_2 *sh2,
				char *net_name, uint32 type, char *remark,
				uint32 perms, uint32 max_uses, uint32 num_uses,
				char *path)
{
//	DEBUG(5,("init_srv_share_info2: %s %8x %s\n", net_name, type, remark));

	sh2->ptr_netname = (net_name != NULL) ? 1 : 0;
	sh2->type        = type;
	sh2->ptr_remark  = (remark != NULL) ? 1 : 0;
	sh2->perms       = perms;
	sh2->max_uses    = max_uses;
	sh2->num_uses    = num_uses;
	sh2->type        = type;
	sh2->ptr_path    = (path != NULL) ? 1 : 0;
	sh2->ptr_passwd  = 0;
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

static int srv_io_share_info2(char *desc, SH_INFO_2 *sh2, prs_struct *ps, int depth)
{
	if (sh2 == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_share_info2");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_netname", ps, depth, &sh2->ptr_netname))
		return False;
	if(!prs_uint32("type       ", ps, depth, &sh2->type))
		return False;
	if(!prs_uint32("ptr_remark ", ps, depth, &sh2->ptr_remark))
		return False;
	if(!prs_uint32("perms      ", ps, depth, &sh2->perms))
		return False;
	if(!prs_uint32("max_uses   ", ps, depth, &sh2->max_uses))
		return False;
	if(!prs_uint32("num_uses   ", ps, depth, &sh2->num_uses))
		return False;
	if(!prs_uint32("ptr_path   ", ps, depth, &sh2->ptr_path))
		return False;
	if(!prs_uint32("ptr_passwd ", ps, depth, &sh2->ptr_passwd))
		return False;

	return True;
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

static int srv_io_srv_share_ctr(char *desc, SRV_SHARE_INFO_CTR *ctr, prs_struct *ps, int depth)
{
	if (ctr == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_srv_share_ctr");
	depth++;

	if (UNMARSHALLING(ps)) {
		memset(ctr, '\0', sizeof(SRV_SHARE_INFO_CTR));
	}

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("info_level", ps, depth, &ctr->info_level))
		return False;

	if (ctr->info_level == 0)
		return True;

	if(!prs_uint32("switch_value", ps, depth, &ctr->switch_value))
		return False;
	if(!prs_uint32("ptr_share_info", ps, depth, &ctr->ptr_share_info))
		return False;

	if (ctr->ptr_share_info == 0)
		return True;

	if(!prs_uint32("num_entries", ps, depth, &ctr->num_entries))
		return False;
	if(!prs_uint32("ptr_entries", ps, depth, &ctr->ptr_entries))
		return False;

	if (ctr->ptr_entries == 0) {
		if (ctr->num_entries == 0)
			return True;
		else
			return False;
	}

	if(!prs_uint32("num_entries2", ps, depth, &ctr->num_entries2))
		return False;

	if (ctr->num_entries2 != ctr->num_entries)
		return False;

	switch (ctr->switch_value) {
	case 1:
	{
		SRV_SHARE_INFO_1 *info1 = ctr->share.info1;
		int num_entries = ctr->num_entries;
		int i;

		if (UNMARSHALLING(ps)) {
			if (!(info1 = malloc(num_entries * sizeof(SRV_SHARE_INFO_1))))
				return False;
			memset(info1, '\0', num_entries * sizeof(SRV_SHARE_INFO_1));
			ctr->share.info1 = info1;
		}

		for (i = 0; i < num_entries; i++) {
			if(!srv_io_share_info1("", &info1[i].info_1, ps, depth))
				return False;
		}

		for (i = 0; i < num_entries; i++) {
			if(!srv_io_share_info1_str("", &info1[i].info_1_str, ps, depth))
				return False;
		}

		break;
	}

	case 2:
	{
		SRV_SHARE_INFO_2 *info2 = ctr->share.info2;
		int num_entries = ctr->num_entries;
		int i;

		if (UNMARSHALLING(ps)) {
			if (!(info2 = malloc(num_entries * sizeof(SRV_SHARE_INFO_2))))
				return False;
			memset(info2, '\0', num_entries * sizeof(SRV_SHARE_INFO_2));
			ctr->share.info2 = info2;
		}

		for (i = 0; i < num_entries; i++) {
			if(!srv_io_share_info2("", &info2[i].info_2, ps, depth))
				return False;
		}

		for (i = 0; i < num_entries; i++) {
			if(!srv_io_share_info2_str("", &info2[i].info_2_str, ps, depth))
				return False;
		}

		break;
	}

	default:
//		DEBUG(5,("%s no share info at switch_value %d\n",
//			 tab_depth(depth), ctr->switch_value));
		break;
	}

	return True;
}

/*******************************************************************
 Frees a SRV_SHARE_INFO_CTR structure.
********************************************************************/

void free_srv_share_info_ctr(SRV_SHARE_INFO_CTR *ctr)
{
	if(!ctr)
		return;
	if(ctr->share.info)
		free(ctr->share.info);
	memset(ctr, '\0', sizeof(SRV_SHARE_INFO_CTR));
}

/*******************************************************************
 Frees a SRV_Q_NET_SHARE_ENUM structure.
********************************************************************/

void free_srv_q_net_share_enum(SRV_Q_NET_SHARE_ENUM *q_n)
{
	if(!q_n)
		return;
	free_srv_share_info_ctr(&q_n->ctr);
	memset(q_n, '\0', sizeof(SRV_Q_NET_SHARE_ENUM));
}

/*******************************************************************
 Frees a SRV_R_NET_SHARE_ENUM structure.
********************************************************************/

void free_srv_r_net_share_enum(SRV_R_NET_SHARE_ENUM *r_n)
{
	if(!r_n)
		return;
	free_srv_share_info_ctr(&r_n->ctr);
	memset(r_n, '\0', sizeof(SRV_R_NET_SHARE_ENUM));
}

/*******************************************************************
 Inits a SRV_Q_NET_SHARE_ENUM structure.
********************************************************************/
#if 0 //0514
void init_srv_q_net_share_enum(SRV_Q_NET_SHARE_ENUM *q_n, 
				char *srv_name, uint32 info_level,
				uint32 preferred_len, ENUM_HND *hnd)
{

//	DEBUG(5,("init_q_net_share_enum\n"));

	init_buf_unistr2(&q_n->uni_srv_name, &q_n->ptr_srv_name, srv_name);

	q_n->ctr.info_level = q_n->ctr.switch_value = info_level;
	q_n->ctr.ptr_share_info = 0;
	q_n->preferred_len = preferred_len;

	memcpy(&q_n->enum_hnd, hnd, sizeof(*hnd));
}
#endif
/*******************************************************************
 Reads or writes a structure.
********************************************************************/

int srv_io_q_net_share_enum(char *desc, SRV_Q_NET_SHARE_ENUM *q_n, prs_struct *ps, int depth)
{
	if (q_n == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_q_net_share_enum");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_srv_name", ps, depth, &q_n->ptr_srv_name))
		return False;
	if(!smb_io_unistr2("", &q_n->uni_srv_name, True, ps, depth))
		return False;

	if(!srv_io_srv_share_ctr("share_ctr", &q_n->ctr, ps, depth))
		return False;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("preferred_len", ps, depth, &q_n->preferred_len))
		return False;

	if(!smb_io_enum_hnd("enum_hnd", &q_n->enum_hnd, ps, depth))
		return False;

	return True;
}

/*******************************************************************
 Reads or writes a structure.
********************************************************************/

int srv_io_r_net_share_enum(char *desc, SRV_R_NET_SHARE_ENUM *r_n, prs_struct *ps, int depth)
{
	if (r_n == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_r_net_share_enum");
	depth++;

	if(!srv_io_srv_share_ctr("share_ctr", &r_n->ctr, ps, depth))
		return False;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("total_entries", ps, depth, &r_n->total_entries))
		return False;
	if(!smb_io_enum_hnd("enum_hnd", &r_n->enum_hnd, ps, depth))
		return False;
	if(!prs_uint32("status     ", ps, depth, &r_n->status))
		return False;

	return True;
}

/*******************************************************************
 Inits a SRV_INFO_101 structure.
 ********************************************************************/

void init_srv_info_101(SRV_INFO_101 *sv101, uint32 platform_id, char *name,
				uint32 ver_major, uint32 ver_minor,
				uint32 srv_type, char *comment)
{
//0514	DEBUG(5,("init_srv_info_101\n"));

	sv101->platform_id  = platform_id;
	init_buf_unistr2(&sv101->uni_name, &sv101->ptr_name, name);
	sv101->ver_major    = ver_major;
	sv101->ver_minor    = ver_minor;
	sv101->srv_type     = srv_type;
	init_buf_unistr2(&sv101->uni_comment, &sv101->ptr_comment, comment);
}

/*******************************************************************
 Reads or writes a SRV_INFO_101 structure.
 ********************************************************************/

static int srv_io_info_101(char *desc, SRV_INFO_101 *sv101, prs_struct *ps, int depth)
{
	if (sv101 == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_info_101");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("platform_id ", ps, depth, &sv101->platform_id))
		return False;
	if(!prs_uint32("ptr_name    ", ps, depth, &sv101->ptr_name))
		return False;
	if(!prs_uint32("ver_major   ", ps, depth, &sv101->ver_major))
		return False;
	if(!prs_uint32("ver_minor   ", ps, depth, &sv101->ver_minor))
		return False;
	if(!prs_uint32("srv_type    ", ps, depth, &sv101->srv_type))
		return False;
	if(!prs_uint32("ptr_comment ", ps, depth, &sv101->ptr_comment))
		return False;

	if(!prs_align(ps))
		return False;

	if(!smb_io_unistr2("uni_name    ", &sv101->uni_name, True, ps, depth))
		return False;
	if(!smb_io_unistr2("uni_comment ", &sv101->uni_comment, True, ps, depth))
		return False;

	return True;
}

/*******************************************************************
 Inits a SRV_INFO_102 structure.
 ********************************************************************/

void init_srv_info_102(SRV_INFO_102 *sv102, uint32 platform_id, char *name,
				char *comment, uint32 ver_major, uint32 ver_minor,
				uint32 srv_type, uint32 users, uint32 disc, uint32 hidden,
				uint32 announce, uint32 ann_delta, uint32 licenses,
				char *usr_path)
{
//0514	DEBUG(5,("init_srv_info_102\n"));

	sv102->platform_id  = platform_id;
	init_buf_unistr2(&sv102->uni_name, &sv102->ptr_name, name);
	sv102->ver_major    = ver_major;
	sv102->ver_minor    = ver_minor;
	sv102->srv_type     = srv_type;
	init_buf_unistr2(&sv102->uni_comment, &sv102->ptr_comment, comment);

	/* same as 101 up to here */

	sv102->users        = users;
	sv102->disc         = disc;
	sv102->hidden       = hidden;
	sv102->announce     = announce;
	sv102->ann_delta    =ann_delta;
	sv102->licenses     = licenses;
	init_buf_unistr2(&sv102->uni_usr_path, &sv102->ptr_usr_path, usr_path);
}


/*******************************************************************
 Reads or writes a SRV_INFO_102 structure.
 ********************************************************************/

static int srv_io_info_102(char *desc, SRV_INFO_102 *sv102, prs_struct *ps, int depth)
{
	if (sv102 == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_info102");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("platform_id ", ps, depth, &sv102->platform_id))
		return False;
	if(!prs_uint32("ptr_name    ", ps, depth, &sv102->ptr_name))
		return False;
	if(!prs_uint32("ver_major   ", ps, depth, &sv102->ver_major))
		return False;
	if(!prs_uint32("ver_minor   ", ps, depth, &sv102->ver_minor))
		return False;
	if(!prs_uint32("srv_type    ", ps, depth, &sv102->srv_type))
		return False;
	if(!prs_uint32("ptr_comment ", ps, depth, &sv102->ptr_comment))
		return False;

	/* same as 101 up to here */

	if(!prs_uint32("users       ", ps, depth, &sv102->users))
		return False;
	if(!prs_uint32("disc        ", ps, depth, &sv102->disc))
		return False;
	if(!prs_uint32("hidden      ", ps, depth, &sv102->hidden))
		return False;
	if(!prs_uint32("announce    ", ps, depth, &sv102->announce))
		return False;
	if(!prs_uint32("ann_delta   ", ps, depth, &sv102->ann_delta))
		return False;
	if(!prs_uint32("licenses    ", ps, depth, &sv102->licenses))
		return False;
	if(!prs_uint32("ptr_usr_path", ps, depth, &sv102->ptr_usr_path))
		return False;

	if(!smb_io_unistr2("uni_name    ", &sv102->uni_name, True, ps, depth))
		return False;
	if(!prs_align(ps))
		return False;
	if(!smb_io_unistr2("uni_comment ", &sv102->uni_comment, True, ps, depth))
		return False;
	if(!prs_align(ps))
		return False;
	if(!smb_io_unistr2("uni_usr_path", &sv102->uni_usr_path, True, ps, depth))
		return False;

	return True;
}

/*******************************************************************
 Reads or writes a SRV_INFO_102 structure.
 ********************************************************************/

static int srv_io_info_ctr(char *desc, SRV_INFO_CTR *ctr, prs_struct *ps, int depth)
{
	if (ctr == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_info_ctr");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("switch_value", ps, depth, &ctr->switch_value))
		return False;
	if(!prs_uint32("ptr_srv_ctr ", ps, depth, &ctr->ptr_srv_ctr))
		return False;

	if (ctr->ptr_srv_ctr != 0 && ctr->switch_value != 0 && ctr != NULL) {
		switch (ctr->switch_value) {
		case 101:
			if(!srv_io_info_101("sv101", &ctr->srv.sv101, ps, depth))
				return False;
			break;
		case 102:
			if(!srv_io_info_102("sv102", &ctr->srv.sv102, ps, depth))
				return False;
			break;
		default:
//0514			DEBUG(5,("%s no server info at switch_value %d\n",
//0514					 tab_depth(depth), ctr->switch_value));
			break;
		}
		if(!prs_align(ps))
			return False;
	}

	return True;
}

/*******************************************************************
 Inits a SRV_Q_NET_SRV_GET_INFO structure.
 ********************************************************************/
#if 0 //0516
void init_srv_q_net_srv_get_info(SRV_Q_NET_SRV_GET_INFO *srv,
				char *server_name, uint32 switch_value)
{
//	DEBUG(5,("init_srv_q_net_srv_get_info\n"));

	init_buf_unistr2(&srv->uni_srv_name, &srv->ptr_srv_name, server_name);

	srv->switch_value = switch_value;
}
#endif
/*******************************************************************
 Reads or writes a structure.
********************************************************************/

int srv_io_q_net_srv_get_info(char *desc, SRV_Q_NET_SRV_GET_INFO *q_n, prs_struct *ps, int depth)
{
	if (q_n == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_q_net_srv_get_info");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("ptr_srv_name  ", ps, depth, &q_n->ptr_srv_name))
		return False;
	if(!smb_io_unistr2("", &q_n->uni_srv_name, True, ps, depth))
		return False;

	if(!prs_align(ps))
		return False;

	if(!prs_uint32("switch_value  ", ps, depth, &q_n->switch_value))
		return False;

	return True;
}

/*******************************************************************
 Inits a SRV_R_NET_SRV_GET_INFO structure.
 ********************************************************************/

void init_srv_r_net_srv_get_info(SRV_R_NET_SRV_GET_INFO *srv,
				uint32 switch_value, SRV_INFO_CTR *ctr, uint32 status)
{
//	DEBUG(5,("init_srv_r_net_srv_get_info\n"));

	srv->ctr = ctr;

	if (status == 0x0) {
		srv->ctr->switch_value = switch_value;
		srv->ctr->ptr_srv_ctr  = 1;
	} else {
		srv->ctr->switch_value = 0;
		srv->ctr->ptr_srv_ctr  = 0;
	}

	srv->status = status;
}

/*******************************************************************
 Reads or writes a structure.
 ********************************************************************/

int srv_io_r_net_srv_get_info(char *desc, SRV_R_NET_SRV_GET_INFO *r_n, prs_struct *ps, int depth)
{
	if (r_n == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "srv_io_r_net_srv_get_info");
	depth++;

	if(!prs_align(ps))
		return False;

	if(!srv_io_info_ctr("ctr", r_n->ctr, ps, depth))
		return False;

	if(!prs_uint32("status      ", ps, depth, &r_n->status))
		return False;

	return True;
}



