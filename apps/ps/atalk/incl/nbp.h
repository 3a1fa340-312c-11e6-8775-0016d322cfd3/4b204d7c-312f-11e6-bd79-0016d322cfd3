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

#ifndef _NBP_H
#define _NBP_H

struct nbphdr {
//*** LITTLE_ENDIAN ***
	uint8  nh_cnt : 4;
	uint8  nh_op  : 4;
//*********************
	uint8  nh_id  : 8;
}PACK;

#define SZ_NBPHDR	sizeof(struct nbphdr)

struct nbptuple {
	uint16	nt_net;
	uint8 	nt_node;
	uint8 	nt_port;
	uint8 	nt_enum;
}PACK;

#define SZ_NBPTUPLE	sizeof(struct nbptuple)

#define NBPSTRLEN	32

//*
//* Note that NBPOP_RGSTR, _UNRGSTR, _OK, and _ERROR, are not standard.
//* as Apple adds more NBPOPs, we need to check for collisions with our
//* extra values.
//*
#define NBPOP_BRRQ			0x1
#define NBPOP_LKUP			0x2
#define NBPOP_LKUPREPLY		0x3
#define NBPOP_FWD			0x4
#define NBPOP_RGSTR			0xc
#define NBPOP_UNRGSTR		0xd
#define NBPOP_OK			0xe
#define NBPOP_ERROR			0xf

#define NBPMATCH_NOGLOB		(1<<1)
#define NBPMATCH_NOZONE		(1<<2)

#define NBP_DDPTYPE         2
#define NBP_SOCKET_NUM      2

#define nbpEquals 			'='
#define nbpWild   			0xc5
#define nbpStar   			'*'


int16 nbp_rgstr(void);
void nbp_input(cyg_addrword_t data);
uint8 *GetATPortName(int i);

#endif  _NBP_H
