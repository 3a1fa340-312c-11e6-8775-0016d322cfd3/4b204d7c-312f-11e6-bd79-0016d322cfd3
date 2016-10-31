//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2004 Free Software Foundation, Inc.                        
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================

/*
 * init.c - misc lwip ecos glue functions 
 */
#include <pkgconf/system.h>
//#include <pkgconf/net_lwip.h>
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#if LWIP_SLIP
#include "netif/slipif.h"
#endif

#if PPP_SUPPORT
#include "netif/ppp/ppp.h"
#endif

#include "netif/loopif.h"
#include <cyg/hal/hal_if.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_LWIP_ETH
#include "netif/etharp.h"

#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>
#include "psglobal.h"
#include "pstarget.h"
#include "psdefine.h"
#include "eeprom.h"


extern void ecosglue_init(void);
#endif


// =============================================================================================================
// copy from zot_tcpip 
// =============================================================================================================

//NETWORK Task create information definition
//#define NETWORK_TASK_PRI         5 
//#define NETWORK_TASK_STACK_SIZE	 4096
//static char 			NETWORK_Stack[NETWORK_TASK_STACK_SIZE];
//static cyg_thread       NETWORK_Task;
//static cyg_handle_t     NETWORK_TaskHdl;

//DHCP Task create information definition
#if defined(N716U2W) || defined(N716U2)
#define DHCP_TASK_PRI         8
#endif
#if defined(NDWP2020)
#define DHCP_TASK_PRI         6
#endif
#define DHCP_TASK_STACK_SIZE	 4096
static u8_t 			DHCP_Stack[DHCP_TASK_STACK_SIZE];
static cyg_thread       DHCP_Task;
static cyg_handle_t     DHCP_TaskHdl;

struct netif *Lanface = NULL;
//#if defined(WIRELESS_CARD)
struct netif *WLanface = NULL;
//#endif
struct netif *ULanface = NULL;

cyg_sem_t dhcp_sem;


typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
} MIB_DHCP;

//ZOTIPS extern MIB_DHCP                   *mib_DHCP_p;
extern MIB_DHCP                   *mib_DHCP_p;	//ZOTIPS

void erase_netif_ipaddr()
{	
	struct ip_addr ipaddr, netmask, gw;
	
	ipaddr.addr =  0;
	netmask.addr = 0;
	gw.addr = 0;
	
    #if defined(WIRELESS_CARD)
	if(WLanface != NULL)
 	    netif_set_addr( WLanface, &ipaddr, &netmask, &gw);
    #endif
	netif_set_addr( Lanface, &ipaddr, &netmask, &gw);	
	
	mib_DHCP_p->IPAddr = DWordSwap(Lanface->ip_addr.addr);
	mib_DHCP_p->SubnetMask = DWordSwap(Lanface->netmask.addr);
	mib_DHCP_p->GwyAddr = DWordSwap(Lanface->gw.addr);
}

void update_netif_ipaddr( struct netif *netif )
{
#if defined(WIRELESS_CARD) 
	if (netif == WLanface){ //Ron modified on 12/07/04
	    Lanface->ip_addr.addr = netif->ip_addr.addr;
	    Lanface->netmask.addr = netif->netmask.addr;
	    Lanface->gw.addr = netif->gw.addr;
	}else if (netif == Lanface){
		
		if(WLanface != NULL){
		WLanface->ip_addr.addr = netif->ip_addr.addr;
		WLanface->netmask.addr = netif->netmask.addr;
		WLanface->gw.addr = netif->gw.addr;
		}
		
	}else{
//		LWIP_ASSERT("updata_netif_ipaddr :network interface does not exist");
	}
#else
    Lanface->ip_addr.addr = netif->ip_addr.addr;
    Lanface->netmask.addr = netif->netmask.addr;
    Lanface->gw.addr = netif->gw.addr;
#endif /* !WIRELESS_CARD */
	
	mib_DHCP_p->IPAddr = DWordSwap(Lanface->ip_addr.addr);
	mib_DHCP_p->SubnetMask = DWordSwap(Lanface->netmask.addr);
	mib_DHCP_p->GwyAddr = DWordSwap(Lanface->gw.addr);
		
    /*
    printk("ip_addr.addr = %X\n", netif->ip_addr.addr);
    */

	NSET32(EEPROM_Data.BoxIPAddress, netif->ip_addr.addr);
	NSET32(EEPROM_Data.SubNetMask, netif->netmask.addr);
	NSET32(EEPROM_Data.GetwayAddress, netif->gw.addr);
}

