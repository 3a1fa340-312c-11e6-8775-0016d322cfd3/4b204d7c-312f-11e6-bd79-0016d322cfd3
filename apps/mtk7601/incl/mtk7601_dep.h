#ifndef _MTK7601_DEP_H_
#define _MTK7601_DEP_H_

/* os depend struct and function */
#define MODULE_INFO(tag, info)
#define MODULE_PARM(a, b)
#define MODULE_LICENSE(_license) MODULE_INFO(_license, _license)
#define MODULE_AUTHOR(_author) MODULE_INFO(_author, _author)
#define MODULE_DESCRIPTION(_description) MODULE_INFO(_description, _description)
#define MODULE_PARM_DESC(_parm, desc) MODULE_INFO(_parm, desc)

#define MOD_INC_USE_COUNT do{} while(0)
#define MOD_DEC_USE_COUNT do{} while(0)
#define in_interrupt()  0
#define MEM_ALLOC_FLAG  0
#define LINUX_VERSION_CODE      4
#define WIRELESS_EXT            19
#define __user
#ifndef IN
#define IN
#endif /* IN */
#ifndef OUT
#define OUT
#endif /* OUT */

/* Packet types */
#define PACKET_HOST             0               /* To us                */
#define PACKET_BROADCAST        1               /* To all               */
#define PACKET_MULTICAST        2               /* To group             */
#define PACKET_OTHERHOST        3               /* To someone else      */
#define PACKET_OUTGOING         4               /* Outgoing of any type */
#define PACKET_LOOPBACK         5               /* MC/BRD frame looped back */
#define PACKET_USER             6               /* To user space        */
#define PACKET_KERNEL           7               /* To kernel space      */

/* Don't change this without changing skb_csum_unnecessary! */
#define CHECKSUM_NONE           0
#define CHECKSUM_UNNECESSARY    1
#define CHECKSUM_COMPLETE       2
#define CHECKSUM_PARTIAL        3

#define SIGHUP           1
#define SIGINT           2
#define SIGQUIT          3
#define SIGILL           4
#define SIGTRAP          5
#define SIGABRT          6
#define SIGIOT           6
#define SIGBUS           7
#define SIGFPE           8
#define SIGKILL          9
#define SIGUSR1         10
#define SIGSEGV         11
#define SIGUSR2         12
#define SIGPIPE         13
#define SIGALRM         14
#define SIGTERM         15
#define SIGSTKFLT       16
#define SIGCHLD         17
#define SIGCONT         18
#define SIGSTOP         19
#define SIGTSTP         20
#define SIGTTIN         21
#define SIGTTOU         22
#define SIGURG          23
#define SIGXCPU         24
#define SIGXFSZ         25
#define SIGVTALRM       26
#define SIGPROF         27
#define SIGWINCH        28
#define SIGIO           29

#define IWEVCUSTOM      0x8C02          /* Driver specific ascii string */

#define KERN_SOH        "\001"          /* ASCII Start Of Header */
#define KERN_SOH_ASCII  '\001'
#define KERN_EMERG      KERN_SOH ""    /* system is unusable */
#define KERN_ALERT      KERN_SOH "1"    /* action must be taken immediately */
#define KERN_CRIT       KERN_SOH "2"    /* critical conditions */
#define KERN_ERR        KERN_SOH "3"    /* error conditions */
#define KERN_WARNING    KERN_SOH "4"    /* warning conditions */
#define KERN_NOTICE     KERN_SOH "5"    /* normal but significant condition */
#define KERN_INFO       KERN_SOH "6"    /* informational */
#define KERN_DEBUG      KERN_SOH "7"    /* debug-level messages */
#define KERN_DEFAULT    KERN_SOH "d"    /* the default kernel loglevel */

#define IFNAMSIZ        16
#define ether_setup     NULL

#define __dev_alloc_skb(a, b)   dev_alloc_skb(a)
#define KERNEL_VERSION(a,b,c) b

