
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
#include "nterr.h"

//for pipes_struct must include
#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"

#include "rpc_srv.h"

//from smbutil.c
extern int string_sub(char *s,const char *pattern,const char *insert, uint32 len);

//from parsesrv.c
extern void init_srv_share_info1(SH_INFO_1 *sh1, char *net_name, uint32 type, char *remark);
extern void init_srv_share_info1_str(SH_INFO_1_STR *sh1, char *net_name, char *remark);
extern void init_srv_share_info2(SH_INFO_2 *sh2, char *net_name, uint32 type, char *remark,
    				  	uint32 perms, uint32 max_uses, uint32 num_uses,	char *path);
extern void init_srv_share_info2_str(SH_INFO_2_STR *sh2, char *net_name, char *remark, char *path);
extern int srv_io_r_net_share_enum(char *desc, SRV_R_NET_SHARE_ENUM *r_n, prs_struct *ps, int depth);
extern void free_srv_r_net_share_enum(SRV_R_NET_SHARE_ENUM *r_n);
extern void init_srv_info_102(SRV_INFO_102 *sv102, uint32 platform_id, char *name, char *comment, uint32 ver_major, uint32 ver_minor,
						uint32 srv_type, uint32 users, uint32 disc, uint32 hidden, 	uint32 announce, uint32 ann_delta, uint32 licenses,
						char *usr_path);\
extern void init_srv_info_101(SRV_INFO_101 *sv101, uint32 platform_id, char *name,	uint32 ver_major, uint32 ver_minor,
						uint32 srv_type, char *comment);
extern void init_srv_r_net_srv_get_info(SRV_R_NET_SRV_GET_INFO *srv, uint32 switch_value, SRV_INFO_CTR *ctr, uint32 status);						
extern int srv_io_q_net_srv_get_info(char *desc, SRV_Q_NET_SRV_GET_INFO *q_n, prs_struct *ps, int depth);
extern int srv_io_r_net_srv_get_info(char *desc, SRV_R_NET_SRV_GET_INFO *r_n, prs_struct *ps, int depth);
extern int srv_io_q_net_share_enum(char *desc, SRV_Q_NET_SHARE_ENUM *q_n, prs_struct *ps, int depth);
extern void free_srv_q_net_share_enum(SRV_Q_NET_SHARE_ENUM *q_n);

//from parsemis.c
extern void init_enum_hnd(ENUM_HND *enh, uint32 hnd);
extern uint32 get_enum_hnd(ENUM_HND *enh);

//from spipe.c
extern int api_rpcTNP(pipes_struct *p, char *rpc_name, struct api_struct *api_rpc_cmds,	prs_struct *rpc_in);

//0514extern int DEBUGLEVEL;
//extern pstring global_myname;

/*******************************************************************
 Fill in a share info level 1 structure.
 ********************************************************************/

static void init_srv_share_info_1(SRV_SHARE_INFO_1 *sh1, int snum)
{
	int len_net_name;
	pstring net_name;
	pstring remark;
	uint32 type;
	// char *lp_service[] ={"IPC$", &Smbprinterserver[0], &Smbprinterserver[1], &Smbprinterserver[2]};
	char *lp_command[4]; // ={"IPC SERVICE", "Printer","Printer","Printer"};      
	char *lp_service[4]; //={"IPC$", &Smbprinterserver[0], &Smbprinterserver[1], &Smbprinterserver[2]};    
	//char *lp_command[] ={"IPC SERVICE", "Printer","Printer","Printer"};  

//	pstrcpy(net_name, Smbprinterserver[snum]);
//	pstrcpy(remark, "printer");

	lp_command[0] = "IPC SERVICE";
	lp_command[1] = "Printer";
	lp_command[2] = "Printer";
	lp_command[3] = "Printer";

    lp_service[0] = "IPC$";
    lp_service[1] = Smbprinterserver[0];
    lp_service[2] = Smbprinterserver[1];
    lp_service[3] = Smbprinterserver[2];        

	pstrcpy(net_name, lp_service[snum]);
	pstrcpy(remark, lp_command[snum]);

//0604	pstring_sub(remark,"%S",lp_servicename(snum));
	string_sub(remark,"%S", lp_service[snum], sizeof(pstring));
	len_net_name = strlen(net_name);

	/* work out the share type */
	type = STYPE_DISKTREE;
		
//0604	if (lp_print_ok(snum))
	if (1)
		type = STYPE_PRINTQ;
	if (!strcmp("IPC$", net_name))
		type = STYPE_IPC;
	if (net_name[len_net_name] == '$')
		type |= STYPE_HIDDEN;

	init_srv_share_info1(&sh1->info_1, net_name, type, remark);
	init_srv_share_info1_str(&sh1->info_1_str, net_name, remark);
}

/*******************************************************************
 Fill in a share info level 2 structure.
 ********************************************************************/

