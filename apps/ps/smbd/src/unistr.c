/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Samba utility functions
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

#include "rpc_misc.h"

#include "nameserv.h"

#include "btorder.h"


//0509 int global_is_multibyte_codepage;
//0509 int (*_skip_multibyte_char)(char c);
//0509extern int DEBUGLEVEL;

/*
 * The following are the codepage to ucs2 and vica versa maps.
 * These are dynamically loaded from a unicode translation file.
 */

//0705static smb_ucs2_t *doscp_to_ucs2;
static uint16 *ucs2_to_doscp;

//0704static smb_ucs2_t *unixcp_to_ucs2;
//0704static uint16 *ucs2_to_unixcp;

#ifndef MAXUNI
#define MAXUNI 1024
#endif

/*******************************************************************
 Return a DOS codepage version of a little-endian unicode string.
 len is the filename length (ignoring any terminating zero) in uin16
 units. Always null terminates.
 Hack alert: uses fixed buffer(s).
********************************************************************/

char *dos_unistrn2(uint16 *src, int len)
{
//0504  static char lbufs[8][MAXUNI];
//0504	static char lbufs[8][128];
	static char lbufs[8][128];
	static int nexti;
	char *lbuf = lbufs[nexti];
	char *p;

	nexti = (nexti+1)%8;

//0504	for (p = lbuf; (len > 0) && (p-lbuf < MAXUNI-3) && *src; len--, src++) {
	for (p = lbuf; (len > 0) && (p-lbuf < 128-3) && *src; len--, src++) {
		uint16 ucs2_val = SVAL(src,0);
		uint16 cp_val = ucs2_to_doscp[ucs2_val];

		if (cp_val < 256)
			*p++ = (char)cp_val;
		else {
			*p++ = (cp_val >> 8) & 0xff;
			*p++ = (cp_val & 0xff);
		}
	}

	*p = 0;
	return lbuf;
}


//0608static char lbufs[8][MAXUNI];
static char lbufs[8][128];
static int nexti;

/*******************************************************************
 Return a DOS codepage version of a little-endian unicode string.
 Hack alert: uses fixed buffer(s).
********************************************************************/

char *dos_unistr2(uint16 *src)
{
	char *lbuf = lbufs[nexti];
	char *p;

	nexti = (nexti+1)%8;

//0705	for (p = lbuf; *src && (p-lbuf < MAXUNI-3); src++) {
	for (p = lbuf; *src && (p-lbuf < 128-3); src++) {

		uint16 ucs2_val = SVAL(src,0);
		uint16 cp_val = ucs2_to_doscp[ucs2_val];

		if (cp_val < 256)
			*p++ = (char)cp_val;
		else {
			*p++ = (cp_val >> 8) & 0xff;
			*p++ = (cp_val & 0xff);
		}
	}

	*p = 0;
	return lbuf;
}

/*******************************************************************
Return a DOS codepage version of a little-endian unicode string
********************************************************************/

char *dos_unistr2_to_str(UNISTR2 *str)
{
	char *lbuf = lbufs[nexti];
	char *p;
	uint16 *src = str->buffer;
	int max_size = MIN(sizeof(str->buffer)-3, str->uni_str_len);

	nexti = (nexti+1)%8;

	for (p = lbuf; *src && p-lbuf < max_size; src++) {
		uint16 ucs2_val = SVAL(src,0);
		uint16 cp_val = ucs2_to_doscp[ucs2_val];

		if (cp_val < 256)
			*p++ = (char)cp_val;
		else {
			*p++ = (cp_val >> 8) & 0xff;
			*p++ = (cp_val & 0xff);
		}
	}

	*p = 0;
	return lbuf;
}

/*******************************************************************
 Create a null-terminated unicode string from a null-terminated DOS
 codepage string.
 Return number of unicode chars copied, excluding the null character.
 Unicode strings created are in little-endian format.
********************************************************************/

uint32 dos_struni2(char *dst, const char *src, uint32 max_len)
{
	uint32 len = 0;

	if (dst == NULL)
		return 0;

	if (src != NULL) {
		for (; *src && len < max_len-2; len++, dst +=2) {
//			uint32 skip = get_character_len(*src);
			smb_ucs2_t val = (*src & 0xff);

			/*
			 * If this is a multibyte character (and all DOS/Windows
			 * codepages have at maximum 2 byte multibyte characters)
			 * then work out the index value for the unicode conversion.
			 */

//			if (skip == 2)
//				val = ((val << 8) | (src[1] & 0xff));

//0514			SSVAL(dst,0,doscp_to_ucs2[val]);
			SSVAL(dst,0,val);
//			if (skip)
//				src += skip;
//			else
				src++;
		}
	}

	SSVAL(dst,0,0);

	return len;
}

/*******************************************************************
 Count the number of characters in a smb_ucs2_t string.
********************************************************************/

uint32 wstrlen(const smb_ucs2_t *src)
{
  uint32 len;

  for(len = 0; *src; len++)
    ;

  return len;
}

/*******************************************************************
 Compare the first n characters of s1 to s2. len is in ucs2 units.
********************************************************************/

int wstrncmp(const smb_ucs2_t *s1, const smb_ucs2_t *s2, uint32 len)
{
	smb_ucs2_t c1, c2;

	for (; len != 0; --len) {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 != c2)
			return c1 - c2;

		if (c1 == 0)
			return 0;

    }
	return 0;
}

/*******************************************************************
 Search for ucs2 char c from the beginning of s.
********************************************************************/ 

smb_ucs2_t *wstrchr(const smb_ucs2_t *s, smb_ucs2_t c)
{
	do {
		if (*s == c)
			return (smb_ucs2_t *)s;
	} while (*s++);

	return NULL;
}

/*******************************************************************
skip past some strings in a buffer
********************************************************************/
char *skip_string(char *buf,uint32 n)
{
  while (n--)
    buf += strlen(buf) + 1;
  return(buf);
}