enum net_device_flags {
IFF_UP                          = 1<<0,  /* sysfs */
IFF_BROADCAST                   = 1<<1,  /* volatile */
IFF_DEBUG                       = 1<<2,  /* sysfs */
IFF_LOOPBACK                    = 1<<3,  /* volatile */
IFF_POINTOPOINT                 = 1<<4,  /* volatile */
IFF_NOTRAILERS                  = 1<<5,  /* sysfs */
IFF_RUNNING                     = 1<<6,  /* volatile */
IFF_NOARP                       = 1<<7,  /* sysfs */
IFF_PROMISC                     = 1<<8,  /* sysfs */
IFF_ALLMULTI                    = 1<<9,  /* sysfs */
IFF_MASTER                      = 1<<10, /* volatile */
IFF_SLAVE                       = 1<<11, /* volatile */
IFF_MULTICAST                   = 1<<12, /* sysfs */
IFF_PORTSEL                     = 1<<13, /* sysfs */
IFF_AUTOMEDIA                   = 1<<14, /* sysfs */
IFF_DYNAMIC                     = 1<<15, /* sysfs */
IFF_LOWER_UP                    = 1<<16, /* volatile */
IFF_DORMANT                     = 1<<17, /* volatile */
IFF_ECHO                        = 1<<18, /* volatile */
};

#define IFF_UP                          IFF_UP
#define IFF_BROADCAST                   IFF_BROADCAST
#define IFF_DEBUG                       IFF_DEBUG
#define IFF_LOOPBACK                    IFF_LOOPBACK
#define IFF_POINTOPOINT                 IFF_POINTOPOINT
#define IFF_NOTRAILERS                  IFF_NOTRAILERS
#define IFF_RUNNING                     IFF_RUNNING
#define IFF_NOARP                       IFF_NOARP
#define IFF_PROMISC                     IFF_PROMISC
#define IFF_ALLMULTI                    IFF_ALLMULTI
#define IFF_MASTER                      IFF_MASTER
#define IFF_SLAVE                       IFF_SLAVE
#define IFF_MULTICAST                   IFF_MULTICAST
#define IFF_PORTSEL                     IFF_PORTSEL
#define IFF_AUTOMEDIA                   IFF_AUTOMEDIA
#define IFF_DYNAMIC                     IFF_DYNAMIC
#define IFF_LOWER_UP                    IFF_LOWER_UP
#define IFF_DORMANT                     IFF_DORMANT
#define IFF_ECHO                        IFF_ECHO

#define IFF_VOLATILE    (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST|IFF_ECHO|\
                        IFF_MASTER|IFF_SLAVE|IFF_RUNNING|IFF_LOWER_UP|IFF_DORMANT)

#define IF_GET_IFACE    0x0001          /* for querying only */
#define IF_GET_PROTO    0x0002
/* For definitions see hdlc.h */
#define IF_IFACE_V35    0x1000          /* V.35 serial interface        */
#define IF_IFACE_V24    0x1001          /* V.24 serial interface        */
#define IF_IFACE_X21    0x1002          /* X.21 serial interface        */
#define IF_IFACE_T1     0x1003          /* T1 telco serial interface    */
#define IF_IFACE_E1     0x1004          /* E1 telco serial interface    */
#define IF_IFACE_SYNC_SERIAL 0x1005     /* can't be set by software     */
#define IF_IFACE_X21D   0x1006          /* X.21 Dual Clocking (FarSite) */
/* For definitions see hdlc.h */
#define IF_PROTO_HDLC   0x2000          /* raw HDLC protocol            */
#define IF_PROTO_PPP    0x2001          /* PPP protocol                 */
#define IF_PROTO_CISCO  0x2002          /* Cisco HDLC protocol          */
#define IF_PROTO_FR     0x2003          /* Frame Relay protocol         */
#define IF_PROTO_FR_ADD_PVC 0x2004      /*    Create FR PVC             */
#define IF_PROTO_FR_DEL_PVC 0x2005      /*    Delete FR PVC             */
#define IF_PROTO_X25    0x2006          /* X.25                         */
#define IF_PROTO_HDLC_ETH 0x2007        /* raw HDLC, Ethernet emulation */
#define IF_PROTO_FR_ADD_ETH_PVC 0x2008  /*  Create FR Ethernet-bridged PVC */
#define IF_PROTO_FR_DEL_ETH_PVC 0x2009  /*  Delete FR Ethernet-bridged PVC */
#define IF_PROTO_FR_PVC 0x200A          /* for reading PVC status       */
#define IF_PROTO_FR_ETH_PVC 0x200B
#define IF_PROTO_RAW    0x200C          /* RAW Socket                   */

