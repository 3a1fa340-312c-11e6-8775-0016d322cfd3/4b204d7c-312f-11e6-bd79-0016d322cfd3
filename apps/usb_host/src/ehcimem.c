/*
 * Copyright (c) 2001 by David Brownell
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
 * There's basically three types of memory:
 *	- data used only by the HCD ... kmalloc is fine
 *	- async and periodic schedules, shared by HC and HCD ... these
 *	  need to use pci_pool or pci_alloc_consistent
 *	- driver buffers, read/written by HC ... single shot DMA mapped 
 *
 * There's also PCI "register" data, which is memory mapped.
 * No memory seen by this driver is pagable.
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

//PCI module
extern uint32 * pci_alloc_usb_desc(uint32 * dma_addr);
extern void pci_free_usb_desc(uint32 addr);

/*
void *
kaligned_alloc(size_t nb, size_t align)
{
	uint32 *cp;
	char *p;

	if( align < sizeof(uint32) )
		align = sizeof(uint32);
	align--;

	p = kmalloc( nb + sizeof(uint32) + align, 0 );

	if( p== NULL ) return NULL;

	cp = ((uint32)( p + sizeof(uint32) + align ) & ~(align));
	cp[-1] = p;

	return cp;
}

void
kaligned_free(void *block)
{
	char *cp = block;
	char *p;

	if( block == NULL ) return;

	p = *(uint32 *)( cp - sizeof(uint32) );

	kfree( p );
}
*/

/*kmalloc*/			//635u2
void * ehci_kmalloc( int size )
{
	char *p;
	
	//eason 20100220	p = kaligned_alloc( size, 32 );
	p = kaligned_alloc( size, 4096 );
	if( p )
		memset( p, 0, size );
	return ( p );
}
//719AW
void * frame_kmalloc( int size )
{
	char *p;

	p = kaligned_alloc( size, 4096 );
	if( p )
		memset( p, 0, size );
	return ( p );
}

void ehci_kfree( void *ptr )
{
	kaligned_free( ptr );
}

/*-------------------------------------------------------------------------*/
/* 
 * Allocator / cleanup for the per device structure
 * Called by hcd init / removal code
 */
struct usb_hcd *ehci_hcd_alloc (void)
{
	struct ehci_hcd *ehci;

	ehci = (struct ehci_hcd *)
		kmalloc (sizeof (struct ehci_hcd), GFP_KERNEL);
	if (ehci != 0) {
		memset (ehci, 0, sizeof (struct ehci_hcd));
		return &ehci->hcd;
	}
	return 0;
}

void ehci_hcd_free (struct usb_hcd *hcd)
{
	kfree (hcd_to_ehci (hcd), 0);
}

/*-------------------------------------------------------------------------*/

/* Allocate the key transfer structures from the previously allocated pool */

struct ehci_qtd *ehci_qtd_alloc (struct ehci_hcd *ehci, int flags)
{
	struct ehci_qtd		*qtd;
	dma_addr_t		dma;

//original Source	//635u2
//	qtd = pci_pool_alloc (ehci->qtd_pool, flags, &dma);
//	qtd = ehci_kmalloc (sizeof (struct ehci_qtd));
//719AW	qtd = pci_alloc_usb_desc(&dma);	//615wu
	qtd = ehci_kmalloc (sizeof (struct ehci_qtd));
	dma = (dma_addr_t)qtd;
	
	if (qtd != 0) {
		memset (qtd, 0, sizeof *qtd);
		qtd->qtd_dma = dma;
		qtd->hw_next = EHCI_LIST_END;
		qtd->hw_alt_next = EHCI_LIST_END;
		INIT_LIST_HEAD (&qtd->qtd_list);
	}
	return qtd;
}

void ehci_qtd_free (struct ehci_hcd *ehci, struct ehci_qtd *qtd)
{
//original Source	//635u2
//	pci_pool_free (ehci->qtd_pool, qtd, qtd->qtd_dma);
//	ehci_kfree(qtd);	//635u2
//719AW	pci_free_usb_desc(qtd->qtd_dma);	//615wu
	ehci_kfree(qtd);
//ZOT716u2	armond_printf("EHCI free QTD: %x\n",qtd);
}


struct ehci_qh *ehci_qh_alloc (struct ehci_hcd *ehci, int flags)
{
	struct ehci_qh		*qh;
	dma_addr_t		dma;

//original Source	//635u2
//	qh = (struct ehci_qh *)
//		pci_pool_alloc (ehci->qh_pool, flags, &dma);
//	qh = ehci_kmalloc (sizeof (struct ehci_qh));
//719AW	qh = pci_alloc_usb_desc(&dma);	//615wu
	qh = ehci_kmalloc (sizeof (struct ehci_qh));
	dma = (dma_addr_t)qh;
	
	if (qh) {
		memset (qh, 0, sizeof *qh);
		atomic_set (&qh->refcount, 1);
		qh->qh_dma = dma;
		// INIT_LIST_HEAD (&qh->qh_list);
		INIT_LIST_HEAD (&qh->qtd_list);
	}
	return qh;
}

/* to share a qh (cpu threads, or hc) */
struct ehci_qh *qh_get (/* ehci, */ struct ehci_qh *qh)
{
	// dbg ("get %p (%d++)", qh, qh->refcount.counter);
	atomic_inc (&qh->refcount);
	return qh;
}

