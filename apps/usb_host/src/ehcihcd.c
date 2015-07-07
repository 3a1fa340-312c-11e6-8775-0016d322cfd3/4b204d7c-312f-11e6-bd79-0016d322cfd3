/*
 * Copyright (c) 2000-2002 by David Brownell
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef CONFIG_USB_DEBUG
	#define DEBUG
#else
	#undef DEBUG
#endif

#undef EHCI_DEBUG	//635u2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "list.h"
#include "psglobal.h"
#include "usb.h"
#include "hub.h"
#include "u2hcd.h"
#include "ehci.h"
#include "completion.h"

extern struct hc_driver	*GLOBAL_DRIVER;

//ehci_tasklet	eason 20100818
#define EHCI_TASKLET_TASK_PRI         20
#define EHCI_TASKLET_TASK_STACK_SIZE	 4096
static cyg_uint8 			EHCI_TASKLET_Stack[EHCI_TASKLET_TASK_STACK_SIZE];
static cyg_thread       EHCI_TASKLET_Task;
static cyg_handle_t     EHCI_TASKLET_TaskHdl;
cyg_sem_t ehci_tasklet_sem;
cyg_mutex_t ehci_tasklet_mt;
struct ehci_hcd *ehci_tasklet_hcd = NULL;
//#undef KERN_DEBUG
//#define KERN_DEBUG ""

/*-------------------------------------------------------------------------*/

/*
 * EHCI hc_driver implementation ... experimental, incomplete.
 * Based on the final 1.0 register interface specification.
 *
 * There are lots of things to help out with here ... notably
 * everything "periodic", and of course testing with all sorts
 * of usb 2.0 devices and configurations.
 *
 * USB 2.0 shows up in upcoming www.pcmcia.org technology.
 * First was PCMCIA, like ISA; then CardBus, which is PCI.
 * Next comes "CardBay", using USB 2.0 signals.
 *
 * Contains additional contributions by:
 *	Brad Hards
 *	Rory Bolt
 *	...
 *
 * HISTORY:
 *
 * 2002-05-07	Some error path cleanups to report better errors; wmb();
 *	use non-CVS version id; better iso bandwidth claim.
 * 2002-04-19	Control/bulk/interrupt submit no longer uses giveback() on
 *	errors in submit path.  Bugfixes to interrupt scheduling/processing.
 * 2002-03-05	Initial high-speed ISO support; reduce ITD memory; shift
 *	more checking to generic hcd framework (db).  Make it work with
 *	Philips EHCI; reduce PCI traffic; shorten IRQ path (Rory Bolt).
 * 2002-01-14	Minor cleanup; version synch.
 * 2002-01-08	Fix roothub handoff of FS/LS to companion controllers.
 * 2002-01-04	Control/Bulk queuing behaves.
 *
 * 2001-12-12	Initial patch version for Linux 2.5.1 kernel.
 * 2001-June	Works with usb-storage and NEC EHCI on 2.4
 */
#if 0	//635u2
#define DRIVER_VERSION "2002-May-07"
#define DRIVER_AUTHOR "David Brownell"
#define DRIVER_DESC "USB 2.0 'Enhanced' Host Controller (EHCI) Driver"
#endif

// #define EHCI_VERBOSE_DEBUG
// #define have_split_iso

/* Initial IRQ latency:  lower than default */
static int log2_irq_thresh = 0;		// 0 to 6
#if 0	//635u2
MODULE_PARM (log2_irq_thresh, "i");
MODULE_PARM_DESC (log2_irq_thresh, "log2 IRQ latency, 1-64 microframes");
#endif
#define	INTR_MASK (STS_IAA | STS_FATAL | STS_ERR | STS_INT)

