/*
 * URB OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2001 David Brownell <dbrownell@users.sourceforge.net>
 * 
 * [ Initialisation is based on Linus'  ]
 * [ uhci code and gregs ohci fragments ]
 * [ (C) Copyright 1999 Linus Torvalds  ]
 * [ (C) Copyright 1999 Gregory P. Smith]
 * 
 * 
 * History:
 *
 * 2002/03/08 interrupt unlink fix (Matt Hughes), better cleanup on
 *	load failure (Matthew Frederickson)
 * 2002/01/20 async unlink fixes:  return -EINPROGRESS (per spec) and
 *	make interrupt unlink-in-completion work (db)
 * 2001/09/19 USB_ZERO_PACKET support (Jean Tourrilhes)
 * 2001/07/17 power management and pmac cleanup (Benjamin Herrenschmidt)
 * 2001/03/24 td/ed hashing to remove bus_to_virt (Steve Longerbeam);
 	pci_map_single (db)
 * 2001/03/21 td and dev/ed allocation uses new pci_pool API (db)
 * 2001/03/07 hcca allocation uses pci_alloc_consistent (Steve Longerbeam)
 *
 * 2000/09/26 fixed races in removing the private portion of the urb
 * 2000/09/07 disable bulk and control lists when unlinking the last
 *	endpoint descriptor in order to avoid unrecoverable errors on
 *	the Lucent chips. (rwc@sgi)
 * 2000/08/29 use bandwidth claiming hooks (thanks Randy!), fix some
 *	urb unlink probs, indentation fixes
 * 2000/08/11 various oops fixes mostly affecting iso and cleanup from
 *	device unplugs.
 * 2000/06/28 use PCI hotplug framework, for better power management
 *	and for Cardbus support (David Brownell)
 * 2000/earlier:  fixes for NEC/Lucent chips; suspend/resume handling
 *	when the controller loses power; handle UE; cleanup; ...
 *
 * v5.2 1999/12/07 URB 3rd preview, 
 * v5.1 1999/11/30 URB 2nd preview, cpia, (usb-scsi)
 * v5.0 1999/11/22 URB Technical preview, Paul Mackerras powerbook susp/resume 
 * 	i386: HUB, Keyboard, Mouse, Printer 
 *
 * v4.3 1999/10/27 multiple HCs, bulk_request
 * v4.2 1999/09/05 ISO API alpha, new dev alloc, neg Error-codes
 * v4.1 1999/08/27 Randy Dunlap's - ISO API first impl.
 * v4.0 1999/08/18 
 * v3.0 1999/06/25 
 * v2.1 1999/05/09  code clean up
 * v2.0 1999/05/04 
 * v1.0 1999/04/27 initial release
 */
 


#define OHCI_USE_NPS		// force NoPowerSwitching mode
// #define OHCI_VERBOSE_DEBUG	/* not always helpful */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
//ZOT716u2 #include "ap_api.h"
#include "psglobal.h"
#include "usb.h"
#include "hub.h"
#include "ohci.h"
#include "list.h"
#include "star_intc.h"	//ZOT716u2

//#ifdef CONFIG_PMAC_PBOOK
//#include <asm/machdep.h>
//#include <asm/pmac_feature.h>
//#include <asm/pci-bridge.h>
//#ifndef CONFIG_PM
//#define CONFIG_PM
//#endif
//#endif

#undef OHCI_DEBUG	//635u2

//635u2	
#define SLAB_ATOMIC		GFP_ATOMIC
#define __devinit
#define __devexit
#define __devinitdata
#define __exit
#define __init
#define min_t(type,a,b) ( (a)<(b)?(a):(b) )
//ZOT==> typedef unsigned int	spinlock_t;
ohci_t		* GLOBAL_OHCI;

extern void *kaligned_alloc(size_t nb, size_t align);
extern void kaligned_free(void *block);
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
struct pci_dev{};	//eCos	

//******************************************************************

void * hcca_kmalloc( int size )
{
	char *p;
	
	p = kaligned_alloc( size, 256 );
	if( p )
		memset( p, 0, size );

	return ( p );
}

void * ohci_kmalloc( int size )
{
	char *p;
	
	p = kaligned_alloc( size, 16 );
	if( p )
		memset( p, 0, size );

	return ( p );
}

void * ohci_kfree( void *ptr )
{
	kaligned_free( ptr );	
}
//******************************************************************

/*
 * Version Information
 */
#if 0	//635u2
#define DRIVER_VERSION "v5.3"
#define DRIVER_AUTHOR "Roman Weissgaerber <weissg@vienna.at>, David Brownell"
#define DRIVER_DESC "USB OHCI Host Controller Driver"
#endif
/* For initializing controller (mask in an HCFS mode too) */
#define	OHCI_CONTROL_INIT \
	(OHCI_CTRL_CBSR & 0x3) | OHCI_CTRL_IE | OHCI_CTRL_PLE

#define OHCI_UNLINK_TIMEOUT	(HZ / 10)

static LIST_HEAD (ohci_hcd_list);
spinlock_t usb_ed_lock;	//ZOT==> static spinlock_t usb_ed_lock = SPIN_LOCK_UNLOCKED;

//PCI module
extern uint32 * pci_alloc_hcca(uint32 * framelist_dma);
extern uint32 * pci_alloc_ohci_dev(uint32 * dma);
extern void pci_free_ohci_dev(void);
extern uint32 * pci_alloc_usb_desc(uint32 * dma_addr);
extern void pci_free_usb_desc(uint32 addr);
#if 0	//ZOT716u2
#define __SQRAM__   __attribute__ ((section (".sqbuf")))
uint16 OHCI_PCI_Word_Buf __SQRAM__;
uint32 OHCI_PCI_DWord_Buf __SQRAM__;
#endif 	//ZOT716u2
cyg_handle_t ORH_SysClk;
cyg_handle_t ORH_Counter;
cyg_handle_t ORH_Alarm;  
cyg_alarm ORH_timerAlarm;
struct urb *GLOBAL_OHCI_RH_URB;
#if 0	//ZOT716u2
static uint32 readl (uint32 addr)	//615wu
{	
	pciHost_MemRead(addr, (uint32*)&OHCI_PCI_DWord_Buf, 4, 0, 0);
	return (NGET32((uint8*)&OHCI_PCI_DWord_Buf));
}

static void writel (uint32 data, uint32 addr)	//615wu
{
	OHCI_PCI_DWord_Buf = data;
	pciHost_MemWrite(addr, (uint32*)&OHCI_PCI_DWord_Buf, 4, 0, 0);
}
#endif	//ZOT716u2
//ZOT716u2
static uint32 readl (uint32 addr)
{	
	return (* ( volatile uint32 * )( addr ));
}

static void writel (uint32 data, uint32 addr)
{
	* ( volatile uint32 * )( addr )= data;
}
//ZOT716u2
cyg_interrupt USB_OHCI_interrupt;
cyg_handle_t  USB_OHCI_interrupt_handle;
//ZOT716u2
cyg_uint32 ohci_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	cyg_interrupt_mask(vector);
	cyg_interrupt_acknowledge(vector);
	return(CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);	
}
//ZOT716u2
void mdelay(unsigned int ms)
{
	udelay(ms * 1000);	//ZOT716u2
}

static int ohci_mem_init (struct ohci *ohci)
{
//original Source	//635u2	
//	ohci->td_cache = pci_pool_create ("ohci_td", ohci->ohci_dev,
//		sizeof (struct td),
//		32 /* byte alignment */,
//		0 /* no page-crossing issues */,
//		GFP_KERNEL | OHCI_MEM_FLAGS);
//	if (!ohci->td_cache)
//		return -ENOMEM;
//	ohci->dev_cache = pci_pool_create ("ohci_dev", ohci->ohci_dev,
//		sizeof (struct ohci_device),
//		16 /* byte alignment */,
//		0 /* no page-crossing issues */,
//		GFP_KERNEL | OHCI_MEM_FLAGS);
//	if (!ohci->dev_cache)
//		return -ENOMEM;
	return 0;
}

static void ohci_mem_cleanup (struct ohci *ohci)
{
	if (ohci->td_cache) {
//original Source	//635u2
//		pci_pool_destroy (ohci->td_cache);
		ohci->td_cache = 0;
	}
	if (ohci->dev_cache) {
//original Source	//635u2
//		pci_pool_destroy (ohci->dev_cache);
		ohci->dev_cache = 0;
	}
}

/* TDs ... */
static inline struct td *
td_alloc (struct ohci *hc, int mem_flags)
{
	dma_addr_t	dma;
	struct td	*td;

//original Source	//635u2
//	td = pci_pool_alloc (hc->td_cache, mem_flags, &dma);
//	if (td) {
//		td->td_dma = dma;

		/* hash it for later reverse mapping */
//		if (!hash_add_td (hc, td)) {
//			pci_pool_free (hc->td_cache, td, dma);
//			return NULL;
//		}
//	}
//719AW	td = pci_alloc_usb_desc(&dma);	//615wu
	td = ohci_kmalloc(sizeof(struct td));//td non-cache
	
	if (td) {
//719AW		td->td_dma = dma;
			td->td_dma = td;
	}	
	return td;
}

static inline void
td_free (struct ohci *hc, struct td *td)
{
//original Source	//635u2
//	hash_free_td (hc, td);
//	pci_pool_free (hc->td_cache, td, td->td_dma);
//719AW	pci_free_usb_desc(td->td_dma);	//615wu
	ohci_kfree(td->td_dma);
}


/* DEV + EDs ... only the EDs need to be consistent */
static inline struct ohci_device *
dev_alloc (struct ohci *hc, int mem_flags)
{
	dma_addr_t		dma;
	struct ohci_device	*dev;
	int			i, offset;

//original Source	//635u2
//	dev = pci_pool_alloc (hc->dev_cache, mem_flags, &dma);
//	if (dev) {
//		memset (dev, 0, sizeof (*dev));
//		dev->dma = dma;
//		offset = ((char *)&dev->ed) - ((char *)dev);
//		for (i = 0; i < NUM_EDS; i++, offset += sizeof dev->ed [0])
//			dev->ed [i].dma = dma + offset;
		/* add to hashtable if used */
//	}
//719AW	dev = pci_alloc_ohci_dev(&dma);	//615wu
	dev = ohci_kmalloc(sizeof(struct ohci_device));	//615wu
	if (dev) {
//Z		memset (dev, 0, sizeof (*dev));

//719AW		dev->dma = dma;
		memset (dev, 0, sizeof (*dev));
		dev->dma = dev;
		dma = dev;

		offset = ((char *)&dev->ed) - ((char *)dev);
		for (i = 0; i < NUM_EDS; i++, offset += sizeof dev->ed [0])
			dev->ed [i].dma = dma + offset;
	}
	return dev;
}

static inline void
dev_free (struct ohci *hc, struct ohci_device *dev)
{
//original Source	//635u2
//	pci_pool_free (hc->dev_cache, dev, dev->dma);
//719AW	pci_free_ohci_dev();	//615wu
	ohci_kfree(dev->dma);
}



/*-------------------------------------------------------------------------*/

/* AMD-756 (D2 rev) reports corrupt register contents in some cases.
 * The erratum (#4) description is incorrect.  AMD's workaround waits
 * till some bits (mostly reserved) are clear; ok for all revs.
 */
//original Source	//635u2
//#define read_roothub(hc, register, mask) ({ \
//	u32 temp = readl (&hc->regs->roothub.register); \
//	if (hc->flags & OHCI_QUIRK_AMD756) \
//		while (temp & mask) \
//			temp = readl (&hc->regs->roothub.register); \
//	temp; })

static u32 read_roothub(struct ohci *hc, __u32 reg, u32 mask) {
	__u32 temp;
	__u32 tempreg = reg;
	temp = readl (tempreg);
	if (hc->flags & OHCI_QUIRK_AMD756) 
		while (temp & mask) temp = readl (tempreg); 
	return temp;
}

//original Source	//635u2
//static u32 roothub_a (struct ohci *hc)
//	{ return read_roothub (hc, a, 0xfc0fe000); }
static u32 roothub_a (struct ohci *hc){ 
	return read_roothub (hc, &hc->regs->roothub.a, 0xfc0fe000);}
static inline u32 roothub_b (struct ohci *hc)
	{ return readl (&hc->regs->roothub.b); }
static inline u32 roothub_status (struct ohci *hc)
	{ return readl (&hc->regs->roothub.status); }
//original Source	//635u2
//static u32 roothub_portstatus (struct ohci *hc, int i)
//	{ return read_roothub (hc, portstatus [i], 0xffe0fce0); }
static u32 roothub_portstatus (struct ohci *hc, int i)
	{ return read_roothub (hc,&hc->regs->roothub.portstatus[i], 0xffe0fce0); }

/*-------------------------------------------------------------------------*
 * URB support functions 
 *-------------------------------------------------------------------------*/ 
 
/* free HCD-private data associated with this URB */

static void urb_free_priv (struct ohci *hc, urb_priv_t * urb_priv)
{
	int		i;
	int		last = urb_priv->length - 1;
	int		len;
	int		dir;
	struct td	*td;

	if (last >= 0) {

		/* ISOC, BULK, INTR data buffer starts at td 0 
		 * CTRL setup starts at td 0 */
		td = urb_priv->td [0];

		len = td->urb->transfer_buffer_length,
		dir = usb_pipeout (td->urb->pipe)
					? PCI_DMA_TODEVICE
					: PCI_DMA_FROMDEVICE;

		/* unmap CTRL URB setup */
		if (usb_pipecontrol (td->urb->pipe)) {
//original Source	//635u2	
//			pci_unmap_single (hc->ohci_dev, 
//					td->data_dma, 8, PCI_DMA_TODEVICE);

//719AW			pci_data_unmap(td->data_dma, 8,	PCI_DMA_TODEVICE,1);	//615wu

			/* CTRL data buffer starts at td 1 if len > 0 */
			if (len && last > 0)
				td = urb_priv->td [1]; 		
		} 

		/* unmap data buffer */
//original Source	//635u2	
//		if (len && td->data_dma)
//			pci_unmap_single (hc->ohci_dev, td->data_dma, len, dir);
//719AW		if (len && td->data_dma)
//719AW			pci_data_unmap(td->data_dma, len, dir, 0);	//615wu
	
		for (i = 0; i <= last; i++) {
			td = urb_priv->td [i];
			if (td)
				td_free (hc, td);
		}
	}

	free (urb_priv);	//615wu
}
 
static void urb_rm_priv_locked (struct urb * urb) 
{
	urb_priv_t * urb_priv = urb->hcpriv;
	
	if (urb_priv) {
		urb->hcpriv = NULL;

#ifdef	DO_TIMEOUTS
		if (urb->timeout) {
			list_del (&urb->urb_list);
			urb->timeout -= jiffies;
		}
#endif

		/* Release int/iso bandwidth */
		if (urb->bandwidth) {
			switch (usb_pipetype(urb->pipe)) {
			case PIPE_INTERRUPT:
				usb_release_bandwidth (urb->dev, urb, 0);
				break;
			case PIPE_ISOCHRONOUS:
				usb_release_bandwidth (urb->dev, urb, 1);
				break;
			default:
				break;
			}
		}

		urb_free_priv ((struct ohci *)urb->dev->bus->hcpriv, urb_priv);
		usb_dec_dev_use (urb->dev);
		urb->dev = NULL;
	}
}

static void urb_rm_priv (struct urb * urb)
{
	unsigned long flags;

	spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
	urb_rm_priv_locked (urb);
	spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
}

/*-------------------------------------------------------------------------*/
 
#ifdef DEBUG
static int sohci_get_current_frame_number (struct usb_device * dev);

/* debug| print the main components of an URB     
 * small: 0) header + data packets 1) just header */
 
