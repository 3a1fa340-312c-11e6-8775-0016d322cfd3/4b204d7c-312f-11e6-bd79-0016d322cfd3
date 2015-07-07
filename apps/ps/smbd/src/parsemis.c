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

void init_unistr2(UNISTR2 *str, char *buf, int len);

//from parseprs.c
int prs_uint8s(int charmode, char *name, prs_struct *ps, int depth, uint8 *data8s, int len);
extern int prs_uint16(char *name, prs_struct *ps, int depth, uint16 *data16);
extern int prs_uint32(char *name, prs_struct *ps, int depth, uint32 *data32);
extern int prs_align(prs_struct *ps);
extern int prs_buffer2(int charmode, char *name, prs_struct *ps, int depth, BUFFER2 *str);
extern int prs_unistr2(int charmode, char *name, prs_struct *ps, int depth, UNISTR2 *str);

//from unistr.c
extern uint32 dos_struni2(char *dst, const char *src, uint32 max_len);

/*******************************************************************
 Reads or writes an NTTIME structure.
********************************************************************/

int smb_io_time(char *desc, NTTIME *nttime, prs_struct *ps, int depth)
{
	if (nttime == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "smb_io_time");
	depth++;

	if(!prs_align(ps))
		return False;
	
	if(!prs_uint32("low ", ps, depth, &nttime->low)) /* low part */
		return False;
	if(!prs_uint32("high", ps, depth, &nttime->high)) /* high part */
		return False;

	return True;
}

/*******************************************************************
 Gets an enumeration handle from an ENUM_HND structure.
********************************************************************/

uint32 get_enum_hnd(ENUM_HND *enh)
{
	return (enh && enh->ptr_hnd != 0) ? enh->handle : 0;
}

/*******************************************************************
 Inits an ENUM_HND structure.
********************************************************************/

void init_enum_hnd(ENUM_HND *enh, uint32 hnd)
{
//0509	DEBUG(5,("smb_io_enum_hnd\n"));

	enh->ptr_hnd = (hnd != 0) ? 1 : 0;
	enh->handle = hnd;
}

/*******************************************************************
 Reads or writes an ENUM_HND structure.
********************************************************************/

int smb_io_enum_hnd(char *desc, ENUM_HND *hnd, prs_struct *ps, int depth)
{
	if (hnd == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "smb_io_enum_hnd");
	depth++;

	if(!prs_align(ps))
		return False;
	
	if(!prs_uint32("ptr_hnd", ps, depth, &hnd->ptr_hnd)) /* pointer */
		return False;

	if (hnd->ptr_hnd != 0) {
		if(!prs_uint32("handle ", ps, depth, &hnd->handle )) /* enum handle */
			return False;
	}

	return True;
}

/*******************************************************************
creates a STRHDR structure.
********************************************************************/

void init_str_hdr(STRHDR *hdr, int max_len, int len, uint32 buffer)
{
	hdr->str_max_len = max_len;
	hdr->str_str_len = len;
	hdr->buffer      = buffer;
}

/*******************************************************************
 Reads or writes a STRHDR structure.
********************************************************************/

int smb_io_strhdr(char *desc,  STRHDR *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "smb_io_strhdr");
	depth++;

	prs_align(ps);
	
	if(!prs_uint16("str_str_len", ps, depth, &hdr->str_str_len))
		return False;
	if(!prs_uint16("str_max_len", ps, depth, &hdr->str_max_len))
		return False;
	if(!prs_uint32("buffer     ", ps, depth, &hdr->buffer))
		return False;

	/* oops! XXXX maybe issue a warning that this is happening... */
	if (hdr->str_max_len > MAX_STRINGLEN)
		hdr->str_max_len = MAX_STRINGLEN;
	if (hdr->str_str_len > MAX_STRINGLEN)
		hdr->str_str_len = MAX_STRINGLEN;

	return True;
}

/*******************************************************************
 Inits a UNIHDR structure.
********************************************************************/

void init_uni_hdr(UNIHDR *hdr, int len)
{
	hdr->uni_str_len = 2 * len;
	hdr->uni_max_len = 2 * len;
	hdr->buffer      = len != 0 ? 1 : 0;
}

/*******************************************************************
 Reads or writes a UNIHDR structure.
********************************************************************/

int smb_io_unihdr(char *desc, UNIHDR *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "smb_io_unihdr");
	depth++;

	if(!prs_align(ps))
		return False;
	
	if(!prs_uint16("uni_str_len", ps, depth, &hdr->uni_str_len))
		return False;
	if(!prs_uint16("uni_max_len", ps, depth, &hdr->uni_max_len))
		return False;
	if(!prs_uint32("buffer     ", ps, depth, &hdr->buffer))
		return False;

	/* oops! XXXX maybe issue a warning that this is happening... */
	if (hdr->uni_max_len > MAX_UNISTRLEN)
		hdr->uni_max_len = MAX_UNISTRLEN;
	if (hdr->uni_str_len > MAX_UNISTRLEN)
		hdr->uni_str_len = MAX_UNISTRLEN;

	return True;
}

/*******************************************************************
 Inits a BUFHDR structure.
********************************************************************/

void init_buf_hdr(BUFHDR *hdr, int max_len, int len)
{
	hdr->buf_max_len = max_len;
	hdr->buf_len     = len;
}


