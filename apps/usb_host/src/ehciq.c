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

/* this file is part of ehci-hcd.c */

/*-------------------------------------------------------------------------*/

/*
 * EHCI hardware queue manipulation ... the core.  QH/QTD manipulation.
 *
 * Control, bulk, and interrupt traffic all use "qh" lists.  They list "qtd"
 * entries describing USB transactions, max 16-20kB/entry (with 4kB-aligned
 * buffers needed for the larger number).  We use one QH per endpoint, queue
 * multiple (bulk or control) urbs per endpoint.  URBs may need several qtds.
 * A scheduled interrupt qh always (for now) has one qtd, one urb.
 *
 * ISO traffic uses "ISO TD" (itd, and sitd) records, and (along with
 * interrupts) needs careful scheduling.  Performance improvements can be
 * an ongoing challenge.  That's in "ehci-sched.c".
 * 
 * USB 1.1 devices are handled (a) by "companion" OHCI or UHCI root hubs,
 * or otherwise through transaction translators (TTs) in USB 2.0 hubs using
 * (b) special fields in qh entries or (c) split iso entries.  TTs will
 * buffer low/full speed data so the host collects it at high speed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "psglobal.h"
#include "usb.h"
#include "hub.h"
#include "u2hcd.h"
#include "ehci.h"
#include "completion.h"
#include "list.h"

#undef EHCI_DEBUG	//635u2
/*-------------------------------------------------------------------------*/
//PCI module
extern uint32 pci_data_map(uint32 * addr, uint32 len, uint32 direc, uint32 setup, uint32 hcd);
extern uint32 pci_data_unmap(uint32 dma, uint32 len, uint32 direc, uint32 setup, uint32 hcd);
extern void pci_data_sync(uint32 dma, uint32 len, uint32 direc, uint32 setup, uint32 hcd);

/* fill a qtd, returning how much of the buffer we were able to queue up */

static int
qtd_fill (struct ehci_qtd *qtd, dma_addr_t buf, size_t len, int token)
{
	int	i, count;

	/* one buffer entry per 4K ... first might be short or unaligned */
//original Source	//635u2	
//	qtd->hw_buf [0] = cpu_to_le32 (buf);
	qtd->hw_buf [0] = buf;
	count = 0x1000 - (buf & 0x0fff);	/* rest of that page */
	if (likely (len < count))		/* ... iff needed */
		count = len;
	else {
		buf +=  0x1000;
		buf &= ~0x0fff;

		/* per-qtd limit: from 16K to 20K (best alignment) */
		for (i = 1; count < len && i < 5; i++) {
			u64	addr = buf;
//original Source	//635u2	
//			qtd->hw_buf [i] = cpu_to_le32 ((u32)addr);
			qtd->hw_buf [i] = (u32)addr;
//original Source	//635u2	
//			qtd->hw_buf_hi [i] = cpu_to_le32 ((u32)(addr >> 32));
			qtd->hw_buf_hi [i] = (u32)(addr >> 32);
			buf += 0x1000;
			if ((count + 0x1000) < len)
				count += 0x1000;
			else
				count = len;
		}
	}
//original Source	//635u2	
//	qtd->hw_token = cpu_to_le32 ((count << 16) | token);
	qtd->hw_token = (count << 16) | token;
	qtd->length = count;

#if 0
	vdbg ("  qtd_fill %p, token %8x bytes %d dma %x",
		qtd, le32_to_cpu (qtd->hw_token), count, qtd->hw_buf [0]);
#endif

	return count;
}

/*-------------------------------------------------------------------------*/

/* update halted (but potentially linked) qh */

void qh_update (struct ehci_qh *qh, struct ehci_qtd *qtd)
{
	qh->hw_current = 0;
	qh->hw_qtd_next = QTD_NEXT (qtd->qtd_dma);
	qh->hw_alt_next = EHCI_LIST_END;

	/* HC must see latest qtd and qh data before we clear ACTIVE+HALT */
	wmb ();
	qh->hw_token &= __constant_cpu_to_le32 (QTD_TOGGLE | QTD_STS_PING);
}

/*-------------------------------------------------------------------------*/

#define IS_SHORT_READ(token) (QTD_LENGTH (token) != 0 && QTD_PID (token) == 1)

