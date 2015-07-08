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
#ifndef _PAP_H
#define _PAP_H

#define PAP_OPEN	1
#define PAP_OPENREPLY	2
#define PAP_READ	3
#define PAP_DATA	4
#define PAP_TICKLE	5
#define PAP_CLOSE	6
#define PAP_CLOSEREPLY	7
#define PAP_SENDSTATUS	8
#define PAP_STATUS	9

#define PAP_MAXDATA	512
#define PAP_MAXQUANTUM	8

//#define PAP_QUEUE_SIZE ((int)((PAP_MAXDATA*PAP_MAXQUANTUM) / BLOCKSIZE)+1)
#define PAP_QUEUE_SIZE (4)	//5/19/2000

//Jesse #define PAP_SOCKSUM		12
#define PAP_SOCKSUM		6
typedef struct pap_sock {
	BYTE	flag; 
	int     needrelase;
};

extern struct pap_sock pap_socksum[PAP_SOCKSUM];

#endif  _PAP_H
