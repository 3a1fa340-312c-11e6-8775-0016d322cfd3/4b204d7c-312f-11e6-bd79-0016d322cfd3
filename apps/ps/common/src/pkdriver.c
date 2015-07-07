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
#include "at.h"

#ifdef WEBADMIN	 //6/26/2000
uint32 IPXENIIRecv;
uint32 IPX8023Recv;
uint32 IPX8022Recv;
uint32 IPXSNAPRecv;
#endif WEBADMIN

cyg_handle_t * Process_OthPkt_handle;
cyg_mbox * Process_OthPkt_mbox;

//BridgeTask Packet Task
#define BridgeTask_TASK_STACK_SIZE		4096//2048
#define BridgeTask_TASK_PRI     		20
static 					cyg_thread BridgeTask_Pckt_Task;
static 					cyg_handle_t BridgeTask_Pckt_TaskHdl;
static UINT8 			BridgeTask_Stack[BridgeTask_TASK_STACK_SIZE];

//Process Other Packet Task
#define sysinfo_TASK_NAME  "Process_Other_Pckt_Task"
#define sysinfo_TASK_STACK_SIZE  4096//1024
#define sysinfo_TASK_PRI         20
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

//eason 20100210 extern int usbnet_start_xmit( char * data, unsigned int len, int flag);

extern int STAR_MAC_Plugout;
extern struct netif *Lanface;
extern struct netif *ULanface;

//-----------------------------------------------eason 20091008 
//ZOT==> typedef unsigned int	spinlock_t; //eason 20090828
//ZOT==> #define spin_lock_irqsave( s, f )		{ f = dirps(); *s = 1; }
//ZOT==> #define spin_unlock_irqrestore( s, f )  { *s = 0; restore(f); }
//----------------------------------------------eason 20091008 WLAN
extern struct netif *WLanface;
#define WLAN_TX_TASK_PRI         	20
#define WLAN_TX_TASK_STACK_SIZE  	8192
static	uint8			WLAN_TX_Stack[WLAN_TX_TASK_STACK_SIZE];
static  cyg_thread		WLAN_TX_Task;
static  cyg_handle_t	WLAN_TX_TaskHdl;
void WLAN_TX_Thread(cyg_addrword_t data);
spinlock_t WLan_TX_lock;//ZOT==> static spinlock_t WLan_TX_lock = 0; //eason 20090828
int WLan_queuecount = 0; //eason 20091008
struct sk_buff *WLAN_READ_QUEUE = NULL; //eason 20091008
cyg_sem_t wlan_tx_sem;
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

int usb_usb_send;

#if 0
struct sk_buff {
	struct sk_buff *next;		/* Links mbufs belonging to single packets */
	struct sk_buff *anext;		/* Links packets on queues */
	uint size;				/* Size of associated data buffer */
	int refcnt;				/* Reference count */
	struct sk_buff *list;		/* List we are on */
	uint8 *data;			/* Active working pointers */
	uint len;
	char cb[42];
};
#endif

struct sk_buff {
	struct sk_buff *next;		/* Links mbufs belonging to single packets */
	struct sk_buff *prev;		/* Links packets on queues */
	uint size;				    /* Size of associated data buffer */
	int refcnt;				    /* Reference count */
	struct sk_buff_head * list; /* List we are on				*/
	uint8 *data;			    /* Active working pointers */
	uint8 *original_data;
	uint len;

    uint8                    __pkt_type_offset[0];
    uint8                    pkt_type:3;
    uint8                    pfmemalloc:1;
    uint8                    ignore_df:1;

    uint8                    nf_trace:1;
    uint8                    ip_summed:2;
    uint8                    ooo_okay:1;
    uint8                    l4_hash:1;
    uint8                    sw_hash:1;
    uint8                    wifi_acked_valid:1;
    uint8                    wifi_acked:1;

    uint8                    no_fcs:1;
    /* Indicates the inner headers are valid in the skbuff. */
    uint8                    encapsulation:1;
    uint8                    encap_hdr_csum:1;
    uint8                    csum_valid:1;
    uint8                    csum_complete_sw:1;
    uint8                    csum_level:2;
    uint8                    csum_bad:1;

    uint8                    ipvs_property:1;
    uint8                    inner_protocol_type:1;
    uint8                    remcsum_offload:1;
    unsigned short           protocol;

    struct net_device       *dev;
    //sk_buff_data_t          tail;
    unsigned char*          tail;
    uint8                   *head;

	char cb[42];
	};

