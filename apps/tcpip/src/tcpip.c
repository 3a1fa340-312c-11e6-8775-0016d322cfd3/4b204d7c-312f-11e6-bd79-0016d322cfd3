/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"

#include "lwip/sys.h"

#include "lwip/memp.h"
#include "lwip/pbuf.h"

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "lwip/tcpip.h"

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "netif/etharp.h"	//ZOT716u2

extern int diag_flag;

int Network_TCPIP_ON;

void arp_timer(void *arg);//ZOT716u2

static void (* tcpip_init_done)(void *arg) = NULL;
static void *tcpip_init_done_arg;
static sys_mbox_t tcp_mbox;
#if LWIP_TCP
static int tcpip_tcp_timer_active = 0;

#ifdef LINKLOCAL_IP
extern cyg_sem_t rendezvous_sem;
extern void Give_ip_by_myself(struct netif *netif);
extern void Link_local_ip_init(void);
//cyg_sem_t linklocal_sem;
//cyg_sem_t linklocal_conflict;
#endif

static void
tcpip_tcp_timer(void *arg)
{
  (void)arg;

  /* call TCP timer handler */
  tcp_tmr();
  /* timer still needed? */
  if (tcp_active_pcbs || tcp_tw_pcbs) {
    /* restart timer */
    sys_timeout(TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
  } else {
    /* disable timer */
    tcpip_tcp_timer_active = 0;
  }
}

void
tcp_timer_needed(void)
{
  /* timer is off but needed again? */
  if (!tcpip_tcp_timer_active && (tcp_active_pcbs || tcp_tw_pcbs)) {
    /* enable and start timer */
    tcpip_tcp_timer_active = 1;
    sys_timeout(TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
  }
}
#endif /* LWIP_TCP */

 extern int Lanrecvcnt;
static void
tcpip_thread(void *arg)
{
  struct tcpip_msg *msg;

  (void)arg;

  ip_init();
#if LWIP_UDP  
  udp_init();
#endif
#if LWIP_TCP
  tcp_init();
#endif
  if (tcpip_init_done != NULL) {
    tcpip_init_done(tcpip_init_done_arg);
  }

	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);	//ZOT716u2

	Network_TCPIP_ON = 1;

  while (1) {                          /* MAIN Loop */
    sys_mbox_fetch(tcp_mbox, (void *)&msg);
    switch (msg->type) {
    case TCPIP_MSG_API:
      LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: API message %p\n", (void *)msg));
      api_msg_input(msg->msg.apimsg);
      break;
    case TCPIP_MSG_INPUT:
cli();
		Lanrecvcnt --;			
sti();
      LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: IP packet %p\n", (void *)msg));
      ip_input(msg->msg.inp.p, msg->msg.inp.netif);
      break;
    case TCPIP_MSG_CALLBACK:
      LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: CALLBACK %p\n", (void *)msg));
      msg->msg.cb.f(msg->msg.cb.ctx);
      break;
    default:
      break;
    }
    memp_free(MEMP_TCPIP_MSG, msg);
  }
}

err_t
tcpip_input(struct pbuf *p, struct netif *inp)
{
  struct tcpip_msg *msg;
  
  msg = memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    pbuf_free(p);    
    return ERR_MEM;  
  }
  
  msg->type = TCPIP_MSG_INPUT;
  msg->msg.inp.p = p;
  msg->msg.inp.netif = inp;
  sys_mbox_post(tcp_mbox, msg);
  return ERR_OK;
}

err_t
tcpip_callback(void (*f)(void *ctx), void *ctx)
{
  struct tcpip_msg *msg;
  
  msg = memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    return ERR_MEM;  
  }
  
  msg->type = TCPIP_MSG_CALLBACK;
  msg->msg.cb.f = f;
  msg->msg.cb.ctx = ctx;
  sys_mbox_post(tcp_mbox, msg);
  return ERR_OK;
}

void
tcpip_apimsg(struct api_msg *apimsg)
{
  struct tcpip_msg *msg;
  msg = memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    memp_free(MEMP_API_MSG, apimsg);
    return;
  }
  msg->type = TCPIP_MSG_API;
  msg->msg.apimsg = apimsg;
  sys_mbox_post(tcp_mbox, msg);
}

void
tcpip_init(void (* initfunc)(void *), void *arg)
{
  tcpip_init_done = initfunc;
  tcpip_init_done_arg = arg;
  tcp_mbox = sys_mbox_new();
  sys_thread_new(tcpip_thread, NULL, TCPIP_THREAD_PRIO);
}