void set_factory_ip()
{
	struct ip_addr ipaddr, netmask, gw;
	
    diag_printf("%s\n", __FUNCTION__);

	memcpy(EEPROM_Data.BoxIPAddress, DEFAULT_Data.BoxIPAddress , 4);
	memcpy(EEPROM_Data.SubNetMask, DEFAULT_Data.SubNetMask , 4);
	memcpy(EEPROM_Data.GetwayAddress, DEFAULT_Data.GetwayAddress , 4);
	
	ipaddr.addr =  NGET32( EEPROM_Data.BoxIPAddress );
	netmask.addr = NGET32( EEPROM_Data.SubNetMask );
	gw.addr = NGET32( EEPROM_Data.GetwayAddress );

#if defined(WIRELESS_CARD) 
	if(WLanface != NULL){
	netif_set_ipaddr(WLanface, &ipaddr);
	netif_set_netmask(WLanface, &netmask);
	netif_set_gw(WLanface, &gw);
	}
#endif 
	
	netif_set_ipaddr(Lanface, &ipaddr);
	netif_set_netmask(Lanface, &netmask);
	netif_set_gw(Lanface, &gw);
	
	mib_DHCP_p->IPAddr = DWordSwap(Lanface->ip_addr.addr);
	mib_DHCP_p->SubnetMask = DWordSwap(Lanface->netmask.addr);
	mib_DHCP_p->GwyAddr = DWordSwap(Lanface->gw.addr);

}

void UseEEPROMIP(){
	struct ip_addr ipaddr, netmask, gw;

	ipaddr.addr =  NGET32( EEPROM_Data.BoxIPAddress );
	netmask.addr = NGET32( EEPROM_Data.SubNetMask );
	gw.addr = NGET32( EEPROM_Data.GetwayAddress );

#if defined(WIRELESS_CARD)
	if(WLanface != NULL){
	    netif_set_ipaddr(WLanface, &ipaddr);
	    netif_set_netmask(WLanface, &netmask);
	    netif_set_gw(WLanface, &gw);
	}
#endif
	
	netif_set_ipaddr(Lanface, &ipaddr);
	netif_set_netmask(Lanface, &netmask);
	netif_set_gw(Lanface, &gw);
	
	mib_DHCP_p->IPAddr = DWordSwap(Lanface->ip_addr.addr);
	mib_DHCP_p->SubnetMask = DWordSwap(Lanface->netmask.addr);
	mib_DHCP_p->GwyAddr = DWordSwap(Lanface->gw.addr);
	
}

void dhcp_serch( int dhcp_count )
{
	while(1)
	{
		if(	dhcp_count == 0)
			break;
		
		if( mib_DHCP_p->IPAddr == 0x0 )
		{
//ZOTIPS			dhcp_discover(WLanface);
			dhcp_discover(Lanface);	//ZOTIPS
			ppause(2000);
			dhcp_count--;
		}
		else
			break;
	}
}


int Need_Rendezous_Reload = 0;

extern cyg_handle_t	rendezvous_TaskHdl;
extern cyg_sem_t rendezvous_sem;

