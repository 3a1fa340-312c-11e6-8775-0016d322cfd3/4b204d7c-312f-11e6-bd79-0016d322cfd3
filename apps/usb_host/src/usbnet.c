/*
 * USB Host-to-Host Links
 * Copyright (C) 2000-2002 by David Brownell <dbrownell@users.sourceforge.net>
 * Copyright (C) 2002 Pavel Machek <pavel@ucw.cz>
 * Copyright (C) 2005 Philip Chang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This is used for "USB networking", connecting USB hosts as peers.
 *
 * It can be used with USB "network cables", for IP-over-USB communications;
 * Ethernet speeds without the Ethernet.  USB devices (including some PDAs)
 * can support such links directly, replacing device-specific protocols
 * with Internet standard ones.
 *
 * The links can be bridged using the Ethernet bridging (net/bridge)
 * support as appropriate.  Devices currently supported include:
 *
 *	- AnchorChip 2720
 *	- Belkin, eTEK (interops with Win32 drivers)
 *	- EPSON USB clients
 *	- GeneSys GL620USB-A
 *	- NetChip 1080 (interoperates with NetChip Win32 drivers)
 *	- Prolific PL-2301/2302 (replaces "plusb" driver)
 *	- PXA-250 or SA-1100 Linux PDAs like iPAQ, Yopy, and Zaurus
 *
 * USB devices can implement their side of this protocol at the cost
 * of two bulk endpoints; it's not restricted to "cable" applications.
 * See the SA1110, Zaurus, or EPSON device/client support in this driver;
 * slave/target drivers such as "usb-eth" (on most SA-1100 PDAs) are
 * used inside USB slave/target devices.
 *
 *
 * Status:
 *
 * - AN2720 ... not widely available, but reportedly works well
 *
 * - Belkin/eTEK ... no known issues
 *
 * - Both GeneSys and PL-230x use interrupt transfers for driver-to-driver
 *   handshaking; it'd be worth implementing those as "carrier detect".
 *   Prefer generic hooks, not minidriver-specific hacks.
 *
 * - For Netchip, should use keventd to poll via control requests to detect
 *   hardware level "carrier detect".
 *
 * - PL-230x ... the initialization protocol doesn't seem to match chip data
 *   sheets, sometimes it's not needed and sometimes it hangs.	Prolific has
 *   not responded to repeated support/information requests.
 *
 * - SA-1100 PDAs ... the standard ARM Linux SA-1100 support works nicely,
 *   as found in www.handhelds.org and other kernels.  The Sharp/Lineo
 *   kernels use different drivers, which also talk to this code.
 *
 * Interop with more Win32 drivers may be a good thing.
 *
 * Seems like reporting "peer connected" (carrier present) events may end
 * up going through the netlink event system, not hotplug ... so new links
 * would likely be handled with a link monitoring thread in some daemon.
 *
 * There are reports that bridging gives lower-than-usual throughput.
 *
 * Need smarter hotplug policy scripts ... ones that know how to arrange
 * bridging with "brctl", and can handle static and dynamic ("pump") setups.
 * Use those eventual "peer connected" events, and zeroconf.
 *
 *
 * CHANGELOG:
 *
 * 13-sep-2000	experimental, new
 * 10-oct-2000	usb_device_id table created.
 * 28-oct-2000	misc fixes; mostly, discard more TTL-mangled rx packets.
 * 01-nov-2000	usb_device_id table and probing api update by
 *		Adam J. Richter <adam@yggdrasil.com>.
 * 18-dec-2000	(db) tx watchdog, "net1080" renaming to "usbnet", device_info
 *		and prolific support, isolate net1080-specific bits, cleanup.
 *		fix unlink_urbs oops in D3 PM resume code path.
 *
 * 02-feb-2001	(db) fix tx skb sharing, packet length, match_flags, ...
 * 08-feb-2001	stubbed in "linuxdev", maybe the SA-1100 folk can use it;
 *		AnchorChips 2720 support (from spec) for testing;
 *		fix bit-ordering problem with ethernet multicast addr
 * 19-feb-2001	Support for clearing halt conditions. SA1100 UDC support
 *		updates. Oleg Drokin (green@iXcelerator.com)
 * 25-mar-2001	More SA-1100 updates, including workaround for ip problem
 *		expecting cleared skb->cb and framing change to match latest
 *		handhelds.org version (Oleg).  Enable device IDs from the
 *		Win32 Belkin driver; other cleanups (db).
 * 16-jul-2001	Bugfixes for uhci oops-on-unplug, Belkin support, various
 *		cleanups for problems not yet seen in the field. (db)
 * 17-oct-2001	Handle "Advance USBNET" product, like Belkin/eTEK devices,
 *		from Ioannis Mavroukakis <i.mavroukakis@btinternet.com>;
 *		rx unlinks somehow weren't async; minor cleanup.
 * 03-nov-2001	Merged GeneSys driver; original code from Jiun-Jie Huang
 *		<huangjj@genesyslogic.com.tw>, updated by Stanislav Brabec
 *		<utx@penguin.cz>.  Made framing options (NetChip/GeneSys)
 *		tie mostly to (sub)driver info.  Workaround some PL-2302
 *		chips that seem to reject SET_INTERFACE requests.
 *
 * 06-apr-2002	Added ethtool support, based on a patch from Brad Hards.
 *		Level of diagnostics is more configurable; they use device
 *		location (usb_device->devpath) instead of address (2.5).
 *		For tx_fixup, memflags can't be NOIO.
 * 07-may-2002	Generalize/cleanup keventd support, handling rx stalls (mostly
 *		for USB 2.0 TTs) and memory shortages (potential) too. (db)
 *		Use "locally assigned" IEEE802 address space. (Brad Hards)
 * 18-oct-2002	Support for Zaurus (Pavel Machek), related cleanup (db).
 * 15-dec-2002	Partial sync with 2.5 code: cleanups and stubbed PXA-250
 *		support (db), fix for framing issues on Z, net1080, and
 *		gl620a (Toby Milne)
 * 18-apr-2005	AX88772,AX88178 support for kernel 2.4.23 above(Philip Chang)
 *
 *-------------------------------------------------------------------------*/
/* //celebi
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/tqueue.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>
*/
// #define	DEBUG			// error path messages, extra info
// #define	VERBOSE 		// more; success messages
// #define	REALLY_QUEUE

#if !defined (DEBUG) && defined (CONFIG_USB_DEBUG)
#define DEBUG
#endif
//#include <linux/usb.h>

#undef DEBUG
#define dbg

#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
//#include "ether.h"
#include "netif/etharp.h"
#include "netif/iface.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"

#include "lwip/ip_addr.h"

#include "usb.h"

//typedef unsigned char        u8;
//typedef unsigned short       u16;
//typedef unsigned int         u32;
//typedef unsigned long        u64;
#define USB_PRINTER_WRITE_BUFFER	USBPRN_WRITE_BUFFER	//ZOT716u2 //8K

typedef struct net_device_stats
{
    u32 rx_packets;
    u32 tx_packets;
    u32 rx_bytes;
    u32 tx_bytes;
    u32 rx_errors;
    u32 rx_dropped;
    u32 tx_dropped;
    u32 multicast;
    u32 collisions;
    u32 rx_length_errors;
    u32 rx_crc_errors;
    u32 tx_fifo_errors;
} NET_DEV_STATS;


#define NT 32 /* Number of Transmit buffers */

#define NR 64 /* Number of Receive buffers */
#define W8100_INT_THRE 1
#define MAX_BUFF_SIZE 1536
#define ETH_ADDR_GAP (ETHER1_PORT_CONFIG_REG - ETHER0_PORT_CONFIG_REG)

#define NUM_TX_QUEUES 2
#define NUM_RX_QUEUES 1

#define RX_MATCHED_FRAMES   1
#define DISCARD_MATCHED_FRAMES 0

#define SKIP_ADDR_TABLE_ENTRY 1
#define DONOT_SKIP_ADDR_TABLE_ENTRY 0
#define GTREGREAD WL_REGS32

#define IFF_PROMISC   0x1
#define IFF_RUNNING   0x2

#define PACKET_HOST   1
#define CHECKSUM_NONE 1

#define SZ_PHY_ADDR 6   /*!< Number of bytes in ethernet MAC address */

typedef struct ETH_TX_DESC 
{
  volatile u32             command_status;
  volatile u32             bytecount_reserved;
  volatile u8              *buff_pointer;
  volatile struct ETH_TX_DESC *next_desc;
}ETH_TX_DESC;

typedef struct ETH_RX_DESC 
{
  volatile u32              command_status;
  volatile u32              buff_size_byte_count;
  volatile u8               *buff_pointer;
  volatile struct ETH_RX_DESC  *next_desc;
}ETH_RX_DESC;

typedef struct ETHERNET_PRIV {
    u32          ethernetDriverStatus;
    u32          ethernetRegisterBaseAddress;
    u32          addressTableHashBase;
    u32          addressTableHashSize;
    u32          addressTableHashMode;
    ETH_TX_DESC     *eth_tx_queue[NUM_TX_QUEUES];
    ETH_RX_DESC     *eth_rx_queue[NUM_RX_QUEUES];
    u32          TDN_ETH[NUM_TX_QUEUES];
    u32          TDN_ETH_TMP[NUM_TX_QUEUES];
    u32          RDN_ETH[NUM_RX_QUEUES];
    u32          ethernet_irq;
    NET_DEV_STATS   stats;
    u8           port;
}ETHERNET_PRIV;

struct net_device
{
    struct ETHERNET_PRIV *priv;
//    int trans_start;
    u16 NetDevType; /* Net device type, see NET_DEV_TYPE */
    u8 PhyAddr[SZ_PHY_ADDR]; /*!< The physical address (MAC address) */
    u32 flags;    /*!< General control word */
    void *PrivData; /*!< Pointer to net device private data */
    /*!< User defined function to allocate buffer with allignment of "allign" bytes */
    UINT8* (*allocBuf)(UINT32 allign, UINT32 bytes);
//Jesse    BOOLEAN (*freeBuf)(void *); /*!< User defined function to free buffer */
};

//celebi
//*************************************************************************
#define __init
#define __exit


//USBNET Thread initiation information
#define USBNET_TASK_PRI         	20
#define USBNET_TASK_STACK_SIZE  	4096
static	uint8			USBNET_Stack[USBNET_TASK_STACK_SIZE];
static  cyg_thread		USBNET_Task;
static  cyg_handle_t	USBNET_TaskHdl;

static cyg_sem_t usb_send_frame_mutex;//ZOT716u2

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */

#define UETH_IP_TYPE             0x800
#define UETH_ARP_TYPE            0x806
#define UBDG_DROP	( (struct netif *)4 )
#define UIS_BROADCAST_HWADDR(x) ( (*(unsigned long *)(x + 2) == 0xffffffff)  \
                                         && (*(unsigned short *)x == 0xffff) )
#define usb_fill_control_urb(a,aa,b,c,d,e,f,g) FILL_CONTROL_URB(a,aa,b,c,d,e,f,g)

struct usb_ctrlrequest
{
	__u8 requesttype;
	__u8 request;
	__u16 value;
	__u16 index;
	__u16 length;
}__attribute__ ((aligned(1), packed));

struct usb_mbuf
{
	struct usb_mbuf *anext;	/* Links packets on queues */
	char *data;
	unsigned int len;
}PACK;

typedef  struct ueth_hdr {
	unsigned char        dest[6];
	unsigned char        src[6];
	unsigned short       type;
}PACK  ueth_hdr;

err_t USB_attach( struct netif *netif);
extern err_t tcpip_input(struct pbuf *p, struct netif *inp);

extern struct netif *Lanface;  
extern struct netif *ULanface;  //ax8817x

void usbnet_thread(cyg_addrword_t data);
static void usbnet_bh (unsigned long param);

unsigned char ULANMAC[6];

#include "mii.h"
//**************************************************************************

/* in 2.5 these standard usb ops take mem_flags */
#define ALLOC_URB(n,flags)	usb_alloc_urb(n)
#define SUBMIT_URB(u,flags)	usb_submit_urb(u)

/* and these got renamed (may move to usb.h) */
#define usb_get_dev		usb_inc_dev_use
#define usb_put_dev		usb_dec_dev_use


/* minidrivers _could_ be individually configured */
//#define CONFIG_USB_AN2720
#define CONFIG_USB_AX8817X
//#define CONFIG_USB_BELKIN
//#define CONFIG_USB_EPSON2888
//#define CONFIG_USB_GENESYS
//#define CONFIG_USB_NET1080
//#define CONFIG_USB_PL2301
//#define CONFIG_USB_ARMLINUX
//#define CONFIG_USB_ZAURUS


#define DRIVER_VERSION		"18-Oct-2002"

/*-------------------------------------------------------------------------*/

/*
 * Nineteen USB 1.1 max size bulk transactions per frame (ms), max.
 * Several dozen bytes of IPv4 data can fit in two such transactions.
 * One maximum size Ethernet packet takes twenty four of them.
 * For high speed, each frame comfortably fits almost 36 max size
 * Ethernet packets (so queues should be bigger).
 */

//celebi #define RX_QLEN(dev) (((dev)->udev->speed == USB_SPEED_HIGH) ? 60 : 1)
//celebi #define TX_QLEN(dev) (((dev)->udev->speed == USB_SPEED_HIGH) ? 60 : 1)
#define RX_QLEN(dev) ( 1)
#define TX_QLEN(dev) ( 1)

// packets are always ethernet inside
// ... except they can be bigger (limit of 64K with NetChip framing)
#define MIN_PACKET	sizeof(struct eth_hdr)
#define MAX_PACKET	32768

// reawaken network queue this soon after stopping; else watchdog barks
#define TX_TIMEOUT_JIFFIES	(5*HZ)

// for vendor-specific control operations
#define CONTROL_TIMEOUT_MS	(500)			/* msec */
#define CONTROL_TIMEOUT_JIFFIES ((CONTROL_TIMEOUT_MS * HZ)/1000)

// between wakeups
#define UNLINK_TIMEOUT_JIFFIES ((3  /*ms*/ * HZ)/1000)

/*-------------------------------------------------------------------------*/

// list of all devices we manage
static DECLARE_MUTEX (usbnet_mutex);
static LIST_HEAD (usbnet_list);

// randomly generated ethernet address
static u8	node_id [ETH_ALEN];



// state we keep for each device we handle
struct usbnet {
	// housekeeping
	struct usb_device	*udev;
	struct driver_info	*driver_info;
//celebi	struct semaphore	mutex;
	struct list_head	dev_list;
//celebi	wait_queue_head_t	*wait;

	// i/o info: pipes etc
	unsigned long in, out;
	unsigned		maxpacket;
	//struct timer_list	delay;

	// protocol/interface state
	struct net_device	net;
	struct net_device_stats stats;
	int			msg_level;
	unsigned long		data [5];
	struct mii_if_info	mii;

#ifdef CONFIG_USB_NET1080
	u16			packet_id;
#endif

	// various kinds of pending driver work
//celebi	struct sk_buff_head	rxq;
//celebi	struct sk_buff_head	txq;
//celebi	struct sk_buff_head	done;
//celebi	struct tasklet_struct	bh;
	struct urb		ctrl_urb, rx_urb, tx_urb, intr_urb; //celebi
	unsigned char		*rx_buff;	//celebi
	unsigned char		*tx_buff;	//celebi
	
//celebi	struct tasklet_struct	mf;  //media_func tasklet
//celebi	struct tasklet_struct	mf0;  //media_func tasklet
	struct ax88178_data	*ax88178_data_ptr;
	struct ax88772_data	*ax88772_data_ptr;
//celebi	struct tq_struct	kevent;
	unsigned long		flags;
#define EVENT_TX_HALT	0
#define EVENT_RX_HALT	1
#define EVENT_RX_MEMORY	2
};

struct usbnet *USBNET=NULL;

struct ax88178_data {
	u16 MediaLink;
	int UseRgmii;
	u8 PhyMode;
	u8 LedMode;
	int nx_state;						//add by philip
	u8 buf[6];
	u16 PhyID;
	u16 buf16_1[1];
	int i;

	int ret;
	int fullduplex;
	u16 phylinkstatus1;
	u16 phylinkstatus2;
	u16 tempshort;
	u16 phyctrl;
	u16 phyreg;
	u16 phyanar;
	u16 phyauxctrl;
};
struct ax88772_data {
	int nx_state;						//add by philip
	int ret;
	u8 buf[6];
	u16 buf16_1[1];
	u16 actual_spm;
	u16 medium_mode;
	u16 buf16_2[1];
	int loopcnt;
};
// device-specific info used by the driver
struct driver_info {
	char		*description;

	int		flags;
#define FLAG_FRAMING_NC 0x0001		/* guard against device dropouts */
#define FLAG_FRAMING_GL 0x0002		/* genelink batches packets */
#define FLAG_FRAMING_Z	0x0004		/* zaurus adds a trailer */
#define FLAG_NO_SETINT	0x0010		/* device can't set_interface() */
#define FLAG_ETHER	0x0020		/* maybe use "eth%d" names */

#define FLAG_FRAMING_AX 0x0040		/* AX88772/178 packets */
	/* init device ... can sleep, or cause probe() failure */
	int	(*bind)(struct usbnet *, struct usb_device *);

	/* reset device ... can sleep */
	int	(*reset)(struct usbnet *);

	/* see if peer is connected ... can sleep */
	int	(*check_connect)(struct usbnet *);

	/* fixup rx packet (strip framing) */
	int	(*rx_fixup)(struct usbnet *dev, char *packet_ptr, int leng);

	/* fixup tx packet (add framing) */
	char (*tx_fixup)(struct usbnet *dev, char *buf, int *len);


	// FIXME -- also an interrupt mechanism
	// useful for at least PL2301/2302 and GL620USB-A

	/* for new devices, use the descriptor-reading code instead */
	int		in;		/* rx endpoint */
	int		out;		/* tx endpoint */
	int		epsize;

	unsigned long	data;		/* Misc driver specific data */
};

// we record the state for each of our queued skbs
enum skb_state {
	illegal = 0,
	tx_start, tx_done,
	rx_start, rx_done, rx_cleanup
};

struct skb_data {	// skb->cb is one of these
	struct urb		*urb;
	struct usbnet		*dev;
	enum skb_state		state;
	size_t			length;
};

static const char driver_name [] = "usbnet";

/* use ethtool to change the level for any given device */
static int msg_level = 1;
//celebi
#if 0
MODULE_PARM (msg_level, "i");
MODULE_PARM_DESC (msg_level, "Initial message level (default = 1)");
#endif

#define mutex_lock(x)	down(x)
#define mutex_unlock(x) up(x)

#define RUN_CONTEXT (in_irq () ? "in_irq" \
			: (in_interrupt () ? "in_interrupt" : "can sleep"))




