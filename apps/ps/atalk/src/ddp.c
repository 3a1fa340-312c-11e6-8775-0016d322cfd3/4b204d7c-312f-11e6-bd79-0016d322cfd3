#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "apple_mbuf.h"
#include "socket2.h"
#include "atalkd.h"

#define	RUNT		60	/* smallest legal size packet, no fcs */
#define	EADDR_LEN	6

static uint8 at_org_code[ 3 ] = {
  0x08, 0x00, 0x07
};

//eCos	static uint8 atPacket[sizeof(struct EtherSNAP)+sizeof(struct ddpehdr)+DDP_MAXSZ];

// DDP control structures list
struct ddp_cb *Ddps;

static struct ddp_cb *ddp_lookup(uint8 atport);
static int atIsMyAddr(struct at_addr *dest);
static uint16 at_cksum(struct ambuf *bp,int16 SkipSize);
static void at_gateway(struct at_addr *dest);

extern WORD MyIntNO;	//615wu //send_pkt


//Create a DDP control block for lsocket, so that we can queue
// incoming datagrams.
struct ddp_cb * open_ddp(uint8 atport)
{
	struct ddp_cb *up;
	unsigned int prot = 0;

	prot = dirps();	//eCos

	if((up = ddp_lookup(atport)) != NULL){
		restore(prot);	//eCos
		return NULL;
	}
	
	up = (struct ddp_cb *)malloc(sizeof(struct ddp_cb));
	if (up ==NULL){		//eCos
		restore(prot);	//eCos
		return NULL;
	}
	memset(up, 0x00, sizeof(struct ddp_cb));
	up->atport = atport;

	up->next = Ddps;
	Ddps = up;

	restore(prot);	//eCos

	return up;
}

// Send a DDP datagram
char send_ddp_buffer[2048];
int
send_ddp(
struct at_socket *lsocket,	// Source socket
struct at_socket *fsocket,	// Destination socket
struct ambuf **bpp		    // Data field, if any
){
	struct ddpehdr deh;
	uint16 datalen;
	unsigned int prot = 0;
	
	if(bpp == NULL)	return -1;
	
	prot = dirps();	//eCos	
	
	datalen = (*bpp)->cnt;

//	if(lsocket->addr.s_net == ATADDR_ANYNET &&
//	   lsocket->addr.s_node == ATADDR_ANYNODE)
//	{
		lsocket->addr.s_net  = at_iface.my.s_net;
		lsocket->addr.s_node = at_iface.my.s_node;
//	}

//	if(lsocket->port == ATADDR_ANYPORT) lsocket->port = GetATport();

//DDP header
	deh.deh_dnet  = fsocket->addr.s_net;
	deh.deh_dnode = fsocket->addr.s_node;
	deh.deh_dport = fsocket->port;
	deh.deh_snet  = lsocket->addr.s_net;
	deh.deh_snode = lsocket->addr.s_node;
	deh.deh_sport = lsocket->port;
	deh.deh_pad   = 0;
	deh.deh_hops  = 0;
	deh.deh_len   = datalen + sizeof( struct ddpehdr );
	deh.deh_sum   = 0;
	deh.deh_bytes = ntohl( deh.deh_bytes );

	if (pushdown2(bpp,&deh,sizeof(struct ddpehdr), send_ddp_buffer) == 1){
		free_p(bpp);	//615wu
		restore(prot);	//eCos
		return -1;
	}

#ifdef ATALKD_CHECKSUM
	//DDP checksum
//	if(ddp_cksum) {

	NSET16( (*bpp)->data+2, htons(at_cksum(*bpp,4)) );	
//	}
#endif ATALKD_CHECKSUM

	ddp_output(*bpp,&fsocket->addr);

	restore(prot);	//eCos

	return (int)datalen;
}

char recv_ddp_buffer[2048];
// Accept a waiting datagram, if available. Returns length of datagram
int recv_ddp(
struct ddp_cb *up,
struct at_socket *fsocket,	// Place to stash incoming socket
struct ambuf **bp			// Place to stash data packet
)
{
	struct at_socket sp;
	struct ambuf *buf;
	uint16 length = 0;
	unsigned int prot = 0;

	if(up == NULL || up->rcvcnt == 0) return -1;

	prot = dirps();	//eCos
	buf = dequeue2(&up->rcvq);

	if (buf == NULL){	//615wu
		restore(prot);	//eCos
		return -1;
	}

	up->rcvcnt--;
	
	// Strip socket header
	if(pullup2(&buf,&sp,sizeof(struct at_socket), recv_ddp_buffer) == 0){
		free_p(&buf);	//615wu
		restore(prot);	//eCos	
		return -1;
	}

	// Fill in the user's foreign socket structure, if given
	if(fsocket != NULL){
		*fsocket = sp;
	}
	// Hand data to user	
	if (buf != NULL){
		
		length = buf->cnt;
		
		if(bp != NULL)
			*bp = buf;
		else{
			free_p(&buf);	
			restore(prot);	//eCos	
			return -1;	
		}
	
	restore(prot);	//eCos	
	return (int)length;
	}else{
		restore(prot);	//eCos	
		return -1;
	}
}

