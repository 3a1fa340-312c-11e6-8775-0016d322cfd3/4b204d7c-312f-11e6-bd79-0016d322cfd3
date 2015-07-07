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
//#include "network.h"
#include "psglobal.h"
#include "atalkd.h"
#include "atp.h"
#include "atp_in.h"

#ifdef EBUG
#include <stdio.h>
#endif

void build_req_packet(
	struct atpbuf	*pktbuf,
	uint16  tid,
	uint8	ctrl,
	struct atp_block	*atpb
){
	struct atphdr hdr;

	//*
	//* fill in the packet fields
	//*
	hdr.atphd_ctrlinfo = ctrl;
	hdr.atphd_bitmap = atpb->atp_bitmap;
	hdr.atphd_tid = htons( tid );
	*(pktbuf->atpbuf_info.atpbuf_data) = ATP_DDPTYPE;
	bcopy( (char *) &hdr, (char *) pktbuf->atpbuf_info.atpbuf_data + 1,
	       sizeof( struct atphdr ));
	bcopy( (char *) atpb->atp_sreqdata, (char *) pktbuf->atpbuf_info.atpbuf_data
	       + ATP_HDRSIZE, atpb->atp_sreqdlen );

    //* set length
	pktbuf->atpbuf_dlen = ATP_HDRSIZE + atpb->atp_sreqdlen;
}

void build_resp_packet(
	struct atpbuf	*pktbuf,
	uint16		tid,
	uint8		ctrl,
	struct atp_block	*atpb,
	int16		seqnum
){
    struct atphdr	hdr;

    //* fill in the packet fields
    //*
	*(pktbuf->atpbuf_info.atpbuf_data) = ATP_DDPTYPE;
	hdr.atphd_ctrlinfo = ctrl;
	hdr.atphd_bitmap = seqnum;
	hdr.atphd_tid = htons( tid );
	bcopy( (char *) &hdr, (char *) pktbuf->atpbuf_info.atpbuf_data + 1,
	       sizeof( struct atphdr ));
	bcopy( (char *) atpb->atp_sresiov[ seqnum ].iov_base,
	       (char *) pktbuf->atpbuf_info.atpbuf_data + ATP_HDRSIZE,
	       atpb->atp_sresiov[ seqnum ].iov_len );

	//* set length
	//*
	pktbuf->atpbuf_dlen = ATP_HDRSIZE + atpb->atp_sresiov[ seqnum ].iov_len;
}


//void dump_packet(int8 *buf, int16 len)
//{
//	int16 i;
//
//	for ( i = 0; i < len; ++i ) {
//		printf( "%x-%c ", buf[i], buf[i] );
//	}
//	putchar( '\n' );
//}

