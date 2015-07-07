/*
 * Copyright (c) 2001-2002 by David Brownell
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

#undef USB2_DEBUG	//635u2
#undef EXPORT_L_SYMBOL	//635u2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
//ZOT716u2 #include "ap_api.h"
#include "psglobal.h"
#include "usb.h"
#include "hub.h"
#include "u2hcd.h"
#include "ehci.h"
#include "completion.h"
#include "list.h"
#include "star_intc.h"	//ZOT716u2

/*-------------------------------------------------------------------------*/

/*
 * USB Host Controller Driver framework
 *
 * Plugs into usbcore (usb_bus) and lets HCDs share code, minimizing
 * HCD-specific behaviors/bugs.  Think of it as the "upper level" of
 * some drivers, where the "lower level" is hardware-specific.
 *
 * This does error checks, tracks devices and urbs, and delegates to a
 * "hc_driver" only for code (and data) that really needs to know about
 * hardware differences.  That includes root hub registers, i/o queues,
 * and so on ... but as little else as possible.
 *
 * Shared code includes most of the "root hub" code (these are emulated,
 * though each HC's hardware works differently) and PCI glue, plus request
 * tracking overhead.  The HCD code should only block on spinlocks or on
 * hardware handshaking; blocking on software events (such as other kernel
 * threads releasing resources, or completing actions) is all generic.
 *
 * Happens the USB 2.0 spec says this would be invisible inside the "USBD",
 * and includes mostly a "HCDI" (HCD Interface) along with some APIs used
 * only by the hub driver ... and that neither should be seen or used by
 * usb client device drivers.
 *
 * Contributors of ideas or unattributed patches include: David Brownell,
 * Roman Weissgaerber, Rory Bolt, ...
 *
 * HISTORY:
 * 2002-sept	Merge some 2.5 updates so we can share hardware level HCD
 * 	code between the 2.4.20+ and 2.5 trees.
 * 2002-feb	merge to 2.4.19
 * 2001-12-12	Initial patch version for Linux 2.5.1 kernel.
 */

/*-------------------------------------------------------------------------*/

/* host controllers we manage */
static LIST_HEAD (hcd_list);

/* used when updating list of hcds */
DECLARE_MUTEX (hcd_list_lock);	//ZOT==> static DECLARE_MUTEX (hcd_list_lock);	//define in usb.h (down & up)

/* used when updating hcd data */
spinlock_t hcd_data_lock;	//ZOT==> static spinlock_t hcd_data_lock = SPIN_LOCK_UNLOCKED;	//define in usb.h

/* used when free hcd data */
spinlock_t hcd_free_lock;	//eason 20100818

static struct usb_operations hcd_operations;

void rh_report_status (cyg_handle_t handle, cyg_addrword_t ptr);

cyg_handle_t RH_SysClk;
cyg_handle_t RH_Counter;
cyg_handle_t RH_Alarm;   
cyg_alarm RH_timerAlarm;
struct urb *GLOBAL_EHCI_RH_URB;
uint32 RH_OK = 0, RH_NEXT = 0;

/*-------------------------------------------------------------------------*/

/*
 * Sharable chunks of root hub code.
 */

/*-------------------------------------------------------------------------*/
#define LINUX_VERSION_CODE 0	//635u2
#define KERNEL_REL	((LINUX_VERSION_CODE >> 16) & 0x0ff)
#define KERNEL_VER	((LINUX_VERSION_CODE >> 8) & 0x0ff)


/* usb 2.0 root hub device descriptor */
static const u8 usb2_rh_dev_descriptor [18] = {
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x00, 0x02, /*  __u16 bcdUSB; v2.0 */

	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x01,       /*  __u8  bDeviceProtocol; [ usb 2.0 single TT ]*/
	0x08,       /*  __u8  bMaxPacketSize0; 8 Bytes */

	0x00, 0x00, /*  __u16 idVendor; */
 	0x00, 0x00, /*  __u16 idProduct; */
	KERNEL_VER, KERNEL_REL, /*  __u16 bcdDevice */

	0x03,       /*  __u8  iManufacturer; */
	0x02,       /*  __u8  iProduct; */
	0x01,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};

/* no usb 2.0 root hub "device qualifier" descriptor: one speed only */

/* usb 1.1 root hub device descriptor */
static const u8 usb11_rh_dev_descriptor [18] = {
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x10, 0x01, /*  __u16 bcdUSB; v1.1 */

	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x00,       /*  __u8  bDeviceProtocol; [ low/full speeds only ] */
	0x08,       /*  __u8  bMaxPacketSize0; 8 Bytes */

	0x00, 0x00, /*  __u16 idVendor; */
 	0x00, 0x00, /*  __u16 idProduct; */
	KERNEL_VER, KERNEL_REL, /*  __u16 bcdDevice */

	0x03,       /*  __u8  iManufacturer; */
	0x02,       /*  __u8  iProduct; */
	0x01,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};


/*-------------------------------------------------------------------------*/

/* Configuration descriptors for our root hubs */

static const u8 fs_rh_config_descriptor [] = {

	/* one configuration */
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19, 0x00, /*  __u16 wTotalLength; */
	0x01,       /*  __u8  bNumInterfaces; (1) */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0x40,       /*  __u8  bmAttributes; 
				 Bit 7: Bus-powered,
				     6: Self-powered,
				     5 Remote-wakwup,
				     4..0: resvd */
	0x00,       /*  __u8  MaxPower; */
      
	/* USB 1.1:
	 * USB 2.0, single TT organization (mandatory):
	 *	one interface, protocol 0
	 *
	 * USB 2.0, multiple TT organization (optional):
	 *	two interfaces, protocols 1 (like single TT)
	 *	and 2 (multiple TT mode) ... config is
	 *	sometimes settable
	 *	NOT IMPLEMENTED
	 */

	/* one interface */
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; [usb1.1 or single tt] */
	0x00,       /*  __u8  if_iInterface; */
     
	/* one endpoint (status change endpoint) */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
 	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
 	0x02, 0x00, /*  __u16 ep_wMaxPacketSize; 1 + (MAX_ROOT_PORTS / 8) */
	0xff        /*  __u8  ep_bInterval; (255ms -- usb 2.0 spec) */
};

static const u8 hs_rh_config_descriptor [] = {

	/* one configuration */
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19, 0x00, /*  __u16 wTotalLength; */
	0x01,       /*  __u8  bNumInterfaces; (1) */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0x40,       /*  __u8  bmAttributes; 
				 Bit 7: Bus-powered,
				     6: Self-powered,
				     5 Remote-wakwup,
				     4..0: resvd */
	0x00,       /*  __u8  MaxPower; */
      
	/* USB 1.1:
	 * USB 2.0, single TT organization (mandatory):
	 *	one interface, protocol 0
	 *
	 * USB 2.0, multiple TT organization (optional):
	 *	two interfaces, protocols 1 (like single TT)
	 *	and 2 (multiple TT mode) ... config is
	 *	sometimes settable
	 *	NOT IMPLEMENTED
	 */

	/* one interface */
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; [usb1.1 or single tt] */
	0x00,       /*  __u8  if_iInterface; */
     
	/* one endpoint (status change endpoint) */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
 	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
 	0x02, 0x00, /*  __u16 ep_wMaxPacketSize; 1 + (MAX_ROOT_PORTS / 8) */
	0x0c        /*  __u8  ep_bInterval; (256ms -- usb 2.0 spec) */
};

