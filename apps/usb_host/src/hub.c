/*
 * USB hub driver.
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999 Johannes Erdfelt
 * (C) Copyright 1999 Gregory P. Smith
 */

#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "usb.h"
#include "hub.h"

//ZOT==> typedef uint32					spinlock_t;

extern inline int dirps();
extern inline void restore(cyg_uint32  old_intr);
extern void wait_ms(unsigned int ms);
/* Wakes up khubd */
spinlock_t hub_event_lock;//ZOT==> static spinlock_t hub_event_lock = SPIN_LOCK_UNLOCKED;
static DECLARE_MUTEX(usb_address0_sem);

static LIST_HEAD(hub_event_list);	/* List of hubs needing servicing */
static LIST_HEAD(hub_list);		/* List containing all of the hubs (for cleanup) */

//ZOT716u2 static DECLARE_WAIT_QUEUE_HEAD(khubd_wait);
cyg_sem_t khubd_wait;	//ZOT716u2

//static int khubd_pid = 0;			/* PID of khubd */
//static DECLARE_MUTEX_LOCKED(khubd_exited);

#define MAX_USB_NUM_PORTS	8
uint32 usb_devices_per_port[MAX_USB_NUM_PORTS] = {0}; // for QC only

uint32 OHCI_LINK[MAX_USB_NUM_PORTS] = {0}; 
uint32 EHCI_LINK[MAX_USB_NUM_PORTS] = {0}; 



/* USB 2.0 spec Section 11.24.4.5 */
static int usb_get_hub_descriptor(struct usb_device *dev, void *data, int size)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		USB_DT_HUB << 8, 0, data, size, HZ);
}

/*
 * USB 2.0 spec Section 11.24.2.1
 */
static int usb_clear_hub_feature(struct usb_device *dev, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_HUB, feature, 0, NULL, 0, HZ);
}

/*
 * USB 2.0 spec Section 11.24.2.2
 * BUG: doesn't handle port indicator selector in high byte of wIndex
 */
static int usb_clear_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature, port, NULL, 0, HZ);
}

/*
 * USB 2.0 spec Section 11.24.2.13
 * BUG: doesn't handle port indicator selector in high byte of wIndex
 */
static int usb_set_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_FEATURE, USB_RT_PORT, feature, port, NULL, 0, HZ);
}

/*
 * USB 2.0 spec Section 11.24.2.6
 */
static int usb_get_hub_status(struct usb_device *dev, void *data)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
		data, sizeof(struct usb_hub_status), HZ);
}

/*
 * USB 2.0 spec Section 11.24.2.7
 */
static int usb_get_port_status(struct usb_device *dev, int port, void *data)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
		data, sizeof(struct usb_hub_status), HZ);
}

static void hub_irq(struct urb *urb)
{
	struct usb_hub *hub = (struct usb_hub *)urb->context;
	unsigned long flags;

	/* Cause a hub reset after 10 consecutive errors */
	if (urb->status) {
		if (urb->status == -ENOENT)
			return;

//{{MARK_DEBUG
//		dbg("nonzero status in irq %d", urb->status);
//}}MARK_DEBUG

		if ((++hub->nerrors < 10) || hub->error)
			return;

		hub->error = urb->status;
	}

	hub->nerrors = 0;

	/* Something happened, let khubd figure it out */
	spin_lock_irqsave(&hub_event_lock, &flags); //ZOT==> spin_lock_irqsave(&hub_event_lock, flags);
	if (list_empty(&hub->event_list)) {
		list_add(&hub->event_list, &hub_event_list);
//		wake_up(&khubd_wait);
//ZOT716u2		khubd_wait = 1;
		cyg_semaphore_post(&khubd_wait);	//ZOT716u2
	}
	spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);
}

static void usb_hub_power_on(struct usb_hub *hub)
{
	int i;

	/* Enable power to the ports */
//{{MARK_DEBUG
//	dbg("enabling power on all ports");
//}}MARK_DEBUG
	for (i = 0; i < hub->descriptor->bNbrPorts; i++)
		usb_set_port_feature(hub->dev, i + 1, USB_PORT_FEAT_POWER);

	/* Wait for power to be enabled */
	wait_ms(hub->descriptor->bPwrOn2PwrGood * 2);
}