static inline void qtd_copy_status (struct urb *urb, size_t length, u32 token)
{
	/* count IN/OUT bytes, not SETUP (even short packets) */
	if (likely (QTD_PID (token) != 2))
		urb->actual_length += length - QTD_LENGTH (token);

	/* don't modify error codes */
	if (unlikely (urb->status == -EINPROGRESS && (token & QTD_STS_HALT))) {
		if (token & QTD_STS_BABBLE) {
			/* FIXME "must" disable babbling device's port too */
			urb->status = -EOVERFLOW;
		} else if (token & QTD_STS_MMF) {
			/* fs/ls interrupt xfer missed the complete-split */
			urb->status = -EPROTO;
		} else if (token & QTD_STS_DBE) {
			urb->status = (QTD_PID (token) == 1) /* IN ? */
				? -ENOSR  /* hc couldn't read data */
				: -ECOMM; /* hc couldn't write data */
		} else if (token & QTD_STS_XACT) {
			/* timeout, bad crc, wrong PID, etc; retried */
			if (QTD_CERR (token))
				urb->status = -EPIPE;
			else {
#ifdef EHCI_DEBUG	//635u2
				dbg ("3strikes");
#endif				
				urb->status = -EPROTO;
			}
		/* CERR nonzero + no errors + halt --> stall */
		} else if (QTD_CERR (token))
			urb->status = -EPIPE;
		else	/* unknown */
			urb->status = -EPROTO;
#ifdef EHCI_DEBUG	//635u2
		dbg ("ep %d-%s qtd token %08x --> status %d",
			/* devpath */
			usb_pipeendpoint (urb->pipe),
			usb_pipein (urb->pipe) ? "in" : "out",
			token, urb->status);
#endif
		/* stall indicates some recovery action is needed */
		if (urb->status == -EPIPE) {
			int	pipe = urb->pipe;

			if (!usb_pipecontrol (pipe))
				usb_endpoint_halt (urb->dev,
					usb_pipeendpoint (pipe),
					usb_pipeout (pipe));
			if (urb->dev->tt && !usb_pipeint (pipe)) {
#ifdef EHCI_DEBUG	//635u2	
	err ("must CLEAR_TT_BUFFER, hub port %d%s addr %d ep %d",
    urb->dev->ttport, /* devpath */
    urb->dev->tt->multi ? "" : " (all-ports TT)",
    urb->dev->devnum, usb_pipeendpoint (urb->pipe));
#endif
				// FIXME something (khubd?) should make the hub
				// CLEAR_TT_BUFFER ASAP, it's blocking other
				// fs/ls requests... hub_tt_clear_buffer() ?
			}
		}
	}
}

static void ehci_urb_complete (
	struct ehci_hcd		*ehci,
	dma_addr_t		addr,
	struct urb		*urb
) {
//original Source	//635u2		
//	if (urb->transfer_buffer_length && usb_pipein (urb->pipe))
//		pci_dma_sync_single (ehci->hcd.pdev, addr,
//			urb->transfer_buffer_length,
//			PCI_DMA_FROMDEVICE);

//719AW	if (urb->transfer_buffer_length && usb_pipein (urb->pipe))
//719AW		pci_data_sync(addr, urb->transfer_buffer_length,
//719AW						PCI_DMA_FROMDEVICE,0,2);

	/* cleanse status if we saw no error */
	if (likely (urb->status == -EINPROGRESS)) {
		if (urb->actual_length != urb->transfer_buffer_length
				&& (urb->transfer_flags & USB_DISABLE_SPD))
			urb->status = -EREMOTEIO;
		else
			urb->status = 0;
	}

	/* only report unlinks once */
	if (likely (urb->status != -ENOENT && urb->status != -ENOTCONN))
		urb->complete (urb);
}

/* urb->lock ignored from here on (hcd is done with urb) */

static void ehci_urb_done (
	struct ehci_hcd		*ehci,
	dma_addr_t		addr,
	struct urb		*urb
) {
//original Source	//635u2
//	if (urb->transfer_buffer_length)   	 		
//		pci_unmap_single (ehci->hcd.pdev,
//			addr,
//			urb->transfer_buffer_length,
//			usb_pipein (urb->pipe)
//			    ? PCI_DMA_FROMDEVICE
//			    : PCI_DMA_TODEVICE);
//719AW	if (urb->transfer_buffer_length)
//719AW		addr = pci_data_unmap(addr, urb->transfer_buffer_length,
//719AW						(usb_pipein(urb->pipe) ?
//719AW				    	PCI_DMA_FROMDEVICE :
//719AW				    	PCI_DMA_TODEVICE),0, 2);

	if (likely (urb->hcpriv != 0)) {
		qh_put (ehci, (struct ehci_qh *) urb->hcpriv);
		urb->hcpriv = 0;
	}

	if (likely (urb->status == -EINPROGRESS)) {
		if (urb->actual_length != urb->transfer_buffer_length
				&& (urb->transfer_flags & USB_DISABLE_SPD))
			urb->status = -EREMOTEIO;
		else
			urb->status = 0;
	}

	/* hand off urb ownership */
	usb_hcd_giveback_urb (&ehci->hcd, urb);
}


/*
 * Process completed qtds for a qh, issuing completions if needed.
 * When freeing:  frees qtds, unmaps buf, returns URB to driver.
 * When not freeing (queued periodic qh):  retain qtds, mapping, and urb.
 * Races up to qh->hw_current; returns number of urb completions.
 */