void dhcp_init(cyg_addrword_t arg)
{
	uint no_timeout = 1;
	int need_LinkLocal = 1;
	
//ZOTIPS	dhcp_start(WLanface);
	dhcp_start(Lanface);	//ZOTIPS
	ppause(2000);
	
	if ( !strcmp(EEPROM_Data.WLESSID, "") || !strcmp(EEPROM_Data.WLESSID, "< ANY >")) 
	    dhcp_serch(45);		// original:25.	Jesse modified this at build0006 of 716U2W on April 28, 2011.
	else
	    dhcp_serch(45);		// original:13.	Jesse modified this at build0006 of 716U2W on April 28, 2011.
	
	while(1)
	{
		cyg_semaphore_init( &dhcp_sem, 0);
		no_timeout = 1;
			
#ifdef LINKLOCAL_IP
	// Ron Add 11/28/04 
		
		if ( (mib_DHCP_p->IPAddr == 0x0) && need_LinkLocal )
//ZOTIPS			Give_ip_by_myself(WLanface);
			Give_ip_by_myself(Lanface);	//ZOTIPS
		else
		{
			if(EEPROM_Data.RENVEnable == 1)
			{
				if( rendezvous_TaskHdl == 0)
					cyg_semaphore_post( &rendezvous_sem);
				else
					Need_Rendezous_Reload = 1;	
			}
		}
		
#else
		if( mib_DHCP_p->IPAddr == 0x0 )
			set_factory_ip();
#endif	
	
		if( (mib_DHCP_p->IPAddr == 0x0 ) || ((NGET32(EEPROM_Data.BoxIPAddress) & 0x0000FFFF)==0x0000FEA9) )
			no_timeout = cyg_semaphore_timed_wait( &dhcp_sem, cyg_current_time() + 90000);
		else
			cyg_semaphore_wait( &dhcp_sem);
			
		
		if( no_timeout == 0 )
			need_LinkLocal = 0;
		else
			need_LinkLocal = 1;
		
		erase_netif_ipaddr();
		delete_dhcp_time();
		ppause(500);	
		dhcp_serch(10);		// original:3.	Jesse modified this at build0006 of 716U2W on April 28, 2011.
				
	}
	
}

// =============================================================================================================
// 
// =============================================================================================================

#ifdef CYGPKG_LWIP_ETH
static void
arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}
#endif

#if LWIP_DHCP
static void lwip_dhcp_fine_tmr(void *arg)
{
    dhcp_fine_tmr();
    sys_timeout(500, (sys_timeout_handler) lwip_dhcp_fine_tmr, NULL);
}

static void lwip_dhcp_coarse_tmr(void *arg)
{
    dhcp_coarse_tmr();
    sys_timeout(60000, (sys_timeout_handler) lwip_dhcp_coarse_tmr, NULL);
}
#endif

//
// This function is called when tcpip thread finished initialisation.
// We start several timers here - these timers are all handled in the
// tcpip thread. That means that also the DHCP stuff is handled in the
// TCPIP thread. If this causes any trouble than it may be necessaray to
// use an own DHCP thread insted.
//

#if 1
void tcpip_init_done_1(void * arg)
{
    
#ifdef CYGPKG_LWIP_ETH
    sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
#endif
#ifdef CYGOPT_LWIP_DHCP_MANAGEMENT
	sys_timeout(500, (sys_timeout_handler) lwip_dhcp_fine_tmr, NULL);
	sys_timeout(60000, (sys_timeout_handler) lwip_dhcp_coarse_tmr, NULL);
#endif
   
	sys_sem_t *sem = arg;
	sys_sem_signal(*sem);
}
#endif

//extern void tcpip_init_done(void * arg);
#if LWIP_HAVE_LOOPIF
extern struct netif ecos_loopif;
#endif

extern void LanPktInit(void);
extern void LanPktStart(void);
extern err_t pk_attach( struct netif *netif);

/*
 * ecosglue_init --> init_hw_drivers --> if_ra305x_init --> eth_drv_init --> [ecosif_init]
 * To use this function to
 * .initialize netif
 * .setup output, link_output function pointer
 * .set netif ip address
 */
