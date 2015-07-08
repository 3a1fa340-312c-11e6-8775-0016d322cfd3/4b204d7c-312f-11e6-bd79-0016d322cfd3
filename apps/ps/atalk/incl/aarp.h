/*
 * Copyright (c) 1990,1991 Regents of The University of Michigan.
 * All Rights Reserved.
 */

/*
 * This structure is used for both phase 1 and 2. Under phase 1
 * the net is not filled in. It is in phase 2. In both cases, the
 * hardware address length is (for some unknown reason) 4. If
 * anyone at Apple could program their way out of paper bag, it
 * would be 1 and 3 respectively for phase 1 and 2.
 */
#ifndef _AARP_H
#define _AARP_H

struct e_ap_node {
	uint8		an_zero;
	uint16		an_net;
	uint8		an_node;
    }PACK;
    
union aapa {
    uint8		ap_pa[4];
	struct e_ap_node ap_node;
};

struct ether_aarp {
	uint16 		aarp_hrd;      //hardware type (Ethernet = 1, Tokenring = 2)
	uint16      aarp_pro;      //protocol type (ATALK= 0x809B)
	uint8       aarp_hln;      //hardware address length = 6
	uint8       aarp_pln;      //Protocol address length = 4
	uint16      aarp_op;	   //Function : request, response, probe
    uint8		aarp_sha[6];   //source hardware address
	union aapa	aarp_spu;      //source protocol address
    uint8		aarp_tha[6];   //target hardware address
	union aapa	aarp_tpu;      //target protocol address
}PACK;
#define aarp_spa	aarp_spu.ap_node.an_node
#define aarp_tpa	aarp_tpu.ap_node.an_node
#define aarp_spnet	aarp_spu.ap_node.an_net
#define aarp_tpnet	aarp_tpu.ap_node.an_net
#define aarp_spnode	aarp_spu.ap_node.an_node
#define aarp_tpnode	aarp_tpu.ap_node.an_node

/* Ron define on 12/03/04 */
#define aarp_spzero	aarp_spu.ap_node.an_zero
#define aarp_tpzero	aarp_tpu.ap_node.an_zero


struct aarptab {
	struct  aarptab *next, *prev;

//615wu //Time until aging this entry
	cyg_handle_t aat_sysClk;
	cyg_handle_t aat_counter;
	cyg_handle_t aat_alarm;
	cyg_alarm aat_timerAlarm;

	struct ambuf *pending;		/* Queue of datagrams awaiting resolution */
    struct  at_addr	aat_ataddr;
    uint8	aat_enaddr[ 6 ];
	enum {
		AARP_PENDING,	/* Incomplete */
		AARP_VALID	/* Complete */
	} state;
};

#define AARPHRD_ETHER	0x0001

#define AARPOP_REQUEST	0x01
#define AARPOP_RESPONSE	0x02
#define AARPOP_PROBE	0x03

#endif  _AARP_H
