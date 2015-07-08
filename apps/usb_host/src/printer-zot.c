/*
 * printer.c  Version 0.8
 *
 * Copyright (c) 1999 Michael Gee	<michael@linuxspecific.com>
 * Copyright (c) 1999 Pavel Machek	<pavel@suse.cz>
 * Copyright (c) 2000 Randy Dunlap	<randy.dunlap@intel.com>
 * Copyright (c) 2000 Vojtech Pavlik	<vojtech@suse.cz>
 *
 * USB Printer Device Class driver for USB printers and printer cables
 *
 * Sponsored by SuSE
 *
 * ChangeLog:
 *	v0.1 - thorough cleaning, URBification, almost a rewrite
 *	v0.2 - some more cleanups
 *	v0.3 - cleaner again, waitqueue fixes
 *	v0.4 - fixes in unidirectional mode
 *	v0.5 - add DEVICE_ID string support
 *	v0.6 - never time out
 *	v0.7 - fixed bulk-IN read and poll (David Paschal, paschal@rcsis.com)
 *	v0.8 - add devfs support
 *	v0.9 - fix unplug-while-open paths
 */

/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "usb.h"
#include "prnport.h"
#include "led.h"

#define USB_PRINTER_WRITE_BUFFER	USBPRN_WRITE_BUFFER	//ZOT716u2 //8K
#define USB_PRINTER_READ_BUFFER  	USBPRN_READ_BUFFER	//ZOT716u2 //256byte

extern struct parport 	PortIO[NUM_OF_PRN_PORT];

#define LP_PERRORP			0x8
#define LP_POUTPA 			0x20
#define LP_PSELECD          0x10

#define USBLP_BUF_SIZE		8192
//#define USBLP_BUF_SIZE		4096
#define DEVICE_ID_SIZE		LENGTH_OF_DEVICE_ID

#define IOCNR_GET_DEVICE_ID	1
#define LPIOC_GET_DEVICE_ID(len) _IOC(_IOC_READ, 'P', IOCNR_GET_DEVICE_ID, len)	/* get device_id string */
#define LPGETSTATUS		0x060b		/* same as in drivers/char/lp.c */

/*
 * A DEVICE_ID string may include the printer's serial number.
 * It should end with a semi-colon (';').
 * An example from an HP 970C DeskJet printer is (this is one long string,
 * with the serial number changed):
MFG:HEWLETT-PACKARD;MDL:DESKJET 970C;CMD:MLC,PCL,PML;CLASS:PRINTER;DESCRIPTION:Hewlett-Packard DeskJet 970C;SERN:US970CSEPROF;VSTATUS:$HB0$NC0,ff,DN,IDLE,CUT,K1,C0,DP,NR,KP000,CP027;VP:0800,FL,B0;VJ:                    ;
 */

/*
 * USB Printer Requests
 */

#define USBLP_REQ_GET_ID	0x00
#define USBLP_REQ_GET_STATUS	0x01
#define USBLP_REQ_RESET		0x02

#define USBLP_MINORS		NUM_OF_USB_PRN_DEVICE
#define USBLP_MINOR_BASE	0

#define USBLP_WRITE_TIMEOUT	(5*HZ)			/* 5 seconds */

struct usblp {
	struct usb_device 	*dev;			/* USB device */
//	devfs_handle_t		devfs;			/* devfs device */
	struct urb		readurb, writeurb;	/* The urbs */
//	wait_queue_head_t	wait;			/* Zzzzz ... */
	void	* wait;		//ZOT716u2
	int			readcount;		/* Counter for reads */
	int			ifnum;			/* Interface number */
	int			minor;			/* minor number of device */
	unsigned int		quirks;			/* quirks flags */
	unsigned char		used;			/* True if open */
	unsigned char		bidir;			/* interface is bidirectional */
	unsigned char		*device_id_string;	/* IEEE 1284 DEVICE ID string (ptr) */
										/* first 2 bytes are (big-endian) length */
};

//extern devfs_handle_t usb_devfs_handle;			/* /dev/usb dir. */

static struct usblp *usblp_table[USBLP_MINORS];

/* Quirks: various printer quirks are handled by this table & its flags. */

struct quirk_printer_struct {
	__u16 vendorId;
	__u16 productId;
	unsigned int quirks;
};