void qh_put (struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	// dbg ("put %p (--%d)", qh, qh->refcount.counter);
	if (!atomic_dec_and_test (&qh->refcount))
		return;
	/* clean qtds first, and know this is not linked */
#ifdef EHCI_DEBUG	//635u2
	if (!list_empty (&qh->qtd_list) || qh->qh_next.ptr) {
		dbg ("unused qh not empty!");
		BUG ();
	}
#endif
//original Source	//635u2
//	pci_pool_free (ehci->qh_pool, qh, qh->qh_dma);
//	ehci_kfree(qh);	//635u2
//719AW	pci_free_usb_desc(qh->qh_dma);	//615wu
	ehci_kfree(qh);
//ZOT716u2	armond_printf("EHCI free QH: %x\n",qh);
}

/*-------------------------------------------------------------------------*/

/* The queue heads and transfer descriptors are managed from pools tied 
 * to each of the "per device" structures.
 * This is the initialisation and cleanup code.
 */

void ehci_mem_cleanup (struct ehci_hcd *ehci)
{
	/* PCI consistent memory and pools */
//original Source	//635u2
//	if (ehci->qtd_pool)
//		pci_pool_destroy (ehci->qtd_pool);
	ehci->qtd_pool = 0;

//original Source	//635u2
//	if (ehci->qh_pool) {
//		pci_pool_destroy (ehci->qh_pool);
		ehci->qh_pool = 0;
//	}

//original Source	//635u2
//	if (ehci->itd_pool)
//		pci_pool_destroy (ehci->itd_pool);
	ehci->itd_pool = 0;

//original Source	//635u2
//	if (ehci->sitd_pool)
//		pci_pool_destroy (ehci->sitd_pool);
	ehci->sitd_pool = 0;

//original Source	//635u2
//	if (ehci->periodic)
//		pci_free_consistent (ehci->hcd.pdev,
//			ehci->periodic_size * sizeof (u32),
//			ehci->periodic, ehci->periodic_dma);
	ehci->periodic = 0;

	/* shadow periodic table */
	if (ehci->pshadow)
//719AW		kfree (ehci->pshadow);
			ehci_kfree(ehci->pshadow);
			
	ehci->pshadow = 0;
}

/* remember to add cleanup code (above) if you add anything here */
int ehci_mem_init (struct ehci_hcd *ehci, int flags)
{
//original Source	//635u2
	int i;

	/* QTDs for control/bulk/intr transfers */
//	ehci->qtd_pool = pci_pool_create ("ehci_qtd", ehci->hcd.pdev,
//			sizeof (struct ehci_qtd),
//			32 /* byte alignment (for hw parts) */,
//			4096 /* can't cross 4K */,
//			flags);
//	if (!ehci->qtd_pool) {
#ifdef EHCI_DEBUG	//635u2
//		dbg ("no qtd pool");
#endif
//		ehci_mem_cleanup (ehci);
//		return -ENOMEM;
//	}

	/* QH for control/bulk/intr transfers */
//	ehci->qh_pool = pci_pool_create ("ehci_qh", ehci->hcd.pdev,
//			sizeof (struct ehci_qh),
//			32 /* byte alignment (for hw parts) */,
//			4096 /* can't cross 4K */,
//			flags);
//	if (!ehci->qh_pool) {
#ifdef EHCI_DEBUG	//635u2
//		dbg ("no qh pool");
#endif
//		ehci_mem_cleanup (ehci);
//		return -ENOMEM;
//	}

	/* ITD for high speed ISO transfers */
//	ehci->itd_pool = pci_pool_create ("ehci_itd", ehci->hcd.pdev,
//			sizeof (struct ehci_itd),
//			32 /* byte alignment (for hw parts) */,
//			4096 /* can't cross 4K */,
//			flags);
//	if (!ehci->itd_pool) {
#ifdef EHCI_DEBUG	//635u2
//		dbg ("no itd pool");
#endif
//		ehci_mem_cleanup (ehci);
//		return -ENOMEM;
//	}

	/* SITD for full/low speed split ISO transfers */
//	ehci->sitd_pool = pci_pool_create ("ehci_sitd", ehci->hcd.pdev,
//			sizeof (struct ehci_sitd),
//			32 /* byte alignment (for hw parts) */,
//			4096 /* can't cross 4K */,
//			flags);
//	if (!ehci->sitd_pool) {
#ifdef EHCI_DEBUG	//635u2
//		dbg ("no sitd pool");
#endif
//		ehci_mem_cleanup (ehci);
//		return -ENOMEM;
//	}

	/* Hardware periodic table */
//	ehci->periodic = (u32 *)
//		pci_alloc_consistent (ehci->hcd.pdev,
//			ehci->periodic_size * sizeof (u32),
//			&ehci->periodic_dma);
//719AW
	ehci->periodic = (u32 *) frame_kmalloc(ehci->periodic_size * sizeof (u32));
	ehci->periodic_dma = (uint32)ehci->periodic;
	
	if (ehci->periodic == 0) {
#ifdef EHCI_DEBUG	//635u2
//		dbg ("no hw periodic table");
#endif
		ehci_mem_cleanup (ehci);
		return -ENOMEM;
	}
	for (i = 0; i < ehci->periodic_size; i++)
		ehci->periodic [i] = EHCI_LIST_END;

	/* software shadow of hardware table */
//719AW	ehci->pshadow = kmalloc (ehci->periodic_size * sizeof (void *), flags);
	ehci->pshadow = (uint32)ehci_kmalloc (ehci->periodic_size * sizeof (void *));
	
	if (ehci->pshadow == 0) {
#ifdef EHCI_DEBUG	//635u2
//		dbg ("no shadow periodic table");
#endif		
		ehci_mem_cleanup (ehci);
		return -ENOMEM;
	}
	memset (ehci->pshadow, 0, ehci->periodic_size * sizeof (void *));

	return 0;
}