#define ARPHRD_IEEE80211_PRISM 802      /* IEEE 802.11 + Prism2 header  */
/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM   0               /* from KA9Q: NET/ROM pseudo    */
#define ARPHRD_ETHER    1               /* Ethernet 10Mbps              */
#define ETH_ALEN        6               /* Octets in one ethernet addr   */

#define SIOCSIFHWADDR   0x8924          /* set hardware address         */
#define SIOCGIFHWADDR   0x8927          /* Get hardware address         */
#define SIOCETHTOOL     0x8946          /* Ethtool interface            */

#define VERIFY_READ     0
#define VERIFY_WRITE    1

#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#define ETH_P_IPV6      0x86DD          /* IPv6 over bluebook           */
#define O_RDONLY        00000000
#define O_WRONLY        00000001
#define O_CREAT         00000100        /* not fcntl */

#define O_TRUNC         0x0200          /* not fcntl */

#include "module_config.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "rtmp_type.h"
#include "list.h"
#include "usb.h"
#include "wlanif.h"
#include "eeprom.h"
#include "lwip/netif.h"
#include "network.h"
#include "cyg/kernel/kapi.h"
//#include "iw_handler.h"


#define __init
#define __exit
#define atomic_t unsigned long
#define atomic_read(v) (*v)
typedef unsigned int dma_addr_t;
typedef unsigned int u_int32_t;
typedef unsigned short __be16;
typedef short       __s16;
typedef int         __s32;    


#define likely(x) (x)
#define __cpu_to_le16(x) ((__u16)(x))

#define typecheck(type,x) \
({      type __dummy; \
        typeof(x) __dummy2; \
        (void)(&__dummy == &__dummy2); \
        1; \
})

#define time_after(a,b)         \
        (typecheck(unsigned long, a) && \
        typecheck(unsigned long, b) && \
        ((long)((b) - (a)) < 0))

#define vmalloc(x) aligned_alloc(x, 16)
#define vfree(x)   aligned_free(x)

static inline long access_ok(int type, const void __user * addr,
                             unsigned long size)
{
    return 1;
};

#define get_unaligned(ptr) ((typeof(*(ptr)))({          \
    typeof(*(ptr)) v;                                   \
    unsigned char *s = (unsigned char*)(ptr);           \
    unsigned char *d = (unsigned char*)&v;              \
    switch (sizeof(v)) {                                \
        case 8: *d++ = *s++;                            \
                *d++ = *s++;                            \
                *d++ = *s++;                            \
                *d++ = *s++;                            \
        case 4: *d++ = *s++;                            \
                *d++ = *s++;                            \
        case 2: *d++ = *s++;                            \
        case 1: *d++ = *s++;                            \
            break;                                      \
        default:                                        \
            break;                                      \
    }                                                   \
    v; }))

struct __wait_queue_head {
    spinlock_t              lock;
    cyg_sem_t               semaphore;
    struct list_head        task_list;
    int                     test;
};

typedef struct __wait_queue_head wait_queue_head_t;

struct net_device_stats {
};

typedef struct {
    unsigned long seg;
} mm_segment_t;

typedef long long       __kernel_loff_t;
typedef __kernel_loff_t         loff_t;
typedef long                    off_t;
typedef unsigned char           u_char;
typedef unsigned  gfp_t;

#if 0
/*
 * Meta data about the request passed to the iw_handler.
 * Most handlers can safely ignore what's in there.
 * The 'cmd' field might come handy if you want to use the same handler
 * for multiple command...
 * This struct is also my long term insurance. I can add new fields here
 * without breaking the prototype of iw_handler...
 */
struct iw_request_info {
        __u16           cmd;            /* Wireless Extension command */
        __u16           flags;          /* More to come ;-) */
};

/*
 * This is how a function handling a Wireless Extension should look
 * like (both get and set, standard and private).
 */
typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra);

/*
 * This define all the handler that the driver export.
 * As you need only one per driver type, please use a static const
 * shared by all driver instances... Same for the members...
 * This will be linked from net_device in <linux/netdevice.h>
 */
