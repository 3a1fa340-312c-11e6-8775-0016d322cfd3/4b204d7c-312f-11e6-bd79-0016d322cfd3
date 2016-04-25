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

#ifndef USE_PS_LIBS
#undef ATALKD
#undef NETBEUI
#endif /* USE_PS_LIBS */

#ifdef WEBADMIN	 //6/26/2000
uint32 IPXENIIRecv;
uint32 IPX8023Recv;
uint32 IPX8022Recv;
uint32 IPXSNAPRecv;
#endif WEBADMIN

cyg_handle_t * Process_OthPkt_handle;
cyg_mbox * Process_OthPkt_mbox;

//BridgeTask Packet Task
#ifdef STAR_MAC
#define BridgeTask_TASK_STACK_SIZE		4096//2048
#define BridgeTask_TASK_PRI     		20
static 					cyg_thread BridgeTask_Pckt_Task;
static 					cyg_handle_t BridgeTask_Pckt_TaskHdl;
static UINT8 			BridgeTask_Stack[BridgeTask_TASK_STACK_SIZE];
#endif

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
#ifdef STAR_MAC
void BridgeTask(cyg_addrword_t Data);
#endif
void ProcessOtherPcktTask(cyg_addrword_t Data);

WORD MyIntNO;
extern WORD IPXInput(WORD InputFrameType, BYTE *SrcAddress,IPXHeader *p,int16 DataLen);
extern void aarp_input2ambf(void * Data,int16 DataLen);
extern void ddp_input2ambf(char *data,unsigned int len);

//eason 20100210 extern int usbnet_start_xmit( char * data, unsigned int len, int flag);

extern struct netif *Lanface;
extern struct netif *WLanface;
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
#ifdef WIRELESS_CARD
void WLAN_TX_Thread(cyg_addrword_t data);
spinlock_t WLan_TX_lock;//ZOT==> static spinlock_t WLan_TX_lock = 0; //eason 20090828
int WLan_queuecount = 0; //eason 20091008
struct sk_buff *WLAN_READ_QUEUE = NULL; //eason 20091008
cyg_sem_t wlan_tx_sem;
#endif

//ZOT
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

#ifdef STAR_MAC
struct pbuf *WLan_tx_dequeue(struct sk_buff **q);
#endif

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

int usb_usb_send = 0;

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
#ifdef MT7688_MAC
extern err_t
low_level_output(struct netif *netif, struct pbuf *p);
#endif

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

#ifdef WIRELESS_CARD
	if( dst == WLanface)
	{
        #ifdef STAR_MAC
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
        #endif /* STAR_MAC */
        #ifdef MT7688_MAC
        low_level_output(WLanface, p);
        #endif /* MT7688_MAC */
		return 0;
	}
#endif /* WIRELESS_CARD */

	if( dst == ULanface)
	{
#if defined(PreTest) || defined(NDWP2020)
		usbnet_start_xmit( p, p->tot_len, 1);
#endif	
		return 0;
	}


    if( dst == Lanface){
        #ifdef STAR_MAC
        send_frame(p, p->tot_len, 1);
        #endif
        #ifdef MT7688_MAC
        low_level_output(Lanface, p);
        #endif
    }	

	if( dst != Lanface && 
        #ifdef WIRELESS_CARD
        dst != WLanface && 
        #endif
        dst != ULanface) {
#if defined(N716U2W)
        #ifdef STAR_MAC
		send_frame(p, p->tot_len, 1);
        #endif /* STAR_MAC */
        #ifdef MT7688_MAC
        low_level_output(Lanface, p);
        #endif /* MT7688_MAC */
#endif /* N716U2W */
#ifdef WIRELESS_CARD
        #ifdef STAR_MAC
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
        #endif /* STAR_MAC */
        #ifdef MT7688_MAC
        low_level_output(WLanface, p);
        #endif /* MT7688_MAC */
#endif /* WIRELESS_CARD */
	}
	return 0;
}

#ifdef MT7688_MAC
extern err_t
low_level_send_packet(struct netif *netif, unsigned char *pdata, int len);
#endif

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
#if defined(PreTest) || defined(NDWP2020)
		 usbnet_start_xmit( buffer, length, 0);
#endif
		return 0;
	}

#ifdef WIRELESS_CARD
	if( dst == WLanface)
	{
        #ifdef STAR_MAC
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
        #endif /* STAR_MAC */
        #ifdef MT7688_MAC
        low_level_send_packet(WLanface, buffer, length);
        #endif /* MT7688_MAC */
		return 0;
	}
