#include <cyg/hal/hal_arch.h>	//ZOT
#include <cyg/kernel/kapi.h>	//ZOT
#include "network.h"
#include "arch/cc.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/ip_addr.h"
#include "netif/iface.h"
#include "netif/etharp.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "ipx.h"
#include "netbeui.h"


#ifdef WEBADMIN	 //6/26/2000
uint32 IPXENIIRecv;
uint32 IPX8023Recv;
uint32 IPX8022Recv;
uint32 IPXSNAPRecv;
#endif //WEBADMIN

cyg_handle_t * Process_OthPkt_handle;
cyg_mbox * Process_OthPkt_mbox;

//BridgeTask Packet Task
#define BridgeTask_TASK_STACK_SIZE		2048
#define BridgeTask_TASK_PRI     		20
static 					cyg_thread BridgeTask_Pckt_Task;
static 					cyg_handle_t BridgeTask_Pckt_TaskHdl;
static UINT8 			BridgeTask_Stack[BridgeTask_TASK_STACK_SIZE];

//Process Other Packet Task
#define sysinfo_TASK_NAME  "Process_Other_Pckt_Task"
#define sysinfo_TASK_STACK_SIZE  1024 //ZOT716u2 1024
#define sysinfo_TASK_PRI         20	//ZOT716u2
static cyg_thread       Process_Other_Pckt_Task;
static cyg_handle_t     Process_Other_Pckt_TaskHdl;
static UINT8            Process_Other_Pckt_Stack[sysinfo_TASK_STACK_SIZE];

int Lanrecvcnt = 0;
int OtherPcktrecvcnt = 0;
void OtherPckt_input(char *data, int length);
void BridgeTask(cyg_addrword_t Data);
void ProcessOtherPcktTask(cyg_addrword_t Data);

WORD MyIntNO;
extern WORD IPXInput(WORD InputFrameType, BYTE *SrcAddress,IPXHeader *p,int16 DataLen);
extern void aarp_input2ambf(void * Data,int16 DataLen);
extern void ddp_input2ambf(char *data,unsigned int len);


extern int STAR_MAC_Plugout;
extern struct netif *Lanface;

//ZOT
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

//ZOT
//ZOT716u2 static void
void arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}


void cpyfrompbuf(char *buffer, struct pbuf *p, int length)
{
	struct pbuf *q;
	char *cur_prt;

	cur_prt = buffer;
	for(q = p; q != NULL; q = q->next) {
		/* Read enough bytes to fill this pbuf in the chain. The
		* available data in the pbuf is given by the q->len
		* variable. */
		memcpy( cur_prt,q->payload, q->len);
		cur_prt +=q->len;
	}
}

//ZOT
//for TCP/IP only
err_t SendPacket(struct netif *netif, struct pbuf *p)
{
#if 0
	struct pbuf *q;
	char *ptr, *cur_prt;
#endif
	struct ether ep;
	
	if(p->tot_len > 1514)
		return -1;

	memcpy( &ep, p->payload, sizeof(struct ether) );


	if(STAR_MAC_Plugout == 0)
	{
#if 0
		ptr = malloc(p->tot_len);
		if(ptr == NULL)
			return -1;
		cur_prt = ptr;
		for(q = p; q != NULL; q = q->next) {
			/* Read enough bytes to fill this pbuf in the chain. The
			* available data in the pbuf is given by the q->len
			* variable. */
			memcpy( cur_prt,q->payload, q->len);
			cur_prt +=q->len;
		}
	
		send_frame(ptr, p->tot_len, 1);
		free(ptr);
#endif 
		send_frame(p, p->tot_len, 1);
	}
	
	return 0;
}

//ZOT
//for other packets, but TCP/IP
int send_pkt(int intno,UINT8 *buffer,unsigned int length)
{
    struct ether ep;

	if(length > 1514)
		return -1;

	memcpy( &ep, buffer, sizeof(struct ether) );

	
	if(STAR_MAC_Plugout == 0)
	{
		send_frame(buffer, length, 0);
	}
	
	return 0;
}
//ZOT
err_t
ethernetif_output(struct netif *netif, struct pbuf *p,
      struct ip_addr *ipaddr)
{
  
