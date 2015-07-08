#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "led.h"
#include "star_gpio.h"	//ZOT716u2

//LED Task Task create information definition
#define LED_TASK_PRI         20	//ZOT716u2
#define LED_TASK_STACK_SIZE  512 //ZOT716u2 1024

static unsigned char	LED_Stack[LED_TASK_STACK_SIZE];
static cyg_thread       LED_Task;
static cyg_handle_t     LED_TaskHdl;

#define GPIO_REG_OUTPUTS    0x8000D00C //copy from api.c

//ZOT716u2 unsigned short CurLEDvalue;
unsigned int CurLEDvalue;	//ZOT716u2

//ZOT716u2 unsigned char WirelessLightToggle =0;
unsigned char StatusLightToggle =0;
unsigned char LanLightToggle =0;
unsigned char USBLightToggle = 0;
WORD LEDErrorCode;

void LightToggleProc(cyg_addrword_t data);
//void LightToggleProc(void);
////////// LED Function //////////////////////////////////////
#define W81_REGS32(x)    (*(volatile unsigned long *)(x))

void LED_Init(void)
{
#if 0	//ZOT716u2
#ifdef USB_LED
//	unsigned int val;
		
//	val = W81_REGS32(0x8000A878);
//	W81_REGS32(0x8000A878) = val & ~0x20000;	
#endif		
#endif	//ZOT716u2

	//LED (GPIO 16) Light on	
	GPIOA_DATA_OUTPUT_REG = 0x00028000;

		//Create LED Task Thread
	    cyg_thread_create(LED_TASK_PRI,
	                  LightToggleProc,
	                  0,
	                  "LightToggleProc",
	                  (void *) (LED_Stack),
	                  LED_TASK_STACK_SIZE,
	                  &LED_TaskHdl,
	                  &LED_Task);
		
		//Start LED Task Thread
		cyg_thread_resume(LED_TaskHdl);

}


void Light_On(unsigned char LED)
{
	unsigned int value;
	
	value = GPIOA_DATA_INPUT_REG;
	value &= LED_MASK;
	switch(LED){
		case Status_Lite:
			value &= ~(GPIO_16_MASK);
			break;
		case Usb11_Lite:
			value &= ~(GPIO_15_MASK);
			value |= (GPIO_17_MASK);
			break;
		case Usb20_Lite:
			value &= ~(GPIO_17_MASK);
			value |= (GPIO_15_MASK);
			break;			
		}
	GPIOA_DATA_OUTPUT_REG = value;
}

void Light_Off(unsigned char LED)
{
	unsigned int value;
	
	value = GPIOA_DATA_INPUT_REG;
	value &= LED_MASK;
	switch(LED){
		case Status_Lite:
			value |= (GPIO_16_MASK);
			break;
		case Usb11_Lite:
		case Usb20_Lite:			
			value &= ~(GPIO_15_MASK);
			value &= ~(GPIO_17_MASK);
			break;			
		}
	GPIOA_DATA_OUTPUT_REG = value;
}

//// ----  LED ON Forever -----
void LightOnForever(unsigned char LED)
{
	volatile int nDelay;
	LEDErrorCode = LED;
	cli();
	for (;;) {
	    Light_On( Status_Lite );
		for( nDelay = 0; nDelay < 100000 * 10; nDelay++ );
		Light_Off( Status_Lite );
		for( nDelay = 0; nDelay < 100000 * 10; nDelay++ );
		
	}
	
}



extern uint8   PSUpgradeMode;

void LightToggleProc(cyg_addrword_t data)
{
	int nLEDValue, i;
	int status;
	uint32 start_timer = 0;
	volatile short temp16;
//ZOT716u2	uint8 ForceWirelessLED = 0;
	start_timer = jiffies;
	
	for(;;) {

        /* Read the current GPIO value */
        if( PSUpgradeMode == WAIT_UPGRADE_MODE  )
        {
        	USBLightToggle = 5;
        }
		
		nLEDValue = 0; 
#if 0  //ZOT716u2 		
		if( ForceWirelessLED )	
		{
		if (WirelessLightToggle){
			WirelessLightToggle--;		
			if( WirelessLightToggle > 5 ) WirelessLightToggle = 5;	
		}
		}
#endif	//ZOT716u2  		
		if (StatusLightToggle){
			nLEDValue |= (1 <<Status_Lite);
			StatusLightToggle--;			
			if( StatusLightToggle > 5 ) StatusLightToggle = 5;	
		}

#ifdef  USB_LED		
		if (USBLightToggle){
		    nLEDValue |= (1 << Usb_Lite);

			USBLightToggle--;			
			if( USBLightToggle > 5 ) USBLightToggle = 5;	
		}
#endif		
#if 0	//ZOT716u2
		if (nLEDValue){
			WL_READ_WORD(GPIO_REG_OUTPUTS, CurLEDvalue);	
			CurLEDvalue |= nLEDValue;			
			WL_WRITE_WORD(GPIO_REG_OUTPUTS, CurLEDvalue);		
			ppause(50);
	
			WL_READ_WORD(GPIO_REG_OUTPUTS, CurLEDvalue);
			CurLEDvalue &= ~nLEDValue;
			WL_WRITE_WORD(GPIO_REG_OUTPUTS, CurLEDvalue);		
			ppause(30);
		} else {					
			/* wait for next polling*/
			ppause(50);
		}
#else	//ZOT716u2	
		if (nLEDValue){
			CurLEDvalue = GPIOA_DATA_INPUT_REG;	
			CurLEDvalue &= LED_MASK;
			
			//Light off
			if(nLEDValue & (1 << Status_Lite)){
				CurLEDvalue |= (GPIO_16_MASK);
			}
			if((nLEDValue & (1 << Usb11_Lite)) || (nLEDValue & (1 << Usb20_Lite))){		
				CurLEDvalue &= ~(GPIO_15_MASK);
				CurLEDvalue &= ~(GPIO_17_MASK);
			}				
						
			GPIOA_DATA_OUTPUT_REG = CurLEDvalue;		
			ppause(50);

			CurLEDvalue = GPIOA_DATA_INPUT_REG;	
			CurLEDvalue &= LED_MASK;
			
			//Light on
			if(nLEDValue & (1 << Status_Lite)){
				CurLEDvalue &= ~(GPIO_16_MASK);
			}
			if(nLEDValue & (1 << Usb11_Lite)){		
				CurLEDvalue &= ~(GPIO_15_MASK);
				CurLEDvalue |= (GPIO_17_MASK);
			}else if(nLEDValue & (1 << Usb20_Lite)){
				CurLEDvalue &= ~(GPIO_17_MASK);
				CurLEDvalue |= (GPIO_15_MASK);				
			}			
						
			GPIOA_DATA_OUTPUT_REG = CurLEDvalue;		
			ppause(30);
			
		} else {					
			/* wait for next polling*/
			ppause(50);
		}
#endif	//ZOT716u2			
	}

}

//ZOT716u2 void Light_Flash( int nOnLoop, int nOffLoop ){}

//ZOT716u2 void Light_ALL_Flash( int nOnLoop, int nOffLoop ){}