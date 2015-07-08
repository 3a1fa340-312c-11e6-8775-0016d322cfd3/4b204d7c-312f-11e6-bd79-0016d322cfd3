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

#ifndef _AT_H
#define _AT_H

//*
//* Supported protocols
//*
#define ATPROTO_DDP	    0
#define ATPROTO_AARP	254

//*
//* Ethernet types, for DIX.
//* These should really be in some global header file, but we can't
//* count on them being there, and it's annoying to patch system files.
//*
#define ETHERTYPE_AT	0x809B		/* AppleTalk protocol */
#define ETHERTYPE_AARP	0x80F3		/* AppleTalk ARP */

#define DDP_MAXSZ	587

//*
//* If ATPORT_FIRST <= Port < ATPORT_RESERVED,
//* Port was created by a privileged process.
//* If ATPORT_RESERVED <= Port < ATPORT_LAST,
//* Port was not necessarily created by a
//* privileged process.
//*
#define ATPORT_FIRST	1
#define ATPORT_RESERVED	135
//#define ATPORT_RESERVED	128
#define ATPORT_LAST	255

//*
//* AppleTalk address.
//*
struct at_addr {
    uint16	s_net;
    uint8	s_node;
}PACK;
;

//*
//* AppleTalk socket
//*
struct at_socket {
	struct at_addr	addr;
	uint8  port;
};

#define ATADDR_ANYNET	(uint16)0x0000
#define ATADDR_ANYNODE	(uint8)0x00
#define ATADDR_ANYPORT	(uint8)0x00
#define ATADDR_BCAST	(uint8)0xff  /* There is no BCAST for NET */

//*
//* Socket address, AppleTalk style.  We keep magic information in the
//* zero bytes.  There are three types, NONE, CONFIG which has the phase
//* and a net range, and IFACE which has the network address of an
//* interface.  IFACE may be filled in by the client, and is filled in
//* by the kernel.
//*
struct sockaddr_at {
	uint8  sat_len;		//eCos
	uint8  sat_family;	//eCos
	uint8  sat_port;
	struct at_addr	sat_addr;
	char   sat_zero[ 8 ];
}PACK;

//*
//* AppleTalk Address Status
//*
#define AT_PROBING   0x01
#define AT_RESET     0x02
#define AT_UNSTABLE  0x04
#define AT_STABLE    0x08

struct NetRange {
	uint16 first;
	uint16 last;
};

struct AT_IFACE {
	uint8  zonename[33];
	struct at_addr my;
	struct at_addr gate;
	struct NetRange netrange;
	uint8  status;	 //PROBEING, RESET, UNSTABLE, STABLE
};

#endif _AT_H