static void urb_print (struct urb * urb, char * str, int small)
{
	unsigned int pipe= urb->pipe;
	
	if (!urb->dev || !urb->dev->bus) {
#ifdef OHCI_DEBUG	//635u2
		dbg("%s URB: no dev", str);
#endif		
		return;
	}
	
#ifndef	OHCI_VERBOSE_DEBUG
	if (urb->status != 0)
#endif
#ifdef OHCI_DEBUG	//635u2
	dbg("%s URB:[%4x] dev:%2d,ep:%2d-%c,type:%s,flags:%4x,len:%d/%d,stat:%d(%x)", 
			str,
		 	sohci_get_current_frame_number (urb->dev), 
		 	usb_pipedevice (pipe),
		 	usb_pipeendpoint (pipe), 
		 	usb_pipeout (pipe)? 'O': 'I',
		 	usb_pipetype (pipe) < 2? (usb_pipeint (pipe)? "INTR": "ISOC"):
		 		(usb_pipecontrol (pipe)? "CTRL": "BULK"),
		 	urb->transfer_flags, 
		 	urb->actual_length, 
		 	urb->transfer_buffer_length,
		 	urb->status, urb->status);
#endif		 	
#ifdef	OHCI_VERBOSE_DEBUG
	if (!small) {
		int i, len;

		if (usb_pipecontrol (pipe)) {
			printk (KERN_DEBUG __FILE__ ": cmd(8):");
			for (i = 0; i < 8 ; i++) 
				printk (" %02x", ((__u8 *) urb->setup_packet) [i]);
			printk ("\n");
		}
		if (urb->transfer_buffer_length > 0 && urb->transfer_buffer) {
			printk (KERN_DEBUG __FILE__ ": data(%d/%d):", 
				urb->actual_length, 
				urb->transfer_buffer_length);
			len = usb_pipeout (pipe)? 
						urb->transfer_buffer_length: urb->actual_length;
			for (i = 0; i < 16 && i < len; i++) 
				printk (" %02x", ((__u8 *) urb->transfer_buffer) [i]);
			printk ("%s stat:%d\n", i < len? "...": "", urb->status);
		}
	} 
#endif
}
#ifdef OHCI_DEBUG	//635u2
/* just for debugging; prints non-empty branches of the int ed tree inclusive iso eds*/
void ep_print_int_eds (ohci_t * ohci, char * str) {
	int i, j;
	 __u32 * ed_p;
	for (i= 0; i < 32; i++) {
		j = 5;
		ed_p = &(ohci->hcca->int_table [i]);
		if (*ed_p == 0)
		    continue;
		printk (KERN_DEBUG __FILE__ ": %s branch int %2d(%2x):", str, i, i);
		while (*ed_p != 0 && j--) {
			ed_t *ed = dma_to_ed (ohci, le32_to_cpup(ed_p));
			printk (" ed: %4x;", ed->hwINFO);
			ed_p = &ed->hwNextED;
		}
		printk ("\n");
	}
}
#endif

static void ohci_dump_intr_mask (char *label, __u32 mask)
{
#ifdef OHCI_DEBUG	//635u2
	dbg ("%s: 0x%08x%s%s%s%s%s%s%s%s%s",
		label,
		mask,
		(mask & OHCI_INTR_MIE) ? " MIE" : "",
		(mask & OHCI_INTR_OC) ? " OC" : "",
		(mask & OHCI_INTR_RHSC) ? " RHSC" : "",
		(mask & OHCI_INTR_FNO) ? " FNO" : "",
		(mask & OHCI_INTR_UE) ? " UE" : "",
		(mask & OHCI_INTR_RD) ? " RD" : "",
		(mask & OHCI_INTR_SF) ? " SF" : "",
		(mask & OHCI_INTR_WDH) ? " WDH" : "",
		(mask & OHCI_INTR_SO) ? " SO" : ""
		);
#endif
}

static void maybe_print_eds (char *label, __u32 value)
{
	if (value)
	{
#ifdef OHCI_DEBUG	//635u2		
		dbg ("%s %08x", label, value);
#endif		
	}
}

static char *hcfs2string (int state)
{
	switch (state) {
		case OHCI_USB_RESET:	return "reset";
		case OHCI_USB_RESUME:	return "resume";
		case OHCI_USB_OPER:	return "operational";
		case OHCI_USB_SUSPEND:	return "suspend";
	}
	return "?";
}

// dump control and status registers
static void ohci_dump_status (ohci_t *controller)
{
	struct ohci_regs	*regs = controller->regs;
	__u32			temp;

	temp = readl (&regs->revision) & 0xff;
	if (temp != 0x10)
	{
#ifdef OHCI_DEBUG	//635u2		
		dbg ("spec %d.%d", (temp >> 4), (temp & 0x0f));
#endif	
	}

	temp = readl (&regs->control);
#ifdef OHCI_DEBUG	//635u2	
	dbg ("control: 0x%08x%s%s%s HCFS=%s%s%s%s%s CBSR=%d", temp,
		(temp & OHCI_CTRL_RWE) ? " RWE" : "",
		(temp & OHCI_CTRL_RWC) ? " RWC" : "",
		(temp & OHCI_CTRL_IR) ? " IR" : "",
		hcfs2string (temp & OHCI_CTRL_HCFS),
		(temp & OHCI_CTRL_BLE) ? " BLE" : "",
		(temp & OHCI_CTRL_CLE) ? " CLE" : "",
		(temp & OHCI_CTRL_IE) ? " IE" : "",
		(temp & OHCI_CTRL_PLE) ? " PLE" : "",
		temp & OHCI_CTRL_CBSR
		);
#endif
	temp = readl (&regs->cmdstatus);
#ifdef OHCI_DEBUG	//635u2		
	dbg ("cmdstatus: 0x%08x SOC=%d%s%s%s%s", temp,
		(temp & OHCI_SOC) >> 16,
		(temp & OHCI_OCR) ? " OCR" : "",
		(temp & OHCI_BLF) ? " BLF" : "",
		(temp & OHCI_CLF) ? " CLF" : "",
		(temp & OHCI_HCR) ? " HCR" : ""
		);
#endif
	ohci_dump_intr_mask ("intrstatus", readl (&regs->intrstatus));
	ohci_dump_intr_mask ("intrenable", readl (&regs->intrenable));
	// intrdisable always same as intrenable
	// ohci_dump_intr_mask ("intrdisable", readl (&regs->intrdisable));

	maybe_print_eds ("ed_periodcurrent", readl (&regs->ed_periodcurrent));

	maybe_print_eds ("ed_controlhead", readl (&regs->ed_controlhead));
	maybe_print_eds ("ed_controlcurrent", readl (&regs->ed_controlcurrent));

	maybe_print_eds ("ed_bulkhead", readl (&regs->ed_bulkhead));
	maybe_print_eds ("ed_bulkcurrent", readl (&regs->ed_bulkcurrent));

	maybe_print_eds ("donehead", readl (&regs->donehead));
}

static void ohci_dump_roothub (ohci_t *controller, int verbose)
{
	__u32			temp, ndp, i;

	temp = roothub_a (controller);
	if (temp == ~(u32)0)
		return;
	ndp = (temp & RH_A_NDP);

	if (verbose) {
#ifdef OHCI_DEBUG	//635u2	
		dbg ("roothub.a: %08x POTPGT=%d%s%s%s%s%s NDP=%d", temp,
			((temp & RH_A_POTPGT) >> 24) & 0xff,
			(temp & RH_A_NOCP) ? " NOCP" : "",
			(temp & RH_A_OCPM) ? " OCPM" : "",
			(temp & RH_A_DT) ? " DT" : "",
			(temp & RH_A_NPS) ? " NPS" : "",
			(temp & RH_A_PSM) ? " PSM" : "",
			ndp
			);
#endif
		temp = roothub_b (controller);
#ifdef OHCI_DEBUG	//635u2	
		dbg ("roothub.b: %08x PPCM=%04x DR=%04x",
			temp,
			(temp & RH_B_PPCM) >> 16,
			(temp & RH_B_DR)
			);
#endif			
		temp = roothub_status (controller);

#ifdef OHCI_DEBUG	//635u2	
		dbg ("roothub.status: %08x%s%s%s%s%s%s",
			temp,
			(temp & RH_HS_CRWE) ? " CRWE" : "",
			(temp & RH_HS_OCIC) ? " OCIC" : "",
			(temp & RH_HS_LPSC) ? " LPSC" : "",
			(temp & RH_HS_DRWE) ? " DRWE" : "",
			(temp & RH_HS_OCI) ? " OCI" : "",
			(temp & RH_HS_LPS) ? " LPS" : ""
			);
#endif
	}
	
	for (i = 0; i < ndp; i++) {
		temp = roothub_portstatus (controller, i);
#ifdef OHCI_DEBUG	//635u2	
		dbg ("roothub.portstatus [%d] = 0x%08x%s%s%s%s%s%s%s%s%s%s%s%s",
			i,
			temp,
			(temp & RH_PS_PRSC) ? " PRSC" : "",
			(temp & RH_PS_OCIC) ? " OCIC" : "",
			(temp & RH_PS_PSSC) ? " PSSC" : "",
			(temp & RH_PS_PESC) ? " PESC" : "",
			(temp & RH_PS_CSC) ? " CSC" : "",

			(temp & RH_PS_LSDA) ? " LSDA" : "",
			(temp & RH_PS_PPS) ? " PPS" : "",
			(temp & RH_PS_PRS) ? " PRS" : "",
			(temp & RH_PS_POCI) ? " POCI" : "",
			(temp & RH_PS_PSS) ? " PSS" : "",

			(temp & RH_PS_PES) ? " PES" : "",
			(temp & RH_PS_CCS) ? " CCS" : ""
			);
#endif
	}
}

static void ohci_dump (ohci_t *controller, int verbose)
{
#ifdef OHCI_DEBUG	//635u2	
	dbg ("OHCI controller usb-%s state", controller->ohci_dev->slot_name);
#endif
	// dumps some of the state we know about
	ohci_dump_status (controller);
#ifdef OHCI_DEBUG	//635u2
	if (verbose)
		ep_print_int_eds (controller, "hcca");		
	dbg ("hcca frame #%04x", controller->hcca->frame_no);
#endif
	ohci_dump_roothub (controller, 1);
}


#endif

/*-------------------------------------------------------------------------*
 * Interface functions (URB)
 *-------------------------------------------------------------------------*/

/* return a request to the completion handler */
 
