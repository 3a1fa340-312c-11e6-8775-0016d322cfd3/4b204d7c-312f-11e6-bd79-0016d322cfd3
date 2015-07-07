/*
 * printer.c  Version 0.13
 *
 * Copyright (c) 1999 Michael Gee	<michael@linuxspecific.com>
 * Copyright (c) 1999 Pavel Machek	<pavel@suse.cz>
 * Copyright (c) 2000 Randy Dunlap	<rddunlap@osdl.org>
 * Copyright (c) 2000 Vojtech Pavlik	<vojtech@suse.cz>
 # Copyright (c) 2001 Pete Zaitcev	<zaitcev@redhat.com>
 # Copyright (c) 2001 David Paschal	<paschal@rcsis.com>
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
 *	v0.7 - fixed bulk-IN read and poll (David Paschal)
 *	v0.8 - add devfs support
 *	v0.9 - fix unplug-while-open paths
 *	v0.10- remove sleep_on, fix error on oom (oliver@neukum.org)
 *	v0.11 - add proto_bias option (Pete Zaitcev)
 *	v0.12 - add hpoj.sourceforge.net ioctls (David Paschal)
 *	v0.13 - alloc space for statusbuf (<status> not on stack);
 *		use usb_buffer_alloc() for read buf & write buf;
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

int IS_HP_GDI_PRINTER_HP_1020 = 0;
int IS_SHARP_AR_M207 = 0;
int IS_SET_AR_M207_CMD = 0;

#define USB_PRINTER_WRITE_BUFFER	USBPRN_WRITE_BUFFER	//ZOT716u2 //8K
#define USB_PRINTER_READ_BUFFER  	USBPRN_READ_BUFFER	//ZOT716u2 //256byte
#define usb_fill_bulk_urb(a,aa,b,c,d,e,f)	FILL_BULK_URB(a,aa,b,c,d,e,f)	

extern struct parport 	PortIO[NUM_OF_PRN_PORT];

/* 
 * bit defines for 8255 status port
 * base + 1
 * accessed with LP_S(minor), which gets the byte...
 */
#define LP_PBUSY	0x80  /* inverted input, active high */
#define LP_PACK		0x40  /* unchanged input, active low */
#define LP_POUTPA	0x20  /* unchanged input, active high */
#define LP_PSELECD	0x10  /* unchanged input, active high */
#define LP_PERRORP	0x08  /* unchanged input, active low */

#define USBLP_BUF_SIZE		8192
#define USBLP_DEVICE_ID_SIZE		LENGTH_OF_DEVICE_ID

/* ioctls: */
#define LPGETSTATUS		0x060b		/* same as in drivers/char/lp.c */
#define IOCNR_GET_DEVICE_ID		1
#define IOCNR_GET_PROTOCOLS		2
#define IOCNR_SET_PROTOCOL		3
#define IOCNR_HP_SET_CHANNEL		4
#define IOCNR_GET_BUS_ADDRESS		5
#define IOCNR_GET_VID_PID		6
/* Get device_id string: */
#define LPIOC_GET_DEVICE_ID(len) _IOC(_IOC_READ, 'P', IOCNR_GET_DEVICE_ID, len)
/* The following ioctls were added for http://hpoj.sourceforge.net: */
/* Get two-int array:
 * [0]=current protocol (1=7/1/1, 2=7/1/2, 3=7/1/3),
 * [1]=supported protocol mask (mask&(1<<n)!=0 means 7/1/n supported): */
#define LPIOC_GET_PROTOCOLS(len) _IOC(_IOC_READ, 'P', IOCNR_GET_PROTOCOLS, len)
/* Set protocol (arg: 1=7/1/1, 2=7/1/2, 3=7/1/3): */
#define LPIOC_SET_PROTOCOL _IOC(_IOC_WRITE, 'P', IOCNR_SET_PROTOCOL, 0)
/* Set channel number (HP Vendor-specific command): */
#define LPIOC_HP_SET_CHANNEL _IOC(_IOC_WRITE, 'P', IOCNR_HP_SET_CHANNEL, 0)
/* Get two-int array: [0]=bus number, [1]=device address: */
#define LPIOC_GET_BUS_ADDRESS(len) _IOC(_IOC_READ, 'P', IOCNR_GET_BUS_ADDRESS, len)
/* Get two-int array: [0]=vendor ID, [1]=product ID: */
#define LPIOC_GET_VID_PID(len) _IOC(_IOC_READ, 'P', IOCNR_GET_VID_PID, len)

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
#define USBLP_REQ_HP_CHANNEL_CHANGE_REQUEST	0x00	/* HP Vendor-specific */

#define USBLP_MINORS		NUM_OF_USB_PRN_DEVICE
#define USBLP_MINOR_BASE	0

#define USBLP_WRITE_TIMEOUT	(5*HZ)			/* 5 seconds */

#define USBLP_FIRST_PROTOCOL	1
#define USBLP_LAST_PROTOCOL	3
#define USBLP_MAX_PROTOCOLS	(USBLP_LAST_PROTOCOL+1)

/*
 * some arbitrary status buffer size;
 * need a status buffer that is allocated via kmalloc(), not on stack
 */
#define STATUS_BUF_SIZE		8

struct usblp {
	struct usb_device 	*dev;			/* USB device */
//ZOT	devfs_handle_t		devfs;			/* devfs device */
//ZOT	struct semaphore	sem;			/* locks this struct, especially "dev" */
	char			*writebuf;		/* write transfer_buffer */
	char			*readbuf;		/* read transfer_buffer */
	char			*statusbuf;		/* status transfer_buffer */
	struct urb		*readurb, *writeurb;	/* The urbs */
//ZOT	wait_queue_head_t	wait;			/* Zzzzz ... */
	void	* write_wait;		//ZOT716u2
	void	* read_wait;		//ZOT716u2
	int			readcount;		/* Counter for reads */
	int			ifnum;			/* Interface number */
	/* Alternate-setting numbers and endpoints for each protocol
	 * (7/1/{index=1,2,3}) that the device supports: */
	struct {
		int				alt_setting;
		struct usb_endpoint_descriptor	*epwrite;
		struct usb_endpoint_descriptor	*epread;
	}			protocol[USBLP_MAX_PROTOCOLS];
	int			current_protocol;
	int			minor;			/* minor number of device */
	int			wcomplete;		/* writing is completed */
	int			rcomplete;		/* reading is completed */
	unsigned int		quirks;			/* quirks flags */
	unsigned char		used;			/* True if open */
	unsigned char		present;		/* True if not disconnected */
	unsigned char		bidir;			/* interface is bidirectional */
	unsigned char		*device_id_string;	/* IEEE 1284 DEVICE ID string (ptr) */
										/* first 2 bytes are (big-endian) length */
};