//NETWORK Task create information definition
#define NETWORK_TASK_PRI         1
#define NETWORK_TASK_STACK_SIZE	 4096
static u8_t 			NETWORK_Stack[NETWORK_TASK_STACK_SIZE];
static cyg_thread       NETWORK_Task;
static cyg_handle_t     NETWORK_TaskHdl;

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

struct netif *Lanface;
struct netif *WLanface = NULL;
struct netif *ULanface = NULL;

cyg_sem_t dhcp_sem;

typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
}MIB_DHCP;
//ZOTIPS extern MIB_DHCP                   *mib_DHCP_p;
MIB_DHCP                   *mib_DHCP_p;	//ZOTIPS

void erase_netif_ipaddr()
{	
	struct ip_addr ipaddr, netmask, gw;
	
	ipaddr.addr =  0;
	netmask.addr = 0;
	gw.addr = 0;
	
	if(WLanface != NULL)
 	netif_set_addr( WLanface, &ipaddr, &netmask, &gw);
	netif_set_addr( Lanface, &ipaddr, &netmask, &gw);	
//	SetIP(0x0, WLanface);
//	SetNetmask(0x0, WLanface);
//	SetGateway(0x0, WLanface);
	
//	SetIP(0x0, Lanface);
//	SetNetmask(0x0, Lanface);
//	SetGateway(0x0, Lanface);
	
	mib_DHCP_p->IPAddr = DWordSwap(Lanface->ip_addr.addr);
	mib_DHCP_p->SubnetMask = DWordSwap(Lanface->netmask.addr);
	mib_DHCP_p->GwyAddr = DWordSwap(Lanface->gw.addr);
}

void updata_netif_ipaddr( struct netif *netif )
{
#if 1	//ZOTIPS

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
#endif //ZOTIPS
	
	mib_DHCP_p->IPAddr = DWordSwap(Lanface->ip_addr.addr);
	mib_DHCP_p->SubnetMask = DWordSwap(Lanface->netmask.addr);
	mib_DHCP_p->GwyAddr = DWordSwap(Lanface->gw.addr);
		
#ifdef MTK7601
    printk("ip_addr.addr = %X\n", netif->ip_addr.addr);
#endif

	NSET32(EEPROM_Data.BoxIPAddress, netif->ip_addr.addr);
	NSET32(EEPROM_Data.SubNetMask, netif->netmask.addr);
	NSET32(EEPROM_Data.GetwayAddress, netif->gw.addr);
}