/*******************************************************************
 Inits a BUFFER2 structure.
********************************************************************/

void init_buffer2(BUFFER2 *str, uint8 *buf, int len)
{
//2/8/2002	ZERO_STRUCTP(str);
	memset((char *)str ,0, sizeof(BUFFER2));	

	/* max buffer size (allocated size) */
	str->buf_max_len = len;
	str->undoc       = 0;
	str->buf_len = buf != NULL ? len : 0;

	if (buf != NULL)
		memcpy(str->buffer, buf, MIN(str->buf_len, sizeof(str->buffer)));
}

/*******************************************************************
 Reads or writes a BUFFER2 structure.
   the uni_max_len member tells you how large the buffer is.
   the uni_str_len member tells you how much of the buffer is really used.
********************************************************************/

int smb_io_buffer2(char *desc, BUFFER2 *buf2, uint32 buffer, prs_struct *ps, int depth)
{
	if (buf2 == NULL)
		return False;

	if (buffer) {

//0607		prs_debug(ps, depth, desc, "smb_io_buffer2");
		depth++;

		if(!prs_align(ps))
			return False;
		
		if(!prs_uint32("uni_max_len", ps, depth, &buf2->buf_max_len))
			return False;
		if(!prs_uint32("undoc      ", ps, depth, &buf2->undoc))
			return False;
		if(!prs_uint32("buf_len    ", ps, depth, &buf2->buf_len))
			return False;

		/* oops! XXXX maybe issue a warning that this is happening... */
		if (buf2->buf_max_len > MAX_UNISTRLEN)
			buf2->buf_max_len = MAX_UNISTRLEN;
		if (buf2->buf_len > MAX_UNISTRLEN)
			buf2->buf_len = MAX_UNISTRLEN;

		/* buffer advanced by indicated length of string
		   NOT by searching for null-termination */

		if(!prs_buffer2(True, "buffer     ", ps, depth, buf2))
			return False;

	} else {

//0607		prs_debug(ps, depth, desc, "smb_io_buffer2 - NULL");
		depth++;
		memset((char *)buf2, '\0', sizeof(BUFFER2));

	}
	return True;
}

/*******************************************************************
creates a UNISTR2 structure: sets up the buffer, too
********************************************************************/

void init_buf_unistr2(UNISTR2 *str, uint32 *ptr, char *buf)
{
	if (buf != NULL) {

		*ptr = 1;
		init_unistr2(str, buf, strlen(buf)+1);

	} else {

		*ptr = 0;
		init_unistr2(str, "", 0);

	}
}

/*******************************************************************
 Inits a UNISTR2 structure.
********************************************************************/

void init_unistr2(UNISTR2 *str, char *buf, int len)
{
//2/8/2002	ZERO_STRUCTP(str);
	memset((char *)str ,0, sizeof(BUFFER2));	
	/* set up string lengths. */
	str->uni_max_len = len;
	str->undoc       = 0;
	str->uni_str_len = len;

	/* store the string (null-terminated 8 bit chars into 16 bit chars) */
	dos_struni2((char *)str->buffer, buf, sizeof(str->buffer));
}

/*******************************************************************
 Reads or writes a UNISTR2 structure.
 XXXX NOTE: UNISTR2 structures need NOT be null-terminated.
   the uni_str_len member tells you how long the string is;
   the uni_max_len member tells you how large the buffer is.
********************************************************************/

int smb_io_unistr2(char *desc, UNISTR2 *uni2, uint32 buffer, prs_struct *ps, int depth)
{
	if (uni2 == NULL)
		return False;

	if (buffer) {

//0607		prs_debug(ps, depth, desc, "smb_io_unistr2");
		depth++;

		if(!prs_align(ps))
			return False;
		
		if(!prs_uint32("uni_max_len", ps, depth, &uni2->uni_max_len))
			return False;
		if(!prs_uint32("undoc      ", ps, depth, &uni2->undoc))
			return False;
		if(!prs_uint32("uni_str_len", ps, depth, &uni2->uni_str_len))
			return False;

		/* oops! XXXX maybe issue a warning that this is happening... */
		if (uni2->uni_max_len > MAX_UNISTRLEN)
			uni2->uni_max_len = MAX_UNISTRLEN;
		if (uni2->uni_str_len > MAX_UNISTRLEN)
			uni2->uni_str_len = MAX_UNISTRLEN;

		/* buffer advanced by indicated length of string
		   NOT by searching for null-termination */
		if(!prs_unistr2(True, "buffer     ", ps, depth, uni2))
			return False;

	} else {

//0607		prs_debug(ps, depth, desc, "smb_io_unistr2 - NULL");
		depth++;
		memset((char *)uni2, '\0', sizeof(UNISTR2));

	}

	return True;
}

/*******************************************************************
 Reads or writes an POLICY_HND structure.
********************************************************************/

int smb_io_pol_hnd(char *desc, POLICY_HND *pol, prs_struct *ps, int depth)
{
	if (pol == NULL)
		return False;

//0607	prs_debug(ps, depth, desc, "smb_io_pol_hnd");
	depth++;

	if(!prs_align(ps))
		return False;
	
	if(!prs_uint8s (False, "data", ps, depth, pol->data, POL_HND_SIZE))
		return False;

	return True;
}