#ifdef DEBUG
static void usblp_dump(struct usblp *usblp) {
	int p;

	dbg("usblp=0x%p", usblp);
	dbg("dev=0x%p", usblp->dev);
	dbg("present=%d", usblp->present);
	dbg("readbuf=0x%p", usblp->readbuf);
	dbg("writebuf=0x%p", usblp->writebuf);
	dbg("readurb=0x%p", usblp->readurb);
	dbg("writeurb=0x%p", usblp->writeurb);
	dbg("readcount=%d", usblp->readcount);
	dbg("ifnum=%d", usblp->ifnum);
    for (p = USBLP_FIRST_PROTOCOL; p <= USBLP_LAST_PROTOCOL; p++) {
	dbg("protocol[%d].alt_setting=%d", p, usblp->protocol[p].alt_setting);
	dbg("protocol[%d].epwrite=%p", p, usblp->protocol[p].epwrite);
	dbg("protocol[%d].epread=%p", p, usblp->protocol[p].epread);
    }
	dbg("current_protocol=%d", usblp->current_protocol);
	dbg("minor=%d", usblp->minor);
	dbg("wcomplete=%d", usblp->wcomplete);
	dbg("rcomplete=%d", usblp->rcomplete);
	dbg("quirks=%d", usblp->quirks);
	dbg("used=%d", usblp->used);
	dbg("bidir=%d", usblp->bidir);
	dbg("device_id_string=\"%s\"",
		usblp->device_id_string ?
			usblp->device_id_string + 2 :
			(unsigned char *)"(null)");
}
#endif

//ZOT	extern devfs_handle_t usb_devfs_handle;			/* /dev/usb dir. */

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
	{ 0x03f0, 0x0504, USBLP_QUIRK_BIDIR }, /* HP DeskJet 885C */
	{ 0x03f0, 0x0604, USBLP_QUIRK_BIDIR }, /* HP DeskJet 840C */   
	{ 0x03f0, 0x0804, USBLP_QUIRK_BIDIR }, /* HP DeskJet 816C */   
	{ 0x03f0, 0x1104, USBLP_QUIRK_BIDIR }, /* HP Deskjet 959C */
	{ 0x0409, 0xefbe, USBLP_QUIRK_BIDIR }, /* NEC Picty900 (HP OEM) */
	{ 0x0409, 0xbef4, USBLP_QUIRK_BIDIR }, /* NEC Picty760 (HP OEM) */
	{ 0x0409, 0xf0be, USBLP_QUIRK_BIDIR }, /* NEC Picty920 (HP OEM) */
	{ 0x0409, 0xf1be, USBLP_QUIRK_BIDIR }, /* NEC Picty800 (HP OEM) */
	{ 0, 0 }
};
 
static int usblp_select_alts(struct usblp *usblp);
static int usblp_set_protocol(struct usblp *usblp, int protocol);
static int usblp_cache_device_id_string(struct usblp *usblp);

//ZOT static DECLARE_MUTEX(usblp_sem);	/* locks the existence of usblp's. */

/*
 * Functions for usblp control messages.
 */

static int usblp_ctrl_msg(struct usblp *usblp, int request, int type, int dir, int recip, int value, void *buf, int len)
{
	int retval;
	int index = usblp->ifnum;

	/* High byte has the interface index.
	   Low byte has the alternate setting.
	 */
	if ((request == USBLP_REQ_GET_ID) && (type == USB_TYPE_CLASS)) {
	  index = (usblp->ifnum<<8)|usblp->protocol[usblp->current_protocol].alt_setting;
	}

	retval = usb_control_msg(usblp->dev,
		dir ? usb_rcvctrlpipe(usblp->dev, 0) : usb_sndctrlpipe(usblp->dev, 0),
		request, type | dir | recip, value, index, buf, len, USBLP_WRITE_TIMEOUT);
//ZOT	dbg("usblp_control_msg: rq: 0x%02x dir: %d recip: %d value: %d idx: %d len: %#x result: %d",
//ZOT		request, !!dir, recip, value, index, len, retval);
	return retval < 0 ? retval : 0;
}

#define usblp_read_status(usblp, status)\
	usblp_ctrl_msg(usblp, USBLP_REQ_GET_STATUS, USB_TYPE_CLASS, USB_DIR_IN, USB_RECIP_INTERFACE, 0, status, 1)
#define usblp_get_id(usblp, config, id, maxlen)\
	usblp_ctrl_msg(usblp, USBLP_REQ_GET_ID, USB_TYPE_CLASS, USB_DIR_IN, USB_RECIP_INTERFACE, config, id, maxlen)
#define usblp_reset(usblp)\
	usblp_ctrl_msg(usblp, USBLP_REQ_RESET, USB_TYPE_CLASS, USB_DIR_OUT, USB_RECIP_OTHER, 0, NULL, 0)

#define usblp_hp_channel_change_request(usblp, channel, buffer) \
	usblp_ctrl_msg(usblp, USBLP_REQ_HP_CHANNEL_CHANGE_REQUEST, USB_TYPE_VENDOR, USB_DIR_IN, USB_RECIP_INTERFACE, channel, buffer, 1)

/*
 * See the description for usblp_select_alts() below for the usage
 * explanation.  Look into your /proc/bus/usb/devices and dmesg in
 * case of any trouble.
 */
static int proto_bias = -1;

/*
 * URB callback.
 */

static void usblp_bulk_read(struct urb *urb)
{
	struct usblp *usblp = urb->context;

	if (!usblp || !usblp->dev || !usblp->used || !usblp->present)
		return;

//ZOT	if (unlikely(urb->status))
//ZOT		warn("usblp%d: nonzero read/write bulk status received: %d",
//ZOT			usblp->minor, urb->status);
	usblp->rcomplete = 1;
	
//ZOT	wake_up_interruptible(&usblp->wait);
	cyg_semaphore_post(usblp->read_wait);	//ZOT716u2

}

