//===============================Bridge table===============================//
#include "arch/cc.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "netif/iface.h"
#include "bridge.h"

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

static bdg_hash_table bdg_table[HASH_SIZE];
static struct bdg_stats bdg_stats ;
static struct bdg_softc ifp2sc[BDG_MAX_PORTS];
static int bdg_loops;

#define BRIDGE_RELOCATION				3

void bdginit(void);
static void flush_table(void);
struct netif *bridge_in(struct netif *ifp, struct ether *hdr);
struct netif *bridge_dst_lookup(struct ether *hdr);
//===========================================================================//



//***************************ZOT  BRIDGE **********************************************

UINT32 zot_get16(UINT8 *cp)
{
	UINT32 x;

	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
}

/* Machine-independent, alignment insensitive network-to-host long conversion */
UINT32 zot_get32(UINT8 *cp)
{
	UINT32 rval;

	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;

	return rval;
}

#define BDG_MATCH(a,b) ( \
    zot_get16((unsigned short *)a+2) == zot_get16((unsigned short *)b+2) && \
    zot_get32((unsigned int *)a) == zot_get32((unsigned int *)b) )
#define IS_ETHER_BROADCAST(a) ( \
	zot_get32((unsigned int *)a) == 0xffffffff && \
	zot_get16((unsigned short *)a+2) == 0xffff )


/*
 * completely flush the bridge table.
 */
static void
flush_table()
{	
	int i;

	for (i=0; i< HASH_SIZE; i++)
		bdg_table[i].name= NULL; /* clear table */
}


/*
 * local MAC addresses are held in a small array. This makes comparisons
 * much faster.
 */

unsigned char bdg_addresses[6*BDG_MAX_PORTS];
int bdg_ports ;
/*
 * initialization of bridge code. This needs to be done after all
 * interfaces have been configured.
 */
void
bdginit(void)
{
	struct netif *ifp;
	UINT8 *eth_addr ;
	struct bdg_softc *bp;

	/*initial Bridge Function */

	memset(bdg_table, 0, HASH_SIZE * sizeof(struct hash_table));
	flush_table();

	bzero(ifp2sc, BDG_MAX_PORTS * sizeof(struct bdg_softc));
	bzero(&bdg_stats, sizeof(bdg_stats) );
	bdg_ports = 0 ;
	eth_addr = bdg_addresses ;

//	for(ifp = Ifaces; ifp != NULL;ifp = ifp->next)
	for(ifp = netif_list; ifp != NULL;ifp = ifp->next)
	{
//		if(ifp->iftype->type==CL_ETHERNET){
			bp = &ifp2sc[ifp->dev];
			bcopy(ifp->hwaddr, eth_addr, 6);
			eth_addr += 6 ;
			bp->ifp = ifp ;
			bp->flags = 0 ;
			bp->cluster_id = 0;
			bp->magic = 0xDEADBEEF ;

//			sprintf(bdg_stats.s[ifp->dev].name,"%s%d:%d", ifp->name, ifp->dev,	ntohs(bp->cluster_id));
			//sprintf(bdg_stats.s[ifp->dev].name,"%s%d:", ifp->name, ifp->dev);
			bdg_ports++;
//		}    
	}
	
}


/*
 * bridge_in() is invoked to perform bridging decision on input packets.
 * On Input:
 *	 m		packet to be bridged. The mbuf need not to hold the
 *		whole packet, only the first 14 bytes suffice. We
 *		assume them to be contiguous. No alignment assumptions
 *		because they are not a problem on i386 class machines.
 *
 * On Return: destination of packet, one of
 *	 BDG_BCAST	broadcast
 *	 BDG_MCAST	multicast
 *	 BDG_LOCAL	is only for a local address (do not forward)
 *	 BDG_DROP	drop the packet
 *	 ifp	ifp of the destination interface.
 *
 * Forwarding is not done directly to give a chance to some drivers
 * to fetch more of the packet, or simply drop it completely.
 */
static int bdg_ipfw_colls;