/*-------------------------------------------------------------------------*/
#if 0	//635u2
#include "ehci.h"
#include "ehci-dbg.c"
#endif
/*-------------------------------------------------------------------------*/
#if 0	//ZOT716u2
#define __SQRAM__   __attribute__ ((section (".sqbuf")))
uint16 EHCI_PCI_Word_Buf __SQRAM__;
uint32 EHCI_PCI_DWord_Buf __SQRAM__;
uint16 inw2 (uint32 addr)
{	
	pciHost_MemRead(addr, (uint32*)&EHCI_PCI_Word_Buf, 2, 0, 2);
	return (NGET16((uint8*)&EHCI_PCI_Word_Buf));
}

void outw2 (uint16 data, uint32 addr)
{
	EHCI_PCI_Word_Buf = data;
	pciHost_MemWrite(addr, (uint32*)&EHCI_PCI_Word_Buf, 2, 0, 2);
}

uint32 inl2 (uint32 addr)
{	
	pciHost_MemRead(addr, (uint32*)&EHCI_PCI_DWord_Buf, 4, 0, 2);
	return (NGET32((uint8*)&EHCI_PCI_DWord_Buf));
}

void outl2 (uint32 data, uint32 addr)
{
	EHCI_PCI_DWord_Buf = data;
	pciHost_MemWrite(addr, (uint32*)&EHCI_PCI_DWord_Buf, 4, 0, 2);
}

#endif //ZOT716u2
//ZOT716u2
uint16 inw2 (uint32 addr)
{	
	return (* ( volatile uint16 * )( addr ));
}

void outw2 (uint16 data, uint32 addr)
{
	* ( volatile uint16 * )( addr )= data;
}

uint32 inl2 (uint32 addr)
{	
	return (* ( volatile uint32 * )( addr ));
}

void outl2 (uint32 data, uint32 addr)
{
	* ( volatile uint32 * )( addr )= data;
}

/*
 * hc states include: unknown, halted, ready, running
 * transitional states are messy just now
 * trying to avoid "running" unless urbs are active
 * a "ready" hc can be finishing prefetched work
 */

/* halt a non-running controller */
static void ehci_reset (struct ehci_hcd *ehci)
{
	u32	command = readl (&ehci->regs->command);

	command |= CMD_RESET;
#ifdef EHCI_DEBUG	//635u2
	dbg_cmd (ehci, "reset", command);
#endif
	writel (command, &ehci->regs->command);
	while (readl (&ehci->regs->command) & CMD_RESET)
		continue;
	ehci->hcd.state = USB_STATE_HALT;
}

/* idle the controller (from running) */
void ehci_ready (struct ehci_hcd *ehci)
{
	u32	command;

#ifdef DEBUG
	if (!HCD_IS_RUNNING (ehci->hcd.state))
		BUG ();
#endif

	while (!(readl (&ehci->regs->status) & (STS_ASS | STS_PSS)))
		udelay (100);
	command = readl (&ehci->regs->command);
	command &= ~(CMD_ASE | CMD_IAAD | CMD_PSE);
	writel (command, &ehci->regs->command);

	// hardware can take 16 microframes to turn off ...
	ehci->hcd.state = USB_STATE_READY;
}

/*-------------------------------------------------------------------------*/
#if 0	//635u2
#include "ehci-hub.c"
#include "ehci-mem.c"
#include "ehci-q.c"
#include "ehci-sched.c"
#endif
/*-------------------------------------------------------------------------*/

//eason 20100818 void ehci_tasklet (unsigned long param);
void ehci_tasklet(cyg_addrword_t arg);

/* called by khubd or root hub init threads */