static int sohci_return_urb (struct ohci *hc, struct urb * urb)
{
	urb_priv_t * urb_priv = urb->hcpriv;
	struct urb * urbt;
	unsigned long flags;
	int i;
	
	if (!urb_priv)
		return -1; /* urb already unlinked */

	/* just to be sure */
	if (!urb->complete) {
		urb_rm_priv (urb);
		return -1;
	}
	
#ifdef DEBUG
	urb_print (urb, "RET", usb_pipeout (urb->pipe));
#endif

	switch (usb_pipetype (urb->pipe)) {
  		case PIPE_INTERRUPT:
//original Source	//635u2	
//			pci_unmap_single (hc->ohci_dev,
//				urb_priv->td [0]->data_dma,
//				urb->transfer_buffer_length,
//				usb_pipeout (urb->pipe)
//					? PCI_DMA_TODEVICE
//					: PCI_DMA_FROMDEVICE);

//719AW			pci_data_unmap(urb_priv->td [0]->data_dma, urb->transfer_buffer_length,
//719AW				(usb_pipeout (urb->pipe) ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE), 0);	//615wu

			urb->complete (urb);

			/* implicitly requeued */
  			urb->actual_length = 0;
			urb->status = -EINPROGRESS;
			td_submit_urb (urb);
  			break;
  			
		case PIPE_ISOCHRONOUS:
			for (urbt = urb->next; urbt && (urbt != urb); urbt = urbt->next);
			if (urbt) { /* send the reply and requeue URB */	
//original Source	//635u2	
//				pci_unmap_single (hc->ohci_dev,
//					urb_priv->td [0]->data_dma,
//					urb->transfer_buffer_length,
//					usb_pipeout (urb->pipe)
//						? PCI_DMA_TODEVICE
//						: PCI_DMA_FROMDEVICE);

//719AW				pci_data_unmap(urb_priv->td [0]->data_dma, urb->transfer_buffer_length,
//719AW					(usb_pipeout (urb->pipe) ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE), 0);	//615wu

				urb->complete (urb);
				spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
				urb->actual_length = 0;
  				urb->status = USB_ST_URB_PENDING;
  				urb->start_frame = urb_priv->ed->last_iso + 1;
  				if (urb_priv->state != URB_DEL) {
  					for (i = 0; i < urb->number_of_packets; i++) {
  						urb->iso_frame_desc[i].actual_length = 0;
  						urb->iso_frame_desc[i].status = -EXDEV;
  					}
  					td_submit_urb (urb);
  				}
  				spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
  				
  			} else { /* unlink URB, call complete */
				urb_rm_priv (urb);
				urb->complete (urb); 	
			}		
			break;
  				
		case PIPE_BULK:
		case PIPE_CONTROL: /* unlink URB, call complete */
			urb_rm_priv (urb);
			urb->complete (urb);	
			break;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* get a transfer request */
 
static int sohci_submit_urb (struct urb * urb)
{
	ohci_t * ohci;
	ed_t * ed;
	urb_priv_t * urb_priv;
	unsigned int pipe = urb->pipe;
	int maxps = usb_maxpacket (urb->dev, pipe, usb_pipeout (pipe));
	int i, size = 0;
	unsigned long flags;
	int bustime = 0;
	int mem_flags = ALLOC_FLAGS;
	
	if (!urb->dev || !urb->dev->bus)
		return -ENODEV;
	
	if (urb->hcpriv)			/* urb already in use */
		return -EINVAL;

//	if(usb_endpoint_halted (urb->dev, usb_pipeendpoint (pipe), usb_pipeout (pipe))) 
//		return -EPIPE;
	
	usb_inc_dev_use (urb->dev);
	ohci = (ohci_t *) urb->dev->bus->hcpriv;
	
#ifdef DEBUG
	urb_print (urb, "SUB", usb_pipein (pipe));
#endif

//_____________________________________________________________	
//ZOT716u2	if (ohci->rh.send) {	
//ZOT716u2		armond_printf("my stop OHCI Detcet\n");
//ZOT716u2		cyg_alarm_delete(ORH_Alarm);
//ZOT716u2		cyg_clock_delete(ORH_SysClk);
//ZOT716u2		cyg_counter_delete(ORH_Counter);
//ZOT716u2	}
//_____________________________________________________________	
	
	/* handle a request to the virtual root hub */
	if (usb_pipedevice (pipe) == ohci->rh.devnum) 
		return rh_submit_urb (urb);
	
	/* when controller's hung, permit only roothub cleanup attempts
	 * such as powering down ports */
	if (ohci->disabled) {
		usb_dec_dev_use (urb->dev);	
		
//_____________________________________________________________		
//ZOT716u2		if (ohci->rh.send) {
//ZOT716u2			armond_printf("my start OHCI Detcet\n");
//ZOT716u2			rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2		}
//_____________________________________________________________	
    				
		return -ESHUTDOWN;
	}

	/* every endpoint has a ed, locate and fill it */
	if (!(ed = ep_add_ed (urb->dev, pipe, urb->interval, 1, mem_flags))) {
		usb_dec_dev_use (urb->dev);	

//_____________________________________________________________		
//ZOT716u2		if (ohci->rh.send) {
//ZOT716u2			armond_printf("my start OHCI Detcet\n");
//ZOT716u2			rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2		}
//_____________________________________________________________	
    	
		return -ENOMEM;
	}

	/* for the private part of the URB we need the number of TDs (size) */
	switch (usb_pipetype (pipe)) {
		case PIPE_BULK:	/* one TD for every 4096 Byte */
			size = (urb->transfer_buffer_length - 1) / 4096 + 1;

			/* If the transfer size is multiple of the pipe mtu,
			 * we may need an extra TD to create a empty frame
			 * Jean II */
			if ((urb->transfer_flags & USB_ZERO_PACKET) &&
			    usb_pipeout (pipe) &&
			    (urb->transfer_buffer_length != 0) && 
			    ((urb->transfer_buffer_length % maxps) == 0))
				size++;
			break;
		case PIPE_ISOCHRONOUS: /* number of packets from URB */
			size = urb->number_of_packets;
			if (size <= 0) {
				usb_dec_dev_use (urb->dev);	
				return -EINVAL;
			}
			for (i = 0; i < urb->number_of_packets; i++) {
  				urb->iso_frame_desc[i].actual_length = 0;
  				urb->iso_frame_desc[i].status = -EXDEV;
  			}
			break;
		case PIPE_CONTROL: /* 1 TD for setup, 1 for ACK and 1 for every 4096 B */
			size = (urb->transfer_buffer_length == 0)? 2: 
						(urb->transfer_buffer_length - 1) / 4096 + 3;
			break;
		case PIPE_INTERRUPT: /* one TD */
			size = 1;
			break;
	}

	/* allocate the private part of the URB */
//original Source	//635u2	
//	urb_priv = kmalloc (sizeof (urb_priv_t) + size * sizeof (td_t *), 
//							in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

	urb_priv = malloc ( (sizeof (urb_priv_t) + size * sizeof (td_t *)) );	//615wu
	if (!urb_priv) {
		usb_dec_dev_use (urb->dev);	
		
//_____________________________________________________________		
//ZOT716u2		if (ohci->rh.send) {
//ZOT716u2			armond_printf("my start OHCI Detcet\n");
//ZOT716u2			rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2		}
//_____________________________________________________________	
    			
		return -ENOMEM;
	}
	memset (urb_priv, 0, sizeof (urb_priv_t) + size * sizeof (td_t *));
	
	/* fill the private part of the URB */
	urb_priv->length = size;
	urb_priv->ed = ed;	

	/* allocate the TDs (updating hash chains) */
	spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
	for (i = 0; i < size; i++) { 
		urb_priv->td[i] = td_alloc (ohci, SLAB_ATOMIC);
		if (!urb_priv->td[i]) {
			urb_priv->length = i;
			urb_free_priv (ohci, urb_priv);
			spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
			usb_dec_dev_use (urb->dev);	
			
//_____________________________________________________________		
//ZOT716u2			if (ohci->rh.send) {
//ZOT716u2				armond_printf("my start OHCI Detcet\n");
//ZOT716u2				rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2			}
//_____________________________________________________________	
    				
			return -ENOMEM;
		}
	}	

	if (ed->state == ED_NEW || (ed->state & ED_DEL)) {
		urb_free_priv (ohci, urb_priv);
		spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
		usb_dec_dev_use (urb->dev);	
		
//_____________________________________________________________		
//ZOT716u2		if (ohci->rh.send) {
//ZOT716u2			armond_printf("my start OHCI Detcet\n");
//ZOT716u2			rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2		}
//_____________________________________________________________	
    			
		return -EINVAL;
	}
	
	/* allocate and claim bandwidth if needed; ISO
	 * needs start frame index if it was't provided.
	 */
	switch (usb_pipetype (pipe)) {
		case PIPE_ISOCHRONOUS:
			if (urb->transfer_flags & USB_ISO_ASAP) { 
				urb->start_frame = ((ed->state == ED_OPER)
					? (ed->last_iso + 1)
					: (le16_to_cpu (ohci->hcca->frame_no) + 10)) & 0xffff;
			}	
			/* FALLTHROUGH */
		case PIPE_INTERRUPT:
			if (urb->bandwidth == 0) {
				bustime = usb_check_bandwidth (urb->dev, urb);
			}
			if (bustime < 0) {
				urb_free_priv (ohci, urb_priv);
				spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
				usb_dec_dev_use (urb->dev);	

//_____________________________________________________________		
//ZOT716u2			if (ohci->rh.send) {
//ZOT716u2				armond_printf("my start OHCI Detcet\n");
//ZOT716u2				rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2			}
//_____________________________________________________________	
    	
				return bustime;
			}
			usb_claim_bandwidth (urb->dev, urb, bustime, usb_pipeisoc (urb->pipe));
#ifdef	DO_TIMEOUTS
			urb->timeout = 0;
#endif
	}

	urb->actual_length = 0;
	urb->hcpriv = urb_priv;
	urb->status = USB_ST_URB_PENDING;

	/* link the ed into a chain if is not already */
	if (ed->state != ED_OPER)
		ep_link (ohci, ed);

	/* fill the TDs and link it to the ed */
	td_submit_urb (urb);

#ifdef	DO_TIMEOUTS
	/* maybe add to ordered list of timeouts */
	if (urb->timeout) {
		struct list_head	*entry;

		// FIXME:  usb-uhci uses relative timeouts (like this),
		// while uhci uses absolute ones (probably better).
		// Pick one solution and change the affected drivers.
		urb->timeout += jiffies;

		list_for_each (entry, &ohci->timeout_list) {
			struct urb	*next_urb;

			next_urb = list_entry (entry, struct urb, urb_list);
			if (time_after_eq (urb->timeout, next_urb->timeout))
				break;
		}
		list_add (&urb->urb_list, entry);

		/* drive timeouts by SF (messy, but works) */
		writel (OHCI_INTR_SF, &ohci->regs->intrenable);	
	}
#endif

	spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);

//_____________________________________________________________		
//ZOT716u2	if (ohci->rh.send) {
//ZOT716u2		armond_printf("my start OHCI Detcet\n");
//ZOT716u2		rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2	}
//_____________________________________________________________	

	return 0;	
}

/*-------------------------------------------------------------------------*/

/* deactivate all TDs and remove the private part of the URB */
/* interrupt callers must use async unlink mode */

static int sohci_unlink_urb (struct urb * urb)
{
	unsigned long flags;
	ohci_t * ohci;
	
	if (!urb) /* just to be sure */ 
		return -EINVAL;
		
	if (!urb->dev || !urb->dev->bus)
		return -ENODEV;

	ohci = (ohci_t *) urb->dev->bus->hcpriv; 

#ifdef DEBUG
	urb_print (urb, "UNLINK", 1);
#endif		  

	/* handle a request to the virtual root hub */
	if (usb_pipedevice (urb->pipe) == ohci->rh.devnum)
		return rh_unlink_urb (urb);

	if (urb->hcpriv && (urb->status == USB_ST_URB_PENDING)) { 
		if (!ohci->disabled) {
			urb_priv_t  * urb_priv;

			/* interrupt code may not sleep; it must use
			 * async status return to unlink pending urbs.
			 */
			if (!(urb->transfer_flags & USB_ASYNC_UNLINK)
					&& in_interrupt ()) {
#ifdef OHCI_DEBUG	//635u2						
				err ("bug in call from %p; use async!",
					__builtin_return_address(0));
#endif				
				return -EWOULDBLOCK;
			}

			/* flag the urb and its TDs for deletion in some
			 * upcoming SF interrupt delete list processing
			 */
			spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
			urb_priv = urb->hcpriv;

			if (!urb_priv || (urb_priv->state == URB_DEL)) {
				spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
				return 0;
			}
				
			urb_priv->state = URB_DEL; 
			ep_rm_ed (urb->dev, urb_priv->ed);
			urb_priv->ed->state |= ED_URB_DEL;

			if (!(urb->transfer_flags & USB_ASYNC_UNLINK)) {
				DECLARE_WAIT_QUEUE_HEAD (unlink_wakeup); 
//original Source	//635u2	
//				DECLARE_WAITQUEUE (wait, current);
				int timeout = OHCI_UNLINK_TIMEOUT;

//original Source	//635u2	
//				add_wait_queue (&unlink_wakeup, &wait);
				urb_priv->wait = &unlink_wakeup;
				spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);

				/* wait until all TDs are deleted */
//original Source	//635u2	
//				set_current_state(TASK_UNINTERRUPTIBLE);
//original Source	//635u2
//				while (timeout && (urb->status == USB_ST_URB_PENDING))	
//					timeout = schedule_timeout (timeout);
//				set_current_state(TASK_RUNNING);
//				remove_wait_queue (&unlink_wakeup, &wait); 
				if (urb->status == USB_ST_URB_PENDING) {
#ifdef OHCI_DEBUG	//635u2
					err ("unlink URB timeout");
#endif					
					return -ETIMEDOUT;
				}
			} else {
				/* usb_dec_dev_use done in dl_del_list() */
				urb->status = -EINPROGRESS;
				spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
				return -EINPROGRESS;
			}
		} else {
			urb_rm_priv (urb);
			if (urb->transfer_flags & USB_ASYNC_UNLINK) {
				urb->status = -ECONNRESET;
				if (urb->complete)
					urb->complete (urb); 
			} else 
				urb->status = -ENOENT;
		}	
	}	
	return 0;
}

/*-------------------------------------------------------------------------*/

/* allocate private data space for a usb device */

static int sohci_alloc_dev (struct usb_device *usb_dev)
{
	struct ohci_device * dev;

	dev = dev_alloc ((struct ohci *) usb_dev->bus->hcpriv, ALLOC_FLAGS);
	if (!dev)
		return -ENOMEM;

	usb_dev->hcpriv = dev;
	return 0;
}

/*-------------------------------------------------------------------------*/

/* may be called from interrupt context */
/* frees private data space of usb device */
  
static int sohci_free_dev (struct usb_device * usb_dev)
{
	unsigned long flags;
	int i, cnt = 0;
	ed_t * ed;
	struct ohci_device * dev = usb_to_ohci (usb_dev);
	ohci_t * ohci = usb_dev->bus->hcpriv;
	
	if (!dev)
		return 0;
	
	if (usb_dev->devnum >= 0) {
	
		/* driver disconnects should have unlinked all urbs
		 * (freeing all the TDs, unlinking EDs) but we need
		 * to defend against bugs that prevent that.
		 */
		spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);	
		for(i = 0; i < NUM_EDS; i++) {
  			ed = &(dev->ed[i]);
  			if (ed->state != ED_NEW) {
  				if (ed->state == ED_OPER) {
					/* driver on that interface didn't unlink an urb */
#ifdef OHCI_DEBUG	//635u2	
					dbg ("driver usb-%s dev %d ed 0x%x unfreed URB",
						ohci->ohci_dev->slot_name, usb_dev->devnum, i);
#endif						
					ep_unlink (ohci, ed);
				}
  				ep_rm_ed (usb_dev, ed);
  				ed->state = ED_DEL;
  				cnt++;
  			}
  		}
  		spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
  		
		/* if the controller is running, tds for those unlinked
		 * urbs get freed by dl_del_list at the next SF interrupt
		 */
		if (cnt > 0) {

			if (ohci->disabled) {
				/* FIXME: Something like this should kick in,
				 * though it's currently an exotic case ...
				 * the controller won't ever be touching
				 * these lists again!!
				dl_del_list (ohci,
					le16_to_cpu (ohci->hcca->frame_no) & 1);
				 */
#ifdef OHCI_DEBUG	//635u2
				warn ("TD leak, %d", cnt);
#endif
			} else if (!in_interrupt ()) {
				DECLARE_WAIT_QUEUE_HEAD (freedev_wakeup); 
//original Source	//635u2	
//				DECLARE_WAITQUEUE (wait, current);
				int timeout = OHCI_UNLINK_TIMEOUT*1000;

				/* SF interrupt handler calls dl_del_list */
//original Source	//635u2	
//				add_wait_queue (&freedev_wakeup, &wait);
				dev->wait = &freedev_wakeup;
//original Source	//635u2	
//				set_current_state(TASK_UNINTERRUPTIBLE);
//original Source	//635u2
//				while (timeout && dev->ed_cnt)	
//					timeout = schedule_timeout (timeout);
//				set_current_state(TASK_RUNNING);
//				remove_wait_queue (&freedev_wakeup, &wait);
				while (timeout && dev->ed_cnt)
					timeout--;
				//ppause(timeout);	//635u2 waiting for interrupt to finish "dl_del_list" (dev->ed_cnt)

				if (dev->ed_cnt) {
#ifdef OHCI_DEBUG	//635u2					
					err ("free device %d timeout", usb_dev->devnum);
#endif
//original Source	//635u2
//					return -ETIMEDOUT;
				}
			} else {
				/* likely some interface's driver has a refcount bug */
#ifdef OHCI_DEBUG	//635u2				
				err ("bus %s devnum %d deletion in interrupt",
					ohci->ohci_dev->slot_name, usb_dev->devnum);
				BUG ();
#endif				
			}
		}
	}

	/* free device, and associated EDs */
	dev_free (ohci, dev);

	return 0;
}

/*-------------------------------------------------------------------------*/

/* tell us the current USB frame number */

static int sohci_get_current_frame_number (struct usb_device *usb_dev) 
{
	ohci_t * ohci = usb_dev->bus->hcpriv;
	
	return le16_to_cpu (ohci->hcca->frame_no);
}

/*-------------------------------------------------------------------------*/
//Setup for 635u2
struct usb_operations sohci_device_operations = {
	sohci_alloc_dev,
	sohci_free_dev,
	sohci_get_current_frame_number,
	sohci_submit_urb,
	sohci_unlink_urb
};

/*-------------------------------------------------------------------------*
 * ED handling functions
 *-------------------------------------------------------------------------*/  
		
/* search for the right branch to insert an interrupt ed into the int tree 
 * do some load ballancing;
 * returns the branch and 
 * sets the interval to interval = 2^integer (ld (interval)) */

static int ep_int_ballance (ohci_t * ohci, int interval, int load)
{
	int i, branch = 0;
   
	/* search for the least loaded interrupt endpoint branch of all 32 branches */
	for (i = 0; i < 32; i++) 
		if (ohci->ohci_int_load [branch] > ohci->ohci_int_load [i]) branch = i; 
  
	branch = branch % interval;
	for (i = branch; i < 32; i += interval) ohci->ohci_int_load [i] += load;

	return branch;
}

/*-------------------------------------------------------------------------*/

/*  2^int( ld (inter)) */

static int ep_2_n_interval (int inter)
{	
	int i;
	for (i = 0; ((inter >> i) > 1 ) && (i < 5); i++); 
	return 1 << i;
}

/*-------------------------------------------------------------------------*/

/* the int tree is a binary tree 
 * in order to process it sequentially the indexes of the branches have to be mapped 
 * the mapping reverses the bits of a word of num_bits length */
 
static int ep_rev (int num_bits, int word)
{
	int i, wout = 0;

	for (i = 0; i < num_bits; i++) wout |= (((word >> i) & 1) << (num_bits - i - 1));
	return wout;
}

/*-------------------------------------------------------------------------*/

/* link an ed into one of the HC chains */

static int ep_link (ohci_t * ohci, ed_t * edi)
{	 
	int int_branch;
	int i;
	int inter;
	int interval;
	int load;
	__u32 * ed_p;
	volatile ed_t * ed = edi;
	
	ed->state = ED_OPER;
	
	switch (ed->type) {
	case PIPE_CONTROL:
		ed->hwNextED = 0;
		if (ohci->ed_controltail == NULL) {
//original Source	//635u2	
//			writel (ed->dma, &ohci->regs->ed_controlhead);
			writel ((uint32)ed->dma, &ohci->regs->ed_controlhead);	//615wu
		} else {
//original Source	//635u2	
//			ohci->ed_controltail->hwNextED = cpu_to_le32 (ed->dma);
			ohci->ed_controltail->hwNextED = (uint32)ed->dma;	//615wu
		}
		ed->ed_prev = ohci->ed_controltail;
		if (!ohci->ed_controltail && !ohci->ed_rm_list[0] &&
			!ohci->ed_rm_list[1] && !ohci->sleeping) {
			ohci->hc_control |= OHCI_CTRL_CLE;
//original Source	//635u2	
//			writel (ohci->hc_control, &ohci->regs->control);
			writel ((uint32)ohci->hc_control, &ohci->regs->control);
		}
		ohci->ed_controltail = edi;	  
		break;
		
	case PIPE_BULK:
		ed->hwNextED = 0;
		if (ohci->ed_bulktail == NULL) {
//original Source	//635u2	
//			writel (ed->dma, &ohci->regs->ed_bulkhead);
			writel ((uint32)ed->dma, &ohci->regs->ed_bulkhead);	//615wu
		} else {
//original Source	//635u2	
//			ohci->ed_bulktail->hwNextED = cpu_to_le32 (ed->dma);
			ohci->ed_bulktail->hwNextED = (uint32)ed->dma;	//615wu
		}
		ed->ed_prev = ohci->ed_bulktail;
		if (!ohci->ed_bulktail && !ohci->ed_rm_list[0] &&
			!ohci->ed_rm_list[1] && !ohci->sleeping) {
			ohci->hc_control |= OHCI_CTRL_BLE;
			writel (ohci->hc_control, &ohci->regs->control);
		}
		ohci->ed_bulktail = edi;	  
		break;
		
	case PIPE_INTERRUPT:
		load = ed->int_load;
		interval = ep_2_n_interval (ed->int_period);
		ed->int_interval = interval;
		int_branch = ep_int_ballance (ohci, interval, load);
		ed->int_branch = int_branch;
		
		for (i = 0; i < ep_rev (6, interval); i += inter) {
			inter = 1;
//original Source	//635u2	
//			for (ed_p = &(ohci->hcca->int_table[ep_rev (5, i) + int_branch]); 
//				(*ed_p != 0) && ((dma_to_ed (ohci, le32_to_cpup (ed_p)))->int_interval >= interval); 
//				ed_p = &((dma_to_ed (ohci, le32_to_cpup (ed_p)))->hwNextED)) 
//					inter = ep_rev (6, (dma_to_ed (ohci, le32_to_cpup (ed_p)))->int_interval);
//			ed->hwNextED = *ed_p; 
//			*ed_p = cpu_to_le32 (ed->dma);

			for (ed_p = &(ohci->hcca->int_table[ep_rev (5, i) + int_branch]); 
				(*ed_p != 0) && ( ( (ed_t *) bus_to_virt( le32_to_cpup (ed_p)))->int_interval >= interval); 
				ed_p = &(((ed_t *) bus_to_virt( le32_to_cpup (ed_p)))->hwNextED)) 
					inter = ep_rev (6, ((ed_t *) bus_to_virt( le32_to_cpup (ed_p)))->int_interval);
			ed->hwNextED = *ed_p; 
			*ed_p = (uint32) (ed->dma);	//615wu
		}
#ifdef DEBUG
		ep_print_int_eds (ohci, "LINK_INT");
#endif
		break;
		
	case PIPE_ISOCHRONOUS:
		ed->hwNextED = 0;
		ed->int_interval = 1;
		if (ohci->ed_isotail != NULL) {
//original Source	//635u2	
//			ohci->ed_isotail->hwNextED = cpu_to_le32 (ed->dma);
			ohci->ed_isotail->hwNextED = ed->dma;	//615wu
			ed->ed_prev = ohci->ed_isotail;
		} else {
			for ( i = 0; i < 32; i += inter) {
				inter = 1;
				for (ed_p = &(ohci->hcca->int_table[ep_rev (5, i)]); 
					*ed_p != 0; 
//original Source	//635u2	
//					ed_p = &((dma_to_ed (ohci, le32_to_cpup (ed_p)))->hwNextED)) 
//						inter = ep_rev (6, (dma_to_ed (ohci, le32_to_cpup (ed_p)))->int_interval);
//				*ed_p = cpu_to_le32 (ed->dma);	
					ed_p = &(((ed_t *) bus_to_virt (le32_to_cpup (ed_p)))->hwNextED)) 
					inter = ep_rev (6, ((ed_t *) bus_to_virt (le32_to_cpup (ed_p)))->int_interval);
				*ed_p = (uint32) (ed->dma);	//615wu
			}	
			ed->ed_prev = NULL;
		}	
		ohci->ed_isotail = edi;  
#ifdef DEBUG
		ep_print_int_eds (ohci, "LINK_ISO");
#endif
		break;
	}	 	
	return 0;
}