static void init_srv_share_info_2(SRV_SHARE_INFO_2 *sh2, int snum)
{
	int len_net_name;
	pstring net_name;
	pstring remark;
	pstring path;
	pstring passwd;
	uint32 type;

	pstrcpy(net_name, _MyServerName);
//0604	pstrcpy(remark, lp_comment(snum));
	string_sub(remark,"%S",_MyServerName,sizeof(pstring));
//0604	pstrcpy(path, lp_pathname(snum));
	pstrcpy(passwd, "");
	len_net_name = strlen(net_name);

	/* work out the share type */
	type = STYPE_DISKTREE;
		
//0529	if (lp_print_ok(snum))
	if (1)
		type = STYPE_PRINTQ;
	if (!strcmp("IPC$", net_name))
		type = STYPE_IPC;
	if (net_name[len_net_name] == '$')
		type |= STYPE_HIDDEN;

	init_srv_share_info2(&sh2->info_2, net_name, type, remark, 0, 0xffffffff, 1, path);
	init_srv_share_info2_str(&sh2->info_2_str, net_name, remark, path);
}

/*******************************************************************
 Fill in a share info structure.
 ********************************************************************/

static int init_srv_share_info_ctr(SRV_SHARE_INFO_CTR *ctr,
	       uint32 info_level, uint32 *resume_hnd, uint32 *total_entries)
{
//0608	int num_entries = 0;
//0603	int num_services = lp_numservices();
//0608	int snum;
	int num_entries = NUM_OF_PRN_PORT +1; //the first one service is IPC$
	int num_services = NUM_OF_PRN_PORT + 3;
	int snum = 0;

//	DEBUG(5,("init_srv_share_info_ctr\n"));

//2/8/2002	ZERO_STRUCTPN(ctr);
	memset((char *)ctr, 0 , sizeof(SRV_SHARE_INFO_CTR));

	ctr->info_level = ctr->switch_value = info_level;
	*resume_hnd = 0;

	/* Count the number of entries. */
//	for (snum = 0; snum < num_services; snum++) {
//0604		if (lp_browseable(snum) && lp_snum_ok(snum))
//		if (1)
//			num_entries++;
//	}

	*total_entries = num_entries;
	ctr->num_entries2 = ctr->num_entries = num_entries;
	ctr->ptr_share_info = ctr->ptr_entries = 1;

//0608	if (!num_entries)
//		return True;

	switch (info_level) {
	case 1:
	{
		SRV_SHARE_INFO_1 *info1;
		int i = 0;

		info1 = mallocw(num_entries * sizeof(SRV_SHARE_INFO_1));
#if 0 //0608
		for (snum = *resume_hnd; snum < num_services; snum++) {
			if (lp_browseable(snum) && lp_snum_ok(snum)) {
				init_srv_share_info_1(&info1[i++], snum);
			}
		}
#endif
		for (snum = *resume_hnd; snum < num_entries; snum++) {
				init_srv_share_info_1(&info1[i++], snum);
			}
		ctr->share.info1 = info1;
		break;
	}

	case 2:
	{
		SRV_SHARE_INFO_2 *info2;
		int i = 0;

		info2 = mallocw(num_entries * sizeof(SRV_SHARE_INFO_2));

		for (snum = *resume_hnd; snum < num_services; snum++) {
//0604			if (lp_browseable(snum) && lp_snum_ok(snum)) {
			if (1) {
				init_srv_share_info_2(&info2[i++], snum);
			}
		}

		ctr->share.info2 = info2;
		break;
	}

	default:
//		DEBUG(5,("init_srv_share_info_ctr: unsupported switch value %d\n", info_level));
		return False;
	}

	return True;
}

/*******************************************************************
 Inits a SRV_R_NET_SHARE_ENUM structure.
********************************************************************/

static void init_srv_r_net_share_enum(SRV_R_NET_SHARE_ENUM *r_n,
				      uint32 info_level, uint32 resume_hnd)  
{
//	DEBUG(5,("init_srv_r_net_share_enum: %d\n", __LINE__));

	if (init_srv_share_info_ctr(&r_n->ctr, info_level,
				    &resume_hnd, &r_n->total_entries)) {
		r_n->status = 0x0;
	} else {
		r_n->status = 0xC0000000 | NT_STATUS_INVALID_INFO_CLASS;
	}

	init_enum_hnd(&r_n->enum_hnd, resume_hnd);
}

/*******************************************************************
 Net share enum.
********************************************************************/

static int srv_reply_net_share_enum(SRV_Q_NET_SHARE_ENUM *q_n,
				prs_struct *rdata)
{
	SRV_R_NET_SHARE_ENUM r_n;
	int ret;

//	DEBUG(5,("srv_net_share_enum: %d\n", __LINE__));

	/* Create the list of shares for the response. */
	init_srv_r_net_share_enum(&r_n,
				q_n->ctr.info_level,
				get_enum_hnd(&q_n->enum_hnd));

	/* store the response in the SMB stream */
	ret = srv_io_r_net_share_enum("", &r_n, rdata, 0);

	/* Free the memory used by the response. */
	free_srv_r_net_share_enum(&r_n);

//	DEBUG(5,("srv_net_share_enum: %d\n", __LINE__));

	return ret;
}


/*******************************************************************
net server get info
********************************************************************/