static int usb_hub_configure(struct usb_hub *hub, struct usb_endpoint_descriptor *endpoint)
{
	struct usb_device *dev = hub->dev;
	struct usb_hub_status *hubstatus;
	char portstr[USB_MAXCHILDREN + 1];
	unsigned int pipe;
	int i, maxp, ret;

	hub->descriptor = kmalloc(sizeof(*hub->descriptor), GFP_KERNEL);
	if (!hub->descriptor) {
//{{MARK_DEBUG
//		err("Unable to kmalloc %d bytes for hub descriptor", HUB_DESCRIPTOR_MAX_SIZE);
//}}MARK_DEBUG
		return -1;
	}

	/* Request the entire hub descriptor. */
	ret = usb_get_hub_descriptor(dev, hub->descriptor, sizeof(*hub->descriptor));
		/* <hub->descriptor> is large enough for a hub with 127 ports;
		 * the hub can/will return fewer bytes here. */
	if (ret < 0) {
//{{MARK_DEBUG
//		err("Unable to get hub descriptor (err = %d)", ret);
//}}MARK_DEBUG
		kfree(hub->descriptor, 0);
		return -1;
	}

	hub->nports = dev->maxchild = hub->descriptor->bNbrPorts;

	le16_to_cpus(&hub->descriptor->wHubCharacteristics);


//{{MARK_DEBUG
//	dbg("%d port%s detected", hub->nports, (hub->nports == 1) ? "" : "s");
//
//	if (hub->descriptor->wHubCharacteristics & HUB_CHAR_COMPOUND)
//		dbg("part of a compound device");
//	else
//		dbg("standalone hub");
//
//
//	switch (hub->descriptor->wHubCharacteristics & HUB_CHAR_LPSM) {
//		case 0x00:
//			dbg("ganged power switching");
//			break;
//		case 0x01:
//			dbg("individual port power switching");
//			break;
//		case 0x02:
//		case 0x03:
//			dbg("unknown reserved power switching mode");
//			break;
//	}
//
//	switch (hub->descriptor->wHubCharacteristics & HUB_CHAR_OCPM) {
//		case 0x00:
//			dbg("global over-current protection");
//			break;
//		case 0x08:
//			dbg("individual port over-current protection");
//			break;
//		case 0x10:
//		case 0x18:
//			dbg("no over-current protection");
//                        break;
//	}
//
//	dbg("power on to power good time: %dms", hub->descriptor->bPwrOn2PwrGood * 2);
//	dbg("hub controller current requirement: %dmA", hub->descriptor->bHubContrCurrent);
//}}MARK_DEBUG

	switch (dev->descriptor.bDeviceProtocol) {
		case 0:
			break;
		case 1:
//			dbg("Single TT");
			hub->tt.hub = dev;
			break;
		case 2:
//			dbg("TT per port");
			hub->tt.hub = dev;
			hub->tt.multi = 1;
			break;
		default:
//			dbg("Unrecognized hub protocol %d",
//				dev->descriptor.bDeviceProtocol);
			break;
	}
	for (i = 0; i < dev->maxchild; i++)
		portstr[i] = hub->descriptor->DeviceRemovable[((i + 1) / 8)] & (1 << ((i + 1) % 8)) ? 'F' : 'R';
	portstr[dev->maxchild] = 0;

//{{MARK_DEBUG
//	dbg("port removable status: %s", portstr);
//}}MARK_DEBUG

	hubstatus = kmalloc(sizeof *hubstatus, GFP_KERNEL);
	if (!hubstatus) {
		kfree(hub->descriptor, 0);
		return -1;
	}              
	ret = usb_get_hub_status(dev, hubstatus);
	if (ret < 0) {
//{{MARK_DEBUG
//		err("Unable to get hub status (err = %d)", ret);
//}}MARK_DEBUG
		kfree(hubstatus, 0);
		kfree(hub->descriptor, 0);
		return -1;
	}

	le16_to_cpus(&hubstatus->wHubStatus);

//{{MARK_DEBUG
//	dbg("local power source is %s",
//		(hubstatus.wHubStatus & HUB_STATUS_LOCAL_POWER) ? "lost (inactive)" : "good");
//
//	dbg("%sover-current condition exists",
//		(hubstatus.wHubStatus & HUB_STATUS_OVERCURRENT) ? "" : "no ");
//}}MARK_DEBUG
	kfree(hubstatus, 0);

	/* Start the interrupt endpoint */
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

	if (maxp > sizeof(hub->buffer))
		maxp = sizeof(hub->buffer);

	hub->urb = usb_alloc_urb(0);
	if (!hub->urb) {
//{{MARK_DEBUG
//		err("couldn't allocate interrupt urb");
//}}MARK_DEBUG
		kfree(hub->descriptor, 0);
		return -1;
	}

	FILL_INT_URB(hub->urb, dev, pipe, hub->buffer, maxp, hub_irq, hub,
		/* NOTE:  in 2.5 fill_int_urb() converts the encoding */
		(dev->speed == USB_SPEED_HIGH)
			? 1 << (endpoint->bInterval - 1)
			: endpoint->bInterval);
				
	ret = usb_submit_urb(hub->urb);
	if (ret) {
//{{MARK_DEBUG
//		err("usb_submit_urb failed (%d)", ret);
//}}MARK_DEBUG
		kfree(hub->descriptor, 0);
		return -1;
	}
		
	/* Wake up khubd */
//	wake_up(&khubd_wait);
//ZOT716u2	khubd_wait = 1;
	cyg_semaphore_post(&khubd_wait);	//ZOT716u2

	usb_hub_power_on(hub);

	return 0;
}

static void *hub_probe(struct usb_device *dev, unsigned int i,
		       const struct usb_device_id *id)
{
	struct usb_interface_descriptor *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_hub *hub;
	unsigned long flags;

	interface = &dev->actconfig->interface[i].altsetting[0];

	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	if ((interface->bInterfaceSubClass != 0) &&
	    (interface->bInterfaceSubClass != 1)) {
//{{MARK_DEBUG
//		err("invalid subclass (%d) for USB hub device #%d",
//			interface->bInterfaceSubClass, dev->devnum);
//}}MARK_DEBUG
		return NULL;
	}

	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	if (interface->bNumEndpoints != 1) {
//{{MARK_DEBUG
//		err("invalid bNumEndpoints (%d) for USB hub device #%d",
//			interface->bNumEndpoints, dev->devnum);
//}}MARK_DEBUG
		return NULL;
	}

	endpoint = &interface->endpoint[0];

	/* Output endpoint? Curiousier and curiousier.. */
	if (!(endpoint->bEndpointAddress & USB_DIR_IN)) {
//{{MARK_DEBUG
//		err("Device #%d is hub class, but has output endpoint?",
//			dev->devnum);
//}}MARK_DEBUG
		return NULL;
	}

	/* If it's not an interrupt endpoint, we'd better punt! */
	if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT) {
//{{MARK_DEBUG
//		err("Device #%d is hub class, but has endpoint other than interrupt?",
//			dev->devnum);
//}}MARK_DEBUG
		return NULL;
	}

	/* We found a hub */