/*-------------------------------------------------------------------------*/

/* scan the periodic table to find and unlink this ED */
static void periodic_unlink (
	struct ohci	*ohci,
	struct ed	*ed,
	unsigned	index,
	unsigned	period
) {
	for (; index < NUM_INTS; index += period) {
		__u32	*ed_p = &ohci->hcca->int_table [index];

		/* ED might have been unlinked through another path */
		while (*ed_p != 0) {
//original Source	//635u2	
//			if ((dma_to_ed (ohci, le32_to_cpup (ed_p))) == ed) {
			if (((ed_t *) bus_to_virt( le32_to_cpup (ed_p))) == ed) {
				*ed_p = ed->hwNextED;		
				break;
			}
//original Source	//635u2	
//			ed_p = & ((dma_to_ed (ohci,
//				le32_to_cpup (ed_p)))->hwNextED);
			ed_p = & (((ed_t *) bus_to_virt( le32_to_cpup (ed_p)))->hwNextED);
		}
	}	
}

/* unlink an ed from one of the HC chains. 
 * just the link to the ed is unlinked.
 * the link from the ed still points to another operational ed or 0
 * so the HC can eventually finish the processing of the unlinked ed */

static int ep_unlink (ohci_t * ohci, ed_t * ed) 
{
	int i;

//original Source	//635u2	
//	ed->hwINFO |= cpu_to_le32 (OHCI_ED_SKIP);
	ed->hwINFO |=  (OHCI_ED_SKIP);	//635u2
	
	switch (ed->type) {
	case PIPE_CONTROL:
		if (ed->ed_prev == NULL) {
			if (!ed->hwNextED) {
				ohci->hc_control &= ~OHCI_CTRL_CLE;
				writel (ohci->hc_control, &ohci->regs->control);
			}
			writel (le32_to_cpup (&ed->hwNextED), &ohci->regs->ed_controlhead);
		} else {
			ed->ed_prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_controltail == ed) {
			ohci->ed_controltail = ed->ed_prev;
		} else {
//original Source	//635u2	
//			(dma_to_ed (ohci, le32_to_cpup (&ed->hwNextED)))->ed_prev = ed->ed_prev;
			((ed_t *)bus_to_virt(le32_to_cpup(&ed->hwNextED)))->ed_prev = ed->ed_prev;	//635u2
		}
		break;
      
	case PIPE_BULK:
		if (ed->ed_prev == NULL) {
			if (!ed->hwNextED) {
				ohci->hc_control &= ~OHCI_CTRL_BLE;
				writel (ohci->hc_control, &ohci->regs->control);
			}
			writel (le32_to_cpup (&ed->hwNextED), &ohci->regs->ed_bulkhead);
		} else {
			ed->ed_prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_bulktail == ed) {
			ohci->ed_bulktail = ed->ed_prev;
		} else {
//original Source	//635u2	
//			(dma_to_ed (ohci, le32_to_cpup (&ed->hwNextED)))->ed_prev = ed->ed_prev;
			((ed_t *)bus_to_virt(le32_to_cpup(&ed->hwNextED)))->ed_prev = ed->ed_prev;	//635u2
		}
		break;
      
	case PIPE_INTERRUPT:
		periodic_unlink (ohci, ed, 0, 1);
		for (i = ed->int_branch; i < 32; i += ed->int_interval)
		    ohci->ohci_int_load[i] -= ed->int_load;
#ifdef DEBUG
		ep_print_int_eds (ohci, "UNLINK_INT");
#endif
		break;
		
	case PIPE_ISOCHRONOUS:
		if (ohci->ed_isotail == ed)
			ohci->ed_isotail = ed->ed_prev;
		if (ed->hwNextED != 0) 
//original Source	//635u2	
//		    (dma_to_ed (ohci, le32_to_cpup (&ed->hwNextED)))->ed_prev = ed->ed_prev;
			((ed_t *) bus_to_virt( le32_to_cpup (&ed->hwNextED)))->ed_prev = ed->ed_prev;
				    
		if (ed->ed_prev != NULL)
			ed->ed_prev->hwNextED = ed->hwNextED;
		else
			periodic_unlink (ohci, ed, 0, 1);
#ifdef DEBUG
		ep_print_int_eds (ohci, "UNLINK_ISO");
#endif
		break;
	}
	ed->state = ED_UNLINK;
	return 0;
}


/*-------------------------------------------------------------------------*/

/* add/reinit an endpoint; this should be done once at the usb_set_configuration command,
 * but the USB stack is a little bit stateless  so we do it at every transaction
 * if the state of the ed is ED_NEW then a dummy td is added and the state is changed to ED_UNLINK
 * in all other cases the state is left unchanged
 * the ed info fields are setted anyway even though most of them should not change */
 
static ed_t * ep_add_ed (
	struct usb_device * usb_dev,
	unsigned int pipe,
	int interval,
	int load,
	int mem_flags
)
{
   	ohci_t * ohci = usb_dev->bus->hcpriv;
	td_t * td;
	ed_t * ed_ret;
	volatile ed_t * ed; 
	unsigned long flags;
 	
	spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
	
	ed = ed_ret = &(usb_to_ohci (usb_dev)->ed[(usb_pipeendpoint (pipe) << 1) | 
			(usb_pipecontrol (pipe)? 0: usb_pipeout (pipe))]);	

	if ((ed->state & ED_DEL) || (ed->state & ED_URB_DEL)) {
		/* pending delete request */
		spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
		return NULL;
	}
	
	if (ed->state == ED_NEW) {
//original Source	//635u2	
//		ed->hwINFO = cpu_to_le32 (OHCI_ED_SKIP); /* skip ed */
		ed->hwINFO =  OHCI_ED_SKIP;	//635u2
  		/* dummy td; end of td list for ed */
		td = td_alloc (ohci, SLAB_ATOMIC);
		/* hash the ed for later reverse mapping */
//original Source	//635u2	
// 		if (!td || !hash_add_ed (ohci, (ed_t *)ed)) {
		if(!td){	//635u2
			/* out of memory */
		        if (td)
		            td_free(ohci, td);
			spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
			return NULL;
		}
//original Source	//635u2	
//		ed->hwTailP = cpu_to_le32 (td->td_dma);
		ed->hwTailP = td->td_dma;    //615wu
		ed->hwHeadP = ed->hwTailP;	
		ed->state = ED_UNLINK;
		ed->type = usb_pipetype (pipe);
		usb_to_ohci (usb_dev)->ed_cnt++;
	}

	ohci->dev[usb_pipedevice (pipe)] = usb_dev;
	
//original Source	//635u2	
//	ed->hwINFO = cpu_to_le32 (usb_pipedevice (pipe)
	ed->hwINFO = (usb_pipedevice (pipe)	//635u2
			| usb_pipeendpoint (pipe) << 7
			| (usb_pipeisoc (pipe)? 0x8000: 0)
			| (usb_pipecontrol (pipe)? 0: (usb_pipeout (pipe)? 0x800: 0x1000)) 
			| usb_pipeslow (pipe) << 13
			| usb_maxpacket (usb_dev, pipe, usb_pipeout (pipe)) << 16);
  
  	if (ed->type == PIPE_INTERRUPT && ed->state == ED_UNLINK) {
  		ed->int_period = interval;
  		ed->int_load = load;
  	}
  	
	spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
	return ed_ret; 
}

/*-------------------------------------------------------------------------*/
 
/* request the removal of an endpoint
 * put the ep on the rm_list and request a stop of the bulk or ctrl list 
 * real removal is done at the next start frame (SF) hardware interrupt */
 
static void ep_rm_ed (struct usb_device * usb_dev, ed_t * ed)
{    
	unsigned int frame;
	ohci_t * ohci = usb_dev->bus->hcpriv;

	if ((ed->state & ED_DEL) || (ed->state & ED_URB_DEL))
		return;
	
//original Source	//635u2	
//	ed->hwINFO |= cpu_to_le32 (OHCI_ED_SKIP);
	ed->hwINFO |=  (OHCI_ED_SKIP);
	
	if (!ohci->disabled) {
		switch (ed->type) {
			case PIPE_CONTROL: /* stop control list */
				ohci->hc_control &= ~OHCI_CTRL_CLE;
				writel (ohci->hc_control, &ohci->regs->control); 
  				break;
			case PIPE_BULK: /* stop bulk list */
				ohci->hc_control &= ~OHCI_CTRL_BLE;
				writel (ohci->hc_control, &ohci->regs->control); 
				break;
		}
	}

	frame = le16_to_cpu (ohci->hcca->frame_no) & 0x1;
	ed->ed_rm_list = ohci->ed_rm_list[frame];
	ohci->ed_rm_list[frame] = ed;

	if (!ohci->disabled && !ohci->sleeping) {
		/* enable SOF interrupt */
		writel (OHCI_INTR_SF, &ohci->regs->intrstatus);
		writel (OHCI_INTR_SF, &ohci->regs->intrenable);
	}
}

/*-------------------------------------------------------------------------*
 * TD handling functions
 *-------------------------------------------------------------------------*/

/* enqueue next TD for this URB (OHCI spec 5.2.8.2) */

static void
//original Source	//635u2	
//td_fill (ohci_t * ohci, unsigned int info,
//	dma_addr_t data, int len,
//	struct urb * urb, int index)
td_fill (ohci_t * ohci, unsigned int info,
	char * data, int len,
	struct urb * urb, int index)
{
	volatile td_t  * td, * td_pt;
	urb_priv_t * urb_priv = urb->hcpriv;
	
	if (index >= urb_priv->length) {
#ifdef OHCI_DEBUG	//635u2		
		err("internal OHCI error: TD index > length");
#endif		
		return;
	}
	
	/* use this td as the next dummy */
	td_pt = urb_priv->td [index];
	td_pt->hwNextTD = 0;

	/* fill the old dummy TD */
//original Source	//635u2	
//	td = urb_priv->td [index] = dma_to_td (ohci,
//			le32_to_cpup (&urb_priv->ed->hwTailP) & ~0xf);
	td = urb_priv->td [index] = (td_t  * ) bus_to_virt(le32_to_cpup(&urb_priv->ed->hwTailP) & ~0xf);	//635u2

	td->ed = urb_priv->ed;
	td->next_dl_td = NULL;
	td->index = index;
	td->urb = urb; 
//original Source	//635u2	
//	td->data_dma = data;
	td->data_dma = ((!data || !len)		//635u2
			? 0
			: (uint32)data);
			
//original Source	//635u2	
//	if (!len)
//		data = 0;

//original Source	//635u2	
//	td->hwINFO = cpu_to_le32 (info);
	td->hwINFO = info;	//635u2
	if ((td->ed->type) == PIPE_ISOCHRONOUS) {
//original Source	//635u2	
//		td->hwCBP = cpu_to_le32 (data & 0xFFFFF000);
		td->hwCBP = (( (!data||!len) ? 0 : virt_to_bus(data)) & 0xFFFFF000);	//635u2
		td->ed->last_iso = info & 0xffff;
	} else {
//original Source	//635u2	
//		td->hwCBP = cpu_to_le32 (data); 
		td->hwCBP = ((!data || !len)
			? 0
			: (uint32)data);//635u2
	}			
//original Source	//635u2	
//	if (data)
//		td->hwBE = cpu_to_le32 (data + len - 1);
//	else
//		td->hwBE = 0;
	td->hwBE = ((!data || !len)
			? 0
			:(uint32) ( data + len - 1));//635u2
//original Source	//635u2		
//	td->hwNextTD = cpu_to_le32 (td_pt->td_dma);
	td->hwNextTD = (uint32)td_pt;
//original Source	//635u2		
//	td->hwPSW [0] = cpu_to_le16 ((data & 0x0FFF) | 0xE000);
	td->hwPSW [0] = cpu_to_le16 (( (uint32) data & 0x0FFF) | 0xE000);

	/* append to queue */
	wmb();
	td->ed->hwTailP = td->hwNextTD;
}

/*-------------------------------------------------------------------------*/
 
/* prepare all TDs of a transfer */

static void td_submit_urb (struct urb * urb)
{ 
	urb_priv_t * urb_priv = urb->hcpriv;
	ohci_t * ohci = (ohci_t *) urb->dev->bus->hcpriv;
//original Source	//635u2	
//	dma_addr_t data;
	char * data;
	int data_len = urb->transfer_buffer_length;
	int maxps = usb_maxpacket (urb->dev, urb->pipe, usb_pipeout (urb->pipe));
	int cnt = 0; 
	__u32 info = 0;
  	unsigned int toggle = 0;

	/* OHCI handles the DATA-toggles itself, we just use the USB-toggle bits for reseting */
  	if(usb_gettoggle(urb->dev, usb_pipeendpoint(urb->pipe), usb_pipeout(urb->pipe))) {
  		toggle = TD_T_TOGGLE;
	} else {
  		toggle = TD_T_DATA0;
		usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe), usb_pipeout(urb->pipe), 1);
	}
	
	urb_priv->td_cnt = 0;

	if (data_len) {
//original Source	//635u2	
//		data = pci_map_single (ohci->ohci_dev,
//			urb->transfer_buffer, data_len,
//			usb_pipeout (urb->pipe)
//				? PCI_DMA_TODEVICE
//				: PCI_DMA_FROMDEVICE
//			);
//719AW		data = pci_data_map(urb->transfer_buffer, data_len,
//719AW			(usb_pipein(urb->pipe) ? PCI_DMA_FROMDEVICE : PCI_DMA_TODEVICE), 0);
		data = urb->transfer_buffer; //719AW
	} else
		data = 0;
	
	switch (usb_pipetype (urb->pipe)) {
		case PIPE_BULK:
			info = usb_pipeout (urb->pipe)? 
				TD_CC | TD_DP_OUT : TD_CC | TD_DP_IN ;
			while(data_len > 4096) {		
				td_fill (ohci, info | (cnt? TD_T_TOGGLE:toggle), data, 4096, urb, cnt);
				data += 4096; data_len -= 4096; cnt++;
			}
			info = usb_pipeout (urb->pipe)?
				TD_CC | TD_DP_OUT : TD_CC | TD_R | TD_DP_IN ;
			td_fill (ohci, info | (cnt? TD_T_TOGGLE:toggle), data, data_len, urb, cnt);
			cnt++;

			/* If the transfer size is multiple of the pipe mtu,
			 * we may need an extra TD to create a empty frame
			 * Note : another way to check this condition is
			 * to test if(urb_priv->length > cnt) - Jean II */
			if ((urb->transfer_flags & USB_ZERO_PACKET) &&
			    usb_pipeout (urb->pipe) &&
			    (urb->transfer_buffer_length != 0) && 
			    ((urb->transfer_buffer_length % maxps) == 0)) {
				td_fill (ohci, info | (cnt? TD_T_TOGGLE:toggle), 0, 0, urb, cnt);
				cnt++;
			}

			if (!ohci->sleeping) {
				wmb();
				writel (OHCI_BLF, &ohci->regs->cmdstatus); /* start bulk list */
			}
			break;

		case PIPE_INTERRUPT:
			info = usb_pipeout (urb->pipe)? 
				TD_CC | TD_DP_OUT | toggle: TD_CC | TD_R | TD_DP_IN | toggle;
			td_fill (ohci, info, data, data_len, urb, cnt++);
			break;

		case PIPE_CONTROL:
			info = TD_CC | TD_DP_SETUP | TD_T_DATA0;
			td_fill (ohci, info,
//original Source	//635u2	
//				pci_map_single (ohci->ohci_dev,
//					urb->setup_packet, 8,
//					PCI_DMA_TODEVICE),
//719AW				pci_data_map(urb->setup_packet, 8,
//719AW									PCI_DMA_TODEVICE, 1),
				urb->setup_packet,	//719AW	
				8, urb, cnt++); 
			if (data_len > 0) {  
				info = usb_pipeout (urb->pipe)? 
					TD_CC | TD_R | TD_DP_OUT | TD_T_DATA1 : TD_CC | TD_R | TD_DP_IN | TD_T_DATA1;
				/* NOTE:  mishandles transfers >8K, some >4K */
				td_fill (ohci, info, data, data_len, urb, cnt++);  
				
			} 
			info = usb_pipeout (urb->pipe)? 
 				TD_CC | TD_DP_IN | TD_T_DATA1: TD_CC | TD_DP_OUT | TD_T_DATA1;
//original Source	//635u2	
//			td_fill (ohci, info, data, 0, urb, cnt++);
			td_fill (ohci, info, NULL , 0, urb, cnt++);
			if (!ohci->sleeping) {
				wmb();
				writel (OHCI_CLF, &ohci->regs->cmdstatus); /* start Control list */
			}
			break;

		case PIPE_ISOCHRONOUS:
			for (cnt = 0; cnt < urb->number_of_packets; cnt++) {
				td_fill (ohci, TD_CC|TD_ISO | ((urb->start_frame + cnt) & 0xffff), 
					( u8 * )data + urb->iso_frame_desc[cnt].offset, 
					urb->iso_frame_desc[cnt].length, urb, cnt); 
			}
			break;
	} 
	if (urb_priv->length != cnt)
	{
#ifdef OHCI_DEBUG	//635u2		 
		dbg("TD LENGTH %d != CNT %d", urb_priv->length, cnt);
#endif	
	}
	
}

