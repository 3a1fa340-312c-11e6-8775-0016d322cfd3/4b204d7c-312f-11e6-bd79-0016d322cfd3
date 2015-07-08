/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-1997,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1997,
 *  Copyright (C) Paul Ashton                       1997.
 *  Copyright (C) Hewlett-Packard Company           1999.
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
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "smbinc.h"
#include "smb.h"

#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"

//for struct REG_Q_INFO must include
#include "rpc_sec.h"
#include "rpc_reg.h"

//from parseprs.c
extern int prs_uint8(char *name, prs_struct *ps, int depth, uint8 *data8);
extern int prs_uint8s(int charmode, char *name, prs_struct *ps, int depth, uint8 *data8s, int len);
extern int prs_uint32(char *name, prs_struct *ps, int depth, uint32 *data32);
extern int prs_align(prs_struct *ps);

//from parsemis.c
extern int smb_io_pol_hnd(char *desc, POLICY_HND *pol, prs_struct *ps, int depth);
extern int smb_io_unihdr(char *desc, UNIHDR *hdr, prs_struct *ps, int depth);
extern int smb_io_unistr2(char *desc, UNISTR2 *uni2, uint32 buffer, prs_struct *ps, int depth);
extern int smb_io_time(char *desc, NTTIME *nttime, prs_struct *ps, int depth);
extern void init_buffer2(BUFFER2 *str, uint8 *buf, int len);
extern int smb_io_buffer2(char *desc, BUFFER2 *buf2, uint32 buffer, prs_struct *ps, int depth);

//from unistr.c
extern uint32 dos_struni2(char *dst, const char *src, uint32 max_len);

/*******************************************************************
reads or writes a structure.
********************************************************************/

int reg_io_q_info(char *desc,  REG_Q_INFO *r_q, prs_struct *ps, int depth)
{
	if (r_q == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "reg_io_q_info");
	depth++;

	if(!prs_align(ps))
		return False;
	
	if(!smb_io_pol_hnd("", &r_q->pol, ps, depth))
		return False;
	if(!smb_io_unihdr ("", &r_q->hdr_type, ps, depth))
		return False;
	if(!smb_io_unistr2("", &r_q->uni_type, r_q->hdr_type.buffer, ps, depth))
		return False;

	if(!prs_align(ps))
		return False;
	
	if(!prs_uint32("ptr1", ps, depth, &r_q->ptr1))
		return False;

	if (r_q->ptr1 != 0) {
		if(!smb_io_time("", &r_q->time, ps, depth))
			return False;
		if(!prs_uint8 ("major_version1", ps, depth, &r_q->major_version1))
			return False;
		if(!prs_uint8 ("minor_version1", ps, depth, &r_q->minor_version1))
			return False;
		if(!prs_uint8s(False, "pad1", ps, depth, r_q->pad1, sizeof(r_q->pad1)))
			return False;
	}

	if(!prs_uint32("ptr2", ps, depth, &r_q->ptr2))
		return False;

	if (r_q->ptr2 != 0) {
		if(!prs_uint8 ("major_version2", ps, depth, &r_q->major_version2))
			return False;
		if(!prs_uint8 ("minor_version2", ps, depth, &r_q->minor_version2))
			return False;
		if(!prs_uint8s(False, "pad2", ps, depth, r_q->pad2, sizeof(r_q->pad2)))
			return False;
	}

	if(!prs_uint32("ptr3", ps, depth, &r_q->ptr3))
		return False;

	if (r_q->ptr3 != 0) {
		if(!prs_uint32("unknown", ps, depth, &r_q->unknown))
			return False;
	}

	return True;
}

/*******************************************************************
 Inits a structure.
********************************************************************/

void init_reg_r_info(REG_R_INFO *r_r,
				uint32 level, char *os_type,
				uint32 unknown_0, uint32 unknown_1,
				uint32 status)
{
	uint8 buf[512];
	int len = dos_struni2((char *)buf, os_type, sizeof(buf));

	r_r->ptr1 = 1;
	r_r->level = level;

	r_r->ptr_type = 1;
	init_buffer2(&r_r->uni_type, buf, len*2);

	r_r->ptr2 = 1;
	r_r->unknown_0 = unknown_0;

	r_r->ptr3 = 1;
	r_r->unknown_1 = unknown_1;

	r_r->status = status;
}

/*******************************************************************
reads or writes a structure.
********************************************************************/

int reg_io_r_info(char *desc, REG_R_INFO *r_r, prs_struct *ps, int depth)
{
	if (r_r == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "reg_io_r_info");
	depth++;

	if(!prs_align(ps))
		return False;
	
	if(!prs_uint32("ptr1", ps, depth, &r_r->ptr1))
		return False;

	if (r_r->ptr1 != 0) {
		if(!prs_uint32("level", ps, depth, &r_r->level))
			return False;
		if(!prs_uint32("ptr_type", ps, depth, &r_r->ptr_type))
			return False;

		if(!smb_io_buffer2("uni_type", &r_r->uni_type, r_r->ptr_type, ps, depth))
			return False;
		if(!prs_align(ps))
			return False;

		if(!prs_uint32("ptr2", ps, depth, &r_r->ptr2))
			return False;

		if (r_r->ptr2 != 0) {
			if(!prs_uint32("unknown_0", ps, depth, &r_r->unknown_0))
				return False;
		}

		if(!prs_uint32("ptr3", ps, depth, &r_r->ptr3))
			return False;

		if (r_r->ptr3 != 0) {
			if(!prs_uint32("unknown_1", ps, depth, &r_r->unknown_1))
				return False;
		}

	}
	if(!prs_uint32("status", ps, depth, &r_r->status))
		return False;

	return True;
}


