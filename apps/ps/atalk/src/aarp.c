/*
 * Copyright (c) 1990,1991 Regents of The University of Michigan.
 * All Rights Reserved.
 */

#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "eeprom.h"
#include "atalkd.h"
#include "apple_mbuf.h"
#include "aarp.h"

#define PROBE_TIME      150L
#define AARPTAB_SIZE    10
#define NODE_PER_NET    253 //Phase 2 node per net
#define AARPT_PENDTIME  5
#define AARPT_AGE       60	//60 Sec

struct aarptab *aarptab[AARPTAB_SIZE] = {0};

#define AARPTAB_HASH(a) \
	( ( ( NGET16(&(a).s_net) << 8 ) + (a).s_node ) % AARPTAB_SIZE )

typedef struct  AARPPACKET{
	struct EtherSNAP  ethdr;
	struct ether_aarp ethaarp;
}PACK AARPPACKET;

//AARPPACKET  AarpPacket;

uint8 atmulticastaddr[ 6 ] = {
    0x09, 0x00, 0x07, 0xff, 0xff, 0xff
};

static struct aarptab * aarp_lookup(struct at_addr *addr);
void aarptfree(cyg_handle_t handle, cyg_addrword_t ptr);
static struct aarptab *aarptadd(struct at_addr	*addr,uint8 *hw_addr);
static void aarp_update(struct aarptab *aat, uint8 *hw_addr);
static int16 at_broadcast(struct at_addr *sat_addr);
static void aarp_set_rand_node(uint8 *node);

//615wu---------------------------------------------------------------
extern WORD MyIntNO; //send_pkt
extern int urandom(unsigned int n);
extern void restore( int is );
extern int dirps();
extern uint16 NGET16( uint8 *pSrc );
extern uint32 NGET32( uint8 *pSrc );

cyg_sem_t SEM_AARP_PROBE;

void aarp_input2ambf(void * Data,int16 DataLen)
{
	struct ambuf *apple_mbuf;
		
	if( (Data == NULL) || (DataLen <= 0)|| (DataLen > AMbufSize))
		return;	//615wu
		
	apple_mbuf = malloc(sizeof(struct ambuf));
	if (apple_mbuf == NULL)
		return ;
	memset(apple_mbuf, 0x00, sizeof(struct ambuf));
		
	memcpy(apple_mbuf->data,Data,DataLen);
	apple_mbuf->size = AMbufSize;
	apple_mbuf->cnt = DataLen;
	
	aarp_input(&apple_mbuf);
	return;
}

struct aarptab * aarp_lookup(struct at_addr *addr)
{
	struct aarptab *aat;

	for(aat= aarptab[AARPTAB_HASH(*addr)]; aat != NULL; aat = NGET32(&aat->next) )
	{
		if ( NGET16(&aat->aat_ataddr.s_net) == NGET16(&addr->s_net) &&
		    aat->aat_ataddr.s_node == addr->s_node )
			break;
	}
	
	return aat;
}

