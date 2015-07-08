/*
 * Copyright (c) 1998-2000 Luigi Rizzo
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/net/bridge.h,v 1.4 2000/02/08 14:53:55 luigi Exp $
 */

extern int do_bridge;
/*
 * the hash table for bridge
 */
typedef struct hash_table {
    struct iface *name ;
    unsigned char etheraddr[6] ;
    unsigned short used ;
    unsigned short loops;
} bdg_hash_table ;

#define BDG_MAX_PORTS 4
extern u8_t bdg_addresses[6*BDG_MAX_PORTS];
extern int bdg_ports ;

/*
 * out of the 6 bytes, the last ones are more "variable". Since
 * we are on a little endian machine, we have to do some gimmick...
 */
#ifndef HASH_SIZE  
#define HASH_SIZE 2048  /* must be a power of 2 */
#endif
//#define HASH_SIZE 4096    /* must be a power of 2 */
#define HASH_FN(addr)   (	\
	ntohs( ((short *)addr)[1] ^ ((short *)addr)[2] ) & (HASH_SIZE -1))

extern void bdginit(void);
extern int bcomp(void *src, void *dst, int tsize);
#define bzero(p, l)         memset((char *) p, 0, l);
#define bcopy(s, d, l)      memcpy((char *) d,(char *) s, l);
/*
 * The following constants are not legal ifnet pointers, and are used
 * as return values from the classifier, bridge_dst_lookup()
 * The same values are used as index in the statistics arrays,
 * with BDG_FORWARD replacing specifically forwarded packets.
 */
#define BDG_BCAST	( (struct iface *)1 )
#define BDG_MCAST	( (struct iface *)2 )
#define BDG_LOCAL	( (struct iface *)3 )
#define BDG_DROP	( (struct iface *)4 )
#define BDG_UNKNOWN	( (struct iface *)5 )
#define BDG_IN		( (struct iface *)7 )
#define BDG_OUT		( (struct iface *)8 )
#define BDG_FORWARD	( (struct iface *)9 )

#define PF_BDG 3 /* XXX superhack */
/*
 * statistics, passed up with sysctl interface and ns -p bdg
 */

#define STAT_MAX (int)BDG_FORWARD
struct bdg_port_stat {
    char name[16];
    u32_t collisions;
    u32_t p_in[STAT_MAX+1];
} ;

struct bdg_stats {
    struct bdg_port_stat s[16];
} ;


/*
 * We need additional info for the bridge. The bdg_ifp2sc[] array
 * provides a pointer to this struct using the if_index.
 * bdg_softc has a backpointer to the struct iface, the bridge
 * flags, and a cluster (bridging occurs only between port of the
 * same cluster).
 */
struct bdg_softc {
    struct netif *ifp ;
    int flags ;
#define IFF_BDG_PROMISC 0x0001  /* set promisc mode on this if. */  
#define	IFF_MUTE        0x0002  /* mute this if for bridging.   */
#define	IFF_USED        0x0004  /* use this if for bridging.    */
    short cluster_id ; 			/* in network format */
    u32_t magic;
};

/* XXX make it static of size BDG_MAX_PORTS */

#define	USED(ifp)    (ifp2sc[ifp->dev].flags & IFF_USED)
#define	MUTED(ifp)   (ifp2sc[ifp->dev].flags & IFF_MUTE)
#define	MUTE(ifp)    (ifp2sc[ifp->dev].flags |= IFF_MUTE)
#define	UNMUTE(ifp)  (ifp2sc[ifp->dev].flags &= ~IFF_MUTE)
#define	CLUSTER(ifp) (ifp2sc[ifp->dev].cluster_id)
#define	IFP_CHK(ifp, x) \
	if (ifp2sc[ifp->dev].magic != 0xDEADBEEF) { x ; }

#define	SAMECLUSTER(ifp,src,eh)	(src == NULL || CLUSTER(ifp) == CLUSTER(src) )
#define BDG_STAT(ifp, type)     (bdg_stats.s[ifp->dev].p_in[(int)type]++)
