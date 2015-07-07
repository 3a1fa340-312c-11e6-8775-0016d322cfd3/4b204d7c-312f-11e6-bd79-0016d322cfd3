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

#ifdef DROP_ATPTREL
static int	release_count = 0;
#endif DROP_ATPTREL

#ifdef PC_OUTPUT
void print_addr(char *s, struct sockaddr_at	*saddr);
#endif PC_OUTPUT

int resend_request(ATP ah)
{
	//*
	//* update bitmap and send request packet
	//*
	struct atphdr	req_hdr;

#ifdef EBUG
	printf( "\n<%d> resend_request: resending %d byte request packet",
	        getpid(), ah->atph_reqpkt->atpbuf_dlen );
	print_addr( " to", &ah->atph_reqpkt->atpbuf_addr );
	putchar( '\n' );
	bprint( ah->atph_reqpkt->atpbuf_info.atpbuf_data,
	        ah->atph_reqpkt->atpbuf_dlen );
#endif

	bcopy( ah->atph_reqpkt->atpbuf_info.atpbuf_data + 1, (char *)&req_hdr,
	       sizeof( struct atphdr ));
	req_hdr.atphd_bitmap = ah->atph_rbitmap;
	bcopy( (char *)&req_hdr, ah->atph_reqpkt->atpbuf_info.atpbuf_data + 1,
	       sizeof( struct atphdr ));

	ah->atph_reqtv = msclock();
	if ( sendto2( ah->atph_socket, ah->atph_reqpkt->atpbuf_info.atpbuf_data,
	             ah->atph_reqpkt->atpbuf_dlen, 0,
	             (struct sockaddr *)&ah->atph_reqpkt->atpbuf_addr,
	             sizeof( struct sockaddr_at )) != ah->atph_reqpkt->atpbuf_dlen )             
	{
		return( -1 );
	}

	if ( ah->atph_reqtries > 0 ) {
		--(ah->atph_reqtries);
	}

	return( 0 );
}