void aarp_output(uint16 aarp_op, struct at_addr *addr)
{
	struct ether_aarp *ea;
	struct sockaddr	sa;
	AARPPACKET  *AarpPacket_out;

	//AARP Request

	AarpPacket_out = malloc(sizeof(AARPPACKET));
	if (AarpPacket_out == NULL)
		return;

	memcpy(AarpPacket_out->ethdr.dest,atmulticastaddr,   6);
	memcpy(AarpPacket_out->ethdr.src ,MyPhysNodeAddress, 6);
	AarpPacket_out->ethdr.length = htons(sizeof(struct ether_aarp) + 8);
	AarpPacket_out->ethdr.llc_dsap = AarpPacket_out->ethdr.llc_ssap = LLC_SNAP_LSAP;
	AarpPacket_out->ethdr.control = LLC_UI;
	memset(AarpPacket_out->ethdr.org_code,'\0', 3);
	AarpPacket_out->ethdr.ether_type = htons( ETHERTYPE_AARP );

//	ea = &AarpPacket.ethaarp;
	ea = &(AarpPacket_out->ethaarp);  //don't use the same memory for send_pkt

	ea->aarp_hrd = htons( AARPHRD_ETHER );
	ea->aarp_pro = htons( ETHERTYPE_AT );
	ea->aarp_hln = sizeof( ea->aarp_sha );
	ea->aarp_pln = sizeof( ea->aarp_spu );
	ea->aarp_op  = htons(aarp_op);

	memcpy(ea->aarp_sha,MyPhysNodeAddress,sizeof( ea->aarp_sha ));
	memset(ea->aarp_tha,'\0',sizeof( ea->aarp_tha ));

	/* Ron Add to correct AARP frame on 12/03/04 
	
	   Since AarpPacket will be used passing WPA-TKIP, the encrypting function 
	   will rewrite all the buffer(i.e. AarpPacket) during TKIP encryption.
	   The original source ingnore set ea->aarp_spzero and ea->aarp_tpzero to zero,
	   but when WPA-TKIP enable, they "NEED" to be set to zero or we can not sure 
	   this two bytes is still zero, then bugs will present.
	   
	   We don't find this bug because we don't have TKIP before. So nobody will modify
	   this buffer(i.e. AarpPacket). Anyway, this two bytes "NEED" to be set
	   	   
	*/	
	ea->aarp_spzero = 0; //Ron Add to correct AARP frame on 12/03/04, see above comment
	
	ea->aarp_spnet  = at_iface.my.s_net;
	ea->aarp_spnode = at_iface.my.s_node;
	
	ea->aarp_tpzero = 0; //Ron Add to correct AARP frame on 12/03/04, see above comment
	ea->aarp_tpnet  = addr->s_net;
	ea->aarp_tpnode = addr->s_node;

//	send_pkt(MyIntNO,(uint8 *)&AarpPacket,60);
	send_pkt(MyIntNO,(uint8 *)AarpPacket_out,60); //don't use the same memory for send_pkt
	free(AarpPacket_out);
}

uint16 aarpresolve(
	struct ambuf *bp,	
	struct at_addr *sat_addr,
	uint8  *dest_enaddr
)
{
	struct aarptab	*aat;
	int			s;

	if ( at_broadcast(sat_addr)) {
		bcopy(atmulticastaddr,dest_enaddr,sizeof(atmulticastaddr));
		return 1;
	}

	if( (aat = aarp_lookup( sat_addr )) == NULL) {
		// no entry
		if((aat = aarptadd(sat_addr , 0)) == NULL) {
 			//not enough memory
			free_p(&bp);
			return 0;
		}
		enqueue2(&aat->pending,&bp);
		aarp_output(AARPOP_REQUEST,sat_addr);
		return 0;
	}
	if(aat->state == AARP_VALID) {
		//update timer
	
		//update alarm
		if ((aat->aat_sysClk != 0) && (aat->aat_counter!= 0) && (aat->aat_alarm != 0) && (aat->aat_timerAlarm.enabled)){
			cyg_alarm_disable(aat->aat_alarm);				//eCos	
			cyg_alarm_initialize(aat->aat_alarm, cyg_current_time() + (AARPT_AGE * TICKS_PER_SEC), 0);
		}else{	
			free_q(&aat->pending);
			free_p(&bp);
			return 0;
		}                
	
		bcopy(aat->aat_enaddr,dest_enaddr,6);
		return 1;
	}
	//still pending
	free_q(&aat->pending);

	enqueue2(&aat->pending,&bp);
	aarp_output(AARPOP_REQUEST,sat_addr);
	return 0;
}

