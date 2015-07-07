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
#include "atp_in.h"

int atp_close(ATP ah)
{
	struct atpbuf   *cq;
	int	            i;

	// remove from list of open atp sockets & discard queued data
#ifdef EBUG
	print_bufuse( ah, "atp_close");
#endif EBUG

	if(ah == NULL) return 0;

	if ( close_s2( ah->atph_socket ) < 0 ) {
		return -1;
	}

	while ( ah->atph_queue != NULL ) {
		cq = ah->atph_queue;
		ah->atph_queue = cq->atpbuf_next;
		free_atpbuf( cq );
	}

	while ( ah->atph_sent != NULL ) {
		cq = ah->atph_sent;
		for ( i = 0; i < 8; ++i ) {
			if ( cq->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] != NULL ) {
				free_atpbuf( cq->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] );
			}
		}
		ah->atph_sent = cq->atpbuf_next;
		free_atpbuf( cq );
	}

	if ( ah->atph_reqpkt != NULL ) {
		free_atpbuf( ah->atph_reqpkt );
		ah->atph_reqpkt = NULL;
	}

	for ( i = 0; i < 8; ++i ) {
		if ( ah->atph_resppkt[ i ] != NULL ) {
			free_atpbuf( ah->atph_resppkt[ i ] );
			ah->atph_resppkt[ i ] = NULL;
		}
	}

#ifdef EBUG
	print_bufuse( ah, "atp_close end");
#endif EBUG

//	free_atpbuf( (struct atpbuf *) ah );
	free(ah); //4/25/99

	return 0;
}