/*-------------------------------------------------------------------------*/
//ZOT716u2
cyg_interrupt USB_EHCI_interrupt;
cyg_handle_t  USB_EHCI_interrupt_handle;
//ZOT716u2
cyg_uint32 ehci_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	cyg_interrupt_mask(vector);
	cyg_interrupt_acknowledge(vector);
	return(CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);	
}
/*
 * NEC D720101 Configuration EXT Register Setup
 * Clock_sel : System clock is 48-MHz Oscillator
 * PCI Slot : 2
 */
#ifdef NEC_D720101
void nec_ext_setup(void)
{
	//OHCI #1
	(*(volatile unsigned int *)(0xD00010E4) = (0x20));
	//EHCI
	(*(volatile unsigned int *)(0xD00012E4) = (0x20));
}
#endif

/*
 * helper routine for returning string descriptors in UTF-16LE
 * input can actually be ISO-8859-1; ASCII is its 7-bit subset
 */
static int ascii2utf (char *s, u8 *utf, int utfmax)
{
	int retval;

	for (retval = 0; *s && utfmax > 1; utfmax -= 2, retval += 2) {
		*utf++ = *s++;
		*utf++ = 0;
	}
	return retval;
}

/*
 * rh_string - provides manufacturer, product and serial strings for root hub
 * @id: the string ID number (1: serial number, 2: product, 3: vendor)
 * @pci_desc: PCI device descriptor for the relevant HC
 * @type: string describing our driver 
 * @data: return packet in UTF-16 LE
 * @len: length of the return packet
 *
 * Produces either a manufacturer, product or serial number string for the
 * virtual root hub device.
 */
static int rh_string (
	int		id,
	struct usb_hcd	*hcd,
	u8		*data,
	int		len
) {
	char buf [100];

	// language ids
	if (id == 0) {
		*data++ = 4; *data++ = 3;	/* 4 bytes string data */
		*data++ = 0; *data++ = 0;	/* some language id */
		return 4;

	// serial number
	} else if (id == 1) {
		strcpy (buf, hcd->bus_name);

	// product description
	} else if (id == 2) {
                strcpy (buf, hcd->product_desc);

 	// id 3 == vendor description
	} else if (id == 3) {
#ifdef USB2_DEBUG	//635u2
                sprintf (buf, "%s %s %s", UTS_SYSNAME, UTS_RELEASE,
			hcd->description);
#endif
	// unsupported IDs --> "protocol stall"
	} else
	    return 0;

	data [0] = 2 + ascii2utf (buf, data + 2, len - 2);
	data [1] = 3;	/* type == string */
	return data [0];
}


/* Root hub control transfers execute synchronously */
static int rh_call_control (struct usb_hcd *hcd, struct urb *urb)
{
//original Source	//635u2
//	struct usb_ctrlrequest *cmd = (struct usb_ctrlrequest *) urb->setup_packet;
	devrequest * cmd = (devrequest *) urb->setup_packet;
 	u16		typeReq, wValue, wIndex, wLength;
	const u8	*bufp = 0;
	u8		*ubuf = urb->transfer_buffer;
	int		len = 0;

//original Source	//635u2
//	typeReq  = (cmd->bRequestType << 8) | cmd->bRequest;
//	wValue   = le16_to_cpu (cmd->wValue);
//	wIndex   = le16_to_cpu (cmd->wIndex);
//	wLength  = le16_to_cpu (cmd->wLength);

	typeReq  = (cmd->requesttype << 8) | cmd->request;
	wValue        = le16_to_cpu (cmd->value);
	wIndex        = le16_to_cpu (cmd->index);
	wLength       = le16_to_cpu (cmd->length);
	
	if (wLength > urb->transfer_buffer_length)
		goto error;

	/* set up for success */
	urb->status = 0;
	urb->actual_length = wLength;
	switch (typeReq) {

	/* DEVICE REQUESTS */

	case DeviceRequest | USB_REQ_GET_STATUS:
		// DEVICE_REMOTE_WAKEUP
		ubuf [0] = 1; // selfpowered
		ubuf [1] = 0;
			/* FALLTHROUGH */
	case DeviceOutRequest | USB_REQ_CLEAR_FEATURE:
	case DeviceOutRequest | USB_REQ_SET_FEATURE:
#ifdef USB2_DEBUG	//635u2
		dbg ("no device features yet yet");
#endif
		break;
	case DeviceRequest | USB_REQ_GET_CONFIGURATION:
		ubuf [0] = 1;
			/* FALLTHROUGH */
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		break;
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (wValue & 0xff00) {
		case USB_DT_DEVICE << 8:
			if (hcd->driver->flags & HCD_USB2)
				bufp = usb2_rh_dev_descriptor;
			else if (hcd->driver->flags & HCD_USB11)
				bufp = usb11_rh_dev_descriptor;
			else
				goto error;
			len = 18;
			break;
		case USB_DT_CONFIG << 8:
			if (hcd->driver->flags & HCD_USB2) {
				bufp = hs_rh_config_descriptor;
				len = sizeof hs_rh_config_descriptor;
			} else {
				bufp = fs_rh_config_descriptor;
				len = sizeof fs_rh_config_descriptor;
			}
			break;
		case USB_DT_STRING << 8:
			urb->actual_length = rh_string (
				wValue & 0xff, hcd,
				ubuf, wLength);
			break;
		default:
			goto error;
		}
		break;
	case DeviceRequest | USB_REQ_GET_INTERFACE:
		ubuf [0] = 0;
			/* FALLTHROUGH */
	case DeviceOutRequest | USB_REQ_SET_INTERFACE:
		break;
	case DeviceOutRequest | USB_REQ_SET_ADDRESS:
		// wValue == urb->dev->devaddr
#ifdef USB2_DEBUG	//635u2
		dbg ("%s root hub device address %d",
			hcd->bus_name, wValue);
#endif
		break;

	/* INTERFACE REQUESTS (no defined feature/status flags) */

	/* ENDPOINT REQUESTS */

	case EndpointRequest | USB_REQ_GET_STATUS:
		// ENDPOINT_HALT flag
		ubuf [0] = 0;
		ubuf [1] = 0;
			/* FALLTHROUGH */
	case EndpointOutRequest | USB_REQ_CLEAR_FEATURE:
	case EndpointOutRequest | USB_REQ_SET_FEATURE:
#ifdef USB2_DEBUG	//635u2
		dbg ("no endpoint features yet");
#endif
		break;

	/* CLASS REQUESTS (and errors) */

	default:
		/* non-generic request */
		urb->status = hcd->driver->hub_control (hcd,
			typeReq, wValue, wIndex,
			ubuf, wLength);
		break;
error:
		/* "protocol stall" on error */
		urb->status = -EPIPE;
#ifdef USB2_DEBUG	//635u2
		dbg ("unsupported hub control message (maxchild %d)",
				urb->dev->maxchild);
#endif
	}
	if (urb->status) {
		urb->actual_length = 0;
#ifdef USB2_DEBUG	//635u2
		dbg ("CTRL: TypeReq=0x%x val=0x%x idx=0x%x len=%d ==> %d",
			typeReq, wValue, wIndex, wLength, urb->status);
#endif	
	}
	if (bufp) {
		if (urb->transfer_buffer_length < len)
			len = urb->transfer_buffer_length;
		urb->actual_length = len;
		// always USB_DIR_IN, toward host
		memcpy (ubuf, bufp, len);
	}

	/* any errors get returned through the urb completion */
	usb_hcd_giveback_urb (hcd, urb);
	return 0;
}