#define USBLP_QUIRK_BIDIR	0x1	/* reports bidir but requires unidirectional mode (no INs/reads) */
#define USBLP_QUIRK_USB_INIT	0x2	/* needs vendor USB init string */

static struct quirk_printer_struct quirk_printers[] = {
	{ 0x03f0, 0x0004, USBLP_QUIRK_BIDIR }, /* HP DeskJet 895C */
	{ 0x03f0, 0x0104, USBLP_QUIRK_BIDIR }, /* HP DeskJet 880C */
	{ 0x03f0, 0x0204, USBLP_QUIRK_BIDIR }, /* HP DeskJet 815C */
	{ 0x03f0, 0x0304, USBLP_QUIRK_BIDIR }, /* HP DeskJet 810C/812C */
	{ 0x03f0, 0x0404, USBLP_QUIRK_BIDIR }, /* HP DeskJet 830C */
	{ 0, 0 }
};
 
/*
 * Functions for usblp control messages.
 */

static int usblp_ctrl_msg(struct usblp *usblp, int request, int dir, int recip, int value, void *buf, int len)
{
	int retval = usb_control_msg(usblp->dev,
		dir ? usb_rcvctrlpipe(usblp->dev, 0) : usb_sndctrlpipe(usblp->dev, 0),
		request, USB_TYPE_CLASS | dir | recip, value, usblp->ifnum, buf, len, HZ * 5);
//{{MARK_DEBUG
//	dbg("usblp_control_msg: rq: 0x%02x dir: %d recip: %d value: %d len: %#x result: %d",
//		request, !!dir, recip, value, len, retval);
//}}MARK_DEBUG
	return retval < 0 ? retval : 0;
}

#define usblp_read_status(usblp, status)\
	usblp_ctrl_msg(usblp, USBLP_REQ_GET_STATUS, USB_DIR_IN, USB_RECIP_INTERFACE, 0, status, 1)
#define usblp_get_id(usblp, config, id, maxlen)\
	usblp_ctrl_msg(usblp, USBLP_REQ_GET_ID, USB_DIR_IN, USB_RECIP_INTERFACE, config, id, maxlen)
#define usblp_reset(usblp)\
	usblp_ctrl_msg(usblp, USBLP_REQ_RESET, USB_DIR_OUT, USB_RECIP_OTHER, 0, NULL, 0)

/*
 * See below for the usage explanation, the code is self-evident
 * (this time it is true). Look into your /proc/bus/usb/devices
 * and dmesg in case of any trouble.
 *
 * The parameter can only set the bias for all printers in the system.
 * If you know a better way to do it, feel free to send a patch.
 */
static int proto_bias = -1;


/*
 * URB callback.
 */

static void usblp_bulk(struct urb *urb)
{
	struct usblp *usblp = urb->context;

	if (!usblp || !usblp->dev || !usblp->used)
		return;

//{{MARK_DEBUG
//	if (urb->status)
//		warn("usblp%d: nonzero read/write bulk status received: %d",
//			usblp->minor, urb->status);
//}}MARK_DEBUG

	cyg_semaphore_post(usblp->wait);	//ZOT716u2
//	wake_up_interruptible(&usblp->wait);
}

/*
 * Get and print printer errors.
 */

//static char *usblp_messages[] = { "ok", "out of paper", "off-line", "on fire" };

static int usblp_check_status(struct usblp *usblp, int errno )
{
	unsigned char status, newerr = 0;
	int error;

	error = usblp_read_status (usblp, &status);
	if (error < 0) {
//{{MARK_DEBUG
//		err("usblp%d: error %d reading printer status",
//			usblp->minor, error);
//}}MARK_DEBUG
		return 0;
	}

	if (~status & LP_PERRORP) {
		newerr = 3;
		if (status & LP_POUTPA) newerr = 1;
		if (~status & LP_PSELECD) newerr = 2;
	}

//{{MARK_DEBUG
//	if (newerr != errno)
//		dbg("usblp%d: %s", usblp->minor, usblp_messages[newerr]);
//}}MARK_DEBUG

	return newerr;
}

/*
 * File op functions.
 */

