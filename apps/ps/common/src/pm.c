#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"

#include "star_gpio.h"
#include "star_powermgt.h"
#include "eeprom.h"
#include "led.h"
#define ZOT_IDLE_TASK_PRI         20	
#define ZOT_IDLE_TASK_STACK_SIZE	 3072 
static uint8 			ZOT_IDLE_Stack[ZOT_IDLE_TASK_STACK_SIZE];
static cyg_thread       ZOT_IDLE_Task;
static cyg_handle_t     ZOT_IDLE_TaskHdl;

#ifndef USE_PS_LIBS
#undef DO_STATUS_PRINT
#endif /* USE_PS_LIBS */

#ifdef WPSBUTTON_LEDFLASH_FLICK
int flash_wps_led = 0;
#endif	// WPSBUTTON_LEDFLASH_FLICK

#if defined(ARCH_ARM)
//eason test
extern void set_realtek_wps(int event);	//eason 20100407
#endif /* defined ARCH_ARM */
extern int ResetToDefalutFlash(int tmpip, int tmpkey, int nouse);

void zot_idle_task(cyg_addrword_t data)
{
	unsigned int value = 0 ,value1 = 0;
	unsigned int start = 0, current = 0;
	unsigned int needprint = 0, needboot = 0, needwps = 0;
	
    #if defined(ARCH_MIPS)
    #if defined(N716U2) || defined(N716U2W)
    value = get_reset_input();      /* GPIO39 */
    #endif /* N716U2 || N716U2W */
    #if defined(NDWP2020)
    value = get_wps_input();        /* GPIO38 */
    #endif /* DWP2020 */
    if (value == 0) {
        ResetToDefalutFlash(1, 0, 0);
        REBOOT();//Andy:20170105 Reboot after power on load default+++
    }
    #endif /* ARCH_MIPS */

	while(1){
		
		//===================================================	
		//Reset button (GPIO 0) WPS button (GPIO 17)
		//===================================================	
        #if defined(ARCH_ARM)
		value = GPIOA_DATA_INPUT_REG;
		value &= 0x01;
		value1 = GPIOA_MEM_MAP_VALUE(0x06);
		value1 &= 0xc2;
        #endif /* defined ARCH_ARM */
        #if defined(ARCH_MIPS)
        #if defined(NDWP2020)
        value  = get_wps_input();
        #endif /* NDWP2020 */
        #if defined(N716U2) || defined(N716U2W)
        value  = get_reset_input();
        #endif /* N716U2 || N716U2W */
        #if defined (N716U2W)
        value1 = get_wps_input();
        #endif /* N716U2W */
        #endif /* defined ARCH_MIPS */
		if(value == 0){
			if (start == 0){
				start = cyg_current_time();	
			}else{
				
				current = cyg_current_time();

				// George updated this at build0008 of 716U2W on May 4, 2012.
// #if defined(O_CONRAD)
//                 if( (current - start) > 950 ) needboot = 1;		// 10 seconds
//                 if( (current - start) > 1425 ) needprint = 1;	// 15 seconds
// #else
//                 if( (current - start) > 10 ) needboot = 1;
//                 if( (current - start) > 300 ) needprint = 1;
                #if  defined(NDWP2020)
                if ((current - start) > 500) {
                    needboot = 1;
                    needwps  = 0;
                }
                else needwps = 1;
                #endif /* NDWP2020 */
                //Andy:20170105 add test page+++
                #if defined(N716U2)
                if ((current - start) > 500)
                    needprint=1;
                #endif /* N716U2 */
                //Andy:20170105 add test page---

                #if defined(N716U2W) || defined(N716U2)
                //#if defined(N716U2W)
                if (current - start > 50) needboot = 1;
                #endif /* N716U2W */
//#endif	// defined(O_CONRAD)
			}
		}
        #if defined(N716U2W)
		else if(value1 == 0)
		{
			if (start == 0)
			{
				start = cyg_current_time();	
			}
			else
			{
				current = cyg_current_time();

				if( (current - start) > 50 )
				{
					needwps = 1;
                    #ifdef WPSBUTTON_LEDFLASH_FLICK
					// Lance
					flash_wps_led = 1;
                    #endif	// WPSBUTTON_LEDFLASH_FLICK
				}
			}							
		}
        #endif // defined(N716U2W)
		else
		{
			start = 0;

#ifdef WIRELESS_CARD
			if(needwps)
			{
                #ifndef MTK7601 
                #if defined(ARCH_ARM)
				set_realtek_wps(1);
                #endif /* defined ARCH_ARM */
                #endif 
                #ifdef WPSBUTTON_LEDFLASH_FLICK
                flash_wps_led = 1;
                #endif /* WPSBUTTON_LEDFLASH_FLICK */
                wlan_set_wps_on();
				needwps = 0;
			}
#endif
			
			if (needprint){
#ifdef DO_STATUS_PRINT
				SendStatusData(0);
				needboot = 0;
#endif//DO_STATUS_PRINT
			}
			needprint = 0;
			if (needboot) 
			{
			//Andy:20161229  Start HTTPD after system reset+++
				#if defined(O_ZOTCH) || defined(O_LINEUP)//Andy:20161201 add O_LINEUP to Start HTTPD after system reset.
					// George added this at build0019 in 716U2 on November 18, 2010.
					// Start HTTPD after system reboot.
					// Start IPPD, too? No. It's up to the administrator.
					EEPROM_Data.PrintServerMode2 |= PS_HTTP_MODE;
					
					if( WriteToEEPROM(&EEPROM_Data) != 0 )
						ErrLightOnForever(Status_Lite); // Write EEPROM Data error
					
					// Jesse modified this at build0019 in 716U2 on November 18, 2010.
					REBOOT();
					//ppause(10000);
					// Jesse modified this at build0019 in 716U2 on November 23, 2010.
					while(1)
						cyg_thread_yield();
				#else
					REBOOT();
				#endif	// defined(O_ZOTCH) || defined(O_LINEUP)
			//Andy:20161229  Start HTTPD after system reset---
			}
		}		
		
        sys_check_stack();
		cyg_thread_yield();
	}
}

void zot_idle_task_init(void)
{
    cyg_thread_create(ZOT_IDLE_TASK_PRI,
                  zot_idle_task,
                  0,
                  "ZOT-IDLE-TASK",
                  (void *) (ZOT_IDLE_Stack),
                  ZOT_IDLE_TASK_STACK_SIZE,
                  &ZOT_IDLE_TaskHdl,
                  &ZOT_IDLE_Task);

	cyg_thread_resume(ZOT_IDLE_TaskHdl);  	
}
