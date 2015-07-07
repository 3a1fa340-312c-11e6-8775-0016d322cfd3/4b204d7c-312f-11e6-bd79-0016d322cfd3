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
#ifndef	_DDP_H
#define	_DDP_H


/* User Data delivery Protocol control block
 * Each entry on the receive queue consists of the
 * remote socket structure, followed by any data
 */
struct ddp_cb {
	struct ddp_cb *next;
    uint8  atport;
	struct ambuf *rcvq;	//Queue of pending datagrams
	int rcvcnt;		// Count of pending datagrams
	int user;		// User link
};
extern struct ddp_cb *Ddps;	/* Hash table for DDP structures */

/*
 * <-1byte(8bits) ->
 * +---------------+
 * | 0 | hopc  |len|
 * +---------------+
 * | len (cont)    |
 * +---------------+
 * |               |
 * +- DDP csum    -+
 * |               |
 * +---------------+
 * |               |
 * +- Dest NET    -+
 * |               |
 * +---------------+
 * |               |
 * +- Src NET     -+
 * |               |
 * +---------------+
 * | Dest NODE     |
 * +---------------+
 * | Src NODE      |
 * +---------------+
 * | Dest PORT     |
 * +---------------+
 * | Src PORT      |
 * +---------------+
 *
 * On Apples, there is also a ddp_type field, after src_port. However,
 * under this unix implementation, user level processes need to be able
 * to set the ddp_type. In later revisions, the ddp_type may only be
 * available in a raw_appletalk interface.
 */
struct elaphdr {
    uint8	el_dnode;
    uint8	el_snode;
    uint8	el_type;
};

#define	SZ_ELAPHDR	3

#define ELAP_DDPSHORT	0x01
#define ELAP_DDPEXTEND	0x02

/*
 * Extended DDP header. Includes sickness for dealing with arbitrary
 * bitfields on a little-endian arch.
 */

//DDP externded header

typedef struct
{
//**LITTLE_ENDIAN **
	unsigned short	dub_sum:16;
	unsigned short	dub_len:10;
	unsigned short	dub_hops:4;
	unsigned short	dub_pad:2;
}PACK e_du_bits;

struct ddpehdr {
    union {
	e_du_bits du_bits;
	DWORD	du_bytes;
    } deh_u;
#define deh_pad		deh_u.du_bits.dub_pad
#define deh_hops	deh_u.du_bits.dub_hops
#define deh_len		deh_u.du_bits.dub_len
#define deh_sum		deh_u.du_bits.dub_sum
#define deh_bytes	deh_u.du_bytes
	uint16		deh_dnet;
	uint16		deh_snet;
	uint8		deh_dnode;
	uint8		deh_snode;
	uint8		deh_dport;
	uint8		deh_sport;
};

#define DDP_MAXHOPS	15

#endif	//_DDP_H