static int usblp_open( struct usblp *usblp )
{
	int retval;

//	lock_kernel();

	retval = -ENODEV;
	if (!usblp || !usblp->dev)
		goto out;

	retval = -EBUSY;
	if (usblp->used)
		goto out;

	/*
	 * TODO: need to implement LP_ABORTOPEN + O_NONBLOCK as in drivers/char/lp.c ???
	 * This is #if 0-ed because we *don't* want to fail an open
	 * just because the printer is off-line.
	 */
#if 0
	if ((retval = usblp_check_status(usblp, 0))) {
		retval = retval > 1 ? -EIO : -ENOSPC;
		goto out;
	}
#else
	retval = 0;	
#endif

	usblp->used = 1;
//	file->private_data = usblp;

	usblp->writeurb.transfer_buffer_length = 0;
	usblp->writeurb.status = 0;
#ifdef SUPPORT_PRN_COUNT
	if (usblp->bidir) {
		usblp->readcount = 0;
		usblp->readurb.dev = usblp->dev;
//		usblp->readurb.transfer_buffer_length = usb_maxpacket (usblp->readurb.dev, usblp->readurb.pipe, usb_pipeout (usblp->readurb.pipe));
		if( usb_submit_urb(&usblp->readurb) < 0 ) {
			retval = -EIO;
			usblp->used = 0;
		}
	}
#endif //SUPPORT_PRN_COUNT
out:
//	unlock_kernel();
	return retval;
}

static void usblp_cleanup (struct usblp *usblp)
{
//	devfs_unregister (usblp->devfs);

	cyg_semaphore_destroy(usblp->wait);	//ZOT716u2
	free(usblp->wait);	//ZOT716u2

	usblp_table [usblp->minor] = NULL;
//{{MARK_DEBUG
//	info ("usblp%d: removed", usblp->minor);
//}}MARK_DEBUG

//615wu	kfree (usblp->writeurb.transfer_buffer);
	kfree (usblp->device_id_string, 0);
	kfree (usblp, 0);
}

static int usblp_release( int nPort )
{
	struct usblp *usblp = usblp_table[nPort];

	usblp->used = 0;
	if (usblp->dev) {
		if (usblp->bidir)
			usb_unlink_urb(&usblp->readurb);
		usb_unlink_urb(&usblp->writeurb);
//		up(&usblp->sem);
	} else 		/* finish cleanup from disconnect */
		usblp_cleanup (usblp);
//	unlock_kernel();
	return 0;
}

/* No kernel lock - fine */
//Charles, fix me
#if 0
//static unsigned int usblp_poll(struct file *file, struct poll_table_struct *wait)
//{
//	struct usblp *usblp = file->private_data;
//	poll_wait(file, &usblp->wait, wait);
// 	return ((!usblp->bidir || usblp->readurb.status  == -EINPROGRESS) ? 0 : POLLIN  | POLLRDNORM)
// 			       | (usblp->writeurb.status == -EINPROGRESS  ? 0 : POLLOUT | POLLWRNORM);
//}

//static int usblp_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
//{
//	struct usblp *usblp = file->private_data;
//	int length, err;
//	unsigned char status;
//
//	if (_IOC_TYPE(cmd) == 'P')	/* new-style ioctl number */
//	
//		switch (_IOC_NR(cmd)) {
//
//			case IOCNR_GET_DEVICE_ID: /* get the DEVICE_ID string */
//				if (_IOC_DIR(cmd) != _IOC_READ)
//					return -EINVAL;
//
//				err = usblp_get_id(usblp, 0, usblp->device_id_string, DEVICE_ID_SIZE - 1);
//				if (err < 0) {
//					dbg ("usblp%d: error = %d reading IEEE-1284 Device ID string",
//						usblp->minor, err);
//					usblp->device_id_string[0] = usblp->device_id_string[1] = '\0';
//					return -EIO;
//				}
//
//				length = (usblp->device_id_string[0] << 8) + usblp->device_id_string[1]; /* big-endian */
//				if (length < DEVICE_ID_SIZE)
//					usblp->device_id_string[length] = '\0';
//				else
//					usblp->device_id_string[DEVICE_ID_SIZE - 1] = '\0';
//
//				dbg ("usblp%d Device ID string [%d/max %d]='%s'",
//					usblp->minor, length, _IOC_SIZE(cmd), &usblp->device_id_string[2]);
//
//				if (length > _IOC_SIZE(cmd)) length = _IOC_SIZE(cmd); /* truncate */
//
//				if (copy_to_user((unsigned char *) arg, usblp->device_id_string, (unsigned long) length))
//					return -EFAULT;
//
//				break;
//
//			default:
//				return -EINVAL;
//		}
//	else	/* old-style ioctl value */
//		switch (cmd) {
//
//			case LPGETSTATUS:
//				if (usblp_read_status(usblp, &status)) {
//					err("usblp%d: failed reading printer status", usblp->minor);
//					return -EIO;
//				}
//				if (copy_to_user ((unsigned char *)arg, &status, 1))
//					return -EFAULT;
//				break;
//
//			default:
//				return -EINVAL;
//		}
//
//	return 0;
//}
#endif