//{{MARK_DEBUG
//	dbg("USB hub found");
//}}MARK_DEBUG

	hub = kmalloc(sizeof(*hub), GFP_KERNEL);
	if (!hub) {
//{{MARK_DEBUG
//		err("couldn't kmalloc hub struct");
//}}MARK_DEBUG
		return NULL;
	}

	memset(hub, 0, sizeof(*hub));

	INIT_LIST_HEAD(&hub->event_list);
	hub->dev = dev;
	atomic_set(&hub->refcnt, 1);

	/* Record the new hub's existence */
	spin_lock_irqsave(&hub_event_lock, &flags); //ZOT==> spin_lock_irqsave(&hub_event_lock, flags);
	INIT_LIST_HEAD(&hub->hub_list);
	list_add(&hub->hub_list, &hub_list);
	spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);

//ZOT
	if( dev->parent == NULL )
		dev->hub_level_count = 0;
	else	
	{
		dev->hub_level_count = dev->parent->hub_level_count + 1;
	}
		

	if (usb_hub_configure(hub, endpoint) >= 0)
		return hub;

//{{MARK_DEBUG
//	err("hub configuration failed for device #%d", dev->devnum);
//}}MARK_DEBUG

	/* free hub, but first clean up its list. */
	spin_lock_irqsave(&hub_event_lock, &flags); //ZOT==> spin_lock_irqsave(&hub_event_lock, flags);

	/* Delete it and then reset it */
	list_del(&hub->event_list);
	INIT_LIST_HEAD(&hub->event_list);
	list_del(&hub->hub_list);
	INIT_LIST_HEAD(&hub->hub_list);

	spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);

	kfree(hub, 0);

	return NULL;
}

static void hub_get(struct usb_hub *hub)
{
	atomic_inc(&hub->refcnt);
}

static void hub_put(struct usb_hub *hub)
{
	if (atomic_dec_and_test(&hub->refcnt)) {
		if (hub->descriptor) {
			kfree(hub->descriptor, 0);
			hub->descriptor = NULL;
		}

		kfree(hub, 0);
	}
}

static void hub_disconnect(struct usb_device *dev, void *ptr)
{
	struct usb_hub *hub = (struct usb_hub *)ptr;
	unsigned long flags;

	if (hub->urb) {
		usb_unlink_urb(hub->urb);
		usb_free_urb(hub->urb);
		hub->urb = NULL;
	}

	spin_lock_irqsave(&hub_event_lock, &flags); //ZOT==> spin_lock_irqsave(&hub_event_lock, flags);

	/* Delete it and then reset it */
	list_del(&hub->event_list);
	INIT_LIST_HEAD(&hub->event_list);
	list_del(&hub->hub_list);
	INIT_LIST_HEAD(&hub->hub_list);

	spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);

	hub_put(hub);
}

static int hub_ioctl(struct usb_device *hub, unsigned int code, void *user_data)
{
	/* assert ifno == 0 (part of hub spec) */
	switch (code) {
// Charles, fix me
#if 0
	case USBDEVFS_HUB_PORTINFO: {
		struct usbdevfs_hub_portinfo *info = user_data;
		unsigned long flags;
		int i;

		spin_lock_irqsave(&hub_event_lock, &flags); //ZOT==> spin_lock_irqsave(&hub_event_lock, flags);
		if (hub->devnum <= 0)
			info->nports = 0;
		else {
			info->nports = hub->maxchild;
			for (i = 0; i < info->nports; i++) {
				if (hub->children[i] == NULL)
					info->port[i] = 0;
				else
					info->port[i] = hub->children[i]->devnum;
			}
		}
		spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);

		return info->nports + 1;
		}
#endif
	default:
		return -ENOSYS;
	}
}

static int usb_hub_reset(struct usb_hub *hub)
{
	struct usb_device *dev = hub->dev;
	int i;

	/* Disconnect any attached devices */
	for (i = 0; i < hub->descriptor->bNbrPorts; i++) {
		if (dev->children[i])
			usb_disconnect(&dev->children[i]);
	}

	/* Attempt to reset the hub */
	if (hub->urb)
		usb_unlink_urb(hub->urb);
	else
		return -1;

	if (usb_reset_device(dev))
		return -1;

	hub->urb->dev = dev;                                                    
	if (usb_submit_urb(hub->urb))
		return -1;

	usb_hub_power_on(hub);

	return 0;
}