struct netif *
bridge_in(struct netif *ifp, struct ether *eh)
{
	struct netif *old,*dst ;
	int b_index;
	int dropit = MUTED(ifp) ;

	b_index = HASH_FN(eh->source);
	bdg_table[b_index].used = 1 ;
	old = bdg_table[b_index].name ;

	if ( old ) { /* the entry is valid. */
		if (!BDG_MATCH( eh->source, bdg_table[b_index].etheraddr) ) {
			bdg_ipfw_colls++ ;
			bdg_table[b_index].name = NULL ;
			bdg_table[b_index].loops = 0;
		} else if (old != ifp) {
			/*
			 * found a loop. Either a machine has moved, or there
			 * is a misconfiguration/reconfiguration of the network.
			 * First, do not forward this packet!
			 * Record the relocation anyways; then, if loops persist,
			 * suspect a reconfiguration and disable forwarding
			 * from the old interface.
			 */

			bdg_table[b_index].loops++;

			if( bdg_table[b_index].loops <= BRIDGE_RELOCATION )
				return BDG_DROP;

			bdg_table[b_index].name = ifp; /* relocate address */
			dropit = 1 ;

		}
		else
			bdg_table[b_index].loops = 0;
	}
	else
		bdg_table[b_index].loops = 0;

	/*
	 * now write the source address into the table
	 */
	if (bdg_table[b_index].name == NULL) {
		memcpy(bdg_table[b_index].etheraddr, eh->source, 6);
		bdg_table[b_index].name = ifp ;
	}
	dst = bridge_dst_lookup(eh);
	/* Return values:
	 *	 BDG_BCAST, BDG_MCAST, BDG_LOCAL, BDG_UNKNOWN, BDG_DROP, ifp.
	 * For muted interfaces, the first 3 are changed in BDG_LOCAL,
	 * and others to BDG_DROP. Also, for incoming packets, ifp is changed
	 * to BDG_DROP in case ifp == src . These mods are not necessary
	 * for outgoing packets from ether_output().
	 */
	BDG_STAT(ifp, BDG_IN);
	switch ((int)dst)
	{
	case (int)BDG_BCAST:
	case (int)BDG_MCAST:
	case (int)BDG_LOCAL:
	case (int)BDG_UNKNOWN:
	case (int)BDG_DROP:
		BDG_STAT(ifp, dst);
		break ;
	default :
		if (dst == ifp || dropit )
			BDG_STAT(ifp, BDG_DROP);
		else
			BDG_STAT(ifp, BDG_FORWARD);
		break ;
	}

	if ( dropit ) {
		if (dst == BDG_BCAST || dst == BDG_MCAST || dst == BDG_LOCAL)
			return BDG_LOCAL ;
		else
			return BDG_DROP ;
	} else {
		return (dst == ifp ? BDG_DROP : dst ) ;
	}
}

/*
 * Find the right pkt destination:
 *	BDG_BCAST	is a broadcast
 *	BDG_MCAST	is a multicast
 *	BDG_LOCAL	is for a local address
 *	BDG_DROP	must be dropped
 *	other		ifp of the dest. interface (incl.self)
 */

struct netif *
bridge_dst_lookup(struct ether *eh)
{
	int b_index ;
	UINT8 *eth_addr = bdg_addresses ;
	struct netif *dst;

	if (IS_ETHER_BROADCAST(eh->dest))
		return BDG_BCAST ;
	if (eh->dest[0] & 1)
		return BDG_MCAST ;
	/*
	 * Lookup local addresses in case one matches.
	 */
	for (b_index = bdg_ports, eth_addr=bdg_addresses; b_index ; b_index--, eth_addr += 6)
		if (BDG_MATCH(eth_addr, eh->dest))
			return BDG_LOCAL ;
	/*
	 * Look for a possible destination in table
	 */
	b_index= HASH_FN( eh->dest );
	dst = bdg_table[b_index].name;
	if (dst && BDG_MATCH( bdg_table[b_index].etheraddr, eh->dest))
		return dst ;
	else
		return BDG_UNKNOWN ;
}








//***************************ZOT  BRIDGE **********************************************