/*-------------------------------------------------------------------------*/

/*
 * Root Hub interrupt transfers are synthesized with a timer.
 * Completions are called in_interrupt() but not in_irq().
 */
static int rh_status_urb (struct usb_hcd *hcd, struct urb *urb) 
{
	int	len = 1 + (urb->dev->maxchild / 8);
	int interval = 255;	//ZOT716u2

	/* rh_timer protected by hcd_data_lock */
//original Source	//635u2	
//	if (timer_pending (&hcd->rh_timer)
//			|| urb->status != -EINPROGRESS
//			|| !HCD_IS_RUNNING (hcd->state)
//			|| urb->transfer_buffer_length < len) {
#ifdef USB2_DEBUG	//635u2
		dbg ("not queuing status urb, stat %d", urb->status);
#endif
//		return -EINVAL;
//	}

	urb->hcpriv = hcd;	/* nonzero to indicate it's queued */
	

//635u2	init_timer (&hcd->rh_timer);
//635u2	hcd->rh_timer.function = rh_report_status;
//635u2	hcd->rh_timer.data = (unsigned long) urb;
	/* USB 2.0 spec says 256msec; this is close enough */
//635u2	hcd->rh_timer.expires = jiffies + HZ/4;
//635u2	add_timer (&hcd->rh_timer);

	/******Setup RootHub detect timer function******/
//os	stop_timer( &hcd->rh_timer );
//os	hcd->rh_timer.func = rh_report_status;
//os	hcd->rh_timer.arg = (void *)urb;
//os	set_timer( &hcd->rh_timer, 20 );
//os	start_timer( &hcd->rh_timer );

    /* Attach the timer to the real-time clock */    
    RH_SysClk = cyg_real_time_clock();
    
    GLOBAL_EHCI_RH_URB = urb;

    cyg_clock_to_counter(RH_SysClk, &RH_Counter);

    cyg_alarm_create(RH_Counter, (cyg_alarm_t *)rh_report_status,
                     (cyg_addrword_t) urb,
                     &RH_Alarm, &RH_timerAlarm);

    /* This creates a periodic timer */
    cyg_alarm_initialize(RH_Alarm, cyg_current_time() + interval, 0); //every setting trigger once
	
	return 0;
}

//os static void rh_report_status (unsigned long ptr)
void rh_report_status (cyg_handle_t handle, cyg_addrword_t ptr)
{
	struct urb	*urb;
	struct usb_hcd	*hcd;
	int		length;
	unsigned long	flags;
	u32 temp32;

	urb = (struct urb *) ptr;
	cyg_alarm_delete(RH_Alarm);
	cyg_clock_delete(RH_SysClk);
	cyg_counter_delete(RH_Counter);
	
	spin_lock_irqsave (&urb->lock, &flags);	//ZOT==> spin_lock_irqsave (&urb->lock, flags);
#if 0	//ZOT716u2
	/* Ron Add, for monitor GPIO11 6/7/2004*/
	if ((*(volatile unsigned long *)(0x8000d010) & 0x0800) ==0){ //GPIO11 still Pull low 
		ApEvtMsg_t *dummymsg;
		static int EHC_errcounter =0;
		
		hcd_irq(dummymsg);
		EHC_errcounter++;	
	}
#endif //ZOT716u2	
	if (!urb->dev) {
		spin_unlock_irqrestore (&urb->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&urb->lock, flags);
		return;
	}

	hcd = urb->dev->bus->hcpriv;
#if 0	//ZOT716u2
	//________________###test###_____________________//
	//Read Command
	temp32 = inl2((u32)hcd->regs + 0x00);
	//Read Status
	temp32 = inl2((u32)hcd->regs + 0x04);
	//Read Interrupt
	temp32 = inl2((u32)hcd->regs + 0x08);
	//Read Frame Index
	temp32 = inl2((u32)hcd->regs + 0x0C);
	//Read Port
	temp32 = inl2((u32)hcd->regs + 0x48);
	//_______________________________________________//		
#endif	//ZOT716u2	
	
	if (urb->status == -EINPROGRESS) {
		if (HCD_IS_RUNNING (hcd->state)) {
			length = hcd->driver->hub_status_data (hcd,	urb->transfer_buffer);
			
			spin_unlock_irqrestore (&urb->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&urb->lock, flags);
			if (length > 0) {
				urb->actual_length = length;
				urb->status = 0;
				urb->complete (urb);
			}
			spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
			
			urb->status = -EINPROGRESS;
			if (HCD_IS_RUNNING (hcd->state)&& rh_status_urb (hcd, urb) != 0) {
				/* another driver snuck in? */
#ifdef USB2_DEBUG	//635u2
				dbg ("%s, can't resubmit roothub status urb?",
					hcd->bus_name);
#endif
				spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);
#ifdef USB2_DEBUG	//635u2
				BUG ();
#endif
			}
			spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);
		} else
			spin_unlock_irqrestore (&urb->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&urb->lock, flags);
	} else {
		/* this urb's been unlinked */
		urb->hcpriv = 0;
		spin_unlock_irqrestore (&urb->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&urb->lock, flags);

		usb_hcd_giveback_urb (hcd, urb);
	}
}

/*-------------------------------------------------------------------------*/

static int rh_urb_enqueue (struct usb_hcd *hcd, struct urb *urb)
{
	if (usb_pipeint (urb->pipe)) {
		int		retval;
		unsigned long	flags;

		spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
		retval = rh_status_urb (hcd, urb);
//		armond_printf("EHCI start RH Detect\n");
//_____________________________________________________________		
//ZOT716u2		RH_OK = 1;
//_____________________________________________________________		
		spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);
		return retval;
	}
	if (usb_pipecontrol (urb->pipe)){
//_____________________________________________________________			
//ZOT716u2		if(RH_OK)
//ZOT716u2			RH_NEXT = 1;
//_____________________________________________________________				
//		armond_printf("EHCI RH submit\n");	
		return rh_call_control (hcd, urb);
	}else
		return -EINVAL;
}

/*-------------------------------------------------------------------------*/

static void rh_status_dequeue (struct usb_hcd *hcd, struct urb *urb)
{
	unsigned long	flags;

	spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
//original Source	//635u2
//	del_timer_sync (&hcd->rh_timer);
//original Source	//635u2
//	hcd->rh_timer.data = 0;
//os	hcd->rh_timer.arg = 0;
	spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);

	/* we rely on RH callback code not unlinking its URB! */
	usb_hcd_giveback_urb (hcd, urb);
}

