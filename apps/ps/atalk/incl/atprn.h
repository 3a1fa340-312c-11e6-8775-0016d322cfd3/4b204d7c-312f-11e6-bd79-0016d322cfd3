/*
 * Copyright (c) 1990,1995 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */
#ifndef _PRINTER_H
#define _PRINTER_H

struct atprn {
#ifdef _PC
	uint8 *p_ppdfile;
#endif _PC
	ATP	   p_atp;
	ATP    cur_atp;	 //current connect ATP;
	struct sockaddr_at	sat;  //remote socket address
	uint8 connid;
	uint8 quantum;
	uint8 *jobname;
};

#define P_PIPED		(1<<0)
#define P_SPOOLED	(1<<1)
#define P_REGISTERED	(1<<2)
#define P_ACCOUNT	(1<<3)
#define P_AUTH		(1<<4)

extern struct atprn pr[];

#endif _PRINTER_H
