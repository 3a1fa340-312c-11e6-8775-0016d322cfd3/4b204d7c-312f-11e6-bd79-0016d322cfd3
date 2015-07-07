#include <cyg/kernel/kapi.h>	// Kernel API.
#include <cyg/infra/diag.h>		// For diagnostic printing.
//ZOT716u2
#include "pstarget.h"
#include "psglobal.h"
#include "star_intc.h"
#include "star_gpio.h"
#include "star_misc.h"
#include "star_powermgt.h"
#include "star_timer.h"

#if 0
cyg_interrupt ZOT_GPIO_interrupt;
cyg_handle_t  ZOT_GPIO_interrupt_handle;
#endif

unsigned int PLL_clock;
unsigned int CPU_clock;
unsigned int AHB_clock;
unsigned int APB_clock;

#if 0
cyg_uint32 zot_gpio_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	cyg_interrupt_mask(vector);
	cyg_interrupt_acknowledge(vector);
	return(CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);	
}

cyg_uint32 zot_gpio_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	cyg_interrupt_disable();
	HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();	//ZOT716u2
}
#endif

unsigned long ttb_base = 0x500000;	//ZOT==> 0 + 0x4000;

struct ARM_MMU_FIRST_LEVEL_FAULT {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_FAULT_ID 0x0

struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
    int id : 2;
    int imp : 2;
    int domain : 4;
    int sbz : 1;
    int base_address : 23;
};
#define ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1

struct ARM_MMU_FIRST_LEVEL_SECTION {
    int id : 2;
    int b : 1;
    int c : 1;
    int imp : 1;
    int domain : 4;
    int sbz0 : 1;
    int ap : 2;
    int sbz1 : 8;
    int base_address : 12;
};
#define ARM_MMU_FIRST_LEVEL_SECTION_ID 0x2

struct ARM_MMU_FIRST_LEVEL_RESERVED {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_RESERVED_ID 0x3

union ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARM_MMU_FIRST_LEVEL_FAULT fault;
    struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct ARM_MMU_FIRST_LEVEL_SECTION section;
    struct ARM_MMU_FIRST_LEVEL_RESERVED reserved;
};

#define ARM_UNCACHEABLE                         0
#define ARM_CACHEABLE                           1
#define ARM_UNBUFFERABLE                        0
#define ARM_BUFFERABLE                          1

#define ARM_ACCESS_PERM_NONE_NONE               0
#define ARM_ACCESS_PERM_RO_NONE                 0
#define ARM_ACCESS_PERM_RO_RO                   0
#define ARM_ACCESS_PERM_RW_NONE                 1
#define ARM_ACCESS_PERM_RW_RO                   2
#define ARM_ACCESS_PERM_RW_RW                   3

#define ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
   (unsigned long *)((unsigned long)(ttb_base) + ((table_index) << 2))

#define ARM_FIRST_LEVEL_PAGE_TABLE_SIZE 0x4000

#define ARM_MMU_SECTION(ttb_base, actual_base, virtual_base,              \
                        cacheable, bufferable, perm)                      \
    CYG_MACRO_START                                                       \
        register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
                                                                          \
        desc.word = 0;                                                    \
        desc.section.id = ARM_MMU_FIRST_LEVEL_SECTION_ID;                 \
        desc.section.imp = 1;                                             \
        desc.section.domain = 0;                                          \
        desc.section.c = (cacheable);                                     \
        desc.section.b = (bufferable);                                    \
        desc.section.ap = (perm);                                         \
        desc.section.base_address = (actual_base);                        \
        *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, (virtual_base)) \
                            = desc.word;                                  \
    CYG_MACRO_END

#define X_ARM_MMU_SECTION(abase,vbase,size,cache,buff,access)      \
    { int i; int j = abase; int k = vbase;                         \
      for (i = size; i > 0 ; i--,j++,k++)                          \
      {                                                            \
        ARM_MMU_SECTION(ttb_base, j, k, cache, buff, access);      \
      }                                                            \
    }