/*-------------------------------------------------------------------------*
 * Done List handling functions
 *-------------------------------------------------------------------------*/


/* calculate the transfer length and update the urb */

static void dl_transfer_length(td_t * td)
{
	__u32 tdINFO, tdBE, tdCBP;
 	__u16 tdPSW;
 	struct urb * urb = td->urb;
 	urb_priv_t * urb_priv = urb->hcpriv;
	int dlen = 0;
	int cc = 0;
	
	tdINFO = le32_to_cpup (&td->hwINFO);
  	tdBE   = le32_to_cpup (&td->hwBE);
  	tdCBP  = le32_to_cpup (&td->hwCBP);


  	if (tdINFO & TD_ISO) {
 		tdPSW = le16_to_cpu (td->hwPSW[0]);
 		cc = (tdPSW >> 12) & 0xF;
		if (cc < 0xE)  {
			if (usb_pipeout(urb->pipe)) {
				dlen = urb->iso_frame_desc[td->index].length;
			} else {
				dlen = tdPSW & 0x3ff;
			}
			urb->actual_length += dlen;
			urb->iso_frame_desc[td->index].actual_length = dlen;
			if (!(urb->transfer_flags & USB_DISABLE_SPD) && (cc == TD_DATAUNDERRUN))
				cc = TD_CC_NOERROR;
					 
			urb->iso_frame_desc[td->index].status = cc_to_error[cc];
		}
	} else { /* BULK, INT, CONTROL DATA */
		if (!(usb_pipetype (urb->pipe) == PIPE_CONTROL && 
				((td->index == 0) || (td->index == urb_priv->length - 1)))) {
 			if (tdBE != 0) {
 				if (td->hwCBP == 0)
					urb->actual_length += tdBE - td->data_dma + 1;
  				else
					urb->actual_length += tdCBP - td->data_dma;
			}
  		}
  	}
}

/* handle an urb that is being unlinked */

static void dl_del_urb (struct urb * urb)
{
	ohci_wait_queue_head_t * wait_head = ((urb_priv_t *)(urb->hcpriv))->wait;

	urb_rm_priv_locked (urb);

	if (urb->transfer_flags & USB_ASYNC_UNLINK) {
		urb->status = -ECONNRESET;
		if (urb->complete)
			urb->complete (urb);
	} else {
		urb->status = -ENOENT;
		if (urb->complete)
			urb->complete (urb);

		/* unblock sohci_unlink_urb */
		if (wait_head)
			wake_up (wait_head);
	}
}

/*-------------------------------------------------------------------------*/

/* replies to the request have to be on a FIFO basis so
 * we reverse the reversed done-list */
 
static td_t * dl_reverse_done_list (ohci_t * ohci)
{
	__u32 td_list_hc;
	td_t * td_rev = NULL;
	td_t * td_list = NULL;
  	urb_priv_t * urb_priv = NULL;
  	unsigned long flags;
  	
  	spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
  	
	td_list_hc = le32_to_cpup (&ohci->hcca->done_head) & 0xfffffff0;
	ohci->hcca->done_head = 0;
	
	while (td_list_hc) {	
//original Source	//635u2	
//		td_list = dma_to_td (ohci, td_list_hc);
		td_list = (td_t * )bus_to_virt ( td_list_hc);	//635u2
		if (TD_CC_GET (le32_to_cpup (&td_list->hwINFO))) {
			urb_priv = (urb_priv_t *) td_list->urb->hcpriv;
#ifdef OHCI_DEBUG	//635u2
			dbg(" USB-error/status: %x : %p", 
					TD_CC_GET (le32_to_cpup (&td_list->hwINFO)), td_list);
#endif				
//original Source	//635u2
//			if (td_list->ed->hwHeadP & cpu_to_le32 (0x1)) {
			if (td_list->ed->hwHeadP & (0x1)) {	//635u2
				if (urb_priv && ((td_list->index + 1) < urb_priv->length)) {
//original Source	//635u2
//					td_list->ed->hwHeadP = 
//						(urb_priv->td[urb_priv->length - 1]->hwNextTD & cpu_to_le32 (0xfffffff0)) |
//									(td_list->ed->hwHeadP & cpu_to_le32 (0x2));
					td_list->ed->hwHeadP = 
						(urb_priv->td[urb_priv->length - 1]->hwNextTD & (0xfffffff0)) |
									(td_list->ed->hwHeadP & (0x2));	//635u2
					urb_priv->td_cnt += urb_priv->length - td_list->index - 1;
				} else 
//original Source	//635u2				
//					td_list->ed->hwHeadP &= cpu_to_le32 (0xfffffff2);
					td_list->ed->hwHeadP &= (0xfffffff2);	//635u2
			}
		}

		td_list->next_dl_td = td_rev;	
		td_rev = td_list;
		td_list_hc = le32_to_cpup (&td_list->hwNextTD) & 0xfffffff0;	
	}	
	spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
	return td_list;
}

/*-------------------------------------------------------------------------*/

/* there are some pending requests to remove 
 * - some of the eds (if ed->state & ED_DEL (set by sohci_free_dev)
 * - some URBs/TDs if urb_priv->state == URB_DEL */
 
static void dl_del_list (ohci_t  * ohci, unsigned int frame)
{
	unsigned long flags;
	ed_t * ed;
	__u32 edINFO;
	__u32 tdINFO;
	td_t * td = NULL, * td_next = NULL, * tdHeadP = NULL, * tdTailP;
	__u32 * td_p;
	int ctrl = 0, bulk = 0;

	spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);

	for (ed = ohci->ed_rm_list[frame]; ed != NULL; ed = ed->ed_rm_list) {
//original Source	//635u2
//		tdTailP = dma_to_td (ohci, le32_to_cpup (&ed->hwTailP) & 0xfffffff0);
//		tdHeadP = dma_to_td (ohci, le32_to_cpup (&ed->hwHeadP) & 0xfffffff0);
		tdTailP = le32_to_cpup (&ed->hwTailP) & 0xfffffff0;	//635u2
		tdHeadP = le32_to_cpup (&ed->hwHeadP) & 0xfffffff0;	//635u2
		edINFO = le32_to_cpup (&ed->hwINFO);
		td_p = &ed->hwHeadP;

		for (td = tdHeadP; td != tdTailP; td = td_next) { 
			struct urb * urb = td->urb;
			urb_priv_t * urb_priv = td->urb->hcpriv;
//original Source	//635u2			
//			td_next = dma_to_td (ohci, le32_to_cpup (&td->hwNextTD) & 0xfffffff0);
			td_next = le32_to_cpup (&td->hwNextTD) & 0xfffffff0;	//635u2
			if ((urb_priv->state == URB_DEL) || (ed->state & ED_DEL)) {
				tdINFO = le32_to_cpup (&td->hwINFO);
				if (TD_CC_GET (tdINFO) < 0xE)
					dl_transfer_length (td);
//original Source	//635u2					
//				*td_p = td->hwNextTD | (*td_p & cpu_to_le32 (0x3));
				*td_p = td->hwNextTD | (*td_p & (0x3));	//635u2
				/* URB is done; clean up */
				if (++(urb_priv->td_cnt) == urb_priv->length)
					dl_del_urb (urb);
			} else {
				td_p = &td->hwNextTD;
			}
		}

		if (ed->state & ED_DEL) { /* set by sohci_free_dev */
			struct ohci_device * dev = usb_to_ohci (ohci->dev[edINFO & 0x7F]);
			td_free (ohci, tdTailP); /* free dummy td */
//original Source	//635u2
//   	 		ed->hwINFO = cpu_to_le32 (OHCI_ED_SKIP); 
			ed->hwINFO =  (OHCI_ED_SKIP);	//635u2
			ed->state = ED_NEW;
//original Source	//635u2
//No hash
//			hash_free_ed(ohci, ed);
   	 		/* if all eds are removed wake up sohci_free_dev */
   	 		if (!--dev->ed_cnt) {
				ohci_wait_queue_head_t *wait_head = dev->wait;

				dev->wait = 0;
				if (wait_head)
					wake_up (wait_head);
			}
   	 	} else {
   	 		ed->state &= ~ED_URB_DEL;
//original Source	//635u2   	 		
//			tdHeadP = dma_to_td (ohci, le32_to_cpup (&ed->hwHeadP) & 0xfffffff0);
			tdHeadP = le32_to_cpup (&ed->hwHeadP) & 0xfffffff0;	//635u2
			if (tdHeadP == tdTailP) {
				if (ed->state == ED_OPER)
					ep_unlink(ohci, ed);
			} else
//original Source	//635u2
//   	 			ed->hwINFO &= ~cpu_to_le32 (OHCI_ED_SKIP);
   	 				ed->hwINFO &= ~ (OHCI_ED_SKIP);	//635u2
   	 	}

		switch (ed->type) {
			case PIPE_CONTROL:
				ctrl = 1;
				break;
			case PIPE_BULK:
				bulk = 1;
				break;
		}
   	}
   	
	/* maybe reenable control and bulk lists */ 
	if (!ohci->disabled) {
		if (ctrl) 	/* reset control list */
			writel (0, &ohci->regs->ed_controlcurrent);
		if (bulk)	/* reset bulk list */
			writel (0, &ohci->regs->ed_bulkcurrent);
		if (!ohci->ed_rm_list[!frame] && !ohci->sleeping) {
			if (ohci->ed_controltail)
				ohci->hc_control |= OHCI_CTRL_CLE;
			if (ohci->ed_bulktail)
				ohci->hc_control |= OHCI_CTRL_BLE;
			writel (ohci->hc_control, &ohci->regs->control);   
		}
	}

   	ohci->ed_rm_list[frame] = NULL;
   	spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
}


  		
/*-------------------------------------------------------------------------*/

/* td done list */

static void dl_done_list (ohci_t * ohci, td_t * td_list)
{
  	td_t * td_list_next = NULL;
	ed_t * ed;
	int cc = 0;
	struct urb * urb;
	urb_priv_t * urb_priv;
 	__u32 tdINFO, edHeadP, edTailP;
 	
 	unsigned long flags;
 	
  	while (td_list) {
   		td_list_next = td_list->next_dl_td;
   		
  		urb = td_list->urb;
  		urb_priv = urb->hcpriv;
  		tdINFO = le32_to_cpup (&td_list->hwINFO);
  		
   		ed = td_list->ed;
   		
   		dl_transfer_length(td_list);
 			
  		/* error code of transfer */
  		cc = TD_CC_GET (tdINFO);
  		if (cc == TD_CC_STALL)
			usb_endpoint_halt(urb->dev,
				usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe));
  		
  		if (!(urb->transfer_flags & USB_DISABLE_SPD)
				&& (cc == TD_DATAUNDERRUN))
			cc = TD_CC_NOERROR;

  		if (++(urb_priv->td_cnt) == urb_priv->length) {
			if ((ed->state & (ED_OPER | ED_UNLINK))
					&& (urb_priv->state != URB_DEL)) {
  				urb->status = cc_to_error[cc];
  				sohci_return_urb (ohci, urb);
  			} else {
				spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
  				dl_del_urb (urb);
				spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
			}
  		}
  		
  		spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
  		if (ed->state != ED_NEW) { 
  			edHeadP = le32_to_cpup (&ed->hwHeadP) & 0xfffffff0;
  			edTailP = le32_to_cpup (&ed->hwTailP);

			/* unlink eds if they are not busy */
     			if ((edHeadP == edTailP) && (ed->state == ED_OPER)) 
     				ep_unlink (ohci, ed);
     		}	
     		spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
     	
    		td_list = td_list_next;
  	}  
}




/*-------------------------------------------------------------------------*
 * Virtual Root Hub 
 *-------------------------------------------------------------------------*/
 
/* Device descriptor */
static __u8 root_hub_dev_des[] =
{
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x10,	    /*  __u16 bcdUSB; v1.1 */
	0x01,
	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x00,       /*  __u8  bDeviceProtocol; */
	0x08,       /*  __u8  bMaxPacketSize0; 8 Bytes */
	0x00,       /*  __u16 idVendor; */
	0x00,
	0x00,       /*  __u16 idProduct; */
 	0x00,
	0x00,       /*  __u16 bcdDevice; */
 	0x00,
	0x00,       /*  __u8  iManufacturer; */
	0x02,       /*  __u8  iProduct; */
	0x01,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};


/* Configuration descriptor */
static __u8 root_hub_config_des[] =
{
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19,       /*  __u16 wTotalLength; */
	0x00,
	0x01,       /*  __u8  bNumInterfaces; */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0x40,       /*  __u8  bmAttributes; 
                 Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup, 4..0: resvd */
	0x00,       /*  __u8  MaxPower; */
      
	/* interface */	  
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; */
	0x00,       /*  __u8  if_iInterface; */
     
	/* endpoint */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
 	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
 	0x02,       /*  __u16 ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
 	0x00,
	0xff        /*  __u8  ep_bInterval; 255 ms */
};

/* Hub class-specific descriptor is constructed dynamically */


/*-------------------------------------------------------------------------*/

/* prepare Interrupt pipe data; HUB INTERRUPT ENDPOINT */ 
 
