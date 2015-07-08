/*
 * Our own memory maintenance for atp
*/

#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "atalkd.h"
#include "atp.h"

static struct atpbuf 	*free_list = NULL;	/* free buffers */

int	atpNumBufs = 0;

struct atpbuf *alloc_atpbuf(void)
{
	struct atpbuf *bp;

	if( free_list == NULL) {
		if((free_list = (struct atpbuf *)malloc(sizeof( struct atpbuf ))) == NULL) {
			return NULL;
		}
		memset(free_list, 0x00, sizeof( struct atpbuf ));
		free_list->atpbuf_next = NULL;
		atpNumBufs++;
	}

	bp = free_list;
	free_list = free_list->atpbuf_next;
	return bp;
}

int free_atpbuf(struct atpbuf *bp)
{
	if ( bp == NULL ) return -1;
	bp->atpbuf_next = free_list;
	free_list = bp;
	return 0;
}

void atpbuf_garbage(void)
{
	struct atpbuf *bp;

	while( (bp = free_list) != NULL)
	{
		free_list = free_list->atpbuf_next;
		free(bp);
		atpNumBufs--;
	}
}

#ifdef EBUG
void print_bufuse(ATP ah, char *s)
{
	struct atpbuf	*bp;
	int	             i, sentcount, incount, respcount;

	sentcount = 0;
	for ( bp = ah->atph_sent; bp != NULL; bp = bp->atpbuf_next ) {
		++sentcount;
		for ( i = 0; i < 8; ++i ) {
			if ( bp->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] != NULL ) {
				++sentcount;
			}
		}
	}

	if ( ah->atph_reqpkt != NULL ) {
		++sentcount;
	}

	incount = 0;
	for ( bp = ah->atph_queue; bp != NULL; bp = bp->atpbuf_next, ++incount );

	respcount = 0;
	for ( i = 0; i < 8; ++i ) {
		if ( ah->atph_resppkt[ i ] != NULL ) {
			++respcount;
		}
	}

	printf( "<%d> %s: bufs total %d  sent %d  incoming %d  req %d  resp %d\n",
	getpid(), s, numbufs, sentcount, incount,
	( ah->atph_reqpkt != NULL ) ? 1: 0, respcount );
}
#endif EBUG