static void usblp_bulk_write(struct urb *urb)
{
	struct usblp *usblp = urb->context;

	if (!usblp || !usblp->dev || !usblp->used || !usblp->present)
		return;

//ZOT	if (unlikely(urb->status))
//ZOT		warn("usblp%d: nonzero read/write bulk status received: %d",
//ZOT			usblp->minor, urb->status);
	usblp->wcomplete = 1;

//ZOT	wake_up_interruptible(&usblp->wait);
	cyg_semaphore_post(usblp->write_wait);	//ZOT716u2
}

/*
 * Get and print printer errors.
 */

static char *usblp_messages[] = { "ok", "out of paper", "off-line", "on fire" };

static int usblp_check_status(struct usblp *usblp, int err)
{
	unsigned char status, newerr = 0;
	int error;

	error = usblp_read_status (usblp, usblp->statusbuf);
	if (error < 0) {
//ZOT		err("usblp%d: error %d reading printer status",
//ZOT			usblp->minor, error);
		return 0;
	}

	status = *usblp->statusbuf;

	if (~status & LP_PERRORP)
		newerr = 3;
	if (status & LP_POUTPA)
		newerr = 1;
	if (~status & LP_PSELECD)
		newerr = 2;

//ZOT	if (newerr != err)
//ZOT		info("usblp%d: %s", usblp->minor, usblp_messages[newerr]);

	return newerr;
}

unsigned char AR_M207_CLEAR_DATA_1[6] = { 0x81, 0x81, 0x00, 0x00, 0x00, 0x06 };
unsigned char AR_M207_CLEAR_DATA_2[6] = { 0x81, 0x81, 0x00, 0x00, 0x00, 0x03 };
unsigned char AR_M207_GET_STATUS[6] = { 0x81, 0x81, 0x00, 0x00, 0x00, 0x0F };
unsigned char AR_M207_STATUS_OK[16] =  { 0x81, 0x31, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00};


//ZOT static int usblp_open(struct inode *inode, struct file *file)
int usblp_open( struct usblp *usblp )
{
//ZOT	int minor = MINOR(inode->i_rdev) - USBLP_MINOR_BASE;
//ZOT	struct usblp *usblp;
	int retval,len = 0;
	char write_gdi[512]={0};
	char *pBuf;
	int total_len = 0, loop_cnt = 0;
	int i1284 = NUM_OF_1284_PORT + usblp->dev->printer_port;

//ZOT	if (minor < 0 || minor >= USBLP_MINORS)
//ZOT		return -ENODEV;

//ZOT	down (&usblp_sem);
//ZOT	usblp  = usblp_table[minor];

	retval = -ENODEV;
	if (!usblp || !usblp->dev || !usblp->present)
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
//ZOT	file->private_data = usblp;

	usblp->writeurb->transfer_buffer_length = 0;
	usblp->wcomplete = 1; /* we begin writeable */
	usblp->rcomplete = 0;

#ifdef SUPPORT_PRN_COUNT
	if (usblp->bidir) {
		usblp->readcount = 0;
		usblp->readurb->dev = usblp->dev;
		if (usb_submit_urb(usblp->readurb) < 0) {
			retval = -EIO;
			usblp->used = 0;
			file->private_data = NULL;
		}
	}
#endif //SUPPORT_PRN_COUNT
	
	if(    (IS_SHARP_AR_M207 == 1) 
		&& (PortIO[i1284].DataPtr[0] == 0x81) 
		&& (PortIO[i1284].DataPtr[1] == 0x81)
		)
	{
		pBuf = AR_M207_GET_STATUS;
		total_len = sizeof(AR_M207_GET_STATUS);
		while(1)
		{
			if (!usblp || !usblp->dev || !usblp->present)
				break;
			
			usblp_write(usblp, pBuf, total_len,NULL);
			ppause(1000);
				
			usblp->readurb->dev = usblp->dev;
	 		usblp->readcount = 0;
			usblp->rcomplete = 0;
						
			if (!usblp || !usblp->dev || !usblp->present)
				break;
			
			usb_submit_urb(usblp->readurb);
			memset(write_gdi, 0, 512);
			usblp_read( usblp, write_gdi, 16, NULL);
			if( memcmp(write_gdi,AR_M207_STATUS_OK,16) == 0 )
				break;
			ppause(1000);
		}
		
		IS_SET_AR_M207_CMD = 1;
		
		for(loop_cnt = 0; loop_cnt < 6 ; loop_cnt ++)
		{
			if(    (PortIO[i1284].DataPtr[0] == 0x81) 
				&& (PortIO[i1284].DataPtr[1] == 0x81)
				&& (PortIO[i1284].DataPtr[5] == 0x01)
				)
			{
				break;
			}
			
			if(    (PortIO[i1284].DataPtr[0] == 0x81) 
				&& (PortIO[i1284].DataPtr[1] == 0x81)
				)
			{
				memset(write_gdi, 0, 512);
				memcpy(write_gdi, PortIO[usblp->minor].DataPtr, 6);
				PortIO[i1284].DataPtr += 6;
				PortIO[i1284].DataSize -= 6;
				memcpy(write_gdi+6, PortIO[usblp->minor].DataPtr, write_gdi[3]);
				PortIO[i1284].DataPtr += write_gdi[3];
				PortIO[i1284].DataSize -= write_gdi[3];
				usblp_write(usblp, write_gdi, 6 + write_gdi[3] ,NULL);
				ppause(2000);
			}
		
		}
		

	}

	
out:

//ZOT	up (&usblp_sem);
	return retval;
}

static void usblp_cleanup (struct usblp *usblp)
{
//ZOT	devfs_unregister (usblp->devfs);
	cyg_semaphore_destroy(usblp->write_wait);	//ZOT716u2
	cyg_semaphore_destroy(usblp->read_wait);	//ZOT716u2
	free(usblp->write_wait);	//ZOT716u2
	free(usblp->read_wait);	//ZOT716u2	

	usblp_table [usblp->minor] = NULL;
//ZOT	info("usblp%d: removed", usblp->minor);

//ZOT	kfree (usblp->writebuf);
//ZOT	kfree (usblp->readbuf);
	kaligned_free(usblp->writebuf);
	kaligned_free(usblp->readbuf);
	kfree (usblp->device_id_string, 0);
	kfree (usblp->statusbuf, 0);
	usb_free_urb(usblp->writeurb);
	usb_free_urb(usblp->readurb);
	kfree (usblp, 0);
}