static void usb_hub_disconnect(struct usb_device *dev)
{
	struct usb_device *parent = dev->parent;
	int i;

	/* Find the device pointer to disconnect */
	if (parent) {
		for (i = 0; i < parent->maxchild; i++) {
			if (parent->children[i] == dev) {
				usb_disconnect(&parent->children[i]);
				return;
			}
		}
	}

//{{MARK_DEBUG
//	err("cannot disconnect hub %d", dev->devnum);
//}}MARK_DEBUG
}

static int usb_hub_port_status(struct usb_device *hub, int port,
			       u16 *status, u16 *change)
{
	struct usb_port_status *portsts;
	int ret = -ENOMEM;

	portsts = kmalloc(sizeof(*portsts), GFP_KERNEL);
	if (portsts) {
		ret = usb_get_port_status(hub, port + 1, portsts);
		if (ret < 0)
		{
//			err("%s (%d) failed (err = %d)", __FUNCTION__, hub->devnum, ret);
		}
		else {
			*status = le16_to_cpu(portsts->wPortStatus);
			*change = le16_to_cpu(portsts->wPortChange); 
//			dbg("port %d, portstatus %x, change %x, %s", port + 1,
//				*status, *change, portspeed(*status));
			ret = 0;
		}
		kfree(portsts, 0);
	}
	return ret;
}

#define HUB_RESET_TRIES		5
#define HUB_PROBE_TRIES		2
#define HUB_SHORT_RESET_TIME	10
#define HUB_LONG_RESET_TIME	200
#define HUB_RESET_TIMEOUT	500

/* return: -1 on error, 0 on success, 1 on disconnect.  */
static int usb_hub_port_wait_reset(struct usb_device *hub, int port,
				struct usb_device *dev, unsigned int delay)
{
	int delay_time, ret;
	u16 portstatus;
	u16 portchange;

	for (delay_time = 0; delay_time < HUB_RESET_TIMEOUT; delay_time += delay) {
		/* wait to give the device a chance to reset */
		wait_ms(delay);

		/* read and decode port status */
		ret = usb_hub_port_status(hub, port, &portstatus, &portchange);
		if (ret < 0) {
//{{MARK_DEBUG
//			err("get_port_status(%d) failed (err = %d)", port + 1, ret);
//}}MARK_DEBUG
			return -1;
		}

		/* Device went away? */
		if (!(portstatus & USB_PORT_STAT_CONNECTION))
			return 1;
//{{MARK_DEBUG
//		dbg("port %d, portstatus %x, change %x, %s", port + 1,
//			portstatus, portchange,
//			portstatus & (1 << USB_PORT_FEAT_LOWSPEED) ? "1.5 Mb/s" : "12 Mb/s");
//}}MARK_DEBUG

		/* bomb out completely if something weird happened */
		if ((portchange & USB_PORT_STAT_C_CONNECTION))
			return -1;

		/* if we`ve finished resetting, then break out of the loop */
//635u2 for usb2.0		
		if (!(portstatus & USB_PORT_STAT_RESET) &&
		    (portstatus & USB_PORT_STAT_ENABLE)) {
			if (portstatus & USB_PORT_STAT_HIGH_SPEED)
				dev->speed = USB_SPEED_HIGH;
			else if (portstatus & USB_PORT_STAT_LOW_SPEED)
				dev->speed = USB_SPEED_LOW;
			else
				dev->speed = USB_SPEED_FULL;
			return 0;
		}

		/* switch to the long delay after two short delay failures */
		if (delay_time >= 2 * HUB_SHORT_RESET_TIME)
			delay = HUB_LONG_RESET_TIME;

//{{MARK_DEBUG
//		dbg("port %d of hub %d not reset yet, waiting %dms", port + 1,
//			hub->devnum, delay);
//}}MARK_DEBUG
	}

	return -1;
}

/* return: -1 on error, 0 on success, 1 on disconnect.  */
static int usb_hub_port_reset(struct usb_device *hub, int port,
				struct usb_device *dev, unsigned int delay)
{
	int i, status;

	/* Reset the port */
	for (i = 0; i < HUB_RESET_TRIES; i++) {
		usb_set_port_feature(hub, port + 1, USB_PORT_FEAT_RESET);

		/* return on disconnect or reset */
		status = usb_hub_port_wait_reset(hub, port, dev, delay);
		if (status != -1) {
			usb_clear_port_feature(hub, port + 1, USB_PORT_FEAT_C_RESET);
			return status;
		}

//{{MARK_DEBUG
//		dbg("port %d of hub %d not enabled, trying reset again...",
//			port + 1, hub->devnum);
//}}MARK_DEBUG
		delay = HUB_LONG_RESET_TIME;
	}

//{{MARK_DEBUG
//	err("Cannot enable port %i of hub %d, disabling port.",
//		port + 1, hub->devnum);
//	err("Maybe the USB cable is bad?");
//}}MARK_DEBUG

	return -1;
}

void usb_hub_port_disable(struct usb_device *hub, int port)
{
	int ret;

	ret = usb_clear_port_feature(hub, port + 1, USB_PORT_FEAT_ENABLE);

//{{MARK_DEBUG
//	if (ret)
//		err("cannot disable port %d of hub %d (err = %d)",
//			port + 1, hub->devnum, ret);
//}}MARK_DEBUG
}