 /* resolve hardware address, then send (or queue) packet */
  return etharp_output(netif, ipaddr, p);
 
}

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'
//ZOT
err_t pk_attach( struct netif *netif)
{
	int i;
	struct ethernetif *ethernetif;
    
  	ethernetif = malloc(sizeof(struct ethernetif));
	if (ethernetif == NULL)
	{
  		LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
  		return ERR_MEM;
	}
	
	netif->state = ethernetif;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = ethernetif_output;
	netif->linkoutput = SendPacket;
	
	ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
	
	/* set MAC hardware address length */
	netif->hwaddr_len = 6;
	
	/* set MAC hardware address */
	for ( i = 0; i < 6; ++i)
    {
        netif->hwaddr[i] = MyPhysNodeAddress[i];
    }
	
	/* maximum transfer unit */
	netif->mtu = 1500;
	
	/* broadcast capability */
	netif->flags = NETIF_FLAG_BROADCAST;
	
	netif_set_up(netif);

	return ERR_OK;
}
extern err_t tcpip_input(struct pbuf *p, struct netif *inp);
typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
}MIB_DHCP;
MIB_DHCP            *mib_DHCP_p;
MIB_DHCP			mib_DHCP;

//ZOT
void LanPktInit()
{
	struct netif *netif;
	struct ip_addr ipaddr, netmask, gw;	
        
	netif_init();
	
	if(EEPROM_Data.PrintServerMode & PS_DHCP_ON)
	{
		IP4_ADDR( &ipaddr,0,0,0,0);
		IP4_ADDR( &netmask, 0,0,0,0);
		IP4_ADDR( &gw, 0,0,0,0);
		mib_DHCP.IPAddr = 0;
		mib_DHCP.SubnetMask = 0;
		mib_DHCP.GwyAddr = 0;
		
	}
	else
	{
		ipaddr.addr = NGET32(EEPROM_Data.BoxIPAddress);
		netmask.addr = NGET32(EEPROM_Data.SubNetMask);
		gw.addr = NGET32(EEPROM_Data.GetwayAddress);

		mib_DHCP.IPAddr = get32(EEPROM_Data.BoxIPAddress);
		mib_DHCP.SubnetMask = get32(EEPROM_Data.SubNetMask);
		mib_DHCP.GwyAddr = get32(EEPROM_Data.GetwayAddress);
	}

	netif = malloc(sizeof(struct netif));
	Lanface = netif_add( netif ,&ipaddr, &netmask, &gw, MyPhysNodeAddress, pk_attach, tcpip_input);
	
	Lanface->name[0] = 0x4C;	      
//	pk_attach(0 , netif);
//Jesse	netif = malloc(sizeof(struct netif));
	netif_set_default(netif); 
	
	mib_DHCP_p = &mib_DHCP;
		
	etharp_init();
//ZOT716u2 	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);	//move to tcpip_thread
    cyg_thread_create(BridgeTask_TASK_PRI,
                  BridgeTask,
                  0,
                  "BridgeTask",
                  (void *)(BridgeTask_Stack),
                  BridgeTask_TASK_STACK_SIZE,
                  &BridgeTask_Pckt_TaskHdl,
                  &BridgeTask_Pckt_Task);
	
	
	Process_OthPkt_handle = malloc(sizeof(cyg_handle_t));	//ZOT716u2
	Process_OthPkt_mbox = malloc(sizeof(cyg_mbox));	//ZOT716u2

	cyg_mbox_create(Process_OthPkt_handle, Process_OthPkt_mbox);
	
    cyg_thread_create(sysinfo_TASK_PRI,
                  ProcessOtherPcktTask,
                  0,
                  sysinfo_TASK_NAME,
                  (void *)(Process_Other_Pckt_Stack),
                  sysinfo_TASK_STACK_SIZE,
                  &Process_Other_Pckt_TaskHdl,
                  &Process_Other_Pckt_Task);
     
     

}