static int ehci_start (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	u32			temp;
	struct usb_device	*udev;
	int			retval;
	u32			hcc_params;
	u8          tempbyte;
	u32			temp32;	

	// FIXME:  given EHCI 0.96 or later, and a controller with
	// the USBLEGSUP/USBLEGCTLSTS extended capability, make sure
	// the BIOS doesn't still own this controller.

	spin_lock_init (&ehci->lock);

	ehci->caps = (struct ehci_caps *) hcd->regs;
//test
/*	ehci->caps = (struct ehci_caps *) kmalloc(sizeof(struct ehci_caps) + sizeof(struct ehci_regs),0);
	ehci->caps->length = (readw(0x1e000000 + 0) & 0x00FF);
	ehci->caps->hci_version = readw(0x1e000000 + 2);
	ehci->caps->hcs_params = readl(0x1e000000 + 4);
	ehci->caps->hcc_params = readl(0x1e000000 + 8);
*/	
	ehci->regs = (struct ehci_regs *) ((u32)hcd->regs + (readw(hcd->regs) & 0x00FF));	//635u2
#ifdef EHCI_DEBUG	//635u2
	dbg_hcs_params (ehci, "ehci_start");
	dbg_hcc_params (ehci, "ehci_start");
#endif
	/* cache this readonly data; minimize PCI reads */
	ehci->hcs_params = readl (&ehci->caps->hcs_params);

	/*
	 * hw default: 1K periodic list heads, one per frame.
	 * periodic_size can shrink by USBCMD update if hcc_params allows.
	 */
	ehci->periodic_size = DEFAULT_I_TDPS;
	if ((retval = ehci_mem_init (ehci, SLAB_KERNEL)) < 0)
		return retval;
	hcc_params = readl (&ehci->caps->hcc_params);

	/* controllers may cache some of the periodic schedule ... */
	if (HCC_ISOC_CACHE (hcc_params)) 	// full frame cache
		ehci->i_thresh = 8;
	else					// N microframes cached
		ehci->i_thresh = 2 + HCC_ISOC_THRES (hcc_params);

	ehci->async = 0;
	ehci->reclaim = 0;
	ehci->next_uframe = -1;

	/* controller state:  unknown --> reset */

	/* EHCI spec section 4.1 */
	// FIXME require STS_HALT before reset...
	ehci_reset (ehci);
	writel (INTR_MASK, &ehci->regs->intr_enable);
	//________________###test###_____________________//
	//Read Interrupt Mask
//ZOT716u2	temp32 = readl (&ehci->regs->intr_enable);
	//_______________________________________________//	
	writel (ehci->periodic_dma, &ehci->regs->frame_list);	//615wu Print Server have no perodic request
	//________________###test###_____________________//
	//Read Periodic
//ZOT716u2	temp32 = readl (&ehci->regs->frame_list);
	//_______________________________________________//	
	/*
	 * hcc_params controls whether ehci->regs->segment must (!!!)
	 * be used; it constrains QH/ITD/SITD and QTD locations.
	 * pci_pool consistent memory always uses segment zero.
	 */
	if (HCC_64BIT_ADDR (hcc_params)) {
		writel (0, &ehci->regs->segment);
		/*
		 * FIXME Enlarge pci_set_dma_mask() when possible.  The DMA
		 * mapping API spec now says that'll affect only single shot
		 * mappings, and the pci_pool data will stay safe in seg 0.
		 * That's what we want:  no extra copies for USB transfers.
		 */
#ifdef EHCI_DEBUG	//635u2
		info ("restricting 64bit DMA mappings to segment 0 ...");
#endif
	}

	/* clear interrupt enables, set irq latency */
	temp = readl (&ehci->regs->command) & 0xff;
	if (log2_irq_thresh < 0 || log2_irq_thresh > 6)
	    log2_irq_thresh = 0;
	temp |= 1 << (16 + log2_irq_thresh);
	// keeping default periodic framelist size
	temp &= ~(CMD_IAAD | CMD_ASE | CMD_PSE);
	// Philips, Intel, and maybe others need CMD_RUN before the
	// root hub will detect new devices (why?); NEC doesn't
	temp |= CMD_RUN;
	writel (temp, &ehci->regs->command);
	//________________###test###_____________________//
	//Read Command
//ZOT716u2	temp32 = readl (&ehci->regs->command);
	//_______________________________________________//		
#ifdef EHCI_DEBUG	//635u2
	dbg_cmd (ehci, "init", temp);
#endif
	/* set async sleep time = 10 us ... ? */
	
	//________________###test###_____________________//
	//Read Status
//ZOT716u2	temp32 = readl (&ehci->regs->status);
	
	//Read PCI USB2 status and command
//ZOT716u2	temp32 = pciHost_ConfigRead(0x04, 2);
	
	//Read Frame Index
//ZOT716u2	temp32 = readl (&ehci->regs->frame_index);
	//_______________________________________________//	
//pending  635u2
//	ehci->tasklet.func = ehci_tasklet;
//	ehci->tasklet.data = (unsigned long) ehci;

	/* wire up the root hub */
	hcd->bus->root_hub = udev = usb_alloc_dev (NULL, hcd->bus);
	if (!udev) {
done2:
		ehci_mem_cleanup (ehci);
		return -ENOMEM;
	}

	/*
	 * Start, enabling full USB 2.0 functionality ... usb 1.1 devices
	 * are explicitly handed to companion controller(s), so no TT is
	 * involved with the root hub.
	 */
	ehci->hcd.state = USB_STATE_READY;
	writel (FLAG_CF, &ehci->regs->configured_flag);
	//________________###test###_____________________//
	//Read Flag
//ZOT716u2	temp32 = readl (&ehci->regs->configured_flag);
	//_______________________________________________//	
	
	readl (&ehci->regs->command);	/* unblock posted write */

        /* PCI Serial Bus Release Number is at 0x60 offset */
//os	pci_read_config_byte (hcd->pdev, 0x60, &tempbyte);
//ZOT716u2	tempbyte = pciHost_ConfigRead(0x60,2);

	temp = readw (&ehci->caps->hci_version);
#ifdef EHCI_DEBUG	//635u2
	info ("USB %x.%x support enabled, EHCI rev %x.%2x",
	      ((tempbyte & 0xf0)>>4),
	      (tempbyte & 0x0f),
	       temp >> 8,
	       temp & 0xff);
#endif
	/*
	 * From here on, khubd concurrently accesses the root
	 * hub; drivers will be talking to enumerated devices.
	 *
	 * Before this point the HC was idle/ready.  After, khubd
	 * and device drivers may start it running.
	 */
	usb_connect (udev);
	udev->speed = USB_SPEED_HIGH;
	
//ZOT716u2	armond_printf("EHCI Root Hub New start\n");
	
	if (usb_new_device (udev) != 0) {
		if (hcd->state == USB_STATE_RUNNING)
			ehci_ready (ehci);
		while (readl (&ehci->regs->status) & (STS_ASS | STS_PSS))
			udelay (100);
		ehci_reset (ehci);
		// usb_disconnect (udev); 
		hcd->bus->root_hub = 0;
		usb_free_dev (udev); 
		retval = -ENODEV;
		goto done2;
	}
//ZOT716u2	armond_printf("EHCI Root Hub New OK end\n");
	
	return 0;
}

