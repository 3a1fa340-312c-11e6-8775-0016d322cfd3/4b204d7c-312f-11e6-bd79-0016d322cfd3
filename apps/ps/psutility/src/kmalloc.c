/* memory allocation routines
 * Copyright 1991 Phil Karn, KA9Q
 *
 * Adapted from alloc routine in K&R; memory statistics and interrupt
 * protection added for use with net package. Must be used in place of
 * standard Turbo-C library routines because the latter check for stack/heap
 * collisions. This causes erroneous failures because process stacks are
 * allocated off the heap.
 */

#include <stdio.h>

#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "psdefine.h"

#define	HUGE
#define huge
#define BLOCK2_SIZE       0x10000L  //64K

//move from alloc.c 8/11/2000
union header {
	struct {
		union header huge *Next;
		uint32          McbSize;
#if defined(PC_OUTPUT) || defined(PS_OUTPUT)
		uint32          Number;
#endif
	} s;
	char c[12];	/* For debugging; also ensure size is 12 bytes */
};

typedef union header MCB;

#define	MCBSIZE	(sizeof (MCB))
#define	BTOU(nb)	((((nb) + MCBSIZE - 1) / MCBSIZE) + 1)


#define KMEM_BLOCKS        (((kHW_RAM+1) * 64)/64)	 //512K

unsigned char *_kFarHeap[kHW_RAM+1];
unsigned char *_kPktHeap[24]; // (128K+64)/8K = 24 

static long kAllocSize;	    /* Total allocate size*/
static long kMaxAllocSize;	/* Total maximun allocate size*/



unsigned long kAvailmem;      /* Heap memory, ABLKSIZE units */
unsigned long kPktAvailmem = 0;
unsigned long kmini_Availmem;  //minimun available memory

static MCB HUGE *kAllocp = NULL;
static MCB kBase;

static MCB HUGE *kPktAllocp;
static MCB kPktBase;