typedef  struct m_eth_hdr {
	unsigned char        dest[HW_ADDR_LEN];
	unsigned char        src[HW_ADDR_LEN];
	unsigned short       type;
} PACK m_eth_hdr;


//Recognize Other packet for eCos
int ProcessOtherPckt(unsigned char *pFrame, unsigned int lenFrame)
{
	m_eth_hdr *eth;
	unsigned short etherType;
	uint16 FrameType;
	
	eth = (m_eth_hdr *)pFrame;
	//check Ethernet type
    etherType = ntohs(eth->type);
	
	if(etherType == IPXENII){
		FrameType = IPXENII;
#ifdef WEBADMIN
		IPXENIIRecv++;
#endif //WEBADMIN
	}
	else 
		FrameType = NGET16(&pFrame[14]);
		
	switch(FrameType) {
		case IPX8023:
#ifdef WEBADMIN
			IPX8023Recv++;
#endif //WEBADMIN
		case IPXENII:
			if (lenFrame > 14)
			    IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[14],(lenFrame-14));
			break;
		case IPX8022:
#ifdef WEBADMIN
			IPX8022Recv++;
#endif //WEBADMIN
			if (lenFrame > 17)
			    IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[17],(lenFrame-17));
			break;
		case IPXSNAP:
#ifdef WEBADMIN
			IPXSNAPRecv++;
#endif //WEBADMIN
			*((char*)&FrameType+1) = pFrame[20]; //ethernet type
			*(char*)&FrameType     = pFrame[21];
			
			if(FrameType == IPXENII) {
				if (lenFrame > 22)
				    IPXInput(IPXSNAP,eth->src,(IPXHeader *)&pFrame[22],(lenFrame-22));
			}

			break;
#ifdef NETBEUI
		case IPXBEUI:
			if(!memcmp(eth->dest,NETBEUI_MULTICAST,6)){
				if (lenFrame > 17)
				    NT_NETBEUItoIPXInput(FrameType,eth->src,(NETBEUIHeader *)&pFrame[17],(lenFrame-17));
			}
			else return -1;
		break;
#endif
		default:
			return -1;
	}
	return 0;
}

void LanRecv(unsigned char *data, unsigned int len)
{
	struct pbuf *p,*q;
	struct netif *netif;
	char *ptr;
	m_eth_hdr *eth;
	unsigned short etherType;
    unsigned char bCastFlag = 0;
    
    if ((data == NULL) || (len == NULL))
	{
		return ;
	}
	
	netif = Lanface;
	
	eth = (m_eth_hdr *)data;	
			
	//check Ethernet type
    etherType = ntohs(eth->type);

	if( !memcmp(eth->dest, MyPhysNodeAddress, 6) || ((eth->dest[0]& 0x01) == 1) || IS_BROADCAST_HWADDR(eth->dest) )
	{// Normal Process. If else, do below bridge process
		switch (etherType)
			{
		    case ETH_IP_TYPE:
		    
				if( Lanrecvcnt >= 20)			    
			    	break;
		    
#ifndef L2_ZERO_CPY		
				p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
#else			
				p = pbuf_alloc(PBUF_RAW, len, PBUF_REF);
				p->flags = PBUF_FLAG_REF2; //Ron 6/25/2005, use for IP Layer Zero Copy
#endif			
				if( p == NULL )
					return;
					
				ptr = data;
				for(q = p; q != NULL; q = q->next) {
				/* Read enough bytes to fill this pbuf in the chain. The
				* available data in the pbuf is given by the q->len
				* variable. */
#ifndef L2_ZERO_CPY				
					memcpy(q->payload, ptr, q->len);
#else				
					q->payload = ptr;
#endif
					ptr +=q->len;
				
				}
				/* update ARP table */
			    etharp_ip_input(netif, p);
			    /* skip Ethernet header */
			    pbuf_header(p, -sizeof(struct eth_hdr));
			    /* pass to network layer */
			    tcpip_input(p, netif);
			    Lanrecvcnt ++;
				break;
			
			case ETH_ARP_TYPE:
				p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
					
				if( p == NULL )
					return;
				
				ptr = data;
				for(q = p; q != NULL; q = q->next) {
				  /* Read enough bytes to fill this pbuf in the chain. The
				   * available data in the pbuf is given by the q->len
				   * variable. */
				   memcpy(q->payload, ptr, q->len);
				   ptr +=q->len;
					 
				}
				etharp_arp_input(netif, MyPhysNodeAddress, p);	
				break;
			
			default:
//				ProcessOtherPckt(data, len);
				OtherPckt_input(data, len);
				break;
			}//switch	    
	}//if		

}