/* USB 2.0 spec, 7.1.7.3 / fig 7-29:
 *
 * Between connect detection and reset signaling there must be a delay
 * of 100ms at least for debounce and power-settling. The corresponding
 * timer shall restart whenever the downstream port detects a disconnect.
 * 
 * Apparently there are some bluetooth and irda-dongles and a number
 * of low-speed devices which require longer delays of about 200-400ms.
 * Not covered by the spec - but easy to deal with.
 *
 * This implementation uses 400ms minimum debounce timeout and checks
 * every 100ms for transient disconnects to restart the delay.
 */

#define HUB_DEBOUNCE_TIMEOUT	400
#define HUB_DEBOUNCE_STEP	100

/* return: -1 on error, 0 on success, 1 on disconnect.  */
static int usb_hub_port_debounce(struct usb_device *hub, int port)
{
	int ret;
	unsigned delay_time;
	u16 portchange, portstatus;

	for (delay_time = 0; delay_time < HUB_DEBOUNCE_TIMEOUT; /* empty */ ) {

		/* wait debounce step increment */
		wait_ms(HUB_DEBOUNCE_STEP);

		ret = usb_hub_port_status(hub, port, &portstatus, &portchange);
		if (ret < 0)
			return -1;

		if ((portchange & USB_PORT_STAT_C_CONNECTION)) {
			usb_clear_port_feature(hub, port+1, USB_PORT_FEAT_C_CONNECTION);
			delay_time = 0;
		}
		else
			delay_time += HUB_DEBOUNCE_STEP;
	}
	return ((portstatus&USB_PORT_STAT_CONNECTION)) ? 0 : 1;
}