/* always called by thread; normally rmmod */

static void ehci_stop (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s: stop", hcd->bus_name);
#endif
	if (hcd->state == USB_STATE_RUNNING)
		ehci_ready (ehci);
	while (readl (&ehci->regs->status) & (STS_ASS | STS_PSS))
		udelay (100);
	ehci_reset (ehci);

	// root hub is shut down separately (first, when possible)
	scan_async (ehci);
	if (ehci->next_uframe != -1)
		scan_periodic (ehci);
	ehci_mem_cleanup (ehci);
#ifdef EHCI_DEBUG	//635u2
	dbg_status (ehci, "ehci_stop completed", readl (&ehci->regs->status));
#endif
}

int ehci_get_frame (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	return (readl (&ehci->regs->frame_index) >> 3) % ehci->periodic_size;
}

/*-------------------------------------------------------------------------*/

#ifdef	CONFIG_PM

/* suspend/resume, section 4.3 */

static int ehci_suspend (struct usb_hcd *hcd, u32 state)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	int			ports;
	int			i;
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s: suspend to %d", hcd->bus_name, state);
#endif
	ports = HCS_N_PORTS (ehci->hcs_params);

	// FIXME:  This assumes what's probably a D3 level suspend...

	// FIXME:  usb wakeup events on this bus should resume the machine.
	// pci config register PORTWAKECAP controls which ports can do it;
	// bios may have initted the register...

	/* suspend each port, then stop the hc */
	for (i = 0; i < ports; i++) {
		int	temp = readl (&ehci->regs->port_status [i]);

		if ((temp & PORT_PE) == 0
				|| (temp & PORT_OWNER) != 0)
			continue;
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s: suspend port %d", hcd->bus_name, i);
#endif
		temp |= PORT_SUSPEND;
		writel (temp, &ehci->regs->port_status [i]);
	}

	if (hcd->state == USB_STATE_RUNNING)
		ehci_ready (ehci);
	while (readl (&ehci->regs->status) & (STS_ASS | STS_PSS))
		udelay (100);
	writel (readl (&ehci->regs->command) & ~CMD_RUN, &ehci->regs->command);