static int rh_send_irq (ohci_t * ohci, void * rh_data, int rh_len)
{
	int num_ports;
	int i;
	int ret=0;//635u2
	int len;

	__u8 data[8]={0,0,0,0,0,0,0,0};//635u2

	num_ports = roothub_a (ohci) & RH_A_NDP; 
	if (num_ports > MAX_ROOT_PORTS) {
#ifdef OHCI_DEBUG	//635u2		
		err ("bogus NDP=%d for OHCI usb-%s", num_ports,
			ohci->ohci_dev->slot_name);
		err ("rereads as NDP=%d",
			readl (&ohci->regs->roothub.a) & RH_A_NDP);
#endif
		/* retry later; "should not happen" */
		return 0;
	}
//original Source	//635u2
//Don't care root hub status
/*	*(__u8 *) data = (roothub_status (ohci) & (RH_HS_LPSC | RH_HS_OCIC))
		? 1: 0;
	ret = *(__u8 *) data;
*/
	for ( i = 0; i < num_ports; i++) {
		*(__u8 *) (data + (i + 1) / 8) |= 
			((roothub_portstatus (ohci, i) &
				(RH_PS_CSC | RH_PS_PESC | RH_PS_PSSC | RH_PS_OCIC | RH_PS_PRSC))
			    ? 1: 0) << ((i + 1) % 8);
		ret += *(__u8 *) (data + (i + 1) / 8);
	}
	len = i/8 + 1;
  
	if (ret > 0) { 
		memcpy(rh_data, data,
		       min_t(unsigned int, len,
			   min_t(unsigned int, rh_len, sizeof(data))));
		return len;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* Virtual Root Hub INTs are polled by this timer every "interval" ms */
//os static void rh_int_timer_do (unsigned long ptr)
void rh_int_timer_do (cyg_handle_t handle, cyg_addrword_t ptr)
{
	int len; 

	struct urb * urb = (struct urb *) ptr;
	ohci_t * ohci = urb->dev->bus->hcpriv;
	//________________###test###_____________________//
//ZOT716u2	uint32 test_val32 =  0;
	//_______________________________________________//
	
	cyg_alarm_delete(ORH_Alarm);				//eCos
	cyg_clock_delete(ORH_SysClk);			//eCos
	cyg_counter_delete(ORH_Counter);			//eCos
	
	if (ohci->disabled)
		return;

	/* ignore timers firing during PM suspend, etc */
	if ((ohci->hc_control & OHCI_CTRL_HCFS) != OHCI_USB_OPER)
		goto out;

	//________________###test###_____________________//
	//Polling interrupt
//Ron 8/16/2004	hc_interrupt(NULL);
	//_______________________________________________//	

	//________________###test###_____________________//
	//Read Frame Number
//	test_val32 = readl (&ohci->regs->fmnumber);
	//_______________________________________________//	

	if(ohci->rh.send) { 
		len = rh_send_irq (ohci, urb->transfer_buffer, urb->transfer_buffer_length);
		if (len > 0) {
			urb->actual_length = len;
#ifdef DEBUG
			urb_print (urb, "RET-t(rh)", usb_pipeout (urb->pipe));
#endif
			if (urb->complete)
				urb->complete (urb);
		}
	}
 out:
	rh_init_int_timer (urb);
}

/*-------------------------------------------------------------------------*/

/* Root Hub INTs are polled by this timer */

static int rh_init_int_timer (struct urb * urb) 
{
	ohci_t * ohci = urb->dev->bus->hcpriv;
//ZOT716u2	int interval = 10;

	ohci->rh.interval = urb->interval;
//original Source	//635u2
/*	init_timer (&ohci->rh.rh_int_timer);
	ohci->rh.rh_int_timer.function = rh_int_timer_do;
	ohci->rh.rh_int_timer.data = (unsigned long) urb;
	ohci->rh.rh_int_timer.expires = 
			jiffies + (HZ * (urb->interval < 30? 30: urb->interval)) / 1000;
	add_timer (&ohci->rh.rh_int_timer);
*/

//635u2
//os	stop_timer( &ohci->rh.rh_int_timer );	
//os	ohci->rh.rh_int_timer.func = rh_int_timer_do;
//os	ohci->rh.rh_int_timer.arg = (void *)urb;
//os	set_timer( &ohci->rh.rh_int_timer, 20 );
//os	start_timer( &ohci->rh.rh_int_timer );

    /* Attach the timer to the real-time clock */
    ORH_SysClk = cyg_real_time_clock();

	GLOBAL_OHCI_RH_URB = urb;

    cyg_clock_to_counter(ORH_SysClk, &ORH_Counter);

    cyg_alarm_create(ORH_Counter, (cyg_alarm_t *)rh_int_timer_do,
                     (cyg_addrword_t) urb,
                     &ORH_Alarm, &ORH_timerAlarm);

    /* This creates a periodic timer */
//ZOT716u2    cyg_alarm_initialize(ORH_Alarm, cyg_current_time() + interval, 0); //every setting trigger once
    cyg_alarm_initialize(ORH_Alarm, cyg_current_time() + (urb->interval/MSPTICK), 0); //every setting trigger once //ZOT716u2
		
	return 0;
}

/*-------------------------------------------------------------------------*/
#undef  OK	//635u2
#define OK(x) 			len = (x); break
#define WR_RH_STAT(x) 		writel((x), &ohci->regs->roothub.status)
#define WR_RH_PORTSTAT(x) 	writel((x), &ohci->regs->roothub.portstatus[wIndex-1])
#define RD_RH_STAT		roothub_status(ohci)
//original Source	//635u2
//#define RD_RH_PORTSTAT		roothub_portstatus(ohci,wIndex-1)
#define RD_RH_PORTSTAT(wIndex)		roothub_portstatus(ohci,wIndex-1)

/* request to virtual root hub */

static int rh_submit_urb (struct urb * urb)
{
	struct usb_device * usb_dev = urb->dev;
	ohci_t * ohci = usb_dev->bus->hcpriv;
	unsigned int pipe = urb->pipe;
//original Source	//635u2
//	struct usb_ctrlrequest * cmd = (struct usb_ctrlrequest *) urb->setup_packet;
	devrequest * cmd = (devrequest *) urb->setup_packet;
	void * data = urb->transfer_buffer;
	int leni = urb->transfer_buffer_length;
	int len = 0;
	int status = TD_CC_NOERROR;
	
	__u32 datab[4];
	__u8  * data_buf = (__u8 *) datab;
	
 	__u16 bmRType_bReq;
	__u16 wValue; 
	__u16 wIndex;
	__u16 wLength;

	if (usb_pipeint(pipe)) {
		ohci->rh.urb =  urb;
		ohci->rh.send = 1;
		ohci->rh.interval = urb->interval;
		rh_init_int_timer(urb);
		urb->status = cc_to_error [TD_CC_NOERROR];
		
		return 0;
	}
	//original Source	//635u2	
	//bmRType_bReq  = cmd->bRequestType | (cmd->bRequest << 8);
	//wValue        = le16_to_cpu (cmd->wValue);
	//wIndex        = le16_to_cpu (cmd->wIndex);
	//wLength       = le16_to_cpu (cmd->wLength);
	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);
	wValue        = le16_to_cpu (cmd->value);
	wIndex        = le16_to_cpu (cmd->index);
	wLength       = le16_to_cpu (cmd->length);
	
	
	switch (bmRType_bReq) {
	/* Request Destination:
	   without flags: Device, 
	   RH_INTERFACE: interface, 
	   RH_ENDPOINT: endpoint,
	   RH_CLASS means HUB here, 
	   RH_OTHER | RH_CLASS  almost ever means HUB_PORT here 
	*/
  
		case RH_GET_STATUS: 				 		
//original Source	//635u2
//				*(__u16 *) data_buf = cpu_to_le16 (1); OK (2);
				NSET16( data_buf, cpu_to_le16 (1) );
				OK (2);
		case RH_GET_STATUS | RH_INTERFACE: 	 		
//original Source	//635u2
//				*(__u16 *) data_buf = cpu_to_le16 (0); OK (2);
				NSET16( data_buf, cpu_to_le16 (0) );
				OK (2);
		case RH_GET_STATUS | RH_ENDPOINT:	 		
//original Source	//635u2
//				*(__u16 *) data_buf = cpu_to_le16 (0); OK (2);  
				NSET16( data_buf, cpu_to_le16 (0) );
				OK (2); 
		case RH_GET_STATUS | RH_CLASS: 				
//original Source	//635u2
//				*(__u32 *) data_buf = cpu_to_le32 (
//					RD_RH_STAT & ~(RH_HS_CRWE | RH_HS_DRWE));
				NSET32( data_buf,(RD_RH_STAT & ~(RH_HS_CRWE | RH_HS_DRWE)) );
				OK (4);
		case RH_GET_STATUS | RH_OTHER | RH_CLASS: 	
//original Source	//635u2
//				*(__u32 *) data_buf = cpu_to_le32 (RD_RH_PORTSTAT); OK (4);
				NSET32( data_buf,(RD_RH_PORTSTAT(wIndex)) );
				OK (4);
		case RH_CLEAR_FEATURE | RH_ENDPOINT:  
			switch (wValue) {
				case (RH_ENDPOINT_STALL): OK (0);
			}
			break;

		case RH_CLEAR_FEATURE | RH_CLASS:
			switch (wValue) {
				case RH_C_HUB_LOCAL_POWER:
					OK(0);
				case (RH_C_HUB_OVER_CURRENT): 
						WR_RH_STAT(RH_HS_OCIC); OK (0);
			}
			break;
		
		case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
			switch (wValue) {
				case (RH_PORT_ENABLE): 			
						WR_RH_PORTSTAT (RH_PS_CCS ); OK (0);
				case (RH_PORT_SUSPEND):			
						WR_RH_PORTSTAT (RH_PS_POCI); OK (0);
				case (RH_PORT_POWER):			
						WR_RH_PORTSTAT (RH_PS_LSDA); OK (0);
				case (RH_C_PORT_CONNECTION):	
						WR_RH_PORTSTAT (RH_PS_CSC ); OK (0);
				case (RH_C_PORT_ENABLE):		
						WR_RH_PORTSTAT (RH_PS_PESC); OK (0);
				case (RH_C_PORT_SUSPEND):		
						WR_RH_PORTSTAT (RH_PS_PSSC); OK (0);
				case (RH_C_PORT_OVER_CURRENT):	
						WR_RH_PORTSTAT (RH_PS_OCIC); OK (0);
				case (RH_C_PORT_RESET):			
						WR_RH_PORTSTAT (RH_PS_PRSC); OK (0); 
			}
			break;
 
		case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
			switch (wValue) {
				case (RH_PORT_SUSPEND):			
						WR_RH_PORTSTAT (RH_PS_PSS ); OK (0); 
				case (RH_PORT_RESET): /* BUG IN HUP CODE *********/
						if (RD_RH_PORTSTAT(wIndex) & RH_PS_CCS)
						    WR_RH_PORTSTAT (RH_PS_PRS);
						OK (0);
				case (RH_PORT_POWER):			
						WR_RH_PORTSTAT (RH_PS_PPS ); OK (0); 
				case (RH_PORT_ENABLE): /* BUG IN HUP CODE *********/
						if (RD_RH_PORTSTAT(wIndex) & RH_PS_CCS)
						    WR_RH_PORTSTAT (RH_PS_PES );
						OK (0);
			}
			break;

		case RH_SET_ADDRESS: ohci->rh.devnum = wValue; OK(0);

		case RH_GET_DESCRIPTOR:
			switch ((wValue & 0xff00) >> 8) {
				case (0x01): /* device descriptor */
					len = min_t(unsigned int,
						  leni,
						  min_t(unsigned int,
						      sizeof (root_hub_dev_des),
						      wLength));
					data_buf = root_hub_dev_des; OK(len);
				case (0x02): /* configuration descriptor */
					len = min_t(unsigned int,
						  leni,
						  min_t(unsigned int,
						      sizeof (root_hub_config_des),
						      wLength));
					data_buf = root_hub_config_des; OK(len);
				case (0x03): /* string descriptors */
					len = usb_root_hub_string (wValue & 0xff,
						(int)(long) ohci->regs, "OHCI",
						data, wLength);
					if (len > 0) {
						data_buf = data;
						OK(min_t(int, leni, len));
					}
					// else fallthrough
				default: 
					status = TD_CC_STALL;
			}
			break;
		
		case RH_GET_DESCRIPTOR | RH_CLASS:
		    {
			    __u32 temp = roothub_a (ohci);

			    data_buf [0] = 9;		// min length;
			    data_buf [1] = 0x29;
			    data_buf [2] = temp & RH_A_NDP;
			    data_buf [3] = 0;
			    if (temp & RH_A_PSM) 	/* per-port power switching? */
				data_buf [3] |= 0x1;
			    if (temp & RH_A_NOCP)	/* no overcurrent reporting? */
				data_buf [3] |= 0x10;
			    else if (temp & RH_A_OCPM)	/* per-port overcurrent reporting? */
				data_buf [3] |= 0x8;

			    datab [1] = 0;
			    data_buf [5] = (temp & RH_A_POTPGT) >> 24;
			    temp = roothub_b (ohci);
			    data_buf [7] = temp & RH_B_DR;
			    if (data_buf [2] < 7) {
				data_buf [8] = 0xff;
			    } else {
				data_buf [0] += 2;
				data_buf [8] = (temp & RH_B_DR) >> 8;
				data_buf [10] = data_buf [9] = 0xff;
			    }
				
			    len = min_t(unsigned int, leni,
				      min_t(unsigned int, data_buf [0], wLength));
			    OK (len);
			}
 
		case RH_GET_CONFIGURATION: 	*(__u8 *) data_buf = 0x01; OK (1);

		case RH_SET_CONFIGURATION: 	WR_RH_STAT (0x10000); OK (0);

		default: 
#ifdef OHCI_DEBUG	//635u2
			dbg ("unsupported root hub command");
#endif			
			status = TD_CC_STALL;
	}
	
#ifdef	DEBUG
	// ohci_dump_roothub (ohci, 0);
#endif

	len = min_t(int, len, leni);
	if (data != data_buf)
	    memcpy (data, data_buf, len);
  	urb->actual_length = len;
	urb->status = cc_to_error [status];
	
#ifdef DEBUG
	urb_print (urb, "RET(rh)", usb_pipeout (urb->pipe));
#endif

	urb->hcpriv = NULL;
	usb_dec_dev_use (usb_dev);
	urb->dev = NULL;
	if (urb->complete)
	    	urb->complete (urb);
	    	
//_____________________________________________________________		
//ZOT716u2	if (ohci->rh.send) {
//ZOT716u2		armond_printf("my start OHCI Detcet\n");
//ZOT716u2		rh_init_int_timer(GLOBAL_OHCI_RH_URB);
//ZOT716u2	}
//_____________________________________________________________	
    	
	return 0;
}

/*-------------------------------------------------------------------------*/

static int rh_unlink_urb (struct urb * urb)
{
	ohci_t * ohci = urb->dev->bus->hcpriv;
 
	if (ohci->rh.urb == urb) {
		ohci->rh.send = 0;
//original Source	//635u2
//Don't care time out
//		del_timer (&ohci->rh.rh_int_timer);	//635u2
		ohci->rh.urb = NULL;

		urb->hcpriv = NULL;
		usb_dec_dev_use(urb->dev);
		urb->dev = NULL;
		if (urb->transfer_flags & USB_ASYNC_UNLINK) {
			urb->status = -ECONNRESET;
			if (urb->complete)
				urb->complete (urb);
		} else
			urb->status = -ENOENT;
	}
	return 0;
}
 
/*-------------------------------------------------------------------------*
 * HC functions
 *-------------------------------------------------------------------------*/

/* reset the HC and BUS */

static int hc_reset (ohci_t * ohci)
{
	int timeout = 30 ,i;
	int smm_timeout = 50; /* 0,5 sec */

	//________________###test###_____________________//
//ZOT716u2	uint32 test_val32 =  0;
	//_______________________________________________//
	 	
#ifndef __hppa__
	/* PA-RISC doesn't have SMM, but PDC might leave IR set */
	if (readl (&ohci->regs->control) & OHCI_CTRL_IR) { /* SMM owns the HC */
		writel (OHCI_OCR, &ohci->regs->cmdstatus); /* request ownership */
#ifdef OHCI_DEBUG	//635u2
		dbg("USB HC TakeOver from SMM");
#endif		
		while (readl (&ohci->regs->control) & OHCI_CTRL_IR) {
			wait_ms (10);
			if (--smm_timeout == 0) {
#ifdef OHCI_DEBUG	//635u2				
				err("USB HC TakeOver failed!");
#endif				
				return -1;
			}
		}
	}
#endif	
		
	/* Disable HC interrupts */
	writel (OHCI_INTR_MIE, &ohci->regs->intrdisable);
	
	//________________###test###_____________________//
	//Read Interrupt disable
//ZOT716u2	test_val32 = readl (&ohci->regs->intrdisable);
	//_______________________________________________//	
	
#ifdef OHCI_DEBUG	//635u2
	dbg("USB HC reset_hc usb-%s: ctrl = 0x%x ;",
		ohci->ohci_dev->slot_name,
		readl (&ohci->regs->control));
#endif
  	/* Reset USB (needed by some controllers) */
	writel (0, &ohci->regs->control);
     
	//________________###test###_____________________//
	//Read control
//ZOT716u2	test_val32 = readl (&ohci->regs->control);
	//_______________________________________________//	     
      	
	/* HC Reset requires max 10 ms delay */
	writel (OHCI_HCR,  &ohci->regs->cmdstatus);
	
	//________________###test###_____________________//
	//Read cmdstatus
//ZOT716u2	test_val32 = readl (&ohci->regs->cmdstatus);
	//_______________________________________________//	   	
	
	while ((readl (&ohci->regs->cmdstatus) & OHCI_HCR) != 0) {
		if (--timeout == 0) {
#ifdef OHCI_DEBUG	//635u2			
			err("USB HC reset timed out!");
#endif		
			return -1;
		}
//original Source	//635u2			
		//udelay (1);
//ZOT716u2		for(i = 0 ; i < 5; i++); //635u2
		udelay(1);	//ZOT716u2
	}	 
	return 0;
}

/*-------------------------------------------------------------------------*/

/* Start an OHCI controller, set the BUS operational
 * enable interrupts 
 * connect the virtual root hub */

static int hc_start (ohci_t * ohci)
{
  	__u32 mask;
  	unsigned int fminterval;
  	struct usb_device  * usb_dev;
	struct ohci_device * dev;

	//________________###test###_____________________//
//ZOT716u2	uint32 test_val32 =  0;
	//_______________________________________________//
	
	ohci->disabled = 1;

	/* Tell the controller where the control and bulk lists are
	 * The lists are empty now. */
	 
	writel (0, &ohci->regs->ed_controlhead);
	
	//________________###test###_____________________//
	//Read control head
//ZOT716u2	test_val32 = readl (&ohci->regs->ed_controlhead);
	//_______________________________________________//	   	
	
	writel (0, &ohci->regs->ed_bulkhead);

	//________________###test###_____________________//
	//Read bulk head
//ZOT716u2	test_val32 = readl (&ohci->regs->ed_bulkhead);
	//_______________________________________________//	

//original Source	//635u2	
//	writel (ohci->hcca_dma, &ohci->regs->hcca); /* a reset clears this */
    writel ((uint32)ohci->hcca_dma, &ohci->regs->hcca); //615wu

	//________________###test###_____________________//
	//Read hcca
//ZOT716u2	test_val32 = readl (&ohci->regs->hcca);
	//_______________________________________________//	
    
  	fminterval = 0x2edf;
	writel ((fminterval * 9) / 10, &ohci->regs->periodicstart);
	fminterval |= ((((fminterval - 210) * 6) / 7) << 16); 
	writel (fminterval, &ohci->regs->fminterval);	
	
	//________________###test###_____________________//
	//Read frame interval
//ZOT716u2	test_val32 = readl (ohci->regs->fminterval);
	//_______________________________________________//		
	
	writel (0x628, &ohci->regs->lsthresh);

	//________________###test###_____________________//
	//Read lsthresh
//ZOT716u2	test_val32 = readl (&ohci->regs->lsthresh);
	//_______________________________________________//	

 	/* start controller operations */
 	ohci->hc_control = OHCI_CONTROL_INIT | OHCI_USB_OPER;
	ohci->disabled = 0;
 	writel (ohci->hc_control, &ohci->regs->control);

	//________________###test###_____________________//
	//Read control
//ZOT716u2	test_val32 = readl (&ohci->regs->control);
	//_______________________________________________//	
 
	/* Choose the interrupts we care about now, others later on demand */
	mask = OHCI_INTR_MIE | OHCI_INTR_UE | OHCI_INTR_WDH | OHCI_INTR_SO;
	writel (mask, &ohci->regs->intrenable);
	
	//________________###test###_____________________//
	//Read interrupt ebable
//ZOT716u2	test_val32 = readl (&ohci->regs->intrenable);
	//_______________________________________________//		
	
	writel (mask, &ohci->regs->intrstatus);

	//________________###test###_____________________//
	//Read interrupt status
//ZOT716u2	test_val32 = readl (&ohci->regs->intrstatus);
	//_______________________________________________//	

#ifdef	OHCI_USE_NPS
	if(ohci->flags & OHCI_QUIRK_SUCKYIO)
	{
		/* NSC 87560 at least requires different setup .. */
		writel ((roothub_a (ohci) | RH_A_NOCP) &
			~(RH_A_OCPM | RH_A_POTPGT | RH_A_PSM | RH_A_NPS),
			&ohci->regs->roothub.a);
	}
	else
	{
		/* required for AMD-756 and some Mac platforms */
		writel ((roothub_a (ohci) | RH_A_NPS) & ~RH_A_PSM,
			&ohci->regs->roothub.a);
	}
	writel (RH_HS_LPSC, &ohci->regs->roothub.status);
#endif	/* OHCI_USE_NPS */

	// POTPGT delay is bits 24-31, in 2 ms units.
//original Source	//635u2
//	mdelay ((roothub_a (ohci) >> 23) & 0x1fe);
//ZOT716u2  	ppause((roothub_a (ohci) >> 23) & 0x1fe);	//635u2
  	mdelay ((roothub_a (ohci) >> 23) & 0x1fe);	//ZOT716u2
  	
	/* connect the virtual root hub */
	ohci->rh.devnum = 0;
	usb_dev = usb_alloc_dev (NULL, ohci->bus);
	if (!usb_dev) {
	    ohci->disabled = 1;
	    return -ENOMEM;
	}

	dev = usb_to_ohci (usb_dev);
	ohci->bus->root_hub = usb_dev;
	usb_connect (usb_dev);
	usb_dev->speed = USB_SPEED_FULL;	//ZOT716u2	
	if (usb_new_device (usb_dev) != 0) {
		usb_free_dev (usb_dev); 
		ohci->disabled = 1;
		return -ENODEV;
	}
	
	return 0;
}

/*-------------------------------------------------------------------------*/

/* called only from interrupt handler */

static void check_timeouts (struct ohci *ohci)
{
	spin_lock (&usb_ed_lock);
	while (!list_empty (&ohci->timeout_list)) {
		struct urb	*urb;

		urb = list_entry (ohci->timeout_list.next, struct urb, urb_list);
//original Source	//635u2
//Don't care time out
/*
		if (time_after (jiffies, urb->timeout))
			break;

		list_del_init (&urb->urb_list);
*/
		if (urb->status != -EINPROGRESS)
			continue;

		urb->transfer_flags |= USB_TIMEOUT_KILLED | USB_ASYNC_UNLINK;
		spin_unlock (&usb_ed_lock);

		// outside the interrupt handler (in a timer...)
		// this reference would race interrupts
		sohci_unlink_urb (urb);

		spin_lock (&usb_ed_lock);
	}
	spin_unlock (&usb_ed_lock);
}


/*-------------------------------------------------------------------------*/

/* an interrupt happens */
//original Source	//635u2
//static void hc_interrupt (int irq, void * __ohci, struct pt_regs * r)
//ZOT716u2 void hc_interrupt (ApEvtMsg_t *msg)	//615wu
void hc_interrupt()	//ZOT716u2
{
	ohci_t * ohci = GLOBAL_OHCI;
	struct ohci_regs * regs = ohci->regs;
 	int ints; 

	/* avoid (slow) readl if only WDH happened */
	if ((ohci->hcca->done_head != 0)
			&& !(le32_to_cpup (&ohci->hcca->done_head) & 0x01)) {
		ints =  OHCI_INTR_WDH;

	/* cardbus/... hardware gone before remove() */
	} else if ((ints = readl (&regs->intrstatus)) == ~(u32)0) {
		ohci->disabled++;
#ifdef OHCI_DEBUG	//635u2		
		err ("%s device removed!", ohci->ohci_dev->slot_name);
#endif		
		return;

	/* interrupt for some other device? */
	} else if ((ints &= readl (&regs->intrenable)) == 0) {
		return;
	} 

	// dbg("Interrupt: %x frame: %x", ints, le16_to_cpu (ohci->hcca->frame_no));

	if (ints & OHCI_INTR_UE) {
		ohci->disabled++;
#ifdef OHCI_DEBUG	//635u2		
		err ("OHCI Unrecoverable Error, controller usb-%s disabled",
			ohci->ohci_dev->slot_name);
#endif			
		// e.g. due to PCI Master/Target Abort

#ifdef	DEBUG
		ohci_dump (ohci, 1);
#else
		// FIXME: be optimistic, hope that bug won't repeat often.
		// Make some non-interrupt context restart the controller.
		// Count and limit the retries though; either hardware or
		// software errors can go forever...
#endif
		hc_reset (ohci);
	}
  
	if (ints & OHCI_INTR_WDH) {
		writel (OHCI_INTR_WDH, &regs->intrdisable);	
//Jesse		
//		if( (ohci->hcca->done_head > 0x200000 )|| ( ohci->hcca->done_head < 0x8060) )
//			ohci->hcca->done_head = 0;
		dl_done_list (ohci, dl_reverse_done_list (ohci));
		writel (OHCI_INTR_WDH, &regs->intrenable); 
	}
  
	if (ints & OHCI_INTR_SO) {
#ifdef OHCI_DEBUG	//635u2	
		dbg("USB Schedule overrun");
#endif		
		writel (OHCI_INTR_SO, &regs->intrenable); 	 
	}

	// FIXME:  this assumes SOF (1/ms) interrupts don't get lost...
	if (ints & OHCI_INTR_SF) { 
		unsigned int frame = le16_to_cpu (ohci->hcca->frame_no) & 1;
		writel (OHCI_INTR_SF, &regs->intrdisable);	
		if (ohci->ed_rm_list[!frame] != NULL) {
			dl_del_list (ohci, !frame);
		}
		if (ohci->ed_rm_list[frame] != NULL)
			writel (OHCI_INTR_SF, &regs->intrenable);	
	}
//original Source	//635u2
//Don't care time out
/*
	if (!list_empty (&ohci->timeout_list)) {
		check_timeouts (ohci);
// FIXME:  enable SF as needed in a timer;
// don't make lots of 1ms interrupts
// On unloaded USB, think 4k ~= 4-5msec
		if (!list_empty (&ohci->timeout_list))
			writel (OHCI_INTR_SF, &regs->intrenable);	
	}
*/
	writel (ints, &regs->intrstatus);
	writel (OHCI_INTR_MIE, &regs->intrenable);	
}
//ZOT716u2
void ohci_dsr(void)
{
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_USB11_BIT_INDEX);
	hc_interrupt();
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_USB11_BIT_INDEX);	
}

