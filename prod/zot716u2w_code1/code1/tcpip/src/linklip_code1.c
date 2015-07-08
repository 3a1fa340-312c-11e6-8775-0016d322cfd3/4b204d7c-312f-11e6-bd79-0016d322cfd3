/* This module is responsible for Link Locak IP Task. It will dependent with
   ARP module and DHCP module.
   
   When we can not get IP from DHCP Server, the Link Local IP present.
   After ip query for a while (must over 2 Secons), if noboby conflict it(ARP 
   module will infromm if IP conflict), the IP can be set to sysyem. 
   Next sysyem reboot, we should use the this IP for initial query IP.
     												Ron Create on 12/13/04 */
     												
#include "lwip/opt.h"

#include "lwip/sys.h"

#include "lwip/memp.h"
#include "lwip/pbuf.h"

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "lwip/tcpip.h"

#include "lwip/linkip.h"

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

int LinkLocal_state;


// Ron Add 12/06/04
#ifdef LINKLOCAL_IP //Temp source code in this module, need move to "suitable" module and "directory"
//NETWORK Task create information definition
#define LINKLOCAL_IP_TASK_PRI    		 20	//ZOT716u2    
#define LINKLOCAL_IP_TASK_STACK_SIZE	 1024
static u8_t 			LINKLOCAL_IP_Stack[LINKLOCAL_IP_TASK_STACK_SIZE];
static cyg_thread       LINKLOCAL_IP_Task;
static cyg_handle_t     LINKLOCAL_IP_TaskHdl;

cyg_sem_t linklocal_sem;
cyg_sem_t linklocal_conflict;
cyg_sem_t rendezvous_sem;
cyg_sem_t DhcpWaitLinklocal_sem;

extern struct netif *Lanface;   
//ZOTIPS extern struct netif *WLanface;  

#define MAX_RENDEZ_ARP_QUERY 		4  //Max count can sure nobody conflict
#define MAX_RENDE_RETRY_QUERY		16   //Max retry count to give up arp_query

void SetIP(uint32 IP, struct netif *netif){
	
	netif->ip_addr.addr = IP;
}

void SetNetmask(uint32 NetMask, struct netif *netif){
	
	netif->netmask.addr = NetMask;
}

void SetGateway(uint32 GetWay, struct netif *netif){

	netif->gw.addr = GetWay;	
}

int is_linklocal_ip( uint32 addr){
	
	if( (NGET32(EEPROM_Data.BoxIPAddress) == addr))
		return 1;
	return 0;	
}

int is_linklocal( uint32 addr ){

	if((NGET32(EEPROM_Data.BoxIPAddress) & 0x0000FFFF)==0x0000FEA9 ) //net-if is link-locak 
		return 1;
	if( (addr & 0x0000FFFF) == 0x0000FEA9 ) //source is link-locak
		return 1;
	return 0;		
}

/* Get Link-Local IP function:
   param1: BoxIPAddr, is the system IP pointer
   return: Random link-local ip by int32 type
								Ron Add 12/07/04 */
static uint32 get_linklocal_ip(char *BoxIPAddr, int chang_ip_flag){
	uint32 tempIP;
	int randnum;
	
	tempIP = NGET32(BoxIPAddr);
	
	if ( ((NGET32(BoxIPAddr) & 0x0000FFFF) == 0x0000FEA9) && (chang_ip_flag == 0) )
		tempIP = NGET32(BoxIPAddr);
	else
	{
		tempIP = 0x0000FEA9 + (htonl(( (rand()*clock())% (254*256-1)+0x100))& 0xFFFF0000); 
	}							 
	return tempIP;
}

static int LinkLocal_IP_Query(uint32 IP, struct netif *netif){
	struct ip_addr ipaddr;
	
	ipaddr.addr = IP;
	return etharp_request(netif, &ipaddr);
}

static uint32 SetLinkLocalIP(struct netif *netif, int chang_ip_flag){	
	struct ip_addr ipaddr;
	uint32 LinkLocalIP;
	uint16 probe_count =0;
	
	LinkLocalIP = get_linklocal_ip(EEPROM_Data.BoxIPAddress, chang_ip_flag);
	
    NSET32(EEPROM_Data.BoxIPAddress,LinkLocalIP);
    NSET32(EEPROM_Data.SubNetMask, 0x0000FFFF );
    NSET32(EEPROM_Data.GetwayAddress,0);
	
/*	SetIP(LinkLocalIP, netif);
	SetNetmask(0x0000FFFF, netif);
	SetGateway(0, netif);
	
	ipaddr.addr = LinkLocalIP;
	updata_netif_ipaddr(netif);
*/	
	return LinkLocalIP;
}

static uint32 set_netif_ip(struct netif *netif)
{
	struct ip_addr ipaddr, netmask, gw;
	uint32 LinkLocalIP;
	
	LinkLocalIP = get_linklocal_ip(EEPROM_Data.BoxIPAddress, 0);
	
	ipaddr.addr =  LinkLocalIP;
	netmask.addr = 0x0000FFFF;
	gw.addr = 0;
	
	netif_set_addr( netif, &ipaddr, &netmask, &gw);
/*	
	SetIP(LinkLocalIP, netif);
	SetNetmask(0x0000FFFF, netif);
	SetGateway(0, netif);
*/	
	updata_netif_ipaddr(netif);
}