// Delete a DDP control block
int del_ddp(struct ddp_cb *conn)
{	
	struct ambuf *bp;
	struct ddp_cb *up;
	struct ddp_cb *ddplast = NULL;
	unsigned int prot = 0;

	prot = dirps();	//eCos

	for(up = Ddps;up != NULL;ddplast = up,up = up->next){
		if(up == conn) break;
	}
	if(up == NULL){
		// Either conn was NULL or not found on list	
	    restore(prot);	//eCos
		return -1;
	}	
	
	// Get rid of any pending packets
	while(up->rcvcnt != 0){
		bp = up->rcvq;
		up->rcvq = up->rcvq->anext;
		free_p(&bp);			
		//for changeing rcvcnt don't change thread 
		up->rcvcnt--;
	}
	// Remove from list
	if(ddplast != NULL)
		ddplast->next = up->next;
	else
		Ddps = up->next;	/* was first on list */

	free(up);
	
	restore(prot);	//eCos
	
	return 0;
}


void ddp_input2ambf(char *data,unsigned int len)
{
	struct ambuf *bpp;
		
    if (!ATD_INIT_OK)
        return;
		
	if((data == NULL) || (len <= 0) || (len > AMbufSize))
		return;	//615wu
		
	bpp = malloc(sizeof(struct ambuf)); 
	if (bpp == NULL)
		return;
	memset(bpp, 0x00, sizeof(struct ambuf));
	
	memcpy(bpp->data, data, len);
	bpp->size = AMbufSize;
	bpp->cnt = len;
	
	ddp_input(&bpp);
	return;
}
char ddp_input_buffer[2048];	
void ddp_input(struct ambuf **bpp)
{
	struct at_addr  to;
	struct at_socket from;
	struct ddpehdr	*deh;
	struct ddpehdr  ddpe;
	struct ddp_cb   *up;
	int16           dlen, mlen;
	uint16          cksum;
	unsigned int prot = 0;
	
	if(bpp == NULL || *bpp == NULL) return;

	if ((*bpp)->cnt > AMbufSize){	//615wu
		free_p(bpp);
		return;		
	}
	if ( (*bpp)->cnt < sizeof( struct ddpehdr ) ) {	
		free_p(bpp);
		return;
	}

	deh = mtod(*bpp, struct ddpehdr *);
	NSET32( &deh->deh_bytes, ntohl( NGET32(&deh->deh_bytes) ) );
	dlen = deh->deh_len;
	cksum = deh->deh_sum;

	to.s_net  = NGET16(&deh->deh_dnet);
	to.s_node = deh->deh_dnode;

	if(!atIsMyAddr(&to)) {
		free_p(bpp);
		return;
	}

	//*
	//* Adjust the length, removing any padding that may have been added
	//* at a link layer.  We do this before we attempt to forward a packet,
	//* possibly on a different media.
	//*

	if( (*bpp)->cnt < dlen ) {
		free_p(bpp);
		return;
	}

	prot = dirps();	//eCos	

	if (( up = ddp_lookup(deh->deh_dport)) == NULL ) {
		free_p(bpp);	
		restore(prot);	//eCos
		return;
	}
	if( (*bpp)->cnt > dlen) {
		if(trim_mbuf2(bpp,dlen) == -1){
			free_p(bpp);	
			restore(prot);	//eCos
			return;	
		}
	}

#ifdef ATALKD_CHECKSUM
	if (ddp_cksum && cksum && cksum != at_cksum(*bpp,4) ) {
		free_p(bpp);
		restore(prot);	//eCos		
		return;
	}
#endif ATALKD_CHECKSUM	
	
	from.addr.s_net  = NGET16(&deh->deh_snet);
	from.addr.s_node = deh->deh_snode;
	from.port        = deh->deh_sport;

	if (pullup2(bpp,&ddpe, sizeof( struct ddpehdr ), ddp_input_buffer) == 0){  //remove ddp header
		free_p(bpp);	
		restore(prot);	//eCos	
		return;	
	}

	if (pushdown2(bpp,&from,sizeof(from), ddp_input_buffer) == 1){		      //put	"from" socket
		free_p(bpp);	
		restore(prot);	//eCos	
		return;	
	}

	// Queue it
	if ((*bpp)!= NULL){
		up->rcvcnt++;	
		enqueue2(&up->rcvq,bpp);
		cyg_semaphore_post(&((itop(up->user))->sem_f));
	}
	
	restore(prot);	//eCos	
	return;
}