/*-------------------------------------------------------------------------*/

/* allocate OHCI */

static ohci_t * __devinit hc_alloc_ohci (struct pci_dev *dev, void * mem_base)
{
	ohci_t * ohci;
//original Source	//635u2
/*
	ohci = (ohci_t *) kmalloc (sizeof *ohci, GFP_KERNEL);
	if (!ohci)
		return NULL;
		
	memset (ohci, 0, sizeof (ohci_t));

	ohci->hcca = pci_alloc_consistent (dev, sizeof *ohci->hcca,
			&ohci->hcca_dma);
        if (!ohci->hcca) {
                kfree (ohci);
                return NULL;
        }
        memset (ohci->hcca, 0, sizeof (struct ohci_hcca));
*/
	ohci = (ohci_t *)malloc (sizeof *ohci);	//615wu
	if (!ohci)
		return NULL;

	memset (ohci, 0, sizeof (ohci_t));

//os	ohci->hcca = hcca_kmalloc(sizeof *ohci->hcca);//HCCA non-cache
//719AW	ohci->hcca = pci_alloc_hcca(&ohci->hcca_dma);	//615wu
//Z    memset (ohci->hcca, 0, sizeof (struct ohci_hcca));

	ohci->hcca = hcca_kmalloc(sizeof(struct ohci_hcca));	//719AW
	ohci->hcca_dma = ohci->hcca;							//719AW

	ohci->disabled = 1;
	ohci->sleeping = 0;
	ohci->irq = -1;
	ohci->regs = mem_base;   

//os	ohci->ohci_dev = dev;
//os	pci_set_drvdata(dev, ohci);
	GLOBAL_OHCI = ohci;
 
	INIT_LIST_HEAD (&ohci->ohci_hcd_list);
	list_add (&ohci->ohci_hcd_list, &ohci_hcd_list);

	INIT_LIST_HEAD (&ohci->timeout_list);

	ohci->bus = usb_alloc_bus (&sohci_device_operations);
	if (!ohci->bus) {
//os		pci_set_drvdata (dev, NULL);
//original Source	//635u2	
//		pci_free_consistent (ohci->ohci_dev, sizeof *ohci->hcca,
//				ohci->hcca, ohci->hcca_dma);
		free (ohci);	//615wu
		return NULL;
	}
//original Source	//635u2	
//	ohci->bus->bus_name = dev->slot_name;

	ohci->bus->hcpriv = (void *) ohci;

	return ohci;
} 


/*-------------------------------------------------------------------------*/

/* De-allocate all resources.. */

static void hc_release_ohci (ohci_t * ohci)
{	
#ifdef OHCI_DEBUG	//635u2
	dbg ("USB HC release ohci usb-%s", ohci->ohci_dev->slot_name);
#endif
	/* disconnect all devices */    
	if (ohci->bus->root_hub)
		usb_disconnect (&ohci->bus->root_hub);

	if (!ohci->disabled)
		hc_reset (ohci);
	
	if (ohci->irq >= 0) {
//		free_irq (ohci->irq, ohci);//635u2
		ohci->irq = -1;
	}
//os	pci_set_drvdata(ohci->ohci_dev, NULL);
	if (ohci->bus) {
		if (ohci->bus->busnum != -1)
			usb_deregister_bus (ohci->bus);

		usb_free_bus (ohci->bus);
	}

	list_del (&ohci->ohci_hcd_list);
	INIT_LIST_HEAD (&ohci->ohci_hcd_list);

	ohci_mem_cleanup (ohci);
    
	/* unmap the IO address space */
//original Source	//635u2	
//	iounmap (ohci->regs);

//original Source	//635u2	
//	pci_free_consistent (ohci->ohci_dev, sizeof *ohci->hcca,
//		ohci->hcca, ohci->hcca_dma);
	free (ohci);	//615wu
}

/*-------------------------------------------------------------------------*/

/* Increment the module usage count, start the control thread and
 * return success. */

//static struct pci_driver ohci_pci_driver;
 
static int __devinit
hc_found_ohci (struct pci_dev *dev, int irq,
	void *mem_base, const struct pci_device_id *id)
{
	ohci_t * ohci;
	char buf[8], *bufp = buf;
	int ret;

#ifndef __sparc__
	sprintf(buf, "%d", irq);
#else
	bufp = __irq_itoa(irq);
#endif
#if 0
	printk(KERN_INFO __FILE__ ": USB OHCI at membase 0x%lx, IRQ %s\n",
		(unsigned long)	mem_base, bufp);
	printk(KERN_INFO __FILE__ ": usb-%s, %s\n", dev->slot_name, dev->name);
#endif    
	ohci = hc_alloc_ohci (dev, mem_base);
	if (!ohci) {
		return -ENOMEM;
	}
//os	pci_connect_interface("ohci_irq",hc_interrupt,ohci,&ohci->regs->intrstatus);	//635u2	for interrupt
	if ((ret = ohci_mem_init (ohci)) < 0) {
		hc_release_ohci (ohci);
		return ret;
	}
//os	ohci->flags = id->driver_data;
	
	/* Check for NSC87560. We have to look at the bridge (fn1) to identify
	   the USB (fn2). This quirk might apply to more or even all NSC stuff
	   I don't know.. */
	   
//os	if(dev->vendor == PCI_VENDOR_ID_NS)
//os	{
//os		struct pci_dev *fn1  = pci_find_slot(dev->bus->number, PCI_DEVFN(PCI_SLOT(dev->devfn), 1));
//os		if(fn1 && fn1->vendor == PCI_VENDOR_ID_NS && fn1->device == PCI_DEVICE_ID_NS_87560_LIO)
//os			ohci->flags |= OHCI_QUIRK_SUCKYIO;
//os		
//os	}
#if 0	//635u2	
	if (ohci->flags & OHCI_QUIRK_SUCKYIO)
		printk (KERN_INFO __FILE__ ": Using NSC SuperIO setup\n");
	if (ohci->flags & OHCI_QUIRK_AMD756)
		printk (KERN_INFO __FILE__ ": AMD756 erratum 4 workaround\n");
#endif
	if (hc_reset (ohci) < 0) {
		hc_release_ohci (ohci);
		return -ENODEV;
	}

	/* FIXME this is a second HC reset; why?? */
	writel (ohci->hc_control = OHCI_USB_RESET, &ohci->regs->control);
	wait_ms (10);

	usb_register_bus (ohci->bus);

//original Source	//635u2
//	if (request_irq (irq, hc_interrupt, SA_SHIRQ,
//			ohci_pci_driver.name, ohci) != 0) 
//		{
#ifdef OHCI_DEBUG	//635u2
//		err ("request interrupt %s failed", bufp);
#endif
//		hc_release_ohci (ohci);
//		return -EBUSY;
//	}

//ZOT716u2
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_USB11_BIT_INDEX);
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_USB11_BIT_INDEX);
	
    cyg_interrupt_create(INTC_USB11_BIT_INDEX,
                             0,                     
                             0,                     
                             &ohci_isr,
                             &ohci_dsr,
                             &USB_OHCI_interrupt_handle,
                             &USB_OHCI_interrupt);
    cyg_interrupt_attach(USB_OHCI_interrupt_handle); 

	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_USB11_BIT_INDEX);

	ohci->irq = irq;     

	if (hc_start (ohci) < 0) {
#ifdef OHCI_DEBUG	//635u2		
		err ("can't start usb-%s", dev->slot_name);
#endif
		hc_release_ohci (ohci);
		return -EBUSY;
	}