/*-------------------------------------------------------------------------*/



/* PCI-based HCs are normal, but custom bus glue should be ok */

//original Source	//635u2	
//static void hcd_irq (int irq, void *__hcd, struct pt_regs *r);
//os void hcd_irq();
//ZOT716u2 void hcd_irq(ApEvtMsg_t *msg);
static void hc_died (struct usb_hcd *hcd);

/*-------------------------------------------------------------------------*/

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * usb_hcd_pci_probe - initialize PCI-based HCDs
 * @dev: USB Host Controller being probed
 * @id: pci hotplug id connecting controller to HCD framework
 * Context: !in_interrupt()
 *
 * Allocates basic PCI resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 * Store this function in the HCD's struct pci_driver as probe().
 */
void ehci_dsr(void);	//ZOT716u2 
struct usb_hcd		*GLOBAL_HCD;
struct hc_driver	*GLOBAL_DRIVER;
 
int usb_hcd_pci_probe (unsigned int sz, unsigned int mem_addr)
{
	struct hc_driver	*driver;
	unsigned long		resource, len;
	void			*base;
	struct usb_bus		*bus;
	struct usb_hcd		*hcd;
	int			retval, region;
	char			buf [8], *bufp = buf;
	struct ehci_hcd		*ehci;	//635u2 for interrupt
	uint32 val;

//os	if (!id || !(driver = (struct hc_driver *) id->driver_data))
//os		return -EINVAL;
		driver = GLOBAL_DRIVER;	//eCos
//os	if (pci_enable_device (dev) < 0)
//os		return -ENODEV;
	
//os        if (!dev->irq) {
#ifdef USB2_DEBUG	//635u2
        	err ("Found HC with no IRQ.  Check BIOS/PCI %s setup!",
			dev->slot_name);
#endif
//os   	        return -ENODEV;
//os        }
	
//os	if (driver->flags & HCD_MEMORY) {	// EHCI, OHCI
		region = 0;
//os		resource = pci_resource_start (dev, 0);
//os		len = pci_resource_len (dev, 0);
//os		if (!request_mem_region (resource, len, driver->description)) {
#ifdef USB2_DEBUG	//635u2
			dbg ("controller already in use");
#endif
//os			return -EBUSY;
//os		}
//original Source	//635u2	
//os		base = ioremap_nocache (resource, len);
//os		base = ioremap (resource, len);	//map to PCI memory space

		base = mem_addr;	//eCos
		len = sz;			//eCos

		if (base == NULL) {
#ifdef USB2_DEBUG	//635u2
			dbg ("error mapping memory");
#endif
			retval = -EFAULT;
clean_1:
//os			release_mem_region (resource, len);
#ifdef USB2_DEBUG	//635u2
			err ("init %s fail, %d", dev->slot_name, retval);
#endif			
			return retval;
		}

//os	} 
#if 0
	else { 				// UHCI
		resource = len = 0;
		for (region = 0; region < PCI_ROM_RESOURCE; region++) {
			if (!(pci_resource_flags (dev, region) & IORESOURCE_IO))
				continue;

			resource = pci_resource_start (dev, region);
			len = pci_resource_len (dev, region);
			if (request_region (resource, len,
					driver->description))
				break;
		}
		if (region == PCI_ROM_RESOURCE) {
#ifdef USB2_DEBUG	//635u2
			dbg ("no i/o regions available");
#endif
			return -EBUSY;
		}
		base = (void *) resource;
	}
#endif
	
	// driver->start(), later on, will transfer device from
	// control by SMM/BIOS to control by Linux (if needed)

//os	pci_set_master (dev);
	
//	armond_printf("EHCI Probe\n");
	
	//set master and Memory space to enable EHCI	//eCos
//ZOT716u2	val = pciHost_ConfigRead(0x4,2);		
//ZOT716u2	pciHost_ConfigWrite(0x04, val|0x6,2);

	hcd = driver->hcd_alloc ();
	if (hcd == NULL){
#ifdef USB2_DEBUG	//635u2
		dbg ("hcd alloc fail");
#endif
		retval = -ENOMEM;
clean_2:
		if (driver->flags & HCD_MEMORY) {
//original Source	//635u2	
//			iounmap (base);
			goto clean_1;
		} else {
//os			release_region (resource, len);
#ifdef USB2_DEBUG	//635u2
			err ("init %s fail, %d", dev->slot_name, retval);
#endif
			return retval;
		}
	}
//os	pci_set_drvdata(dev, hcd);
	GLOBAL_HCD = hcd;				//eCos
	hcd->driver = driver;
	hcd->description = driver->description;
//os	hcd->pdev = dev;			//eCos
#ifdef USB2_DEBUG	//635u2	
	info ("%s @ %s, %s", hcd->description,  dev->slot_name, dev->name);
#endif

#ifndef __sparc__
//	sprintf (buf, "%d", dev->irq);	//635u2
#else
	bufp = __irq_itoa(dev->irq);
#endif
/*Request Interrupt*/	//635u2
//original Source	//635u2	
//	if (request_irq (dev->irq, hcd_irq, SA_SHIRQ, hcd->description, hcd) != 0) 
	if (0)	//635u2
	{
#ifdef USB2_DEBUG	//635u2
		err ("request interrupt %s failed", bufp);
#endif
		retval = -EBUSY;
clean_3:
		driver->hcd_free (hcd);
		goto clean_2;
	}
//os	hcd->irq = dev->irq;

//ZOT716u2
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_USB20_BIT_INDEX);
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_USB20_BIT_INDEX);
	
    cyg_interrupt_create(INTC_USB20_BIT_INDEX,
                             0,                     
                             0,                     
                             &ehci_isr,
                             &ehci_dsr,
                             &USB_EHCI_interrupt_handle,
                             &USB_EHCI_interrupt);
    cyg_interrupt_attach(USB_EHCI_interrupt_handle); 

	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_USB20_BIT_INDEX);

	hcd->irq = 0; //GPIO do			//eCos

	hcd->regs = base;
	hcd->region = region;
#ifdef USB2_DEBUG	//635u2
	info ("irq %s, %s %p", bufp,
		(driver->flags & HCD_MEMORY) ? "pci mem" : "io base",
		base);
#endif

// FIXME simpler: make "bus" be that data, not pointer to it.
	bus = usb_alloc_bus (&hcd_operations);
	if (bus == NULL) {
#ifdef USB2_DEBUG	//635u2
		dbg ("usb_alloc_bus fail");
#endif
		retval = -ENOMEM;
//original Source	//635u2	
//		free_irq (dev->irq, hcd);
		goto clean_3;
	}
	hcd->bus = bus;
//os	hcd->bus_name = dev->slot_name;		/* prefer bus->bus_name */
//os	bus->bus_name = dev->slot_name;
//os	hcd->product_desc = dev->name;
	bus->hcpriv = (void *) hcd;

	INIT_LIST_HEAD (&hcd->dev_list);
	INIT_LIST_HEAD (&hcd->hcd_list);

	down (&hcd_list_lock);
	list_add (&hcd->hcd_list, &hcd_list);
	up (&hcd_list_lock);

	usb_register_bus (bus);

	if ((retval = driver->start (hcd)) < 0)
		usb_hcd_pci_remove ();
	
	//request interrupt		//635u2