int
qh_completions (
	struct ehci_hcd		*ehci,
	struct ehci_qh		*qh,
	int			freeing
) {
	struct ehci_qtd		*qtd, *last;
	struct list_head	*next, *qtd_list = &qh->qtd_list;
	int			unlink = 0, halted = 0;
	unsigned long		flags;
	int			retval = 0;
	u8			state;

	state = qh->qh_state;

	spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
	if (unlikely (list_empty (qtd_list))) {
		spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> 	spin_unlock_irqrestore (&ehci->lock, flags);
		return retval;
	}

	/* scan QTDs till end of list, or we reach an active one */
	for (qtd = list_entry (qtd_list->next, struct ehci_qtd, qtd_list),
			    	last = 0, next = 0;
			next != qtd_list;
			last = qtd, qtd = list_entry (next,
						struct ehci_qtd, qtd_list)) {
		struct urb	*urb = qtd->urb;
		u32		token = 0;

		/* clean up any state from previous QTD ...*/
		if (last) {
			if (likely (last->urb != urb)) {
				/* complete() can reenter this HCD */
				spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> 	spin_unlock_irqrestore (&ehci->lock, flags);
				if (likely (freeing != 0))
					ehci_urb_done (ehci, last->buf_dma,
						last->urb);
				else
					ehci_urb_complete (ehci, last->buf_dma,
						last->urb);
				spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
				retval++;
			}

			/* qh overlays can have HC's old cached copies of
			 * next qtd ptrs, if an URB was queued afterwards.
			 */
//original Source	//635u2	
//			if (cpu_to_le32 (last->qtd_dma) == qh->hw_current
			if (last->qtd_dma == qh->hw_current
					&& last->hw_next != qh->hw_qtd_next) {
				qh->hw_alt_next = last->hw_alt_next;
				qh->hw_qtd_next = last->hw_next;
			}

			if (likely (freeing != 0))
				ehci_qtd_free (ehci, last);
			last = 0;
		}
		next = qtd->qtd_list.next;

		/* QTDs at tail may be active if QH+HC are running,
		 * or when unlinking some urbs queued to this QH
		 */
		token = le32_to_cpu (qtd->hw_token);
		halted = halted
			|| (__constant_cpu_to_le32 (QTD_STS_HALT)
				& qh->hw_token) != 0
			|| (ehci->hcd.state == USB_STATE_HALT)
			|| (qh->qh_state == QH_STATE_IDLE);

		/* fault: unlink the rest, since this qtd saw an error? */
		if (unlikely ((token & QTD_STS_HALT) != 0)) {
			freeing = unlink = 1;
			/* status copied below */

		/* QH halts only because of fault (above) or unlink (here). */
		} else if (unlikely (halted != 0)) {

			/* unlinking everything because of HC shutdown? */
			if (ehci->hcd.state == USB_STATE_HALT) {
				freeing = unlink = 1;

			/* explicit unlink, maybe starting here? */
			} else if (qh->qh_state == QH_STATE_IDLE
					&& (urb->status == -ECONNRESET
						|| urb->status == -ENOENT)) {
				freeing = unlink = 1;

			/* QH halted to unlink urbs _after_ this?  */
			} else if (!unlink && (token & QTD_STS_ACTIVE) != 0) {
				qtd = 0;
				continue;
			}

			/* unlink the rest?  once we start unlinking, after
			 * a fault or explicit unlink, we unlink all later
			 * urbs.  usb spec requires that.
			 */
			if (unlink && urb->status == -EINPROGRESS)
				urb->status = -ECONNRESET;

			/* token in overlay may be most current */
			if (state == QH_STATE_IDLE
					&& cpu_to_le32 (qtd->qtd_dma)
						== qh->hw_current)
				token = le32_to_cpu (qh->hw_token);
			
		/* Else QH is active, so we must not modify QTDs
		 * that HC may be working on.  No more qtds to check.
		 */
		} else if (unlikely ((token & QTD_STS_ACTIVE) != 0)) {
			next = qtd_list;
			qtd = 0;
			continue;
		}

		spin_lock (&urb->lock);
		qtd_copy_status (urb, qtd->length, token);
		spin_unlock (&urb->lock);

		/*
		 * NOTE:  this won't work right with interrupt urbs that
		 * need multiple qtds ... only the first scan of qh->qtd_list
		 * starts at the right qtd, yet multiple scans could happen
		 * for transfers that are scheduled across multiple uframes. 
		 * (Such schedules are not currently allowed!)
		 */
		if (likely (freeing != 0))
			list_del (&qtd->qtd_list);
		else {
			/* restore everything the HC could change
			 * from an interrupt QTD
			 */
			qtd->hw_token = (qtd->hw_token
					& __constant_cpu_to_le32 (0x8300))
//original Source	//635u2	
//				| cpu_to_le32 (qtd->length << 16)
				| (qtd->length << 16)
				| __constant_cpu_to_le32 (QTD_STS_ACTIVE
					| (EHCI_TUNE_CERR << 10));
			qtd->hw_buf [0] &= ~__constant_cpu_to_le32 (0x0fff);

			/* this offset, and the length above,
			 * are likely wrong on QTDs #2..N
			 */
//original Source	//635u2	
//			qtd->hw_buf [0] |= cpu_to_le32 (0x0fff & qtd->buf_dma);
			qtd->hw_buf [0] |= (0x0fff & qtd->buf_dma);
		}

#if 0
		if (urb->status == -EINPROGRESS)
			vdbg ("  qtd %p ok, urb %p, token %8x, len %d",
				qtd, urb, token, urb->actual_length);
		else
			vdbg ("urb %p status %d, qtd %p, token %8x, len %d",
				urb, urb->status, qtd, token,
				urb->actual_length);
#endif

		/* SETUP for control urb? */
//original Source	//635u2
//		if (unlikely (QTD_PID (token) == 2))
//			pci_unmap_single (ehci->hcd.pdev,
//				qtd->buf_dma, sizeof (struct usb_ctrlrequest),
//				PCI_DMA_TODEVICE);
//719AW		if (unlikely (QTD_PID (token) == 2))
//719AW			qtd->buf_dma = pci_data_unmap(qtd->buf_dma, sizeof(devrequest),
//719AW						PCI_DMA_TODEVICE,1,2);
	}

	/* patch up list head? */
	if (unlikely (halted && !list_empty (qtd_list))) {
		qh_update (qh, list_entry (qtd_list->next,
				struct ehci_qtd, qtd_list));
	}
	spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> 	spin_unlock_irqrestore (&ehci->lock, flags);

	/* last urb's completion might still need calling */
	if (likely (last != 0)) {
		if (likely (freeing != 0)) {
			ehci_urb_done (ehci, last->buf_dma, last->urb);
			ehci_qtd_free (ehci, last);
		} else
			ehci_urb_complete (ehci, last->buf_dma, last->urb);
		retval++;
	}
	return retval;
}