//ZOT
//for TCP/IP only
err_t SendPacket(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	char *ptr, *cur_prt;
	struct ether ep;
	struct netif *dst;
	struct sk_buff* pSkb;
	unsigned long flags;//eason 20091008
	if(p->tot_len > 1514)
		return -1;

	memcpy( &ep, p->payload, sizeof(struct ether) );


	dst = bridge_dst_lookup(&ep);

	if(usb_usb_send == 1)
		dst = ULanface;

	if( dst == WLanface)
	{
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		if(WLan_queuecount > 10)
		{
			spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
			return;
		}
		WLan_queuecount++;
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		
		pSkb = dev_alloc_skb(2000);
		if( pSkb == NULL)
			return -1;
		cur_prt = pSkb->data;
		for(q = p; q != NULL; q = q->next) {
			memcpy( cur_prt,q->payload, q->len);
			cur_prt +=q->len;
		}
		pSkb->len = p->tot_len;
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		WLan_tx_enqueue(&WLAN_READ_QUEUE,&pSkb);
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		return 0;
	}

	if( dst == ULanface)
	{
#ifdef PreTest
		usbnet_start_xmit( p, p->tot_len, 1);
#endif	
		return 0;
	}




if( dst == Lanface){
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
#endif //0
		send_frame(p, p->tot_len, 1);
	}
}	
	if( dst != WLanface && dst != Lanface && dst != ULanface){
		send_frame(p, p->tot_len, 1);

		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		if(WLan_queuecount > 10)
		{
			spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
			return;
		}
		WLan_queuecount++;
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
	
		pSkb = dev_alloc_skb(2000);
		if( pSkb == NULL)
			return -1;
		cur_prt = pSkb->data;
		for(q = p; q != NULL; q = q->next) {
			memcpy( cur_prt,q->payload, q->len);
			cur_prt +=q->len;
		}
		pSkb->len = p->tot_len;
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		WLan_tx_enqueue(&WLAN_READ_QUEUE,&pSkb);
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008	
	}
	return 0;
}

//ZOT
//for other packets, but TCP/IP
int send_pkt(int intno,UINT8 *buffer,unsigned int length)
{
    struct ether ep;
    struct netif *dst;
	struct sk_buff* pSkb;
	unsigned long flags;//eason 20091008	
	
	if(length > 1514)
		return -1;

	memcpy( &ep, buffer, sizeof(struct ether) );


	dst = bridge_dst_lookup(&ep);

	if(usb_usb_send == 1)
		dst = ULanface;

	if( dst == ULanface)
	{
#ifdef PreTest
		 usbnet_start_xmit( buffer, length, 0);
#endif
		return 0;
	}

	if( dst == WLanface)
	{
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		if(WLan_queuecount > 10)
		{
			spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
			return;
		}
		WLan_queuecount++;
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		
		pSkb = dev_alloc_skb(2000);
		if( pSkb == NULL)
			return -1;
		memcpy( pSkb->data,buffer, length);
		pSkb->len = length;
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		WLan_tx_enqueue(&WLAN_READ_QUEUE,&pSkb);
		spin_unlock_irqrestore(&WLan_TX_lock, &flags); //ZOT==> spin_unlock_irqrestore(&WLan_TX_lock, flags);//eason 20091008
		return 0;
	}
	if( dst == Lanface)
	{
		send_frame(buffer, length, 0);
	}
	
	if( dst != WLanface && dst != Lanface && dst != ULanface){
		send_frame(buffer, length, 0);
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		if(WLan_queuecount > 10)
		{
			spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
			return;
		}
		WLan_queuecount++;
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		
		pSkb = dev_alloc_skb(2000);
		if( pSkb == NULL)
			return -1;
		memcpy( pSkb->data , buffer, length);
		pSkb->len = length;
		spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
		WLan_tx_enqueue(&WLAN_READ_QUEUE,&pSkb);
		spin_unlock_irqrestore(&WLan_TX_lock, &flags); //ZOT==> spin_unlock_irqrestore(&WLan_TX_lock, flags);//eason 20091008
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
        
	spin_lock_init(&WLan_TX_lock);//ZOT==>
        
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


	// enable bridge
	bdginit();


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
//---------------------------------------------------eason 20100210     
		//Create WLANNET_TX_ Thread
		cyg_thread_create(WLAN_TX_TASK_PRI,
							WLAN_TX_Thread,
							0,
							"WLANTX",
							(void *) (WLAN_TX_Stack),
							WLAN_TX_TASK_STACK_SIZE,
							&WLAN_TX_TaskHdl,
							&WLAN_TX_Task);
//---------------------------------------------------eason 20100210
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
#endif WEBADMIN
	}
	else 
		FrameType = NGET16(&pFrame[14]);
		
	switch(FrameType) {
		case IPX8023:
#ifdef WEBADMIN
			IPX8023Recv++;
#endif WEBADMIN
		case IPXENII:
			if (lenFrame > 14)
			    IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[14],(lenFrame-14));
			break;
		case IPX8022:
#ifdef WEBADMIN
			IPX8022Recv++;
#endif WEBADMIN
			if (lenFrame > 17)
			    IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[17],(lenFrame-17));
			break;
		case IPXSNAP:
#ifdef WEBADMIN
			IPXSNAPRecv++;