//os	ehci = hcd_to_ehci (hcd);
//os	pci_connect_interface("ehci_irq",hcd_irq,hcd,&ehci->regs->status);	//635u2	for interrupt
	
	return retval;
}
#ifdef EXPORT_L_SYMBOL	//635u2
EXPORT_SYMBOL (usb_hcd_pci_probe);
#endif


/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_hcd_pci_remove - shutdown processing for PCI-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_hcd_pci_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 * Store this function in the HCD's struct pci_driver as remove().
 */
void usb_hcd_pci_remove ()
{
	struct usb_hcd		*hcd;
	struct usb_device	*hub;

//	hcd = pci_get_drvdata(dev);
	hcd = GLOBAL_HCD;		//eCos
	if (!hcd)
		return;
#ifdef USB2_DEBUG	//635u2
	info ("remove: %s, state %x", hcd->bus_name, hcd->state);

	if (in_interrupt ()) BUG ();
#endif
	hub = hcd->bus->root_hub;
	hcd->state = USB_STATE_QUIESCING;

#ifdef USB2_DEBUG	//635u2
	dbg ("%s: roothub graceful disconnect", hcd->bus_name);
#endif
	usb_disconnect (&hub);
	// usb_disconnect (&hcd->bus->root_hub);

	hcd->driver->stop (hcd);
	hcd->state = USB_STATE_HALT;

//original Source	//635u2	
//	free_irq (hcd->irq, hcd);
//os	if (hcd->driver->flags & HCD_MEMORY) {
//original Source	//635u2	
//		iounmap (hcd->regs);
//os		release_mem_region (pci_resource_start (dev, 0),pci_resource_len (dev, 0));
//os	} else {
//os		release_region (pci_resource_start (dev, hcd->region),pci_resource_len (dev, hcd->region));
//os	}

	down (&hcd_list_lock);
	list_del (&hcd->hcd_list);
	up (&hcd_list_lock);

	usb_deregister_bus (hcd->bus);
	usb_free_bus (hcd->bus);
	hcd->bus = NULL;

	hcd->driver->hcd_free (hcd);
}
#ifdef EXPORT_L_SYMBOL	//635u2
EXPORT_SYMBOL (usb_hcd_pci_remove);
#endif


#ifdef	CONFIG_PM

/*
 * Some "sleep" power levels imply updating struct usb_driver
 * to include a callback asking hcds to do their bit by checking
 * if all the drivers can suspend.  Gets involved with remote wakeup.
 *
 * If there are pending urbs, then HCs will need to access memory,
 * causing extra power drain.  New sleep()/wakeup() PM calls might
 * be needed, beyond PCI suspend()/resume().  The root hub timer
 * still be accessing memory though ...
 *
 * FIXME:  USB should have some power budgeting support working with
 * all kinds of hubs.
 *
 * FIXME:  This assumes only D0->D3 suspend and D3->D0 resume.
 * D1 and D2 states should do something, yes?
 *
 * FIXME:  Should provide generic enable_wake(), calling pci_enable_wake()
 * for all supported states, so that USB remote wakeup can work for any
 * devices that support it (and are connected via powered hubs).
 *
 * FIXME:  resume doesn't seem to work right any more...
 */


// 2.4 kernels have issued concurrent resumes (w/APM)
// we defend against that error; PCI doesn't yet.

/**
 * usb_hcd_pci_suspend - power management suspend of a PCI-based HCD
 * @dev: USB Host Controller being suspended
 *
 * Store this function in the HCD's struct pci_driver as suspend().
 */
int usb_hcd_pci_suspend (struct pci_dev *dev, u32 state)
{
	struct usb_hcd		*hcd;
	int			retval;

	hcd = pci_get_drvdata(dev);
#ifdef USB2_DEBUG	//635u2	
	info ("suspend %s to state %d", hcd->bus_name, state);
#endif
	pci_save_state (dev, hcd->pci_state);

	// FIXME for all connected devices, leaf-to-root:
	// driver->suspend()
	// proposed "new 2.5 driver model" will automate that

	/* driver may want to disable DMA etc */
	retval = hcd->driver->suspend (hcd, state);
	hcd->state = USB_STATE_SUSPENDED;

 	pci_set_power_state (dev, state);
	return retval;
}
#ifdef EXPORT_L_SYMBOL	//635u2
EXPORT_SYMBOL (usb_hcd_pci_suspend);
#endif
/**
 * usb_hcd_pci_resume - power management resume of a PCI-based HCD
 * @dev: USB Host Controller being resumed
 *
 * Store this function in the HCD's struct pci_driver as resume().
 */
int usb_hcd_pci_resume (struct pci_dev *dev)
{
	struct usb_hcd		*hcd;
	int			retval;

	hcd = pci_get_drvdata(dev);
#ifdef USB2_DEBUG	//635u2	
	info ("resume %s", hcd->bus_name);
#endif
	/* guard against multiple resumes (APM bug?) */
	atomic_inc (&hcd->resume_count);
	if (atomic_read (&hcd->resume_count) != 1) {
#ifdef USB2_DEBUG	//635u2
		err ("concurrent PCI resumes for %s", hcd->bus_name);
#endif
		retval = 0;
		goto done;
	}

	retval = -EBUSY;
	if (hcd->state != USB_STATE_SUSPENDED) {
#ifdef USB2_DEBUG	//635u2
		dbg ("can't resume, not suspended!");
#endif
		goto done;
	}
	hcd->state = USB_STATE_RESUMING;

	pci_set_power_state (dev, 0);
	pci_restore_state (dev, hcd->pci_state);

	retval = hcd->driver->resume (hcd);
	if (!HCD_IS_RUNNING (hcd->state)) {
#ifdef USB2_DEBUG	//635u2
		dbg ("resume %s failure, retval %d", hcd->bus_name, retval);
#endif
		hc_died (hcd);
// FIXME:  recover, reset etc.
	} else {
		// FIXME for all connected devices, root-to-leaf:
		// driver->resume ();
		// proposed "new 2.5 driver model" will automate that
	}

done:
	atomic_dec (&hcd->resume_count);
	return retval;
}
#ifdef EXPORT_L_SYMBOL	//635u2
EXPORT_SYMBOL (usb_hcd_pci_resume);
#endif

#endif	/* CONFIG_PM */



/*-------------------------------------------------------------------------*/

/*
 * Generic HC operations.
 */

/*-------------------------------------------------------------------------*/

/* called from khubd, or root hub init threads for hcd-private init */
static int hcd_alloc_dev (struct usb_device *udev)
{
	struct hcd_dev		*dev;
	struct usb_hcd		*hcd;
	unsigned long		flags;

	if (!udev || udev->hcpriv)
		return -EINVAL;
	if (!udev->bus || !udev->bus->hcpriv)
		return -ENODEV;
	hcd = udev->bus->hcpriv;
	if (hcd->state == USB_STATE_QUIESCING)
		return -ENOLINK;

	dev = (struct hcd_dev *) kmalloc (sizeof *dev, GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;
	memset (dev, 0, sizeof *dev);

	INIT_LIST_HEAD (&dev->dev_list);
	INIT_LIST_HEAD (&dev->urb_list);

	spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
	list_add (&dev->dev_list, &hcd->dev_list);
	// refcount is implicit
	udev->hcpriv = dev;
	udev->speed = USB_SPEED_HIGH;	//635u2
	spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);

	return 0;
}