/*-------------------------------------------------------------------------*/

/*
 * reverse of qh_urb_transaction:  free a list of TDs.
 * used for cleanup after errors, before HC sees an URB's TDs.
 */
void qtd_list_free (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	struct list_head	*qtd_list
) {
	struct list_head	*entry, *temp;
	int			unmapped = 0;
	int setup;

	list_for_each_safe (entry, temp, qtd_list) {
		struct ehci_qtd	*qtd;

		qtd = list_entry (entry, struct ehci_qtd, qtd_list);
		list_del (&qtd->qtd_list);
		if (unmapped != 2) {
			int	direction;
			size_t	size;

			/* for ctrl unmap twice: SETUP and DATA;
			 * else (bulk, intr) just once: DATA
			 */
			if (!unmapped++ && usb_pipecontrol (urb->pipe)) {
				direction = PCI_DMA_TODEVICE;
//original Source	//635u2
//				size = sizeof (struct usb_ctrlrequest);
				size = sizeof (devrequest);
				setup = 1; //615wu
			} else {
				direction = usb_pipein (urb->pipe)
					? PCI_DMA_FROMDEVICE
					: PCI_DMA_TODEVICE;
				size = qtd->urb->transfer_buffer_length;
				unmapped++;
				setup = 0; //615wu
			}
//original Source	//635u2
//			if (qtd->buf_dma)
//				pci_unmap_single (ehci->hcd.pdev,
//					qtd->buf_dma,
//					size, direction);
//719AW			if (qtd->buf_dma)
//719AW				qtd->buf_dma = pci_data_unmap(qtd->buf_dma, size,direction,setup,2);
		}
		ehci_qtd_free (ehci, qtd);
	}
}

/*
 * create a list of filled qtds for this URB; won't link into qh.
 */