static int usblp_write( struct usblp *usblp, const char *buffer, size_t count, int *ppos)
{
	int timeout, errno = 0, writecount = 0;
	uint32 start;

//ZOT716u2	armond_printf("Start usblp_write\n");

	while (writecount < count) {
		// FIXME:  only use urb->status inside completion
		// callbacks; this way is racey...
		if (usblp->writeurb.status == -EINPROGRESS) {

//			if (file->f_flags & O_NONBLOCK)
//				return -EAGAIN;
#if 0	//ZOT716u2
			start = jiffies;
			timeout = USBLP_WRITE_TIMEOUT;

			while ( usblp->writeurb.status == -EINPROGRESS) {
	
//				if (signal_pending(current))
//					return writecount ? writecount : -EINTR;
				//cyg_thread_yield();
				
//				timeout = interruptible_sleep_on_timeout(&usblp->wait, timeout);
				
				if( jiffies - start > timeout ) break;
				
				cyg_thread_yield();
			}
#else	//ZOT716u2
			timeout = USBLP_WRITE_TIMEOUT;
			if (timeout && usblp->writeurb.status == -EINPROGRESS) {
				timeout = cyg_semaphore_timed_wait(usblp->wait, cyg_current_time() + ((timeout / MSPTICK)?(timeout / MSPTICK):1));
			}
#endif	//ZOT716u2			
		}

		if (!usblp->dev)
			return -ENODEV;

		if (usblp->writeurb.status != 0) {

//Print Server Recover Mechanism for OHCI	//635u2
			if (usblp->writeurb.status != -EINPROGRESS) {
				usblp->writeurb.status = 0;
				return writecount;
			}

			if (usblp->quirks & USBLP_QUIRK_BIDIR) {
//{{MARK_DEBUG
//				if (usblp->writeurb.status != -EINPROGRESS)
//					err("usblp%d: error %d writing to printer",
//						usblp->minor, usblp->writeurb.status);
//}}MARK_DEBUG
				errno = usblp->writeurb.status;
			}
			else {
//				errno = usblp_check_status(usblp, errno);
				errno = usblp->writeurb.status;
			}
			continue;
		}

		writecount += usblp->writeurb.transfer_buffer_length;
		usblp->writeurb.transfer_buffer_length = 0;

		if (writecount == count)
			continue;

		usblp->writeurb.transfer_buffer_length = (count - writecount) < USBLP_BUF_SIZE ?
							 (count - writecount) : USBLP_BUF_SIZE;

		copy_from_user(usblp->writeurb.transfer_buffer, buffer + writecount,
				usblp->writeurb.transfer_buffer_length);

		usblp->writeurb.dev = usblp->dev;
		usb_submit_urb(&usblp->writeurb);
//635u2
/*
#if defined(N635U2)|| defined(N635U2W)
		if(usblp->dev->ttport == 0)
		{
			if(usblp->dev->speed == USB_SPEED_HIGH)
				PrnLightToggle[1]++;
			else
				PrnLightToggle[3]++;	
		}
		else
		{
			if(usblp->dev->speed == USB_SPEED_HIGH)
				PrnLightToggle[2]++;
			else	
				PrnLightToggle[4]++;
		}
#else
		USBLightToggle++;
#endif	
*/	
#if 0	//ZOT716u2
		USBLightToggle++;
#else	//ZOT716u2
		if(usblp->dev->speed == USB_SPEED_HIGH)
			USB20LightToggle++;
		else
			USB11LightToggle++;
#endif	//ZOT716u2
	}

	return count;
}