struct OtherPcktr_mbuf
{
	struct OtherPcktr_mbuf *next;	/* Links packets on queues */
	char *data;
	unsigned int len;
};


struct OtherPcktr_mbuf *OTHERPCKT_READ_QUEUE = NULL;

static struct OtherPcktr_mbuf *OtherPckt_dequeue(struct OtherPcktr_mbuf **q)
{
	struct OtherPcktr_mbuf *bp;
	
	if(q == NULL)
		return NULL;
	
	if((bp = *q) != NULL){
		*q = bp->next;
		bp->next = NULL;
	}

	return bp;
}

void ProcessOtherPcktTask(cyg_addrword_t Data)
{
	struct OtherPcktr_mbuf *buf;
	int i = 0;
#if 0	//ZOT716u2
	while(1){
		cyg_thread_yield();
		while(i	< 8)
		{
			cli();	//ZOT716u2
			if( (buf = OtherPckt_dequeue(OTHERPCKT_READ_QUEUE))== NULL)
			{
				sti();	//ZOT716u2			
				break;
			}
			OtherPcktrecvcnt --;			
			sti();	//ZOT716u2
			
			
			
			ProcessOtherPckt( buf->data, buf->len);
			free(buf->data);
			free(buf);
			i++;
		}
		i = 0;
	}
#else	//ZOT716u2
	unsigned int timeout = 1000;	//1 second
	void *msg;
	
	while (1) {
	
		msg = cyg_mbox_timed_get(Process_OthPkt_mbox, cyg_current_time() + (timeout / MSPTICK));
		
		if (msg == NULL)
		{
			continue;	//timeout;
		}
		else
		{
			buf = (struct OtherPcktr_mbuf *)msg;
			ProcessOtherPckt( buf->data, buf->len);
			free(buf->data);
			free(buf);			
			cli();	
			OtherPcktrecvcnt --;
			sti();	
			
		}
	
	}	
#endif	//ZOT716u2	
}

void OtherPckt_input(char *data, int length)
{
	struct OtherPcktr_mbuf *mbuf;
	
	if(OtherPcktrecvcnt >= 20)
		return;
	
	mbuf = malloc(sizeof(struct OtherPcktr_mbuf));
	if (!mbuf)
		return;
	memset(mbuf, 0, sizeof(struct OtherPcktr_mbuf));	
	mbuf->data = malloc(length);
	if (!mbuf->data)
	{
		free(mbuf);
		return;	
	}
	memcpy(mbuf->data, data, length);
	mbuf->len = length;

	cyg_mbox_put(Process_OthPkt_mbox, mbuf);	//ZOT716u2

	OtherPcktrecvcnt++;
	
}

void LanPktStart()
{
	cyg_thread_resume(BridgeTask_Pckt_TaskHdl);
	
	cyg_thread_resume(Process_Other_Pckt_TaskHdl);
}

void BridgeTask(cyg_addrword_t Data)
{
	while (1)
    {
    	cyg_thread_yield();
    	star_nic_receive_packet(0, 1); // Receive Once
    }	
}