static void kmallocinit(void)
{
	MCB  HUGE *CurBlock;
	uint16   nu;
	uint16 	i;	

	WORD BlockSize;

	kBase.s.McbSize = 1;
	kAllocp = CurBlock = &kBase;

	for(i = 0 ; i < KMEM_BLOCKS; i++) {
		_kFarHeap[i]= kFarHeap_base + i * (0x10000);
	}

    #if 0
    // for wireless packet use
    for (i = 0 ; i < 24; i++) {
        _kPktHeap[i] = kPktHeap_base + i * (0x2000);
    }
    #endif

    //_kFarHeap[i++] = kFarHeap_hugebase;
    //_kFarHeap[i++] = kFarHeap_hugebase + 0x20000;

//Memory Block 1

	//Charles
	/*
	BlockSize = (BLOCK2_BEGIN_SEG - AsmHeapBeginSeg) << 4;

	nu = (BlockSize-1)/MCBSIZE;

	CurBlock->s.Next = (MCB HUGE*) MK_FP(AsmHeapBeginSeg,0);
	CurBlock->s.Next->s.McbSize = nu;
	CurBlock = CurBlock->s.Next;
	Availmem +=	nu;
	*/

#if (KMEM_BLOCKS > 2)
{
	
	nu = (BLOCK2_SIZE-1)/MCBSIZE; //64K

	//Charles
	//for(i = 1 ; i < MEM_BLOCKS ; i++) {
	for(i = 0 ; i < KMEM_BLOCKS ; i++) {
		//CurBlock->s.Next = (MCB HUGE*) MK_FP(BLOCK2_BEGIN_SEG*i,0);
		CurBlock->s.Next = (MCB HUGE*)_kFarHeap[i];
		CurBlock->s.Next->s.McbSize = nu;
		CurBlock = CurBlock->s.Next;
		kAvailmem +=	nu;
	}

    #if 0
    kPktAllocp = CurBlock = &kPktBase;

    nu = (0x2000 - 1)/MCBSIZE;
    for (i = 0 ; i < 24 ; i++) {
        CurBlock->s.Next = (MCB HUGE*)_kPktHeap[i];
        CurBlock->s.Next->s.McbSize = nu;
        CurBlock = CurBlock->s.Next;
        kPktAvailmem += nu;
    }
    #endif
}
#else
#if (KMEM_BLOCKS == 2)
//Memory Block 2 (TOTAL RAM SIZE = 128K)
	nu = (BLOCK2_SIZE-1)/MCBSIZE; //64K
//	CurBlock->s.Next = (MCB HUGE*) MK_FP(BLOCK2_BEGIN_SEG,0);
	CurBlock->s.Next = (MCB HUGE*)_kFarHeap[i];
	CurBlock->s.Next->s.McbSize = nu;
	CurBlock = CurBlock->s.Next;
	kAvailmem +=	nu;
#endif
#endif

	CurBlock->s.Next = &kBase;
	kmini_Availmem = kAvailmem; //3/29/99
}
/* Allocate block of 'nb' bytes */
void *
kmalloc(size_t nb, int flag)
{
	register MCB HUGE *CurMcb, HUGE *PreMcb;
	register unsigned nu;
	int i_state;
	int i;

	if(nb == 0) return NULL;

	/* Round up to full block, then add one for header */
	nu = BTOU(nb);

	i_state = dirps();

	/* Initialize heap pointers if necessary */
	if(kAllocp == NULL) kmallocinit();

	/* Search heap list */
    if (flag == 0) 
        PreMcb = kAllocp;
    else
        PreMcb = kPktAllocp;
   
	for(CurMcb = PreMcb->s.Next;
	    ;
	    PreMcb = CurMcb, CurMcb = CurMcb->s.Next )
	{
		if(CurMcb->s.McbSize >= nu){
			/* This chunk is at least as large as we need */
			if(CurMcb->s.McbSize <= nu + 1){
				/* This is either a perfect fit (size == nu)
				 * or the free chunk is just one unit larger.
				 * In either case, alloc the whole thing,
				 * because there's no point in keeping a free
				 * block only large enough to hold the header.
				 */
				PreMcb->s.Next = CurMcb->s.Next;
			} else {
				/* Carve out piece from end of entry */
				CurMcb->s.McbSize -= nu;
				CurMcb += CurMcb->s.McbSize;
				CurMcb->s.McbSize = nu;
			}
			CurMcb->s.Next = CurMcb;	/* for auditing */
#if defined(PC_OUTPUT) || defined(PS_OUTPUT)
{
			struct address addr[3];
			int j, corrupt = 0;

			CurMcb->s.Number = ++Allocs;
			kAllocSize += CurMcb->s.McbSize;
//			AllocDumpStack(addr);
//			AllocInsertMemCount(CurMcb->s.McbSize,Allocs,addr);
			if(kAllocSize > kMaxAllocSize) kMaxAllocSize = kAllocSize;
#if 0
			if(istate()){
				for(j=1;j<CurMcb->s.McbSize;j++){
					if(memcmp(CurMcb[j].c,Debugpat,sizeof(Debugpat)) != 0){
						corrupt = j;
						break;
					}
				}
				if(corrupt)
					printf("%p %6lu C: %u",CurMcb,CurMcb->s.McbSize * MCBSIZE,corrupt);
			}
#endif /* 0 */
}
#endif /* PC_OUTPUT */
            if (flag == 0) {
			    kAvailmem -= CurMcb->s.McbSize;
			    if(kAvailmem < kmini_Availmem) kmini_Availmem = kAvailmem; //3/29/99
			    CurMcb++;
			    break;
            }
            else {
                kPktAvailmem -= CurMcb->s.McbSize;
                CurMcb++;
                break;
            }
		}
		if((CurMcb == kAllocp) || (CurMcb == kPktAllocp)) {
		    CurMcb = NULL;
#if defined(PC_OUTPUT) || defined(PS_OUTPUT)
			//RED ALARM
			AllocFails++;
	    	printf("Not enough memory !...\n");
#endif
		    break;
		}
	}

	restore(i_state);
	return (void *)CurMcb;
}