static int usblp_read( struct usblp *usblp, char *buffer, size_t count, int *ppos)
{
	if (!usblp->bidir)
		return -EINVAL;

	if (usblp->readurb.status == -EINPROGRESS) {

//		if (file->f_flags & O_NONBLOCK)
//			return -EAGAIN;
#if 0	//ZOT716u2
		while (usblp->readurb.status == -EINPROGRESS) {
//			if (signal_pending(current))
//				return -EINTR;
			cyg_thread_yield();
//			interruptible_sleep_on(&usblp->wait);
		}
#else	//ZOT716u2
		while (usblp->readurb.status == -EINPROGRESS) {
			cyg_semaphore_wait(usblp->wait);
		}
#endif	//ZOT716u2		
	}

	if (!usblp->dev)
		return -ENODEV;

#ifdef SUPPORT_PRN_COUNT
	count = count < USBLP_BUF_SIZE ? count : USBLP_BUF_SIZE;

	copy_to_user(buffer, (uint8 *)usblp->readurb.transfer_buffer + usblp->readcount, count);
#else
	if (usblp->readurb.status) {
//{{MARK_DEBUG
//		err("usblp%d: error %d reading from printer",
//			usblp->minor, usblp->readurb.status);
//}}MARK_DEBUG
		usblp->readurb.dev = usblp->dev;
 		usblp->readcount = 0;
		usb_submit_urb(&usblp->readurb);
		return -EIO;
	}

	count = count < usblp->readurb.actual_length - usblp->readcount ?
		count :	usblp->readurb.actual_length - usblp->readcount;

	if (copy_to_user(buffer, (uint8 *)usblp->readurb.transfer_buffer + usblp->readcount, count))
		return -EFAULT;

#endif //SUPPORT_PRN_COUNT

	if ((usblp->readcount += count) == usblp->readurb.actual_length) {
		usblp->readcount = 0;
		usblp->readurb.dev = usblp->dev;
		usb_submit_urb(&usblp->readurb);
	}

	return count;
}

/*
 * Checks for printers that have quirks, such as requiring unidirectional
 * communication but reporting bidirectional; currently some HP printers
 * have this flaw (HP 810, 880, 895, etc.), or needing an init string
 * sent at each open (like some Epsons).
 * Returns 1 if found, 0 if not found.
 *
 * HP recommended that we use the bidirectional interface but
 * don't attempt any bulk IN transfers from the IN endpoint.
 * Here's some more detail on the problem:
 * The problem is not that it isn't bidirectional though. The problem
 * is that if you request a device ID, or status information, while
 * the buffers are full, the return data will end up in the print data
 * buffer. For example if you make sure you never request the device ID
 * while you are sending print data, and you don't try to query the
 * printer status every couple of milliseconds, you will probably be OK.
 */
static unsigned int usblp_quirks (__u16 vendor, __u16 product)
{
	int i;

	for (i = 0; quirk_printers[i].vendorId; i++) {
		if (vendor == quirk_printers[i].vendorId &&
		    product == quirk_printers[i].productId)
			return quirk_printers[i].quirks;
 	}
	return 0;
}

/*
static struct file_operations usblp_fops = {
	owner:		THIS_MODULE,
	read:		usblp_read,
	write:		usblp_write,
	poll:		usblp_poll,
	ioctl:		usblp_ioctl,
	open:		usblp_open,
	release:	usblp_release,
};
*/