void aarp_input(struct ambuf **bpp)
{
	struct  aarptab	*aat;
	struct  ether_aarp	*ea;
	struct  sockaddr_at	sat;
	struct  at_addr	spa, tpa;
	int	    op, s;
	unsigned int prot = 0;
	AARPPACKET  *AarpPacket;

	if(bpp == NULL || *bpp == NULL) return;	//615wu

	if ((*bpp)->cnt > AMbufSize){	//615wu
		free_p(bpp);
		return;		
	}

	ea = mtod( *bpp, struct ether_aarp *);

	// Check to see if from my hardware address	or broadcast
	if (!memcmp(ea->aarp_sha,MyPhysNodeAddress,sizeof(ea->aarp_sha)) ||
	    NGET32(ea->aarp_sha) == 0xFFFFFFFF )
	{
		free_p(bpp);
		return;
	}

	op = ntohs( ea->aarp_op );

	spa.s_net = ea->aarp_spnet;
	tpa.s_net = ea->aarp_tpnet;

	spa.s_node = ea->aarp_spnode;
	tpa.s_node = ea->aarp_tpnode;

	//Source address same as me
	if ( spa.s_net == at_iface.my.s_net && spa.s_node == at_iface.my.s_node ) {
		if ( at_iface.status & AT_PROBING ) {
			/*
			* We're probing, someone either responded to our probe, or
			* probed for the same address we'd like to use. Change the
			* address we're probing for.
			*/
			at_iface.status = AT_RESET;
			cyg_semaphore_post (&SEM_AARP_PROBE);
			free_p(bpp);
			return;
		} else if ( op != AARPOP_PROBE ) {
			//address duplicate
		    /*
			* This is not a probe, and we're not probing. This means
			* that someone's saying they have the same source address
			* as the one we're using. Get upset...
			*/
			free_p(bpp);
			return;
		}
	}

	prot = dirps();	//eCos

	if ((aat = aarp_lookup( &spa )) != NULL) {
    	//Find in aarp cache table
		if ( op == AARPOP_PROBE ) {
	    	/*
		     * Someone's probing for spa, dealocate the one we've got,
		     * so that if the prober keeps the address, we'll be able
	    	 * to arp for him.
		     */
			cyg_alarm_disable(aat->aat_alarm);				//eCos
			aarptfree( aat->aat_alarm, aat );
	    	free_p(bpp);
	    	restore(prot);	//eCos
			return;
		}
		aarp_update(aat, ea->aarp_sha);
	}

	//AARP_REQUEST and want to find me
	if ( aat == 0 && tpa.s_net == at_iface.my.s_net && tpa.s_node == at_iface.my.s_node
	    && op != AARPOP_PROBE )
	{
		aarptadd( &spa, ea->aarp_sha );
	}

	//*
	//* Don't respond to responses, and never respond if we're
	//* still probing.
	//*
	if ( tpa.s_net != at_iface.my.s_net || tpa.s_node != at_iface.my.s_node ||
	     op == AARPOP_RESPONSE || (at_iface.status & AT_PROBING)) {
		free_p(bpp);
		restore(prot);	//eCos
		return;
	}

	AarpPacket = malloc(sizeof(AARPPACKET));
	
	if (AarpPacket == NULL){
		free_p(bpp);
		restore(prot);	//eCos
		return;
		}
	//Want Response
	memcpy(AarpPacket->ethdr.dest,      ea->aarp_sha,sizeof( ea->aarp_sha ));
	memcpy(AarpPacket->ethdr.src       ,MyPhysNodeAddress,sizeof( ea->aarp_sha ));
	AarpPacket->ethdr.length = htons(sizeof(struct ether_aarp) + 8);
    AarpPacket->ethdr.llc_dsap = AarpPacket->ethdr.llc_ssap = LLC_SNAP_LSAP;
	AarpPacket->ethdr.control = LLC_UI;
	memset(AarpPacket->ethdr.org_code,'\0', 3);
	AarpPacket->ethdr.ether_type = htons( ETHERTYPE_AARP );

	memcpy(ea->aarp_tha,ea->aarp_sha,sizeof( ea->aarp_sha ));
	memcpy(ea->aarp_sha,MyPhysNodeAddress,sizeof( ea->aarp_sha ));

	ea->aarp_tpnet = ea->aarp_spnet;
	ea->aarp_spnet = at_iface.my.s_net;

	ea->aarp_tpnode = ea->aarp_spnode;
	ea->aarp_spnode = at_iface.my.s_node;

	ea->aarp_spzero = 0;
	ea->aarp_tpzero = 0;

	ea->aarp_op = htons( AARPOP_RESPONSE );
	memcpy(&(AarpPacket->ethaarp),(*bpp)->data,sizeof(struct ether_aarp));
	free_p(bpp);

    send_pkt(MyIntNO,(uint8 *)AarpPacket,60);
    
    free(AarpPacket);
    
    restore(prot);	//eCos
	return;
}

