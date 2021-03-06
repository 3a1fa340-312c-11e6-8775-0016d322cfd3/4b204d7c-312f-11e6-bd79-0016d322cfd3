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
#include "atalkd.h"
#include "atp.h"
#ifdef EBUG
#include <stdio.h>
#endif

int atp_rresp(
	ATP              ah,   // open atp handle
	struct atp_block *atpb // parameter block
){
	int		len, i, rc;

#ifdef EBUG
	print_bufuse( ah, "atp_rresp" );
#endif

	//* check parameters
	if ( atpb->atp_rresiovcnt <= 0 || atpb->atp_rresiovcnt > 8 ) {
		return( -1 );
	}

	while (( rc = atp_rsel( ah, atpb->atp_saddr, ATP_TRESP )) == 0 ) {
		;
	}

	if ( rc != ATP_TRESP ) {
		return( rc );
	}

	for ( i = 0; i < 8; ++i ) {
		if ( ah->atph_resppkt[ i ] == NULL ) {
			break;
		}
		len = ah->atph_resppkt[ i ]->atpbuf_dlen - ATP_HDRSIZE;
		if (   i > atpb->atp_rresiovcnt - 1 ||
		     len > atpb->atp_rresiov[ i ].iov_len )
		{
			return( -1 );
		}
#ifdef EBUG
		fprintf( stderr, "atp_rresp copying %d bytes packet %d\n", len, i );
		bprint( (char *)ah->atph_resppkt[ i ]->atpbuf_info.atpbuf_data,len + ATP_HDRSIZE );
#endif
		bcopy( (char *)ah->atph_resppkt[ i ]->atpbuf_info.atpbuf_data
		       +ATP_HDRSIZE, (char *)atpb->atp_rresiov[ i ].iov_base, len );
		atpb->atp_rresiov[ i ].iov_len = len;
		free_atpbuf( ah->atph_resppkt[ i ] );
		ah->atph_resppkt[ i ] = NULL;
	}
	atpb->atp_rresiovcnt = i;

	return( 0 );
}