static int usblp_select_alts(struct usb_device *dev, unsigned int ifnum);
static void *usblp_probe(struct usb_device *dev, unsigned int ifnum,const struct usb_device_id *id)
{
	struct usb_interface_descriptor *interface;
	struct usb_endpoint_descriptor *epread, *epwrite;
	struct usblp *usblp;
	int minor, bidir, quirks;
	int alts;
	int length, errno;
	char *buf,*buf1;
	char name[6];

//ZOT716u2	armond_printf("usblp probe\n");

	alts = usblp_select_alts(dev, ifnum);
	interface = &dev->actconfig->interface[ifnum].altsetting[alts];
	if (usb_set_interface(dev, ifnum, alts)) {
//{{MARK_DEBUG
//		err("can't set desired altsetting %d on interface %d", alts, ifnum);
//}}MARK_DEBUG
//ZOT716u2		armond_printf("usb_set_interface Error\n");
	}	
	bidir = (interface->bInterfaceProtocol > 1);

	epwrite = interface->endpoint + 0;
	epread = bidir ? interface->endpoint + 1 : NULL;

	if ((epwrite->bEndpointAddress & 0x80) == 0x80) {
		if (interface->bNumEndpoints == 1)
			return NULL;
		epwrite = interface->endpoint + 1;
		epread = bidir ? interface->endpoint + 0 : NULL;
	}

	if ((epwrite->bEndpointAddress & 0x80) == 0x80)
		return NULL;

	if (bidir && (epread->bEndpointAddress & 0x80) != 0x80)
		return NULL;

	for (minor = 0; minor < USBLP_MINORS && usblp_table[minor]; minor++);
	if (usblp_table[minor]) {
//{{MARK_DEBUG
//		err("no more free usblp devices");
//}}MARK_DEBUG
		return NULL;
	}

	if ((usblp = kmalloc(sizeof(struct usblp), GFP_KERNEL)) == NULL ) {
//{{MARK_DEBUG
//		err("out of memory");
//}}MARK_DEBUG
		return NULL;
	}
	memset(usblp, 0, sizeof(struct usblp));

	/* lookup quirks for this printer */
	quirks = usblp_quirks(dev->descriptor.idVendor, dev->descriptor.idProduct);

	if (bidir && (quirks & USBLP_QUIRK_BIDIR)) {
		bidir = 0;
		epread = NULL;
//{{MARK_DEBUG
//		dbg ("Disabling reads from problem bidirectional printer on usblp%d",
//			minor);
//}}MARK_DEBUG
	}

	usblp->dev = dev;
	usblp->ifnum = ifnum;
	usblp->minor = minor;
	usblp->bidir = bidir;
	usblp->quirks = quirks;

//	init_waitqueue_head(&usblp->wait);
	usblp->wait = malloc (sizeof(cyg_sem_t));	//ZOT716u2
	cyg_semaphore_init(usblp->wait, 0);		//ZOT716u2

//615wu
/*
	if ((buf = kmalloc(USBLP_BUF_SIZE * (bidir ? 2 : 1), GFP_KERNEL))==NULL) {
//{{MARK_DEBUG
//		err("out of memory");
//}}MARK_DEBUG
		kfree(usblp);
		return NULL;
	}
*/

//719AW	buf = USB_PRINTER_WRITE_BUFFER;
	buf = kaligned_alloc(8192, 4096);	//eason 20100407	
	
	if ((usblp->device_id_string = kmalloc(DEVICE_ID_SIZE, GFP_KERNEL))==NULL) {
//{{MARK_DEBUG
//		err("out of memory");
//}}MARK_DEBUG
		kfree(usblp, 0);
//615wu		kfree(buf);
		return NULL;
	}
	
//719AW	buf1 =	USB_PRINTER_READ_BUFFER;
	buf1 = kaligned_alloc(8192, 4096);	//eason 20100407

	FILL_BULK_URB(&usblp->writeurb, dev, usb_sndbulkpipe(dev, epwrite->bEndpointAddress),
		buf, 0, usblp_bulk, usblp);

	if (bidir)
//615WU		FILL_BULK_URB(&usblp->readurb, dev, usb_rcvbulkpipe(dev, epread->bEndpointAddress),
//			buf + USBLP_BUF_SIZE, USBLP_BUF_SIZE, usblp_bulk, usblp);
		FILL_BULK_URB(&usblp->readurb, dev, usb_rcvbulkpipe(dev, epread->bEndpointAddress),
			buf1, 256, usblp_bulk, usblp);

	/* Get the device_id string if possible. FIXME: Could make this kmalloc(length). */

	errno = usblp_get_id(usblp, 0, usblp->device_id_string, DEVICE_ID_SIZE - 1);
	if (errno >= 0) {
		length = (usblp->device_id_string[0] << 8) + usblp->device_id_string[1]; /* big-endian */
		if (length < DEVICE_ID_SIZE)
			usblp->device_id_string[length] = '\0';
		else
			usblp->device_id_string[DEVICE_ID_SIZE - 1] = '\0';
//{{MARK_DEBUG
//		dbg ("usblp%d Device ID string [%d]=%s",
//			minor, length, &usblp->device_id_string[2]);
//}}MARK_DEBUG
//ZOT716u2		armond_printf("usblp_get_id OK\n");
	}
	else {
//{{MARK_DEBUG
//		err ("usblp%d: error = %d reading IEEE-1284 Device ID string",
//			minor, errno);
//}}MARK_DEBUG
		usblp->device_id_string[0] = usblp->device_id_string[1] = '\0';
//ZOT716u2		armond_printf("usblp_get_id error:%d\n",errno);
	}


//#ifdef DEBUG
//	usblp_check_status(usblp, 0);
//#endif

//	sprintf(name, "lp%d", minor);
	
	/* Create with perms=664 */
//	usblp->devfs = devfs_register(usb_devfs_handle, name,
//				      DEVFS_FL_DEFAULT, USB_MAJOR,
//				      USBLP_MINOR_BASE + minor,
//				      S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
//				      S_IWGRP, &usblp_fops, NULL);
//	if (usblp->devfs == NULL)
//		err("usblp%d: device node registration failed", minor);

//{{MARK_DEBUG
//	dbg("usblp%d: USB %sdirectional printer dev %d if %d alt %d",
//		minor, bidir ? "Bi" : "Uni", dev->devnum, ifnum, alts);
//}}MARK_DEBUG

	usblp_table[minor] = usblp;

	usbprn_attach( minor );

	return usblp;
}