int
recv_atp(
	ATP	        ah,
	struct sockaddr_at *fromaddr,
	uint8       *func,
	uint16      tid,
	char		*rbuf,
	int16       wait
){
//*
//  Receive a packet from address fromaddr of the correct function type
//  and with the correct tid.  fromaddr = AT_ANY... and function == ATP_TYPEANY
//  and tid == ATP_TIDANY can be used to wildcard match.
//
//  recv_atp returns the length of the packet received (or -1 if error)
//  The function code for the packet received is returned in *func (ATP_TREQ or
//    ATP_TRESP).
//*
	struct atpbuf	*pq, *cq;
	struct atphdr	ahdr;
	uint16  rfunc;
	uint16  rtid;
	int			i;
	int			dlen = -1;
	int			recvlen;
	struct sockaddr_at	faddr;
	int			faddrlen;
	struct atpbuf	*inbuf;

	tid = htons( tid );

    //* first check the queue
    //*
#ifdef EBUG
	print_bufuse( ah, "recv_atp checking queue" );
#endif
	for ( pq = NULL, cq = ah->atph_queue; cq != NULL;
	      pq = cq, cq = NGET32(cq->atpbuf_next) )
	{
		bcopy( (char *) cq->atpbuf_info.atpbuf_data + 1, (char *)&ahdr,
		       sizeof( struct atphdr ));
		rfunc = ahdr.atphd_ctrlinfo & ATP_FUNCMASK;
#ifdef EBUG
		printf( "<%d> checking", getpid());
		printf( " tid=%hu func=", ntohs( ahdr.atphd_tid ));
		print_func( rfunc );
		print_addr( " from", &cq->atpbuf_addr );
		putchar( '\n' );
#endif
		if ((( tid & ahdr.atphd_tid ) == ahdr.atphd_tid ) &&
		    (( *func & rfunc ) == rfunc )
		    && at_addr_eq( fromaddr, &cq->atpbuf_addr ))
		{
			break;
		}
	}

	if ( cq != NULL ) {
		//* we found one in the queue -- copy to rbuf
		//*
		dlen = cq->atpbuf_dlen;
		*func = rfunc;
		bcopy( (char *) &cq->atpbuf_addr, (char *) fromaddr,
		       sizeof( struct sockaddr_at ));
		bcopy( (char *) cq->atpbuf_info.atpbuf_data, (char *) rbuf,
		       cq->atpbuf_dlen );

		//* remove packet from queue and free buffer
		//*
		if ( pq == NULL ) {
			ah->atph_queue = NULL;
		} else {
			pq->atpbuf_next = cq->atpbuf_next;
		}
		free_atpbuf( cq );
#ifdef PC_OUTPUT
		if(dlen <= 0)
		    printf("RECV_ATP(0) error (atp_pkt.c)\n");
#endif PC_OUTPUT
		return( dlen );
	}

	//* we need to get it the net -- call on ddp to receive a packet
	//*
#ifdef EBUG
	printf( "<%d>", getpid());
	print_addr( " waiting on address", &ah->atph_saddr );
	printf( "\nfor tid=%hu func=", ntohs( tid ));
	print_func( *func );
	print_addr( " from", fromaddr );
	putchar( '\n' );
#endif

	do {
#ifdef EBUG
		fflush( stdout );
#endif
		faddrlen = sizeof( struct sockaddr_at );
		memset( (char *) &faddr,'\0', sizeof( struct sockaddr_at ));
		if (( recvlen = recvfrom2( ah->atph_socket, rbuf, ATP_BUFSIZ, 0,
		    (struct sockaddr *) &faddr, &faddrlen )) < 0 ) 
		{
		    return -1;
		}
		bcopy( rbuf + 1, (char *)&ahdr, sizeof( struct atphdr ));
		if ( recvlen >= ATP_HDRSIZE && *rbuf == ATP_DDPTYPE) {
			// this is a valid ATP packet -- check for a match
			rfunc = ahdr.atphd_ctrlinfo & ATP_FUNCMASK;
			rtid = ahdr.atphd_tid;
#ifdef EBUG
			printf( "<%d> got tid=%hu func=", getpid(), ntohs( rtid ));
			print_func( rfunc );
			print_addr( " from", &faddr );
			putchar( '\n' );
			bprint( rbuf, recvlen );
#endif
			if( rfunc == ATP_TREL ) {
				//* remove response from sent list
				for ( pq = NULL, cq = ah->atph_sent; cq != NULL;
					pq = cq, cq = cq->atpbuf_next ) {
			if( at_addr_eq( &faddr, &cq->atpbuf_addr ) &&
			    cq->atpbuf_info.atpbuf_xo.atpxo_tid == ntohs( rtid ))
				break;
			}
			if( cq != NULL ) {
#ifdef EBUG
				printf( "<%d> releasing transaction %hu\n", getpid(), ntohs( rtid ));
#endif
				if ( pq == NULL ) {
					ah->atph_sent = cq->atpbuf_next;
				} else {
					pq->atpbuf_next = cq->atpbuf_next;
				}
				for ( i = 0; i < 8; ++i ) {
					if ( cq->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] != NULL )
					{
						free_atpbuf( cq->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] );
					}
				}
				free_atpbuf( cq );
			}

		} else
			if ((( tid & rtid ) == rtid ) &&
			    (( *func & rfunc ) == rfunc ) &&
			    at_addr_eq( fromaddr, &faddr ))
			{ // got what we wanted
				*func = rfunc;
				dlen = recvlen;
				bcopy( (char *) &faddr, (char *) fromaddr,
				       sizeof( struct sockaddr_at ));

			}
			else {
				// add packet to incoming queue
#ifdef EBUG
				printf( "<%d> queuing incoming...\n", getpid() );
#endif
				if (( inbuf = alloc_atpbuf()) == NULL ) {
					return -1;
				}
				bcopy( (char *)&faddr, (char *)&inbuf->atpbuf_addr,
				       sizeof( struct sockaddr_at ));
				inbuf->atpbuf_next = ah->atph_queue;
				inbuf->atpbuf_dlen = recvlen;
				bcopy( (char *)rbuf,
				       (char *)inbuf->atpbuf_info.atpbuf_data, recvlen );
			}
		}
		if ( !wait && dlen < 0 ) {
			return( 0 );
		}

	} while ( dlen < 0 );

#ifdef PC_OUTPUT
	if(dlen <= 0)
	    printf("RECV_ATP(2) error (atp_pkt.c)\n");
#endif PC_OUTPUT
	return( dlen );
}


int at_addr_eq(
	struct sockaddr_at	*paddr,		//* primary address
	struct sockaddr_at	*saddr		//* secondary address
){
//* compare two atalk addresses -- only check the non-zero fields
//* of paddr against saddr.
//* return zero if not equal, non-zero if equal
//*
	return (   ( paddr->sat_port == ATADDR_ANYPORT
	           || paddr->sat_port == saddr->sat_port )
	        && (  paddr->sat_addr.s_net == ATADDR_ANYNET
	           || paddr->sat_addr.s_net == saddr->sat_addr.s_net )
	        && ( paddr->sat_addr.s_node == ATADDR_ANYNODE
	           || paddr->sat_addr.s_node == saddr->sat_addr.s_node )
	       );
}


#ifdef EBUG
void print_addr(char *s, struct sockaddr_at *saddr)
{
	printf( "%s ", s );
	saddr->sat_family == AF_APPLETALK ? printf( "at." ) :
		printf( "%d.", saddr->sat_family );
	saddr->sat_addr.s_net == ATADDR_ANYNET ? printf( "*." ) :
		printf( "%d.", ntohs( saddr->sat_addr.s_net ));
	saddr->sat_addr.s_node == ATADDR_ANYNODE ? printf( "*." ) :
		printf( "%d.", saddr->sat_addr.s_node );
	saddr->sat_port == ATADDR_ANYPORT ? printf( "*" ) :
		printf( "%d", saddr->sat_port );
}


void print_func(uint8 ctrlinfo;
{
	switch ( ctrlinfo & ATP_FUNCMASK ) {
		case ATP_TREQ:
			printf( "TREQ" );
			break;
		case ATP_TRESP:
			printf( "TRESP" );
			break;
		case ATP_TREL:
			printf( "ANY/TREL" );
			break;
		case ATP_TIDANY:
			printf( "*" );
			break;
		default:
			printf( "%x", ctrlinfo & ATP_FUNCMASK );
	}
}
#endif