#ifdef DEBUG
#define devdbg(usbnet, fmt, arg...) \
	printk(KERN_DEBUG "%s: " fmt "\n" , (usbnet)->net.name , ## arg)
#else
#define devdbg(usbnet, fmt, arg...) do {} while(0)
#endif

#define deverr(usbnet, fmt, arg...) \
	printk(KERN_ERR "%s: " fmt "\n" , (usbnet)->net.name , ## arg)
#define devwarn(usbnet, fmt, arg...) \
	printk(KERN_WARNING "%s: " fmt "\n" , (usbnet)->net.name , ## arg)
#define devinfo(usbnet, fmt, arg...) \
	do { if ((usbnet)->msg_level >= 1) \
	printk(KERN_INFO "%s: " fmt "\n" , (usbnet)->net.name , ## arg); \
	} while (0)




//celebi static void usbnet_get_drvinfo (struct net_device *, struct ethtool_drvinfo *);
//celebi static u32 usbnet_get_link (struct net_device *);
//celebi static u32 usbnet_get_msglevel (struct net_device *);
//celebi static void usbnet_set_msglevel (struct net_device *, u32);
//celebi static struct ethtool_ops usbnet_ethtool_ops;

/* mostly for PDA style devices, which are always present */
static int always_connected (struct usbnet *dev)
{
	return 0;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/* handles CDC Ethernet and many other network "bulk data" interfaces */
int usbnet_get_endpoints(struct usbnet *dev, struct usb_interface *intf)
{
	int				tmp;
	struct usb_interface_descriptor	*alt = NULL;
	struct usb_endpoint_descriptor	*in = NULL, *out = NULL;
	struct usb_endpoint_descriptor	*status = NULL;

	for (tmp = 0; tmp < intf->num_altsetting; tmp++) {
		unsigned	ep;

		in = out = status = NULL;
		alt = intf->altsetting + tmp;

		/* take the first altsetting with in-bulk + out-bulk;
		 * remember any status endpoint, just in case;
		 * ignore other endpoints and altsetttings.
		 */
		for (ep = 0; ep < alt->bNumEndpoints; ep++) {
			struct usb_endpoint_descriptor	*e;
			int				intr = 0;

			e = alt->endpoint + ep;
			switch (e->bmAttributes) {
			case USB_ENDPOINT_XFER_INT:
				if (!(e->bEndpointAddress & USB_DIR_IN))
				continue;
				intr = 1;
				/* FALLTHROUGH */
			case USB_ENDPOINT_XFER_BULK:
				break;
			default:
				continue;
			}
			if (e->bEndpointAddress & USB_DIR_IN) {
				if (!intr && !in)
					in = e;
				else if (intr && !status)
					status = e;
			} else {
				if (!out)
					out = e;
			}
		}
			if (in && out)
			break;
	}
	if (!alt || !in || !out)
	return -EINVAL;

	if (alt->bAlternateSetting != 0
			|| !(dev->driver_info->flags & FLAG_NO_SETINT)) {
		tmp = usb_set_interface (dev->udev, alt->bInterfaceNumber,
				alt->bAlternateSetting);
		if (tmp < 0)
			return tmp;
	}

	dev->in = usb_rcvbulkpipe (dev->udev,
			in->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);
	dev->out = usb_sndbulkpipe (dev->udev,
			out->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

	return 0;
}

//celebi
#if 0
static void skb_return (struct usbnet *dev, struct sk_buff *skb)
{
	int	status;

	skb->dev = &dev->net;
	skb->protocol = eth_type_trans (skb, &dev->net);
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;

#ifdef	VERBOSE
	devdbg (dev, "< rx, len %d, type 0x%x",
		skb->len + sizeof (struct ethhdr), skb->protocol);
#endif
	memset (skb->cb, 0, sizeof (struct skb_data));
	status = netif_rx (skb);
	if (status != NET_RX_SUCCESS)
		devdbg (dev, "netif_rx status %d", status);
}
#endif
/*-------------------------------------------------------------------------*/







#ifdef	CONFIG_USB_AN2720

/*-------------------------------------------------------------------------
 *
 * AnchorChips 2720 driver ... http://www.cypress.com
 *
 * This doesn't seem to have a way to detect whether the peer is
 * connected, or need any reset handshaking.  It's got pretty big
 * internal buffers (handles most of a frame's worth of data).
 * Chip data sheets don't describe any vendor control messages.
 *
 *-------------------------------------------------------------------------*/

static const struct driver_info an2720_info = {
	.description =	"AnchorChips/Cypress 2720",
	// no reset available!
	// no check_connect available!

	.in = 2, .out = 2,		// direction distinguishes these
	.epsize =64,
};

#endif	/* CONFIG_USB_AN2720 */


#ifdef CONFIG_USB_AX8817X
/* ASIX AX8817X based USB 2.0 Ethernet Devices */

#define HAVE_HARDWARE
//celebi #define NEED_MII

//celebi #include <linux/crc32.h>

#define AX_CMD_SET_SW_MII		0x06
#define AX_CMD_READ_MII_REG		0x07
#define AX_CMD_WRITE_MII_REG		0x08
#define AX_CMD_SET_HW_MII		0x0a
#define AX_CMD_READ_EEPROM		0x0b
#define AX_CMD_WRITE_EEPROM		0x0c
#define AX_CMD_WRITE_RX_CTL		0x10
#define AX_CMD_READ_IPG012		0x11
#define AX_CMD_WRITE_IPG0		0x12
#define AX_CMD_WRITE_IPG1		0x13
#define AX_CMD_WRITE_IPG2		0x14
#define AX_CMD_WRITE_MULTI_FILTER	0x16
#define AX_CMD_READ_NODE_ID		0x17
#define AX_CMD_READ_PHY_ID		0x19
#define AX_CMD_WRITE_MEDIUM_MODE	0x1b
#define AX_CMD_READ_MONITOR_MODE	0x1c
#define AX_CMD_WRITE_MONITOR_MODE	0x1d
#define AX_CMD_WRITE_GPIOS		0x1f
#define AX_CMD_SW_RESET 		0x20
#define AX_CMD_SW_PHY_STATUS		0x21
#define AX_CMD_SW_PHY_SELECT		0x22
#define AX88772_CMD_READ_NODE_ID	0x13

#define AX_MONITOR_MODE 		0x01
#define AX_MONITOR_LINK 		0x02
#define AX_MONITOR_MAGIC		0x04
#define AX_MONITOR_HSFS 		0x10

#define AX_MCAST_FILTER_SIZE		8
#define AX_MAX_MCAST			64
#define AX_INTERRUPT_BUFSIZE		8

#define AX88772_IPG0_DEFAULT		0x15
#define AX88772_IPG1_DEFAULT		0x0c
#define AX88772_IPG2_DEFAULT		0x12

#define AX_SWRESET_CLEAR			0x00
#define AX_SWRESET_RR			0x01
#define AX_SWRESET_RT			0x02
#define AX_SWRESET_PRTE			0x04
#define AX_SWRESET_PRL			0x08
#define AX_SWRESET_BZ			0x10
#define AX_SWRESET_IPRL			0x20
#define AX_SWRESET_IPPD			0x40

/* GPIO REGISTER */
#define AXGPIOS_GPO0EN			0X01
#define AXGPIOS_GPO0			0X02
#define AXGPIOS_GPO1EN			0X04
#define AXGPIOS_GPO1			0X08
#define AXGPIOS_GPO2EN			0X10
#define AXGPIOS_GPO2			0X20
#define AXGPIOS_RSE			0X80

#define AX_RX_CTL_MFB			0x0300		/* Maximum Frame size 16384bytes */
#define AX_RX_CTL_START			0x0080		/* Ethernet MAC start */
#define AX_RX_CTL_AP			0x0020		/* Accept physcial address from Multicast array */
#define AX_RX_CTL_AM			0x0010	
#define AX_RX_CTL_AB			0x0008		/* Accetp Brocadcast frames*/
#define AX_RX_CTL_SEP			0x0004		/* Save error packets */	
#define AX_RX_CTL_AMALL			0x0002		/* Accetp all multicast frames */
#define AX_RX_CTL_PRO			0x0001		/* Promiscuous Mode */
#define AX_RX_CTL_STOP			0x0000		/* Stop MAC */

#define AX88772A_IPG0_DEFAULT		0x15
#define AX88772A_IPG1_DEFAULT		0x16
#define AX88772A_IPG2_DEFAULT		0x1A

#define AX88772_MEDIUM_FULL_DUPLEX	0x0002
#define AX88772_MEDIUM_RESERVED		0x0004
#define AX88772_MEDIUM_RX_FC_ENABLE	0x0010
#define AX88772_MEDIUM_TX_FC_ENABLE	0x0020
#define AX88772_MEDIUM_PAUSE_FORMAT	0x0080
#define AX88772_MEDIUM_RX_ENABLE	0x0100
#define AX88772_MEDIUM_100MB		0x0200
#define AX88772_MEDIUM_DEFAULT	\
	(AX88772_MEDIUM_FULL_DUPLEX | AX88772_MEDIUM_RX_FC_ENABLE | \
	 AX88772_MEDIUM_TX_FC_ENABLE | AX88772_MEDIUM_100MB | \
	 AX88772_MEDIUM_RESERVED | AX88772_MEDIUM_RX_ENABLE )

struct ax88772a_data {
	struct usbnet *dev;
//	struct workqueue_struct *ax_work;
//	struct work_struct check_link;
	unsigned long autoneg_start;
	u8 Event;
	u8 TickToExpire;
	u8 DlyIndex;
	u8 DlySel;
};

struct ax8817x_data {
	u8 multi_filter[AX_MCAST_FILTER_SIZE];
	struct urb *int_urb;
	u8 *int_buf;
};
static int ax8817x_read_cmd(struct usbnet *dev, u8 cmd, u16 value, u16 index,
			    u16 size, void *data)
{
	return usb_control_msg(
		dev->udev,
		usb_rcvctrlpipe(dev->udev, 0),
		cmd,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		data,
		size,
		CONTROL_TIMEOUT_JIFFIES);
}

static int ax8817x_write_cmd(struct usbnet *dev, u8 cmd, u16 value, u16 index,
			     u16 size, void *data)
{
	return usb_control_msg(
		dev->udev,
		usb_sndctrlpipe(dev->udev, 0),
		cmd,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		data,
		size,
		CONTROL_TIMEOUT_JIFFIES);
}

static void ax8817x_async_cmd_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->context;

//celebi	if (urb->status < 0)
//celebi		printk(KERN_DEBUG "ax8817x_async_cmd_callback() failed with %d",
//celebi			urb->status);

	kfree(req, 0);
	usb_free_urb(urb);
}

//celebi
#if 0
static void ax88178_async_read_cmd_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->setup_packet;
	struct usbnet *dev = urb->context;

	if (urb->status < 0)
		printk(KERN_DEBUG "ax8817x_async_cmd_callback() failed with %d",
			urb->status);
	tasklet_schedule(&dev->mf);
	kfree(req, 0);
	usb_free_urb(urb);
}
#endif
//celebi
#if 0
static void ax88772_async_read_cmd_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->setup_packet;
	struct usbnet *dev = urb->context;
	struct ax88772_data *ax772dataptr = (struct ax88772_data *)dev->ax88772_data_ptr;
	int test;
	if (urb->status < 0)
		printk(KERN_DEBUG "ax88772_async_cmd_callback() failed with %d",
			urb->status);
	if(ax772dataptr->nx_state==4){
	test=1;
	}
	tasklet_schedule(&dev->mf0);
	kfree(req, 0);
	usb_free_urb(urb);
}
#endif
//celebi
#if 0
static void ax88178_async_write_cmd_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->setup_packet;
	struct usbnet *dev = urb->context;

	if (urb->status < 0)
		printk(KERN_DEBUG "ax8817x_async_cmd_callback() failed with %d",
			urb->status);
	tasklet_schedule(&dev->mf);
	kfree(req, 0);
	usb_free_urb(urb);
}
#endif
//celebi
#if 0
static void ax88772_async_write_cmd_callback(struct urb *urb)
{
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)urb->setup_packet;
	struct usbnet *dev = urb->context;

	if (urb->status < 0)
		printk(KERN_DEBUG "ax88772_async_cmd_callback() failed with %d",
			urb->status);
	tasklet_schedule(&dev->mf0);
	kfree(req, 0);
	usb_free_urb(urb);
}

static void ax8817x_interrupt_complete(struct urb *urb)
{
	struct usbnet *dev = (struct usbnet *)urb->context;
	struct ax8817x_data *data = (struct ax8817x_data *)&dev->data;
	int link;

	if (urb->status < 0) {
		devdbg(dev,"ax8817x_interrupt_complete() failed with %d",
			urb->status);
	} else {
		if (data->int_buf[5] == 0x90) {
			link = data->int_buf[2] & 0x01;
			if (netif_carrier_ok(&dev->net) != link) {
				if (link)
					netif_carrier_on(&dev->net);
				else
					netif_carrier_off(&dev->net);
				devdbg(dev, "ax8817x - Link Status is: %d", link);
			}
		}
		usb_submit_urb(data->int_urb);
	}
}
#endif


static void media_func(unsigned long);
static void media_func772(unsigned long);

static void ax88178_interrupt_complete(struct urb *urb)
{
//celebi
#if 0
	struct usbnet *dev = (struct usbnet *)urb->context;
	struct ax8817x_data *data = (struct ax8817x_data *)&dev->data;
	struct ax88178_data *ax178dataptr = (struct ax88178_data *)dev->ax88178_data_ptr;
	int link;

	if (urb->status < 0) {
		devdbg(dev,"ax88178_interrupt_complete() failed with %d",
			urb->status);
	} else {
	    //	if (data->int_buf[5] == 0x90) {
			link = data->int_buf[2] & 0x01;
			if (netif_carrier_ok(&dev->net) != link) {
				if (link&&(ax178dataptr->nx_state==0)){
					tasklet_schedule(&dev->mf);
				}
				else
					netif_carrier_off(&dev->net);
				devdbg(dev, "ax8817x - Link Status is: %d", link);
			}
	   //	}

		if(ax178dataptr->nx_state==970){	      //recheck media type
		  netif_carrier_on(&dev->net);
		  ax178dataptr->nx_state=0;
		}
		usb_submit_urb(data->int_urb);
	}
#endif	
}

static void ax88772_interrupt_complete(struct urb *urb)
{
//celebi
#if 0
	struct usbnet *dev = (struct usbnet *)urb->context;
	struct ax8817x_data *data = (struct ax8817x_data *)&dev->data;
	struct ax88772_data *ax772dataptr = (struct ax88772_data *)dev->ax88772_data_ptr;
	int link;
	int tmp;

	if (urb->status < 0) {
		devdbg(dev,"ax88772_interrupt_complete() failed with %d",
			urb->status);
	} else {
		link = data->int_buf[2] & 0x01;
		tmp = netif_carrier_ok(&dev->net);
		if ( tmp != link) {
			if (link&&(ax772dataptr->nx_state==0)){
				   ax772dataptr->nx_state=1;
				tasklet_schedule(&dev->mf0);
			}
			else
				netif_carrier_off(&dev->net);
//			devdbg(dev, "ax88772 - Link Status is: %d", link);
		}

		if(ax772dataptr->nx_state==970){	      //recheck media type
		  netif_carrier_on(&dev->net);
		  ax772dataptr->nx_state=0;
		}
		usb_submit_urb(data->int_urb);
	}
#endif	
}
//celebi
#if 0
static void ax88178_read_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,
				    u16 size, void *data)
{
	struct usb_ctrlrequest *req;
	int status;
	struct urb *urb;

	if ((urb = usb_alloc_urb(0)) == NULL) {
		devdbg(dev, "Error allocating URB in read_cmd_async!");
		return;
	}

	if ((req = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC)) == NULL) {
		deverr(dev, "Failed to allocate memory for control request");
		usb_free_urb(urb);
		return;
	}

	req->bRequestType = USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);
#if 0 //celebi
	usb_fill_control_urb(urb, dev->udev,
			     usb_rcvctrlpipe(dev->udev, 0),
			     (void *)req, data, size,
			     ax88178_async_read_cmd_callback,dev); //I don't use req as urb->context here
#endif

	if((status = usb_submit_urb(urb)) < 0) {
		deverr(dev, "Error submitting the control message: status=%d", status);
		kfree(req, 0);
		usb_free_urb(urb);
	}
}
#endif
//celebi
#if 0
static void ax88772_read_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,
				    u16 size, void *data)
{
	struct usb_ctrlrequest *req;
	int status;
	struct urb *urb;
	struct ax88772_data *ax772dataptr = (struct ax88772_data *)dev->ax88772_data_ptr;
	int test;
	if ((urb = usb_alloc_urb(0)) == NULL) {
		devdbg(dev, "Error allocating URB in read_cmd_async!");
		return;
	}

	if ((req = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC)) == NULL) {
		deverr(dev, "Failed to allocate memory for control request");
		usb_free_urb(urb);
		return;
	}

	req->bRequestType = USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);
	if(ax772dataptr->nx_state==4){
	test=1;
	}
#if 0 //celebi	
	usb_fill_control_urb(urb, dev->udev,
			     usb_rcvctrlpipe(dev->udev, 0),
			     (void *)req, data, size,
			     ax88772_async_read_cmd_callback,dev); //I don't use req as urb->context here
#endif

	if((status = usb_submit_urb(urb)) < 0) {
		deverr(dev, "Error submitting the control message: status=%d", status);
		kfree(req, 0);
		usb_free_urb(urb);
	}
}
#endif

static void ax8817x_write_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,
				    u16 size, void *data)
{
	struct usb_ctrlrequest *req;
	int status;
	struct urb *urb;

	if ((urb = ALLOC_URB(0, GFP_ATOMIC)) == NULL) {
		devdbg(dev, "Error allocating URB in write_cmd_async!");
		return;
	}

	if ((req = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC)) == NULL) {
		devdbg(dev, "Failed to allocate memory for control request");
		usb_free_urb(urb);
		return;
	}
/*
	req->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);
*/
	req->requesttype = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->request = cmd;
	req->value = cpu_to_le16(value);
	req->index = cpu_to_le16(index);
	req->length = cpu_to_le16(size);


	usb_fill_control_urb(urb, dev->udev,
			     usb_sndctrlpipe(dev->udev, 0),
			     (void *)req, data, size,
			     ax8817x_async_cmd_callback, req);

	if((status = SUBMIT_URB(urb, GFP_ATOMIC)) < 0)
		devdbg(dev, "Error submitting the control message: status=%d", status);
}

//celebi
#if 0
static void ax88178_write_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,
				    u16 size, void *data)
{
	struct usb_ctrlrequest *req;
	int status;
	struct urb *urb;

	if ((urb = usb_alloc_urb(0)) == NULL) {
		devdbg(dev, "Error allocating URB in write_cmd_async!");
		return;
	}

	if ((req = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC)) == NULL) {
		deverr(dev, "Failed to allocate memory for control request");
		usb_free_urb(urb);
		return;
	}

	req->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);
#if 0 //celebi
	usb_fill_control_urb(urb, dev->udev,
			     usb_sndctrlpipe(dev->udev, 0),
			     (void *)req, data, size,
			     ax88178_async_write_cmd_callback, dev);
#endif 
	if((status = usb_submit_urb(urb)) < 0) {
		deverr(dev, "Error submitting the control message: status=%d", status);
		kfree(req, 0);
		usb_free_urb(urb);
	}
}
#endif
//celebi
#if 0
static void ax88772_write_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,
				    u16 size, void *data)
{
	struct usb_ctrlrequest *req;
	int status;
	struct urb *urb;

	if ((urb = usb_alloc_urb(0)) == NULL) {
		devdbg(dev, "Error allocating URB in write_cmd_async!");
		return;
	}

	if ((req = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC)) == NULL) {
		deverr(dev, "Failed to allocate memory for control request");
		usb_free_urb(urb);
		return;
	}
//celebi
/*
	req->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);
*/
	req->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE;
	req->bRequest = cmd;
	req->wValue = cpu_to_le16(value);
	req->wIndex = cpu_to_le16(index);
	req->wLength = cpu_to_le16(size);

#if 0 //celebi
	usb_fill_control_urb(urb, dev->udev,
			     usb_sndctrlpipe(dev->udev, 0),
			     (void *)req, data, size,
			     ax88772_async_write_cmd_callback, dev);
#endif
	if((status = usb_submit_urb(urb)) < 0) {
//celebi		deverr(dev, "Error submitting the control message: status=%d", status);
		kfree(req, 0);
		usb_free_urb(urb);
	}
}
#endif

//celebi
#if 0
static void ax8817x_set_multicast(struct net_device *net)
{
	struct usbnet *dev = (struct usbnet *) net->priv;
	u8 rx_ctl = 0x8c;

	if (net->flags & IFF_PROMISC) {
		rx_ctl |= 0x01;
	} else if (net->flags & IFF_ALLMULTI
		   || net->mc_count > AX_MAX_MCAST) {
		rx_ctl |= 0x02;
	} else if (net->mc_count == 0) {
		/* just broadcast and directed */
	} else {
		struct dev_mc_list *mc_list = net->mc_list;
		u8 *multi_filter;
		u32 crc_bits;
		int i;

		multi_filter = kmalloc(AX_MCAST_FILTER_SIZE, GFP_ATOMIC);
		if (multi_filter == NULL) {
			/* Oops, couldn't allocate a buffer for setting the multicast
			   filter. Try all multi mode. */
			rx_ctl |= 0x02;
		} else {
			memset(multi_filter, 0, AX_MCAST_FILTER_SIZE);

			/* Build the multicast hash filter. */
			for (i = 0; i < net->mc_count; i++) {
				crc_bits =
				    ether_crc(ETH_ALEN,
					      mc_list->dmi_addr) >> 26;
				multi_filter[crc_bits >> 3] |=
				    1 << (crc_bits & 7);
				mc_list = mc_list->next;
			}

			ax8817x_write_cmd_async(dev, AX_CMD_WRITE_MULTI_FILTER, 0, 0,
					   AX_MCAST_FILTER_SIZE, multi_filter);

			rx_ctl |= 0x10;
		}
	}

	ax8817x_write_cmd_async(dev, AX_CMD_WRITE_RX_CTL, rx_ctl, 0, 0, NULL);
}
#endif

static int ax8817x_mdio_read(struct net_device *netdev, int phy_id, int loc)
{
	struct usbnet *dev = netdev->priv;
	u16 res;
	u8 buf[4];

	ax8817x_write_cmd(dev, AX_CMD_SET_SW_MII, 0, 0, 0, &buf);
	ax8817x_read_cmd(dev, AX_CMD_READ_MII_REG, phy_id, (__u16)loc, 2, (u16 *)&res);
	ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf);

	return res & 0xffff;
}

static void ax8817x_mdio_write(struct net_device *netdev, int phy_id, int loc, int val)
{
	struct usbnet *dev = netdev->priv;
	u16 res = val;
	u8 buf[4];

	ax8817x_write_cmd(dev, AX_CMD_SET_SW_MII, 0, 0, 0, &buf);
	ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG, phy_id, (__u16)loc, 2, (u16 *)&res);
	ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf);
}

/* same as above, but converts resulting value to cpu byte order */
static int ax8817x_mdio_read_le(struct net_device *netdev, int phy_id, int loc)
{
	return le16_to_cpu(ax8817x_mdio_read(netdev,phy_id, loc));
}

/* same as above, but converts new value to le16 byte order before writing */
static void
ax8817x_mdio_write_le(struct net_device *netdev, int phy_id, int loc, int val)
{
	ax8817x_mdio_write( netdev, phy_id, loc, cpu_to_le16(val) );
}

//celebi 
#if 0
static void ax8817x_get_wol(struct net_device *net, struct ethtool_wolinfo *wolinfo)
{
	struct usbnet *dev = (struct usbnet *)net->priv;
	u8 opt;

	if (ax8817x_read_cmd(dev, AX_CMD_READ_MONITOR_MODE, 0, 0, 1, &opt) < 0) {
		wolinfo->supported = 0;
		wolinfo->wolopts = 0;
		return;
	}
	wolinfo->supported = WAKE_PHY | WAKE_MAGIC;
	wolinfo->wolopts = 0;
	if (opt & AX_MONITOR_MODE) {
		if (opt & AX_MONITOR_LINK)
			wolinfo->wolopts |= WAKE_PHY;
		if (opt & AX_MONITOR_MAGIC)
			wolinfo->wolopts |= WAKE_MAGIC;
	}
}

static int ax8817x_set_wol(struct net_device *net, struct ethtool_wolinfo *wolinfo)
{
	struct usbnet *dev = (struct usbnet *)net->priv;
	u8 opt = 0;
	u8 buf[1];

	if (wolinfo->wolopts & WAKE_PHY)
		opt |= AX_MONITOR_LINK;
	if (wolinfo->wolopts & WAKE_MAGIC)
		opt |= AX_MONITOR_MAGIC;
	if (opt != 0)
		opt |= AX_MONITOR_MODE;

	if (ax8817x_write_cmd(dev, AX_CMD_WRITE_MONITOR_MODE,
			      opt, 0, 0, &buf) < 0)
		return -EINVAL;

	return 0;
}

static int ax8817x_get_eeprom(struct net_device *net,
			      struct ethtool_eeprom *eeprom, u8 *data)
{
	struct usbnet *dev = (struct usbnet *)net->priv;
	u16 *ebuf = (u16 *)data;
	int i;

	/* Crude hack to ensure that we don't overwrite memory
	 * if an odd length is supplied
	 */
	if (eeprom->len % 2)
		return -EINVAL;

	/* ax8817x returns 2 bytes from eeprom on read */
	for (i=0; i < eeprom->len / 2; i++) {
		if (ax8817x_read_cmd(dev, AX_CMD_READ_EEPROM,
			eeprom->offset + i, 0, 2, &ebuf[i]) < 0)
			return -EINVAL;
	}
	return i * 2;
}

static void ax8817x_get_drvinfo (struct net_device *net,
				 struct ethtool_drvinfo *info)
{
	/* Inherit standard device info */
	usbnet_get_drvinfo(net, info);
	info->eedump_len = 0x3e;
}
#endif

int mii_link_ok (struct mii_if_info *mii)
{
	/* first, a dummy read, needed to latch some MII phys */
	mii->mdio_read(mii->dev, mii->phy_id, MII_BMSR);
	if (mii->mdio_read(mii->dev, mii->phy_id, MII_BMSR) & BMSR_LSTATUS)
		return 1;
	return 0;
}

static u32 ax8817x_get_link (struct net_device *net)
{
	struct usbnet *dev = (struct usbnet *)net->priv;

	return (u32)mii_link_ok(&dev->mii);
}

//celebi
#if 0
static int ax8817x_get_settings(struct net_device *net, struct ethtool_cmd *cmd)
{
	struct usbnet *dev = (struct usbnet *)net->priv;

	return mii_ethtool_gset(&dev->mii,cmd);
}

static int ax8817x_set_settings(struct net_device *net, struct ethtool_cmd *cmd)
{
	struct usbnet *dev = (struct usbnet *)net->priv;

	return mii_ethtool_sset(&dev->mii,cmd);
}
#endif
/* We need to override some ethtool_ops so we require our
   own structure so we don't interfere with other usbnet
   devices that may be connected at the same time. */
//celebi
#if 0
static struct ethtool_ops ax8817x_ethtool_ops = {
	.get_drvinfo		= ax8817x_get_drvinfo,
	.get_link		= ax8817x_get_link,
	.get_msglevel		= usbnet_get_msglevel,
	.set_msglevel		= usbnet_set_msglevel,
	.get_wol		= ax8817x_get_wol,
	.set_wol		= ax8817x_set_wol,
	.get_eeprom		= ax8817x_get_eeprom,
	.get_settings		= ax8817x_get_settings,
	.set_settings		= ax8817x_set_settings,
};
#endif

int ax8817x_bind(struct usbnet *dev, struct usb_device *intf)
{
	int ret;
	u8 *buf;
//	u16 *buf16 = (u16 *) buf;
	u16 *buf16 ;
	int i;
	unsigned long gpio_bits = dev->driver_info->data;
	u8 rx_ctl = 0x8c;
u16 tempvalue;

	if(!(buf = kmalloc (6, GFP_KERNEL)))
		return -ENOMEM;
	buf16 = buf;

	dev->in = usb_rcvbulkpipe(dev->udev, 3);
	dev->out = usb_sndbulkpipe(dev->udev, 2);

	/* Toggle the GPIOs in a manufacturer/model specific way */
	for (i = 2; i >= 0; i--) {
		if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
				       (gpio_bits >> (i * 8)) & 0xff, 0, 0,
				       buf)) < 0){
			kfree(buf, 0);	       	
			return ret;
		}
		wait_ms(5);
	}

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x80, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}

	if ((ret = ax8817x_write_cmd(dev, 0x0d, 0, 0, 0, buf)) < 0) {
//celebi		dbg("enable SROM reading failed: %d", ret);
		kfree(buf, 0);
		return ret;   // ???
	}

	/* Write the MAC address */
	/*
	memset(buf, 0, ETH_ALEN);
	for(i=0;i<3;i++)
	{
 		tempvalue = (u16)(MyPhysNodeAddress[i*2]) | (u16)(MyPhysNodeAddress[(i*2)+1]<< 8);

		if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_EEPROM, 4+i, tempvalue, 0, &MyPhysNodeAddress[i*2])) < 0) {
//celebi		dbg("read AX_CMD_READ_NODE_ID failed: %d", ret);
			return ret;
		}
		if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_EEPROM, 4+i, 0, 2, &buf[i*2])) < 0) {
//celebi		dbg("read AX_CMD_READ_NODE_ID failed: %d", ret);
			return ret;
		}
	}

*/
	wait_ms(150);

	/* Get the MAC address */
	memset(buf, 0, ETH_ALEN);
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_NODE_ID, 0, 0, 6, buf)) < 0) {
//celebi		dbg("read AX_CMD_READ_NODE_ID failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}

	memcpy(dev->net.PhyAddr, buf, ETH_ALEN);
	memcpy(ULANMAC, buf, ETH_ALEN);
	
	memcpy(dev->net.PhyAddr, MyPhysNodeAddress, ETH_ALEN);
	memcpy(ULANMAC, MyPhysNodeAddress, ETH_ALEN);

	if ((ret = ax8817x_write_cmd(dev, 0x0e, 0, 0, 0, buf)) < 0) {
//celebi		dbg("disable SROM reading failed: %d", ret);
		kfree(buf, 0);
		return ret;   // ???
	}


	/* Get IPG values */
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_IPG012, 0, 0, 3, buf)) < 0) {
//celebi		dbg("Error reading IPG values: %d", ret);
		kfree(buf, 0);
		return ret;
	}

	for(i = 0;i < 3;i++) {
		ax8817x_write_cmd(dev, AX_CMD_WRITE_IPG0 + i, 0, 0, 1, &buf[i]);
	}

	/* Get the PHY id */
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_PHY_ID, 0, 0, 2, buf)) < 0) {
//celebi		dbg("error on read AX_CMD_READ_PHY_ID: %02x", ret);
		kfree(buf, 0);
		return ret;
	} else if (ret < 2) {
		/* this should always return 2 bytes */
//celebi		dbg("AX_CMD_READ_PHY_ID returned less than 2 bytes: ret=%02x", ret);
		kfree(buf, 0);
		return -EIO;
	}

	/* Initialize MII structure */
	dev->mii.dev = &dev->net;
	dev->mii.mdio_read = ax8817x_mdio_read;
	dev->mii.mdio_write = ax8817x_mdio_write;
	dev->mii.phy_id_mask = 0x3f;
	dev->mii.reg_num_mask = 0x1f;
	dev->mii.phy_id = buf[1];

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SET_SW_MII, 0, 0, 0, &buf)) < 0) {
//celebi		dbg("Failed to go to software MII mode: %02x", ret);
		kfree(buf, 0);
		return ret;
	}

	*buf16 = cpu_to_le16(BMCR_RESET);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_BMCR, 2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg - MII_BMCR: %02x", ret);
		kfree(buf, 0);
		return ret;
	}

	/* Advertise that we can do full-duplex pause */
	*buf16 = cpu_to_le16(ADVERTISE_ALL | ADVERTISE_CSMA | 0x0400);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_ADVERTISE,
				     2, buf16)) < 0) {
//celebi		dbg("Failed to write MII_REG advertisement: %02x", ret);
		kfree(buf, 0);
		return ret;
	}

	*buf16 = cpu_to_le16(BMCR_ANENABLE | BMCR_ANRESTART);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_BMCR,
				     2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg autonegotiate: %02x", ret);
		kfree(buf, 0);
		return ret;
	}

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf)) < 0) {
//celebi		dbg("Failed to set hardware MII: %02x", ret);
		kfree(buf, 0);
		return ret;
	}

//celebi	dev->net.set_multicast_list = ax8817x_set_multicast;
rx_ctl = 0xFFFF;
	ax8817x_write_cmd_async(dev, AX_CMD_WRITE_RX_CTL, rx_ctl, 0, 0, NULL);
	
	kfree(buf, 0);
}


static int ax88772a_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int ret = -EIO;
	void *buf;
//	struct ax8817x_data *data = (struct ax8817x_data *)&dev->data;
	struct ax88772a_data *ax772a_data = NULL;
	int bmcr;

	dev->in = usb_rcvbulkpipe(dev->udev, 2);  //the 88172 and 88772 in out endpoint is different
	dev->out = usb_sndbulkpipe(dev->udev, 3);     //but 88772 and 88178 is the same
	
	usbnet_get_endpoints(dev,((struct usb_device*)intf)->actconfig->interface + 0);
	
	buf = kmalloc(6, GFP_KERNEL);
	if(!buf) {
//		dbg ("Cannot allocate memory for buffer");
		ret = -ENOMEM;
		goto out1;
	}

	ax772a_data = kmalloc (sizeof(*ax772a_data), GFP_KERNEL);
	if (!ax772a_data) {
//		dbg ("Cannot allocate memory for AX88772A data");
		kfree (buf, 0);
		return -ENOMEM;
	}
	memset (ax772a_data, 0, sizeof(*ax772a_data));