#ifdef	DEBUG
	ohci_dump (ohci, 1);
#endif
	return 0;
}

/*-------------------------------------------------------------------------*/

#ifdef	CONFIG_PM

/* controller died; cleanup debris, then restart */
/* must not be called from interrupt context */

static void hc_restart (ohci_t *ohci)
{
	int temp;
	int i;

	if (ohci->pci_latency)
		pci_write_config_byte (ohci->ohci_dev, PCI_LATENCY_TIMER, ohci->pci_latency);

	ohci->disabled = 1;
	ohci->sleeping = 0;
	if (ohci->bus->root_hub)
		usb_disconnect (&ohci->bus->root_hub);
	
	/* empty the interrupt branches */
	for (i = 0; i < NUM_INTS; i++) ohci->ohci_int_load[i] = 0;
	for (i = 0; i < NUM_INTS; i++) ohci->hcca->int_table[i] = 0;
	
	/* no EDs to remove */
	ohci->ed_rm_list [0] = NULL;
	ohci->ed_rm_list [1] = NULL;

	/* empty control and bulk lists */	 
	ohci->ed_isotail     = NULL;
	ohci->ed_controltail = NULL;
	ohci->ed_bulktail    = NULL;

	if ((temp = hc_reset (ohci)) < 0 || (temp = hc_start (ohci)) < 0) {
#ifdef OHCI_DEBUG	//635u2
		err ("can't restart usb-%s, %d", ohci->ohci_dev->slot_name, temp);
#endif
	} else{
#ifdef OHCI_DEBUG	//635u2
		dbg ("restart usb-%s completed", ohci->ohci_dev->slot_name);
#endif
	}
}

#endif	/* CONFIG_PM */

/*-------------------------------------------------------------------------*/

/* configured so that an OHCI device is always provided */
/* always called with process context; sleeping is OK */

static int __devinit
//os ohci_pci_probe (struct pci_dev *dev, const struct pci_device_id *id)
ohci_pci_probe (unsigned int sz, unsigned int mem_addr)
{
	unsigned long mem_resource, mem_len;
	void *mem_base;
	int status;
	int irq = 1; //GPIO do			//eCos
	uint32 val;

//os	if (pci_enable_device(dev) < 0)
//os		return -ENODEV;

//os        if (!dev->irq) {
#ifdef OHCI_DEBUG	//635u2
        	err("found OHCI device with no IRQ assigned. check BIOS settings!");
#endif
//os		pci_disable_device (dev);
//os   	        return -ENODEV;
//os        }
	
	/* we read its hardware registers as memory */
//os	mem_resource = pci_resource_start(dev, 0);
//os	mem_len = pci_resource_len(dev, 0);
		
//os	if (!request_mem_region (mem_resource, mem_len, ohci_pci_driver.name)) {
#ifdef OHCI_DEBUG	//635u2		
		dbg ("controller already in use");
#endif
//os		pci_disable_device (dev);
//os		return -EBUSY;
//os	}

//original Source	//635u2	
//	mem_base = ioremap_nocache (mem_resource, mem_len);
//os	mem_base = ioremap (mem_resource, mem_len);	//map to PCI memory space
//os	if (!mem_base) {
#ifdef OHCI_DEBUG	//635u2		
		err("Error mapping OHCI memory");
#endif
//os		release_mem_region (mem_resource, mem_len);
//os		pci_disable_device (dev);
//os		return -EFAULT;
//os	}

	/* controller writes into our memory */
//os	pci_set_master (dev);

	//set master and Memory space to enable OHCI	//eCos
//ZOT716u2	val = pciHost_ConfigRead(0x4);		
//ZOT716u2	pciHost_ConfigWrite(0x04, val|0x6, 0);

	mem_base= mem_addr;	//eCos
	mem_len = sz;		//eCos
	__spin_lock_init(&usb_ed_lock);	//ZOT==>
		
	status = hc_found_ohci (NULL, irq, mem_base, NULL);
	if (status < 0) {
//original Source	//635u2	
//		iounmap (mem_base);
//os		release_mem_region (mem_resource, mem_len);
//os		pci_disable_device (dev);
	}
	return status;
} 

/*-------------------------------------------------------------------------*/

/* may be called from interrupt context [interface spec] */
/* may be called without controller present */
/* may be called with controller, bus, and devices active */

static void __devexit
ohci_pci_remove (struct pci_dev *dev)
{
//os	ohci_t		*ohci = pci_get_drvdata(dev);
	ohci_t		*ohci = GLOBAL_OHCI;
#ifdef OHCI_DEBUG	//635u2
	dbg ("remove %s controller usb-%s%s%s",
		hcfs2string (ohci->hc_control & OHCI_CTRL_HCFS),
		dev->slot_name,
		ohci->disabled ? " (disabled)" : "",
		in_interrupt () ? " in interrupt" : ""
		);
#endif
#ifdef	DEBUG
	ohci_dump (ohci, 1);
#endif

	/* don't wake up sleeping controllers, or block in interrupt context */
	if ((ohci->hc_control & OHCI_CTRL_HCFS) != OHCI_USB_OPER || in_interrupt ()) {
#ifdef OHCI_DEBUG	//635u2		
		dbg ("controller being disabled");
#endif
		ohci->disabled = 1;
	}

	/* on return, USB will always be reset (if present) */
	if (ohci->disabled)
		writel (ohci->hc_control = OHCI_USB_RESET,
			&ohci->regs->control);

	hc_release_ohci (ohci);

//os	release_mem_region (pci_resource_start (dev, 0), pci_resource_len (dev, 0));
//os	pci_disable_device (dev);
}


#ifdef	CONFIG_PM

/*-------------------------------------------------------------------------*/

static int
ohci_pci_suspend (struct pci_dev *dev, u32 state)
{
	ohci_t			*ohci = pci_get_drvdata(dev);
	unsigned long		flags;
	u16 cmd;

	if ((ohci->hc_control & OHCI_CTRL_HCFS) != OHCI_USB_OPER) {
#ifdef OHCI_DEBUG	//635u2
		dbg ("can't suspend usb-%s (state is %s)", dev->slot_name,
			hcfs2string (ohci->hc_control & OHCI_CTRL_HCFS));
#endif
		return -EIO;
	}

	/* act as if usb suspend can always be used */
#ifdef OHCI_DEBUG	//635u2
	info ("USB suspend: usb-%s", dev->slot_name);
#endif
	ohci->sleeping = 1;

	/* First stop processing */
  	spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
	ohci->hc_control &= ~(OHCI_CTRL_PLE|OHCI_CTRL_CLE|OHCI_CTRL_BLE|OHCI_CTRL_IE);
	writel (ohci->hc_control, &ohci->regs->control);
	writel (OHCI_INTR_SF, &ohci->regs->intrstatus);
	(void) readl (&ohci->regs->intrstatus);
  	spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);

	/* Wait a frame or two */
	mdelay(1);
	if (!readl (&ohci->regs->intrstatus) & OHCI_INTR_SF)
		mdelay (1);
		
#ifdef CONFIG_PMAC_PBOOK
	if (_machine == _MACH_Pmac)
		disable_irq (ohci->irq);
	/* else, 2.4 assumes shared irqs -- don't disable */
#endif
	/* Enable remote wakeup */
	writel (readl(&ohci->regs->intrenable) | OHCI_INTR_RD, &ohci->regs->intrenable);

	/* Suspend chip and let things settle down a bit */
	ohci->hc_control = OHCI_USB_SUSPEND;
	writel (ohci->hc_control, &ohci->regs->control);
	(void) readl (&ohci->regs->control);
	mdelay (500); /* No schedule here ! */
	switch (readl (&ohci->regs->control) & OHCI_CTRL_HCFS) {
		case OHCI_USB_RESET:
#ifdef OHCI_DEBUG	//635u2
			dbg("Bus in reset phase ???");
#endif
			break;
		case OHCI_USB_RESUME:
#ifdef OHCI_DEBUG	//635u2
			dbg("Bus in resume phase ???");
#endif		
			break;
		case OHCI_USB_OPER:
#ifdef OHCI_DEBUG	//635u2
			dbg("Bus in operational phase ???");
#endif
			break;
		case OHCI_USB_SUSPEND:
#ifdef OHCI_DEBUG	//635u2
			dbg("Bus suspended");
#endif
			break;
	}
	/* In some rare situations, Apple's OHCI have happily trashed
	 * memory during sleep. We disable it's bus master bit during
	 * suspend
	 */
	pci_read_config_word (dev, PCI_COMMAND, &cmd);
	cmd &= ~PCI_COMMAND_MASTER;
	pci_write_config_word (dev, PCI_COMMAND, cmd);
#ifdef CONFIG_PMAC_PBOOK
	{
	   	struct device_node	*of_node;

		/* Disable USB PAD & cell clock */
		of_node = pci_device_to_OF_node (ohci->ohci_dev);
		if (of_node)
			pmac_call_feature(PMAC_FTR_USB_ENABLE, of_node, 0, 0);
	}
#endif
	return 0;
}

/*-------------------------------------------------------------------------*/

static int
ohci_pci_resume (struct pci_dev *dev)
{
	ohci_t		*ohci = pci_get_drvdata(dev);
	int		temp;
	unsigned long	flags;

	/* guard against multiple resumes */
	atomic_inc (&ohci->resume_count);
	if (atomic_read (&ohci->resume_count) != 1) {
#ifdef OHCI_DEBUG	//635u2
		err ("concurrent PCI resumes for usb-%s", dev->slot_name);
#endif
		atomic_dec (&ohci->resume_count);
		return 0;
	}

#ifdef CONFIG_PMAC_PBOOK
	{
		struct device_node *of_node;

		/* Re-enable USB PAD & cell clock */
		of_node = pci_device_to_OF_node (ohci->ohci_dev);
		if (of_node)
			pmac_call_feature(PMAC_FTR_USB_ENABLE, of_node, 0, 1);
	}
#endif

	/* did we suspend, or were we powered off? */
	ohci->hc_control = readl (&ohci->regs->control);
	temp = ohci->hc_control & OHCI_CTRL_HCFS;

#ifdef DEBUG
	/* the registers may look crazy here */
	ohci_dump_status (ohci);
#endif

	/* Re-enable bus mastering */
	pci_set_master(ohci->ohci_dev);
	
	switch (temp) {

	case OHCI_USB_RESET:	// lost power
#ifdef OHCI_DEBUG	//635u2
		info ("USB restart: usb-%s", dev->slot_name);
#endif
		hc_restart (ohci);
		break;

	case OHCI_USB_SUSPEND:	// host wakeup
	case OHCI_USB_RESUME:	// remote wakeup
#ifdef OHCI_DEBUG	//635u2
		info ("USB continue: usb-%s from %s wakeup", dev->slot_name,
			(temp == OHCI_USB_SUSPEND)
				? "host" : "remote");
#endif
		ohci->hc_control = OHCI_USB_RESUME;
		writel (ohci->hc_control, &ohci->regs->control);
		(void) readl (&ohci->regs->control);
		mdelay (20); /* no schedule here ! */
		/* Some controllers (lucent) need a longer delay here */
		mdelay (15);
		temp = readl (&ohci->regs->control);
		temp = ohci->hc_control & OHCI_CTRL_HCFS;
		if (temp != OHCI_USB_RESUME) {
#ifdef OHCI_DEBUG	//635u2
			err ("controller usb-%s won't resume", dev->slot_name);
#endif		
			ohci->disabled = 1;
			return -EIO;
		}

		/* Some chips likes being resumed first */
		writel (OHCI_USB_OPER, &ohci->regs->control);
		(void) readl (&ohci->regs->control);
		mdelay (3);

		/* Then re-enable operations */
		spin_lock_irqsave (&usb_ed_lock, &flags);	//ZOT==> spin_lock_irqsave (&usb_ed_lock, flags);
		ohci->disabled = 0;
		ohci->sleeping = 0;
		ohci->hc_control = OHCI_CONTROL_INIT | OHCI_USB_OPER;
		if (!ohci->ed_rm_list[0] && !ohci->ed_rm_list[1]) {
			if (ohci->ed_controltail)
				ohci->hc_control |= OHCI_CTRL_CLE;
			if (ohci->ed_bulktail)
				ohci->hc_control |= OHCI_CTRL_BLE;
		}
		writel (ohci->hc_control, &ohci->regs->control);
		writel (OHCI_INTR_SF, &ohci->regs->intrstatus);
		writel (OHCI_INTR_SF, &ohci->regs->intrenable);
		/* Check for a pending done list */
		writel (OHCI_INTR_WDH, &ohci->regs->intrdisable);	
		(void) readl (&ohci->regs->intrdisable);
		spin_unlock_irqrestore (&usb_ed_lock, &flags);	//ZOT==> spin_unlock_irqrestore (&usb_ed_lock, flags);
#ifdef CONFIG_PMAC_PBOOK
		if (_machine == _MACH_Pmac)
			enable_irq (ohci->irq);
#endif
		if (ohci->hcca->done_head)
			dl_done_list (ohci, dl_reverse_done_list (ohci));
		writel (OHCI_INTR_WDH, &ohci->regs->intrenable); 
		writel (OHCI_BLF, &ohci->regs->cmdstatus); /* start bulk list */
		writel (OHCI_CLF, &ohci->regs->cmdstatus); /* start Control list */
		break;

	default:
		warn ("odd PCI resume for usb-%s", dev->slot_name);
	}

	/* controller is operational, extra resumes are harmless */
	atomic_dec (&ohci->resume_count);

	return 0;
}

#endif	/* CONFIG_PM */


/*-------------------------------------------------------------------------*/
//arrange for 635u2
#if 0
static const struct pci_device_id __devinitdata ohci_pci_ids [] = { {

	/*
	 * AMD-756 [Viper] USB has a serious erratum when used with
	 * lowspeed devices like mice.
	 */
	0x1022,
	0x740c,
	PCI_ANY_ID,
	PCI_ANY_ID,
	0,
	0,
	OHCI_QUIRK_AMD756,

} , {
	/* no matter who makes it */
	PCI_ANY_ID,
	PCI_ANY_ID,
	PCI_ANY_ID,
	PCI_ANY_ID,

	/* handle any USB OHCI controller */
	((PCI_CLASS_SERIAL_USB << 8) | 0x10),
	~0,

	}, { /* end: all zeroes */ }
};
#endif
#if 0
MODULE_DEVICE_TABLE (pci, ohci_pci_ids);
#endif
#if 0
static struct pci_driver ohci_pci_driver = {
	"usb-ohci",
	&ohci_pci_ids [0],

	ohci_pci_probe,
	//original Source	//635u2	
	//__devexit_p(ohci_pci_remove),
	0,
#ifdef	CONFIG_PM
	ohci_pci_suspend,
	ohci_pci_resume,
#endif	/* PM */
};
#endif
 
/*-------------------------------------------------------------------------*/

int __init ohci_hcd_init (unsigned int sz, unsigned int mem_addr)
{
	//Setup for 635u2
//	ohci_pci_driver.name = "usb-ohci";
//	ohci_pci_driver.id_table = &ohci_pci_ids [0];
//	ohci_pci_driver.probe = ohci_pci_probe;
//	ohci_pci_driver.remove = ohci_pci_remove;
	
#ifdef	CONFIG_PM
	ohci_pci_driver.suspend = ohci_pci_suspend,
	ohci_pci_driver.resume = ohci_pci_resume,
#endif	/* PM */
	
//	return pci_module_init (&ohci_pci_driver);
	ohci_pci_probe(sz, mem_addr);
	return 0;
}

/*-------------------------------------------------------------------------*/

static void __exit ohci_hcd_cleanup (void) 
{	
//	pci_unregister_driver (&ohci_pci_driver);
}
#if 0	//635u2
module_init (ohci_hcd_init);
module_exit (ohci_hcd_cleanup);


MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("GPL");
#endif