// save pci FLADJ value

	/* who tells PCI to reduce power consumption? */

	return 0;
}

static int ehci_resume (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	int			ports;
	int			i;
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s: resume", hcd->bus_name);
#endif
	ports = HCS_N_PORTS (ehci->hcs_params);

	// FIXME:  if controller didn't retain state,
	// return and let generic code clean it up
	// test configured_flag ?

	/* resume HC and each port */
// restore pci FLADJ value
	// khubd and drivers will set HC running, if needed;
	hcd->state = USB_STATE_READY;
	// FIXME Philips/Intel/... etc don't really have a "READY"
	// state ... turn on CMD_RUN too
	for (i = 0; i < ports; i++) {
		int	temp = readl (&ehci->regs->port_status [i]);

		if ((temp & PORT_PE) == 0
				|| (temp & PORT_SUSPEND) != 0)
			continue;
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s: resume port %d", hcd->bus_name, i);
#endif
		temp |= PORT_RESUME;
		writel (temp, &ehci->regs->port_status [i]);
		readl (&ehci->regs->command);	/* unblock posted writes */

		wait_ms (20);
		temp &= ~PORT_RESUME;
		writel (temp, &ehci->regs->port_status [i]);
	}
	readl (&ehci->regs->command);	/* unblock posted writes */
	return 0;
}

#endif

/*-------------------------------------------------------------------------*/

/*
 * tasklet scheduled by some interrupts and other events
 * calls driver completion functions ... but not in_irq()
 */
//eason 20100818 void ehci_tasklet (unsigned long param)
void ehci_tasklet(cyg_addrword_t arg)
{
	//eason 20100818	struct ehci_hcd		*ehci = (struct ehci_hcd *) param;
	struct ehci_hcd		*ehci = NULL;
	int i_state;	//eason 20100818
	
	while(1)//eason 20100818
	{
		cyg_semaphore_wait(&ehci_tasklet_sem);	//eason 20100818
		
		ehci = ehci_tasklet_hcd;				//eason 20100818									
		i_state = dirps();						//eason 20100818
		cyg_mutex_lock(&ehci_tasklet_mt);		//eason 20100818
		
		if (ehci->reclaim_ready)
			end_unlink_async (ehci);
		scan_async (ehci);
		if (ehci->next_uframe != -1)
			scan_periodic (ehci);
		
		cyg_mutex_unlock(&ehci_tasklet_mt);		//eason 20100818
		restore(i_state);						//eason 20100818
	}
}

/*-------------------------------------------------------------------------*/