struct list_head *
qh_urb_transaction (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	struct list_head	*head,
	int			flags
) {
	struct ehci_qtd		*qtd, *qtd_prev;
	dma_addr_t		buf, map_buf;
	int			len, maxpacket;
	u32			token;
	/*
	 * URBs map to sequences of QTDs:  one logical transaction
	 */
	qtd = ehci_qtd_alloc (ehci, flags);
	if (unlikely (!qtd))
		return 0;
	qtd_prev = 0;
	list_add_tail (&qtd->qtd_list, head);
	qtd->urb = urb;

	token = QTD_STS_ACTIVE;
	token |= (EHCI_TUNE_CERR << 10);
	/* for split transactions, SplitXState initialized to zero */

	if (usb_pipecontrol (urb->pipe)) {
		/* control request data is passed in the "setup" pid */
//original Source	//635u2	
//		qtd->buf_dma = pci_map_single (
//					ehci->hcd.pdev,
//					urb->setup_packet,
//					sizeof (struct usb_ctrlrequest),
//					PCI_DMA_TODEVICE);
//719AW		qtd->buf_dma = pci_data_map(urb->setup_packet, sizeof(devrequest),
//719AW									PCI_DMA_TODEVICE, 1, 2);
		qtd->buf_dma = (uint32)urb->setup_packet;
	
		if (unlikely (!qtd->buf_dma))
			goto cleanup;

		/* SETUP pid */
//original Source	//635u2
//		qtd_fill (qtd, qtd->buf_dma, sizeof (struct usb_ctrlrequest),
		qtd_fill (qtd, qtd->buf_dma, sizeof (devrequest),
			token | (2 /* "setup" */ << 8));

		/* ... and always at least one more pid */
		token ^= QTD_TOGGLE;
		qtd_prev = qtd;
		qtd = ehci_qtd_alloc (ehci, flags);
		if (unlikely (!qtd))
			goto cleanup;
		qtd->urb = urb;
		qtd_prev->hw_next = QTD_NEXT (qtd->qtd_dma);
		list_add_tail (&qtd->qtd_list, head);
	} 

	/*
	 * data transfer stage:  buffer setup
	 */
	len = urb->transfer_buffer_length;
	if (likely (len > 0)) {		
//original Source	//635u2	
//		buf = map_buf = pci_map_single (ehci->hcd.pdev,
//			urb->transfer_buffer, len,
//			usb_pipein (urb->pipe)
//			    ? PCI_DMA_FROMDEVICE
//			    : PCI_DMA_TODEVICE);
//719AW		buf = map_buf = pci_data_map(urb->transfer_buffer,
//719AW									len,
//719AW									(usb_pipein(urb->pipe) ?
//719AW							    	PCI_DMA_FROMDEVICE :
//719AW							    	PCI_DMA_TODEVICE), 0, 2);
		buf = map_buf = (uint32)urb->transfer_buffer;

		if (unlikely (!buf))
			goto cleanup;
	} else
		buf = map_buf = 0;

	if (!buf || usb_pipein (urb->pipe))
		token |= (1 /* "in" */ << 8);
	/* else it's already initted to "out" pid (0 << 8) */

	maxpacket = usb_maxpacket (urb->dev, urb->pipe,
			usb_pipeout (urb->pipe));

	/*
	 * buffer gets wrapped in one or more qtds;
	 * last one may be "short" (including zero len)
	 * and may serve as a control status ack
	 */
	for (;;) {
		int this_qtd_len;

		qtd->urb = urb;
		qtd->buf_dma = map_buf;
		this_qtd_len = qtd_fill (qtd, buf, len, token);
		len -= this_qtd_len;
		buf += this_qtd_len;

		/* qh makes control packets use qtd toggle; maybe switch it */
		if ((maxpacket & (this_qtd_len + (maxpacket - 1))) == 0)
			token ^= QTD_TOGGLE;

		if (likely (len <= 0))
			break;

		qtd_prev = qtd;
		qtd = ehci_qtd_alloc (ehci, flags);
		if (unlikely (!qtd))
			goto cleanup;
		qtd->urb = urb;
		qtd_prev->hw_next = QTD_NEXT (qtd->qtd_dma);
		list_add_tail (&qtd->qtd_list, head);
	}

	/*
	 * control requests may need a terminating data "status" ack;
	 * bulk ones may need a terminating short packet (zero length).
	 */
	if (likely (buf != 0)) {
		int	one_more = 0;

		if (usb_pipecontrol (urb->pipe)) {
			one_more = 1;
			token ^= 0x0100;	/* "in" <--> "out"  */
			token |= QTD_TOGGLE;	/* force DATA1 */
		} else if (usb_pipebulk (urb->pipe)
				&& (urb->transfer_flags & USB_ZERO_PACKET)
				&& !(urb->transfer_buffer_length % maxpacket)) {
			one_more = 1;
		}
		if (one_more) {
			qtd_prev = qtd;
			qtd = ehci_qtd_alloc (ehci, flags);
			if (unlikely (!qtd))
				goto cleanup;
			qtd->urb = urb;
			qtd_prev->hw_next = QTD_NEXT (qtd->qtd_dma);
			list_add_tail (&qtd->qtd_list, head);

			/* never any data in such packets */
			qtd_fill (qtd, 0, 0, token);
		}
	}

	/* by default, enable interrupt on urb completion */
	if (likely (!(urb->transfer_flags & URB_NO_INTERRUPT)))
		qtd->hw_token |= __constant_cpu_to_le32 (QTD_IOC);
	return head;

cleanup:
	qtd_list_free (ehci, urb, head);
	return 0;
}

/*-------------------------------------------------------------------------*/

/*
 * Hardware maintains data toggle (like OHCI) ... here we (re)initialize
 * the hardware data toggle in the QH, and set the pseudo-toggle in udev
 * so we can see if usb_clear_halt() was called.  NOP for control, since
 * we set up qh->hw_info1 to always use the QTD toggle bits. 
 */
static inline void
clear_toggle (struct usb_device *udev, int ep, int is_out, struct ehci_qh *qh)
{
#ifdef EHCI_DEBUG	//635u2	
	vdbg ("clear toggle, dev %d ep 0x%x-%s",
		udev->devnum, ep, is_out ? "out" : "in");
#endif
	qh->hw_token &= ~__constant_cpu_to_le32 (QTD_TOGGLE);
	usb_settoggle (udev, ep, is_out, 1);
}

// Would be best to create all qh's from config descriptors,
// when each interface/altsetting is established.  Unlink
// any previous qh and cancel its urbs first; endpoints are
// implicitly reset then (data toggle too).
// That'd mean updating how usbcore talks to HCDs. (2.5?)


