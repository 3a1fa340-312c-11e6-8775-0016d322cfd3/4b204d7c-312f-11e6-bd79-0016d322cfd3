/*
 * Copyright (c) 1990,1991 Regents of The University of Michigan.
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation, and that the name of The University
 * of Michigan not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. This software is supplied as is without expressed or
 * implied warranties of any kind.
 *
 *	Research Systems Unix Group
 *	The University of Michigan
 *	c/o Mike Clark
 *	535 W. William Street
 *	Ann Arbor, Michigan
 *	+1-313-763-0525
 *	netatalk@itd.umich.edu
 */
 
#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "socket2.h"
#include "atalkd.h"
#include "atp.h"

ATP atp_open(uint8 port)
{
	int			s;
	ATP			atp;
//	int			nlen;

#ifdef EBUG
	printf( "<%d> atp_open\n", getpid());
#endif

	if (( s = socket2( AF_APPLETALK, SOCK_DGRAM, 0 )) < 0 ) {
		return 0;
	}

//	if (( atp = (ATP) alloc_atpbuf()) == NULL ) {
//		return 0;
//	}

	if (( atp = (ATP) malloc(sizeof(struct atp_handle)) ) == NULL ) {
		return 0;
	}

	// initialize the atp handle
	memset(atp,'\0',sizeof( struct atp_handle ));
	atp->atph_saddr.sat_family = AF_APPLETALK;
	atp->atph_saddr.sat_addr.s_net = ATADDR_ANYNET;
	atp->atph_saddr.sat_addr.s_node = ATADDR_ANYNODE;
	atp->atph_saddr.sat_port = ( port == 0 ? GetATport() : port );

	if ( bind2( s, (struct sockaddr *) &atp->atph_saddr,
	     sizeof( struct sockaddr_at )) != 0 )
	{
		free_atpbuf( (struct atpbuf *)atp );
		close_s2(s);	//615wu
		return NULL;
	}

	// get the real address from the kernel
//	nlen = sizeof( struct sockaddr_at);
//	if ( getsockname( s, (struct sockaddr *) &atp->atph_saddr, &nlen ) != 0 ) {
//		return NULL;
//	}
	atp->atph_socket = s;
	atp->atph_reqto = -1;
	atp->atph_tid = msclock();

#ifdef EBUG
	srandom( tv.tv_sec );
#endif

	return atp;
}