/*
 * We are a "new" style driver with usb_device_id table,
 * but our requirements are too intricate for simple match to handle.
 *
 * The best interface for us is 7/1/2, because it is compatible
 * with a stream of characters. If we find it, we bind to it.
 *
 *   [XXX] Note that the people from hpoj.sourceforge.net want to be
 *   able to bind to 7/1/3, we have to provide an ioctl for them.
 * 
 * Failing 7/1/2, we look for 7/1/3; the hope is that it was set
 * into a stream mode, also this matches the behaviour of old code.
 *
 * If nothing else, we bind to 7/1/1 - the unidirectional interface.
 */
static int usblp_select_alts(struct usb_device *dev, unsigned int ifnum)
{
	struct usb_interface *if_alt;
	struct usb_interface_descriptor *ifd;
	int alt_711, alt_712, alt_713;
	int i;

	if_alt = &dev->actconfig->interface[ifnum];

	/*
	 * Find what we have.
	 */
	alt_711 = alt_712 = alt_713 = -1; 
	for (i = 0; i < if_alt->num_altsetting; i++) {
		ifd = &if_alt->altsetting[i];

		if (ifd->bInterfaceClass != 7 || ifd->bInterfaceSubClass != 1)
			continue;

		if (ifd->bInterfaceProtocol > 1 && ifd->bNumEndpoints < 2)
			continue;		/* Buggy hardware */

		if (ifd->bInterfaceProtocol == 1)
			alt_711 = i;
		else if (ifd->bInterfaceProtocol == 2)
			alt_712 = i;
		else if (ifd->bInterfaceProtocol == 3)
			alt_713 = i;
	}

	/*
	 * Decide what we use.
	 */
	if (proto_bias == 1 && alt_711 != -1)
		return alt_711;
	if (proto_bias == 2 && alt_712 != -1)
		return alt_712;
	if (proto_bias == 3 && alt_713 != -1)
		return alt_713;

	/* Ordering is important here */
	if (alt_712 != -1)
		return alt_712;
	if (alt_713 != -1)
		return alt_713;
	if (alt_711 != -1)
		return alt_711;

	return if_alt->act_altsetting;		/* Never say never */
}

static void usblp_disconnect(struct usb_device *dev, void *ptr)
{
	struct usblp *usblp = ptr;

	usbprn_unattach( usblp->minor );

	if (!usblp || !usblp->dev) {
//{{MARK_DEBUG
//		err("disconnect on nonexisting interface");
//}}MARK_DEBUG
		return;
	}

	usblp->dev = NULL;

	usb_unlink_urb(&usblp->writeurb);
	if (usblp->bidir)
		usb_unlink_urb(&usblp->readurb);

	if( !usblp->used )
		usblp_cleanup (usblp);
}