static void usblp_unlink_urbs(struct usblp *usblp)
{
	usb_unlink_urb(usblp->writeurb);
	if (usblp->bidir)
		usb_unlink_urb(usblp->readurb);
}

//ZOT static int usblp_release(struct inode *inode, struct file *file)
static int usblp_release( int nPort )
{
	int len = 0;
	char write_gdi[512]={0};
	char *pBuf ;
	int total_len = 0;
//ZOT	struct usblp *usblp = file->private_data;
	struct usblp *usblp = usblp_table[nPort];

	if( (IS_SHARP_AR_M207 == 1) && (IS_SET_AR_M207_CMD == 1 ) )
	{
		IS_SET_AR_M207_CMD = 0;

		pBuf = AR_M207_CLEAR_DATA_1;
		total_len = sizeof(AR_M207_CLEAR_DATA_1);
		usblp_write(usblp, pBuf, total_len,NULL);
		
		ppause(500);
		
		pBuf = AR_M207_CLEAR_DATA_2;
		total_len = sizeof(AR_M207_CLEAR_DATA_2);
		usblp_write(usblp, pBuf, total_len,NULL);
		
		ppause(500);
		
		pBuf = AR_M207_GET_STATUS;
		total_len = sizeof(AR_M207_GET_STATUS);
		while(1)
		{
			if (!usblp || !usblp->dev || !usblp->present)
				break;
			usblp_write(usblp, pBuf, total_len,NULL);
		
			usblp->readurb->dev = usblp->dev;
	 		usblp->readcount = 0;
			usblp->rcomplete = 0;
			if (!usblp || !usblp->dev || !usblp->present)
				break;
			ppause(1000);
			
			usb_submit_urb(usblp->readurb);
			memset(write_gdi,0,512);
			usblp_read( usblp, write_gdi, 16, NULL);
			if( memcmp(write_gdi,AR_M207_STATUS_OK,16)==0)
			{
				ppause(5000);
				break;
			}
			ppause(1000);
			
		}
	}
	
//ZOT	down (&usblp_sem);
	usblp->used = 0;
	if (usblp->present) {
		usblp_unlink_urbs(usblp);
	} else 		/* finish cleanup from disconnect */
		usblp_cleanup (usblp);
//ZOT	up (&usblp_sem);
	return 0;
}

//ZOT 
#if 0
/* No kernel lock - fine */
static unsigned int usblp_poll(struct file *file, struct poll_table_struct *wait)
{
	struct usblp *usblp = file->private_data;
	poll_wait(file, &usblp->wait, wait);
 	return ((!usblp->bidir || !usblp->rcomplete) ? 0 : POLLIN  | POLLRDNORM)
 			       | (!usblp->wcomplete ? 0 : POLLOUT | POLLWRNORM);
}

static int usblp_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct usblp *usblp = file->private_data;
	int length, err, i;
	unsigned char newChannel;
	int status;
	int twoints[2];
	int retval = 0;

	down (&usblp->sem);
	if (!usblp->present) {
		retval = -ENODEV;
		goto done;
	}

	if (_IOC_TYPE(cmd) == 'P')	/* new-style ioctl number */

		switch (_IOC_NR(cmd)) {

			case IOCNR_GET_DEVICE_ID: /* get the DEVICE_ID string */
				if (_IOC_DIR(cmd) != _IOC_READ) {
					retval = -EINVAL;
					goto done;
				}

				length = usblp_cache_device_id_string(usblp);
				if (length < 0) {
					retval = length;
					goto done;
				}
				if (length > _IOC_SIZE(cmd))
					length = _IOC_SIZE(cmd); /* truncate */

				if (copy_to_user((unsigned char *) arg,
						usblp->device_id_string,
						(unsigned long) length)) {
					retval = -EFAULT;
					goto done;
				}

				break;

			case IOCNR_GET_PROTOCOLS:
				if (_IOC_DIR(cmd) != _IOC_READ ||
				    _IOC_SIZE(cmd) < sizeof(twoints)) {
					retval = -EINVAL;
					goto done;
				}

				twoints[0] = usblp->current_protocol;
				twoints[1] = 0;
				for (i = USBLP_FIRST_PROTOCOL;
				     i <= USBLP_LAST_PROTOCOL; i++) {
					if (usblp->protocol[i].alt_setting >= 0)
						twoints[1] |= (1<<i);
				}

				if (copy_to_user((unsigned char *)arg,
						(unsigned char *)twoints,
						sizeof(twoints))) {
					retval = -EFAULT;
					goto done;
				}

				break;

			case IOCNR_SET_PROTOCOL:
				if (_IOC_DIR(cmd) != _IOC_WRITE) {
					retval = -EINVAL;
					goto done;
				}

#ifdef DEBUG
				if (arg == -10) {
					usblp_dump(usblp);
					break;
				}
#endif

				usblp_unlink_urbs(usblp);
				retval = usblp_set_protocol(usblp, arg);
				if (retval < 0) {
					usblp_set_protocol(usblp,
						usblp->current_protocol);
				}
				break;

			case IOCNR_HP_SET_CHANNEL:
				if (_IOC_DIR(cmd) != _IOC_WRITE ||
				    usblp->dev->descriptor.idVendor != 0x03F0 ||
				    usblp->quirks & USBLP_QUIRK_BIDIR) {
					retval = -EINVAL;
					goto done;
				}

				err = usblp_hp_channel_change_request(usblp,
					arg, &newChannel);
				if (err < 0) {
					err("usblp%d: error = %d setting "
						"HP channel",
						usblp->minor, err);
					retval = -EIO;
					goto done;
				}

				dbg("usblp%d requested/got HP channel %ld/%d",
					usblp->minor, arg, newChannel);
				break;

			case IOCNR_GET_BUS_ADDRESS:
				if (_IOC_DIR(cmd) != _IOC_READ ||
				    _IOC_SIZE(cmd) < sizeof(twoints)) {
					retval = -EINVAL;
					goto done;
				}

				twoints[0] = usblp->dev->bus->busnum;
				twoints[1] = usblp->dev->devnum;
				if (copy_to_user((unsigned char *)arg,
						(unsigned char *)twoints,
						sizeof(twoints))) {
					retval = -EFAULT;
					goto done;
				}

				dbg("usblp%d is bus=%d, device=%d",
					usblp->minor, twoints[0], twoints[1]);
				break;

			case IOCNR_GET_VID_PID:
				if (_IOC_DIR(cmd) != _IOC_READ ||
				    _IOC_SIZE(cmd) < sizeof(twoints)) {
					retval = -EINVAL;
					goto done;
				}

				twoints[0] = usblp->dev->descriptor.idVendor;
				twoints[1] = usblp->dev->descriptor.idProduct;
				if (copy_to_user((unsigned char *)arg,
						(unsigned char *)twoints,
						sizeof(twoints))) {
					retval = -EFAULT;
					goto done;
				}

				dbg("usblp%d is VID=0x%4.4X, PID=0x%4.4X",
					usblp->minor, twoints[0], twoints[1]);
				break;

			default:
				retval = -ENOTTY;
		}
	else	/* old-style ioctl value */
		switch (cmd) {

			case LPGETSTATUS:
				if (usblp_read_status(usblp, usblp->statusbuf)) {
					err("usblp%d: failed reading printer status", usblp->minor);
					retval = -EIO;
					goto done;
				}
				status = *usblp->statusbuf;
				if (copy_to_user ((int *)arg, &status, sizeof(int)))
					retval = -EFAULT;
				break;

			default:
				retval = -ENOTTY;
		}

