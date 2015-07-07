/*
 * mib_module.c --
 *
 * This file contains the initialization of the MIB modules.
 *
 * Copyright (c) 1996-1997
 *
 * Erik Schoenfelder		TU Braunschweig, Germany
 *
 *
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of CMU not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  
 * 
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 */
#include "pstarget.h"   //615wu 
#include "psglobal.h"	//615wu
#include "asn1.h"		//615wu
#include "snmpvars.h"
#include "mibmodl.h"

/*
 * initialize and register the modules:
 */

void
init_modules ()
{
    /* standard variables: */
    snmp_vars_init ();

#ifdef HAVE_HR
    hr_init();
#endif
#ifdef HAVE_IDENT
    ident_init();
#endif
#ifdef HAVE_LINUX
    linux_init();
#endif

#ifdef HAVE_EXAMPLE
    examp_init();
#endif


}