/*-------------------------------------------------------------------------*/

static void hc_died (struct usb_hcd *hcd)
{
	struct list_head	*devlist, *urblist;
	struct hcd_dev		*dev;
	struct urb		*urb;
	unsigned long		flags;
	
	/* flag every pending urb as done */
	spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
	list_for_each (devlist, &hcd->dev_list) {
		dev = list_entry (devlist, struct hcd_dev, dev_list);
		list_for_each (urblist, &dev->urb_list) {
			urb = list_entry (urblist, struct urb, urb_list);
#ifdef USB2_DEBUG	//635u2
			dbg ("shutdown %s urb %p pipe %x, current status %d",
				hcd->bus_name, urb, urb->pipe, urb->status);
#endif
			if (urb->status == -EINPROGRESS)
				urb->status = -ESHUTDOWN;
		}
	}
//original Source	//635u2	
//	urb = (struct urb *) hcd->rh_timer.data;
//os	urb = (struct urb *) hcd->rh_timer.arg;
	urb = GLOBAL_EHCI_RH_URB;	//719AW
	if (urb)
		urb->status = -ESHUTDOWN;
	spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);

	if (urb)
		rh_status_dequeue (hcd, urb);
	hcd->driver->stop (hcd);
}

/*-------------------------------------------------------------------------*/

static void urb_unlink (struct urb *urb)
{
	unsigned long		flags;
	struct usb_device	*dev;

	/* Release any periodic transfer bandwidth */
	if (urb->bandwidth)
		usb_release_bandwidth (urb->dev, urb,
			usb_pipeisoc (urb->pipe));

	/* clear all state linking urb to this dev (and hcd) */

	spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
	list_del_init (&urb->urb_list);
	dev = urb->dev;
	urb->dev = NULL;
	usb_dec_dev_use (dev);
	spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);
}


/* may be called in any context with a valid urb->dev usecount */
/* caller surrenders "ownership" of urb */

static int hcd_submit_urb (struct urb *urb)
{
	int			status;
	struct usb_hcd		*hcd;
	struct hcd_dev		*dev;
	unsigned long		flags;
	int			pipe, temp, max;
	int			mem_flags;

	if (!urb || urb->hcpriv || !urb->complete)
		return -EINVAL;

//	armond_printf("EHCI Set URB status:EINPROGRESS\n");
	urb->status = -EINPROGRESS;
	urb->actual_length = 0;
	urb->bandwidth = 0;
	INIT_LIST_HEAD (&urb->urb_list);

	if (!urb->dev || !urb->dev->bus || urb->dev->devnum <= 0){
//		armond_printf("EHCI Error:ENODEV\n");
		return -ENODEV;
	}
	hcd = urb->dev->bus->hcpriv;
	dev = urb->dev->hcpriv;
	if (!hcd || !dev){
//		armond_printf("EHCI Error:ENODEV\n");
		return -ENODEV;
	}

	/* can't submit new urbs when quiescing, halted, ... */
	if (hcd->state == USB_STATE_QUIESCING || !HCD_IS_RUNNING (hcd->state)){
//		armond_printf("EHCI Error:ESHUTDOWN\n");
		return -ESHUTDOWN;
	}
	
	pipe = urb->pipe;
	temp = usb_pipetype (urb->pipe);
	if (usb_endpoint_halted (urb->dev, usb_pipeendpoint (pipe),
			usb_pipeout (pipe))){
//		armond_printf("EHCI Error:EPIPE\n");
		return -EPIPE;
	}
//	armond_printf("EHCI submit urb\n");

	/* NOTE: 2.5 passes this value explicitly in submit() */
	mem_flags = in_interrupt () ? GFP_ATOMIC : GFP_KERNEL;

	/* FIXME there should be a sharable lock protecting us against
	 * config/altsetting changes and disconnects, kicking in here.
	 */

	/* Sanity check, so HCDs can rely on clean data */
	max = usb_maxpacket (urb->dev, pipe, usb_pipeout (pipe));
	if (max <= 0) {
		err ("bogus endpoint (bad maxpacket)");
		return -EINVAL;
	}

	/* "high bandwidth" mode, 1-3 packets/uframe? */
	if (urb->dev->speed == USB_SPEED_HIGH) {
		int	mult;
		switch (temp) {
		case PIPE_ISOCHRONOUS:
		case PIPE_INTERRUPT:
			mult = 1 + ((max >> 11) & 0x03);
			max &= 0x03ff;
			max *= mult;
		}
	}

	/* periodic transfers limit size per frame/uframe */
	switch (temp) {
	case PIPE_ISOCHRONOUS: {
		int	n, len;

		if (urb->number_of_packets <= 0)		    
			return -EINVAL;
		for (n = 0; n < urb->number_of_packets; n++) {
			len = urb->iso_frame_desc [n].length;
			if (len < 0 || len > max) 
				return -EINVAL;
		}

		}
		break;
	case PIPE_INTERRUPT:
		if (urb->transfer_buffer_length > max)
			return -EINVAL;
	}

	/* the I/O buffer must usually be mapped/unmapped */
	if (urb->transfer_buffer_length < 0)
		return -EINVAL;
	

	// FIXME set urb->transfer_dma and/or setup_dma 

	if (urb->next) {
#ifdef USB2_DEBUG	//635u2
		warn ("use explicit queuing not urb->next");
#endif
		return -EINVAL;
	}

#ifdef DEBUG
	/* stuff that drivers shouldn't do, but which shouldn't
	 * cause problems in HCDs if they get it wrong.
	 */
	{
	unsigned int	orig_flags = urb->transfer_flags;
	unsigned int	allowed;

	/* enforce simple/standard policy */
	allowed = USB_ASYNC_UNLINK;	// affects later unlinks
	allowed |= USB_NO_FSBR;		// only affects UHCI
	switch (temp) {
	case PIPE_CONTROL:
		allowed |= USB_DISABLE_SPD;
		break;
	case PIPE_BULK:
		allowed |= USB_DISABLE_SPD | USB_QUEUE_BULK
				| USB_ZERO_PACKET | URB_NO_INTERRUPT;
		break;
	case PIPE_INTERRUPT:
		allowed |= USB_DISABLE_SPD;
		break;
	case PIPE_ISOCHRONOUS:
		allowed |= USB_ISO_ASAP;
		break;
	}
	urb->transfer_flags &= allowed;

	/* fail if submitter gave bogus flags */
	if (urb->transfer_flags != orig_flags) {
#ifdef USB2_DEBUG	//635u2
		err ("BOGUS urb flags, %x --> %x",
			orig_flags, urb->transfer_flags);
#endif
		return -EINVAL;
	}
	}
#endif
	/*
	 * Force periodic transfer intervals to be legal values that are
	 * a power of two (so HCDs don't need to).
	 *
	 * FIXME want bus->{intr,iso}_sched_horizon values here.  Each HC
	 * supports different values... this uses EHCI/UHCI defaults (and
	 * EHCI can use smaller non-default values).
	 */
	switch (temp) {
	case PIPE_ISOCHRONOUS:
	case PIPE_INTERRUPT:
		/* too small? */
		if (urb->interval <= 0)
			return -EINVAL;
		/* too big? */
		switch (urb->dev->speed) {
		case USB_SPEED_HIGH:	/* units are microframes */
			// NOTE usb handles 2^15
			if (urb->interval > (1024 * 8))
				urb->interval = 1024 * 8;
			temp = 1024 * 8;
			break;
		case USB_SPEED_FULL:	/* units are frames/msec */
		case USB_SPEED_LOW:
			if (temp == PIPE_INTERRUPT) {
				if (urb->interval > 255)
					return -EINVAL;
				// NOTE ohci only handles up to 32
				temp = 128;
			} else {
				if (urb->interval > 1024)
					urb->interval = 1024;
				// NOTE usb and ohci handle up to 2^15
				temp = 1024;
			}
			break;
		default:
			return -EINVAL;
		}
		/* power of two? */
		while (temp > urb->interval)
			temp >>= 1;
		urb->interval = temp;
	}


	/*
	 * FIXME:  make urb timeouts be generic, keeping the HCD cores
	 * as simple as possible.
	 */

	// NOTE:  a generic device/urb monitoring hook would go here.
	// hcd_monitor_hook(MONITOR_URB_SUBMIT, urb)
	// It would catch submission paths for all urbs.

	/*
	 * Atomically queue the urb,  first to our records, then to the HCD.
	 * Access to urb->status is controlled by urb->lock ... changes on
	 * i/o completion (normal or fault) or unlinking.
	 */

	// FIXME:  verify that quiescing hc works right (RH cleans up)

	spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
	if (HCD_IS_RUNNING (hcd->state) && hcd->state != USB_STATE_QUIESCING) {
		usb_inc_dev_use (urb->dev);
		list_add (&urb->urb_list, &dev->urb_list);
		status = 0;
	} else {
		INIT_LIST_HEAD (&urb->urb_list);
		status = -ESHUTDOWN;
	}
	spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);
	if (status)
		return status;