/*
 * Each QH holds a qtd list; a QH is used for everything except iso.
 *
 * For interrupt urbs, the scheduler must set the microframe scheduling
 * mask(s) each time the QH gets scheduled.  For highspeed, that's
 * just one microframe in the s-mask.  For split interrupt transactions
 * there are additional complications: c-mask, maybe FSTNs.
 */
struct ehci_qh *
ehci_qh_make (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	struct list_head	*qtd_list,
	int			flags
) {
	struct ehci_qh		*qh = ehci_qh_alloc (ehci, flags);
	u32			info1 = 0, info2 = 0;

	if (!qh)
		return qh;

	/*
	 * init endpoint/device data for this QH
	 */
	info1 |= usb_pipeendpoint (urb->pipe) << 8;
	info1 |= usb_pipedevice (urb->pipe) << 0;

	/* using TT? */
	switch (urb->dev->speed) {
	case USB_SPEED_LOW:
		info1 |= (1 << 12);	/* EPS "low" */
		/* FALL THROUGH */

	case USB_SPEED_FULL:
		/* EPS 0 means "full" */
		info1 |= (EHCI_TUNE_RL_TT << 28);
		if (usb_pipecontrol (urb->pipe)) {
			info1 |= (1 << 27);	/* for TT */
			info1 |= 1 << 14;	/* toggle from qtd */
		}
		info1 |= usb_maxpacket (urb->dev, urb->pipe,
					usb_pipeout (urb->pipe)) << 16;

		info2 |= (EHCI_TUNE_MULT_TT << 30);
		info2 |= urb->dev->ttport << 23;
		info2 |= urb->dev->tt->hub->devnum << 16;

		/* NOTE:  if (usb_pipeint (urb->pipe)) { scheduler sets c-mask }
		 * ... and a 0.96 scheduler might use FSTN nodes too
		 */
		break;

	case USB_SPEED_HIGH:		/* no TT involved */
		info1 |= (2 << 12);	/* EPS "high" */
		info1 |= (EHCI_TUNE_RL_HS << 28);
		if (usb_pipecontrol (urb->pipe)) {
			info1 |= 64 << 16;	/* usb2 fixed maxpacket */
			info1 |= 1 << 14;	/* toggle from qtd */
			info2 |= (EHCI_TUNE_MULT_HS << 30);
		} else if (usb_pipebulk (urb->pipe)) {
			info1 |= 512 << 16;	/* usb2 fixed maxpacket */
			info2 |= (EHCI_TUNE_MULT_HS << 30);
		} else {
			u32	temp;
			temp = usb_maxpacket (urb->dev, urb->pipe,
						usb_pipeout (urb->pipe));
			info1 |= (temp & 0x3ff) << 16;	/* maxpacket */
			/* HS intr can be "high bandwidth" */
			temp = 1 + ((temp >> 11) & 0x03);
			info2 |= temp << 30;		/* mult */
		}
		break;
	default:
#ifdef DEBUG
		BUG ();
#else
		;
#endif
	}

	/* NOTE:  if (usb_pipeint (urb->pipe)) { scheduler sets s-mask } */

	qh->qh_state = QH_STATE_IDLE;
//original Source	//635u2	
//	qh->hw_info1 = cpu_to_le32 (info1);
//	qh->hw_info2 = cpu_to_le32 (info2);
	qh->hw_info1 = info1;
	qh->hw_info2 = info2;

	/* initialize sw and hw queues with these qtds */
	list_splice (qtd_list, &qh->qtd_list);
	qh_update (qh, list_entry (qtd_list->next, struct ehci_qtd, qtd_list));

	/* initialize data toggle state */
	if (!usb_pipecontrol (urb->pipe))
		clear_toggle (urb->dev,
			usb_pipeendpoint (urb->pipe),
			usb_pipeout (urb->pipe),
			qh);

	return qh;
}

/*-------------------------------------------------------------------------*/

/* move qh (and its qtds) onto async queue; maybe enable queue.  */

static void qh_link_async (struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	u32		dma = QH_NEXT (qh->qh_dma);
	struct ehci_qh	*q = ehci->async;

	if (unlikely (!(q))) {
		u32	cmd = readl (&ehci->regs->command);

		/* in case a clear of CMD_ASE didn't take yet */
		while (readl (&ehci->regs->status) & STS_ASS)
			udelay (100);

		qh->hw_info1 |= __constant_cpu_to_le32 (QH_HEAD); /* [4.8] */
		qh->qh_next.qh = qh;
		qh->hw_next = dma;
		wmb ();
		ehci->async = qh;
		writel ((u32)qh->qh_dma, &ehci->regs->async_next);
		cmd |= CMD_ASE | CMD_RUN;
		writel (cmd, &ehci->regs->command);
		ehci->hcd.state = USB_STATE_RUNNING;
		/* posted write need not be known to HC yet ... */
	} else {
		/* splice right after "start" of ring */
		qh->hw_info1 &= ~__constant_cpu_to_le32 (QH_HEAD); /* [4.8] */
		qh->qh_next = q->qh_next;
		qh->hw_next = q->hw_next;
		wmb ();
		q->qh_next.qh = qh;
		q->hw_next = dma;
	}
	qh->qh_state = QH_STATE_LINKED;
	/* qtd completions reported later by interrupt */
}