err_t ecosif_init(struct netif *netif)
{
    struct ip_addr ipaddr, netmask, gw;	

    if (netif == Lanface) {
        LanPktInit();
        LanPktStart();
    }

    if (netif == WLanface) {
        
		if(EEPROM_Data.PrintServerMode & PS_DHCP_ON)
		{
			IP4_ADDR( &ipaddr,0,0,0,0);
			IP4_ADDR( &netmask, 0,0,0,0);
			IP4_ADDR( &gw, 0,0,0,0);
		}
		else
		{
			ipaddr.addr = NGET32(EEPROM_Data.BoxIPAddress);
			netmask.addr = NGET32(EEPROM_Data.SubNetMask);
			gw.addr = NGET32(EEPROM_Data.GetwayAddress);
			// IP_ADDR(&gw, CYGDAT_LWIP_SERV_ADDR);
			// IP_ADDR(&ipaddr, CYGDAT_LWIP_MY_ADDR);
			// IP_ADDR(&netmask, CYGDAT_LWIP_NETMASK);

		}

        netif_add (WLanface, &ipaddr, &netmask, &gw, WLanface->state, pk_attach, tcpip_input);
    }
}

/*
 * to replase LanRecv function
 * rxint -> eth_drv_recv -> if_ra305x_recv -> ecsoif_input -> tcpip stack
 */
//void ecosif_input(struct netif *netif, struct pbuf *p)
//{
//}

err_t ecosif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
  // resolve hardware address, then send (or queue) packet
  return etharp_output(netif, ipaddr, p);
}

/*
 * low_level_ouput -> eth_drv_send ->if_ra305x_send
 */
extern err_t low_level_output(struct netif *netif, struct pbuf *p);


/*
 * Called by the eCos application at startup
 * wraps various init calls
 */
int
zot_network_init(void)
{
    diag_printf("use zot_network_init\n");
#if LWIP_HAVE_LOOPIF
	struct ip_addr ipaddr, netmask, gw;
#endif

	static int inited = 0;

	sys_sem_t sem;
	if (inited)
		return 1;
	inited++;
	
	sys_init();	/* eCos specific initialization */
	mem_init();	/* heap based memory allocator */
	memp_init();	/* pool based memory allocator */
	pbuf_init();	/* packet buffer allocator */
	netif_init();	/* netif layer */
	
	/* Start the stack.It will spawn a new dedicated thread */
	sem = sys_sem_new(0);
	tcpip_init(tcpip_init_done_1,&sem);
	sys_sem_wait(sem);
	sys_sem_free(sem);

#if 0 //LWIP_HAVE_LOOPIF
	IP4_ADDR(&gw, 127,0,0,1);
	IP4_ADDR(&ipaddr, 127,0,0,1);
	IP4_ADDR(&netmask, 255,0,0,0);
  
	netif_add(&ecos_loopif, &ipaddr, &netmask, &gw, NULL, loopif_init,
	    tcpip_input);
#endif

    //
    // initialize low level network dirver
    // (LAN device driver)
    //
    ecosglue_init();

    #if 1
	if(EEPROM_Data.PrintServerMode & PS_DHCP_ON)
	{

#ifdef LINKLOCAL_IP	
        #if defined(N716U2W) || defined(N716U2)
        if(EEPROM_Data.RENVEnable == 1)
        #endif
			Link_local_ip_init();	
#endif
			
        /*
		mib_DHCP_p->IPAddr = 0;
		mib_DHCP_p->SubnetMask = 0;
		mib_DHCP_p->GwyAddr = 0;
        */

        dhcp_start(Lanface);

/*				
			memset( EEPROM_Data.BoxIPAddress, 0, 4);
			memset( EEPROM_Data.SubNetMask, 0, 4);
			memset( EEPROM_Data.GetwayAddress, 0, 4);
*/			
#if 0
		//Create DHCP Thread
	    cyg_thread_create(DHCP_TASK_PRI,
	                  dhcp_init,
	                  0,
	                  "dhcp_init",
	                  (void *) (DHCP_Stack),
	                  DHCP_TASK_STACK_SIZE,
	                  &DHCP_TaskHdl,
	                  &DHCP_Task);
			
		//Start DHCP Thread
		cyg_thread_resume(DHCP_TaskHdl);
#endif
	}
#ifdef RENDEZVOUS
	else
	{
		ppause(3000);
		cyg_semaphore_post( &rendezvous_sem);
	}
#endif	
    #endif

	return 0;
}