void aarptfree(cyg_handle_t handle, cyg_addrword_t p)
{
	struct aarptab *aat, *aat_in_tab;
	struct at_addr sat_addr;
	unsigned int prot = 0;

	if( (aat = (struct aarptab *) p) == NULL) {
		return;
	}
	
	prot = dirps();	//eCos	
	
	sat_addr.s_net =  aat->aat_ataddr.s_net;
    sat_addr.s_node =  aat->aat_ataddr.s_node;
	
	aat_in_tab = aarp_lookup(&sat_addr);
	
	if ((aat_in_tab == NULL) || (aat_in_tab != aat)){
    	restore(prot);	//eCos
		return;
	}
	
	if(aat->next != NULL) aat->next->prev = aat->prev;

	if(aat->prev != NULL) aat->prev->next = aat->next;
	else aarptab[AARPTAB_HASH(aat->aat_ataddr)] = aat->next;	
	
	if ((aat->aat_sysClk != 0) && (aat->aat_counter!= 0) && (aat->aat_alarm != 0)){
		cyg_alarm_delete(aat->aat_alarm);				//eCos
		cyg_clock_delete(aat->aat_sysClk);				//eCos
		cyg_counter_delete(aat->aat_counter);			//eCos
		
		aat->aat_sysClk = 0;									//eCos
		aat->aat_counter = 0;									//eCos
		aat->aat_alarm = 0;										//eCos
		memset(&(aat->aat_timerAlarm), 0x00, sizeof(cyg_alarm));//eCos
	}

	free_q(&aat->pending);
    restore(prot);	//eCos
	
	free(aat);
	return;

}

struct aarptab *aarptadd(struct at_addr	*addr,uint8 *hw_addr)
{
	struct aarptab *aat;
	uint16 hashval;
	
	if( ( aat = (struct aarptab *)malloc( sizeof(struct aarptab) ) ) == NULL){
		return NULL;
	}
	
	memset(aat, 0x00, sizeof(struct aarptab));

	hashval = AARPTAB_HASH(*addr);
	aat->prev =	NULL;
	aat->next = aarptab[hashval];
    aat->aat_ataddr.s_net = addr->s_net;
    aat->aat_ataddr.s_node = addr->s_node;
	aarptab[hashval] = aat;
	
	if(aat->next != NULL){
		aat->next->prev = aat;
	}
	
	aarp_update(aat,hw_addr);

	return aat;
}

