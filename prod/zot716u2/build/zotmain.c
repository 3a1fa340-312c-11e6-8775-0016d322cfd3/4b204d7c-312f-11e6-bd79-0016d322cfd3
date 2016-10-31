/*===========================================================================
//        zotmain.c
//	Copy Righti(2006) ZOT.
//=========================================================================*/
#include <stdio.h>
#include <config.h>
#include <cyg/infra/diag.h>
#include "lwip/sys.h"

//#include <cyg/kernel/kapi.h>	// Kernel API.
//#include <cyg/infra/diag.h>		// For diagnostic printing.

#include "pstarget.h"
#include "psglobal.h"
#ifdef STAR_MAC
#include "star_intc.h"
#include "star_misc.h"
#endif /* def STAR_MAC */



int config_net_mbuf_usage = 200;
int config_net_cluster_usage = 2400;

int opmode = 1; /*1:Gateway, 2:Apcli as WAN port, 3:Repeater Mode */

#if 1
#define STACK_SIZE 0x2000
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;
#endif /* 1 */

void cyg_net_init(void);
int flsh_init (void);
void WLan_get_EEPData (void);
int ralink_gpio_init(void);

extern void usb_drv_init();

// uint8 usbprn_read_status( int nPort )
// {
// 
// }

void zotmain( void )
//void zotmain(cyg_addrword_t p)
{
    cyg_net_init();
    flsh_init();
    ralink_gpio_init();

    read_version();
    EEPROMInit();
    WLan_get_EEPData(); 

    zot_network_init();
    
    #ifdef STAR_MAC
	LanPktInit();
    star_nic_lan_init();
	LanPktStart();
	#endif /* def STAR_MAC */

    //
    //Print Server module
    //
#if defined(USE_PS_LIBS) 
	IPXInitialize();
	NETBEUInit();
	Spooler_init();
#endif /* USE_PS_LIB */

	ps_init();
    LED_Init();

#if defined(USE_PS_LIBS)
    usb_drv_init();
#endif /* USE_PS_LIBS */
	zot_idle_task_init();

    diag_printf("use zot function\n");
}

void dbg_task(cyg_addrword_t p)
{
    lwip_init();

    diag_printf("dbg_task test\n");
}

externC void
cyg_user_start( void )
{
    cyg_thread_create(10,                // Priority - just a number
                      zotmain,           // entry
                      0,                 // entry parameter
                      "main init",       // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
}


void telnet_printf(char* message, int messagelen) {}
//void iplinit(void){}
//void udp_init(){}