#endif WEBADMIN
			*((char*)&FrameType+1) = pFrame[20]; //ethernet type
			*(char*)&FrameType     = pFrame[21];
			
#ifdef ATALKD //2/23/99
			switch(FrameType) {
				case IPXENII:  //IPX SNAP frame
					if (lenFrame > 22)
					    IPXInput(IPXSNAP,eth->src,(IPXHeader *)&pFrame[22],(lenFrame-22));
					break;
				case ETHERTYPE_AARP: //APPLE-TALK AARP
					if (lenFrame > 22)
					    aarp_input2ambf(&pFrame[22],(lenFrame-22));
					break;
				case ETHERTYPE_AT: //APPLE-TALK DDP
					if (lenFrame > 22)
					    ddp_input2ambf(&pFrame[22],(lenFrame-22));
					break;
    			default:
					return -1;
			}
#else			
			
			if(FrameType == IPXENII) {
				if (lenFrame > 22)
				    IPXInput(IPXSNAP,eth->src,(IPXHeader *)&pFrame[22],(lenFrame-22));
			}
#endif ATALKD //2/23/99				
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
#define UBDG_DROP	( (struct netif *)4 )
extern unsigned char LANLightToggle;	//eason 20100809
void LanRecv(unsigned char *data, unsigned int len)
{
	struct pbuf *p,*q;
	struct netif *netif;
	char *ptr;
	m_eth_hdr *eth;
	unsigned short etherType;
	struct netif *ifp;
    
    if ((data == NULL) || (len == NULL))
	{
		return ;
	}

	eth = (m_eth_hdr *)data;


	ifp = bridge_in(Lanface, eth);
	if (ifp == UBDG_DROP) {
		return;
	}

	
	netif = Lanface;
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
				{
					return;
				}
				
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
	LANLightToggle++;	//eason 20100809
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
	
	cyg_thread_resume(WLAN_TX_TaskHdl);   //eason 20091008 
}

void BridgeTask(cyg_addrword_t Data)
{
	while (1)
    {
    	cyg_thread_yield();
    	star_nic_receive_packet(0, 8); // Receive Once
    }	
}
//----------------------------------------------WLAN eason 20100210
void WLAN_TX_Thread(cyg_addrword_t data)
{
	int ret = 0;
	struct sk_buff* pSkb;
	unsigned long flags;//eason 20091008
		int i_state;

	cyg_semaphore_init( &wlan_tx_sem, 0);
	while(1){
			spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
			while( (pSkb = WLan_tx_dequeue(&WLAN_READ_QUEUE))== NULL)
			{
				spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
				cyg_semaphore_timed_wait(&wlan_tx_sem, cyg_current_time() + 10);
				spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
			}			
			spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		//	i_state = dirps();
			Wlan_SendPacket(pSkb);
			//restore(i_state);
			spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
			WLan_queuecount--;
			spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
	}
}

void WLan_tx_enqueue(struct sk_buff **q,struct sk_buff **bpp)
{
	struct sk_buff *p;

	if(q == NULL || bpp == NULL || *bpp == NULL)
		return;

	if(*q == NULL){
		/* List is empty, stick at front */
		*q = *bpp;
	} else {
		for(p = *q ; p->next != NULL ; p = p->next)
			;
		p->next = *bpp;
	}
	*bpp = NULL;	/* We've consumed it */		
	cyg_semaphore_post(&wlan_tx_sem);	//eason 20091008
}
struct pbuf *WLan_tx_dequeue(struct sk_buff **q)
{
	struct sk_buff *bp;
	if(q == NULL)
		return NULL;
	
	if((bp = *q) != NULL){
		*q = bp->next;
		bp->next = NULL;
	}
	return bp;
}

int send_eap_pkt(int intno,UINT8 *buffer,unsigned int length)
{
	struct sk_buff* pSkb;
	unsigned long flags;//eason 20091008	
	
	if(length > 1514)
		return -1;
		
	spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
	if(WLan_queuecount > 10)
	{
		spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		return;
	}
	WLan_queuecount++;
	spin_unlock_irqrestore(&WLan_TX_lock, &flags);//ZOT==>			spin_unlock_irqrestore(&WLan_TX_lock, flags); //eason 20091008
		
	pSkb = dev_alloc_skb(2000);
	if( pSkb == NULL)
		return -1;
	memcpy( pSkb->data,buffer, length);
	pSkb->len = length;
	spin_lock_irqsave(&WLan_TX_lock, &flags);	//ZOT==> spin_lock_irqsave(&WLan_TX_lock, flags);//eason 20091008
	WLan_tx_enqueue(&WLAN_READ_QUEUE,&pSkb);
	spin_unlock_irqrestore(&WLan_TX_lock, &flags); //ZOT==> spin_unlock_irqrestore(&WLan_TX_lock, flags);//eason 20091008
	return 0;	
}
//-----------------------------------------eason 20100210