//_____________________________________________________________
//ZOT716u2	if( RH_OK )
//ZOT716u2	{
//		armond_printf("my stop EHCI RH Detect\n");
//ZOT716u2		cyg_alarm_delete(RH_Alarm);
//ZOT716u2		cyg_clock_delete(RH_SysClk);
//ZOT716u2		cyg_counter_delete(RH_Counter);
//ZOT716u2	}
//_____________________________________________________________


	if (urb->dev == hcd->bus->root_hub)
		status = rh_urb_enqueue (hcd, urb);
	else
		status = hcd->driver->urb_enqueue (hcd, urb, mem_flags);
	/* urb->dev got nulled if hcd called giveback for us
	 * NOTE: ref to urb->dev is a race without (2.5) refcounting,
	 * unless driver only returns status when it didn't giveback 
	 */
	if (status && urb->dev)
		urb_unlink (urb);
//_____________________________________________________________
//ZOT716u2	if(RH_OK & RH_NEXT){
//		armond_printf("my start EHCI RH Detect\n");
//ZOT716u2		rh_status_urb(hcd,GLOBAL_EHCI_RH_URB);
//ZOT716u2	}
//_____________________________________________________________	
	return status;
}

/*-------------------------------------------------------------------------*/

/* called in any context */
static int hcd_get_frame_number (struct usb_device *udev)
{
	struct usb_hcd	*hcd = (struct usb_hcd *)udev->bus->hcpriv;
	return hcd->driver->get_frame_number (hcd);
}

/*-------------------------------------------------------------------------*/

struct completion_splice {		// modified urb context:
	/* did we complete? */
	struct completion	done;

	/* original urb data */
//original Source	//635u2
//	void			(*complete)(struct urb *);
	usb_complete_t 	complete;
	void			*context;
};

static void unlink_complete (struct urb *urb)
{
	struct completion_splice	*splice;

	splice = (struct completion_splice *) urb->context;

	/* issue original completion call */
	urb->complete = splice->complete;
	urb->context = splice->context;
	urb->complete (urb);	

	/* then let the synchronous unlink call complete */
//original Source	//635u2
//	complete (&splice->done);
}

/*
 * called in any context; note ASYNC_UNLINK restrictions
 *
 * caller guarantees urb won't be recycled till both unlink()
 * and the urb's completion function return
 */
static int hcd_unlink_urb (struct urb *urb)
{
	struct hcd_dev			*dev;
	struct usb_hcd			*hcd = 0;
	unsigned long			flags;
	struct completion_splice	*splice;
	int				retval;

	if (!urb)
		return -EINVAL;

	splice = malloc (sizeof(struct completion_splice));	//eason 20100813
	
	if(splice == NULL)
		return -EINVAL;
	/*
	 * we contend for urb->status with the hcd core,
	 * which changes it while returning the urb.
	 *
	 * Caller guaranteed that the urb pointer hasn't been freed, and
	 * that it was submitted.  But as a rule it can't know whether or
	 * not it's already been unlinked ... so we respect the reversed
	 * lock sequence needed for the usb_hcd_giveback_urb() code paths
	 * (urb lock, then hcd_data_lock) in case some other CPU is now
	 * unlinking it.
	 */
	spin_lock_irqsave (&urb->lock, &flags);	//ZOT==> spin_lock_irqsave (&urb->lock, flags);
	spin_lock (&hcd_data_lock);
	if (!urb->hcpriv || urb->transfer_flags & USB_TIMEOUT_KILLED) {
		retval = -EINVAL;
		goto done;
	}

	if (!urb->dev || !urb->dev->bus) {
		retval = -ENODEV;
		goto done;
	}

	/* giveback clears dev; non-null means it's linked at this level */
	dev = urb->dev->hcpriv;
	hcd = urb->dev->bus->hcpriv;
	if (!dev || !hcd) {
		retval = -ENODEV;
		goto done;
	}

	/* For non-periodic transfers, any status except -EINPROGRESS means
	 * the HCD has already started to unlink this URB from the hardware.
	 * In that case, there's no more work to do.
	 *
	 * For periodic transfers, this is the only way to trigger unlinking
	 * from the hardware.  Since we (currently) overload urb->status to
	 * tell the driver to unlink, error status might get clobbered ...
	 * unless that transfer hasn't yet restarted.  One such case is when
	 * the URB gets unlinked from its completion handler.
	 *
	 * FIXME use an URB_UNLINKED flag to match URB_TIMEOUT_KILLED
	 */
	switch (usb_pipetype (urb->pipe)) {
	case PIPE_CONTROL:
	case PIPE_BULK:
		if (urb->status != -EINPROGRESS) {
			retval = -EINVAL;
			goto done;
		}
	}

	/* maybe set up to block on completion notification */
	if ((urb->transfer_flags & USB_TIMEOUT_KILLED))
		urb->status = -ETIMEDOUT;
	else if (!(urb->transfer_flags & USB_ASYNC_UNLINK)) {
		if (in_interrupt ()) {
#ifdef USB2_DEBUG	//635u2			
			dbg ("non-async unlink in_interrupt");
#endif
			retval = -EWOULDBLOCK;
			goto done;
		}
		/* synchronous unlink: block till we see the completion */
		init_completion (&splice->done);
		splice->complete = urb->complete;
		splice->context = urb->context;
		urb->complete = unlink_complete;
		urb->context = splice;
		urb->status = -ENOENT;
	} else {
		/* asynchronous unlink */
		urb->status = -ECONNRESET;
	}
	spin_unlock (&hcd_data_lock);
	spin_unlock_irqrestore (&urb->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&urb->lock, flags);

//original Source	//635u2	
//	if (urb == (struct urb *) hcd->rh_timer.data) {
	if (urb == (struct urb *) GLOBAL_EHCI_RH_URB) {	//719AW
		rh_status_dequeue (hcd, urb);
		retval = 0;
	} else {
		retval = hcd->driver->urb_dequeue (hcd, urb);
// FIXME:  if retval and we tried to splice, whoa!!
#ifdef USB2_DEBUG	//635u2
	if (retval && urb->status == -ENOENT) err ("whoa! retval %d", retval);
#endif
	}

    	/* block till giveback, if needed */
	if (!(urb->transfer_flags & (USB_ASYNC_UNLINK|USB_TIMEOUT_KILLED))
			&& HCD_IS_RUNNING (hcd->state)
			&& !retval) {
#ifdef USB2_DEBUG	//635u2
		dbg ("%s: wait for giveback urb %p",
			hcd->bus_name, urb);
#endif
//original Source	//635u2
		//wait_for_completion (&splice->done);
	} else if ((urb->transfer_flags & USB_ASYNC_UNLINK) && retval == 0) {
		return -EINPROGRESS;
	}
	goto bye;
done:
	spin_unlock (&hcd_data_lock);
	spin_unlock_irqrestore (&urb->lock, &flags);	//ZOT==> spin_unlock_irqrestore (&urb->lock, flags);
bye:
	if (retval)
#ifdef USB2_DEBUG	//635u2
		dbg ("%s: hcd_unlink_urb fail %d",
		    hcd ? hcd->bus_name : "(no bus?)",
		    retval);
#endif
	return retval;
}

