

/* Basic message buffer structure */
struct mbuf {
	struct mbuf *next;	/* Links mbufs belonging to single packets */
	struct mbuf *anext;	/* Links packets on queues */
	u32_t size;		/* Size of associated data buffer */
	int refcnt;		/* Reference count */
	struct mbuf *dup;	/* Pointer to duplicated mbuf */
	u8_t *data;		/* Active working pointers */
	u32_t cnt;
};

/* Interface control structure */
struct iface {
	struct iface *next;	/* Linked list pointer */
	char *name;		/* Ascii string with interface name */

	u32_t addr;		/* IP address */
	u32_t broadcast;	/* Broadcast address */
	u32_t netmask;		/* Network mask */

	u32_t mtu;		/* Maximum transmission unit size */

#ifndef CODE1
	struct mbuf *outq;	/* IP datagram transmission queue */
#endif  !CODE1

	int outlim;		/* Limit on outq length */
	int txbusy;		/* Transmitter is busy */

	/* Device dependent */
	int dev;		/* Subdevice number to pass to send */
					/* To device -- control */

	u8_t *hwaddr;		/* Device hardware address, if any */

	struct iftype * iftype;	/* Pointer to appropriate iftype entry */

				/* Routine to send an IP datagram */
	int (*send)(struct mbuf **,struct iface *,u32_t,u8_t);
			/* Encapsulate any link packet */
	int (*output)(struct iface *,const u8_t *,u8_t *,u32_t,struct mbuf **);
			/* Send raw packet */
	int (*raw)(struct iface *,struct mbuf **);
			/* Send raw packet */
	int (*raw2)(struct iface *,struct mbuf **);

	/* Counters */
	u32_t ipsndcnt; 	/* IP datagrams sent */
	u32_t rawsndcnt;	/* Raw packets sent */
	u32_t iprecvcnt;	/* IP datagrams received */
	u32_t rawrecvcnt;	/* Raw packets received */
	u32_t lastsent;		/* Clock time of last send */
	u32_t lastrecv;		/* Clock time of last receive */
};



/* Interface encapsulation mode table entry. An array of these structures
 * are initialized in config.c with all of the information necessary
 * to attach a device.
 */
struct iftype {
	char *name;		/* Name of encapsulation technique */
	int (*send)(struct mbuf **,struct iface *,u32_t,u8_t);
				/* Routine to send an IP datagram */
	int (*output)(struct iface *,const u8_t *,u8_t *,u32_t,struct mbuf **);
				/* Routine to send link packet */
	char *(*format)(char *,u8_t *);
				/* Function that formats addresses */
	int (*scan)(u8_t *,char *);
				/* Reverse of format */
	int type;		/* Type field for network process */
	int hwalen;		/* Length of hardware address, if any */
	void (*rcvf)(struct iface *,struct mbuf **);
				/* Function that handles incoming packets */
};

//extern struct iftype Iftypes[];



//struct iface *Ifaces;	/* Head of interface list */
//extern struct iface  Loopback;	/* Optional loopback interface */
//extern struct iface  Encap;	    /* IP-in-IP pseudo interface */
//struct netif *Lanface;   //add by Simon 12/1/97
//struct netif *WLanface;  // Charles 2001/08/22

/* Packet driver interface classes */
#define	CL_NONE			0
#define	CL_ETHERNET		1
#define	CL_PRONET_10	2
#define	CL_IEEE8025		3
#define	CL_OMNINET		4
#define	CL_APPLETALK	5
#define	CL_SERIAL_LINE	6
#define	CL_STARLAN		7
#define	CL_ARCNET		8
#define	CL_AX25			9
#define	CL_KISS			10
#define CL_IEEE8023		11
#define CL_FDDI 		12
#define CL_INTERNET_X25 13
#define CL_LANSTAR 		14
#define CL_SLFP 		15
#define	CL_NETROM		16
#define CL_PPP			17
#define	CL_QTSO			18
#define NCLASS			19



#define	EADDR_LEN	6
/* Format of an Ethernet header */
struct ether {
    u8_t dest[EADDR_LEN];
    u8_t source[EADDR_LEN];
    u16_t type;
}PACK_STRUCT_STRUCT;