int ddp_output(struct ambuf *bp,struct at_addr *dest)
{
	struct EtherSNAP *ethdr;
	uint16 size;
	uint8  *data;	
	struct ambuf *tmpbp = bp;
	uint8 *atPacket;	//615wu

	atPacket = malloc(sizeof(struct EtherSNAP)+sizeof(struct ddpehdr)+DDP_MAXSZ);	//eCos
	if (atPacket == NULL){
		free_p(&bp);
		return 0;
	}
	
	ethdr = (struct EtherSNAP *) atPacket;
//eCos	ethdr = (struct EtherSNAP *) atPacket;
	at_gateway(dest); //if need gateway the dest's address will be changed
	if(aarpresolve(bp,dest,ethdr->dest) == 0) {
		free(atPacket);
		return 0;
	}

	data =atPacket + sizeof(struct EtherSNAP);

	size = 0; //DDP Datagram length
	if(tmpbp != NULL){
		memcpy(data,tmpbp->data,tmpbp->cnt);
		data += tmpbp->cnt;
		size += tmpbp->cnt;
	}
	//----------- Ethernet SNAP Frame --------------------------------
	memcpy(ethdr->src,MyPhysNodeAddress,EADDR_LEN);
	ethdr->length   = ntohs(size + 8);  //SNAP HEAD SIZE = 8
	ethdr->llc_dsap = ethdr->llc_ssap = LLC_SNAP_LSAP;	//--+
	ethdr->control  = LLC_UI;							// SNAP
	memcpy(ethdr->org_code, at_org_code, 3);			// Header
	ethdr->ether_type = htons( ETHERTYPE_AT );			//--+
	//-----------------------------------------------------------------

	//Total packet size	 (DDP datagram length + Ethernet header)
	if( (size += sizeof(struct EtherSNAP)) < RUNT) size = RUNT;

	free_p(&bp);		
	send_pkt(MyIntNO,(uint8 *)atPacket,size);	//eCos
	free(atPacket);
	
	return 1;
}

//Look up DDP socket.
//Return control block pointer or NULL if nonexistant
//As side effect, move control block to top of list to speed future
//searches.
//
struct ddp_cb *ddp_lookup(uint8 atport)
{
	struct ddp_cb *up;
	struct ddp_cb *uplast = NULL;

	for(up = Ddps;up != NULL;uplast = up,up = up->next){
		if(atport == up->atport) {
			if(uplast != NULL){
				// Move to top of list
				uplast->next = up->next;
				up->next = Ddps;
				Ddps = up;
			}
			return up;
		}
	}
	
	return NULL;
}

int atIsMyAddr(struct at_addr *dest)
{
	uint16 net;

	net = dest->s_net;

	if(at_iface.my.s_node != dest->s_node &&
	   dest->s_node != ATADDR_BCAST)   return 0;

	if(net == 0 || net == at_iface.my.s_net) return 1;

	return 0;
}

uint8 GetATport(void)
{
	static uint8 CurATport = ATPORT_RESERVED-1;
	uint8 LastATport = CurATport;
	struct ddp_cb *up;

	cyg_scheduler_lock();	//615wu

	do {
		if(++CurATport >= ATPORT_LAST) CurATport = ATPORT_RESERVED;
		for(up = Ddps;up != NULL;up = up->next){
			if(CurATport == up->atport) break;
		}
	} while(LastATport != CurATport && up != NULL);

	if(LastATport == CurATport) {
#ifdef _PC
		printf("GetATport() error !\n");
#endif _PC

		cyg_scheduler_unlock();	//615wu

	    return (0);
	}

	cyg_scheduler_unlock();	//615wu

	return CurATport;
}

#ifdef ATALKD_CHECKSUM
uint16 at_cksum(struct ambuf *bp,int16 SkipSize )
{
	uint32	cksum;
	int16   i,cnt;
	uint8   *data;

	for(cksum = 0;bp != NULL; bp = bp->next) {
		data = bp->data;
		cnt  = bp->cnt;
		for (i = 0 ; i < cnt; i++) {
			if(SkipSize) {
				SkipSize--;
				data++;
				continue;
			}
			cksum = (cksum + *(data++)) << 1;
			if ( cksum & 0x00010000 ) {
				cksum++;
			}
			cksum &= 0x0000ffff;
		}
	}

	if (cksum == 0) cksum = 0x0000ffff;

	return((uint16)cksum);
}
#endif ATALKD_CHECKSUM

void at_gateway(struct at_addr *dest)
{
	if (dest->s_net == 0) return;

	if(ntohs(dest->s_net) >= at_iface.netrange.first &&
	   ntohs(dest->s_net) <= at_iface.netrange.last )
	{
		return;
	}
	if(at_iface.gate.s_net != 0 || at_iface.gate.s_node != 0) {
		dest->s_net  = at_iface.gate.s_net;
		dest->s_node = at_iface.gate.s_node;
	}
}