/*-------------------------------------------------------------------------*/

/* called by khubd, rmmod, apmd, or other thread for hcd-private cleanup */

// FIXME:  likely best to have explicit per-setting (config+alt)
// setup primitives in the usbcore-to-hcd driver API, so nothing
// is implicit.  kernel 2.5 needs a bunch of config cleanup...

static int hcd_free_dev (struct usb_device *udev)
{
	struct hcd_dev		*dev;
	struct usb_hcd		*hcd;
	unsigned long		flags;

	if (!udev || !udev->hcpriv)
		return -EINVAL;

	if (!udev->bus || !udev->bus->hcpriv)
		return -ENODEV;

	// should udev->devnum == -1 ??

	dev = udev->hcpriv;
	hcd = udev->bus->hcpriv;

	/* device driver problem with refcounts? */
	if (!list_empty (&dev->urb_list)) {
#ifdef USB2_DEBUG	//635u2
		dbg ("free busy dev, %s devnum %d (bug!)",
			hcd->bus_name, udev->devnum);
#endif
		return -EINVAL;
	}

	hcd->driver->free_config (hcd, udev);	

//eason 20100818	spin_lock_irqsave (&hcd_data_lock, &flags);	//ZOT==> spin_lock_irqsave (&hcd_data_lock, flags);
	spin_lock_irqsave (&hcd_free_lock, &flags);

	list_del (&dev->dev_list);
	udev->hcpriv = NULL;
//eason 20100818	spin_unlock_irqrestore (&hcd_data_lock, &flags);//ZOT==>	spin_unlock_irqrestore (&hcd_data_lock, flags);
	spin_unlock_irqrestore (&hcd_free_lock, &flags);

	kfree (dev, 0);
	return 0;
}
//Setup for 635u2
static struct usb_operations hcd_operations = {
	hcd_alloc_dev,
	hcd_free_dev,
	hcd_get_frame_number,
	hcd_submit_urb,
	hcd_unlink_urb,
};

/*-------------------------------------------------------------------------*/

//original Source	//635u2	
//static void hcd_irq (int irq, void *__hcd, struct pt_regs *r)
//os	void hcd_irq( void *__hcd )
//ZOT716u2 void hcd_irq(ApEvtMsg_t *msg)
void hcd_irq()	//ZOT716u2
{
	struct usb_hcd		*hcd = GLOBAL_HCD;
	int			start = hcd->state;

	if (unlikely (hcd->state == USB_STATE_HALT))	/* irq sharing? */
		return;

	hcd->driver->irq (hcd);
	if (hcd->state != start && hcd->state == USB_STATE_HALT)
		hc_died (hcd);

}
//ZOT716u2
void ehci_dsr(void)
{
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_USB20_BIT_INDEX);
	hcd_irq();
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_USB20_BIT_INDEX);	
}
/*-------------------------------------------------------------------------*/

/**
 * usb_hcd_giveback_urb - return URB from HCD to device driver
 * @hcd: host controller returning the URB
 * @urb: urb being returned to the USB device driver.
 * Context: in_interrupt()
 *
 * This hands the URB from HCD to its USB device driver, using its
 * completion function.  The HCD has freed all per-urb resources
 * (and is done using urb->hcpriv).  It also released all HCD locks;
 * the device driver won't cause deadlocks if it resubmits this URB,
 * and won't confuse things by modifying and resubmitting this one.
 * Bandwidth and other resources will be deallocated.
 *
 * HCDs must not use this for periodic URBs that are still scheduled
 * and will be reissued.  They should just call their completion handlers
 * until the urb is returned to the device driver by unlinking.
 *
 * NOTE that no urb->next processing is done, even for isochronous URBs.
 * ISO streaming functionality can be achieved by having completion handlers
 * re-queue URBs.  Such explicit queuing doesn't discard error reports.
 */
void usb_hcd_giveback_urb (struct usb_hcd *hcd, struct urb *urb)
{
	urb_unlink (urb);

	// NOTE:  a generic device/urb monitoring hook would go here.
	// hcd_monitor_hook(MONITOR_URB_FINISH, urb, dev)
	// It would catch exit/unlink paths for all urbs, but non-exit
	// completions for periodic urbs need hooks inside the HCD.
	// hcd_monitor_hook(MONITOR_URB_UPDATE, urb, dev)

#ifdef USB2_DEBUG	//635u2
	if (urb->status)
		dbg ("giveback urb %p status %d len %d",
			urb, urb->status, urb->actual_length);
#endif
	// FIXME unmap urb->transfer_dma and/or setup_dma 

	/* pass ownership to the completion handler */
	urb->complete (urb);
}
#ifdef EXPORT_L_SYMBOL	//635u2
EXPORT_SYMBOL (usb_hcd_giveback_urb);
#endif