static void srv_reply_net_srv_get_info(SRV_Q_NET_SRV_GET_INFO *q_n,
				prs_struct *rdata)
{
	SRV_R_NET_SRV_GET_INFO r_n;
	uint32 status = 0x0;
	SRV_INFO_CTR ctr;


//	DEBUG(5,("srv_net_srv_get_info: %d\n", __LINE__));

	switch (q_n->switch_value)
	{
		case 102:
		{
//0529			init_srv_info_102(&ctr.srv.sv102,
//					  500, global_myname, 
//					  string_truncate(lp_serverstring(), MAX_SERVER_STRING_LENGTH),
//					  lp_major_announce_version(), lp_minor_announce_version(),
//					  lp_default_server_announce(),
//					  0xffffffff, /* users */
//					  0xf, /* disc */
//					  0, /* hidden */
//					  240, /* announce */
//					  3000, /* announce delta */
//					  100000, /* licenses */
//					  "c:\\"); /* user path */

			init_srv_info_102(&ctr.srv.sv102,
					  500, _MyNetBIOSName, 
					  "PrinterServer",   //command
					  0x04, 0x02,
					  0x39427,          //srv_type
					  0xffffffff, /* users */
					  0xf, /* disc */
					  0, /* hidden */
					  240, /* announce */
					  3000, /* announce delta */
					  100000, /* licenses */
					  "c:\\"); /* user path */

			break;
		}
		case 101:
		{
//0529			init_srv_info_101(&ctr.srv.sv101,
//					  500, global_myname,
//					  lp_major_announce_version(), lp_minor_announce_version(),
//					  lp_default_server_announce(),
//				    string_truncate(lp_serverstring(), MAX_SERVER_STRING_LENGTH));

			init_srv_info_101(&ctr.srv.sv101,
					  500, _MyNetBIOSName,
					  0x04, 0x02,
//Jesse					  0x39427,               //srv_type
					  0x9a03,               //srv_type
					  "PrinterServer");      //command
					  

			break;
		}
		default:                        
		{
//0603			status = 0xC0000000 | NT_STATUS_INVALID_INFO_CLASS;
			status = 0xC0000000 | 3; //nterr.h

			break;
		}
	}

	/* set up the net server get info structure */
	init_srv_r_net_srv_get_info(&r_n, q_n->switch_value, &ctr, status);

	/* store the response in the SMB stream */
	srv_io_r_net_srv_get_info("", &r_n, rdata, 0);

//	DEBUG(5,("srv_net_srv_get_info: %d\n", __LINE__));
}

/*******************************************************************
********************************************************************/
static int api_srv_net_srv_get_info( uint16 vuid, prs_struct *data,
				    prs_struct *rdata )
{
	SRV_Q_NET_SRV_GET_INFO q_n;

	/* grab the net server get info */
	srv_io_q_net_srv_get_info("", &q_n, data, 0);

	/* construct reply.  always indicate success */
	srv_reply_net_srv_get_info(&q_n, rdata);

	return True;
}

/*******************************************************************
 RPC to enumerate shares.
********************************************************************/

static int api_srv_net_share_enum( uint16 vuid, prs_struct *data,
				    prs_struct *rdata )
{
	SRV_Q_NET_SHARE_ENUM q_n;
	int ret;

	/* Unmarshall the net server get enum. */
	if(!srv_io_q_net_share_enum("", &q_n, data, 0)) {
//		DEBUG(0,("api_srv_net_share_enum: Failed to unmarshall SRV_Q_NET_SHARE_ENUM.\n"));
		return False;
	}

	ret = srv_reply_net_share_enum(&q_n, rdata);

	/* Free any data allocated in the unmarshalling. */
	free_srv_q_net_share_enum(&q_n);

	return ret;
}

/*******************************************************************
\PIPE\srvsvc commands
********************************************************************/
struct api_struct api_srv_cmds[] =
{
//0514	{ "SRV_NETCONNENUM"       , SRV_NETCONNENUM       , api_srv_net_conn_enum    },
//0514	{ "SRV_NETSESSENUM"       , SRV_NETSESSENUM       , api_srv_net_sess_enum    },
	{ "SRV_NETSHAREENUM"      , SRV_NETSHAREENUM      , api_srv_net_share_enum   },
//0514	{ "SRV_NET_SHARE_GET_INFO", SRV_NET_SHARE_GET_INFO, api_srv_net_share_get_info },
//0514	{ "SRV_NETFILEENUM"       , SRV_NETFILEENUM       , api_srv_net_file_enum    },
	{ "SRV_NET_SRV_GET_INFO"  , SRV_NET_SRV_GET_INFO  , api_srv_net_srv_get_info },
//0514	{ "SRV_NET_REMOTE_TOD"    , SRV_NET_REMOTE_TOD    , api_srv_net_remote_tod   },
	{ NULL                    , 0                     , NULL                     }
};

/*******************************************************************
receives a srvsvc pipe and responds.
********************************************************************/
int api_srvsvc_rpc(pipes_struct *p, prs_struct *data)
{
	return api_rpcTNP(p, "api_srvsvc_rpc", api_srv_cmds, data);
}