/*-------------------------------------------------------------------------*/

int
submit_async (
	struct ehci_hcd		*ehci,
	struct urb		*urb,
	struct list_head	*qtd_list,
	int			mem_flags
) {
	struct ehci_qtd		*qtd;
	struct hcd_dev		*dev;
	int			epnum;
	unsigned long		flags;
	struct ehci_qh		*qh = 0;

	qtd = list_entry (qtd_list->next, struct ehci_qtd, qtd_list);
	dev = (struct hcd_dev *)urb->dev->hcpriv;
	epnum = usb_pipeendpoint (urb->pipe);
	if (usb_pipein (urb->pipe))
		epnum |= 0x10;
#ifdef EHCI_DEBUG	//635u2
	vdbg ("%s: submit_async urb %p len %d ep %d-%s qtd %p [qh %p]",
		ehci->hcd.bus_name, urb, urb->transfer_buffer_length,
		epnum & 0x0f, (epnum & 0x10) ? "in" : "out",
		qtd, dev ? dev->ep [epnum] : (void *)~0);
#endif
	spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);

	qh = (struct ehci_qh *) dev->ep [epnum];
	if (likely (qh != 0)) {
		u32	hw_next = QTD_NEXT (qtd->qtd_dma);
		/* maybe patch the qh used for set_address */
		if (unlikely (epnum == 0
				&& le32_to_cpu (qh->hw_info1 & 0x7f) == 0))
//original Source	//635u2	
//			qh->hw_info1 |= cpu_to_le32 (usb_pipedevice(urb->pipe));
			qh->hw_info1 |= (usb_pipedevice(urb->pipe));

		/* is an URB is queued to this qh already? */
		if (unlikely (!list_empty (&qh->qtd_list))) {
			struct ehci_qtd		*last_qtd;
			int			short_rx = 0;

			/* update the last qtd's "next" pointer */
			// dbg_qh ("non-empty qh", ehci, qh);
			last_qtd = list_entry (qh->qtd_list.prev,
					struct ehci_qtd, qtd_list);
			last_qtd->hw_next = hw_next;

			/* previous urb allows short rx? maybe optimize. */
			if (!(last_qtd->urb->transfer_flags & USB_DISABLE_SPD)
					&& (epnum & 0x10)) {
				// only the last QTD for now
				last_qtd->hw_alt_next = hw_next;
				short_rx = 1;
			}

			/* Adjust any old copies in qh overlay too.
			 * Interrupt code must cope with case of HC having it
			 * cached, and clobbering these updates.
			 * ... complicates getting rid of extra interrupts!
			 */
//original Source	//635u2	
//			if (qh->hw_current == cpu_to_le32 (last_qtd->qtd_dma)) {
			if (qh->hw_current == (last_qtd->qtd_dma)) {	
				wmb ();
				qh->hw_qtd_next = hw_next;
				if (short_rx)
					qh->hw_alt_next = hw_next
				    		| (qh->hw_alt_next & 0x1e);
#ifdef EHCI_DEBUG	//635u2
				vdbg ("queue to qh %p, patch", qh);
#endif
			}

		/* no URB queued */
		} else {
			// dbg_qh ("empty qh", ehci, qh);

			/* NOTE: we already canceled any queued URBs
			 * when the endpoint halted.
			 */

			/* usb_clear_halt() means qh data toggle gets reset */
			if (usb_pipebulk (urb->pipe)
					&& unlikely (!usb_gettoggle (urb->dev,
						(epnum & 0x0f),
						!(epnum & 0x10)))) {
				clear_toggle (urb->dev,
					epnum & 0x0f, !(epnum & 0x10), qh);
			}
			qh_update (qh, qtd);
		}
		list_splice (qtd_list, qh->qtd_list.prev);

	} else {
		/* can't sleep here, we have ehci->lock... */
		qh = ehci_qh_make (ehci, urb, qtd_list, SLAB_ATOMIC);
		if (likely (qh != 0)) {
			// dbg_qh ("new qh", ehci, qh);
			dev->ep [epnum] = qh;
		}
	}

	/* Control/bulk operations through TTs don't need scheduling,
	 * the HC and TT handle it when the TT has a buffer ready.
	 */
	if (likely (qh != 0)) {
		urb->hcpriv = qh_get (qh);
		if (likely (qh->qh_state == QH_STATE_IDLE))
			qh_link_async (ehci, qh_get (qh));
	}
	spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> 	spin_unlock_irqrestore (&ehci->lock, flags);
	if (unlikely (qh == 0)) {
		qtd_list_free (ehci, urb, qtd_list);
		return -ENOMEM;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* the async qh for the qtds being reclaimed are now unlinked from the HC */
/* caller must not own ehci->lock */

void end_unlink_async (struct ehci_hcd *ehci)
{
	struct ehci_qh		*qh = ehci->reclaim;

	qh->qh_state = QH_STATE_IDLE;
	qh->qh_next.qh = 0;
	qh_put (ehci, qh);			// refcount from reclaim 
	ehci->reclaim = 0;
	ehci->reclaim_ready = 0;

	qh_completions (ehci, qh, 1);

	// unlink any urb should now unlink all following urbs, so that
	// relinking only happens for urbs before the unlinked ones.
	if (!list_empty (&qh->qtd_list)
			&& HCD_IS_RUNNING (ehci->hcd.state))
		qh_link_async (ehci, qh);
	else
		qh_put (ehci, qh);		// refcount from async list
}

//eason 20100818	extern  void ehci_tasklet (unsigned long param);
extern void ehci_tasklet(cyg_addrword_t arg);
extern struct ehci_hcd *ehci_tasklet_hcd;
extern cyg_sem_t ehci_tasklet_sem;
extern cyg_mutex_t ehci_tasklet_mt;

/* makes sure the async qh will become idle */
/* caller must own ehci->lock */

void start_unlink_async (struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	int		cmd = readl (&ehci->regs->command);
	struct ehci_qh	*prev;

#ifdef DEBUG
	if (ehci->reclaim
			|| !ehci->async
			|| qh->qh_state != QH_STATE_LINKED
#ifdef CONFIG_SMP
// this macro lies except on SMP compiles
			|| !spin_is_locked (&ehci->lock)
#endif
			)
#ifdef EHCI_DEBUG	//635u2
		BUG ();
#endif		
#endif

	qh->qh_state = QH_STATE_UNLINK;
	ehci->reclaim = qh = qh_get (qh);

	// dbg_qh ("start unlink", ehci, qh);

	/* Remove the last QH (qhead)?  Stop async schedule first. */
	if (unlikely (qh == ehci->async && qh->qh_next.qh == qh)) {
		/* can't get here without STS_ASS set */
		if (ehci->hcd.state != USB_STATE_HALT) {
			if (cmd & CMD_PSE)
				writel (cmd & ~CMD_ASE, &ehci->regs->command);
			else {
				ehci_ready (ehci);
				while (readl (&ehci->regs->status) & STS_ASS)
					udelay (100);
			}
		}
		qh->qh_next.qh = ehci->async = 0;

		ehci->reclaim_ready = 1;
//original Source	//635u2
//		tasklet_schedule (&ehci->tasklet);
	ehci_tasklet_hcd = ehci;
//eason 20100818	ehci_tasklet(ehci);
	cyg_semaphore_post(&ehci_tasklet_sem);
		return;
	} 

	if (unlikely (ehci->hcd.state == USB_STATE_HALT)) {
		ehci->reclaim_ready = 1;
//original Source	//635u2
//		tasklet_schedule (&ehci->tasklet);
	ehci_tasklet_hcd = ehci;
	cyg_semaphore_post(&ehci_tasklet_sem);
//ehci_tasklet(ehci);
		return;
	}

	prev = ehci->async;
	while (prev->qh_next.qh != qh && prev->qh_next.qh != ehci->async)
		prev = prev->qh_next.qh;
#ifdef DEBUG
	if (prev->qh_next.qh != qh)
		BUG ();
#endif

	if (qh->hw_info1 & __constant_cpu_to_le32 (QH_HEAD)) {
		ehci->async = prev;
		prev->hw_info1 |= __constant_cpu_to_le32 (QH_HEAD);
	}
	prev->hw_next = qh->hw_next;
	prev->qh_next = qh->qh_next;
	wmb ();

	ehci->reclaim_ready = 0;
	cmd |= CMD_IAAD;
	writel (cmd, &ehci->regs->command);
	/* posted write need not be known to HC yet ... */
}

/*-------------------------------------------------------------------------*/

void scan_async (struct ehci_hcd *ehci)
{
	struct ehci_qh		*qh;
	unsigned long		flags;

	spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
rescan:
	qh = ehci->async;
	if (likely (qh != 0)) {
		do {
			/* clean any finished work for this qh */
			if (!list_empty (&qh->qtd_list)) {
				// dbg_qh ("scan_async", ehci, qh);
				qh = qh_get (qh);
				spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> 	spin_unlock_irqrestore (&ehci->lock, flags);

				/* concurrent unlink could happen here */
				qh_completions (ehci, qh, 1);

				spin_lock_irqsave (&ehci->lock, &flags);	//ZOT==> spin_lock_irqsave (&ehci->lock, flags);
				qh_put (ehci, qh);
			}

			/* unlink idle entries (reduces PCI usage) */
			if (list_empty (&qh->qtd_list) && !ehci->reclaim) {
				if (qh->qh_next.qh != qh) {
					// dbg ("irq/empty");
					start_unlink_async (ehci, qh);
				} else {
					// FIXME:  arrange to stop
					// after it's been idle a while.
				}
			}
			qh = qh->qh_next.qh;
			if (!qh)		/* unlinked? */
				goto rescan;
		} while (qh != ehci->async);
	}

	spin_unlock_irqrestore (&ehci->lock, &flags);	//ZOT==> 	spin_unlock_irqrestore (&ehci->lock, flags);
}
