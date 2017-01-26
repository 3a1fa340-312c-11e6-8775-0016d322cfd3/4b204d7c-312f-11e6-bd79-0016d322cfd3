#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "led.h"
#if defined(ARCH_ARM)
#include "star_gpio.h"	//ZOT716u2
#endif /* defined ARCH_ARM */
#if defined(ARCH_MIPS)
#include "ralink_gpio.h"
#endif /* defined ARCH_MIPS */

//LED Task Task create information definition
#define LED_TASK_PRI         20	//ZOT716u2
#define LED_TASK_STACK_SIZE  2048//512 //ZOT716u2 1024

static unsigned char	LED_Stack[LED_TASK_STACK_SIZE];
static cyg_thread       LED_Task;
static cyg_handle_t     LED_TaskHdl;

#define GPIO_REG_OUTPUTS    0x8000D00C //copy from api.c

//ZOT716u2 unsigned short CurLEDvalue;
unsigned int CurLEDvalue;	//ZOT716u2

unsigned char StatusLightToggle =0;
unsigned char USB11LightToggle = 0;
unsigned char USB20LightToggle = 0;
unsigned char USBTestLightToggle = 0;
WORD LEDErrorCode;

#if defined(WIRELESS_CARD)
BYTE WirelessLightToggle;
#endif
BYTE LANLightToggle = 0;		//eason 20100809
BYTE LANLightToggle_down = 0;	//eason 20100809
BYTE LANLightToggle_100 = 0;

void LightToggleProc(cyg_addrword_t data);
//void LightToggleProc(void);
////////// LED Function //////////////////////////////////////
#define W81_REGS32(x)    (*(volatile unsigned long *)(x))

void LED_Init(void)
{
#if defined(ARCH_ARM)
    int value;

	//LED (GPIO 16) Light on	
#if defined(N716U2W) || defined(N716U2)
	value = GPIOA_DATA_INPUT_REG;
    value |= (GPIO_14_MASK|GPIO_15_MASK);
#endif
#if defined(NDWP2020)
    //value = 0x00014000;
	value = GPIOA_DATA_INPUT_REG;
    value |= (GPIO_14_MASK|GPIO_16_MASK);
#endif

	GPIOA_DATA_OUTPUT_REG = value;
#endif /* defined ARCH_ARM */

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
    #if defined(ARCH_ARM)
	unsigned int value;
	
	value = GPIOA_DATA_INPUT_REG;
	value &= LED_MASK;
    #endif /* defined ARCH_ARM */
	switch(LED){
		case Status_Lite:
            #if defined(ARCH_ARM)
            #if defined(N716U2W) || defined(N716U2)
			value &= ~(GPIO_16_MASK);
            #endif
            #if defined(NDWP2020)
            value &= ~(GPIO_15_MASK);
            #endif
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            light_status_on();
            #endif /* defined ARCH_MIPS */
			break;
		case Usb11_Lite:
        case Usb20_Lite:
            #if defined(ARCH_ARM)
            #if defined(N716U2W) || defined(N716U2)
			value &= ~(GPIO_15_MASK);
            #endif
            #if defined(NDWP2020)
			value &= ~(GPIO_16_MASK);
            #endif
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            light_usb_on();
            #endif
			break;
		case Wireless_Lite:
            #if defined(ARCH_ARM)
			value &= ~(GPIO_14_MASK);
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            light_wireless_on();
            #endif
			break;
		}
    #if defined(ARCH_ARM)
	GPIOA_DATA_OUTPUT_REG = value;
    #endif /* defined ARCH_ARM */
}