void Give_ip_by_myself(struct netif *netif){
	
	if(EEPROM_Data.RENVEnable == 1)
	{
		SetLinkLocalIP(netif ,0);
		cyg_semaphore_init(&DhcpWaitLinklocal_sem,0);
		cyg_semaphore_post(&linklocal_sem);	
		cyg_semaphore_wait(&DhcpWaitLinklocal_sem);	
	}
	else
		set_factory_ip();

}

int LinkLocal_get_current_state()
{
	return LinkLocal_state;
}

void LinkLocal_set_state(int newState)
{
	LinkLocal_state = newState;
}

void init_LinkLocal()
{
	
	if( (LinkLocal_get_current_state() == NO_USE) || (LinkLocal_get_current_state() == IDLE) )
		LinkLocal_state = PROBE;
	cyg_semaphore_init(&linklocal_conflict, 0);	
}

extern int Need_Rendezous_Reload ;

extern cyg_handle_t	rendezvous_TaskHdl;

void LinkLocal_ip_Task(cyg_addrword_t arg)
{
	int prob_conut, retry_count;
	uint32 LinkLocalIP;
	int get_conflict; //get arp reply to conflict our IP
		
	LinkLocal_set_state( NO_USE );
	
	while(1){
		cyg_semaphore_init(&linklocal_sem, 0);
		cyg_semaphore_wait(&linklocal_sem);	
		
		init_LinkLocal();
		
		if(rendezvous_TaskHdl != 0)
		{ 
			cyg_thread_suspend(rendezvous_TaskHdl);
		}
		
		prob_conut = 0;
		retry_count = 0;
		
		while(1){
			switch( LinkLocal_get_current_state() ){
				case PROBE:
//ZOTIPS					LinkLocalIP = SetLinkLocalIP(WLanface, 0 );
					LinkLocalIP = SetLinkLocalIP(Lanface, 0 );	//ZOTIPS
					break;	
				case RETRY:
//ZOTIPS					LinkLocalIP = SetLinkLocalIP(WLanface, 1 );
					LinkLocalIP = SetLinkLocalIP(Lanface, 1 );	//ZOTIPS
					break;
				case ANNOUNCE:
//ZOTIPS					LinkLocalIP = SetLinkLocalIP(WLanface, 0 );
//ZOTIPS					set_netif_ip(WLanface);
					LinkLocalIP = SetLinkLocalIP(Lanface, 0 );	//ZOTIPS
					set_netif_ip(Lanface);	//ZOTIPS
					break;
			}
			
//ZOTIPS			LinkLocal_IP_Query(LinkLocalIP, WLanface);
			LinkLocal_IP_Query(LinkLocalIP, Lanface);	//ZOTIPS
			get_conflict = cyg_semaphore_timed_wait(&linklocal_conflict, cyg_current_time() +(900/MSPTICK));			
			
			switch( LinkLocal_get_current_state() ){
				case PROBE:
						if( get_conflict )
						{
							LinkLocal_set_state( RETRY );
							ppause(1000);
						}
						else	
							prob_conut++;
						if( prob_conut == 3)
						{
							ppause(900);
							LinkLocal_set_state( ANNOUNCE );
						}
					break;	
				case RETRY:
						if( get_conflict )
						{	
							retry_count++;
						}
						else{
							LinkLocal_set_state( PROBE );
							prob_conut = 1;
						}
						ppause(1000);
						if( retry_count == 15)
						{
							ppause(900);
							LinkLocal_set_state( ANNOUNCE );		
						}
					break;
				case ANNOUNCE:
						if(get_conflict)
							ppause(1000);
//ZOTIPS						LinkLocal_IP_Query(LinkLocalIP, WLanface);
						LinkLocal_IP_Query(LinkLocalIP, Lanface);	//ZOTIPS
						WriteToEEPROM(&EEPROM_Data); 
						LinkLocal_set_state( IDLE );
					break;
			}
			
			if( LinkLocal_get_current_state() == IDLE)
				break;
		}	

		if( rendezvous_TaskHdl == 0)
		cyg_semaphore_post( &rendezvous_sem);
		else
		{
			cyg_thread_resume(rendezvous_TaskHdl);
		Need_Rendezous_Reload = 1;
		}
	
		cyg_semaphore_post(&DhcpWaitLinklocal_sem);
		
	}	
}

void Link_local_ip_init(void){

	//Create Link Local Task	
    cyg_thread_create(LINKLOCAL_IP_TASK_PRI,
                  LinkLocal_ip_Task,
                  0,
                  "linklocal_ip_task",
                  (void *) (LINKLOCAL_IP_Stack),
                  LINKLOCAL_IP_TASK_STACK_SIZE,
                  &LINKLOCAL_IP_TaskHdl,
                  &LINKLOCAL_IP_Task);
	
	//Start NETWORK Thread
	cyg_thread_resume(LINKLOCAL_IP_TaskHdl);
}
#endif //#ifdef LINKLOCAL_IP