//	data->ax772a_data = ax772a_data;

//	ax772a_data->ax_work = create_singlethread_workqueue ("ax88772a");
//	if (!ax772a_data->ax_work) {
//		kfree (ax772a_data);
//		kfree (buf);
//		return -ENOMEM;
//	}

	ax772a_data->dev = dev;
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
//	INIT_WORK (&ax772a_data->check_link, ax88772a_link_reset, dev);
//#else
//	INIT_WORK (&ax772a_data->check_link, ax88772a_link_reset);
//#endif

	/* reload eeprom data */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
			AXGPIOS_RSE, 0, 0, buf)) < 0)
		goto out2;

	wait_ms(5);

	/* Initialize MII structure */
	dev->mii.dev = &dev->net;
//	dev->mii.mdio_read = ax8817x_mdio_read_le;
//	dev->mii.mdio_write = ax8817x_mdio_write_le;
	dev->mii.phy_id_mask = 0xff;
	dev->mii.reg_num_mask = 0xff;

	/* Get the PHY id */
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_PHY_ID,
			0, 0, 2, buf)) < 0) {
//		dbg("Error reading PHY ID: %02x", ret);
		goto out2;
	} else if (ret < 2) {
		/* this should always return 2 bytes */
//		dbg("AX_CMD_READ_PHY_ID returned less than 2 bytes: ret=%02x",
//		    ret);
		goto out2;
	}
	dev->mii.phy_id = *((u8 *)buf + 1);

	if(dev->mii.phy_id != 0x10) {
//		dbg("Got wrong PHY ID: %02x", dev->mii.phy_id);
		goto out2;
	}

	/* select the embedded 10/100 Ethernet PHY */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SW_PHY_SELECT,
			AX_SWRESET_BZ | AX_SWRESET_RR, 0, 0, buf)) < 0) {
//		dbg("Select PHY #1 failed: %d", ret);
		goto out2;
	}

	/*
	 * set the embedded Ethernet PHY in power-up
	 * mode and operating state.
	 */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SW_RESET,
			AX_SWRESET_IPRL, 0, 0, buf)) < 0) {
//		dbg("Select PHY #1 failed: %d", ret);
		goto out2;
	}

	/*
	 * set the embedded Ethernet PHY in power-down
	 * mode and operating state.
	 */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SW_RESET,
			AX_SWRESET_IPPD | AX_SWRESET_IPRL, 0, 0, buf)) < 0) {
//		dbg("Select PHY #1 failed: %d", ret);
		goto out2;
	}

	wait_ms(10);

	/*
	 * set the embedded Ethernet PHY in power-up mode
	 * and operating state.
	 */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SW_RESET,
			AX_SWRESET_IPRL, 0, 0, buf)) < 0) {
//		dbg("Select PHY #1 failed: %d", ret);
		goto out2;
	}

	wait_ms(60);

	/* 
	 * set the embedded Ethernet PHY in power-up mode
	 * and reset state.
	 */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SW_RESET,
			AX_SWRESET_CLEAR, 0, 0, buf)) < 0) {
//		dbg("Select PHY #1 failed: %d", ret);
		goto out2;
	}

	/*
	 * set the embedded Ethernet PHY in power-up mode
	 * and operating state.
	 */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SW_RESET,
			AX_SWRESET_IPRL, 0, 0, buf)) < 0) {
//		dbg("Select PHY #1 failed: %d", ret);
		goto out2;
	}

	/* stop MAC operation */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL,
			AX_RX_CTL_STOP, 0, 0, buf)) < 0) {
//		dbg("Reset RX_CTL failed: %d", ret);
		goto out2;
	}

	/* Get the MAC address */
	memset(buf, 0, ETH_ALEN);
	if ((ret = ax8817x_read_cmd(dev, AX88772_CMD_READ_NODE_ID,
				0, 0, ETH_ALEN, buf)) < 0) {
//		dbg("Failed to read MAC address: %d", ret);
		goto out2;
	}
//	memcpy(dev->net->dev_addr, buf, ETH_ALEN);
	memcpy(dev->net.PhyAddr, buf, ETH_ALEN);
		
	memcpy(dev->net.PhyAddr, MyPhysNodeAddress, ETH_ALEN);
	memcpy(ULANMAC, MyPhysNodeAddress, ETH_ALEN);

	/* make sure the driver can enable sw mii operation */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SET_SW_MII,
			0, 0, 0, buf)) < 0) {
//		dbg("Enabling software MII failed: %d", ret);
		goto out2;
	}

//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
//	dev->net->do_ioctl = ax8817x_ioctl;
//	dev->net->set_multicast_list = ax8817x_set_multicast;
//#else
//	dev->net->netdev_ops = &ax88x72_netdev_ops;
//#endif

//	dev->net->ethtool_ops = &ax88772_ethtool_ops;

	ax8817x_mdio_write_le(&dev->net, dev->mii.phy_id, MII_BMCR, BMCR_RESET);
	ax8817x_mdio_write_le(&dev->net, dev->mii.phy_id, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP);

//	mii_nway_restart(&dev->mii);
	/* if autoneg is off, it's an error */
	bmcr = ax8817x_mdio_read_le(&dev->net, dev->mii.phy_id, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		bmcr |= BMCR_ANRESTART;
		ax8817x_mdio_write_le(&dev->net, dev->mii.phy_id, MII_BMCR, bmcr);
	}

//	ax772a_data->autoneg_start = jiffies;
//	ax772a_data->Event = WAIT_AUTONEG_COMPLETE;

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MEDIUM_MODE,
				AX88772_MEDIUM_DEFAULT, 0, 0, buf)) < 0) {
//		dbg("Write medium mode register: %d", ret);
		goto out2;
	}

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_IPG0,
				AX88772A_IPG0_DEFAULT | AX88772A_IPG1_DEFAULT << 8,
				AX88772A_IPG2_DEFAULT, 0, buf)) < 0) {
//		dbg("Write IPG,IPG1,IPG2 failed: %d", ret);
		goto out2;
	}

	/* Set RX_CTL to default values with 2k buffer, and enable cactus */
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL,
			(AX_RX_CTL_START | AX_RX_CTL_AB | AX_RX_CTL_AM | AX_RX_CTL_AMALL | AX_RX_CTL_PRO),
			0, 0, buf)) < 0) {
//		dbg("Reset RX_CTL failed: %d", ret);
		goto out2;
	}

//	/* Asix framing packs multiple eth frames into a 2K usb bulk transfer */
//	if (dev->driver_info->flags & FLAG_FRAMING_AX) {
//		/* hard_mtu  is still the default - the device does not support
//		   jumbo eth frames */
//		dev->rx_urb_size = 2048;
//	}

	kfree (buf, 0);

//	printk (version);
	return ret;
out2:
//	destroy_workqueue (ax772a_data->ax_work);
	kfree (ax772a_data, 0);
	kfree (buf, 0);
out1:
	return ret;
}

//Jesse
/* GPIO 0 .. 2 toggles */
#define AX_GPIO_GPO0EN		0x01	/* GPIO0 Output enable */
#define AX_GPIO_GPO_0		0x02	/* GPIO0 Output value */
#define AX_GPIO_GPO1EN		0x04	/* GPIO1 Output enable */
#define AX_GPIO_GPO_1		0x08	/* GPIO1 Output value */
#define AX_GPIO_GPO2EN		0x10	/* GPIO2 Output enable */
#define AX_GPIO_GPO_2		0x20	/* GPIO2 Output value */
#define AX_GPIO_RESERVED	0x40	/* Reserved */
#define AX_GPIO_RSE		0x80	/* Reload serial EEPROM */

/* AX88772 & AX88178 Medium Mode Register */
#define AX_MEDIUM_PF		0x0080
#define AX_MEDIUM_JFE		0x0040
#define AX_MEDIUM_TFC		0x0020
#define AX_MEDIUM_RFC		0x0010
#define AX_MEDIUM_ENCK		0x0008
#define AX_MEDIUM_AC		0x0004
#define AX_MEDIUM_FD		0x0002
#define AX_MEDIUM_GM		0x0001
#define AX_MEDIUM_SM		0x1000
#define AX_MEDIUM_SBP		0x0800
#define AX_MEDIUM_PS		0x0200
#define AX_MEDIUM_RE		0x0100

#define AX88772_MEDIUM_DEFAULT	\
	(AX_MEDIUM_FD | AX_MEDIUM_RFC | \
	 AX_MEDIUM_TFC | AX_MEDIUM_PS | \
	 AX_MEDIUM_AC | AX_MEDIUM_RE )


//we still can use ax8817x_write_cmd here
static int ax88772_bind(struct usbnet *dev, struct usb_device *intf)
{
	int ii=100,ij=250;
	int ret,embd_phy=0x0;
	u8 *buf;
	u16 buf16_1[1],actual_spm,medium_mode;
//	u16 *buf16 = (u16 *) buf;
	u16 *buf16 ;
	int i;
//celebi	struct ax8817x_data *data = (struct ax8817x_data *)dev->data;
	struct ax88772_data *ax772dataptr;
	 wait_ms(60);
	dev->in = usb_rcvbulkpipe(dev->udev, 2);  //the 88172 and 88772 in out endpoint is different
	dev->out = usb_sndbulkpipe(dev->udev, 3);     //but 88772 and 88178 is the same

	if(!(buf = kmalloc (6, GFP_KERNEL)))
		return -ENOMEM;
	buf16 = buf;
	// allocate irq urb
//celebi	if ((data->int_urb = usb_alloc_urb (0)) == 0) {
//celebi		dbg ("%s: cannot allocate interrupt URB",
//celebi			dev->net.name);
//celebi		return -ENOMEM;
//celebi	}
//celebi	if ((data->int_buf = kmalloc(AX_INTERRUPT_BUFSIZE, GFP_KERNEL)) == NULL) {
//celebi		dbg ("%s: cannot allocate memory for interrupt buffer",
//celebi			dev->net.name);
//celebi		usb_free_urb(data->int_urb);
//celebi		return -ENOMEM;
//celebi	}
//celebi	memset(data->int_buf, 0, AX_INTERRUPT_BUFSIZE);


//celebi	usb_fill_int_urb (data->int_urb, dev->udev,
//celebi		usb_rcvintpipe (dev->udev, 1),
//celebi		data->int_buf, AX_INTERRUPT_BUFSIZE,
//celebi		ax88772_interrupt_complete, dev,
//celebi		dev->udev->speed == USB_SPEED_HIGH ? 128 : 100);

	if (!(ax772dataptr = kmalloc (sizeof *ax772dataptr, GFP_KERNEL))) {
//celebi		dbg ("can't kmalloc dev");
		kfree(buf, 0);
		return -ENOMEM;
	}
	memset (ax772dataptr, 0, sizeof *ax772dataptr);
	dev->ax88772_data_ptr=ax772dataptr;
   //-------------------------------------------------------------------------
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
//Jesse			0x00B0,0, 0,
				AX_GPIO_RSE | AX_GPIO_GPO_2 | AX_GPIO_GPO2EN,0, 0,
				buf)) < 0){
		kfree(buf, 0);			
		return ret;
	}
	wait_ms(5);

//Jesse
	/* 0x10 is the phy id of the embedded 10/100 ethernet phy */
	ret = ax8817x_read_cmd(dev, AX_CMD_READ_PHY_ID, 0, 0, 2, buf);
	embd_phy = buf[1];
	embd_phy = ( embd_phy & 0x1f) == 0x10 ? 1 : 0;
/*	
	if ((ret = ax8817x_write_cmd(dev, 0x22, 0x0001, 0, 0, buf)) < 0) {
//celebi		dbg("select PHy register failed: %d", ret);
		return ret;
	}
*/
//Jesse
	if ((ret = ax8817x_write_cmd(dev, 0x22, embd_phy, 0, 0, buf)) < 0) {
//celebi		dbg("select PHy register failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}
/*	
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0040, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}
*/
//Jesse
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0048, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}

		wait_ms(150);
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}
		wait_ms(150);
/*
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0028, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}
*/
//Jesse
	if (embd_phy) {
		if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0020, 0, 0, buf)) < 0){
			kfree(buf, 0); 
			return ret;
		}
	}
	else{
		if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0004, 0, 0, buf)) < 0) {
			kfree(buf, 0);
			return ret;
		}

	}

		wait_ms(150);

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		kfree(buf, 0);
		return ret;				//CACTUS stop
	}

	/* Get the MAC address */
	memset(buf, 0, ETH_ALEN);
	if ((ret = ax8817x_read_cmd(dev, 0x13, 0, 0, 6, buf)) < 0) {
//celebi		dbg("read AX_CMD_READ_NODE_ID failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}
	memcpy(dev->net.PhyAddr, buf, ETH_ALEN);
		
	memcpy(dev->net.PhyAddr, MyPhysNodeAddress, ETH_ALEN);
	memcpy(ULANMAC, MyPhysNodeAddress, ETH_ALEN);

//Jesse
/*
	if ((ret = ax8817x_write_cmd(dev, 0x06, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("enable PHY reg. reading capability: %d", ret);
		return ret;				//enable Phy register reading capability
	}
	if ( ((ret = ax8817x_read_cmd(dev, 0x07, 0x0010, 0x0002, 0x0002, buf16_1)) < 0)
	    ||(buf16_1[0]!=0x003b)  ) {
//celebi		dbg("read PHY register 2 must be 0x3b00: %d", ret);
		return ret;				//read Phy register 2 must be 0x3b00
	}
*/

	/* Get the PHY id */
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_PHY_ID, 0, 0, 2, buf)) < 0) {
//celebi		dbg("error on read AX_CMD_READ_PHY_ID: %02x", ret);    //cmd 0x19
		kfree(buf, 0);
		return ret;
	} else if (ret < 2) {
		/* this should always return 2 bytes */
//celebi		dbg("AX_CMD_READ_PHY_ID returned less than 2 bytes: ret=%02x", ret);
		kfree(buf, 0);
		return -EIO;
	}

	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0008, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}
		wait_ms(150);
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0028, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}
	
	wait_ms(ij);
	
	/* Initialize MII structure */
	dev->mii.dev = &dev->net;
	dev->mii.mdio_read = ax8817x_mdio_read;
	dev->mii.mdio_write = ax8817x_mdio_write;
	dev->mii.phy_id_mask = 0x3f;
	dev->mii.reg_num_mask = 0x1f;
	dev->mii.phy_id = buf[1];

	if ((ret = ax8817x_write_cmd(dev, 0x06, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("enable PHY reg. reading capability: %d", ret);
		kfree(buf, 0);
		return ret;				//enable Phy register reading capability
	}

	ax8817x_write_cmd(dev, AX_CMD_SET_SW_MII, 0, 0, 0, &buf);


	*buf16 = cpu_to_le16(BMCR_RESET);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_BMCR, 2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg - MII_BMCR: %02x", ret);
		kfree(buf, 0);
		return ret;
	}
	
	ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf);
	
//Jesse
/*
	//wait for link establish
	for (i=0;i<3000;i++){
		if ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMSR, 2, buf16_1)) < 0) {
//celebi			dbg("error on read MII reg - MII_BMSR: %02x", ret);
			return ret;   //
		}
		if((*buf16_1&0x0004)!=0) break;
		wait_ms(10);
	}
	
	if  ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMCR, 2, buf16_1)) < 0) {
//celebi		dbg("error on read MII reg - MII_BMCR: %02x", ret);
		return ret;   //could be 0x0031
	}
*/	

	wait_ms(ii);

	ax8817x_write_cmd(dev, AX_CMD_SET_SW_MII, 0, 0, 0, &buf);

	actual_spm=*buf16_1;  //actual speed mode
	/* Advertise that we can do full-duplex pause */
	*buf16 = cpu_to_le16(ADVERTISE_ALL | ADVERTISE_CSMA | 0x0400);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_ADVERTISE,
				     2, buf16)) < 0) {
//celebi		dbg("Failed to write MII_REG advertisement: %02x", ret);
		kfree(buf, 0);
		return ret;   //could be 0xe105
	}

	ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf);


//	*buf16 = cpu_to_le16(*buf16_1 | BMCR_ANRESTART);
//	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
//				     dev->mii.phy_id, MII_BMCR,
//				     2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg autonegotiate: %02x", ret);
//		return ret;
//	}
	//check PHY REG 0 to see actual speed mode, then set the MAC   regiseter,
	//so at insert module time you can determine it.


	  medium_mode=0x0134;

	  switch (actual_spm&0x2100){
	  case 0x2100:
		 medium_mode|=0x0202;	  //100M,full
		 break;
	  case 0x2000:
		 medium_mode|=0x0200;	  //100M,half
		 break;
	  case 0x0100:
		 medium_mode|=0x0002;	  //10M,full
		 break;
	  default:
		 medium_mode|=0x0000;	  //10M,half
	  }


/*
	if ((ret = ax8817x_write_cmd(dev, 0x1b, medium_mode , 0, 0, buf)) < 0) {
//celebi		dbg("write medium mode register: %d", ret);
		return ret;
	}
*/
//Jesse
	if ((ret = ax8817x_write_cmd(dev, 0x1b, AX88772_MEDIUM_DEFAULT , 0, 0, buf)) < 0) {
		kfree(buf, 0);
		return ret;
	}

/*
	if ((ret = ax8817x_write_cmd(dev, 0x12, 0x0c15, 0x000e, 0, buf)) < 0) {
//celebi		dbg("write IPG,IPG1,IPG2 failed: %d", ret);
		return ret;
	}
*/

//Jesse
	if ((ret = ax8817x_write_cmd(dev, 0x12, 0x15 | 0x0c << 8, 0x12, 0, buf)) < 0) {
//celebi		dbg("write IPG,IPG1,IPG2 failed: %d", ret);
		kfree(buf, 0);
		return ret;
	}
	
	
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf)) < 0) {
//celebi		dbg("Failed to set hardware MII: %02x", ret);
		kfree(buf, 0);
		return ret;
	}

//Jesse
/*
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x0388, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		return ret;				//CACTUS stop
	}
*/
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x009B, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		kfree(buf, 0);
		return ret;				//CACTUS stop
	}
//celebi	dev->net.set_multicast_list = ax8817x_set_multicast;
//celebi	dev->net.ethtool_ops = &ax8817x_ethtool_ops;

//celebi	if((ret = usb_submit_urb(data->int_urb)) < 0) {
//celebi		dbg("Failed to submit interrupt URB: %02x", ret);
//celebi		usb_free_urb(data->int_urb);
//celebi		return ret;
//celebi	}
	kfree(buf, 0);
	return 0;

}


#if 0
//we still can use ax8817x_write_cmd here
static int ax88772_bind(struct usbnet *dev, struct usb_device *intf)
{
	int ret;
	u8 buf[6];
	u16 buf16_1[1],actual_spm,medium_mode;
	u16 *buf16 = (u16 *) buf;
	int i;
//celebi	struct ax8817x_data *data = (struct ax8817x_data *)dev->data;
	struct ax88772_data *ax772dataptr;
	 wait_ms(60);
	dev->in = usb_rcvbulkpipe(dev->udev, 2);  //the 88172 and 88772 in out endpoint is different
	dev->out = usb_sndbulkpipe(dev->udev, 3);     //but 88772 and 88178 is the same


	// allocate irq urb
//celebi	if ((data->int_urb = usb_alloc_urb (0)) == 0) {
//celebi		dbg ("%s: cannot allocate interrupt URB",
//celebi			dev->net.name);
//celebi		return -ENOMEM;
//celebi	}
//celebi	if ((data->int_buf = kmalloc(AX_INTERRUPT_BUFSIZE, GFP_KERNEL)) == NULL) {
//celebi		dbg ("%s: cannot allocate memory for interrupt buffer",
//celebi			dev->net.name);
//celebi		usb_free_urb(data->int_urb);
//celebi		return -ENOMEM;
//celebi	}
//celebi	memset(data->int_buf, 0, AX_INTERRUPT_BUFSIZE);


//celebi	usb_fill_int_urb (data->int_urb, dev->udev,
//celebi		usb_rcvintpipe (dev->udev, 1),
//celebi		data->int_buf, AX_INTERRUPT_BUFSIZE,
//celebi		ax88772_interrupt_complete, dev,
//celebi		dev->udev->speed == USB_SPEED_HIGH ? 128 : 100);

	if (!(ax772dataptr = kmalloc (sizeof *ax772dataptr, GFP_KERNEL))) {
//celebi		dbg ("can't kmalloc dev");
		return -ENOMEM;
	}
	memset (ax772dataptr, 0, sizeof *ax772dataptr);
	dev->ax88772_data_ptr=ax772dataptr;
   //-------------------------------------------------------------------------
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
				0x00B0,0, 0,
				buf)) < 0)
		return ret;
	wait_ms(5);

	if ((ret = ax8817x_write_cmd(dev, 0x22, 0x0001, 0, 0, buf)) < 0) {
//celebi		dbg("select PHy register failed: %d", ret);
		return ret;
	}

	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0040, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}
		wait_ms(150);
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}
		wait_ms(150);
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0028, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}

		wait_ms(150);

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		return ret;				//CACTUS stop
	}

	/* Get the MAC address */
	memset(buf, 0, ETH_ALEN);
	if ((ret = ax8817x_read_cmd(dev, 0x13, 0, 0, 6, buf)) < 0) {
//celebi		dbg("read AX_CMD_READ_NODE_ID failed: %d", ret);
		return ret;
	}
	memcpy(dev->net.PhyAddr, buf, ETH_ALEN);

	if ((ret = ax8817x_write_cmd(dev, 0x06, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("enable PHY reg. reading capability: %d", ret);
		return ret;				//enable Phy register reading capability
	}
	if ( ((ret = ax8817x_read_cmd(dev, 0x07, 0x0010, 0x0002, 0x0002, buf16_1)) < 0)
	    ||(buf16_1[0]!=0x003b)  ) {
//celebi		dbg("read PHY register 2 must be 0x3b00: %d", ret);
		return ret;				//read Phy register 2 must be 0x3b00
	}


	/* Get the PHY id */
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_PHY_ID, 0, 0, 2, buf)) < 0) {
//celebi		dbg("error on read AX_CMD_READ_PHY_ID: %02x", ret);    //cmd 0x19
		return ret;
	} else if (ret < 2) {
		/* this should always return 2 bytes */
//celebi		dbg("AX_CMD_READ_PHY_ID returned less than 2 bytes: ret=%02x", ret);
		return -EIO;
	}

	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0008, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}
		wait_ms(150);
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0028, 0, 0, buf)) < 0) {
//celebi		dbg("generate wave form failed: %d", ret);
		return ret;
	}
		wait_ms(150);
	/* Initialize MII structure */
	dev->mii.dev = &dev->net;
	dev->mii.mdio_read = ax8817x_mdio_read;
	dev->mii.mdio_write = ax8817x_mdio_write;
	dev->mii.phy_id_mask = 0x3f;
	dev->mii.reg_num_mask = 0x1f;
	dev->mii.phy_id = buf[1];


	*buf16 = cpu_to_le16(BMCR_RESET);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_BMCR, 2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg - MII_BMCR: %02x", ret);
		return ret;
	}

	//wait for link establish
	for (i=0;i<3000;i++){
		if ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMSR, 2, buf16_1)) < 0) {
//celebi			dbg("error on read MII reg - MII_BMSR: %02x", ret);
			return ret;   //
		}
		if((*buf16_1&0x0004)!=0) break;
		wait_ms(10);
	}

	if  ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMCR, 2, buf16_1)) < 0) {
//celebi		dbg("error on read MII reg - MII_BMCR: %02x", ret);
		return ret;   //could be 0x0031
	}

	actual_spm=*buf16_1;  //actual speed mode
	/* Advertise that we can do full-duplex pause */
	*buf16 = cpu_to_le16(ADVERTISE_ALL | ADVERTISE_CSMA | 0x0400);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_ADVERTISE,
				     2, buf16)) < 0) {
//celebi		dbg("Failed to write MII_REG advertisement: %02x", ret);
		return ret;   //could be 0xe105
	}

	*buf16 = cpu_to_le16(*buf16_1 | BMCR_ANRESTART);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_BMCR,
				     2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg autonegotiate: %02x", ret);
		return ret;
	}
	//check PHY REG 0 to see actual speed mode, then set the MAC   regiseter,
	//so at insert module time you can determine it.


	  medium_mode=0x0134;

	  switch (actual_spm&0x2100){
	  case 0x2100:
		 medium_mode|=0x0202;	  //100M,full
		 break;
	  case 0x2000:
		 medium_mode|=0x0200;	  //100M,half
		 break;
	  case 0x0100:
		 medium_mode|=0x0002;	  //10M,full
		 break;
	  default:
		 medium_mode|=0x0000;	  //10M,half
	  }


	if ((ret = ax8817x_write_cmd(dev, 0x1b, medium_mode , 0, 0, buf)) < 0) {
//celebi		dbg("write medium mode register: %d", ret);
		return ret;
	}
//	if ((ret = ax8817x_write_cmd(dev, 0x12, 0x0c15, 0x000e, 0, buf)) < 0) {
if ((ret = ax8817x_write_cmd(dev, 0x12, 0x15 | 0x0c, 0x12, 0, buf)) < 0) {
//celebi		dbg("write IPG,IPG1,IPG2 failed: %d", ret);
		return ret;
	}
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &buf)) < 0) {
//celebi		dbg("Failed to set hardware MII: %02x", ret);
		return ret;
	}

	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x009B, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		return ret;				//CACTUS stop
	}
//celebi	dev->net.set_multicast_list = ax8817x_set_multicast;
//celebi	dev->net.ethtool_ops = &ax8817x_ethtool_ops;

//celebi	if((ret = usb_submit_urb(data->int_urb)) < 0) {
//celebi		dbg("Failed to submit interrupt URB: %02x", ret);
//celebi		usb_free_urb(data->int_urb);
//celebi		return ret;
//celebi	}
	return 0;

}
#endif //0