done:
	up (&usblp->sem);
	return retval;
}
#endif //0 //ZOT

//ZOT static ssize_t usblp_write(struct file *file, const char *buffer, size_t count, loff_t *ppos)
static int usblp_write( struct usblp *usblp, const char *buffer, size_t count, int *ppos)
{
//ZOT	DECLARE_WAITQUEUE(wait, current);
//ZOT	struct usblp *usblp = file->private_data;
	int timeout, err = 0, transfer_length = 0;
	size_t writecount = 0;

	while (writecount < count) {
		if (!usblp->wcomplete) {
//ZOT			barrier();
//ZOT			if (file->f_flags & O_NONBLOCK) {
//ZOT				writecount += transfer_length;
//ZOT				return writecount ? writecount : -EAGAIN;
//ZOT			}

			timeout = USBLP_WRITE_TIMEOUT;
#if 0
			add_wait_queue(&usblp->wait, &wait);
			while ( 1==1 ) {

				if (signal_pending(current)) {
					remove_wait_queue(&usblp->wait, &wait);
					return writecount ? writecount : -EINTR;
				}
				set_current_state(TASK_INTERRUPTIBLE);
				if (timeout && !usblp->wcomplete) {
					timeout = schedule_timeout(timeout);
				} else {
					set_current_state(TASK_RUNNING);
					break;
				}
			}
			remove_wait_queue(&usblp->wait, &wait);
#else
			
			if (timeout && !usblp->wcomplete) {
				timeout = cyg_semaphore_timed_wait(usblp->write_wait, cyg_current_time() + ((timeout / MSPTICK)?(timeout / MSPTICK):1));
			}

#endif //0
			
		}

//ZOT		down (&usblp->sem);
		if (!usblp->present) {
//ZOT			up (&usblp->sem);
			return -ENODEV;
		}

		if (usblp->writeurb->status != 0) {

//Print Server Recover Mechanism for OHCI	//635u2
			if (usblp->writeurb->status != -EINPROGRESS) {
				usblp->writeurb->status = 0;
				return writecount;
			}

			if (usblp->quirks & USBLP_QUIRK_BIDIR) {
//ZOT				if (!usblp->wcomplete)
//ZOT					err("usblp%d: error %d writing to printer",
//ZOT						usblp->minor, usblp->writeurb->status);
				err = usblp->writeurb->status;
			} else
				err = usblp_check_status(usblp, err);
//ZOT			up (&usblp->sem);

			/* if the fault was due to disconnect, let khubd's
			 * call to usblp_disconnect() grab usblp->sem ...
			 */
//ZOT			schedule ();
			continue;
		}

		/* We must increment writecount here, and not at the
		 * end of the loop. Otherwise, the final loop iteration may
		 * be skipped, leading to incomplete printer output.
		 */
		writecount += transfer_length;
		if (writecount == count) {
//ZOT			up (&usblp->sem);
			break;
		}

		transfer_length = count - writecount;
		if(transfer_length > USBLP_BUF_SIZE) 
			transfer_length = USBLP_BUF_SIZE;
		
		usblp->writeurb->transfer_buffer_length = transfer_length;

		if (copy_from_user(usblp->writeurb->transfer_buffer, 
				   buffer + writecount, transfer_length)) {
//ZOT			up(&usblp->sem);
//ZOT			return writecount ? writecount : -EFAULT;
		}

		usblp->writeurb->dev = usblp->dev;
		usblp->wcomplete = 0;
		err = usb_submit_urb(usblp->writeurb);
		if (err) {
			usblp->wcomplete = 1;
			if (err != -ENOMEM)
				count = -EIO;
			else
				count = writecount ? writecount : -ENOMEM;
//ZOT			up (&usblp->sem);
			break;
		}
//ZOT		up (&usblp->sem);
		
		if(usblp->dev->speed == USB_SPEED_HIGH)
			USB20LightToggle++;
		else
			USB11LightToggle++;
	}

	return count;
}