#endif

	if( dst == Lanface)
	{
        #ifdef STAR_MAC
		send_frame(buffer, length, 0);
        #endif
        #ifdef MT7688_MAC
        low_level_send_packet(Lanface, buffer, length);
        #endif
	}
	
#ifdef WIRELESS_CARD
	if( dst != WLanface && dst != Lanface && dst != ULanface){
#if defined(N716U2W)
        #ifdef STAR_MAC
		send_frame(buffer, length, 0);
        #endif /* STAR_MAC */
        #ifdef MT7688_MAC
        low_level_send_packet(Lanface, buffer, length);
        #endif /* MT7688_MAC */
#endif /* N716U2W */
        #ifdef STAR_MAC
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
        #endif /* STAR_MAC */
        #ifdef MT7688_MAC
        low_level_send_packet(WLanface, buffer, length);
        #endif /* MT7688_MAC */
	}
#endif /* WIRELESS_CARD */
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
    #ifdef STAR_MAC
	struct ethernetif *ethernetif;
    
  	ethernetif = malloc(sizeof(struct ethernetif));
	if (ethernetif == NULL)
	{
  		LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
  		return ERR_MEM;
	}
	
	netif->state = ethernetif;
    #endif

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
    netif->output = ethernetif_output;
    netif->linkoutput = SendPacket;
	
    #ifdef STAR_MAC
	ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
    #endif
	
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
        
    #ifdef WIRELESS_CARD
	spin_lock_init(&WLan_TX_lock);//ZOT==>
    #endif
        
	//netif_init();
	
	if(EEPROM_Data.PrintServerMode & PS_DHCP_ON)
	{
		IP4_ADDR( &ipaddr,0,0,0,0);
		IP4_ADDR( &netmask, 0,0,0,0);
		IP4_ADDR( &gw, 0,0,0,0);
		mib_DHCP.IPAddr = 0;
		mib_DHCP.SubnetMask = 0;
		mib_DHCP.GwyAddr = 0;
	    diag_printf("DHCP on \n");	
	}
	else
	{
        diag_printf("DHCP off \n");
        /*
		ipaddr.addr = NGET32(EEPROM_Data.BoxIPAddress);
		netmask.addr = NGET32(EEPROM_Data.SubNetMask);
		gw.addr = NGET32(EEPROM_Data.GetwayAddress);

		mib_DHCP.IPAddr = get32(EEPROM_Data.BoxIPAddress);
		mib_DHCP.SubnetMask = get32(EEPROM_Data.SubNetMask);
		mib_DHCP.GwyAddr = get32(EEPROM_Data.GetwayAddress);
        */

	    IP_ADDR(&gw, CYGDAT_LWIP_SERV_ADDR);
	    IP_ADDR(&ipaddr, CYGDAT_LWIP_MY_ADDR);
	    IP_ADDR(&netmask, CYGDAT_LWIP_NETMASK);

	}

    #ifdef STAR_MAC
	netif = malloc(sizeof(struct netif));
	Lanface = netif_add( netif ,&ipaddr, &netmask, &gw, MyPhysNodeAddress, pk_attach, tcpip_input);
    #endif
    #ifdef MT7688_MAC
    if (Lanface != NULL)
        netif = netif_add (Lanface, &ipaddr, &netmask, &gw, Lanface->state, pk_attach, tcpip_input);
    #endif
	
	Lanface->name[0] = 0x4C;	      
	netif_set_default(netif); 
	
	mib_DHCP_p = &mib_DHCP;

	// enable bridge
	bdginit();

    #ifdef STAR_MAC
	etharp_init();
    #endif /* STAR_MAC */
//ZOT716u2 	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);	//move to tcpip_thread
    #ifdef STAR_MAC
    cyg_thread_create(BridgeTask_TASK_PRI,
                  BridgeTask,
                  0,
                  "BridgeTask",
                  (void *)(BridgeTask_Stack),
                  BridgeTask_TASK_STACK_SIZE,
                  &BridgeTask_Pckt_TaskHdl,
                  &BridgeTask_Pckt_Task);
    #endif /* STAR_MAC */	

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
#ifdef WIRELESS_CARD
		//Create WLANNET_TX_ Thread
        #ifdef STAR_MAC
		cyg_thread_create(WLAN_TX_TASK_PRI,
							WLAN_TX_Thread,
							0,
							"WLANTX",
							(void *) (WLAN_TX_Stack),
							WLAN_TX_TASK_STACK_SIZE,
							&WLAN_TX_TaskHdl,
							&WLAN_TX_Task);
        #endif
#endif
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
            #ifdef USE_PS_LIBS
			if (lenFrame > 14)
			    IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[14],(lenFrame-14));
            #endif /* USE_PS_LIBS */
			break;
		case IPX8022:
#ifdef WEBADMIN
			IPX8022Recv++;
#endif WEBADMIN
            #ifdef USE_PS_LIBS
			if (lenFrame > 17)
			    IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[17],(lenFrame-17));
            #endif /* USE_PS_LIBS */
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
                    #ifdef USE_PS_LIBS
					if (lenFrame > 22)
					    IPXInput(IPXSNAP,eth->src,(IPXHeader *)&pFrame[22],(lenFrame-22));
                    #endif /* USE_PS_LIBS */
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
                #ifdef USE_PS_LIBS
				if (lenFrame > 22)
				    IPXInput(IPXSNAP,eth->src,(IPXHeader *)&pFrame[22],(lenFrame-22));
                #endif /*USE_PS_LIBS */
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

#ifdef STAR_MAC
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
#endif

#ifdef MT7688_MAC
void ecosif_input(struct netif *netif, struct pbuf *p)
{
    struct eth_hdr *ethhdr;
    m_eth_hdr *eth;
    struct netif *ifp;

    if ((p == NULL) || (p->len == 0))
        return;

	eth = (m_eth_hdr *)p->payload;

	ifp = bridge_in(netif, eth);
    /*
	if (ifp == UBDG_DROP) {
        if (netif == Lanface)
            diag_printf("termy say, ecosif_input drop -> Lanface\n");
        else if (netif == WLanface)
            diag_printf("termy say, ecosif_input drop -> WLanface\n");
        else
            diag_printf("termy say, ecosif_input drop\n");

		return;
	}
    */
#if 0
    if (netif == WLanface)
        diag_printf("ecosif_input -> WLanface type=%x\n", htons(ethhdr->type));
    if (ifp == WLanface)
        diag_printf("ecosif_input -> router to WLanface\n");
#endif
    ethhdr = p->payload;

    switch (htons(ethhdr->type)) {
        case ETHTYPE_IP:
            LWIP_DEBUGF(0, ("ecosif_input: IP packet\n"));
            etharp_ip_input(netif, p);
            pbuf_header(p, -sizeof(struct eth_hdr));
            netif->input(p, netif);

            Lanrecvcnt ++;
            break;
        case ETHTYPE_ARP:
            LWIP_DEBUGF(0, ("ecosif_input: ARP packet\n"));
            etharp_arp_input(netif, (struct eth_addr *) &netif->hwaddr, p);
            break;
        default:
            LWIP_DEBUGF(0, ("ecosif_input: Other packet\n"));
            OtherPckt_input(p->payload, p->len);
            pbuf_free(p);
            break;
    }

    LANLightToggle ++;
}

void set_wlan_ip()
{
    #if 0 
	struct ip_addr ipaddr, netmask, gw;	

    if (EEPROM_Data.PrintServerMode & PS_DHCP_ON) {
        IP4_ADDR(&ipaddr, 0, 0, 0, 0);
        IP4_ADDR(&netmask, 0, 0, 0, 0);
        IP4_ADDR(&gw, 0, 0, 0, 0);
    }
    else {
        /*
        ipaddr.addr = NGET32(EEPROM_Data.BoxIPAddress);
        netmask.addr = NGET32(EEPROM_Data.SubNetMask);
        gw.addr = NGET32(EEPROM_Data.GetwayAddress);
        */
	    IP_ADDR(&gw, CYGDAT_LWIP_SERV_ADDR);
	    IP_ADDR(&ipaddr, '192.168.1.223');
	    IP_ADDR(&netmask, CYGDAT_LWIP_NETMASK);
    }
    netif_add(WLanface, &ipaddr, &netmask, &gw, WLanface->state, pk_attach, tcpip_input);
    WLanface->name[0] = 'W';
    #endif
}

#endif

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
    #ifdef STAR_MAC
    #if defined(N716U2W) || defined(N716U2)
	cyg_thread_resume(BridgeTask_Pckt_TaskHdl);
    #endif
    #endif
	
	cyg_thread_resume(Process_Other_Pckt_TaskHdl);
	#if defined(STAR_MAC) && defined(WIRELESS_CARD)
	cyg_thread_resume(WLAN_TX_TaskHdl);   //eason 20091008 
    #endif
}

#ifdef STAR_MAC
void BridgeTask(cyg_addrword_t Data)
{
	while (1)
    {
    	cyg_thread_yield();
    	star_nic_receive_packet(0, 8); // Receive Once
    }	
}
#endif

//----------------------------------------------WLAN eason 20100210
#ifdef WIRELESS_CARD
#ifdef STAR_MAC
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
#endif

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
#endif
//-----------------------------------------eason 20100210