static int mediacheck(struct usbnet *dev)
{
	int ret,fullduplex;
	u16 phylinkstatus1,phylinkstatus2,tempshort;
   //	if ((ret = ax8817x_write_cmd(dev, 0x06, 0, 0, 0, buf)) < 0) {
   //		dbg("enable PHY writing ability failed: %d", ret);
   //		return ret; // ???
  //	}
  //	wait_ms(1);	 //maybe no need to do becoz already enable in main ax88178_bind
	struct ax88178_data *ax178dataptr = (struct ax88178_data *)dev->ax88178_data_ptr;

	if ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 0x05, 2, &phylinkstatus1)) < 0) {
//celebi		dbg("error on reading MII register 5 failed: %02x", ret);
		return ret;   //
	}
	if ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 0x0a, 2, &phylinkstatus2)) < 0) {
//celebi		dbg("error on reading MII register 0x0a failed: %02x", ret);
		return ret;   //
	}

	if(ax178dataptr->PhyMode==0){ //1st generation Marvel PHY
		if(ax178dataptr->LedMode==1){
		if ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 25, 2, &tempshort)) < 0) {
//celebi			dbg("error on reading MII register 0x19 failed: %02x", ret);
			return ret;   //
		}
			tempshort &=0xfc0f;
		}
	}
	fullduplex=1;
	if(phylinkstatus2 &(0x0800|0x0400)){  //1000BT full duplex
		ax178dataptr->MediaLink=0x0001|0x0002|0x0008|0x0100;
			if(ax178dataptr->PhyMode==0){
				if(ax178dataptr->LedMode==1){
				tempshort|=0x3e0;
				}
			}
	}else if(phylinkstatus1 &0x0100){  //100BT full duplex
		ax178dataptr->MediaLink=0x0002|0x0100|0x0200;
			if(ax178dataptr->PhyMode==0){
				if(ax178dataptr->LedMode==1){
					tempshort|=0x3b0;
				}
			}
	}else if(phylinkstatus1 &0x0080){   //100BT half duplex
		ax178dataptr->MediaLink=(0x0100|0x0200);
		fullduplex=0;
			if(ax178dataptr->PhyMode==0){
				if(ax178dataptr->LedMode==1){
					tempshort|=0x3b0;
				}
		}
	}else if(phylinkstatus1 &0x0040){
		ax178dataptr->MediaLink=(0x0002|0x0100);
		if(ax178dataptr->PhyMode==0){
			if(ax178dataptr->LedMode==1){
				tempshort|=0x02f0;
			}
		}
	}else{

		ax178dataptr->MediaLink=0x0100;
		fullduplex=0;
		if(ax178dataptr->PhyMode==0){
			if(ax178dataptr->LedMode==1){
				tempshort|=0x02f0;
			}
		}
	}

	if(ax178dataptr->PhyMode==0){
		if(ax178dataptr->LedMode==1){
		     if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)dev->mii.phy_id, 25, 0x0002, &tempshort)) < 0){
//celebi			     dbg("error on writing MII register 0x19 failed: %02x", ret);
			     return ret;
		     }
		}
	}
	ax178dataptr->MediaLink|=0x0004;
	if(ax178dataptr->UseRgmii!=0)
		ax178dataptr->MediaLink|=0x0008;
	if(fullduplex){
		ax178dataptr->MediaLink|=0x0020;  //ebable tx flow control as default;
		ax178dataptr->MediaLink|=0x0010;  //ebable rx flow control as default;
	}

}
static void media_func772(unsigned long param)
{
//celebi
#if 0
	struct usbnet		*dev = (struct usbnet *) param;
    //	struct ax8817x_data *data = (struct ax8817x_data *)dev->data;
	struct ax88772_data *ax772dataptr = (struct ax88772_data *)dev->ax88772_data_ptr;

	  switch (ax772dataptr->nx_state){
	  case 0:
		 return;
	  case 1:
		 goto ax772_case_1;
	  case 2:
		 goto ax772_case_2;
	  case 3:
		 goto ax772_case_3;
	  case 4:
		 goto ax772_case_4;
	  case 5:
		 goto ax772_case_5;
	  case 6:
		 goto ax772_case_6;
	  case 7:
		 goto ax772_case_7;
	  case 8:
		 goto ax772_case_8;
	  case 970:
		 goto ax772_case_970;
	  default:
		 return;
	  }
    //------------------------------------------------------------------------
    // ax88178_read_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,u16 size, void *data)
 ax772_case_1:
	ax772dataptr->nx_state=2;
	ax88772_write_cmd_async(dev, 0x06, 0x0000, 0, 0, ax772dataptr->buf);
	return; 			    //enable Phy register reading capability
     // if ( ((ret = ax8817x_read_cmd_async(dev, 0x07, 0x0010, 0x0002, 0x0002, ax772dataptr->buf16_1)) < 0)
     //     ||(buf16_1[0]!=0x003b)  ) {
     // 	dbg("read PHY register 2 must be 0x3b00: %d", ret);
     // 	return ret;				//read Phy register 2 must be 0x3b00
     // }
 ax772_case_2:
	ax772dataptr->nx_state=3;
	ax88772_read_cmd_async(dev, 0x09, 0x0000, 0x0000, 0x0001, ax772dataptr->buf16_1);
	return;
 ax772_case_3:
	if((ax772dataptr->buf16_1[0]&0x0001)!=0x0001){
	return; 				     //software must be allowed to access PHY reg.
	}

	for (ax772dataptr->loopcnt = 0; ax772dataptr->loopcnt < 200; ax772dataptr->loopcnt++) {
	    ax772dataptr->nx_state=4;
	    ax88772_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMCR, 2, ax772dataptr->buf16_1);
	    return;					 //could be 0x0031,save as actual speed mode
ax772_case_4:
	}

//	ax772dataptr->nx_state=5;
	ax772dataptr->actual_spm=*ax772dataptr->buf16_1;  //actual speed mode
//	/* Advertise that we can do full-duplex pause */
//	ax772dataptr->buf16_2[0] = cpu_to_le16(ADVERTISE_ALL | ADVERTISE_CSMA | 0x0400);
//	ax88772_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, MII_ADVERTISE,2, ax772dataptr->buf16_2);
//	return; 					      //write MII_REG advertisement,could be 0xe105
ax772_case_5:
//	ax772dataptr->nx_state=6;
//	ax772dataptr->buf16_2[0] = cpu_to_le16(*ax772dataptr->buf16_1 | BMCR_ANRESTART);
//	ax88772_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, MII_BMCR,2,ax772dataptr->buf16_2);
//	return; 					//write MII reg autonegotiate.
ax772_case_6:
	ax772dataptr->nx_state=7;
	ax88772_write_cmd_async(dev, AX_CMD_SET_HW_MII, 0, 0, 0, &ax772dataptr->buf);
	return; 				     // set hardware MII
ax772_case_7:
	//check PHY REG 0 to see actual speed mode, then set the MAC   regiseter,
	//so at insert module time you can determine it.
	  ax772dataptr->medium_mode=0x0336;
	  switch (ax772dataptr->actual_spm&0x2100){
	  case 0x2100:
		 ax772dataptr->medium_mode&=0x0336;	//100M,full
		 break;
	  case 0x2000:
		 ax772dataptr->medium_mode&=0x0304;	//100M,half
		 break;
	  case 0x0100:
		 ax772dataptr->medium_mode&=0x0136;	//10M,full
		 break;
	  default:
		 ax772dataptr->medium_mode&=0x0104;	//10M,half
	  }
	ax772dataptr->nx_state=8;
	ax88772_write_cmd_async(dev, 0x1b, ax772dataptr->medium_mode , 0, 0, ax772dataptr->buf);
	return; 			  //write medium mode register.
ax772_case_8:
	ax772dataptr->nx_state=970;
	ax88772_read_cmd_async(dev, 0x1a, 0, 0, 0x0002, ax772dataptr->buf);
	return; 			  //read medium status register.
ax772_case_970:

#endif
	return;
}
static void media_func(unsigned long param)
{
//celebi
#if 0
	struct usbnet		*dev = (struct usbnet *) param;
    //	ax88178_read_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,u16 size, void *data)
    // ax88178_read_cmd_async(struct usbnet *dev, u8 cmd, u16 value, u16 index,u16 size, void *data)
	struct ax88178_data *
ax178dataptr = (struct ax88178_data *)dev->ax88178_data_ptr;
	  switch (ax178dataptr->nx_state){
	  case 0:
		 goto case_0;
	  case 1:
		 goto case_1;
	  case 2:
		 goto case_2;
	  case 3:
		 goto case_3;
	  case 41:
		 goto case_41;
	  case 42:
		 goto case_42;
	  case 43:
		 goto case_43;
	  case 5:
		 goto case_5;
//	  case 51:
//		 goto case_51;
	  case 6:
		 goto case_6;
//	  case 61:
//		 goto case_61;
//	  case 7:
//		 goto case_7;
	  case 8:
		 goto case_8;
	  case 81:
		 goto case_81;
	  case 9:
		 goto case_9;
	  case 91:
		 goto case_91;
	  case 92:
		 goto case_92;
	  case 93:
		 goto case_93;
	  case 100:
		 goto case_100;
	  case 110:
		 goto case_110;
	  case 120:
		 goto case_120;
	  case 130:
		 goto case_130;
	  case 140:
		 goto case_140;
	  case 150:
		 goto case_150;
	  case 160:
		 goto case_160;
	  case 970:
		 goto case_970;
	  default:
		 return;
	  }
case_0:
	//wait for link establish
	for (ax178dataptr->i=0;ax178dataptr->i<30000;ax178dataptr->i++){
	ax178dataptr->nx_state=1;
		ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMSR, 2, ax178dataptr->buf16_1);
		return;

case_1:
	ax178dataptr->nx_state=2;
		if((*ax178dataptr->buf16_1&0x0004)!=0){
		break;
		}
	}

	ax178dataptr->nx_state=2;
	ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMCR, 2, &ax178dataptr->phyctrl);
	return;   //could be 0x0000
case_2:
	ax178dataptr->nx_state=3;
	ax178dataptr->tempshort=ax178dataptr->phyctrl;
	ax178dataptr->phyctrl &=~(0x0800|0x0400);
	if(ax178dataptr->phyctrl != ax178dataptr->tempshort){
	     ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, MII_BMCR, 2, &ax178dataptr->phyctrl);
	     return;
	}
case_3:
	if(ax178dataptr->PhyMode==0){  //MARVELL 1st generation  phy
	     if(ax178dataptr->LedMode==1){
	       ax178dataptr->nx_state=41;
	       ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 24, 2, &ax178dataptr->phyreg);
	       return;
case_41:
	       ax178dataptr->phyreg &= 0xf8ff;
	       ax178dataptr->phyreg |= (1+0x100);
	       ax178dataptr->nx_state=42;
	       ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 24,2,&ax178dataptr->phyreg);
	       return;
case_42:
	       ax178dataptr->nx_state=43;
	       ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 25, 2, &ax178dataptr->phyreg);
	       return;
case_43:
	       ax178dataptr->phyreg &=0xfc0f;
	     }else if(ax178dataptr->LedMode==2){
	       ax178dataptr->nx_state=5;
	       ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 24, 2, &ax178dataptr->phyreg);
	       return;
case_5:
	       ax178dataptr->phyreg &= 0xf886;
	       ax178dataptr->phyreg |= (1+0x10+0x300);
	       ax178dataptr->nx_state=81;
	       ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 24,2,&ax178dataptr->phyreg);
	       return;
//case_51:
	     }else if(ax178dataptr->LedMode==5){
	       ax178dataptr->nx_state=6;
	       ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 24, 2, &ax178dataptr->phyreg);
	       return;
case_6:
	       ax178dataptr->phyreg &= 0xf8be;
	       ax178dataptr->phyreg |= (1+0x40+0x300);
	       ax178dataptr->nx_state=81;
	       ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 24,2,&ax178dataptr->phyreg);
	       return;
//case_61:
	     }
	}else if(ax178dataptr->PhyMode==2){  //AGERE 1st generation  phy
	     if(ax178dataptr->LedMode==4){
	       ax178dataptr->phyreg |= 0xca10;
	       ax178dataptr->nx_state=81;
	       ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 28,2,&ax178dataptr->phyreg);
	       return;
	     }
//case_7:
	}else if(ax178dataptr->PhyMode==1){  //CICADA phy
	     if(ax178dataptr->LedMode==3){
	       ax178dataptr->nx_state=8;
	       ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 27, 2, &ax178dataptr->phyreg);
	       return;
case_8:
	       ax178dataptr->phyreg &= 0xfcff;
	       ax178dataptr->phyreg |= 0x0100;
	       ax178dataptr->nx_state=81;
	       ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 27,2,&ax178dataptr->phyreg);
	       return;
	     }
	}
case_81:
	if(ax178dataptr->PhyMode==0){
		if(ax178dataptr->LedMode==1)
		   ax178dataptr->phyreg |=0x3f0;
	}

	ax178dataptr->phyctrl&= 0xfbff; //~0x0400=~GMII_CONTROL_LOOPBACK;
	ax178dataptr->phyanar=1+(0x0400|0x0100|0x0080|0x0040|0x0020);
	ax178dataptr->phyauxctrl=0x0200; //1000M and full duplex
	ax178dataptr->nx_state=9;
	ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 4,2,&ax178dataptr->phyanar);
	return;
case_9:
	ax178dataptr->nx_state=91;
	ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 9,2,&ax178dataptr->phyauxctrl);
	return;
case_91:
	ax178dataptr->phyctrl|=(0x1000|0x0200);
	ax178dataptr->nx_state=92;
	ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 0,2,&ax178dataptr->phyctrl);
	return;
case_92:
	if(ax178dataptr->PhyMode==0){
		if(ax178dataptr->LedMode==1)
		ax178dataptr->nx_state=93;
		ax88178_write_cmd_async(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 25,2,&ax178dataptr->phyreg);
		return;
	}
case_93:
	ax178dataptr->nx_state=100;
	ax88178_write_cmd_async(dev, 0x06, 0, 0, 0,ax178dataptr->buf);
	return;
case_100:
	ax178dataptr->nx_state=110;
	ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 0x05, 2, &ax178dataptr->phylinkstatus1);
	return;
case_110:
	ax178dataptr->nx_state=120;
	ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 0x0a, 2, &ax178dataptr->phylinkstatus2);
	return;
case_120:
	ax178dataptr->nx_state=130;
	if(ax178dataptr->PhyMode==0){ //1st generation Marvel PHY
		if(ax178dataptr->LedMode==1){
		ax88178_read_cmd_async(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 25, 2, &ax178dataptr->tempshort);
		return;
case_130:
			ax178dataptr->nx_state=140;
			ax178dataptr->tempshort &=0xfc0f;
		}
	}
	ax178dataptr->nx_state=140;
	ax178dataptr->fullduplex=1;
	if(ax178dataptr->phylinkstatus2 &(0x0800|0x0400)){  //1000BT full duplex
		ax178dataptr->MediaLink=0x0001|0x0002|0x0008|0x0100;
			if(ax178dataptr->PhyMode==0){
				if(ax178dataptr->LedMode==1){
				ax178dataptr->tempshort|=0x3e0;
				}
			}
	}else if(ax178dataptr->phylinkstatus1 &0x0100){  //100BT full duplex
		ax178dataptr->MediaLink=0x0002|0x0100|0x0200;
			if(ax178dataptr->PhyMode==0){
				if(ax178dataptr->LedMode==1){
					ax178dataptr->tempshort|=0x3b0;
				}
			}
	}else if(ax178dataptr->phylinkstatus1 &0x0080){   //100BT half duplex
		ax178dataptr->MediaLink=(0x0100|0x0200);
		ax178dataptr->fullduplex=0;
			if(ax178dataptr->PhyMode==0){
				if(ax178dataptr->LedMode==1){
					ax178dataptr->tempshort|=0x3b0;
				}
		}
	}else if(ax178dataptr->phylinkstatus1 &0x0040){
		ax178dataptr->MediaLink=(0x0002|0x0100);
		if(ax178dataptr->PhyMode==0){
			if(ax178dataptr->LedMode==1){
				ax178dataptr->tempshort|=0x02f0;
			}
		}
	}else{

		ax178dataptr->MediaLink=0x0100;
		ax178dataptr->fullduplex=0;
		if(ax178dataptr->PhyMode==0){
			if(ax178dataptr->LedMode==1){
				ax178dataptr->tempshort|=0x02f0;
			}
		}
	}

	if(ax178dataptr->PhyMode==0){
		if(ax178dataptr->LedMode==1){
		     ax88178_write_cmd_async(dev, 0x08, (u8)dev->mii.phy_id, 25, 0x0002, &ax178dataptr->tempshort);
		     return;
		}
	}
case_140:
			ax178dataptr->nx_state=150;
	ax178dataptr->MediaLink|=0x0004;
	if(ax178dataptr->UseRgmii!=0)
		ax178dataptr->MediaLink|=0x0008;
	if(ax178dataptr->fullduplex){
		ax178dataptr->MediaLink|=0x0020;  //ebable tx flow control as default;
		ax178dataptr->MediaLink|=0x0010;  //ebable rx flow control as default;
	}
	ax88178_write_cmd_async(dev, 27, ax178dataptr->MediaLink, 0, 0, ax178dataptr->buf);
	return;
case_150:
//	  ax178dataptr->nx_state=160;
//	  ax88178_write_cmd_async(dev, 18, 0x0c15, 0x000e, 0, ax178dataptr->buf);
//	  return;
case_160:
	ax178dataptr->nx_state=970;
	ax88178_write_cmd_async(dev, 10, 0, 0, 0, ax178dataptr->buf);
	return;
case_970:     //970 always is the end label
//	  ax178dataptr->nx_state=10;
//	  ax88178_write_cmd_async(dev, 16, 0x0388, 0, 0, ax178dataptr->buf);
//	  return;
//case_9:

#endif //celebi
	return;
}
static int ax88178_bind(struct usbnet *dev, struct usb_device *intf)
{
	int ret;
	u8 buf[6];
	u16 *buf16 = (u16 *) buf;
	u16 buf16_1[1],EepromData,PhyID,PhyPatch,phyreg,phyctrl;
	u16 TempShort,phyanar,phyauxctrl;
	int i;
	int   UseGpio0;
//celebi	struct ax8817x_data *data = (struct ax8817x_data *)dev->data;
	struct ax88178_data *ax178dataptr;

	dev->in = usb_rcvbulkpipe(dev->udev, 2);      //the 88172 and 88772 in out endpoint is different
	dev->out = usb_sndbulkpipe(dev->udev, 3);     //but 88772 and 88178 is the same


	// allocate irq urb
//celebi	if ((data->int_urb = usb_alloc_urb (0)) == 0) {
//celebi		dbg ("%s: cannot allocate interrupt URB",
//celebi			dev->net.name);
//celebi		return -ENOMEM;
//celebi	}

//celebi	if ((data->int_buf = kmalloc(AX_INTERRUPT_BUFSIZE, GFP_KERNEL)) == NULL) {
//celebi		dbg ("%s: cannot allocate memory for interrupt buffer",
//celebi			dev->net.name);
//celebi		usb_free_urb(data->int_urb);
//celebi		return -ENOMEM;
//celebi	}
//celebi	memset(data->int_buf, 0, AX_INTERRUPT_BUFSIZE);

//celebi	usb_fill_int_urb (data->int_urb, dev->udev,
//celebi		usb_rcvintpipe (dev->udev, 1),
//celebi		data->int_buf, AX_INTERRUPT_BUFSIZE,
//celebi		ax88178_interrupt_complete, dev,
//celebi		dev->udev->speed == USB_SPEED_HIGH ? 128 : 100);


	if (!(ax178dataptr = kmalloc (sizeof *ax178dataptr, GFP_KERNEL))) {
//celebi		dbg ("can't kmalloc dev");
		return -ENOMEM;
	}
	memset (ax178dataptr, 0, sizeof *ax178dataptr);

	dev->ax88178_data_ptr=ax178dataptr;

	//ax8817x_write_cmd(dev, 0x20, 0x0040, 0, 0, buf) sample...

	if ((ret = ax8817x_write_cmd(dev, 0x0d, 0, 0, 0, buf)) < 0) {
//celebi		dbg("enable SROM reading failed: %d", ret);
		return ret;   // ???
	}

	if ((ret = ax8817x_read_cmd(dev, 0x0b, 0x0017, 0, 2, buf16_1)) < 0) {
//celebi		dbg("read SROM address 17h failed: %d", ret);
		return ret;
	}
	EepromData=buf16_1[0];
	if (EepromData ==0xffff)
	{
		ax178dataptr->PhyMode  = 0;//PHY_MODE_MARVELL;
		ax178dataptr->LedMode  = 0;
		UseGpio0 = 1; //True
	}
	else
	{
		ax178dataptr->PhyMode  = (u8)(EepromData&7);
		ax178dataptr->LedMode  = (u8)(EepromData>>8);
		if(EepromData&0x80)
			UseGpio0=0; //MARVEL se and other
		else
			UseGpio0=1; //cameo
	}

	if ((ret = ax8817x_write_cmd(dev, 0x0e, 0, 0, 0, buf)) < 0) {
//celebi		dbg("disable SROM reading failed: %d", ret);
		return ret; // ???
	}

	if(UseGpio0)
	{
		if(ax178dataptr->PhyMode==0)   //phy mode  marvell
		{
			if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
					0x0081,0, 0,
					buf)) < 0){
//celebi			dbg("write GPIO failed: %d", ret);
			return ret;
			}
			wait_ms(25);
			if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
					0x0031,0, 0,
					buf)) < 0){
//celebi			dbg("write GPIO failed: %d", ret);
			return ret;
			}
			wait_ms(25);
			if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
					0x0011,0, 0,
					buf)) < 0){
//celebi			dbg("write GPIO failed: %d", ret);
			return ret;
			}
			wait_ms(245);
			if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
					0x0031,0, 0,
					buf)) < 0){
//celebi			dbg("write GPIO failed: %d", ret);
			return ret;
			}
		}else{
			if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,
					0x0083,0, 0,
					buf)) < 0){
//celebi			dbg("write GPIO failed: %d", ret);
			return ret;
			}
		}
	}else{
			if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,0x008c,0,0,buf)) < 0){
//celebi			dbg("write GPIO failed: %d", ret);
			return ret;
			}
			if(ax178dataptr->PhyMode!= 1){
			       wait_ms(25);
				if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,0x003c,0,0,buf)) < 0){
//celebi				dbg("write GPIO failed: %d", ret);
				return ret;
				}
				wait_ms(25);
				if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,0x001c,0,0,buf)) < 0){
//celebi				dbg("write GPIO failed: %d", ret);
				return ret;
				}
				wait_ms(245);
				if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,0x003c,0,0,buf)) < 0){
//celebi				dbg("write GPIO failed: %d", ret);
				return ret;
				}

			}else if(ax178dataptr->PhyMode==1){
				wait_ms(350);
				if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,0x0004,0,0,buf)) < 0){
//celebi				dbg("write GPIO failed: %d", ret);
				return ret;
				}
				wait_ms(350);
				if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_GPIOS,0x000c,0,0,buf)) < 0){
//celebi				dbg("write GPIO failed: %d", ret);
				return ret;
				}
			}
	}
	if ((ret = ax8817x_write_cmd(dev, 0x22, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("write S/W reset failed: %d", ret);
		return ret;
	}
	wait_ms(150);
	if ((ret = ax8817x_write_cmd(dev, 0x20, 0x0048, 0, 0, buf)) < 0) {
//celebi		dbg("write S/W reset failed: %d", ret);
		return ret;
	}

	wait_ms(150);

//Jesse
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_RX_CTL, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("send AX_CMD_WRITE_RX_CTL failed: %d", ret);
		return ret;				//stop rcv
	}

	wait_ms(150);

	/* Get the MAC address */
	memset(buf, 0, ETH_ALEN);
	if ((ret = ax8817x_read_cmd(dev, 0x13, 0, 0, 6, buf)) < 0) {
//celebi		dbg("read AX_CMD_READ_NODE_ID failed: %d", ret);
		return ret;
	}
	memcpy(dev->net.PhyAddr, buf, ETH_ALEN);
//-----from here we will do phy initialization,when patch fail we just do a return-----------------------------------

	if ((ret = ax8817x_write_cmd(dev, 0x06, 0x0000, 0, 0, buf)) < 0) {
//celebi		dbg("enable PHY reg. access capability: %d", ret);
		return ret;				//enable Phy register access capability
	}


	/* Get the PHY id */
	if ((ret = ax8817x_read_cmd(dev, AX_CMD_READ_PHY_ID, 0, 0, 2, &PhyID)) < 0) {
//celebi		dbg("error on read AX_CMD_READ_PHY_ID: %02x", ret);
		return ret;
	} else if (ret < 2) {
		/* this should always return 2 bytes */
//celebi		dbg("AX_CMD_READ_PHY_ID returned less than 2 bytes: ret=%02x", ret);
		return -EIO;
	}
	PhyID>>=8;
	PhyID&=0x1f;
	if(!UseGpio0){
	   i=100;
	   while(i--){
	       if ( ( (ret = ax8817x_read_cmd(dev, 0x07, (u8)PhyID, 0x0002, 0x0002, buf16_1)) < 0)
		   ||!( (buf16_1[0]==0x000f)||(buf16_1[0]==0x0141)||(buf16_1[0]==0x0282) )  ) {
//celebi		       dbg("read PHY register 2 must be 0x00ff or 0x0141 or 0x0282 failed: %d", ret);
		       return ret;			       //read Phy register 2 must be 3 of them
	       }
	       break; //means OUI match, so it is ok.
	   }
	   if(i==0)
	   return ret; //OUI not match so it is wrong.
	   wait_ms(5);
	}
	ax178dataptr->UseRgmii=0;  //false
	if(ax178dataptr->PhyMode==0){	   //1st generation MARVEL phy

	       if ( (ret = ax8817x_read_cmd(dev, 0x07, (u8)PhyID, 0x001b, 0x0002, &phyreg)) < 0){
//celebi		       dbg("read register reg 27 failed: %d", ret);
		       return ret;
	       }    //read phy register
	       if(!(phyreg&4)){
		  ax178dataptr->UseRgmii=1;
		  PhyPatch=0x0082;
		  if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 20, 0x0002, &PhyPatch)) < 0) return ret;
		   ax178dataptr->MediaLink |= 0x0008;
	       }
	}else if(ax178dataptr->PhyMode==2){    //AGERE 1st version phy
	       ax178dataptr->UseRgmii=1;
	       ax178dataptr->MediaLink |=0x0008;// MEDIUM_ENABLE_125MHZ;
	       PhyPatch=0x800;
	       if( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch) ) < 0) return ret;
	       PhyPatch=0x0007;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 18, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8805;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xb03e;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8808;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xe110;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8806;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xb03e;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8807;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xff00;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x880e;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xb4d3;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x880f;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xb4d3;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8810;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xb4d3;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8817;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x1c00;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x300d;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0001;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 17, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0002;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 18, 0x0002, &PhyPatch)) < 0) return ret;
	}else if(ax178dataptr->PhyMode==1){    //CICADA 1st version phy
	       ax178dataptr->UseRgmii=1;
	       ax178dataptr->MediaLink |=0x0008;// MEDIUM_ENABLE_125MHZ;

	       PhyPatch=0x0001;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x1c25;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 23, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x234c;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 16, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0212;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xa7fa;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0012;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x3002;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x87fa;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xafac;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x000d;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;

	       PhyPatch=0x001c;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;

	       PhyPatch=0x8fac;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0012;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0400;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 20, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0212;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xa760;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0000;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xfaff;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8760;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xa760;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0000;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xfaff;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8760;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0xafae;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0004;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0671;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8fae;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0012;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0000;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	}else if(ax178dataptr->PhyMode==5){	//CICADA 2nd version phy
	       ax178dataptr->UseRgmii=1;
	       ax178dataptr->MediaLink |= 0x0008;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0212;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x000f;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x472a;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x8fa4;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0212;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0000;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	}else if(ax178dataptr->PhyMode==9){	//CICADA 2nd version for ASIX
	       ax178dataptr->UseRgmii=1;
	       ax178dataptr->MediaLink |= 0x0008;

	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;

	       PhyPatch=0x0212;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0012;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x3002;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;

	       PhyPatch=0x87fa;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x52b5;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x000f;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 2, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x472a;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 1, 0x0002, &PhyPatch)) < 0) return ret;

	       PhyPatch=0x8fa4;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 0, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x2a30;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0212;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 8, 0x0002, &PhyPatch)) < 0) return ret;
	       PhyPatch=0x0000;
	       if ( (ret = ax8817x_write_cmd(dev, 0x08, (u8)PhyID, 31, 0x0002, &PhyPatch)) < 0) return ret;
	}

	/* Initialize MII structure */
	dev->mii.dev = &dev->net;
	dev->mii.mdio_read = ax8817x_mdio_read;
	dev->mii.mdio_write = ax8817x_mdio_write;
	dev->mii.phy_id_mask = 0x3f;
	dev->mii.reg_num_mask = 0x1f;
	dev->mii.phy_id = (u8)PhyID;



	*buf16 = cpu_to_le16(BMCR_RESET);
	if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
				     dev->mii.phy_id, MII_BMCR, 2, buf16)) < 0) {
//celebi		dbg("Failed to write MII reg - MII_BMCR: %02x", ret);
		return ret;
	} //software reset

	//wait for link establish
	for (i=0;i<3000;i++){
		if ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMSR, 2, buf16_1)) < 0) {
	//celebi		dbg("error on reading MII reg - MII_BMSR: %02x", ret);
			return ret;   //
		}
		if((*buf16_1&BMSR_LSTATUS)!=0) break;
	       wait_ms(10);
	}

	if  ((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , MII_BMCR, 2, &phyctrl)) < 0) {
//celebi		dbg("error on read MII reg - MII_BMCR: %02x", ret);
		return ret;   //could be 0x0000
	}

	TempShort=phyctrl;
	phyctrl &=~(BMCR_PDOWN|BMCR_ISOLATE);
	if(phyctrl != TempShort){
	     if ((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,
					  dev->mii.phy_id, MII_BMCR, 2, &phyctrl)) < 0) {
//celebi		     dbg("Failed to write MII reg - MII_BMCR: %02x", ret);
		     return ret;
	     }
	}

	if(ax178dataptr->PhyMode==0){  //MARVELL 1st generation  phy
	     if(ax178dataptr->LedMode==1){
	       if((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 24, 2, &phyreg))< 0) return ret;
	       phyreg &= 0xf8ff;
	       phyreg |= (1+0x100);
	       if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 24,2,&phyreg))< 0) return ret;
	       if((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 25, 2, &phyreg))< 0) return ret;
	       phyreg &=0xfc0f;
	     }else if(ax178dataptr->LedMode==2){
	       if((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 24, 2, &phyreg))< 0) return ret;
	       phyreg &= 0xf886;
	       phyreg |= (1+0x10+0x300);
	       if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 24,2,&phyreg))< 0) return ret;
	     }else if(ax178dataptr->LedMode==5){
	       if((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 24, 2, &phyreg))< 0) return ret;
	       phyreg &= 0xf8be;
	       phyreg |= (1+0x40+0x300);
	       if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 24,2,&phyreg))< 0) return ret;
	     }
	}else if(ax178dataptr->PhyMode==2){  //AGERE 1st generation  phy
	     if(ax178dataptr->LedMode==4){
	       phyreg |= 0xca10;
	       if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 28,2,&phyreg))< 0) return ret;
	     }
	}else if(ax178dataptr->PhyMode==1){  //CICADA phy
	     if(ax178dataptr->LedMode==3){
	       if((ret = ax8817x_read_cmd(dev,AX_CMD_READ_MII_REG,dev->mii.phy_id , 27, 2, &phyreg))< 0) return ret;
	       phyreg &= 0xfcff;
	       phyreg |= 0x0100;
	       if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 27,2,&phyreg))< 0) return ret;
	     }
	}

	if(ax178dataptr->PhyMode==0){
		if(ax178dataptr->LedMode==1)
		   phyreg |=0x3f0;
	}

	phyctrl&= ~BMCR_ISOLATE;
	phyanar=1+(0x0400|ADVERTISE_100FULL|ADVERTISE_100HALF|ADVERTISE_10FULL|ADVERTISE_10HALF);
	phyauxctrl=0x0200; //1000M and full duplex

	if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 4,2,&phyanar))< 0) return ret;
	if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 9,2,&phyauxctrl))< 0) return ret;
	phyctrl|=(BMCR_ANENABLE|BMCR_ANRESTART);
	if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 0,2,&phyctrl))< 0) return ret;

	if(ax178dataptr->PhyMode==0){
		if(ax178dataptr->LedMode==1)
		   if((ret = ax8817x_write_cmd(dev, AX_CMD_WRITE_MII_REG,dev->mii.phy_id, 25,2,&phyreg))< 0) return ret;
	}



	//remember to add a few lines to get real medialink value to se the medium mode register

     // if ((ret = ax8817x_write_cmd(dev, 27, 0x037e, 0, 0, buf)) < 0) {
     // 	dbg("write mode medium reg failed: %d", ret);
     // 	return ret;
     // }
	wait_ms(6000);
	mediacheck(dev);
	if ((ret = ax8817x_write_cmd(dev, 27, ax178dataptr->MediaLink, 0, 0, buf)) < 0) {
//celebi		dbg("write mode medium reg failed: %d", ret);
		return ret;
	}
	if ((ret = ax8817x_write_cmd(dev, 18, 0x0c15, 0x000e, 0, buf)) < 0) {
//celebi		dbg("write IPG IPG1 IPG2 reg failed: %d", ret);
		return ret;
	}
	if ((ret = ax8817x_write_cmd(dev, 10, 0, 0, 0, buf)) < 0) {
//celebi		dbg("disable PHY access failed: %d", ret);
		return ret;
	}