void set_factory_ip()
{
	struct ip_addr ipaddr, netmask, gw;
	
	
	memcpy(EEPROM_Data.BoxIPAddress, DEFAULT_Data.BoxIPAddress , 4);
	memcpy(EEPROM_Data.SubNetMask, DEFAULT_Data.SubNetMask , 4);
	memcpy(EEPROM_Data.GetwayAddress, DEFAULT_Data.GetwayAddress , 4);
	
	ipaddr.addr =  NGET32( EEPROM_Data.BoxIPAddress );
	netmask.addr = NGET32( EEPROM_Data.SubNetMask );
	gw.addr = NGET32( EEPROM_Data.GetwayAddress );

#if 1	//ZOTIPS		
	if(WLanface != NULL){
	netif_set_ipaddr(WLanface, &ipaddr);
	netif_set_netmask(WLanface, &netmask);
	netif_set_gw(WLanface, &gw);
	}
#endif //ZOTIPS
	
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

#if 1	//ZOTIPS		
	if(WLanface != NULL){
	netif_set_ipaddr(WLanface, &ipaddr);
	netif_set_netmask(WLanface, &netmask);
	netif_set_gw(WLanface, &gw);
	}
#endif //ZOTIPS
	
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
/*
void dhcp_init(cyg_addrword_t arg)
{
	int dhcp_count = 0;
	uint no_timeout = 1;
/*	
	ppause(10000);
	ppause(5000); //Ron Add, to temporarily prevent Get DHCP IP fails. Need to improve in the future. 2/3/05
	dhcp_start(WLanface);
//	ppause(5000);
	ppause(3000);


	
	dhcp_start(WLanface);
	ppause(2000);
	
	dhcp_serch(13);

	while(1)
	{
		cyg_semaphore_init( &dhcp_sem, 0);
		
#ifdef LINKLOCAL_IP
	// Ron Add 11/28/04 
		if( no_timeout == 1 )
		{
			if ( mib_DHCP_p->IPAddr == 0x0 )
		Give_ip_by_myself(WLanface);
	else
	{
		if(EEPROM_Data.RENVEnable == 1)
			cyg_semaphore_post( &rendezvous_sem);
	}
		}
		else
		{
			if( (mib_DHCP_p->IPAddr != 0x0 ) && (EEPROM_Data.RENVEnable == 1))
				Need_Rendezous_Reload = 1;
		}
#else
	if( mib_DHCP_p->IPAddr == 0x0 )
		set_factory_ip();
#endif	
				
		if( (mib_DHCP_p->IPAddr == 0x0 ) || ((NGET32(EEPROM_Data.BoxIPAddress) & 0x0000FFFF)==0x0000FEA9) )
			no_timeout = cyg_semaphore_timed_wait( &dhcp_sem, cyg_current_time() + 90000);
		else
			cyg_semaphore_wait( &dhcp_sem);
		
		erase_netif_ipaddr();
		ppause(500);
		dhcp_serch(3);
		
	}
}
*/

extern cyg_handle_t	rendezvous_TaskHdl;

void dhcp_init(cyg_addrword_t arg)
{
	uint no_timeout = 1;
	int need_LinkLocal = 1;
	
	while( Network_TCPIP_ON == 0 )
		ppause(100);
	
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

void zot_network_task(cyg_addrword_t arg)
{
	sys_sem_t sem;

	sys_init();
	mem_init();
	memp_init();
	pbuf_init();
	
	sem = sys_sem_new(0);
	tcpip_init(tcpip_init_done, &sem);

#ifdef PRINT_DIAGNOSTIC
//	if( !diag_flag )
	if( 1 )
#endif
	{
		if(EEPROM_Data.PrintServerMode & PS_DHCP_ON)
		{

#ifdef LINKLOCAL_IP	
            #if defined(N716U2W) || defined(N716U2)
			if(EEPROM_Data.RENVEnable == 1)
            #endif
				Link_local_ip_init();	
#endif
			
			mib_DHCP_p->IPAddr = 0;
			mib_DHCP_p->SubnetMask = 0;
			mib_DHCP_p->GwyAddr = 0;
/*				
			memset( EEPROM_Data.BoxIPAddress, 0, 4);
			memset( EEPROM_Data.SubNetMask, 0, 4);
			memset( EEPROM_Data.GetwayAddress, 0, 4);
*/			
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

		}
#ifdef RENDEZVOUS
		else
		{
			ppause(3000);
			cyg_semaphore_post( &rendezvous_sem);
		}
#endif		
		
		
	}
	
	sys_sem_wait(sem);
	sys_sem_free(sem);
}
#if 0	//ZOT716u2
uint32 TCP_HOLE_FLAG = 0;
cyg_handle_t CloseTcpHole_SysClk;
cyg_handle_t CloseTcpHole_Counter;
cyg_handle_t CloseTcpHole_Alarm;  
cyg_alarm    CloseTcpHole_timerAlarm;

void CloseTcpHole (cyg_handle_t handle, cyg_addrword_t ptr)
{
		TCP_HOLE_FLAG = 0;
		 
		cyg_alarm_delete(CloseTcpHole_Alarm);				//eCos
		cyg_clock_delete(CloseTcpHole_SysClk);			    //eCos
		cyg_counter_delete(CloseTcpHole_Counter);			//eCos	
		
}

void tcp_hole()
{
	int interval = 30000;
	
	TCP_HOLE_FLAG = 1;
  	
  	/* Attach the timer to the CloseTcpHole clock */
    CloseTcpHole_SysClk = cyg_real_time_clock();

    cyg_clock_to_counter(CloseTcpHole_SysClk, &CloseTcpHole_Counter);

    cyg_alarm_create(CloseTcpHole_Counter, (cyg_alarm_t *)CloseTcpHole,
                     0,
                     &CloseTcpHole_Alarm, &CloseTcpHole_timerAlarm);

    /* This creates a periodic timer */
    cyg_alarm_initialize(CloseTcpHole_Alarm, cyg_current_time() + interval, 0); //only trigger once after 5 minutes   

}
#endif	//ZOT716u2
void zot_network_init()
{	
	//Create NETWORK Thread
    cyg_thread_create(NETWORK_TASK_PRI,
                  zot_network_task,
                  0,
                  "zot_network",
                  (void *) (NETWORK_Stack),
                  NETWORK_TASK_STACK_SIZE,
                  &NETWORK_TaskHdl,
                  &NETWORK_Task);
	
	//Start NETWORK Thread
	cyg_thread_resume(NETWORK_TaskHdl);
	
	//Open TCP/IP layer hole
//ZOT716u2	tcp_hole(); 

}