static void ehci_irq (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	u32			status = readl (&ehci->regs->status);
	int			bh;

	status &= INTR_MASK;
	if (!status)			/* irq sharing? */
		return;

	/* clear (just) interrupts */
	writel (status, &ehci->regs->status);
    readl (&ehci->regs->command);	/* unblock posted write */
	bh = 0;

#ifdef	EHCI_VERBOSE_DEBUG
	/* unrequested/ignored: Port Change Detect, Frame List Rollover */
	dbg_status (ehci, "irq", status);
#endif

	/* INT, ERR, and IAA interrupt rates can be throttled */

	/* normal [4.15.1.2] or error [4.15.1.1] completion */
	if (likely ((status & (STS_INT|STS_ERR)) != 0))
		bh = 1;

	/* complete the unlinking of some qh [4.15.2.3] */
	if (status & STS_IAA) {
		ehci->reclaim_ready = 1;
		bh = 1;
	}

	/* PCI errors [4.15.2.4] */
	if (unlikely ((status & STS_FATAL) != 0)) {
#ifdef EHCI_DEBUG	//635u2		
		err ("%s: fatal error, state %x", hcd->bus_name, hcd->state);
#endif
		ehci_reset (ehci);
		// generic layer kills/unlinks all urbs
		// then tasklet cleans up the rest
		bh = 1;
	}

	/* most work doesn't need to be in_irq() */
//original Source	//635u2
//	if (likely (bh == 1))
//		tasklet_schedule (&ehci->tasklet);
	if (likely (bh == 1)){
		//eason 20100818	ehci_tasklet(ehci);	//635u2
		ehci_tasklet_hcd = ehci;
		cyg_semaphore_post(&ehci_tasklet_sem);
	}
}

/*-------------------------------------------------------------------------*/

/*
 * non-error returns are a promise to giveback() the urb later
 * we drop ownership so next owner (or urb unlink) can get it
 *
 * urb + dev is in hcd_dev.urb_list
 * we're queueing TDs onto software and hardware lists
 *
 * hcd-specific init for hcpriv hasn't been done yet
 *
 * NOTE:  EHCI queues control and bulk requests transparently, like OHCI.
 */

extern int submit_async (struct ehci_hcd *ehci,struct urb *urb,struct list_head	*qtd_list,int mem_flags);

static int ehci_urb_enqueue (
	struct usb_hcd	*hcd,
	struct urb	*urb,
	int		mem_flags
) {
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	struct list_head	qtd_list;

	urb->transfer_flags &= ~EHCI_STATE_UNLINK;
	INIT_LIST_HEAD (&qtd_list);
	switch (usb_pipetype (urb->pipe)) {

	case PIPE_CONTROL:
	case PIPE_BULK:
		if (!qh_urb_transaction (ehci, urb, &qtd_list, mem_flags))
			return -ENOMEM;
		return submit_async (ehci, urb, &qtd_list, mem_flags);

	case PIPE_INTERRUPT:
		if (!qh_urb_transaction (ehci, urb, &qtd_list, mem_flags))
			return -ENOMEM;
		return intr_submit (ehci, urb, &qtd_list, mem_flags);

	case PIPE_ISOCHRONOUS:
		if (urb->dev->speed == USB_SPEED_HIGH)
			return itd_submit (ehci, urb, mem_flags);
#ifdef have_split_iso
		else
			return sitd_submit (ehci, urb, mem_flags);
#else
#ifdef EHCI_DEBUG	//635u2
		dbg ("no split iso support yet");
#endif		
		return -ENOSYS;
#endif /* have_split_iso */

	default:	/* can't happen */
		return -ENOSYS;
	}
}

/* remove from hardware lists
 * completions normally happen asynchronously
 */

static int ehci_urb_dequeue (struct usb_hcd *hcd, struct urb *urb)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	struct ehci_qh		*qh = (struct ehci_qh *) urb->hcpriv;
	unsigned long		flags;
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s urb_dequeue %p qh state %d",
		hcd->bus_name, urb, qh->qh_state);