//Jesse
/*
	if ((ret = ax8817x_write_cmd(dev, 16, 0x0388, 0, 0, buf)) < 0) {
//celebi		dbg("write RX ctrl reg failed: %d", ret);
		return ret;
	}
*/
	if ((ret = ax8817x_write_cmd(dev, 16, 0x009B, 0, 0, buf)) < 0) {
//celebi		dbg("write RX ctrl reg failed: %d", ret);
		return ret;
	}
//celebi	dev->net.set_multicast_list = ax8817x_set_multicast;
//celebi	dev->net.ethtool_ops = &ax8817x_ethtool_ops;

//celebi	if((ret = usb_submit_urb(data->int_urb)) < 0) {
//celebi		dbg("Failed to submit interrupt URB: %02x", ret);
//celebi		usb_free_urb(data->int_urb);
//celebi		return ret;
//celebi	}

	return 0;
}
//celebi
#if 0
static int ax88772_rx_fixup (struct usbnet *dev, struct sk_buff *skb)
{
	u32	*header;
	char	*packet;
	struct sk_buff		*ax_skb;
	u16			size;

	header = (u32 *)skb->data;

	// get the packet count of the received skb
	le32_to_cpus (header); //in 386 machine actually do nothing
	//we can't check if exceeding max packet count or negative number here.

	// set the current packet pointer to the first packet
	packet = (char *)(header+1);  //move on 4 bytes header then reach the data ptr.

	//retrive from head and decrement the length for the packet count size 4 bytes
	skb_pull (skb, 4);

	while (skb->len > 0) {
		if((short)(*header&0x0000ffff)!=~( (short)((*header&0xffff0000)>>16) ))
		{
//celebi			dbg ("ax88772: header length data is error");
		}

		// get the packet length
		size = (u16)(*header&0x0000ffff);


		if((skb->len)-((size+1)&0xfffe)==0)
		return 2;
	      //goto last_packet;
		// this may be a broken packet
		if (size > 1514) {  //ask ?? if 1514 is correct
//celebi			dbg ("ax88772: invalid rx length %d", size);
			return 0;
		}

		// allocate the skb for the individual packet
		ax_skb = skb_clone(skb, GFP_ATOMIC);
		if (ax_skb) {
			// copy the packet data to the new skb
			//memcpy(skb_put(ax_skb, size), packet, size);
			ax_skb->len=size;
			ax_skb->data=packet;
			ax_skb->tail=packet+size;
			skb_return (dev, ax_skb); //give new skb to upper layer
		}else{
		return 0;
		}

		// shift the data pointer to the next header
		skb_pull (skb, (size+1)&0xfffe); //just pull even number data bytes

		if(skb->len==0)
		break;

		header =(u32 *) skb->data;
		le32_to_cpus (header); //in 386 machine actually do nothing
		// get next packet and move data ptr to it
		packet=(char *)(header+1);
		skb_pull(skb,4);
	}



	  if (skb->len <0) {
//celebi		  dbg ("ax88772: invalid rx length %d", skb->len);
		  return 0;
	  }
	return 1;
}
#endif
//celebi
static int ax88772_rx_fixup (struct usbnet *dev, char *packet_ptr, int leng)
{
    unsigned long		*header;
	char	*packet;
	u16		size;
	
	header = packet_ptr;
//	size = leng;

	// set the current packet pointer to the first packet
	packet = (char *)(header+1);  //move on 4 bytes header then reach the data ptr.
	
	while( leng > 0)
	{
		if( (short)(*header&0x0000ffff) != ~((short)((*header&0xffff0000)>>16)) )
		{
//celebi			dbg ("ax88772: header length data is error");
		}

		// get the packet length
		size = (u16)(*header&0x0000ffff);
		
		if((leng)-((size+1)&0xfffe)==0)
		return 2;
		
		// this may be a broken packet
		if (size > 1514) {  //ask ?? if 1514 is correct
//celebi			dbg ("ax88772: invalid rx length %d", size);
			return 0;
		}
		
		UlanRecv(packet, size);	//zot
		
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += size;

		header =(u32 *) ( packet+((size+1)&0xfffe));
		// get next packet and move data ptr to it
		packet=(char *)(header+1);
		leng -= (size+1)&0xfffe;
		leng -= 4;
			
	}
	return 1;
}
//celebi
#if 0
static struct sk_buff *
ax88772_tx_fixup (struct usbnet *dev, struct sk_buff *skb, int flags)
{
	int	padlen;   //used in packet size 512 byte case,we have to add 4 byte 0x0000ffff as null packet
	int	headroom = skb_headroom (skb);
	int	tailroom = skb_tailroom (skb);
	u32	*packet_len;
	u32	*padbytes_ptr;

	 padlen =((skb->len+4)%512)?0:4;

	if ((!skb_cloned (skb)) //not cloned and enough space
			&& ((headroom + tailroom) >= (4+padlen))) {
		if ((headroom < 4)||(tailroom<padlen)){
			skb->data = memmove (skb->head + 4,
					     skb->data, skb->len);
			skb->tail = skb->data + skb->len;
		}
	} else {  //it's cloned or not enough space to reserve 4 bytes at head then allocate new skb
		struct sk_buff	*skb2;
		skb2 = skb_copy_expand (skb, 4 , padlen, flags); //only expand 4 bytes in the head
		dev_kfree_skb_any (skb);
		skb = skb2;
		if (!skb)
			return NULL;
	}


	// expand data area 4 bytes in the head
	packet_len=(u32 *) skb_push (skb, 4 ); //shift left data pointer 4 bytes,update len and return it


	//attach real 4 byte data
	packet_len =(u32 *)skb->data;
	*packet_len = (((skb->len-4)^0x0000ffff)<<16)+(skb->len-4); //format is !(HH)!(LL)HHLL; 0x0000HHLL is the skb->len


	if ((skb->len%512)==0){
	    padbytes_ptr=(u32 *)skb->tail;
	    *padbytes_ptr=0xffff0000;
	    skb_put(skb,padlen);
	}
	return skb;
}
#endif
//celebi
static char *
ax88772_tx_fixup (struct usbnet *dev, char *buf, int *len)
{
	int	padlen;   //used in packet size 512 byte case,we have to add 4 byte 0x0000ffff as null packet
	unsigned long	*packet_len;
	unsigned long	*padbytes_ptr;
	char *buf_ptr=dev->tx_buff;
	unsigned long temp;
	unsigned long count=*len;

	padlen =((count+4)%512)?0:4;

	//attach real 4 byte data
	packet_len =(unsigned long *)dev->tx_buff;
	temp = (((unsigned long)(count)^(unsigned long)(0x0000ffff))<<16);
	*packet_len = temp+(unsigned long)(count); //format is !(HH)!(LL)HHLL; 0x0000HHLL is the skb->len
	
	if( padlen )
	{
		buf_ptr = buf_ptr+count+4; 
		padbytes_ptr = (unsigned long *)buf_ptr;
		*padbytes_ptr=0xffff0000;
	}
	
	*len = count+4+padlen;
	return ;
	
}

//clelebi
#if 0
static int ax88178_rx_fixup (struct usbnet *dev, struct sk_buff *skb)
{
	u32	*header;
	char	*packet;
	struct sk_buff		*ax_skb;
	u16			size;

	header = (u32 *)skb->data;

	// get the packet count of the received skb
	le32_to_cpus (header); //in 386 machine actually do nothing
	//we can't check if exceeding max packet count or negative number here.

	// set the current packet pointer to the first packet
	packet = (char *)(header+1);  //move on 4 bytes header then reach the data ptr.

	//retrive from head and decrement the length for the packet count size 4 bytes
	skb_pull (skb, 4);

	while (skb->len > 0) {
		if((short)(*header&0x0000ffff)!=~( (short)((*header&0xffff0000)>>16) ))
		{
//celebi			dbg ("ax88772: header length data is error");
		}

		// get the packet length
		size = (u16)(*header&0x0000ffff);


		if((skb->len)-((size+1)&0xfffe)==0)
		return 2;
	      //goto last_packet;
		// this may be a broken packet
		if (size > 1514) {  //ask ?? if 1514 is correct
//celebi			dbg ("ax88772: invalid rx length %d", size);
			return 0;
		}

		// allocate the skb for the individual packet
		ax_skb = skb_clone(skb, GFP_ATOMIC);
		if (ax_skb) {
			// copy the packet data to the new skb
			//memcpy(skb_put(ax_skb, size), packet, size);
			ax_skb->len=size;
			ax_skb->data=packet;
			ax_skb->tail=packet+size;
			skb_return (dev, ax_skb); //give new skb to upper layer
		}else{
		return 0;
		}

		// shift the data pointer to the next header
		skb_pull (skb, (size+1)&0xfffe); //just pull even number data bytes

		if(skb->len==0)
		break;

		header =(u32 *) skb->data;
		le32_to_cpus (header); //in 386 machine actually do nothing
		// get next packet and move data ptr to it
		packet=(char *)(header+1);
		skb_pull(skb,4);
	}



	  if (skb->len <0) {
//celebi		  dbg ("ax88772: invalid rx length %d", skb->len);
		  return 0;
	  }
	return 1;
}
#endif 

//clelebi
#if 0
static struct sk_buff *
ax88178_tx_fixup (struct usbnet *dev, struct sk_buff *skb, int flags)
{
	int	padlen;   //used in packet size 512 byte case,we have to add 4 byte 0x0000ffff as null packet
	int	headroom = skb_headroom (skb);
	int	tailroom = skb_tailroom (skb);
	u32	*packet_len;
	u32	*padbytes_ptr;

	 padlen =((skb->len+4)%512)?0:4;

	if ((!skb_cloned (skb)) //not cloned and enough space
			&& ((headroom + tailroom) >= (4+padlen))) {
		if ((headroom < 4)||(tailroom<padlen)){
			skb->data = memmove (skb->head + 4,
					     skb->data, skb->len);
			skb->tail = skb->data + skb->len;
		}
	} else {  //it's cloned or not enough space to reserve 4 bytes at head then allocate new skb
		struct sk_buff	*skb2;
		skb2 = skb_copy_expand (skb, 4 , padlen, flags); //only expand 4 bytes in the head
		dev_kfree_skb_any (skb);
		skb = skb2;
		if (!skb)
			return NULL;
	}


	// expand data area 4 bytes in the head
	packet_len=(u32 *) skb_push (skb, 4 ); //shift left data pointer 4 bytes,update len and return it


	//attach real 4 byte data
	packet_len =(u32 *)skb->data;
	*packet_len = (((skb->len-4)^0x0000ffff)<<16)+(skb->len-4); //format is !(HH)!(LL)HHLL; 0x0000HHLL is the skb->len


	if ((skb->len%512)==0){
	    padbytes_ptr=(u32 *)skb->tail;
	    *padbytes_ptr=0xffff0000;
	    skb_put(skb,padlen);
	}
	return skb;
}
#endif

/* //celebi
static const struct driver_info ax8817x_info = {
	.description = "ASIX AX8817x USB 2.0 Ethernet",
	.bind = ax8817x_bind,
	.flags =  FLAG_ETHER,
	.data = 0x00130103,
};
static const struct driver_info dlink_dub_e100_info = {
	.description = "DLink DUB-E100 USB Ethernet",
	.bind = ax8817x_bind,
	.flags =  FLAG_ETHER,
	.data = 0x009f9d9f,
};

static const struct driver_info netgear_fa120_info = {
	.description = "Netgear FA-120 USB Ethernet",
	.bind = ax8817x_bind,
	.flags =  FLAG_ETHER,
	.data = 0x00130103,
};

static const struct driver_info hawking_uf200_info = {
	.description = "Hawking UF200 USB Ethernet",
	.bind = ax8817x_bind,
	.flags =  FLAG_ETHER,
	.data = 0x001f1d1f,
};

static const struct driver_info ax88772_info = {
	.description = "ASIX AX88772 USB 2.0 Ethernet",
	.bind = ax88772_bind,
	.flags =  FLAG_ETHER|FLAG_FRAMING_AX,
	.rx_fixup =	ax88772_rx_fixup,
	.tx_fixup =	ax88772_tx_fixup,
	.data = 0x00130103, //useless here
};
static const struct driver_info ax88178_info = {
	.description = "ASIX AX88178 USB 2.0 Ethernet",
	.bind = ax88178_bind,
	.flags =  FLAG_ETHER|FLAG_FRAMING_AX,
	.rx_fixup =	ax88178_rx_fixup,
	.tx_fixup =	ax88178_tx_fixup,
	.data = 0x00130103,  //useless here
};
*/
static const struct driver_info ax8817x_info = {
	description : "ASIX AX8817x USB 2.0 Ethernet",
	bind : ax8817x_bind,
	flags :  FLAG_ETHER,
	data : 0x00130103,
};
static const struct driver_info dlink_dub_e100_info = {
	description : "DLink DUB-E100 USB Ethernet",
	bind : ax8817x_bind,
	flags :  FLAG_ETHER,
	data : 0x009f9d9f,
};

static const struct driver_info netgear_fa120_info = {
	description : "Netgear FA-120 USB Ethernet",
	bind : ax8817x_bind,
	flags :  FLAG_ETHER,
	data : 0x00130103,
};

static const struct driver_info hawking_uf200_info = {
	description : "Hawking UF200 USB Ethernet",
	bind : ax8817x_bind,
	flags :  FLAG_ETHER,
	data : 0x001f1d1f,
};

static const struct driver_info ax88772_info = {
	description : "ASIX AX88772 USB 2.0 Ethernet",
	bind : ax88772_bind,
	flags :  FLAG_ETHER|FLAG_FRAMING_AX,
	rx_fixup :	ax88772_rx_fixup,
	tx_fixup :	ax88772_tx_fixup,
	data : 0x00130103, //useless here
};
static const struct driver_info ax88178_info = {
	description : "ASIX AX88178 USB 2.0 Ethernet",
	bind : ax88178_bind,
	flags :  FLAG_ETHER|FLAG_FRAMING_AX,
	rx_fixup :	ax88772_rx_fixup,
	tx_fixup :	ax88772_tx_fixup,
	data : 0x00130103,  //useless here
};
static const struct driver_info ax88772a_info = {
	.description = "ASIX AX88772A USB 2.0 Ethernet",
	.bind = ax88772a_bind,
//	.link_reset = ax88772a_link_reset,
	.flags = FLAG_ETHER | FLAG_FRAMING_AX,
	.rx_fixup = ax88772_rx_fixup,
	.tx_fixup = ax88772_tx_fixup,
	.data = 0x00130103,
};
#endif /* CONFIG_USB_AX8817X */


#ifdef	CONFIG_USB_BELKIN

/*-------------------------------------------------------------------------
 *
 * Belkin F5U104 ... two NetChip 2280 devices + Atmel microcontroller
 *
 * ... also two eTEK designs, including one sold as "Advance USBNET"
 *
 *-------------------------------------------------------------------------*/

static const struct driver_info belkin_info = {
	.description =	"Belkin, eTEK, or compatible",
};

#endif	/* CONFIG_USB_BELKIN */



#ifdef	CONFIG_USB_EPSON2888

/*-------------------------------------------------------------------------
 *
 * EPSON USB clients
 *
 * This is the same idea as Linux PDAs (below) except the firmware in the
 * device might not be Tux-powered.  Epson provides reference firmware that
 * implements this interface.  Product developers can reuse or modify that
 * code, such as by using their own product and vendor codes.
 *
 *-------------------------------------------------------------------------*/

static const struct driver_info epson2888_info = {
	.description =	"Epson USB Device",
	.check_connect = always_connected,

	.in = 4, .out = 3,
	.epsize = 64,
};

#endif	/* CONFIG_USB_EPSON2888 */


#ifdef CONFIG_USB_GENESYS

/*-------------------------------------------------------------------------
 *
 * GeneSys GL620USB-A (www.genesyslogic.com.tw)
 *
 * ... should partially interop with the Win32 driver for this hardware
 * The GeneSys docs imply there's some NDIS issue motivating this framing.
 *
 * Some info from GeneSys:
 *  - GL620USB-A is full duplex; GL620USB is only half duplex for bulk.
 *    (Some cables, like the BAFO-100c, use the half duplex version.)
 *  - For the full duplex model, the low bit of the version code says
 *    which side is which ("left/right").
 *  - For the half duplex type, a control/interrupt handshake settles
 *    the transfer direction.  (That's disabled here, partially coded.)
 *    A control URB would block until other side writes an interrupt.
 *
 *-------------------------------------------------------------------------*/

// control msg write command
#define GENELINK_CONNECT_WRITE			0xF0
// interrupt pipe index
#define GENELINK_INTERRUPT_PIPE 		0x03
// interrupt read buffer size
#define INTERRUPT_BUFSIZE			0x08
// interrupt pipe interval value
#define GENELINK_INTERRUPT_INTERVAL		0x10
// max transmit packet number per transmit
#define GL_MAX_TRANSMIT_PACKETS 		32
// max packet length
#define GL_MAX_PACKET_LEN			1514
// max receive buffer size
#define GL_RCV_BUF_SIZE 	\
	(((GL_MAX_PACKET_LEN + 4) * GL_MAX_TRANSMIT_PACKETS) + 4)

struct gl_packet {
	u32		packet_length;
	char		packet_data [1];
};

struct gl_header {
	u32			packet_count;
	struct gl_packet	packets;
};

#ifdef	GENLINK_ACK

// FIXME:  this code is incomplete, not debugged; it doesn't
// handle interrupts correctly.  interrupts should be generic
// code like all other device I/O, anyway.

struct gl_priv {
	struct urb	*irq_urb;
	char		irq_buf [INTERRUPT_BUFSIZE];
};

static inline int gl_control_write (struct usbnet *dev, u8 request, u16 value)
{
	int retval;

	retval = usb_control_msg (dev->udev,
		      usb_sndctrlpipe (dev->udev, 0),
		      request,
		      USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		      value,
		      0,			// index
		      0,			// data buffer
		      0,			// size
		      CONTROL_TIMEOUT_JIFFIES);
	return retval;
}

static void gl_interrupt_complete (struct urb *urb)
{
	int status = urb->status;

	switch (status) {
	case 0:
		/* success */
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* this urb is terminated, clean up */
//celebi		dbg("%s - urb shutting down with status: %d",
//celebi				__FUNCTION__, status);
		return;
	default:
//celebi		dbg("%s - nonzero urb status received: %d",
//celebi				__FUNCTION__, urb->status);
	}

	// NOTE:  2.4 still has automagic resubmit, so this would be
	// wrong ... but this code has never worked, is always disabled.
	status = usb_submit_urb (urb);
	if (status)
		err ("%s - usb_submit_urb failed with result %d",
		     __FUNCTION__, status);
}