static void usb_hub_port_connect_change(struct usb_hub *hubstate, int port,
					u16 portstatus, u16 portchange)
{
	struct usb_device *hub = hubstate->dev;
	struct usb_device *dev;
	unsigned int delay = HUB_SHORT_RESET_TIME;
	int i;

//{{MARK_DEBUG
//	dbg("port %d, portstatus %x, change %x, %s", port + 1, portstatus,
//		portchange, portstatus & (1 << USB_PORT_FEAT_LOWSPEED) ? "1.5 Mb/s" : "12 Mb/s");
//}}MARK_DEBUG

	/* Clear the connection change status */
	usb_clear_port_feature(hub, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	if (hub->children[port])
		usb_disconnect(&hub->children[port]);

	/* Return now if nothing is connected */
	if (!(portstatus & USB_PORT_STAT_CONNECTION)) {

		if( ( hub->parent == NULL) && ( port == 1 ) )
		{
		if(hub->speed == USB_SPEED_HIGH)
			EHCI_LINK[0] = 0;
		else
			OHCI_LINK[0] = 0;
		}
#ifdef USB_LED		

		if( !EHCI_LINK[0] && !OHCI_LINK[0] )	
		{
			usb_devices_per_port[0] = 0;
			Light_Off(Usb11_Lite);	//ZOT716u2 no matter 1.1 or 2.0 
		}
#endif
		
		if (portstatus & USB_PORT_STAT_ENABLE)
			usb_hub_port_disable(hub, port);

		return;
	}

	if (usb_hub_port_debounce(hub, port)) {
//		err("connect-debounce failed, port %d disabled", port+1);
		usb_hub_port_disable(hub, port);
		return;
	}

	down(&usb_address0_sem);
		
	for (i = 0; i < HUB_PROBE_TRIES; i++) {
		struct usb_device *pdev;
		int len;

		/* Allocate a new device struct */
		dev = usb_alloc_dev(hub, hub->bus);
		if (!dev) {
//{{MARK_DEBUG
//			err("couldn't allocate usb_device");
//}}MARK_DEBUG
			break;
		}

		hub->children[port] = dev;

		/* Reset the device */
		if (usb_hub_port_reset(hub, port, dev, delay)) {
			usb_free_dev(dev);
			break;
		}

		/* Find a new device ID for it */
		usb_connect(dev);

		/* Set up TT records, if needed  */
		if (hub->tt) {
			dev->tt = hub->tt;
			dev->ttport = hub->ttport;
		} else if (dev->speed != USB_SPEED_HIGH
				&& hub->speed == USB_SPEED_HIGH) {
			dev->tt = &hubstate->tt;
			dev->ttport = port + 1;
		}

		/* Save readable and stable topology id, distinguishing
		 * devices by location for diagnostics, tools, etc.  The
		 * string is a path along hub ports, from the root.  Each
		 * device's id will be stable until USB is re-cabled, and
		 * hubs are often labeled with these port numbers.
		 *
		 * Initial size: ".NN" times five hubs + NUL = 16 bytes max
		 * (quite rare, since most hubs have 4-6 ports).
		 */
		pdev = dev->parent;
//		if (pdev->devpath [0] != '0')	/* parent not root? */
//			len = snprintf (dev->devpath, sizeof dev->devpath,
//				"%s.%d", pdev->devpath, port + 1);
		/* root == "0", root port 2 == "2", port 3 that hub "2.3" */
//				else
//			len = snprintf (dev->devpath, sizeof dev->devpath,
//				"%d", port + 1);
//		if (len == sizeof dev->devpath)
//			warn ("devpath size! usb/%03d/%03d path %s",
//				dev->bus->busnum, dev->devnum, dev->devpath);
//		info("new USB device %s-%s, assigned address %d",
//			dev->bus->bus_name, dev->devpath, dev->devnum);
		
		if( dev->parent->hub_level_count == 0)
		{
			dev->printer_port = 0;
		}
		else
			dev->printer_port = dev->parent->printer_port;

		/* Run it through the hoops (find a driver, etc) */
		if (!usb_new_device(dev))
		{	
//615wu for usb detect
			if( ( hub->parent == NULL) && ( port == 1 ) )
			{
			if(dev->speed == USB_SPEED_HIGH){
					EHCI_LINK[0] = 1;
				Light_On(Usb20_Lite);
			}else{ 
					OHCI_LINK[0] = 1;
				Light_On(Usb11_Lite);
			}
			
#ifdef USB_LED			
				if( OHCI_LINK[0] || EHCI_LINK[0] )
			{
					usb_devices_per_port[0] = 1;
//ZOT716u2				Light_On(Usb_Lite);
			}
#endif		
			}
			goto done;
		}
		/* Free the configuration if there was an error */
		usb_free_dev(dev);

		/* Switch to a long reset time */
		delay = HUB_LONG_RESET_TIME;
	}

	hub->children[port] = NULL;
	usb_hub_port_disable(hub, port);
done:
	
	up(&usb_address0_sem);
}

static void usb_hub_events(void)
{
	unsigned long flags;
	struct list_head *tmp;
	struct usb_device *dev;
	struct usb_hub *hub;
	struct usb_hub_status *hubsts;
	u16 hubstatus;
	u16 hubchange;
	u16 portstatus;
	u16 portchange;
	int i, ret;

	/*
	 *  We restart the list everytime to avoid a deadlock with
	 * deleting hubs downstream from this one. This should be
	 * safe since we delete the hub from the event list.
	 * Not the most efficient, but avoids deadlocks.
	 */
	while (1) {
		spin_lock_irqsave(&hub_event_lock, &flags); //ZOT==> spin_lock_irqsave(&hub_event_lock, flags);

		if (list_empty(&hub_event_list))
			break;

		/* Grab the next entry from the beginning of the list */
		tmp = hub_event_list.next;

		hub = list_entry(tmp, struct usb_hub, event_list);
		dev = hub->dev;

		list_del(tmp);
		INIT_LIST_HEAD(tmp);

		hub_get(hub);
		spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);

		if (hub->error) {
//{{MARK_DEBUG
//			dbg("resetting hub %d for error %d", dev->devnum, hub->error);
//}}MARK_DEBUG

			if (usb_hub_reset(hub)) {
//{{MARK_DEBUG
//				err("error resetting hub %d - disconnecting", dev->devnum);
//}}MARK_DEBUG
				usb_hub_disconnect(dev);
				hub_put(hub);
				continue;
			}

			hub->nerrors = 0;
			hub->error = 0;
		}

		for (i = 0; i < hub->descriptor->bNbrPorts; i++) {
			ret = usb_hub_port_status(dev, i, &portstatus, &portchange);
			if (ret < 0) {
//{{MARK_DEBUG
//				err("get_port_status failed (err = %d)", ret);
//}}MARK_DEBUG
				continue;
			}

			if (portchange & USB_PORT_STAT_C_CONNECTION) {
//{{MARK_DEBUG
//				dbg("port %d connection change", i + 1);
//}}MARK_DEBUG
				usb_hub_port_connect_change(hub, i, portstatus, portchange);
			} else if (portchange & USB_PORT_STAT_C_ENABLE) {
//{{MARK_DEBUG
//				dbg("port %d enable change, status %x", i + 1, portstatus);
//}}MARK_DEBUG
				usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);

				/*
				 * EM interference sometimes causes bad shielded USB devices to 
				 * be shutdown by the hub, this hack enables them again.
				 * Works at least with mouse driver. 
				 */
				if (!(portstatus & USB_PORT_STAT_ENABLE) && 
				    (portstatus & USB_PORT_STAT_CONNECTION) && (dev->children[i])) {
//{{MARK_DEBUG
//					err("already running port %i disabled by hub (EMI?), re-enabling...",
//						i + 1);
//}}MARK_DEBUG
					usb_hub_port_connect_change(hub, i, portstatus, portchange);
				}
			}

			if (portchange & USB_PORT_STAT_C_SUSPEND) {
//{{MARK_DEBUG
//				dbg("port %d suspend change", i + 1);
//}}MARK_DEBUG
				usb_clear_port_feature(dev, i + 1,  USB_PORT_FEAT_C_SUSPEND);
			}
			
			if (portchange & USB_PORT_STAT_C_OVERCURRENT) {
//{{MARK_DEBUG
//				err("port %d over-current change", i + 1);
//}}MARK_DEBUG
				usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_OVER_CURRENT);
				usb_hub_power_on(hub);
			}

			if (portchange & USB_PORT_STAT_C_RESET) {
//{{MARK_DEBUG
//				dbg("port %d reset change", i + 1);
//}}MARK_DEBUG
				usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
			}
		} /* end for i */

		/* deal with hub status changes */
		hubsts = kmalloc(sizeof *hubsts, GFP_KERNEL);
		if (!hubsts) {		
//{{MARK_DEBUG
//			err("get_hub_status failed");
//}}MARK_DEBUG
		} else {
			if (usb_get_hub_status(dev, hubsts) < 0)
			{
//				err("get_hub_status failed");
			}
			else {
				hubstatus = le16_to_cpup(&hubsts->wHubStatus);
				hubchange = le16_to_cpup(&hubsts->wHubChange);
			if (hubchange & HUB_CHANGE_LOCAL_POWER) {
//{{MARK_DEBUG
//				dbg("hub power change");
//}}MARK_DEBUG

				
				usb_clear_hub_feature(dev, C_HUB_LOCAL_POWER);
			}
			if (hubchange & HUB_CHANGE_OVERCURRENT) {
//{{MARK_DEBUG
//				dbg("hub overcurrent change");
//}}MARK_DEBUG
				wait_ms(500);	/* Cool down */
				
				
				usb_clear_hub_feature(dev, C_HUB_OVER_CURRENT);
                        	usb_hub_power_on(hub);
			}
		}
			kfree(hubsts, 0);
		}