#endif
	switch (usb_pipetype (urb->pipe)) {
	case PIPE_CONTROL:
	case PIPE_BULK:
		spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
		if (ehci->reclaim) {
#ifdef EHCI_DEBUG	//635u2
dbg ("dq: reclaim busy, %s", RUN_CONTEXT);
#endif
			if (in_interrupt ()) {
				spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&ehci->lock, flags);
				return -EAGAIN;
			}
			while (qh->qh_state == QH_STATE_LINKED
					&& ehci->reclaim
					&& ehci->hcd.state != USB_STATE_HALT
					) {
				spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&ehci->lock, flags);
// yeech ... this could spin for up to two frames!
#ifdef EHCI_DEBUG	//635u2	
	dbg ("wait for dequeue: state %d, reclaim %p, hcd state %d", qh->qh_state, ehci->reclaim, ehci->hcd.state);
#endif	
				udelay (100);
				spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
			}
		}
		if (qh->qh_state == QH_STATE_LINKED)
			start_unlink_async (ehci, qh);
		spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&ehci->lock, flags);
		return 0;

	case PIPE_INTERRUPT:
		intr_deschedule (ehci, urb->start_frame, qh,
			(urb->dev->speed == USB_SPEED_HIGH)
			    ? urb->interval
			    : (urb->interval << 3));
		if (ehci->hcd.state == USB_STATE_HALT)
			urb->status = -ESHUTDOWN;
		qh_completions (ehci, qh, 1);
		return 0;

	case PIPE_ISOCHRONOUS:
		// itd or sitd ...

		// wait till next completion, do it then.
		// completion irqs can wait up to 1024 msec,
		urb->transfer_flags |= EHCI_STATE_UNLINK;
		return 0;
	}
	return -EINVAL;
}

/*-------------------------------------------------------------------------*/

// bulk qh holds the data toggle

static void ehci_free_config (struct usb_hcd *hcd, struct usb_device *udev)
{
	struct hcd_dev		*dev = (struct hcd_dev *)udev->hcpriv;
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	int			i;
	unsigned long		flags;

	/* ASSERT:  nobody can be submitting urbs for this any more */
#ifdef EHCI_DEBUG	//635u2
	dbg ("%s: free_config devnum %d", hcd->bus_name, udev->devnum);
#endif
	spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
	for (i = 0; i < 32; i++) {
		if (dev->ep [i]) {
			struct ehci_qh		*qh;

			/* dev->ep never has ITDs or SITDs */
			qh = (struct ehci_qh *) dev->ep [i];
#ifdef EHCI_DEBUG	//635u2
			vdbg ("free_config, ep 0x%02x qh %p", i, qh);
#endif
			if (!list_empty (&qh->qtd_list)) {
#ifdef EHCI_DEBUG	//635u2
				dbg ("ep 0x%02x qh %p not empty!", i, qh);
				BUG ();
#endif
			}
			dev->ep [i] = 0;

			/* wait_ms() won't spin here -- we're a thread */
			while (qh->qh_state == QH_STATE_LINKED
					&& ehci->reclaim
					&& ehci->hcd.state != USB_STATE_HALT
					) {
				spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&ehci->lock, flags);
				wait_ms (1);
				spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
			}
			if (qh->qh_state == QH_STATE_LINKED) {
				start_unlink_async (ehci, qh);
				while (qh->qh_state != QH_STATE_IDLE) {
					spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&ehci->lock, flags);
					wait_ms (1);
					spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
				}
			}
			qh_put (ehci, qh);
		}
	}

	spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&ehci->lock, flags);
}

/*-------------------------------------------------------------------------*/
//635u2
extern struct usb_hcd *ehci_hcd_alloc (void);
extern void ehci_hcd_free (struct usb_hcd *hcd);
extern int ehci_hub_status_data (struct usb_hcd *hcd, char *buf);
extern int ehci_hub_control (struct usb_hcd *hcd,u16 typeReq,u16 wValue,u16 wIndex,char *buf,u16 wLength); 

static const char	hcd_name [] = "ehci-hcd";

static const struct hc_driver ehci_driver = {
	hcd_name,

	/*
	 * generic hardware linkage
	 */
	ehci_irq,
	HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	ehci_start,
#ifdef	CONFIG_PM
	ehci_suspend,
	ehci_resume,
#else    //635u2
	NULL,
	NULL,
#endif
	ehci_stop,

	/*
	 * scheduling support
	 */
	
	ehci_get_frame,
	
	/*
	 * memory lifecycle (except per-request)
	 */
	ehci_hcd_alloc,
	ehci_hcd_free,

	/*
	 * managing i/o requests and associated device resources
	 */
	ehci_urb_enqueue,
	ehci_urb_dequeue,
	ehci_free_config,



	/*
	 * root hub support
	 */
	ehci_hub_status_data,
	ehci_hub_control,
};