static int gl_interrupt_read (struct usbnet *dev)
{
	struct gl_priv	*priv = dev->priv_data;
	int		retval;

	// issue usb interrupt read
	if (priv && priv->irq_urb) {
		// submit urb
		if ((retval = SUBMIT_URB (priv->irq_urb, GFP_KERNEL)) != 0)
			dbg ("gl_interrupt_read: submit fail - %X...", retval);
		else
			dbg ("gl_interrupt_read: submit success...");
	}

	return 0;
}

// check whether another side is connected
static int genelink_check_connect (struct usbnet *dev)
{
	int			retval;

//celebi	dbg ("genelink_check_connect...");

	// detect whether another side is connected
	if ((retval = gl_control_write (dev, GENELINK_CONNECT_WRITE, 0)) != 0) {
//celebi		dbg ("%s: genelink_check_connect write fail - %X",
			dev->net.name, retval);
		return retval;
	}

	// usb interrupt read to ack another side
	if ((retval = gl_interrupt_read (dev)) != 0) {
//celebi		dbg ("%s: genelink_check_connect read fail - %X",
			dev->net.name, retval);
		return retval;
	}

//celebi	dbg ("%s: genelink_check_connect read success", dev->net.name);
	return 0;
}

// allocate and initialize the private data for genelink
static int genelink_init (struct usbnet *dev)
{
	struct gl_priv *priv;

	// allocate the private data structure
	if ((priv = kmalloc (sizeof *priv, GFP_KERNEL)) == 0) {
		dbg ("%s: cannot allocate private data per device",
			dev->net.name);
		return -ENOMEM;
	}

	// allocate irq urb
	if ((priv->irq_urb = ALLOC_URB (0, GFP_KERNEL)) == 0) {
		dbg ("%s: cannot allocate private irq urb per device",
			dev->net.name);
		kfree (priv, 0);
		return -ENOMEM;
	}

	// fill irq urb
	usb_fill_int_urb (priv->irq_urb, dev->udev,
		usb_rcvintpipe (dev->udev, GENELINK_INTERRUPT_PIPE),
		priv->irq_buf, INTERRUPT_BUFSIZE,
		gl_interrupt_complete, 0,
		GENELINK_INTERRUPT_INTERVAL);

	// set private data pointer
	dev->priv_data = priv;

	return 0;
}

// release the private data
static int genelink_free (struct usbnet *dev)
{
	struct gl_priv	*priv = dev->priv_data;

	if (!priv)
		return 0;

// FIXME:  can't cancel here; it's synchronous, and
// should have happened earlier in any case (interrupt
// handling needs to be generic)

	// cancel irq urb first
	usb_unlink_urb (priv->irq_urb);

	// free irq urb
	usb_free_urb (priv->irq_urb);

	// free the private data structure
	kfree (priv, 0);

	return 0;
}

#endif

static int genelink_rx_fixup (struct usbnet *dev, struct sk_buff *skb)
{
	struct gl_header	*header;
	struct gl_packet	*packet;
	struct sk_buff		*gl_skb;
	int			status;
	u32			size;

	header = (struct gl_header *) skb->data;

	// get the packet count of the received skb
	le32_to_cpus (&header->packet_count);
	if ((header->packet_count > GL_MAX_TRANSMIT_PACKETS)
			|| (header->packet_count < 0)) {
		dbg ("genelink: illegal received packet count %d",
			header->packet_count);
		return 0;
	}

	// set the current packet pointer to the first packet
	packet = &header->packets;

	// decrement the length for the packet count size 4 bytes
	skb_pull (skb, 4);

	while (header->packet_count > 1) {
		// get the packet length
		size = packet->packet_length;

		// this may be a broken packet
		if (size > GL_MAX_PACKET_LEN) {
			dbg ("genelink: illegal rx length %d", size);
			return 0;
		}

		// allocate the skb for the individual packet
		gl_skb = alloc_skb (size, GFP_ATOMIC);
		if (gl_skb) {

			// copy the packet data to the new skb
			memcpy (gl_skb->data, packet->packet_data, size);

			// set skb data size
			gl_skb->len = size;
			gl_skb->dev = &dev->net;

			// determine the packet's protocol ID
			gl_skb->protocol = eth_type_trans (gl_skb, &dev->net);

			// update the status
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += size;

			// notify os of the received packet
            /*
			status = netif_rx (gl_skb);
            */
		}

		// advance to the next packet
		packet = (struct gl_packet *)
			&packet->packet_data [size];
		header->packet_count--;

		// shift the data pointer to the next gl_packet
		skb_pull (skb, size + 4);
	}

	// skip the packet length field 4 bytes
	skb_pull (skb, 4);

	if (skb->len > GL_MAX_PACKET_LEN) {
		dbg ("genelink: illegal rx length %d", skb->len);
		return 0;
	}
	return 1;
}

static struct sk_buff *
genelink_tx_fixup (struct usbnet *dev, struct sk_buff *skb, int flags)
{
	int	padlen;
	int	length = skb->len;
	int	headroom = skb_headroom (skb);
	int	tailroom = skb_tailroom (skb);
	u32	*packet_count;
	u32	*packet_len;

	// FIXME:  magic numbers, bleech
	padlen = ((skb->len + (4 + 4*1)) % 64) ? 0 : 1;

	if ((!skb_cloned (skb))
			&& ((headroom + tailroom) >= (padlen + (4 + 4*1)))) {
		if ((headroom < (4 + 4*1)) || (tailroom < padlen)) {
			skb->data = memmove (skb->head + (4 + 4*1),
					     skb->data, skb->len);
			skb->tail = skb->data + skb->len;
		}
	} else {
		struct sk_buff	*skb2;
		skb2 = skb_copy_expand (skb, (4 + 4*1) , padlen, flags);
		dev_kfree_skb_any (skb);
		skb = skb2;
	}

	// attach the packet count to the header
	packet_count = (u32 *) skb_push (skb, (4 + 4*1));
	packet_len = packet_count + 1;

	// FIXME little endian?
	*packet_count = 1;
	*packet_len = length;

	// add padding byte
	if ((skb->len % dev->maxpacket) == 0)
		skb_put (skb, 1);

	return skb;
}

static const struct driver_info genelink_info = {
	.description =	"Genesys GeneLink",
	.flags =	FLAG_FRAMING_GL | FLAG_NO_SETINT,
	.rx_fixup =	genelink_rx_fixup,
	.tx_fixup =	genelink_tx_fixup,

	.in = 1, .out = 2,
	.epsize =64,

#ifdef	GENELINK_ACK
	.check_connect =genelink_check_connect,
#endif
};

#endif /* CONFIG_USB_GENESYS */



#ifdef	CONFIG_USB_NET1080

/*-------------------------------------------------------------------------
 *
 * Netchip 1080 driver ... http://www.netchip.com
 * Used in LapLink cables
 *
 *-------------------------------------------------------------------------*/

/*
 * NetChip framing of ethernet packets, supporting additional error
 * checks for links that may drop bulk packets from inside messages.
 * Odd USB length == always short read for last usb packet.
 *	- nc_header
 *	- Ethernet header (14 bytes)
 *	- payload
 *	- (optional padding byte, if needed so length becomes odd)
 *	- nc_trailer
 *
 * This framing is to be avoided for non-NetChip devices.
 */

struct nc_header {		// packed:
	u16	hdr_len;		// sizeof nc_header (LE, all)
	u16	packet_len;		// payload size (including ethhdr)
	u16	packet_id;		// detects dropped packets
#define MIN_HEADER	6

	// all else is optional, and must start with:
	// u16	vendorId;		// from usb-if
	// u16	productId;
} __attribute__((__packed__));

#define PAD_BYTE	((unsigned char)0xAC)

struct nc_trailer {
	u16	packet_id;
} __attribute__((__packed__));

// packets may use FLAG_FRAMING_NC and optional pad
#define FRAMED_SIZE(mtu) (sizeof (struct nc_header) \
				+ sizeof (struct ethhdr) \
				+ (mtu) \
				+ 1 \
				+ sizeof (struct nc_trailer))

#define MIN_FRAMED	FRAMED_SIZE(0)


/*
 * Zero means no timeout; else, how long a 64 byte bulk packet may be queued
 * before the hardware drops it.  If that's done, the driver will need to
 * frame network packets to guard against the dropped USB packets.  The win32
 * driver sets this for both sides of the link.
 */
#define NC_READ_TTL_MS	((u8)255)	// ms

/*
 * We ignore most registers and EEPROM contents.
 */
#define REG_USBCTL	((u8)0x04)
#define REG_TTL 	((u8)0x10)
#define REG_STATUS	((u8)0x11)

/*
 * Vendor specific requests to read/write data
 */
#define REQUEST_REGISTER	((u8)0x10)
#define REQUEST_EEPROM		((u8)0x11)

static int
nc_vendor_read (struct usbnet *dev, u8 req, u8 regnum, u16 *retval_ptr)
{
	int status = usb_control_msg (dev->udev,
		usb_rcvctrlpipe (dev->udev, 0),
		req,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		0, regnum,
		retval_ptr, sizeof *retval_ptr,
		CONTROL_TIMEOUT_JIFFIES);
	if (status > 0)
		status = 0;
	if (!status)
		le16_to_cpus (retval_ptr);
	return status;
}

static inline int
nc_register_read (struct usbnet *dev, u8 regnum, u16 *retval_ptr)
{
	return nc_vendor_read (dev, REQUEST_REGISTER, regnum, retval_ptr);
}

// no retval ... can become async, usable in_interrupt()
static void
nc_vendor_write (struct usbnet *dev, u8 req, u8 regnum, u16 value)
{
	usb_control_msg (dev->udev,
		usb_sndctrlpipe (dev->udev, 0),
		req,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value, regnum,
		0, 0,			// data is in setup packet
		CONTROL_TIMEOUT_JIFFIES);
}

static inline void
nc_register_write (struct usbnet *dev, u8 regnum, u16 value)
{
	nc_vendor_write (dev, REQUEST_REGISTER, regnum, value);
}


#if 0
static void nc_dump_registers (struct usbnet *dev)
{
	u8	reg;
	u16	*vp = kmalloc (sizeof (u16));

	if (!vp) {
		dbg ("no memory?");
		return;
	}

	dbg ("%s registers:", dev->net.name);
	for (reg = 0; reg < 0x20; reg++) {
		int retval;

		// reading some registers is trouble
		if (reg >= 0x08 && reg <= 0xf)
			continue;
		if (reg >= 0x12 && reg <= 0x1e)
			continue;

		retval = nc_register_read (dev, reg, vp);
		if (retval < 0)
			dbg ("%s reg [0x%x] ==> error %d",
				dev->net.name, reg, retval);
		else
			dbg ("%s reg [0x%x] = 0x%x",
				dev->net.name, reg, *vp);
	}
	kfree (vp, 0);
}
#endif


/*-------------------------------------------------------------------------*/

/*
 * Control register
 */

#define USBCTL_WRITABLE_MASK	0x1f0f
// bits 15-13 reserved, r/o
#define USBCTL_ENABLE_LANG	(1 << 12)
#define USBCTL_ENABLE_MFGR	(1 << 11)
#define USBCTL_ENABLE_PROD	(1 << 10)
#define USBCTL_ENABLE_SERIAL	(1 << 9)
#define USBCTL_ENABLE_DEFAULTS	(1 << 8)
// bits 7-4 reserved, r/o
#define USBCTL_FLUSH_OTHER	(1 << 3)
#define USBCTL_FLUSH_THIS	(1 << 2)
#define USBCTL_DISCONN_OTHER	(1 << 1)
#define USBCTL_DISCONN_THIS	(1 << 0)

static inline void nc_dump_usbctl (struct usbnet *dev, u16 usbctl)
{
#ifdef DEBUG
	devdbg (dev, "net1080 %s-%s usbctl 0x%x:%s%s%s%s%s;"
			" this%s%s;"
			" other%s%s; r/o 0x%x",
		dev->udev->bus->bus_name, dev->udev->devpath,
		usbctl,
		(usbctl & USBCTL_ENABLE_LANG) ? " lang" : "",
		(usbctl & USBCTL_ENABLE_MFGR) ? " mfgr" : "",
		(usbctl & USBCTL_ENABLE_PROD) ? " prod" : "",
		(usbctl & USBCTL_ENABLE_SERIAL) ? " serial" : "",
		(usbctl & USBCTL_ENABLE_DEFAULTS) ? " defaults" : "",

		(usbctl & USBCTL_FLUSH_OTHER) ? " FLUSH" : "",
		(usbctl & USBCTL_DISCONN_OTHER) ? " DIS" : "",
		(usbctl & USBCTL_FLUSH_THIS) ? " FLUSH" : "",
		(usbctl & USBCTL_DISCONN_THIS) ? " DIS" : "",
		usbctl & ~USBCTL_WRITABLE_MASK
		);
#endif
}

/*-------------------------------------------------------------------------*/

/*
 * Status register
 */

#define STATUS_PORT_A		(1 << 15)

#define STATUS_CONN_OTHER	(1 << 14)
#define STATUS_SUSPEND_OTHER	(1 << 13)
#define STATUS_MAILBOX_OTHER	(1 << 12)
#define STATUS_PACKETS_OTHER(n) (((n) >> 8) && 0x03)

#define STATUS_CONN_THIS	(1 << 6)
#define STATUS_SUSPEND_THIS	(1 << 5)
#define STATUS_MAILBOX_THIS	(1 << 4)
#define STATUS_PACKETS_THIS(n)	(((n) >> 0) && 0x03)

#define STATUS_UNSPEC_MASK	0x0c8c
#define STATUS_NOISE_MASK	((u16)~(0x0303|STATUS_UNSPEC_MASK))


static inline void nc_dump_status (struct usbnet *dev, u16 status)
{
#ifdef DEBUG
	devdbg (dev, "net1080 %s-%s status 0x%x:"
			" this (%c) PKT=%d%s%s%s;"
			" other PKT=%d%s%s%s; unspec 0x%x",
		dev->udev->bus->bus_name, dev->udev->devpath,
		status,

		// XXX the packet counts don't seem right
		// (1 at reset, not 0); maybe UNSPEC too

		(status & STATUS_PORT_A) ? 'A' : 'B',
		STATUS_PACKETS_THIS (status),
		(status & STATUS_CONN_THIS) ? " CON" : "",
		(status & STATUS_SUSPEND_THIS) ? " SUS" : "",
		(status & STATUS_MAILBOX_THIS) ? " MBOX" : "",

		STATUS_PACKETS_OTHER (status),
		(status & STATUS_CONN_OTHER) ? " CON" : "",
		(status & STATUS_SUSPEND_OTHER) ? " SUS" : "",
		(status & STATUS_MAILBOX_OTHER) ? " MBOX" : "",

		status & STATUS_UNSPEC_MASK
		);
#endif
}

/*-------------------------------------------------------------------------*/

/*
 * TTL register
 */

#define TTL_THIS(ttl)	(0x00ff & ttl)
#define TTL_OTHER(ttl)	(0x00ff & (ttl >> 8))
#define MK_TTL(this,other)	((u16)(((other)<<8)|(0x00ff&(this))))

static inline void nc_dump_ttl (struct usbnet *dev, u16 ttl)
{
#ifdef DEBUG
	devdbg (dev, "net1080 %s-%s ttl 0x%x this = %d, other = %d",
		dev->udev->bus->bus_name, dev->udev->devpath,
		ttl,

		TTL_THIS (ttl),
		TTL_OTHER (ttl)
		);
#endif
}

/*-------------------------------------------------------------------------*/

static int net1080_reset (struct usbnet *dev)
{
	u16		usbctl, status, ttl;
	u16		*vp = kmalloc (sizeof (u16), GFP_KERNEL);
	int		retval;

	if (!vp)
		return -ENOMEM;

	// nc_dump_registers (dev);

	if ((retval = nc_register_read (dev, REG_STATUS, vp)) < 0) {
		dbg ("can't read %s-%s status: %d",
			dev->udev->bus->bus_name, dev->udev->devpath, retval);
		goto done;
	}
	status = *vp;
	// nc_dump_status (dev, status);

	if ((retval = nc_register_read (dev, REG_USBCTL, vp)) < 0) {
		dbg ("can't read USBCTL, %d", retval);
		goto done;
	}
	usbctl = *vp;
	// nc_dump_usbctl (dev, usbctl);

	nc_register_write (dev, REG_USBCTL,
			USBCTL_FLUSH_THIS | USBCTL_FLUSH_OTHER);

	if ((retval = nc_register_read (dev, REG_TTL, vp)) < 0) {
		dbg ("can't read TTL, %d", retval);
		goto done;
	}
	ttl = *vp;
	// nc_dump_ttl (dev, ttl);

	nc_register_write (dev, REG_TTL,
			MK_TTL (NC_READ_TTL_MS, TTL_OTHER (ttl)) );
	dbg ("%s: assigned TTL, %d ms", dev->net.name, NC_READ_TTL_MS);

	if (dev->msg_level >= 2)
		devinfo (dev, "port %c, peer %sconnected",
			(status & STATUS_PORT_A) ? 'A' : 'B',
			(status & STATUS_CONN_OTHER) ? "" : "dis"
			);
	retval = 0;

done:
	kfree (vp, 0);
	return retval;
}

static int net1080_check_connect (struct usbnet *dev)
{
	int			retval;
	u16			status;
	u16			*vp = kmalloc (sizeof (u16), GFP_KERNEL);

	if (!vp)
		return -ENOMEM;
	retval = nc_register_read (dev, REG_STATUS, vp);
	status = *vp;
	kfree (vp, 0);
	if (retval != 0) {
		dbg ("%s net1080_check_conn read - %d", dev->net.name, retval);
		return retval;
	}
	if ((status & STATUS_CONN_OTHER) != STATUS_CONN_OTHER)
		return -ENOLINK;
	return 0;
}

static int net1080_rx_fixup (struct usbnet *dev, struct sk_buff *skb)
{
	struct nc_header	*header;
	struct nc_trailer	*trailer;

	if (!(skb->len & 0x01)
			|| MIN_FRAMED > skb->len
			|| skb->len > FRAMED_SIZE (dev->net.mtu)) {
		dev->stats.rx_frame_errors++;
		dbg ("rx framesize %d range %d..%d mtu %d", skb->len,
			(int)MIN_FRAMED, (int)FRAMED_SIZE (dev->net.mtu),
			dev->net.mtu);
		return 0;
	}

	header = (struct nc_header *) skb->data;
	le16_to_cpus (&header->hdr_len);
	le16_to_cpus (&header->packet_len);
	if (FRAMED_SIZE (header->packet_len) > MAX_PACKET) {
		dev->stats.rx_frame_errors++;
		dbg ("packet too big, %d", header->packet_len);
		return 0;
	} else if (header->hdr_len < MIN_HEADER) {
		dev->stats.rx_frame_errors++;
		dbg ("header too short, %d", header->hdr_len);
		return 0;
	} else if (header->hdr_len > MIN_HEADER) {
		// out of band data for us?
		dbg ("header OOB, %d bytes",
			header->hdr_len - MIN_HEADER);
		// switch (vendor/product ids) { ... }
	}
	skb_pull (skb, header->hdr_len);

	trailer = (struct nc_trailer *)
		(skb->data + skb->len - sizeof *trailer);
	skb_trim (skb, skb->len - sizeof *trailer);

	if ((header->packet_len & 0x01) == 0) {
		if (skb->data [header->packet_len] != PAD_BYTE) {
			dev->stats.rx_frame_errors++;
			dbg ("bad pad");
			return 0;
		}
		skb_trim (skb, skb->len - 1);
	}
	if (skb->len != header->packet_len) {
		dev->stats.rx_frame_errors++;
		dbg ("bad packet len %d (expected %d)",
			skb->len, header->packet_len);
		return 0;
	}
	if (header->packet_id != get_unaligned (&trailer->packet_id)) {
		dev->stats.rx_fifo_errors++;
		dbg ("(2+ dropped) rx packet_id mismatch 0x%x 0x%x",
			header->packet_id, trailer->packet_id);
		return 0;
	}
#if 0
	devdbg (dev, "frame <rx h %d p %d id %d", header->hdr_len,
		header->packet_len, header->packet_id);
#endif
	return 1;
}

static struct sk_buff *
net1080_tx_fixup (struct usbnet *dev, struct sk_buff *skb, int flags)
{
	int			padlen;
	struct sk_buff		*skb2;

	padlen = ((skb->len + sizeof (struct nc_header)
			+ sizeof (struct nc_trailer)) & 0x01) ? 0 : 1;
	if (!skb_cloned (skb)) {
		int	headroom = skb_headroom (skb);
		int	tailroom = skb_tailroom (skb);

		if ((padlen + sizeof (struct nc_trailer)) <= tailroom
			    && sizeof (struct nc_header) <= headroom)
			return skb;

		if ((sizeof (struct nc_header) + padlen
					+ sizeof (struct nc_trailer)) <
				(headroom + tailroom)) {
			skb->data = memmove (skb->head
						+ sizeof (struct nc_header),
					    skb->data, skb->len);
			skb->tail = skb->data + skb->len;
			return skb;
		}
	}
	skb2 = skb_copy_expand (skb,
				sizeof (struct nc_header),
				sizeof (struct nc_trailer) + padlen,
				flags);
	dev_kfree_skb_any (skb);
	return skb2;
}

static const struct driver_info net1080_info = {
	.description =	"NetChip TurboCONNECT",
	.flags =	FLAG_FRAMING_NC,
	.reset =	net1080_reset,
	.check_connect =net1080_check_connect,
	.rx_fixup =	net1080_rx_fixup,
	.tx_fixup =	net1080_tx_fixup,
};

#endif /* CONFIG_USB_NET1080 */



#ifdef CONFIG_USB_PL2301

/*-------------------------------------------------------------------------
 *
 * Prolific PL-2301/PL-2302 driver ... http://www.prolifictech.com
 *
 *-------------------------------------------------------------------------*/

/*
 * Bits 0-4 can be used for software handshaking; they're set from
 * one end, cleared from the other, "read" with the interrupt byte.
 */
#define PL_S_EN 	(1<<7)		/* (feature only) suspend enable */
/* reserved bit -- rx ready (6) ? */
#define PL_TX_READY	(1<<5)		/* (interrupt only) transmit ready */
#define PL_RESET_OUT	(1<<4)		/* reset output pipe */
#define PL_RESET_IN	(1<<3)		/* reset input pipe */
#define PL_TX_C 	(1<<2)		/* transmission complete */
#define PL_TX_REQ	(1<<1)		/* transmission received */
#define PL_PEER_E	(1<<0)		/* peer exists */

static inline int
pl_vendor_req (struct usbnet *dev, u8 req, u8 val, u8 index)
{
	return usb_control_msg (dev->udev,
		usb_rcvctrlpipe (dev->udev, 0),
		req,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		val, index,
		0, 0,
		CONTROL_TIMEOUT_JIFFIES);
}

static inline int
pl_clear_QuickLink_features (struct usbnet *dev, int val)
{
	return pl_vendor_req (dev, 1, (u8) val, 0);
}

static inline int
pl_set_QuickLink_features (struct usbnet *dev, int val)
{
	return pl_vendor_req (dev, 3, (u8) val, 0);
}

/*-------------------------------------------------------------------------*/

static int pl_reset (struct usbnet *dev)
{
	return pl_set_QuickLink_features (dev,
		PL_S_EN|PL_RESET_OUT|PL_RESET_IN|PL_PEER_E);
}

static const struct driver_info prolific_info = {
	.description =	"Prolific PL-2301/PL-2302",
	.flags =	FLAG_NO_SETINT,
		/* some PL-2302 versions seem to fail usb_set_interface() */
	.reset =	pl_reset,
};

#endif /* CONFIG_USB_PL2301 */



#ifdef	CONFIG_USB_ARMLINUX

/*-------------------------------------------------------------------------
 *
 * Standard ARM kernels include a "usb-eth" driver, or a newer
 * "ethernet gadget" driver for basic USB connectivity.  The vendor
 * and product code may also be used for other non-CDC Linux devices,
 * if they all maintain protocol compatibility.
 *
 * That means lots of hardware could match here, possibly using
 * different endpoint numbers (and bcdVersion ids).  so we rely on
 * endpoint descriptors to sort that out for us.
 *
 * (Current Zaurus models need a different driver; see later.)
 *
 *-------------------------------------------------------------------------*/

static const struct driver_info linuxdev_info = {
	.description =	"Linux Device",
	.check_connect = always_connected,
};

static const struct driver_info yopy_info = {
	.description =	"Yopy",
	.check_connect = always_connected,
};

static const struct driver_info blob_info = {
	.description =	"Boot Loader OBject",
	.check_connect = always_connected,
};

#endif	/* CONFIG_USB_ARMLINUX */


#ifdef CONFIG_USB_ZAURUS

//celebi #include <linux/crc32.h>

/*-------------------------------------------------------------------------
 *
 * Zaurus PDAs are also ARM based, but currently use different drivers
 * (and framing) for USB slave/gadget controllers than the case above.
 *
 * For the current version of that driver, the main way that framing is
 * nonstandard (also from perspective of the CDC ethernet model!) is a
 * crc32, added to help detect when some sa1100 usb-to-memory DMA errata
 * haven't been fully worked around.
 *
 *-------------------------------------------------------------------------*/

static struct sk_buff *
zaurus_tx_fixup (struct usbnet *dev, struct sk_buff *skb, int flags)
{
	int			padlen;
	struct sk_buff		*skb2;

	padlen = 2;
	if (!skb_cloned (skb)) {
		int	tailroom = skb_tailroom (skb);
		if ((padlen + 4) <= tailroom)
			goto done;
	}
	skb2 = skb_copy_expand (skb, 0, 4 + padlen, flags);
	dev_kfree_skb_any (skb);
	skb = skb2;
	if (skb) {
		u32		fcs;
done:
		fcs = crc32_le (~0, skb->data, skb->len);
		fcs = ~fcs;

		*skb_put (skb, 1) = fcs       & 0xff;
		*skb_put (skb, 1) = (fcs>> 8) & 0xff;
		*skb_put (skb, 1) = (fcs>>16) & 0xff;
		*skb_put (skb, 1) = (fcs>>24) & 0xff;
	}
	return skb;
}