void CacheEnable(void)
{
    
	unsigned long *mmu_tlb_base = 0x500000; //ZOT==> 0 + 0x4000;
    unsigned long i;
    
    memset(mmu_tlb_base,0,0x4000);

//                  Actual    Virtual  Size    Attributes                                                    Function
    //		         Base      Base     MB      cached?        buffered?        access permissions
    //             xxx00000  xxx00000
    X_ARM_MMU_SECTION(0x000,  0x000,    5,         ARM_CACHEABLE,   ARM_UNBUFFERABLE,   ARM_ACCESS_PERM_RW_RW); // SDRAM (& LCD registers?)
	X_ARM_MMU_SECTION(0x005,  0x005, (1024*4)-5,   ARM_UNCACHEABLE,   ARM_UNBUFFERABLE,   ARM_ACCESS_PERM_RW_RW);

	asm (	"mov	r0, #0\n"
			
			/*  clean Dcache all */
			"mcr	p15, 0, r0, c7, c14, 0\n"
			
			/*  invalidate I cache */
			"mcr	p15, 0, ip, c7, c5, 0\n"		
			
			/* drain WB */
			"mcr	p15, 0, r3, c7, c10, 4\n"	
			
			/* invalidate BTB */
			"mcr	p15, 0, r3, c7, c5, 6\n"		
			"nop\n"
			"nop\n"
			
			/* load page table pointer */
			"mov	r4, %0\n"
			"mcr	p15, 0, r4, c2, c0, 0\n"		
			
			/* invalidate UTLB */
			"mcr	p15, 0, r0, c8, c7, 0\n"
			"nop\n"
			"nop\n"
			
			/* turn-on ECR */
			"mov     r0, #1\n"
			"nop\n"
			"nop\n"
        	"mcr     p15, 0, r0, c1, c1, 0\n"
			
			/* Domains 0, 1 = manager, 2 = client */
			"mov	r0, #0x2f\n"
			/* load domain access register */
			"mcr	p15, 0, r0, c3, c0\n"
				
			/* get control register v4 */
//			"mrc	p15, 0, r0, c0, c0, 1\n"	
			"mrc	p15, 0, r0, c1, c0, 0\n"	
			"ldr	r1, =0x197D\n"
			"orr	r0, r0, r1\n"
			"mcr	p15, 0, r0, c1, c0, 0\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"mov	r0, #0x0\n"
			"mrc	p15, 0, r0, c7, c5,	4\n"	
			::"r"(ttb_base) );	



	//Enable I-cache.
//Test
//	asm("mrc	p15, 0, r0, c1, c0, 0");
//    asm("ldr	r2, =0x00000001");
//    asm("orr	r0, r0, r2");
//    asm("mcr	p15, 0, r0, c1, c0, 0");

//	asm("mrc	p15, 0, r0, c1, c0, 0");
//    asm("ldr	r2, =0x00000002");
//    asm("orr	r0, r0, r2");
//    asm("mcr	p15, 0, r0, c1, c0, 0");

//
//    asm("mrc	p15, 0, r0, c1, c0, 0");
//    asm("ldr	r2, =0x00001000");
//    asm("orr	r0, r0, r2");
//    asm("mcr	p15, 0, r0, c1, c0, 0");            
}

void CacheDisable(void)
{
	asm (	"mov	r0, #0\n"
		
		/*  clean Dcache all */
		"mcr	p15, 0, r0, c7, c14, 0\n"
		
		/*  invalidate I cache */
		"mcr	p15, 0, ip, c7, c5, 0\n"		
		
		/* drain WB */
		"mcr	p15, 0, r3, c7, c10, 4\n"	
		
		/* invalidate BTB */
		"mcr	p15, 0, r3, c7, c5, 6\n"		
		"nop\n"
		"nop\n"
		);
}

void IOInit(void)
{
	unsigned int val;
	
	//===================================================	
	//Timer2 for udelay
	//===================================================	
	switch (PWRMGT_SYSTEM_CLOCK_CONTROL_REG & 0x3) {
	case 0x0:
		PLL_clock = 175000000;
		break;
	case 0x1:
		PLL_clock = 200000000;
		break;
	case 0x2:
		PLL_clock = 225000000;
		break;
	case 0x3:
		PLL_clock = 250000000;
		break;
	}

	CPU_clock = PLL_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 2) & 0x3) + 1);
	AHB_clock = CPU_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 4) & 0x3) + 1);
	APB_clock = AHB_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 8) & 0x3) + 1);

	val = TIMER1_TIMER2_CONTROL_REG;

	// Clock Source: PCLK
	val &= ~(1 << TIMER2_CLOCK_SOURCE_BIT_INDEX);

	// Down Count Mode
	val |= (1 << TIMER2_UP_DOWN_COUNT_BIT_INDEX);

	TIMER1_TIMER2_CONTROL_REG = val;

	//===================================================	
	//Reset button (GPIO 0, EXT interrupt 29)
	//===================================================	
#if 0
	HAL_MISC_ENABLE_EXT_INT29_PINS();
	val = INTC_INTERRUPT_TRIGGER_MODE_REG;
	//Level
	if (val & (1UL << 29)) {
		val &= ~(1UL << 29);
		INTC_INTERRUPT_TRIGGER_MODE_REG = val;
	}
	//Low active
	val = INTC_INTERRUPT_TRIGGER_LEVEL_REG;

		if (!(val & (1UL << 29))) {
			val |= (1UL << 29);
			INTC_INTERRUPT_TRIGGER_LEVEL_REG = val;
		}

    cyg_interrupt_create(29,
                             0,                     
                             0,                     
                             &zot_gpio_isr,
                             &zot_gpio_dsr,
                             &ZOT_GPIO_interrupt_handle,
                             &ZOT_GPIO_interrupt);
    cyg_interrupt_attach(ZOT_GPIO_interrupt_handle); 

	HAL_INTC_ENABLE_INTERRUPT_SOURCE(29);
#endif	
	//===================================================	
	//LED init (GPIO 16, GPIO 15(green)+GPIO 14(yellow))
	//===================================================		
	//Output
	GPIOA_DIRECTION_REG = 0x0001c000;
}
