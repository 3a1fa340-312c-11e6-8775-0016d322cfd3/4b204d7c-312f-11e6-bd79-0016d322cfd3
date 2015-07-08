
/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-1997,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1997,
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
#include "smb.h"

#include "rpc_misc.h"

//0606extern int DEBUGLEVEL;

#ifndef MAX_OPEN_POLS
#define MAX_OPEN_POLS 64
#endif


/****************************************************************************
  find policy index by handle
****************************************************************************/
int find_lsa_policy_by_hnd(POLICY_HND *hnd)
{
//0705	struct policy *p = find_lsa_policy(hnd);

//0705	return p?p->pnum:-1;
	return -1;
}