//ZOT716u2		cyg_thread_yield();


    } /* end while (1) */



	spin_unlock_irqrestore(&hub_event_lock, &flags);//ZOT==> spin_unlock_irqrestore(&hub_event_lock, flags);
}
void usb_hub_thread(cyg_addrword_t data)
{
//	lock_kernel();

	/*
	 * This thread doesn't need any user-level access,
	 * so get rid of all our resources
	 */

//	daemonize();

	/* Setup a nice name */
//	strcpy(current->comm, "khubd");

	/* Send me a signal to get me die (for debugging) */
	while( 1 )
	{
		usb_hub_events();
//		interruptible_sleep_on(&khubd_wait);
#if 0	//ZOT716u2
		while( khubd_wait == 0 ) ppause( 1000 );
		khubd_wait = 0;
#else	//ZOT716u2
		cyg_semaphore_wait(&khubd_wait);
#endif	//ZOT716u2		
	}	

//{{MARK_DEBUG
//	dbg("usb_hub_thread exiting");
//}}MARK_DEBUG

//	up_and_exit(&khubd_exited, 0);
}

static struct usb_device_id hub_id_table [] = {
    {	USB_DEVICE_ID_MATCH_INT_CLASS,	// match_flags
		0, 								// idVendor
		0, 								// idProduct
		0, 								// bcdDevice_lo
		0, 								// bcdDevice_hi
		0, 								// bDeviceClass
		0, 								// bDeviceSubClass
		0, 								// bDeviceProtocol
  		USB_CLASS_HUB,					// bInterfaceClass
		0, 								// bInterfaceSubClass
		0, 								// bInterfaceProtocol
		NULL, 							// driver_info
	}, 
	{ }	// Terminating entry
};


//MODULE_DEVICE_TABLE (usb, hub_id_table);

static struct usb_driver hub_driver = {
	/*name*/		"hub",
};


/*
 * This should be a separate module.
 */
 
 //USBHUB Task create information definition
#define USBHUB_TASK_PRI         20	//ZOT716u2
#define USBHUB_TASK_STACK_SIZE	4096//2048
static uint8 			USBHUB_Stack[USBHUB_TASK_STACK_SIZE];
static cyg_thread       USBHUB_Task;
static cyg_handle_t     USBHUB_TaskHdl;

int usb_hub_init(void)
{
	hub_driver.probe	  = hub_probe;
	hub_driver.disconnect = hub_disconnect;
	hub_driver.ioctl	  = hub_ioctl;
	hub_driver.id_table   = hub_id_table;

	__spin_lock_init(&hub_event_lock);	//ZOT==>
	if (usb_register(&hub_driver) < 0) {
//{{MARK_DEBUG
//		err("Unable to register USB hub driver");
//}}MARK_DEBUG
		return -1;
	}
	
	init_MUTEX(&usb_address0_sem);	//ZOT==>
	cyg_semaphore_init(&khubd_wait, 0);		//ZOT716u2
	
	//Create USBHUB Thread
    cyg_thread_create(USBHUB_TASK_PRI,
                  usb_hub_thread,
                  0,
                  "HUB",
                  (void *) (USBHUB_Stack),
                  USBHUB_TASK_STACK_SIZE,
                  &USBHUB_TaskHdl,
                  &USBHUB_Task);
	
	//Start PSmain Thread
	cyg_thread_resume(USBHUB_TaskHdl);

//	if( newproc( "HUB", 2048, , 0, NULL, NULL, 0 ) == NULL ){
//		usb_deregister(&hub_driver);
//{{MARK_DEBUG
//		err("failed to start usb_hub_thread");
//}}MARK_DEBUG
//		return -1;
//	}

	return 0;

//	pid = kernel_thread(usb_hub_thread, NULL,
//		CLONE_FS | CLONE_FILES | CLONE_SIGHAND);
//	if (pid >= 0) {
//		khubd_pid = pid;
//
//		return 0;
//	}
//
//	/* Fall through if kernel_thread failed */
//	usb_deregister(&hub_driver);
//	err("failed to start usb_hub_thread");

//	return -1;
}

void usb_hub_cleanup(void)
{
//	int ret;

	cyg_semaphore_destroy(&khubd_wait);	//ZOT716u2 never do this.

	/* Kill the thread */
//	ret = kill_proc(khubd_pid, SIGTERM, 1);

//	down(&khubd_exited);

	/*
	 * Hub resources are freed for us by usb_deregister. It calls
	 * usb_driver_purge on every device which in turn calls that
	 * devices disconnect function if it is using this driver.
	 * The hub_disconnect function takes care of releasing the
	 * individual hub resources. -greg
	 */
	usb_deregister(&hub_driver);
} /* usb_hub_cleanup() */