/* SA-1100 based */
static const struct driver_info zaurus_sl5x00_info = {
	.description =	"Sharp Zaurus SL-5x00",
	.flags =	FLAG_FRAMING_Z,
	.check_connect = always_connected,
	.tx_fixup =	zaurus_tx_fixup,

	.in = 2, .out = 1,
	.epsize = 64,
};

/* PXA-2xx based */
static const struct driver_info zaurus_pxa_info = {
	.description =	"Sharp Zaurus, PXA-2xx based",
	.flags =	FLAG_FRAMING_Z,
	.check_connect = always_connected,
	.tx_fixup =	zaurus_tx_fixup,

	.in = 1, .out = 2,
	.epsize = 64,
};

#endif


/*-------------------------------------------------------------------------
 *
 * Network Device Driver (peer link to "Host Device", from USB host)
 *
 *-------------------------------------------------------------------------*/
//celebi
#if 0
static int usbnet_change_mtu (struct net_device *net, int new_mtu)
{
	struct usbnet	*dev = (struct usbnet *) net->priv;

	if (new_mtu <= MIN_PACKET || new_mtu > MAX_PACKET)
		return -EINVAL;
#ifdef	CONFIG_USB_NET1080
	if (((dev->driver_info->flags) & FLAG_FRAMING_NC)) {
		if (FRAMED_SIZE (new_mtu) > MAX_PACKET)
			return -EINVAL;
	}
#endif
#ifdef	CONFIG_USB_GENESYS
	if (((dev->driver_info->flags) & FLAG_FRAMING_GL)
			&& new_mtu > GL_MAX_PACKET_LEN)
		return -EINVAL;
#endif
	// no second zero-length packet read wanted after mtu-sized packets
	if (((new_mtu + sizeof (struct ethhdr)) % dev->maxpacket) == 0)
		return -EDOM;
	net->mtu = new_mtu;
	return 0;
}
#endif
/*-------------------------------------------------------------------------*/
//celebi
#if 0
static struct net_device_stats *usbnet_get_stats (struct net_device *net)
{
	return &((struct usbnet *) net->priv)->stats;
}
#endif
/*-------------------------------------------------------------------------*/

/* urb completions may be in_irq; avoid doing real work then. */
//celebi
#if 0
static void defer_bh (struct usbnet *dev, struct sk_buff *skb)
{
	struct sk_buff_head	*list = skb->list;
	unsigned long		flags;

	spin_lock_irqsave (&list->lock, &flags);	//ZOT==> spin_lock_irqsave (&list->lock, flags);
	__skb_unlink (skb, list);
	spin_unlock (&list->lock);
	spin_lock (&dev->done.lock);
	__skb_queue_tail (&dev->done, skb);
	if (dev->done.qlen == 1)
//celebi		tasklet_schedule (&dev->bh);
		usbnet_bh(dev);
	spin_unlock_irqrestore (&dev->done.lock, &flags);//ZOT==>	spin_unlock_irqrestore (&dev->done.lock, flags);
}
#endif

/* some work can't be done in tasklets, so we use keventd
 *
 * NOTE:  annoying asymmetry:  if it's active, schedule_task() fails,
 * but tasklet_schedule() doesn't.  hope the failure is rare.
 */
//celebi
#if 0 
static void defer_kevent (struct usbnet *dev, int work)
{
	set_bit (work, &dev->flags);
	if (!schedule_task (&dev->kevent))
		err ("%s: kevent %d may have been dropped",
			dev->net.name, work);
	else
		dbg ("%s: kevent %d scheduled", dev->net.name, work);
}
#endif
/*-------------------------------------------------------------------------*/

static void rx_complete (struct urb *urb);
static void rx_submit (struct usbnet *dev, struct urb *urb, int flags)
{
//celebi	struct sk_buff		*skb;
//celebi	struct skb_data 	*entry;
	int			retval = 0;
	unsigned long		lockflags;
	size_t			size;

#ifdef CONFIG_USB_NET1080
	if (dev->driver_info->flags & FLAG_FRAMING_NC)
		size = FRAMED_SIZE (dev->net.mtu);
	else
#endif
#ifdef CONFIG_USB_GENESYS
	if (dev->driver_info->flags & FLAG_FRAMING_GL)
		size = GL_RCV_BUF_SIZE;
	else
#endif
#ifdef CONFIG_USB_ZAURUS
	if (dev->driver_info->flags & FLAG_FRAMING_Z)
		size = 6 + (sizeof (struct ethhdr) + dev->net.mtu);
	else
#endif
//celebi		size = (sizeof (struct ethhdr) + dev->net.mtu);
//celebi		size = 2048;
//celebi	if ((skb = alloc_skb (size, flags)) == 0) {
//celebi		dbg ("no rx skb");
//celebi		defer_kevent (dev, EVENT_RX_MEMORY);
//celebi		usb_free_urb (urb);
//celebi		return;
//celebi	}

//celebi	entry = (struct skb_data *) skb->cb;
//celebi	entry->urb = urb;
//celebi	entry->dev = dev;
//celebi	entry->state = rx_start;
//celebi	entry->length = 0;

//celebi	usb_fill_bulk_urb (urb, dev->udev, dev->in,
//celebi		skb->data, size, rx_complete, skb);
//celebi	urb->transfer_flags |= USB_ASYNC_UNLINK;
		
//		size = 1514;
		size = 1518;
		FILL_BULK_URB( &dev->rx_urb, dev->udev,
			dev->in,
			dev->rx_buff, size, 
			rx_complete, dev );

//celebi	spin_lock_irqsave (&dev->rxq.lock, lockflags);

//celebi	if (netif_running (&dev->net)
//celebi			&& !test_bit (EVENT_RX_HALT, &dev->flags)) {
	if(1){
		switch (retval = SUBMIT_URB (urb, GFP_ATOMIC)){
		case -EPIPE:
//celebi			defer_kevent (dev, EVENT_RX_HALT);
			break;
		case -ENOMEM:
//celebi			defer_kevent (dev, EVENT_RX_MEMORY);
			break;
		default:
//celebi			dbg ("%s rx submit, %d", dev->net.name, retval);
//celebi			tasklet_schedule (&dev->bh);
//celebi			usbnet_bh(dev);
			break;
		case 0:
//celebi			__skb_queue_tail (&dev->rxq, skb);
			break;
		}
	} else {
//celebi		dbg ("rx: stopped");
		retval = -ENOLINK;
	}


//celebi	spin_unlock_irqrestore (&dev->rxq.lock, lockflags);
	if (retval) {
//celebi		dev_kfree_skb_any (skb);
//celebi		usb_free_urb (urb);
	}
}


/*-------------------------------------------------------------------------*/
//celebi
#if 0
static inline void rx_process (struct usbnet *dev, struct sk_buff *skb)
{
	if (dev->driver_info->rx_fixup
			&& !dev->driver_info->rx_fixup (dev, skb))
		goto error;
	// else network stack removes extra byte if we forced a short packet

	if (skb->len) {
		int	status;

		skb->dev = &dev->net;
		skb->protocol = eth_type_trans (skb, &dev->net);
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += skb->len;

#ifdef	VERBOSE
		devdbg (dev, "< rx, len %d, type 0x%x",
			skb->len + sizeof (struct ethhdr), skb->protocol);
#endif
		memset (skb->cb, 0, sizeof (struct skb_data));
		status = netif_rx (skb);
		if (status != NET_RX_SUCCESS)
			devdbg (dev, "netif_rx status %d", status);
	} else {
		dbg ("drop");
error:
		dev->stats.rx_errors++;
		skb_queue_tail (&dev->done, skb);
	}
}
#endif

/*-------------------------------------------------------------------------*/
int UlanRecv(unsigned char *pFrame, unsigned int lenFrame);

static void rx_complete (struct urb *urb)
{
//celebi	struct sk_buff		*skb = (struct sk_buff *) urb->context;
//celebi	struct skb_data 	*entry = (struct skb_data *) skb->cb;
//celebi	struct usbnet		*dev = entry->dev;
	struct usbnet		*dev = USBNET;
	int	urb_status = urb->status;
	int count = urb->actual_length;
	char *packet_ptr=NULL;

	if(USBNET == NULL)
		return;

//celebi	skb_put (skb, urb->actual_length);
//celebi	entry->state = rx_done;
//celebi	entry->urb = 0;

	switch (urb_status) {
	    // success
	    case 0:
//celebi		if (MIN_PACKET > skb->len || skb->len > MAX_PACKET) {
		if ( (MIN_PACKET > count) || (count > MAX_PACKET)) {
//celebi			entry->state = rx_cleanup;
			dev->stats.rx_errors++;
			dev->stats.rx_length_errors++;
//celebi			dbg ("rx length %d", skb->len);
		}
		break;

	    // stalls need manual reset. this is rare ... except that
	    // when going through USB 2.0 TTs, unplug appears this way.
	    // we avoid the highspeed version of the ETIMEOUT/EILSEQ
	    // storm, recovering as needed.
	    case -EPIPE:
//celebi		defer_kevent (dev, EVENT_RX_HALT);
		// FALLTHROUGH

	    // software-driven interface shutdown
	    case -ECONNRESET:		// according to API spec
	    case -ECONNABORTED: 	// some (now fixed?) UHCI bugs
//celebi		dbg ("%s rx shutdown, code %d", dev->net.name, urb_status);
//celebi		entry->state = rx_cleanup;
		// do urb frees only in the tasklet (UHCI has oopsed ...)
//celebi		entry->urb = urb;
//celebi		urb = 0;
		break;

	    // data overrun ... flush fifo?
	    case -EOVERFLOW:
//celebi		dev->stats.rx_over_errors++;
		dev->stats.rx_length_errors++; //celebi
		// FALLTHROUGH

	    default:
		// on unplug we get ETIMEDOUT (ohci) or EILSEQ (uhci)
		// until khubd sees its interrupt and disconnects us.
		// that can easily be hundreds of passes through here.
//celebi		entry->state = rx_cleanup;
		dev->stats.rx_errors++;
//celebi		dbg ("%s rx: status %d", dev->net.name, urb_status);
		break;
	}

//celebi	defer_bh (dev, skb);
#if 1 //not recvive
	packet_ptr = dev->rx_buff;
	if (dev->driver_info->rx_fixup) {
		count = dev->driver_info->rx_fixup (dev, packet_ptr, count);
		
	}
	else{
		UlanRecv(packet_ptr, count);	//zot

	dev->stats.rx_packets++;
	dev->stats.rx_bytes += count;
	}
#endif //0	


	if (urb) {
//celebi		if (netif_running (&dev->net)
//celebi				&& !test_bit (EVENT_RX_HALT, &dev->flags)) {
		if( 1 ){
			rx_submit (dev, urb, GFP_ATOMIC);
			return;
		}
//celebi		usb_free_urb (urb);
	}
#ifdef	VERBOSE
	dbg ("no read resubmitted");
#endif /* VERBOSE */
}

/*-------------------------------------------------------------------------*/

// unlink pending rx/tx; completion handlers do all other cleanup
//celebi
#if 0
static int unlink_urbs (struct sk_buff_head *q)
{
	unsigned long		flags;
	struct sk_buff		*skb, *skbnext;
	int			count = 0;

	spin_lock_irqsave (&q->lock, &flags); //ZOT==>	spin_lock_irqsave (&q->lock, flags);
	for (skb = q->next; skb != (struct sk_buff *) q; skb = skbnext) {
		struct skb_data 	*entry;
		struct urb		*urb;
		int			retval;

		entry = (struct skb_data *) skb->cb;
		urb = entry->urb;
		skbnext = skb->next;

		// during some PM-driven resume scenarios,
		// these (async) unlinks complete immediately
		retval = usb_unlink_urb (urb);
		if (retval != -EINPROGRESS && retval != 0)
			dbg ("unlink urb err, %d", retval);
		else
			count++;
	}
	spin_unlock_irqrestore (&q->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&q->lock, flags);
	return count;
}
#endif

/*-------------------------------------------------------------------------*/

// precondition: never called in_interrupt
static int usbnet_stop (struct net_device *net)
{
	struct usbnet		*dev = (struct usbnet *) net->priv;
	int			temp;
//celebi	DECLARE_WAIT_QUEUE_HEAD (unlink_wakeup);
//celebi	DECLARE_WAITQUEUE (wait, current);

//celebi	mutex_lock (&dev->mutex);
//celebi	netif_stop_queue (net);

//celebi	if (dev->msg_level >= 2)
//celebi		devinfo (dev, "stop stats: rx/tx %ld/%ld, errs %ld/%ld",
//celebi			dev->stats.rx_packets, dev->stats.tx_packets,
//celebi			dev->stats.rx_errors, dev->stats.tx_errors
//celebi			);

	// ensure there are no more active urbs
//celebi	add_wait_queue (&unlink_wakeup, &wait);
//celebi	dev->wait = &unlink_wakeup;
//celebi	temp = unlink_urbs (&dev->txq) + unlink_urbs (&dev->rxq);

	// maybe wait for deletions to finish.
//celebi	while (skb_queue_len (&dev->rxq)
//celebi			&& skb_queue_len (&dev->txq)
//celebi			&& skb_queue_len (&dev->done)) {
//celebi		set_current_state (TASK_UNINTERRUPTIBLE);
//celebi		schedule_timeout (UNLINK_TIMEOUT_JIFFIES);
//celebi		dbg ("waited for %d urb completions", temp);
//celebi	}
//celebi	dev->wait = 0;
//celebi	remove_wait_queue (&unlink_wakeup, &wait);

//celebi	mutex_unlock (&dev->mutex);
	return 0;
}

/*-------------------------------------------------------------------------*/

// posts reads, and enables write queing

// precondition: never called in_interrupt
static int usbnet_open (struct net_device *net)
{
	struct usbnet		*dev = (struct usbnet *) net->priv;
	int			retval = 0;
	struct driver_info	*info = dev->driver_info;

//celebi	mutex_lock (&dev->mutex);

	// put into "known safe" state
	if (info->reset && (retval = info->reset (dev)) < 0) {
//celebi		devinfo (dev, "open reset fail (%d) usbnet usb-%s-%s, %s",
//celebi			retval,
//celebi			dev->udev->bus->bus_name, dev->udev->devpath,
//celebi			info->description);
		goto done;
	}

	// insist peer be connected
	if (info->check_connect && (retval = info->check_connect (dev)) < 0) {
//celebi		devdbg (dev, "can't open; %d", retval);
		goto done;
	}

//celebi	netif_start_queue (net);
//celebi	if (dev->msg_level >= 2)
//celebi		devinfo (dev, "open: enable queueing "
//celebi				"(rx %d, tx %d) mtu %d %s framing",
//celebi			RX_QLEN(dev), TX_QLEN(dev), dev->net.mtu,
//celebi			(info->flags & (FLAG_FRAMING_NC | FLAG_FRAMING_GL))
//celebi			    ? ((info->flags & FLAG_FRAMING_NC)
//celebi				? "NetChip"
//celebi				: "GeneSys")
//celebi			    : "raw"
//celebi			);

	// delay posting reads until we're fully open
//celebi	tasklet_schedule (&dev->bh);
	usbnet_bh(dev);
done:
//celebi	mutex_unlock (&dev->mutex);
	return retval;
}


/*-------------------------------------------------------------------------*/
//celebi
#if 0
static void usbnet_get_drvinfo (struct net_device *net, struct ethtool_drvinfo *info)
{
	struct usbnet *dev = net->priv;

	strncpy (info->driver, driver_name, sizeof info->driver);
	strncpy (info->version, DRIVER_VERSION, sizeof info->version);
	strncpy (info->fw_version, dev->driver_info->description,
		sizeof info->fw_version);
	usb_make_path (dev->udev, info->bus_info, sizeof info->bus_info);
}
#endif
//celebi
#if 0
static u32 usbnet_get_link (struct net_device *net)
{
	struct usbnet *dev = net->priv;

	/* If a check_connect is defined, return it's results */
	if (dev->driver_info->check_connect)
		return dev->driver_info->check_connect (dev) == 0;

	/* Otherwise, we're up to avoid breaking scripts */
	return 1;
}
#endif
//celebi
#if 0
static u32 usbnet_get_msglevel (struct net_device *net)
{
	struct usbnet *dev = net->priv;

	return dev->msg_level;
}
#endif
//celebi
#if 0
static void usbnet_set_msglevel (struct net_device *net, u32 level)
{
	struct usbnet *dev = net->priv;

	dev->msg_level = level;
}

static int usbnet_ioctl (struct net_device *net, struct ifreq *rq, int cmd)
{
#ifdef NEED_MII
	{
	struct usbnet *dev = (struct usbnet *)net->priv;

	if (dev->mii.mdio_read != NULL && dev->mii.mdio_write != NULL)
		return generic_mii_ioctl(&dev->mii,
				(struct mii_ioctl_data *) &rq->ifr_data,
				cmd, NULL);
	}
#endif
	return -EOPNOTSUPP;
}
#endif

/*-------------------------------------------------------------------------*/
//celebi
#if 0
/* work that cannot be done in interrupt context uses keventd.
 *
 * NOTE:  "uhci" and "usb-uhci" may have trouble with this since they don't
 * queue control transfers to individual devices, and other threads could
 * trigger control requests concurrently.  hope that's rare.
 */
static void
kevent (void *data)
{
	struct usbnet		*dev = data;
	int			status;

	/* usb_clear_halt() needs a thread context */
	if (test_bit (EVENT_TX_HALT, &dev->flags)) {
		unlink_urbs (&dev->txq);
		status = usb_clear_halt (dev->udev, dev->out);
		if (status < 0)
			err ("%s: can't clear tx halt, status %d",
				dev->net.name, status);
		else {
			clear_bit (EVENT_TX_HALT, &dev->flags);
			netif_wake_queue (&dev->net);
		}
	}
	if (test_bit (EVENT_RX_HALT, &dev->flags)) {
		unlink_urbs (&dev->rxq);
		status = usb_clear_halt (dev->udev, dev->in);
		if (status < 0)
			err ("%s: can't clear rx halt, status %d",
				dev->net.name, status);
		else {
			clear_bit (EVENT_RX_HALT, &dev->flags);
//celebi			tasklet_schedule (&dev->bh);
			usbnet_bh(dev);
		}
	}

	/* tasklet could resubmit itself forever if memory is tight */
	if (test_bit (EVENT_RX_MEMORY, &dev->flags)) {
		struct urb	*urb = 0;

		if (netif_running (&dev->net))
			urb = ALLOC_URB (0, GFP_KERNEL);
		else
			clear_bit (EVENT_RX_MEMORY, &dev->flags);
		if (urb != 0) {
			clear_bit (EVENT_RX_MEMORY, &dev->flFags);
			rx_submit (dev, urb, GFP_KERNEL);
//celebi			tasklet_schedule (&dev->bh);
			usbnet_bh(dev);
		}
	}

	if (dev->flags)
		dbg ("%s: kevent done, flags = 0x%lx",
			dev->net.name, dev->flags);
}
#endif
/*-------------------------------------------------------------------------*/
static void tx_complete (struct urb *urb)
{
//celebi	struct sk_buff		*skb = (struct sk_buff *) urb->context;
//celebi	struct skb_data 	*entry = (struct skb_data *) skb->cb;
//celebi	struct usbnet		*dev = entry->dev;

//celebi	if (urb->status == -EPIPE)
//celebi		defer_kevent (dev, EVENT_TX_HALT);
//celebi	urb->dev = 0;
//celebi	entry->state = tx_done;
//celebi	defer_bh (dev, skb);

cyg_semaphore_post(&usb_send_frame_mutex);	//ZOT716u2

}

/*-------------------------------------------------------------------------*/
//celebi
#if 0
static void usbnet_tx_timeout (struct net_device *net)
{
	struct usbnet		*dev = (struct usbnet *) net->priv;

	unlink_urbs (&dev->txq);
//celebi	tasklet_schedule (&dev->bh);
	usbnet_bh(dev);
	// FIXME: device recovery -- reset?
}
#endif
/*-------------------------------------------------------------------------*/
//celebi
#if 0
static int usbnet_start_xmit (struct sk_buff *skb, struct net_device *net)
{
	struct usbnet		*dev = (struct usbnet *) net->priv;
	int			length;
	int			retval = NET_XMIT_SUCCESS;
	struct urb		*urb = 0;
	struct skb_data 	*entry;
	struct driver_info	*info = dev->driver_info;
	unsigned long		flags;
#ifdef	CONFIG_USB_NET1080
	struct nc_header	*header = 0;
	struct nc_trailer	*trailer = 0;
#endif	/* CONFIG_USB_NET1080 */

	// some devices want funky USB-level framing, for
	// win32 driver (usually) and/or hardware quirks
	if (info->tx_fixup) {
		skb = info->tx_fixup (dev, skb, GFP_ATOMIC);
		if (!skb) {
			dbg ("can't tx_fixup skb");
			goto drop;
		}
	}
	length = skb->len;

	if (!(urb = ALLOC_URB (0, GFP_ATOMIC))) {
		dbg ("no urb");
		goto drop;
	}

	entry = (struct skb_data *) skb->cb;
	entry->urb = urb;
	entry->dev = dev;
	entry->state = tx_start;

	entry->length = length;

	// FIXME: reorganize a bit, so that fixup() fills out NetChip
	// framing too. (Packet ID update needs the spinlock...)
	// [ BETTER:  we already own net->xmit_lock, that's enough ]

#ifdef	CONFIG_USB_NET1080
	if (info->flags & FLAG_FRAMING_NC) {
		header = (struct nc_header *) skb_push (skb, sizeof *header);
		header->hdr_len = cpu_to_le16 (sizeof (*header));
		header->packet_len = cpu_to_le16 (length);
		if (!((skb->len + sizeof *trailer) & 0x01))
			*skb_put (skb, 1) = PAD_BYTE;
		trailer = (struct nc_trailer *) skb_put (skb, sizeof *trailer);
	} else
#endif	/* CONFIG_USB_NET1080 */

	/* don't assume the hardware handles USB_ZERO_PACKET */
	if ((length % dev->maxpacket) == 0)
		skb->len++;

	usb_fill_bulk_urb (urb, dev->udev, dev->out,
			skb->data, skb->len, tx_complete, skb);
	urb->transfer_flags |= USB_ASYNC_UNLINK;

	spin_lock_irqsave (&dev->txq.lock, &flags);	//ZOT==> spin_lock_irqsave (&dev->txq.lock, flags);

#ifdef	CONFIG_USB_NET1080
	if (info->flags & FLAG_FRAMING_NC) {
		header->packet_id = cpu_to_le16 (dev->packet_id++);
		put_unaligned (header->packet_id, &trailer->packet_id);
#if 0
		devdbg (dev, "frame >tx h %d p %d id %d",
			header->hdr_len, header->packet_len,
			header->packet_id);
#endif
	}
#endif	/* CONFIG_USB_NET1080 */

	switch ((retval = SUBMIT_URB (urb, GFP_ATOMIC))) {
	case -EPIPE:
		netif_stop_queue (net);
		defer_kevent (dev, EVENT_TX_HALT);
		break;
	default:
		dbg ("%s tx: submit urb err %d", net->name, retval);
		break;
	case 0:
		net->trans_start = jiffies;
		__skb_queue_tail (&dev->txq, skb);
		if (dev->txq.qlen >= TX_QLEN(dev))
			netif_stop_queue (net);
	}
	spin_unlock_irqrestore (&dev->txq.lock, &flags);//ZOT==>	spin_unlock_irqrestore (&dev->txq.lock, flags);

	if (retval) {
		devdbg (dev, "drop, code %d", retval);
drop:
		retval = NET_XMIT_SUCCESS;
		dev->stats.tx_dropped++;
		if (skb)
			dev_kfree_skb_any (skb);
		usb_free_urb (urb);
#ifdef	VERBOSE
	} else {
		devdbg (dev, "> tx, len %d, type 0x%x",
			length, skb->protocol);
#endif
	}
	return retval;
}
#endif



int usbnet_start_xmit( char * buf, unsigned int len, int flag)
{
	struct usbnet *dev = USBNET;
	int retval=0;

	
	if(USBNET == NULL)
		return retval;

	cyg_semaphore_wait(&usb_send_frame_mutex);//ZOT716u2
	
	if (dev->driver_info->tx_fixup) {
		if(flag)
			cpyfrompbuf(dev->tx_buff+4, buf, len);
		else
    		memcpy((void *)(dev->tx_buff+4), buf, len);
    		
//		memcpy( dev->tx_buff+4, buf, len );
		dev->driver_info->tx_fixup (dev, buf, &len);
	
	}
	else
	{
		if(flag)
			cpyfrompbuf(dev->tx_buff, buf, len);
		else
    		memcpy((void *)dev->tx_buff, buf, len);
	}
	
	/* don't assume the hardware handles USB_ZERO_PACKET */
	if ((len % dev->maxpacket) == 0)
		len++;
	
	FILL_BULK_URB( &dev->tx_urb, dev->udev,
			dev->out,
			dev->tx_buff, len, 
			tx_complete, dev );
			
	dev->tx_urb.transfer_buffer_length = len;

	dev->tx_urb.transfer_flags |= USB_ASYNC_UNLINK;
	if ((retval = usb_submit_urb(&dev->tx_urb))) 
	{
		devdbg (dev, "drop, code %d", retval);
//celbi		retval = NET_XMIT_SUCCESS;
		dev->stats.tx_dropped++;

	}

//	cyg_semaphore_post(&usb_send_frame_mutex);	//ZOT716u2
	
	return retval;	
	
}

/*-------------------------------------------------------------------------*/

// tasklet ... work that avoided running in_irq()
//celebi