struct iw_handler_def {
        /* Array of handlers for standard ioctls
         * We will call dev->wireless_handlers->standard[ioctl - SIOCIWFIRST]
         */
        const iw_handler *      standard;
        /* Number of handlers defined (more precisely, index of the
         * last defined handler + 1) */
        __u16                   num_standard;

#ifdef CONFIG_WEXT_PRIV
        __u16                   num_private;
        /* Number of private arg description */
        __u16                   num_private_args;
        /* Array of handlers for private ioctls
         * Will call dev->wireless_handlers->private[ioctl - SIOCIWFIRSTPRIV]
         */
        const iw_handler *      private;
 
        /* Arguments of private handler. This one is just a list, so you
         * can put it in any order you want and should not leave holes...
         * We will automatically export that to user space... */
        const struct iw_priv_args *     private_args;
#endif
 
        /* New location of get_wireless_stats, to de-bloat struct net_device.
         * The old pointer in struct net_device will be gradually phased
         * out, and drivers are encouraged to use this one... */
        struct iw_statistics*   (*get_wireless_stats)(struct net_device *dev);
};
#endif

struct net_device {
    char            name[IFNAMSIZ];
    void*           priv;
    const struct    iw_handler_def *   wireless_handlers;
    unsigned char   dev_addr[8];
    unsigned int    flags;
    unsigned short  type;
};

struct net_device_stats {
    unsigned long   rx_packets;
    unsigned long   tx_packets;
    unsigned long   rx_bytes;
    unsigned long   tx_bytes;
    unsigned long   rx_errors;
    unsigned long   tx_errors;
    unsigned long   rx_dropped;
    unsigned long   tx_dropped;
    unsigned long   multicast;
    unsigned long   collisions;
    unsigned long   rx_length_errors;
    unsigned long   rx_over_errors;
    unsigned long   rx_crc_errors;
    unsigned long   rx_frame_errors;
    unsigned long   rx_fifo_errors;
    unsigned long   rx_missed_errors;
    unsigned long   tx_aborted_errors;
    unsigned long   tx_carrier_errors;
    unsigned long   tx_fifo_errors;
    unsigned long   tx_heartbeat_errors;
    unsigned long   tx_window_errors;
    unsigned long   rx_compressed;
    unsigned long   tx_compressed;
};

typedef struct __wait_queue wait_queue_t;

typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int flags, void *key);

struct __wait_queue {
    unsigned int            flags;
    void                    *private;
    wait_queue_func_t       func;
    struct list_head        task_list;
};


//static inline void init_waitqueue_entry(wait_queue_t *q, struct task_struct *p)
static inline void init_waitqueue_entry(wait_queue_t *q, void *p)
{
    q->flags        = 0;
    q->private      = p;
    /*
    q->func         = default_wake_function;
    */
}

typedef unsigned char *sk_buff_data_t;

struct completion {
    unsigned int done;
    wait_queue_head_t wait;
};

struct tasklet_struct
{
	struct tasklet_struct *next;
	unsigned long state;
	atomic_t count;
	void (*func)(unsigned long);
	unsigned long data;
};

struct timer_list {
	struct list_head entry;
	unsigned long expires;
	unsigned long magic;
	void (*function)(unsigned long);
	unsigned long data;
	struct timer_base_s *base;
};

struct sk_buff {
	struct sk_buff *next;		/* Links mbufs belonging to single packets */
	struct sk_buff *prev;		/* Links packets on queues */
	uint size;				    /* Size of associated data buffer */
	int refcnt;				    /* Reference count */
	struct sk_buff_head * list; /* List we are on				*/
	uint8 *data;			    /* Active working pointers */
	uint8 *original_data;
	uint len;

    __u8                    __pkt_type_offset[0];
    __u8                    pkt_type:3;
    __u8                    pfmemalloc:1;
    __u8                    ignore_df:1;

    __u8                    nf_trace:1;
    __u8                    ip_summed:2;
    __u8                    ooo_okay:1;
    __u8                    l4_hash:1;
    __u8                    sw_hash:1;
    __u8                    wifi_acked_valid:1;
    __u8                    wifi_acked:1;

    __u8                    no_fcs:1;
    /* Indicates the inner headers are valid in the skbuff. */
    __u8                    encapsulation:1;
    __u8                    encap_hdr_csum:1;
    __u8                    csum_valid:1;
    __u8                    csum_complete_sw:1;
    __u8                    csum_level:2;
    __u8                    csum_bad:1;