/*-------------------------------------------------------------------------*/

/* EHCI spec says PCI is required. */
//arrange for 635u2
/* PCI driver selection metadata; PCI hotplugging uses this */
#if 0
static const struct pci_device_id __devinitdata pci_ids [] = { {

	/* handle any USB 2.0 EHCI controller */

	/* no matter who makes it */
	PCI_ANY_ID,
	PCI_ANY_ID,
	PCI_ANY_ID,
	PCI_ANY_ID,
	
	((PCI_CLASS_SERIAL_USB << 8) | 0x20),
	~0,
	(unsigned long) &ehci_driver,

}, { /* end: all zeroes */ }
};
#endif
#if 0	//635u2
MODULE_DEVICE_TABLE (pci, pci_ids);
#endif
/* pci driver glue; this is a "new style" PCI driver module */
#if 0
struct pci_driver ehci_pci_driver = {
	(char *) hcd_name,
	pci_ids,
	usb_hcd_pci_probe,
	usb_hcd_pci_remove,

#ifdef	CONFIG_PM
	usb_hcd_pci_suspend,
	usb_hcd_pci_resume,
#endif
};
#endif
#if 0	//635u2
#define DRIVER_INFO DRIVER_VERSION " " DRIVER_DESC
EXPORT_NO_SYMBOLS;
MODULE_DESCRIPTION (DRIVER_INFO);
MODULE_AUTHOR (DRIVER_AUTHOR);
MODULE_LICENSE ("GPL");
#endif

extern cyg_mutex_t hcd_list_lock;	//ZOT==>
extern cyg_mutex_t hcd_data_lock;	//ZOT==>
extern cyg_mutex_t hcd_free_lock;	//eason 20100818


int ehci_hcd_init (unsigned int sz, unsigned int mem_addr) 
{
#ifdef EHCI_DEBUG	//635u2	
	dbg (DRIVER_INFO);
	dbg ("block sizes: qh %Zd qtd %Zd itd %Zd sitd %Zd",
		sizeof (struct ehci_qh), sizeof (struct ehci_qtd),
		sizeof (struct ehci_itd), sizeof (struct ehci_sitd));
#endif
#if 0
	//Setup for 635u2
	ehci_pci_driver.name = (char *) hcd_name;
	ehci_pci_driver.id_table = &pci_ids [0];
	ehci_pci_driver.probe = usb_hcd_pci_probe;
	ehci_pci_driver.remove = usb_hcd_pci_remove;
#endif	
#ifdef	CONFIG_PM
	ehci_pci_driver.suspend = usb_hcd_pci_suspend,
	ehci_pci_driver.resume = usb_hcd_pci_resume,
#endif	/* PM */

	init_MUTEX(&hcd_list_lock);
	__spin_lock_init(&hcd_data_lock);
	__spin_lock_init(&hcd_free_lock);	//eason 20100818
	
//	return pci_module_init (&ehci_pci_driver);
	GLOBAL_DRIVER = &ehci_driver;	//eCos
	usb_hcd_pci_probe(sz, mem_addr);			//eCos
	
//eason 20100818
	cyg_semaphore_init(&ehci_tasklet_sem, 0);	
	cyg_mutex_init(&ehci_tasklet_mt);
	
	cyg_thread_create(EHCI_TASKLET_TASK_PRI,
                  ehci_tasklet,
                  0,
                  "ehci_tasklet",
                  (void *) (EHCI_TASKLET_Stack),
                  EHCI_TASKLET_TASK_STACK_SIZE,
                  &EHCI_TASKLET_TaskHdl,
                  &EHCI_TASKLET_Task);
	
	//Start EHCI_TASKLET Thread
	cyg_thread_resume(EHCI_TASKLET_TaskHdl);	
	
	return 0;
}
#if 0	//635u2
module_init (init);
#endif
static void __exit cleanup (void) 
{	
//	pci_unregister_driver (&ehci_pci_driver);
}
#if 0	//635u2
module_exit (cleanup);
#endif
