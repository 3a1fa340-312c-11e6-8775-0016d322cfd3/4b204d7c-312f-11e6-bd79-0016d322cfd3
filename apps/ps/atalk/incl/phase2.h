/*
 * Copyright (c) 1990,1991 Regents of The University of Michigan.
 * All Rights Reserved.
 */
#ifndef _PHASE2_H
#define _PHASE2_H

struct EtherSNAP {
	uint8  dest[6];
	uint8  src[6];
	uint16 length;
	uint8  llc_dsap;
	uint8  llc_ssap;
	uint8  control;
	uint8  org_code[3];
	uint16 ether_type;
}PACK;

#define LLC_UI          0x03
#define LLC_SNAP_LSAP   0xAA

#endif _PHASE2_H