    __u8                    ipvs_property:1;
    __u8                    inner_protocol_type:1;
    __u8                    remcsum_offload:1;
    __be16                  protocol;

    struct net_device       *dev;
    sk_buff_data_t          tail;
    uint8                   *head;

	char cb[42];
	};
	
/*
 *	For all data larger than 16 octets, we need to use a
 *	pointer to memory allocated in user space.
 */
//struct	iw_point
//{
//  void __user	*pointer;	    /* Pointer to the data  (in user space) */
//  __u16		length;		    /* number of fields or size in bytes */
//  __u16		flags;		    /* Optional params */
//}PACK;	


/*
 *	Packet/Time period missed in the wireless adapter due to
 *	"wireless" specific problems...
 */
//struct	iw_missed
//{
//	__u32		beacon;		/* Missed beacons/superframe */
//}PACK;

/*
 *	Quality of the link
 */
//struct	iw_quality
//{
//	__u8		qual;		/* link quality (%retries, SNR,%missed beacons or better...) */
//	__u8		level;		/* signal level (dBm) */
//	__u8		noise;		/* noise level (dBm) */
//	__u8		updated;	/* Flags to know if updated */
//}PACK;

/*
 *	Packet discarded in the wireless adapter due to
 *	"wireless" specific problems...
 *	Note : the list of counter and statistics in net_device_stats
 *	is already pretty exhaustive, and you should use that first.
 *	This is only additional stats...
 */
//struct	iw_discarded
//{
//	__u32		nwid;		/* Rx : Wrong nwid/essid */
//	__u32		code;		/* Rx : Unable to code/decode (WEP) */
//	__u32		fragment;	/* Rx : Can't perform MAC reassembly */
//	__u32		retries;	/* Tx : Max MAC retries num reached */
//	__u32		misc;		/* Others cases */
//}PACK;
/* ------------------------ WIRELESS STATS ------------------------ */
/*
 * Wireless statistics (used for /proc/net/wireless)
 */
//struct	iw_statistics
//{
//	__u16		status;		        /* Status * - device dependent for now */

//	struct iw_quality	qual;		/* Quality of the link * (instant/mean/max) */
//	struct iw_discarded	discard;	/* Packet discarded counts */
//	struct iw_missed	miss;		/* Packet missed counts */
//}PACK;

//ZOT Timer
struct zot_timer {
	struct zot_timer *next;	/* Linked-list pointer */
	cyg_uint32 duration;	/* Duration of timer, in ticks */
	cyg_uint32 expiration;	/* Clock time at expiration */
	void (*func)(void *);	/* Function to call at expiration */
	void *arg;		        /* Arg to pass function */
	char state;		        /* Timer state */
#define	TIMER_STOP	    0
#define	TIMER_RUN	    1
#define	TIMER_EXPIRE	2
};

void set_timer(struct zot_timer *t, cyg_uint32 interval);
void start_timer(struct zot_timer *t);
void stop_timer(struct zot_timer *timer);

/* Software timers
 * There is one of these structures for each simulated timer.
 * Whenever the timer is running, it is on a linked list
 * pointed to by "Timers". The list is sorted in ascending order of
 * expiration, with the first timer to expire at the head. This
 * allows the timer process to avoid having to scan the entire list
 * on every clock tick; once it finds an unexpired timer, it can
 * stop searching.
 *
 * Stopping a timer or letting it expire causes it to be removed
 * from the list. Starting a timer puts it on the list at the right
 * place.
 */
struct timer {
	struct timer *next;	    /* Linked-list pointer */
	int32 duration;		    /* Duration of timer, in ticks */
	int32 expiration;	    /* Clock time at expiration */
	void (*func)(void *);	/* Function to call at expiration */
	void *arg;		        /* Arg to pass function */
	char state;		        /* Timer state */
#define	TIMER_STOP	    0
#define	TIMER_RUN	    1
#define	TIMER_EXPIRE	2
};

//char * strsep(char **stringp, const char* delim);

static inline unsigned int skb_headroom(const struct sk_buff *skb)
{
    return skb->data - skb->head;
}

#endif /* _MTK7601_DEP_H_ */

