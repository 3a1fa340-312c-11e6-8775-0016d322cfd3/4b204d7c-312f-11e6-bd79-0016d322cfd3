/* memory allocation routines
 * Copyright 1991 Phil Karn, KA9Q
 *
 * Adapted from alloc routine in K&R; memory statistics and interrupt
 * protection added for use with net package. Must be used in place of
 * standard Turbo-C library routines because the latter check for stack/heap
 * collisions. This causes erroneous failures because process stacks are
 * allocated off the heap.
 */

/* Free memory threshold, below which things start to happen to conserve
 * memory, like garbage collection, source quenching and refusing connects
 */
typedef unsigned int size_t;
#define	HUGE
#define NULL	0
#define TOTAL_RAM_SIZE (7+1)*8
#define BLOCK2_BEGIN_SEG  0x1000
#define BLOCK2_SIZE       0x2000L  //8K
#define MEM_BLOCKS        (TOTAL_RAM_SIZE/8)	 //12/18/98
#define HW_RAM	7

unsigned char _FarHeap[HW_RAM+1][BLOCK2_SIZE];
unsigned long Availmem;      /* Heap memory, ABLKSIZE units */
unsigned long mini_Availmem;  //minimun available memory

union header {
	struct {
		union header HUGE *Next;
		unsigned int          McbSize;
	} s;
	char c[12];	/* For debugging; also ensure size is 12 bytes */
};

typedef union header MCB;
#define	MCBSIZE	(sizeof (MCB))
#define	BTOU(nb)	((((nb) + MCBSIZE - 1) / MCBSIZE) + 1)

static MCB Base;
static MCB HUGE *Allocp;

static void mallocinit(void)
{
	MCB  HUGE *CurBlock;
	unsigned short   nu;

	unsigned short BlockSize;

	Base.s.McbSize = 1;
	Allocp = CurBlock = &Base;

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

#if (MEM_BLOCKS > 2)
{
	unsigned short  i;

	nu = (BLOCK2_SIZE-1)/MCBSIZE; //64K

	//Charles
	//for(i = 1 ; i < MEM_BLOCKS ; i++) {
	for(i = 0 ; i < MEM_BLOCKS ; i++) {
		//CurBlock->s.Next = (MCB HUGE*) MK_FP(BLOCK2_BEGIN_SEG*i,0);
		CurBlock->s.Next = (MCB HUGE*)_FarHeap[i];
		CurBlock->s.Next->s.McbSize = nu;
		CurBlock = CurBlock->s.Next;
		Availmem +=	nu;
	}
}
#else
#if (MEM_BLOCKS == 2)
//Memory Block 2 (TOTAL RAM SIZE = 128K)
	nu = (BLOCK2_SIZE-1)/MCBSIZE; //64K
	CurBlock->s.Next = (MCB HUGE*) MK_FP(BLOCK2_BEGIN_SEG,0);
	CurBlock->s.Next->s.McbSize = nu;
	CurBlock = CurBlock->s.Next;
	Availmem +=	nu;
#endif
#endif

	CurBlock->s.Next = &Base;
	mini_Availmem = Availmem; //3/29/99
}

/* Allocate block of 'nb' bytes */
void *
malloc(size_t nb)
{
	register MCB HUGE *CurMcb, HUGE *PreMcb;
	register unsigned nu;
	int i_state;
	int i;

	if(nb == 0) return NULL;

	/* Round up to full block, then add one for header */
	nu = BTOU(nb);

//	i_state = dirps();

	/* Initialize heap pointers if necessary */
	if(Allocp == NULL) mallocinit();

	/* Search heap list */
	for(PreMcb = Allocp, CurMcb = PreMcb->s.Next;
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
			Availmem -= CurMcb->s.McbSize;
			if(Availmem < mini_Availmem) mini_Availmem = Availmem; //3/29/99
			CurMcb++;
			break;
		}
		if(CurMcb == Allocp) {
		    CurMcb = NULL;
		    break;
		}
	}

//	restore(i_state);
	return (void *)CurMcb;
}

/* Put memory block back on heap */
void
free(void *block)
{
	register MCB HUGE *CurMcb, HUGE *FreeMcb;
	int i_state;

	if(block == NULL)	return;		/* Required by ANSI */
	CurMcb = ((MCB HUGE *)block) - 1;

	/* Audit check */
	if(CurMcb->s.Next != CurMcb){
		return;
	}

//	i_state = dirps();

	Availmem += CurMcb->s.McbSize;

 	/* Search the free list looking for the right place to insert */
	for(FreeMcb = Allocp;
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
	} else {
		/* Link to front of this entry */
		CurMcb->s.Next = FreeMcb->s.Next;
	}
	if(FreeMcb + FreeMcb->s.McbSize == CurMcb){
		/* Combine with end of this entry */
		FreeMcb->s.McbSize += CurMcb->s.McbSize;
		FreeMcb->s.Next = CurMcb->s.Next;
	} else {
		/* Link to end of this entry */
		FreeMcb->s.Next = CurMcb;
	}
//	restore(i_state);

}