void aarp_update(struct aarptab *aat, uint8 *hw_addr)
{
	struct ambuf *bp;

	if(hw_addr == NULL){
		aat->state = AARP_PENDING;

		aat->aat_sysClk = cyg_real_time_clock();									//eCos
		cyg_clock_to_counter(aat->aat_sysClk, &aat->aat_counter);					//eCos
		cyg_alarm_create(aat->aat_counter, (cyg_alarm_t *)aarptfree,
	                     (cyg_addrword_t) aat,
	                     &aat->aat_alarm, &aat->aat_timerAlarm);					//eCos
		cyg_alarm_initialize(aat->aat_alarm, cyg_current_time() + (AARPT_PENDTIME * TICKS_PER_SEC), 0);
	
	} else {
	
		// Response has come in, update entry and run through queue
		aat->state = AARP_VALID;

		//update alarm
		if ((aat->aat_sysClk != 0) && (aat->aat_counter!= 0) && (aat->aat_alarm != 0)){
			if (aat->aat_timerAlarm.enabled){
				cyg_alarm_disable(aat->aat_alarm);				//eCos	
				cyg_alarm_initialize(aat->aat_alarm, cyg_current_time() + (AARPT_AGE * TICKS_PER_SEC), 0);
			}else{
				free_q(&aat->pending);
			}
		}else{
			aat->aat_sysClk = cyg_real_time_clock();									//eCos
			cyg_clock_to_counter(aat->aat_sysClk, &aat->aat_counter);					//eCos
			cyg_alarm_create(aat->aat_counter, (cyg_alarm_t *)aarptfree,
		                     (cyg_addrword_t) aat,
		                     &aat->aat_alarm, &aat->aat_timerAlarm);					//eCos
			cyg_alarm_initialize(aat->aat_alarm, cyg_current_time() + (AARPT_AGE * TICKS_PER_SEC), 0);
		}	
   
   		memcpy(aat->aat_enaddr,hw_addr,6);
					
		while((bp = dequeue2(&aat->pending)) != NULL)		
				ddp_output(bp,&aat->aat_ataddr);
		}
}

void aarp_probe(void)
{
	uint16 net;
	int16 nnets;
	uint8 node[NODE_PER_NET];
	int16 i,j,k;

	nnets = at_iface.netrange.last - at_iface.netrange.first + 1;
	net = at_iface.netrange.first + urandom((const unsigned char )EEPROM_Data.EthernetID[5])%255;
	
	
	//Clear SEM_AARP_PROBE count
	cyg_semaphore_init(&SEM_AARP_PROBE,0);
	
	for(i = 0; i < nnets; i++) {
		at_iface.my.s_net = ntohs(net);
		aarp_set_rand_node(node);
		for(j = 0 ; j < NODE_PER_NET; j++) {	//node 1 - 253
			at_iface.status = AT_PROBING;
			at_iface.my.s_node = node[j];
			for(k = 0 ; k < 10 ; k++) {
				aarp_output(AARPOP_PROBE,&at_iface.my);
				cyg_semaphore_timed_wait(&SEM_AARP_PROBE,cyg_current_time() + (PROBE_TIME / MSPTICK));
				if(at_iface.status == AT_RESET) break;
			}
			if(at_iface.status == AT_PROBING) goto AARP_OK;
		}
		net = at_iface.netrange.first +
		      (net - at_iface.netrange.first + 1) % nnets;
	}
#ifdef PC_OUTPUT
	printf("ATALKD: PROBE ADDRESS ERROR !\n");
#endif PC_OUTPUT
AARP_OK:
	at_iface.status = AT_STABLE;
}

int16 probe_last_addr(void)
{
	int16 i;

	for(i = 0 ; i < 10 ; i++) {
		aarp_output(AARPOP_PROBE,&at_iface.my);
		cyg_semaphore_timed_wait(&SEM_AARP_PROBE,cyg_current_time() + (PROBE_TIME / MSPTICK));
		if(at_iface.status == AT_RESET) return (0);	 //fail
	}
	at_iface.status = AT_STABLE;
	return (1);	//ok
}

int16 at_broadcast(struct at_addr *sat_addr)
{
	if ( sat_addr->s_node != ATADDR_BCAST ) return 0;

    if ( sat_addr->s_net == 0 ) {
		return 1;
	} else {
		//at this net range
		if(ntohs(sat_addr->s_net) >= at_iface.netrange.first &&
		   ntohs(sat_addr->s_net) <= at_iface.netrange.last ) {
			return 1;
		}
	}
	return 0;
}

void aarp_set_rand_node(uint8 *node)
{
	int16 i;
	uint8 item,temp;

	for(i = 0 ; i < NODE_PER_NET ; i++) node[i] = i+1;

	for(i = 0 ; i < NODE_PER_NET ; i++) {
		item = (uint8) urandom((char)EEPROM_Data.EthernetID[5])%252;
		temp = node[i];
		node[i] = node[item];
		node[item] = temp;
	}
}