/* Put memory block back on heap */
void
kfree(void *block, int flag)
{
	register MCB HUGE *CurMcb, HUGE *FreeMcb;
	int i_state;

	if(block == NULL)	return;		/* Required by ANSI */
	CurMcb = ((MCB HUGE *)block) - 1;

	/* Audit check */
	if(CurMcb->s.Next != CurMcb){
#ifdef PC_OUTPUT
		if(istate()){
			while(1) {
				printf("\afree: WARNING! invalid pointer (%p) proc %s\n",
					block,Curproc->name);
			}
#ifdef STKTRACE
			stktrace();
#endif
		}
#endif PC_OUTPUT
		return;
	}

	i_state = dirps();

    if (flag == 0)
	    kAvailmem += CurMcb->s.McbSize;
    else
        kPktAvailmem += CurMcb->s.McbSize;

#if defined(PC_OUTPUT) || defined(PS_OUTPUT)
	kAllocSize -= CurMcb->s.McbSize;
//	AllocDeleteMemCount(CurMcb->s.McbSize,CurMcb->s.Number);
	if(Memdebug){
		int i;
		/* Fill data area with pattern to detect later overwrites */
		for(i=1;i<CurMcb->s.McbSize;i++){
			memcpy(CurMcb[i].c,Debugpat,sizeof(Debugpat));
		}
	}
#endif PC_OUTPUT

 	/* Search the free list looking for the right place to insert */
    if (flag == 0)
        FreeMcb = kAllocp;
    else
        FreeMcb = kPktAllocp;

	for(;
	    !(CurMcb > FreeMcb && CurMcb < FreeMcb->s.Next);
	    FreeMcb = FreeMcb->s.Next)
	{
		/* Highest address on circular list? */
		if(FreeMcb >= FreeMcb->s.Next &&
		   (CurMcb > FreeMcb || CurMcb < FreeMcb->s.Next))
			break;
	}
	if(CurMcb + CurMcb->s.McbSize == FreeMcb->s.Next){
		/* Combine with front of this entry */
		CurMcb->s.McbSize += FreeMcb->s.Next->s.McbSize;
		CurMcb->s.Next = FreeMcb->s.Next->s.Next;
#ifdef PC_OUTPUT
		if(Memdebug){
			memcpy(FreeMcb->s.Next->c,Debugpat,sizeof(Debugpat));
		}
#endif PC_OUTPUT
	} else {
		/* Link to front of this entry */
		CurMcb->s.Next = FreeMcb->s.Next;
	}
	if(FreeMcb + FreeMcb->s.McbSize == CurMcb){
		/* Combine with end of this entry */
		FreeMcb->s.McbSize += CurMcb->s.McbSize;
		FreeMcb->s.Next = CurMcb->s.Next;
#ifdef PC_OUTPUT
		if(Memdebug){
			memcpy(CurMcb->c,Debugpat,sizeof(Debugpat));
		}
#endif PC_OUTPUT
	} else {
		/* Link to end of this entry */
		FreeMcb->s.Next = CurMcb;
	}
	restore(i_state);

}

void *
kaligned_alloc(size_t nb, size_t align)
{
	uint32 *cp;
	char *p;

	if( align < sizeof(uint32) )
		align = sizeof(uint32);
	align--;

	p = kmalloc( nb + sizeof(uint32) + align, 0 );

	if( p == NULL ) 
        return NULL;

	memset(p, 0, nb + sizeof(uint32) + align);
	cp = ((uint32)( p + sizeof(uint32) + align ) & ~((uint32)align));
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

	kfree( p , 0 );
}

#if 0
/*
int pkt_use = 0;
*/
void *
pkt_alloc(size_t nb, size_t align)
{
	uint32 *cp;
	char *p;

	if( align < sizeof(uint32) )
		align = sizeof(uint32);
	align--;

	p = kmalloc( nb + sizeof(uint32) + align, 1 );

	if( p == NULL ) 
        return NULL;

	memset(p, 0, nb + sizeof(uint32) + align);
	cp = ((uint32)( p + sizeof(uint32) + align ) & ~((uint32)align));
	cp[-1] = p;

    /*
    pkt_use ++;
    */
	return cp;
}

void
pkt_free(void *block)
{
	char *cp = block;
	char *p;

	if( block == NULL ) return;

	p = *(uint32 *)( cp - sizeof(uint32) );

    /*
    pkt_use --;
    */
	kfree( p , 1 );
}
#endif