void Light_Off(unsigned char LED)
{
    #if defined(ARCH_ARM)
	unsigned int value;
	
	value = GPIOA_DATA_INPUT_REG;
	value &= LED_MASK;
    #endif /* defined ARCH_ARM */
	switch(LED){
		case Status_Lite:
            #if defined(ARCH_ARM)
            #if defined(N716U2W) || defined(N716U2)
            value |= (GPIO_16_MASK);
            #endif
            #if defined(NDWP2020)
            value |= (GPIO_15_MASK);
            #endif
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            light_status_off();
            #endif
			break;
		case Usb11_Lite:
		case Usb20_Lite:			
            #if defined(ARCH_ARM)
            #if defined(N716U2W) || defined(N716U2)
			value |= (GPIO_15_MASK);
            #endif
            #if defined(NDWP2020)
			value |= (GPIO_16_MASK);
            #endif
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            light_usb_off();
            #endif /* defined ARCH_MIPS */
			break;			
		case Wireless_Lite:
            #if defined(ARCH_ARM)
			value |= (GPIO_14_MASK);
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            light_wireless_off();
            #endif /* defined ARCH_MIPS */
			break;
		}
    #if defined(ARCH_ARM)
	GPIOA_DATA_OUTPUT_REG = value;
    #endif /* defined ARCH_ARM */
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

//ZOT716u2 int Lan_Lite_State = Lan_Lite_100;

//ZOT716u2 int rsa_key_Flash_Flag =0;

//ZOT716u2 extern uint32 usb_devices_per_port[0] ; 
//ZOT716u2 extern uint8 AssociatedFlag;

#ifdef WIRELESS_CARD
extern int wlan_get_linkup ();
#endif
extern uint32 usb_devices_per_port[NUM_PORTS] ;

#ifdef WPSBUTTON_LEDFLASH_FLICK
// Lance
extern int flash_wps_led;
#endif	// WPSBUTTON_LEDFLASH_FLICK

void LightToggleProc(cyg_addrword_t data)
{
	int nLEDValue,nTempLEDValue;
	uint32 start_timer = 0;
	int nUSBLED=0;

#ifdef WPSBUTTON_LEDFLASH_FLICK
	int flash_wps_led_count = 0;
	int flash_wps_led_state = 0;
#endif	// WPSBUTTON_LEDFLASH_FLICK

    Light_On(Status_Lite);
	start_timer = jiffies;

	for(;;) {

        /* Read the current GPIO value */

#ifdef WPSBUTTON_LEDFLASH_FLICK
        if(flash_wps_led == 1)
		{
			flash_wps_led_count++;

			if(flash_wps_led_state == 0)
			{
					flash_wps_led_state = 1;
					Light_On( Wireless_Lite );
			}
			else
			{
					flash_wps_led_state = 0;
					Light_Off( Wireless_Lite );
			}

			if(flash_wps_led_count > 300)
			{
					flash_wps_led = 0;
			}
		}
      	else
      	{
	      	flash_wps_led_count = 0;
#endif	// WPSBUTTON_LEDFLASH_FLICK
#ifdef WIRELESS_CARD
            if( wlan_get_linkup() == 1 )
                Light_On( Wireless_Lite );
            else {
                Light_Off( Wireless_Lite );
            }
#endif

#ifdef WPSBUTTON_LEDFLASH_FLICK
        }	// end of if(flash_wps_led == 1)
#endif	// WPSBUTTON_LEDFLASH_FLICK

      	if(usb_devices_per_port[0] == 1)
      		Light_On(Usb11_Lite);
      	else
      		Light_Off(Usb11_Lite);
      	
      	
		nLEDValue = 0; 

		if (StatusLightToggle){
			nLEDValue |= (1 <<Status_Lite);
			StatusLightToggle--;			
			if( StatusLightToggle > 5 ) StatusLightToggle = 5;	
		}

#if defined(WIRELESS_CARD)
		if( WirelessLightToggle )
		{
			nLEDValue |= (1 << Wireless_Lite);

			WirelessLightToggle--;
			if( WirelessLightToggle > 5 ) WirelessLightToggle = 5;
		}
#endif
#if defined(N716U2W) || defined(N716U2)
		if( LANLightToggle )
		{
            nLEDValue |= (1 << Status_Lite);
#if defined(N716U2)
            nLEDValue |= (1 << Network_Lite);
#endif /* N716U2 */

			LANLightToggle--;
			if( LANLightToggle > 5 ) LANLightToggle = 5;
		}
		
		if( LANLightToggle_down )
		{
			Light_Off(Status_Lite);
            cyg_thread_delay(50);
			Light_On( Status_Lite );	
            cyg_thread_delay(50);
            // LANLightToggle_down--;
            // if( LANLightToggle_down > 5 ) LANLightToggle_down = 5;
		}
#endif

#ifdef  USB_LED		
#if 0	//ZOT716u2
		if (USBLightToggle){
		    nLEDValue |= (1 << Usb_Lite);

			USBLightToggle--;			
			if( USBLightToggle > 5 ) USBLightToggle = 5;	
		}
#else	//ZOT716u2
		if (USBTestLightToggle){
			USBTestLightToggle --;
			if(nUSBLED)
			{
				nUSBLED = 0;
				nLEDValue |= (1 << Usb11_Lite);
			}
			else
			{
				nUSBLED = 1;
				nLEDValue |= (1 << Usb20_Lite);
			}
			if( USBTestLightToggle > 5 ) USBTestLightToggle = 5;		
		}else if (USB11LightToggle){
			USB11LightToggle--;	
			nLEDValue |= (1 << Usb11_Lite);		
			if( USB11LightToggle > 5 ) USB11LightToggle = 5;	
		}else if (USB20LightToggle){
			USB20LightToggle--;	
			nLEDValue |= (1 << Usb20_Lite);
			if( USB20LightToggle > 5 ) USB20LightToggle = 5;	
		}		
#endif	//ZOT716u2	
#endif		
#if 0	//ZOT716u2
		if (nLEDValue){
			WL_READ_WORD(GPIO_REG_OUTPUTS, CurLEDvalue);	
			
			CurLEDvalue |= nLEDValue;			
			
			WL_WRITE_WORD(GPIO_REG_OUTPUTS, CurLEDvalue);		
			ppause(50);
			
			if( (usb_devices_per_port[0] == 0) && ( nLEDValue & (1 << Usb_Lite)) )
				nLEDValue &= ~(1 << Usb_Lite);
			
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
            #if defined(ARCH_ARM)
			CurLEDvalue = GPIOA_DATA_INPUT_REG;	
			nTempLEDValue = CurLEDvalue;
			CurLEDvalue &= LED_MASK;
            #endif /* defined ARCH_ARM */
			
			//Light off
			if(nLEDValue & (1 << Status_Lite)){
                #if defined(ARCH_ARM)
                #if defined(N716U2W) || defined(N716U2)
				CurLEDvalue |= (GPIO_16_MASK);
                #endif
                #if defined(NDWP2020)
				CurLEDvalue |= (GPIO_15_MASK);
                #endif
                #endif /* defined ARCH_ARM */
                #if defined(ARCH_MIPS)
                light_status_off();
                #if defined(N716U2) 
                light_network_off();
                #endif /* N716U2 */
                #endif /* defined ARCH_MIPS */
			}

#if defined(WIRELESS_CARD)
			if(nLEDValue & (1 << Wireless_Lite)){
                #if defined (ARCH_ARM)
				CurLEDvalue |= (GPIO_14_MASK);
                #endif /* defined ARCH_ARM */
                #if defined(ARCH_MIPS)
                light_wireless_off();
                #endif /* defined ARCH_MIPS */
			}
#endif
			
			if((nLEDValue & (1 << Usb11_Lite)) || (nLEDValue & (1 << Usb20_Lite))){		
                #if defined(ARCH_ARM)
                #if defined(N716U2W) || defined(N716U2)
				CurLEDvalue |= (GPIO_15_MASK);
                #endif
                #if defined(NDWP2020)
				CurLEDvalue |= (GPIO_16_MASK);
                #endif
                #endif /* defined ARCH_ARM */
                #if defined(ARCH_MIPS)
                light_usb_off();
                #endif /* defined ARCH_MIPS */
			}				
						
            #if defined(ARCH_ARM)
			GPIOA_DATA_OUTPUT_REG = CurLEDvalue;		
            #endif /* defined ARCH_ARM */
			ppause(50);

/*
			CurLEDvalue = GPIOA_DATA_INPUT_REG;	
			CurLEDvalue &= LED_MASK;
			
			//Light on
			if(nLEDValue & (1 << Status_Lite)){
				CurLEDvalue &= ~(GPIO_16_MASK);
			}

#if defined(WIRELESS_CARD)
			if(nLEDValue & (1 << Wireless_Lite)){
				CurLEDvalue &= ~(GPIO_14_MASK);
			}
#endif

			if(nLEDValue & (1 << Usb11_Lite)){		
				CurLEDvalue &= ~(GPIO_15_MASK);
			}else if(nLEDValue & (1 << Usb20_Lite)){
				CurLEDvalue &= ~(GPIO_15_MASK);
			}			
*/						
            #if defined(ARCH_ARM)
			CurLEDvalue = nTempLEDValue;
			GPIOA_DATA_OUTPUT_REG = CurLEDvalue;		
            #endif /* defined ARCH_ARM */
            #if defined(ARCH_MIPS)
            if (nLEDValue & (1 << Status_Lite)){
                light_status_on();
            }
            if (nLEDValue & (1 << Wireless_Lite)){
                light_wireless_on();
            }
            if (nLEDValue & (1 << Usb11_Lite)) {
                light_usb_on();
            }
            else if(nLEDValue & (1 << Usb20_Lite)) {
                light_usb_on(); 
            }
            if (nLEDValue & (1 << Network_Lite)) {
                #if defined(N716U2)
                if (LANLightToggle_down == 0) {
                    if (LANLightToggle_100)
                        light_network_on_100M();
                    else
                        light_network_on_10M();
                }
                #endif /* N716U2 */
            }
            #endif /* defined ARCH_MIPS */
			ppause(30);
			
		} else {					
			/* wait for next polling*/
			ppause(50);
		}
#endif	//ZOT716u2			
        sys_check_stack();
	}

}

//ZOT716u2 void Light_Flash( int nOnLoop, int nOffLoop ){}

//ZOT716u2 void Light_ALL_Flash( int nOnLoop, int nOffLoop ){}