/*
 * WARNING - If a driver calls usb_reset_device, you should simulate a
 * disconnect() and probe() for other interfaces you doesn't claim. This
 * is left up to the driver writer right now. This insures other drivers
 * have a chance to re-setup their interface.
 *
 * Take a look at proc_resetdevice in devio.c for some sample code to
 * do this.
 */
int usb_reset_device(struct usb_device *dev)
{
	struct usb_device *parent = dev->parent;
	struct usb_device_descriptor *descriptor;
	int i, ret, port = -1;

	if (!parent) {
//{{MARK_DEBUG
//		err("attempting to reset root hub!");
//}}MARK_DEBUG
		return -EINVAL;
	}

	for (i = 0; i < parent->maxchild; i++)
		if (parent->children[i] == dev) {
			port = i;
			break;
		}

	if (port < 0)
		return -ENOENT;

	down(&usb_address0_sem);

	/* Send a reset to the device */
	if (usb_hub_port_reset(parent, port, dev, HUB_SHORT_RESET_TIME)) {
		usb_hub_port_disable(parent, port);
		up(&usb_address0_sem);
		return(-ENODEV);
	}

	/* Reprogram the Address */
	ret = usb_set_address(dev);
	if (ret < 0) {
//{{MARK_DEBUG
//		err("USB device not accepting new address (error=%d)", ret);
//}}MARK_DEBUG
		usb_hub_port_disable(parent, port);
		up(&usb_address0_sem);
		return ret;
	}

	/* Let the SET_ADDRESS settle */
	wait_ms(10);

	up(&usb_address0_sem);

	/*
	 * Now we fetch the configuration descriptors for the device and
	 * see if anything has changed. If it has, we dump the current
	 * parsed descriptors and reparse from scratch. Then we leave
	 * the device alone for the caller to finish setting up.
	 *
	 * If nothing changed, we reprogram the configuration and then
	 * the alternate settings.
	 */
	descriptor = kmalloc(sizeof *descriptor, 0);
	if (!descriptor) {
		return -ENOMEM;
	}
	ret = usb_get_descriptor(dev, USB_DT_DEVICE, 0, descriptor,
			sizeof(*descriptor));
	if (ret < 0)
		return ret;

	le16_to_cpus(&descriptor->bcdUSB);
	le16_to_cpus(&descriptor->idVendor);
	le16_to_cpus(&descriptor->idProduct);
	le16_to_cpus(&descriptor->bcdDevice);

	if (memcmp(&dev->descriptor, descriptor, sizeof(*descriptor))) {
		kfree(descriptor, 0);
		usb_destroy_configuration(dev);

		ret = usb_get_device_descriptor(dev);
		if (ret < sizeof(dev->descriptor)) {
//{{MARK_DEBUG
//			if (ret < 0)
//				err("unable to get device descriptor (error=%d)", ret);
//			else
//				err("USB device descriptor short read (expected %Zi, got %i)", sizeof(dev->descriptor), ret);
//}}MARK_DEBUG
        
			clear_bit(dev->devnum, &dev->bus->devmap.devicemap);
			dev->devnum = -1;
			return -EIO;
		}

		ret = usb_get_configuration(dev);
		if (ret < 0) {
//{{MARK_DEBUG
//			err("unable to get configuration (error=%d)", ret);
//}}MARK_DEBUG
			usb_destroy_configuration(dev);
			clear_bit(dev->devnum, &dev->bus->devmap.devicemap);
			dev->devnum = -1;
			return 1;
		}

		dev->actconfig = dev->config;
		usb_set_maxpacket(dev);

		return 1;
	}

	kfree(descriptor, 0);
	ret = usb_set_configuration(dev, dev->actconfig->bConfigurationValue);
	if (ret < 0) {
//{{MARK_DEBUG
//		err("failed to set active configuration (error=%d)", ret);
//}}MARK_DEBUG
		return ret;
	}

	for (i = 0; i < dev->actconfig->bNumInterfaces; i++) {
		struct usb_interface *intf = &dev->actconfig->interface[i];
		struct usb_interface_descriptor *as = &intf->altsetting[intf->act_altsetting];

		ret = usb_set_interface(dev, as->bInterfaceNumber, as->bAlternateSetting);
		if (ret < 0) {
//{{MARK_DEBUG
//			err("failed to set active alternate setting for interface %d (error=%d)", i, ret);
//}}MARK_DEBUG
			return ret;
		}
	}

	return 0;
}

int usb_hub_link_devices( int port )
{
	if( port >= 0 && port < NUM_PORTS )
		return usb_devices_per_port[port];
	return 0;
}

/* Ron Add for QC testing 8/31/04 */
unsigned int usb_device_detect(uint8 *PortNum, uint8 *PortState)
{
	int port;
	
	*PortNum = NUM_PORTS;
		
	for( port = 0; port < NUM_PORTS ; port++)
	{
//		*PortState = usb_devices_per_port[port];
		if( OHCI_LINK[port] && EHCI_LINK[port])
			return -1;
		if( OHCI_LINK[port] || EHCI_LINK[port])	
			*PortState = 1;
		else
			*PortState = 0;
		PortState++;
	}
	return 0;
}