//ZOT static ssize_t usblp_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
static int usblp_read( struct usblp *usblp, char *buffer, size_t count, int *ppos)
{
//ZOT	struct usblp *usblp = file->private_data;
//ZOT	DECLARE_WAITQUEUE(wait, current);
	int timeout;
		
	if (!usblp->bidir)
		return -EINVAL;

//ZOT	down (&usblp->sem);
	if (!usblp->present) {
		count = -ENODEV;
		goto done;
	}

	if (!usblp->rcomplete) {
//ZOT		barrier();

//ZOT		if (file->f_flags & O_NONBLOCK) {
//ZOT			count = -EAGAIN;
//ZOT			goto done;
//ZOT		}

//ZOT
#if 0
		add_wait_queue(&usblp->wait, &wait);
		while (1==1) {
			if (signal_pending(current)) {
				count = -EINTR;
				remove_wait_queue(&usblp->wait, &wait);
				goto done;
			}
			up (&usblp->sem);
			set_current_state(TASK_INTERRUPTIBLE);
			if (!usblp->rcomplete) {
				schedule();
			} else {
				set_current_state(TASK_RUNNING);
				down (&usblp->sem);
				break;
			}
			down (&usblp->sem);
		}
		remove_wait_queue(&usblp->wait, &wait);
#else

//	while (usblp->readurb->status == -EINPROGRESS) {
//			cyg_semaphore_wait(usblp->read_wait);
//		}
		timeout = USBLP_WRITE_TIMEOUT;
		
		if (timeout && !usblp->rcomplete) {
			timeout = cyg_semaphore_timed_wait(usblp->read_wait, cyg_current_time() + ((timeout / MSPTICK)?(timeout / MSPTICK):1));
		}
		
#endif //0 //ZOT
			
	}

	if (!usblp->present) {
		count = -ENODEV;
		goto done;
	}

#ifdef SUPPORT_PRN_COUNT
	count = count < USBLP_BUF_SIZE ? count : USBLP_BUF_SIZE;

	copy_to_user(buffer, (uint8 *)usblp->readurb.transfer_buffer + usblp->readcount, count);
#else

	if (usblp->readurb->status) {
//ZOT		err("usblp%d: error %d reading from printer",
//ZOT			usblp->minor, usblp->readurb->status);
		usblp->readurb->dev = usblp->dev;
 		usblp->readcount = 0;
		usblp->rcomplete = 0;
		if (usb_submit_urb(usblp->readurb) < 0)
		{
//ZOT			dbg("error submitting urb");
		}
		count = -EIO;
		goto done;
	}

	count = count < usblp->readurb->actual_length - usblp->readcount ?
		count :	usblp->readurb->actual_length - usblp->readcount;

	if (copy_to_user(buffer, usblp->readurb->transfer_buffer + usblp->readcount, count)) {
		count = -EFAULT;
		goto done;
	}
#endif //SUPPORT_PRN_COUNT

	if ((usblp->readcount += count) == usblp->readurb->actual_length) {
		usblp->readcount = 0;
		usblp->readurb->dev = usblp->dev;
		usblp->rcomplete = 0;
		if (usb_submit_urb(usblp->readurb)) {
			count = -EIO;
			goto done;
		}
	}

done:
//ZOT	up (&usblp->sem);
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

#if 0 //ZOT
static struct file_operations usblp_fops = {
	owner:		THIS_MODULE,
	read:		usblp_read,
	write:		usblp_write,
	poll:		usblp_poll,
	ioctl:		usblp_ioctl,
	open:		usblp_open,
	release:	usblp_release,
};
#endif //0 //ZOT

extern unsigned char HP1020_BIN[128431];

static void *usblp_probe(struct usb_device *dev, unsigned int ifnum,
			 const struct usb_device_id *id)
{
	struct usblp *usblp = 0;
	int protocol;
	char name[6];
	char *pBuf; 
	int nLength=0,total_len=0,plen=0;

	/* Malloc and start initializing usblp structure so we can use it
	 * directly. */
	if (!(usblp = kmalloc(sizeof(struct usblp), GFP_KERNEL))) {
//ZOT		err("out of memory for usblp");
		goto abort;
	}
	memset(usblp, 0, sizeof(struct usblp));
	usblp->dev = dev;
//ZOT	init_MUTEX (&usblp->sem);

//ZOT	init_waitqueue_head(&usblp->wait);
	usblp->write_wait = malloc (sizeof(cyg_sem_t));	//ZOT716u2
	usblp->read_wait = malloc (sizeof(cyg_sem_t));	//ZOT716u2
	cyg_semaphore_init(usblp->write_wait, 0);		//ZOT716u2
	cyg_semaphore_init(usblp->read_wait, 0);		//ZOT716u2

	usblp->ifnum = ifnum;

	/* Look for a free usblp_table entry. */
	while (usblp_table[usblp->minor]) {
		usblp->minor++;
		if (usblp->minor >= USBLP_MINORS) {
//ZOT			err("no more free usblp devices");
			goto abort;
		}
	}

	usblp->writeurb = usb_alloc_urb(0);
	if (!usblp->writeurb) {
//ZOT		err("out of memory");
		goto abort;
	}
	usblp->readurb = usb_alloc_urb(0);
	if (!usblp->readurb) {
//ZOT		err("out of memory");
		goto abort;
	}

	/* Malloc device ID string buffer to the largest expected length,
	 * since we can re-query it on an ioctl and a dynamic string
	 * could change in length. */
	if (!(usblp->device_id_string = kmalloc(USBLP_DEVICE_ID_SIZE, GFP_KERNEL))) {
//ZOT		err("out of memory for device_id_string");
		goto abort;
	}
	memset(usblp->device_id_string, 0, USBLP_DEVICE_ID_SIZE);

	usblp->writebuf = usblp->readbuf = NULL;
	/* Malloc write & read buffers.  We somewhat wastefully
	 * malloc both regardless of bidirectionality, because the
	 * alternate setting can be changed later via an ioctl. */
//ZOT	if (!(usblp->writebuf = kmalloc(USBLP_BUF_SIZE, GFP_KERNEL))) {
	if (!(usblp->writebuf = kaligned_alloc(8192, 4096))) {
//ZOT		err("out of memory for write buf");
		goto abort;
	}
//ZOT	if (!(usblp->readbuf = kmalloc(USBLP_BUF_SIZE, GFP_KERNEL))) {
	if (!(usblp->readbuf = kaligned_alloc(8192, 4096))) {
//ZOT		err("out of memory for read buf");
		goto abort;
	}

	/* Allocate buffer for printer status */
	usblp->statusbuf = kmalloc(STATUS_BUF_SIZE, GFP_KERNEL);
	if (!usblp->statusbuf) {
//ZOT		err("out of memory for statusbuf");
		goto abort;
	}

	/* Lookup quirks for this printer. */
	usblp->quirks = usblp_quirks(
		dev->descriptor.idVendor,
		dev->descriptor.idProduct);

	/* Analyze and pick initial alternate settings and endpoints. */
	protocol = usblp_select_alts(usblp);
	if (protocol < 0) {
//ZOT		dbg("incompatible printer-class device 0x%4.4X/0x%4.4X",
//ZOT			dev->descriptor.idVendor,
//ZOT			dev->descriptor.idProduct);
		goto abort;			// Jesse marked this line at build0022 of 716U2 on June 15, 2011.
							// EPSON Stylus TX110
	}

	/* Setup the selected alternate setting and endpoints. */
	if (usblp_set_protocol(usblp, protocol) < 0)
		goto abort;			// Jesse marked this line at build0022 of 716U2 on June 15, 2011.
							// EPSON Stylus TX110

	/* Retrieve and store the device ID string. */
	usblp_cache_device_id_string(usblp);

#ifdef DEBUG
	usblp_check_status(usblp, 0);
#endif

	usblp_table[usblp->minor] = usblp;
	
	usbprn_attach( usblp->minor );
	
	/* If we have devfs, create with perms=660. */
//ZOT	sprintf(name, "lp%d", usblp->minor);
//ZOT	usblp->devfs = devfs_register(usb_devfs_handle, name,
//ZOT				      DEVFS_FL_DEFAULT, USB_MAJOR,
//ZOT				      USBLP_MINOR_BASE + usblp->minor,
//ZOT				      S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
//ZOT				      S_IWGRP, &usblp_fops, NULL);

//ZOT	info("usblp%d: USB %sdirectional printer dev %d "
//ZOT		"if %d alt %d proto %d vid 0x%4.4X pid 0x%4.4X",
//ZOT		usblp->minor, usblp->bidir ? "Bi" : "Uni", dev->devnum,
//ZOT		usblp->ifnum,
//ZOT		usblp->protocol[usblp->current_protocol].alt_setting,
//ZOT		usblp->current_protocol, usblp->dev->descriptor.idVendor,
//ZOT		usblp->dev->descriptor.idProduct);

	usblp->present = 1;
	
	if(	   (dev->descriptor.idVendor == 0x03F0) && (dev->descriptor.idProduct == 0x2B17)
		|| (dev->descriptor.idVendor == 0x03F0) && (dev->descriptor.idProduct == 0x4117)
	   )
	 	IS_HP_GDI_PRINTER_HP_1020 = 1;
	
	if(	   (dev->descriptor.idVendor == 0x04DD) && (dev->descriptor.idProduct == 0x9082)

	   )
		IS_SHARP_AR_M207 = 1;
	
	if(IS_HP_GDI_PRINTER_HP_1020 == 1)
	{
		usblp_open( usblp );
		pBuf = HP1020_BIN;
		total_len = sizeof(HP1020_BIN);
		while( total_len > nLength )
		{
			if( (total_len - nLength) > 8192)
				plen = 8192;
		else
				plen = total_len - nLength;
			
			usbprn_write(0, pBuf+nLength, plen);
			nLength += plen;
		}
		usblp_release( usblp->minor );
	}

	return usblp;

abort:
	if (usblp) {
		if (usblp->writebuf)
//ZOT			kfree (usblp->writebuf);
			kaligned_free(usblp->writebuf);
			
		if (usblp->readbuf)
//ZOT			kfree (usblp->readbuf);
			kaligned_free (usblp->readbuf);
			
		kfree(usblp->statusbuf, 0);
		kfree(usblp->device_id_string, 0);
		usb_free_urb(usblp->writeurb);
		usb_free_urb(usblp->readurb);
		kfree(usblp, 0);
	}
	return NULL;
}

/*
 * We are a "new" style driver with usb_device_id table,
 * but our requirements are too intricate for simple match to handle.
 *
 * The "proto_bias" option may be used to specify the preferred protocol
 * for all USB printers (1=7/1/1, 2=7/1/2, 3=7/1/3).  If the device
 * supports the preferred protocol, then we bind to it.
 *
 * The best interface for us is 7/1/2, because it is compatible
 * with a stream of characters. If we find it, we bind to it.
 *
 * Note that the people from hpoj.sourceforge.net need to be able to
 * bind to 7/1/3 (MLC/1284.4), so we provide them ioctls for this purpose.
 * 
 * Failing 7/1/2, we look for 7/1/3, even though it's probably not
 * stream-compatible, because this matches the behaviour of the old code.
 *
 * If nothing else, we bind to 7/1/1 - the unidirectional interface.
 */
static int usblp_select_alts(struct usblp *usblp)
{
	struct usb_interface *if_alt;
	struct usb_interface_descriptor *ifd;
	struct usb_endpoint_descriptor *epd, *epwrite, *epread;
	int p, i, e;

	if_alt = &usblp->dev->actconfig->interface[usblp->ifnum];

	for (p = 0; p < USBLP_MAX_PROTOCOLS; p++)
		usblp->protocol[p].alt_setting = -1;

	/* Find out what we have. */
	for (i = 0; i < if_alt->num_altsetting; i++) {
		ifd = &if_alt->altsetting[i];

		if (ifd->bInterfaceClass != 7 || ifd->bInterfaceSubClass != 1)
			continue;

		if (ifd->bInterfaceProtocol < USBLP_FIRST_PROTOCOL ||
		    ifd->bInterfaceProtocol > USBLP_LAST_PROTOCOL)
			continue;

		/* Look for bulk OUT and IN endpoints. */
		epwrite = epread = 0;
		for (e = 0; e < ifd->bNumEndpoints; e++) {
			epd = &ifd->endpoint[e];

			if ((epd->bmAttributes&USB_ENDPOINT_XFERTYPE_MASK)!=
			    USB_ENDPOINT_XFER_BULK)
				continue;

			if (!(epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK)) {
				if (!epwrite) epwrite=epd;

			} else {
				if (!epread) epread=epd;
			}
		}

		/* Ignore buggy hardware without the right endpoints. */
		if ((!epwrite || (ifd->bInterfaceProtocol > 1 && !epread)) 
				&& !( (ifd->bInterfaceProtocol > 1) && ( ifd->bNumEndpoints == 1) && (epwrite) ) //ZOT 
				)
			continue;

		/* Turn off reads for 7/1/1 (unidirectional) interfaces
		 * and buggy bidirectional printers. */
		if (ifd->bInterfaceProtocol == 1) {
			epread = NULL;
		} else if (usblp->quirks & USBLP_QUIRK_BIDIR) {
//ZOT			info("Disabling reads from problem bidirectional "
//ZOT				"printer on usblp%d", usblp->minor);
			epread = NULL;
		}

		usblp->protocol[ifd->bInterfaceProtocol].alt_setting = i;
		usblp->protocol[ifd->bInterfaceProtocol].epwrite = epwrite;
		usblp->protocol[ifd->bInterfaceProtocol].epread = epread;
	}

	/* If our requested protocol is supported, then use it. */
	if (proto_bias >= USBLP_FIRST_PROTOCOL &&
	    proto_bias <= USBLP_LAST_PROTOCOL &&
	    usblp->protocol[proto_bias].alt_setting != -1)
		return proto_bias;

	/* Ordering is important here. */
	if (usblp->protocol[2].alt_setting != -1) return 2;
	if (usblp->protocol[1].alt_setting != -1) return 1;
	if (usblp->protocol[3].alt_setting != -1) return 3;

	/* If nothing is available, then don't bind to this device. */
	return -1;
}

static int usblp_set_protocol(struct usblp *usblp, int protocol)
{
	int r, alts;

	if (protocol < USBLP_FIRST_PROTOCOL || protocol > USBLP_LAST_PROTOCOL)
		return -EINVAL;

	alts = usblp->protocol[protocol].alt_setting;
	if (alts < 0) return -EINVAL;
	r = usb_set_interface(usblp->dev, usblp->ifnum, alts);
	if (r < 0) {
//ZOT		err("can't set desired altsetting %d on interface %d",
//ZOT			alts, usblp->ifnum);
		return r;
	}

	usb_fill_bulk_urb(usblp->writeurb, usblp->dev,
		usb_sndbulkpipe(usblp->dev,
		  usblp->protocol[protocol].epwrite->bEndpointAddress),
		usblp->writebuf, 0,
		usblp_bulk_write, usblp);

	usblp->bidir = (usblp->protocol[protocol].epread != 0);
	if (usblp->bidir)
		usb_fill_bulk_urb(usblp->readurb, usblp->dev,
			usb_rcvbulkpipe(usblp->dev,
			  usblp->protocol[protocol].epread->bEndpointAddress),
			usblp->readbuf, USBLP_BUF_SIZE,
			usblp_bulk_read, usblp);

	usblp->current_protocol = protocol;
//ZOT	dbg("usblp%d set protocol %d", usblp->minor, protocol);
	return 0;
}

/* Retrieves and caches device ID string.
 * Returns length, including length bytes but not null terminator.
 * On error, returns a negative errno value. */
static int usblp_cache_device_id_string(struct usblp *usblp)
{
	int err, length;

	err = usblp_get_id(usblp, 0, usblp->device_id_string, USBLP_DEVICE_ID_SIZE - 1);
	if (err < 0) {
//ZOT		dbg("usblp%d: error = %d reading IEEE-1284 Device ID string",
//ZOT			usblp->minor, err);
		usblp->device_id_string[0] = usblp->device_id_string[1] = '\0';
		return -EIO;
	}

	/* First two bytes are length in big-endian.
	 * They count themselves, and we copy them into
	 * the user's buffer. */
	length = (usblp->device_id_string[0] << 8) + usblp->device_id_string[1];
	if (length < 2)
		length = 2;
	else if (length >= USBLP_DEVICE_ID_SIZE)
		length = USBLP_DEVICE_ID_SIZE - 1;
	usblp->device_id_string[length] = '\0';

//ZOT	dbg("usblp%d Device ID string [len=%d]=\"%s\"",
//ZOT		usblp->minor, length, &usblp->device_id_string[2]);

	return length;
}

static void usblp_disconnect(struct usb_device *dev, void *ptr)
{
	struct usblp *usblp = ptr;

	IS_HP_GDI_PRINTER_HP_1020 = 0;
	IS_SHARP_AR_M207 = 0;
	
	usbprn_unattach( usblp->minor );

	if (!usblp || !usblp->dev) {
//ZOT		err("bogus disconnect");
//ZOT		BUG ();
	}

//ZOT	down (&usblp_sem);
//ZOT	down (&usblp->sem);
	usblp->present = 0;

	usblp_unlink_urbs(usblp);
//ZOT	up (&usblp->sem);

	if( !usblp->used )
		usblp_cleanup (usblp);
//ZOT	up (&usblp_sem);
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

//ZOT MODULE_DEVICE_TABLE (usb, usblp_ids);

static struct usb_driver usblp_driver = {
	/*name:*/	"usblp",
#if 0
	probe:		usblp_probe,
	disconnect:	usblp_disconnect,
	fops:		&usblp_fops,
	minor:		USBLP_MINOR_BASE,
	id_table:	usblp_ids,
#endif //0		
};

int usblp_init(void)
{
	usblp_driver.probe 		= usblp_probe;
	usblp_driver.disconnect = usblp_disconnect;
	usblp_driver.minor 		= USBLP_MINOR_BASE;
	usblp_driver.id_table	= usblp_ids;

	if (usb_register(&usblp_driver))
		return -1;
//ZOT	info(DRIVER_VERSION ": " DRIVER_DESC);
	return 0;
}

void usblp_exit(void)
{
	usb_deregister(&usblp_driver);
}

#if 0 //ZOT
module_init(usblp_init);
module_exit(usblp_exit);

MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_PARM(proto_bias, "i");
MODULE_PARM_DESC(proto_bias, "Favourite protocol number");
MODULE_LICENSE("GPL");
#endif //0 //ZOT


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
		return 0xD9;
	}
	// not connect
	return 0x7F;
}

int usbprn_attach( int minor )
{
	struct usblp *usblp = usblp_table[minor];
//Jesse	int i1284 = NUM_OF_1284_PORT + usblp->dev->ttport;
	int i1284 = NUM_OF_1284_PORT + usblp->dev->printer_port;
	char *tmp;
	char sharp_string[5] = "SHARP";
	char sharp_language[6] = "RASTER";

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
			
			if(memcmp(PortIO[i1284].Manufacture ,sharp_string, 5) == 0 )
			{
				if(memcmp(PortIO[i1284].CommandSet , sharp_language, 6) == 0 )
					IS_SHARP_AR_M207 = 1;
				
				if( !(PortIO[i1284].SupportLang & (P1284_PJL | P1284_PCL | P1284_POSTSCRIPT)) )
					IS_SHARP_AR_M207 = 1;
			}
			
			
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