int atp_rsel(
	ATP                ah,     // open atp handle
	struct sockaddr_at *faddr, // address to receive from
	int                func    // which function(s) to wait for;
	                           //  0 means request or response
){
	struct atpbuf	*abuf, *pb, *cb;
	struct atphdr	req_hdr, resp_hdr;
	int			i, recvlen, requesting, mask;
	uint8		rfunc;
	uint16		tid;
	uint32      tv;
	struct sockaddr_at	saddr;

#ifdef EBUG
	print_bufuse( ah, "atp_rsel at top" );
#endif
	if ( func == 0 ) {
		func = ATP_FUNCANY;
	}

	requesting = ( func & ATP_TRESP ) && ah->atph_rrespcount > 0 &&
	   ( ah->atph_reqtries > 0 || ah->atph_reqtries == ATP_TRIES_INFINITE );

	if( requesting && ah->atph_rbitmap == 0 ) {
		//*
		//* we already have a complete atp response; just return
		//*
		return( ATP_TRESP );
	}

	if (( abuf = alloc_atpbuf()) == NULL ) {
		return( -1 );
	}

	if ( requesting ) {
#ifdef EBUG
		printf( "<%d> atp_rsel: request pending\n", getpid());
#endif
		if ( msclock() - ah->atph_reqtv > (ah->atph_reqto* 1000L)) {
			if ( resend_request( ah ) < 0 ) {
				free_atpbuf( abuf );
				return( -1 );
			}
		}
	}

	for ( ;; ) {
		rfunc = func;
		if ( requesting )
			setsocketopt2(ah->atph_socket,SO_RCV_TIMEOUT);	
		bcopy( (char *)faddr, (char *)&saddr, sizeof( struct sockaddr_at ));
#ifdef EBUG
		printf( "<%d> atp_rsel calling recv_atp,", getpid());
		print_addr( " accepting from: ", &saddr );
		putchar( '\n' );
#endif EBUG

		recvlen = recv_atp( ah, &saddr, &rfunc, ATP_TIDANY,abuf->atpbuf_info.atpbuf_data, 0 );
		if ( requesting )
			setsocketopt2(ah->atph_socket,0);
		if(recvlen >= 0 ) {
			break;	// we received something
		}

		//      <<<<< error >>>>>

		if ( recvlen < 0 )         //recv_atp time out return value
			break;

		if (!requesting) {
			break;	// error
		}

		if ( ah->atph_reqtries <= 0 &&
			ah->atph_reqtries != ATP_TRIES_INFINITE ) {
			break;
		}

		if ( resend_request( ah ) < 0 ) {
	    	break;	// error
		}
	}

	if ( recvlen <= 0 ) {	/* error */
		free_atpbuf( abuf );
		return( recvlen );  //Time Out or receive TREL
	}

#ifdef EBUG
	AtSaySpace(0,23,79);
	print_addr( "(ATALKD) from: ", &saddr );
	printf( "rcvd %d bytes", recvlen );
#endif

	abuf->atpbuf_dlen = recvlen;
	bcopy( abuf->atpbuf_info.atpbuf_data + 1, (char *)&resp_hdr,
	       sizeof( struct atphdr ));

	if ( rfunc == ATP_TREQ ) {
		//*
		//* we got a request: check to see if it is a duplicate (XO)
		//* while we are at it, we expire old XO responses from sent list
		//*
		bcopy( abuf->atpbuf_info.atpbuf_data + 1, (char *)&req_hdr,
		       sizeof( struct atphdr ));
		tid = ntohs( req_hdr.atphd_tid );
		tv = msclock();
		for ( pb = NULL, cb = ah->atph_sent; cb != NULL;
		      pb = cb, cb = cb->atpbuf_next )
		{
#ifdef EBUG
			printf( "<%d>", getpid());
			print_addr( " examining", &cb->atpbuf_addr );
			printf( " %hu", cb->atpbuf_info.atpbuf_xo.atpxo_tid );
			print_addr( " (looking for", &saddr );
			printf( " %hu)\n", tid );
#endif
			if ( tv - cb->atpbuf_info.atpbuf_xo.atpxo_tv
			     > (cb->atpbuf_info.atpbuf_xo.atpxo_reltime*1000L) )
			{
				// discard expired response
#ifdef PC_OUTPUT
				printf( "(ATALKD) expiring tid %hu\n",
				        cb->atpbuf_info.atpbuf_xo.atpxo_tid );
#endif PC_OUTPUT
				if ( pb == NULL ) {
					ah->atph_sent = cb->atpbuf_next;
				} else {
					pb->atpbuf_next = cb->atpbuf_next;
				}

				for ( i = 0; i < 8; ++i ) {
					if(cb->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] != NULL ) {
						free_atpbuf( cb->atpbuf_info.atpbuf_xo.atpxo_packet[ i ] );
					}
				}
				free_atpbuf( cb );

				if (( cb = pb ) == NULL ) break;

			} else
				if ( at_addr_eq( &saddr, &cb->atpbuf_addr )) {
					if ( cb->atpbuf_info.atpbuf_xo.atpxo_tid == tid ) {
						break;
					}
				}
			}

			if ( cb != NULL ) {
#ifdef EBUG
				printf( "<%d> duplicate request -- re-sending XO resp\n",
				         getpid());
#endif
				// matches an old response -- just re-send and reset expire
				cb->atpbuf_info.atpbuf_xo.atpxo_tv = tv;
				for ( i = 0; i < 8; ++i ) {
					if ( cb->atpbuf_info.atpbuf_xo.atpxo_packet[i] != NULL &&
					     req_hdr.atphd_bitmap & ( 1 << i ))
					{
						sendto2( ah->atph_socket,
						        cb->atpbuf_info.atpbuf_xo.atpxo_packet[i]->atpbuf_info.atpbuf_data,
						        cb->atpbuf_info.atpbuf_xo.atpxo_packet[i]->atpbuf_dlen,
						        0, (struct sockaddr *)&saddr,
						        sizeof( struct sockaddr_at ));						        
					}
				}
			}

			if ( cb == NULL ) {
				// new request -- queue it and return
				bcopy( (char *)&saddr, (char *)&abuf->atpbuf_addr,sizeof( struct sockaddr_at ));
				bcopy( (char *)&saddr, (char *)faddr,sizeof( struct sockaddr_at ));
				abuf->atpbuf_next = ah->atph_queue;
				ah->atph_queue = abuf;
				return( ATP_TREQ );
			} else {
				free_atpbuf( abuf );
				return( 0 );
			}
		}

		//*
		//* we got a response: update bitmap
		//*
		bcopy( ah->atph_reqpkt->atpbuf_info.atpbuf_data + 1, (char *)&req_hdr,
		       sizeof( struct atphdr ));
		if ( requesting && ah->atph_rbitmap & ( 1<<resp_hdr.atphd_bitmap )
		     && req_hdr.atphd_tid == resp_hdr.atphd_tid )
		{
			ah->atph_rbitmap &= ~( 1<<resp_hdr.atphd_bitmap );

			if ( ah->atph_resppkt[ resp_hdr.atphd_bitmap ] != NULL ) {
				free_atpbuf( ah->atph_resppkt[ resp_hdr.atphd_bitmap ] );
			}
			ah->atph_resppkt[ resp_hdr.atphd_bitmap ] = abuf;

			//* if End Of Message, clear all higher bitmap bits
			if ( resp_hdr.atphd_ctrlinfo & ATP_EOM ) {
#ifdef EBUG
				printf( "<%d> EOM -- seq num %d  current bitmap %d\n",
					getpid(), resp_hdr.atphd_bitmap, ah->atph_rbitmap );
#endif
				mask = 1 << resp_hdr.atphd_bitmap;
				ah->atph_rbitmap &= ( mask | mask-1 );
			}

			//* if Send Trans. Status, send updated request
			if ( resp_hdr.atphd_ctrlinfo & ATP_STS ) {
#ifdef EBUG
				puts( "STS" );
#endif
				req_hdr.atphd_bitmap = ah->atph_rbitmap;
				bcopy( (char *)&req_hdr,
				       ah->atph_reqpkt->atpbuf_info.atpbuf_data + 1,
				       sizeof( struct atphdr ));
				if ( sendto2( ah->atph_socket,
				     ah->atph_reqpkt->atpbuf_info.atpbuf_data,
				     ah->atph_reqpkt->atpbuf_dlen, 0,
				     (struct sockaddr *) &ah->atph_reqpkt->atpbuf_addr,
				     sizeof( struct sockaddr_at )) !=
				     ah->atph_reqpkt->atpbuf_dlen )		
				{
					free_atpbuf( abuf );
					return( -1 );
				}
			}
		} else {
			//*
			//* we are not expecting this response -- toss it
			//*
			free_atpbuf( abuf );
#ifdef EBUG
			printf( "atp_rsel: ignoring resp bm=%x tid=%d (expected %x/%d)\n",
			        resp_hdr.atphd_bitmap, ntohs( resp_hdr.atphd_tid ),
			        ah->atph_rbitmap, ah->atph_tid );
#endif EBUG
		}

		if ( !ah->atph_rbitmap && ( req_hdr.atphd_ctrlinfo & ATP_XO )) {
			//*
			//* successful completion - send release
			//* the release consists of DDP type byte + ATP header + 4 user bytes
			//*
			req_hdr.atphd_ctrlinfo = ATP_TREL;
			bcopy( (char *)&req_hdr, ah->atph_reqpkt->atpbuf_info.atpbuf_data + 1,
			       sizeof( struct atphdr ));
			memset(ah->atph_reqpkt->atpbuf_info.atpbuf_data + ATP_HDRSIZE,'\0',4 );
			ah->atph_reqpkt->atpbuf_dlen = sizeof( struct atphdr ) + ATP_HDRSIZE;
#ifdef DROP_ATPTREL
			if (( ++release_count % 10 ) != 0 ) {
#endif DROP_ATPTREL
				sendto2( ah->atph_socket, ah->atph_reqpkt->atpbuf_info.atpbuf_data,
				        ah->atph_reqpkt->atpbuf_dlen, 0,
				        (struct sockaddr *) &ah->atph_reqpkt->atpbuf_addr,
				        sizeof( struct sockaddr_at));		        
// To avoid release transaction packet lose on WLAN MAC ...Ron modified
#ifdef WIRELESS_CARD
				sendto2( ah->atph_socket, ah->atph_reqpkt->atpbuf_info.atpbuf_data,
				        ah->atph_reqpkt->atpbuf_dlen, 0,
				        (struct sockaddr *) &ah->atph_reqpkt->atpbuf_addr,
				        sizeof( struct sockaddr_at));

				sendto2( ah->atph_socket, ah->atph_reqpkt->atpbuf_info.atpbuf_data,
				        ah->atph_reqpkt->atpbuf_dlen, 0,
				        (struct sockaddr *) &ah->atph_reqpkt->atpbuf_addr,
				        sizeof( struct sockaddr_at));
#endif
#ifdef DROP_ATPTREL
			}
#endif DROP_ATPTREL
	}

	if ( ah->atph_rbitmap != 0 ) {
		if(ah->atph_reqtries > 0 || ah->atph_reqtries == ATP_TRIES_INFINITE ){
			return( 0 );
		} else {
			return( -1 );
		}
	}

	bcopy( (char *)&saddr, (char *)faddr, sizeof( struct sockaddr_at ));
	return( ATP_TRESP );
}

#ifdef PC_OUTPUT
void print_addr(char *s, struct sockaddr_at	*saddr)
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
#endif PC_OUTPUT
