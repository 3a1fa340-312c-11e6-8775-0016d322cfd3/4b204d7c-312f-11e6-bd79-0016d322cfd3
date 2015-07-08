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

cyg_interrupt ZOT_GPIO_interrupt;
cyg_handle_t  ZOT_GPIO_interrupt_handle;

unsigned int PLL_clock;
unsigned int CPU_clock;
unsigned int AHB_clock;
unsigned int APB_clock;

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

void CacheEnable(void)
{
	//Enable I-cache.
    asm("mrc	p15, 0, r0, c1, c0, 0");
    asm("ldr	r2, =0x00001000");
    asm("orr	r0, r0, r2");
    asm("mcr	p15, 0, r0, c1, c0, 0");            
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
	
	//===================================================	
	//LED init (GPIO 16, GPIO 15(green)+GPIO 17(yellow))
	//===================================================		
	//Output
	GPIOA_DIRECTION_REG = 0x00038000;
}