static struct usb_device_id usblp_ids [] = {
	{ USB_DEVICE_INFO(7, 1, 1) },
	{ USB_DEVICE_INFO(7, 1, 2) },
	{ USB_DEVICE_INFO(7, 1, 3) },
	{ USB_INTERFACE_INFO(7, 1, 1) },
	{ USB_INTERFACE_INFO(7, 1, 2) },
	{ USB_INTERFACE_INFO(7, 1, 3) },
	{ }						/* Terminating entry */
};

//MODULE_DEVICE_TABLE (usb, usblp_ids);

static struct usb_driver usblp_driver = {
	/*name:*/	"usblp",
};

int usblp_init(void)
{
	usblp_driver.probe 		= usblp_probe;
	usblp_driver.disconnect = usblp_disconnect;
	usblp_driver.minor 		= USBLP_MINOR_BASE;
	usblp_driver.id_table	= usblp_ids;

	if (usb_register(&usblp_driver))
		return -1;
	return 0;
}

void usblp_exit(void)
{
	usb_deregister(&usblp_driver);
}

// Charles, export for 1284.c

uint8 usbprn_read_status( int nPort )
{
	if( PortIO[nPort].base && usblp_table[PortIO[nPort].base-1] )
	{
		struct usblp *usblp = usblp_table[PortIO[nPort].base-1];
		uint8  status;

//		if( usblp_read_status(usblp, &status) == 0 ){
//			if( status & PARPORT_STATUS_PAPEROUT ) 
//				return status;
//		}
		return 0x19;
	}
	// not connect
	return 0x7F;
}

int usbprn_attach( int minor )
{
	struct usblp *usblp = usblp_table[minor];
	int i1284 = NUM_OF_1284_PORT + usblp->dev->ttport;
	char *tmp;

//ZOT716u2	armond_printf("usblp attach\n");

	if( PortIO[i1284].base == 0 )
	{
		// we only attach one printer per port
		PortIO[i1284].base = minor + 1;
		PortIO[i1284].HasGetDeviceID = 1;
		PortIO[i1284].PrnMode = PRN_USB_PRINTER;
		PortIO[i1284].PrnReadBackMode = PRN_USB_PRINTER;

		tmp = malloc( LENGTH_OF_DEVICE_ID );
		if( tmp )
		{
			memcpy( tmp, usblp->device_id_string, LENGTH_OF_DEVICE_ID );
			parse_device_id( i1284, &tmp[2] );
			free( tmp );
		}
	
		return 1;
	}
	return 0;
}

int usbprn_unattach( int minor )
{
	struct usblp *usblp = usblp_table[minor];
	int i1284 = NUM_OF_1284_PORT + usblp->dev->ttport;

	if( PortIO[i1284].base && PortIO[i1284].base-1 == minor )
	{
		PortIO[i1284].base = 0;
		PortIO[i1284].HasGetDeviceID = 0;
		PortIO[i1284].SupportLang = 0;
		PortIO[i1284].PrnMode = PRN_NO_PRINTER;
		PortIO[i1284].PrnReadBackMode = PRN_NO_PRINTER;
		PrnFreeDeviceInfo( i1284 );

		return 1;
	}
	return 0;
}

int usbprn_open(int nPort)
{
	if( PortIO[nPort].base && usblp_table[PortIO[nPort].base-1] )
		usblp_open(	usblp_table[PortIO[nPort].base-1] );
	return 0;
}

int usbprn_close(int nPort)
{
	if( PortIO[nPort].base && usblp_table[PortIO[nPort].base-1] )
		usblp_release( usblp_table[PortIO[nPort].base-1]->minor );
	return 0;
}

int usbprn_write(int nPort, uint8 *pBuf, int nLength)
{
	if( PortIO[nPort].base && usblp_table[PortIO[nPort].base-1] )
		usblp_write( usblp_table[PortIO[nPort].base-1], pBuf, nLength, NULL );
	return nLength;
}

int usbprn_read(int nPort, uint8 *pBuf, int nLength)
{
	if( PortIO[nPort].base && usblp_table[PortIO[nPort].base-1] )
		usblp_read( usblp_table[PortIO[nPort].base-1], pBuf, nLength, NULL );
	return nLength;
}

//module_init(usblp_init);
//module_exit(usblp_exit);

//MODULE_AUTHOR("Michael Gee, Pavel Machek, Vojtech Pavlik, Randy Dunlap");
//MODULE_DESCRIPTION("USB Printer Device Class driver");