static void usbnet_bh (unsigned long param)
{
	struct usbnet		*dev = (struct usbnet *) param;
//celebi	struct sk_buff		*skb;
//celebi	struct skb_data 	*entry;

/* //celebi
	while ((skb = skb_dequeue (&dev->done))) {
		entry = (struct skb_data *) skb->cb;
		switch (entry->state) {
		    case rx_done:
			entry->state = rx_cleanup;
			rx_process (dev, skb);
			continue;
		    case tx_done:
			if (entry->urb->status) {
				// can this statistic become more specific?
				dev->stats.tx_errors++;
				dbg ("%s tx: err %d", dev->net.name,
					entry->urb->status);
			} else {
				dev->stats.tx_packets++;
				dev->stats.tx_bytes += entry->length;
			}
			// FALLTHROUGH:
		    case rx_cleanup:
			usb_free_urb (entry->urb);
			dev_kfree_skb (skb);
			continue;
		    default:
			dbg ("%s: bogus skb state %d",
				dev->net.name, entry->state);
		}
	}
*/
	// waiting for all pending urbs to complete?
//celbi	if (dev->wait) {
//celbi		if ((dev->txq.qlen + dev->rxq.qlen + dev->done.qlen) == 0) {
//celbi			wake_up (dev->wait);
//celbi		}

	// or are we maybe short a few urbs?
//celbi	} else if (netif_running (&dev->net)
//celebi	&& !test_bit (EVENT_RX_HALT, &dev->flags)) {
//celebi		int	temp = dev->rxq.qlen;

//celebi		if (temp < RX_QLEN(dev)) {
//celebi			struct urb	*urb;
//celebi			int		i;
//celebi			for (i = 0; i < 3 && dev->rxq.qlen < RX_QLEN(dev); i++) {
//celebi				if ((urb = ALLOC_URB (0, GFP_ATOMIC)) != 0)
//celebi					rx_submit (dev, urb, GFP_ATOMIC);
//celebi			}
//celbi			if (temp != dev->rxq.qlen)
//celbi				devdbg (dev, "rxqlen %d --> %d",
//celbi						temp, dev->rxq.qlen);
//celebi			if (dev->rxq.qlen < RX_QLEN(dev))
//celbi				tasklet_schedule (&dev->bh);
//celebi				usbnet_bh(dev);
//celebi		}
//celbi		if (dev->txq.qlen < TX_QLEN(dev))
//celbi			netif_wake_queue (&dev->net);
//celebi	}

	rx_submit (dev, &dev->rx_urb, GFP_ATOMIC);

}

/*-------------------------------------------------------------------------
 *
 * USB Device Driver support
 *
 *-------------------------------------------------------------------------*/

// precondition: never called in_interrupt
static void usbnet_disconnect (struct usb_device *udev, void *ptr)
{
	struct usbnet	*dev = (struct usbnet *) ptr;
	struct ax8817x_data *data = (struct ax8817x_data *)dev->data;

	USBNET = NULL;
	
//celebi	devinfo (dev, "unregister usbnet usb-%s-%s, %s",
//celebi		udev->bus->bus_name, udev->devpath,
//celebi		dev->driver_info->description);

//celebi	unregister_netdev (&dev->net);

//celebi	mutex_lock (&usbnet_mutex);
//celebi	mutex_lock (&dev->mutex);
	list_del (&dev->dev_list);
//celebi	mutex_unlock (&usbnet_mutex);

	// assuming we used keventd, it must quiesce too
//celebi	flush_scheduled_tasks ();

	usb_unlink_urb (&dev->rx_urb);
	usb_free_urb (&dev->rx_urb);

	usb_unlink_urb (&dev->tx_urb);
	usb_free_urb (&dev->tx_urb);

//celebi	usb_unlink_urb(data->int_urb);
//celebi	usb_free_urb(data->int_urb);

	kaligned_free(dev->rx_buff);
	kaligned_free(dev->tx_buff);

	kfree (dev, 0);
	usb_put_dev (udev);

	netif_remove( ULanface );
	ULanface = NULL;
	
}

/*-------------------------------------------------------------------------*/

// precondition: never called in_interrupt
static void *
usbnet_probe (struct usb_device *udev, unsigned ifnum,
			const struct usb_device_id *prod)
{
	struct net_device		*net;
	struct driver_info		*info;
	int				altnum = 0;
	int				status;
	struct usbnet			*dev;

	info = (struct driver_info *) prod->driver_info;

#ifdef CONFIG_USB_ZAURUS
	if (info == &zaurus_sl5x00_info) {
		int	status;

		/* old ROMs have more than one config
		 * so we have to make sure config="1" (?)
		 */
		status = usb_set_configuration (udev, 1);
		if (status < 0) {
			err ("set_config failed, %d", status);
			return 0;
		}
		altnum = 1;
	}
#endif

	// more sanity (unless the device is broken)
	if (!(info->flags & FLAG_NO_SETINT)) {
		if (usb_set_interface (udev, ifnum, altnum) < 0) {
//celebi			err ("set_interface failed");
			return 0;
		}
	}

	// set up our own records
	if (!(dev = kmalloc (sizeof *dev, GFP_KERNEL))) {
//celebi		dbg ("can't kmalloc dev");
		return 0;
	}
	memset (dev, 0, sizeof *dev);

//celebi	init_MUTEX_LOCKED (&dev->mutex);
	usb_get_dev (udev);
	dev->udev = udev;
	dev->driver_info = info;
	dev->msg_level = msg_level;
	INIT_LIST_HEAD (&dev->dev_list);
//celebi	skb_queue_head_init (&dev->rxq);
//celebi	skb_queue_head_init (&dev->txq);
//celebi	skb_queue_head_init (&dev->done);
//celebi	dev->bh.func = usbnet_bh;
//celebi	dev->bh.data = (unsigned long) dev;
//celebi	dev->mf.func = media_func;
//celebi	dev->mf.data = (unsigned long) dev;
//celebi	dev->mf0.func = media_func772;
//celebi	dev->mf0.data = (unsigned long) dev;
//celebi	INIT_TQUEUE (&dev->kevent, kevent, dev);

	// set up network interface records
	net = &dev->net;
//celebi	SET_MODULE_OWNER (net);
	net->priv = dev;
		
//719AW	dev->rx_buff = USBPRN_READ_BUFFER;		//celebi
//719AW	dev->tx_buff = USB_PRINTER_WRITE_BUFFER;	//celebi
		dev->rx_buff = kaligned_alloc(8192, 256);	//719AW
		dev->tx_buff = kaligned_alloc(8192, 256);	//719AW
	
//celebi	strcpy (net->name, "usb%d");
//celebi	memcpy (net->PhyAddr, node_id, sizeof node_id);

	// point-to-point link ... we always use Ethernet headers
	// supports win32 interop and the bridge driver.
//celebi	ether_setup (net);

//celebi	net->change_mtu = usbnet_change_mtu;
//celebi	net->get_stats = usbnet_get_stats;
//celebi	net->hard_start_xmit = usbnet_start_xmit;
//celebi	net->open = usbnet_open;
//celebi	net->stop = usbnet_stop;
//celebi	net->watchdog_timeo = TX_TIMEOUT_JIFFIES;
//celebi	net->tx_timeout = usbnet_tx_timeout;
//celebi	net->do_ioctl = usbnet_ioctl;
//celebi	net->ethtool_ops = &usbnet_ethtool_ops;

	// allow device-specific bind/init procedures
	// NOTE net->name still not usable ...
	if (info->bind) {
		status = info->bind (dev, udev);
		// heuristic:  "usb%d" for links we know are two-host,
		// else "eth%d" when there's reasonable doubt.	userspace
		// can rename the link if it knows better.
//celebi		if ((dev->driver_info->flags & FLAG_ETHER) != 0
//celebi				&& (net->dev_addr [0] & 0x02) == 0)
//celebi			strcpy (net->name, "eth%d");
	} else if (!info->in || info->out)
		status = usbnet_get_endpoints (dev, udev->actconfig->interface + ifnum);
	else {
		dev->in = usb_rcvbulkpipe (udev, info->in);
		dev->out = usb_sndbulkpipe (udev, info->out);
	}

	dev->maxpacket = usb_maxpacket (dev->udev, dev->out, 1);

//celebi	register_netdev (&dev->net);
//celebi	devinfo (dev, "register usbnet usb-%s-%s, %s",
//celebi		udev->bus->bus_name, udev->devpath,
//celebi		dev->driver_info->description);

	// ok, it's ready to go.
//celebi	mutex_lock (&usbnet_mutex);
	list_add (&dev->dev_list, &usbnet_list);
//celebi	mutex_unlock (&dev->mutex);

	// start as if the link is up
//celebi	netif_device_attach (&dev->net);

//celebi	mutex_unlock (&usbnet_mutex);

	usbnet_open(net);
	
	cyg_semaphore_init(&usb_send_frame_mutex, 1);;	//ZOT716u2
	
	USBNET = dev;
if(1)	
	{
		struct netif *netif;
		struct ip_addr ipaddr, netmask, gw;	
		
		if(EEPROM_Data.PrintServerMode & PS_DHCP_ON)
		{
			IP4_ADDR( &ipaddr,0,0,0,0);
			IP4_ADDR( &netmask, 0,0,0,0);
			IP4_ADDR( &gw, 0,0,0,0);
		}
		else
		{
			ipaddr.addr = NGET32(EEPROM_Data.BoxIPAddress);
			netmask.addr = NGET32(EEPROM_Data.SubNetMask);
			gw.addr = NGET32(EEPROM_Data.GetwayAddress);
		}
	
		netif = malloc(sizeof(struct netif));
		memset(netif, 0, sizeof(struct netif));
		ULanface = netif_add( netif ,&ipaddr, &netmask, &gw, 0, USB_attach, tcpip_input);
		ULanface->name[0] = 0x55;
	}

	return dev;

}


/*-------------------------------------------------------------------------*/

/*
 * chip vendor names won't normally be on the cables, and
 * may not be on the device.
 */

//celebi static const struct Usb_Device_Id	products [] = {
static const struct usb_device_id products [] = {
	
#ifdef	CONFIG_USB_AN2720
{
	USB_DEVICE (0x0547, 0x2720),	// AnchorChips defaults
	.driver_info =	(unsigned long) &an2720_info,
}, {
	USB_DEVICE (0x0547, 0x2727),	// Xircom PGUNET
	.driver_info =	(unsigned long) &an2720_info,
},
#endif

#ifdef CONFIG_USB_AX8817X
/*
{
	// Linksys USB200M
	USB_DEVICE (0x077b, 0x2226),
	.driver_info =	(unsigned long) &ax8817x_info,
}, {
	// Netgear FA120
	USB_DEVICE (0x0846, 0x1040),
	.driver_info =	(unsigned long) &netgear_fa120_info,
}, {
	// DLink DUB-E100
	USB_DEVICE (0x2001, 0x1a00),
	.driver_info =	(unsigned long) &dlink_dub_e100_info,
}, {
	// Intellinet, ST Lab USB Ethernet
	USB_DEVICE (0x0b95, 0x1720),
	.driver_info =	(unsigned long) &ax8817x_info,
}, {
	// 88772
	USB_DEVICE (0x0b95, 0x7720),
	.driver_info =	(unsigned long) &ax88772_info,
}, {
	// 88178
	USB_DEVICE (0x0b95, 0x1780),
	.driver_info =	(unsigned long) &ax88178_info,
}, {
	// 88178 for billianton linksys
	USB_DEVICE (0x077b, 0x2226),
	.driver_info =	(unsigned long) &ax88178_info,
}, {
	// Hawking UF200, TrendNet TU2-ET100
	USB_DEVICE (0x07b8, 0x420a),
	.driver_info =	(unsigned long) &hawking_uf200_info,
}, {
	// ATEN UC210T
	USB_DEVICE (0x0557, 0x2009),
	.driver_info =	(unsigned long) &ax8817x_info,
},
*/
#endif
{
	// Intellinet, ST Lab USB Ethernet
	USB_DEVICE (0x0b95, 0x1720),
	driver_info :	(unsigned long) &ax8817x_info,
}, {
	// 88772
	USB_DEVICE (0x0b95, 0x7720),
	driver_info :	(unsigned long) &ax88772_info,
}, {
	// 88178
	USB_DEVICE (0x0b95, 0x1780),
	driver_info :	(unsigned long) &ax88178_info,
}, {
	// 88178 for billianton linksys
	USB_DEVICE (0x077b, 0x2226),
	driver_info :	(unsigned long) &ax88178_info,
}, {
	// 88178 for billianton linksys
	USB_DEVICE (0x077b, 0x2226),
	driver_info :	(unsigned long) &ax88178_info,
}, {
	// ATEN UC210T
	USB_DEVICE (0x0557, 0x2009),
	driver_info :	(unsigned long) &ax8817x_info,
}, {
	// ASIX AX88772A 10/100
        USB_DEVICE (0x0b95, 0x772A),
        .driver_info = (unsigned long) &ax88772a_info,
}, {
	// Linksys 200M
        USB_DEVICE (0x13B1, 0x0018),
        .driver_info = (unsigned long) &ax88772a_info,
},

#ifdef	CONFIG_USB_BELKIN
{
	USB_DEVICE (0x050d, 0x0004),	// Belkin
	.driver_info =	(unsigned long) &belkin_info,
}, {
	USB_DEVICE (0x056c, 0x8100),	// eTEK
	.driver_info =	(unsigned long) &belkin_info,
}, {
	USB_DEVICE (0x0525, 0x9901),	// Advance USBNET (eTEK)
	.driver_info =	(unsigned long) &belkin_info,
},
#endif

#ifdef	CONFIG_USB_EPSON2888
{
	USB_DEVICE (0x0525, 0x2888),	// EPSON USB client
	.driver_info	= (unsigned long) &epson2888_info,
},
#endif

#ifdef	CONFIG_USB_GENESYS
{
	USB_DEVICE (0x05e3, 0x0502),	// GL620USB-A
	.driver_info =	(unsigned long) &genelink_info,
},
	/* NOT: USB_DEVICE (0x05e3, 0x0501),	// GL620USB
	 * that's half duplex, not currently supported
	 */
#endif

#ifdef	CONFIG_USB_NET1080
{
	USB_DEVICE (0x0525, 0x1080),	// NetChip ref design
	.driver_info =	(unsigned long) &net1080_info,
}, {
	USB_DEVICE (0x06D0, 0x0622),	// Laplink Gold
	.driver_info =	(unsigned long) &net1080_info,
},
#endif

#ifdef CONFIG_USB_PL2301
{
	USB_DEVICE (0x067b, 0x0000),	// PL-2301
	.driver_info =	(unsigned long) &prolific_info,
}, {
	USB_DEVICE (0x067b, 0x0001),	// PL-2302
	.driver_info =	(unsigned long) &prolific_info,
},
#endif

#ifdef	CONFIG_USB_ARMLINUX
/*
 * SA-1100 using standard ARM Linux kernels, or compatible.
 * Often used when talking to Linux PDAs (iPaq, Yopy, etc).
 * The sa-1100 "usb-eth" driver handles the basic framing.
 * ARMv4.
 *
 * PXA2xx using usb "gadget" driver, or older "usb-eth" much like
 * the sa1100 one. (But PXA hardware uses different endpoints.)
 * ARMv5TE.
 */
{
	// 1183 = 0x049F, both used as hex values?
	// Compaq "Itsy" vendor/product id
	// version numbers vary, along with endpoint usage
	// but otherwise they're protocol-compatible
	USB_DEVICE (0x049F, 0x505A),
	.driver_info =	(unsigned long) &linuxdev_info,
}, {
	USB_DEVICE (0x0E7E, 0x1001),	// G.Mate "Yopy"
	.driver_info =	(unsigned long) &yopy_info,
}, {
	USB_DEVICE (0x8086, 0x07d3),	// "blob" bootloader
	.driver_info =	(unsigned long) &blob_info,
},
#endif

#ifdef	CONFIG_USB_ZAURUS
/*
 * SA-1100 based Sharp Zaurus ("collie"), or compatible.
 * Same idea as above, but different framing.
 */
{
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8004,
	.bInterfaceClass	= 0x0a,
	.bInterfaceSubClass	= 0x00,
	.bInterfaceProtocol	= 0x00,
	.driver_info =	(unsigned long) &zaurus_sl5x00_info,
}, {
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8005, /* A-300 */
	.bInterfaceClass	= 0x02,
	.bInterfaceSubClass	= 0x0a,
	.bInterfaceProtocol	= 0x00,
	.driver_info =	(unsigned long) &zaurus_pxa_info,
}, {
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8006, /* B-500/SL-5600 */
	.bInterfaceClass	= 0x02,
	.bInterfaceSubClass	= 0x0a,
	.bInterfaceProtocol	= 0x00,
	.driver_info =	(unsigned long) &zaurus_pxa_info,
}, {
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8007, /* C-700 */
	.bInterfaceClass	= 0x02,
	.bInterfaceSubClass	= 0x0a,
	.bInterfaceProtocol	= 0x00,
	.driver_info =	(unsigned long) &zaurus_pxa_info,
}, {
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x9031, /* C-750 C-760 */
	.bInterfaceClass	= 0x02,
	.bInterfaceSubClass	= 0x0a,
	.bInterfaceProtocol	= 0x00,
	.driver_info =	(unsigned long) &zaurus_pxa_info,
}, {
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x9032, /* SL-6000 */
	.bInterfaceClass	= 0x02,
	.bInterfaceSubClass	= 0x0a,
	.bInterfaceProtocol	= 0x00,
	.driver_info =	(unsigned long) &zaurus_pxa_info,
},
#endif

	{ },		// END
};
MODULE_DEVICE_TABLE (usb, products);

//celebi
#if 0
static struct usb_driver usbnet_driver = {
	.name = 	driver_name,
	.id_table =	products,
	.probe =	usbnet_probe,
	.disconnect =	usbnet_disconnect,
};
#endif

struct usb_driver usbnet_driver; //celebi

//celebi
#if 0
/* Default ethtool_ops assigned.  Devices can override in their bind() routine */
static struct ethtool_ops usbnet_ethtool_ops = {
	.get_drvinfo		= usbnet_get_drvinfo,
	.get_link		= usbnet_get_link,
	.get_msglevel		= usbnet_get_msglevel,
	.set_msglevel		= usbnet_set_msglevel,
};
/*-------------------------------------------------------------------------*/
#endif

int __init usbnet_init (void)
{
	// compiler should optimize this out
//celebi	if (sizeof (((struct sk_buff *)0)->cb) < sizeof (struct skb_data))
//celebi		BUG ();

//celebi	get_random_bytes (node_id, sizeof node_id);
//celebi	node_id [0] &= 0xfe;	// clear multicast bit
//celebi	node_id [0] |= 0x02;	// set local assignment bit (IEEE802)
	usbnet_driver.name = driver_name; 
	usbnet_driver.probe = usbnet_probe;
	usbnet_driver.disconnect = usbnet_disconnect;
	usbnet_driver.id_table = products;
	
	if (usb_register (&usbnet_driver) < 0)
		return -1;

	if( USBNET_TaskHdl == NULL){
		//Create USBNET Thread
		cyg_thread_create(USBNET_TASK_PRI,
							usbnet_thread,
							0,
							"USBNET",
							(void *) (USBNET_Stack),
							USBNET_TASK_STACK_SIZE,
							&USBNET_TaskHdl,
							&USBNET_Task);
		
		//Start USBNET Thread
		cyg_thread_resume(USBNET_TaskHdl);
	}

	return 0;
}
module_init (usbnet_init);

void __exit usbnet_exit (void)
{
	usb_deregister (&usbnet_driver);
}

//celebi
#if 0
module_exit (usbnet_exit);

EXPORT_NO_SYMBOLS;
MODULE_AUTHOR ("David Brownell <dbrownell@users.sourceforge.net>");
MODULE_DESCRIPTION ("USB Host-to-Host Link Drivers (numerous vendors)");
MODULE_LICENSE ("GPL");
#endif


//celebi
//****************usb init netif******************************
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

extern err_t ethernetif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
extern err_t SendPacket(struct netif *netif, struct pbuf *p);

err_t USB_attach( struct netif *netif)
{
	int i;
	struct ethernetif *ethernetif;
	struct usbnet *dev = USBNET;
    
  	ethernetif = malloc(sizeof(struct ethernetif));
	if (ethernetif == NULL)
	{
  		LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
  		return ERR_MEM;
	}
	
	netif->state = ethernetif;

	netif->output = ethernetif_output;
	netif->linkoutput = SendPacket;
	
	ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
	
	/* set MAC hardware address length */
	netif->hwaddr_len = 6;
	
	/* set MAC hardware address */
	for ( i = 0; i < 6; ++i)
    {
 //       netif->hwaddr[i] = enaddr[i];
 //		netif->hwaddr[i] = dev->net.PhyAddr[i];
 		netif->hwaddr[i] = MyPhysNodeAddress[i];
    }
	
	/* maximum transfer unit */
	netif->mtu = 1500;
	
	/* broadcast capability */
	netif->flags = NETIF_FLAG_BROADCAST;
	
	netif_set_up(netif);
	
	return ERR_OK;
}

uint8 usbnet_link()
{
	uint8 ret;
	struct usbnet *dev = USBNET;
	
	if(USBNET == 0)
		return 0;
	
//	ret = ax8817x_get_link(&dev->net);
	ret = 1;
	return ret; 
}
//*******************************************************

//celebi
//****************usb queue******************************

struct usb_mbuf *USB_RECV_QUEUE = NULL;
cyg_sem_t USB_INPUT_SEM;

static void usb_enqueue(struct usb_mbuf **q,struct usb_mbuf **bpp)
{
	struct usb_mbuf *p;
	int i_state;
	
	if(q == NULL || bpp == NULL || *bpp == NULL)
		return;
	
	if(*q == NULL){
		/* List is empty, stick at front */
		*q = *bpp;
	} else {
		for(p = *q ; p->anext != NULL ; p = p->anext)
			;
		p->anext = *bpp;
	}
	*bpp = NULL;	/* We've consumed it */		
	restore(i_state);	
}

/* Unlink a packet from the head of the queue */
static struct usb_mbuf *usb_dequeue(struct usb_mbuf **q)
{
	struct usb_mbuf *bp;
	int i_state;

	if(q == NULL)
		return NULL;
	i_state = dirps();

	if((bp = *q) != NULL){
		*q = bp->anext;
		bp->anext = NULL;
	}
	
	restore(i_state);
	return bp;
}
//celebi
//*******************************************************

//celebi
//****************ethernet process***********************
struct netif *bridge_in(struct netif *ifp, struct ether *hdr);

void usbnet_thread(cyg_addrword_t data)
{

	struct pbuf *p,*q;
	struct netif *netif;
	char *ptr;
	ueth_hdr *eth;
    unsigned short etherType;
    unsigned char *pFrame;
    unsigned int lenFrame;
    struct usb_mbuf *buf;	
	struct netif *ifp;
	int ret = 0;
	
	cyg_semaphore_init(&USB_INPUT_SEM, 0);;	//ZOT716u2
	
	//receive packet
	while(1){	
		while((buf = usb_dequeue(&USB_RECV_QUEUE)) == NULL)
		{			
			cyg_semaphore_wait(&USB_INPUT_SEM);
		}

		pFrame = buf->data;
		lenFrame = buf->len;
		
		eth = (ueth_hdr *)pFrame;

		ifp = bridge_in(ULanface, eth);
		if (ifp == UBDG_DROP) {
			goto dropPacket;
		}

//	    netif = ULanface;
		netif = Lanface;
	
		//check Ethernet type
	    etherType = ntohs(eth->type);
		
		// Normal Process
		if( !memcmp(eth->dest, ULANMAC, 6) || ((eth->dest[0] & 0x01) == 1) || UIS_BROADCAST_HWADDR(eth->dest) )
		{
			switch(etherType)
			{
			case UETH_IP_TYPE:				
#ifndef L2_ZERO_CPY		
				p = pbuf_alloc(PBUF_RAW, lenFrame, PBUF_POOL);
#else			
				p = pbuf_alloc(PBUF_RAW, lenFrame, PBUF_REF);
				p->flags = PBUF_FLAG_REF2; //Ron 6/25/2005, use for IP Layer Zero Copy
#endif			
				if( p == NULL )
					break;
				
				ptr = pFrame;
				for(q = p; q != NULL; q = q->next) {
					/* Read enough bytes to fill this pbuf in the chain. The
					* available data in the pbuf is given by the q->len
					* variable. */				
#ifndef L2_ZERO_CPY				
					memcpy(q->payload, ptr, q->len);
#else				
					q->payload = ptr;
#endif
					ptr +=q->len;
				}
				
//				if(appbdg_input(p,ULanface))
//					break;
				
				/* update ARP table */
			    etharp_ip_input(netif, p);
			    /* skip Ethernet header */
			    pbuf_header(p, -sizeof(struct ueth_hdr));
			    /* pass to network layer */
		//	    netif->input(p, netif);
			    tcpip_input(p, netif);
				break;
		
			case UETH_ARP_TYPE:
				p = pbuf_alloc(PBUF_RAW, lenFrame, PBUF_POOL);
				
				if( p == NULL )
					break;
				
				ptr = pFrame;
				for(q = p; q != NULL; q = q->next) {
				  /* Read enough bytes to fill this pbuf in the chain. The
				   * available data in the pbuf is given by the q->len
				   * variable. */
				   memcpy(q->payload, ptr, q->len);
				   ptr +=q->len;
				 
				}
				etharp_arp_input(netif, ULANMAC, p);	
				break;
				
			default:
//				ProcessOtherPckt(data, len);
				OtherPckt_input(pFrame, lenFrame);
				break;
			}//switch	    
		}//if	
		
dropPacket:		
		free(buf->data);
	 	free(buf);	
	}
	

}

int UlanRecv(unsigned char *pFrame, unsigned int lenFrame)
{
  struct usb_mbuf *pac;
  
  if((pFrame == NULL) || (lenFrame <= 0) || (lenFrame > 1600))
  		return;
  		
  pac = malloc(sizeof(struct usb_mbuf));
  if (pac <= 0)
  	return;
  	
  memset(pac, 0x00, sizeof(struct usb_mbuf));
  
  pac->data = malloc(lenFrame);
  if(pac->data <= 0){
	free(pac) ;
  	return;
	}
  
  memset(pac->data, 0x00, lenFrame);
  memcpy(pac->data, pFrame, lenFrame);
  
  pac->len = lenFrame;
  
  usb_enqueue(&USB_RECV_QUEUE,&pac);
  cyg_semaphore_post(&USB_INPUT_SEM);
}
//celebi
//*******************************************************
