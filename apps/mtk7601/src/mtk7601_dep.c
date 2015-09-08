
#include "stdio.h"
#include "stdlib.h"
#include "mtk7601_dep.h"
#include "rt_config.h"
#include "pstarget.h"
#include "psglobal.h"

#define UBDG_DROP	( (struct netif *)4 )
//Timer
#define ZOT_Timer_TASK_PRI              20
#define ZOT_Timer_TASK_STACK_SIZE	    8192

static cyg_uint8 		ZOT_Timer_Stack[ZOT_Timer_TASK_STACK_SIZE];
static cyg_thread       ZOT_Timer_Task;
static cyg_handle_t     ZOT_Timer_TaskHdl;
struct zot_timer *zTimers = NULL;

extern PRTMP_ADAPTER       Global_pAd;	
extern struct netif *Lanface;
extern struct netif *WLanface;
extern int Lanrecvcnt;
extern cyg_sem_t rMlmeThread;		
extern cyg_sem_t rRTUSBCmdThread; 	
cyg_mutex_t zot_timer_mt;

spinlock_t WLan_queue_lock;

typedef  struct Weth_hdr {
	unsigned char        dest[HW_ADDR_LEN];
	unsigned char        src[HW_ADDR_LEN];
	unsigned short       type;
} PACK Weth_hdr;

void zot_timer_task(cyg_addrword_t arg)
{
	register struct zot_timer *t;
	register struct zot_timer *expired;

	cyg_uint32 clock;
	int i_state;

	for(;;){

		cyg_thread_yield();

		if(zTimers == NULL)
			continue;	/* No active timers, all done */

		/* Initialize null expired timer list */
		expired = NULL;
		clock = cyg_current_time();
	
		i_state = dirps();
		cyg_mutex_lock(&zot_timer_mt);

		/* Move expired timers to expired list. Note use of
		 * subtraction and comparison to zero rather than the
		 * more obvious simple comparison; this avoids
		 * problems when the clock count wraps around.
		 */
		while((zTimers != NULL) && ((int)(clock - zTimers->expiration) >= 0)){
			/* Save Timers since stop_timer will change it */
			t = zTimers;
			zTimers = t->next;
			t->state = TIMER_EXPIRE;
			/* Add to expired timer list */
			t->next = expired;
			expired = t;
		}
		
		cyg_mutex_unlock(&zot_timer_mt);
		restore(i_state);
		/* Now go through the list of expired timers, removing each
		 * one and kicking the notify function, if there is one
		 */
		while((t = expired) != NULL){
			expired = t->next;
			if(t->func){
				(*t->func)(t->arg);
			}
		}
	}	
}	
	
void ZOT_Timer_init(void)
{
	spin_lock_init(&WLan_queue_lock);//ZOT==>
	
	cyg_mutex_init(&zot_timer_mt);
	
    cyg_thread_create(ZOT_Timer_TASK_PRI,
                  zot_timer_task,
                  0,
                  "zot_timer",
                  (void *) (ZOT_Timer_Stack),
                  ZOT_Timer_TASK_STACK_SIZE,
                  &ZOT_Timer_TaskHdl,
                  &ZOT_Timer_Task);
	
	//Start ZOT_Timer Thread
	cyg_thread_resume(ZOT_Timer_TaskHdl);	
}

void set_timer(struct zot_timer *t, cyg_uint32 interval)
{
	if(t == NULL)
		return;
	/* Round the interval up to the next full tick, and then
	 * add another tick to guarantee that the timeout will not
	 * occur before the interval is up. This is necessary because
	 * we're asynchronous with the system clock.
	 */
	if(interval != 0)
		t->duration = 1 + (interval + MSPTICK - 1)/MSPTICK;
	else
		t->duration = 0;

}

/* Start a timer */
void start_timer(struct zot_timer *t)
{
	register struct zot_timer *tnext;
	struct zot_timer *tprev = NULL;

	if(t == NULL)
		return;
	if(t->state == TIMER_RUN)
		stop_timer(t);
	if(t->duration == 0)
		return;		/* A duration value of 0 disables the timer */

	cyg_mutex_lock(&zot_timer_mt);

	t->expiration = cyg_current_time() + t->duration;
		
	t->state = TIMER_RUN;

	/* Find right place on list for this guy. Once again, note use
	 * of subtraction and comparison with zero rather than direct
	 * comparison of expiration times.
	 */
	for(tnext = zTimers;tnext != NULL;tprev=tnext,tnext = tnext->next){
		if((signed int)(tnext->expiration - t->expiration) >= 0)
			break;
	}
	/* At this point, tprev points to the entry that should go right
	 * before us, and tnext points to the entry just after us. Either or
	 * both may be null.
	 */
	if(tprev == NULL)
		zTimers = t;		/* Put at beginning */
	else
		tprev->next = t;

	t->next = tnext;
	
	cyg_mutex_unlock(&zot_timer_mt);
}

/* Stop a timer */
void stop_timer(struct zot_timer *timer)
{
	register struct zot_timer *t;
	struct zot_timer *tlast = NULL;

	if(timer == NULL || timer->state != TIMER_RUN)
		return;

	cyg_mutex_lock(&zot_timer_mt);

	/* Verify that timer is really on list */
	for(t = zTimers;t != NULL;tlast = t,t = t->next)
		if(t == timer)
			break;

	if(t == NULL){
		cyg_mutex_unlock(&zot_timer_mt);
		return;		/* Should probably panic here */
	}

	/* Delete from active timer list */
	if(tlast != NULL)
		tlast->next = t->next;
	else
		zTimers = t->next;	/* Was first on list */

	t->state = TIMER_STOP;
	
	cyg_mutex_unlock(&zot_timer_mt);
}

void mod_timer(struct timer_list* timer, unsigned long expires)
{
	struct timer* 	ptimer;

	timer->expires = expires;
	ptimer = (struct timer *) timer->magic;

	ptimer->arg = (void*) timer->data;
	ptimer->func = timer->function;
	set_timer (ptimer, timer->expires);
	start_timer (ptimer);	
}

void init_timer (struct timer_list* timer)
{
	struct timer* 	new_t;

	new_t = (struct timer *) malloc (sizeof (struct timer));
	memset ((void*) new_t, 0, sizeof (struct timer));
	memset ((void*) timer, 0, sizeof (struct timer_list));
	timer->magic = (unsigned long)new_t;
}

void add_timer (struct timer_list* timer)
{
	struct timer* 	ptimer;

	ptimer = (struct timer *) timer->magic;
	ptimer->arg = (void*) timer->data;
	ptimer->func = timer->function;
	set_timer (ptimer, timer->expires);
	start_timer (ptimer);
}

int del_timer_sync (struct timer_list* timer)
{
	struct timer* 	ptimer;

	ptimer = (struct timer *) timer->magic;

	stop_timer (ptimer);

	return 0;	
}


/*
int cal_pkt_use = 0;
*/
struct sk_buff* dev_alloc_skb (unsigned int length)
{
	struct sk_buff* skb; 
	int i_state;

	i_state = dirps();
	skb = (struct sk_buff *)malloc(sizeof(struct sk_buff));

	if(skb != NULL) {
    	/* Clear just the header portion */
		memset(skb,0,sizeof(struct sk_buff));
		if((skb->size = length) != 0) {		
			skb->data = aligned_alloc(length, 4096);
			skb->original_data = skb->data;
			if(skb->data == NULL)
			{
				free(skb);
	            restore(i_state);
				return NULL;
			}
			memset(skb->data, 0 ,length);	
			skb->data = ((uint32)skb->data & 0xFFFFF000);
		}
	}else{
        restore(i_state);
		return NULL;	
	}
    
    /*
    cal_pkt_use ++;
    DBGPRINT(RT_DEBUG_ERROR, ("termy say, cal_pkt_use = %d, size = %d, addr = %x\n", cal_pkt_use, length, skb->data));
    */
    
	restore(i_state);
	return skb;	
}

void dev_kfree_skb_any (struct sk_buff* skb)
{
	if (skb)
	{ 
        /*
        cal_pkt_use --;
        DBGPRINT(RT_DEBUG_ERROR, ("termy say free, size = %d, addr = %x\n", skb->size, skb->data));
        */ 
		aligned_free(skb->original_data); 		
		free(skb);
	}	
}


unsigned char* skb_put (struct sk_buff* skb, unsigned int len)
{
	skb->len += len;
	return skb->data;
}

//malloc
void *
aligned_alloc(size_t nb, size_t align)
{
	uint32 *cp;
	char *p;

	if( align < sizeof(uint32) )
		align = sizeof(uint32);
	align--;

	p = malloc( nb + sizeof(uint32) + align );

	if( p== NULL ) return NULL;
	
	memset(p, 0, nb + sizeof(uint32) + align);
	cp = ((uint32)( p + sizeof(uint32) + align ) & ~((uint32)align));
	cp[-1] = p;

	return cp;
}

void
aligned_free(void *block)
{
	char *cp = block;
	char *p;

	if( block == NULL ) return;

	p = *(uint32 *)( cp - sizeof(uint32) );

	free( p );
}

extern unsigned char WirelessLightToggle;

void EAP_input(char *data, int length)
{
	
}

void WLanRecv(unsigned char *data, unsigned int len)
{
	struct pbuf *p,*q;
	struct netif *netif;
	char *ptr;
	Weth_hdr *eth;
	unsigned short etherType;
	struct netif *ifp;
    
    if ((data == NULL) || (len == NULL))
	{
		return ;
	}

	eth = (Weth_hdr *)data;

	ifp = bridge_in(WLanface, eth);
	if (ifp == UBDG_DROP) {
		return;
	}
	
//	netif = WLanface; //Jesse, beacuse Printer server only have one interface in TCP/TP layer
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
			    pbuf_header(p, -sizeof(struct Weth_hdr));
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
				
			/* received 802.1x authentication packets ... Ron 5/19/2003 */
			case ETH_EAP_TYPE:
				EAP_input(data, len);
				break;	
				
			default:
//				ProcessOtherPckt(data, len);
				OtherPckt_input(data, len);
				break;
			}//switch	    
	}//if		
	WirelessLightToggle++;
}

void skb_reserve (struct sk_buff* skb, unsigned int len)
{
	unsigned char *tmp = skb->data;
	
	tmp = (unsigned char *)((unsigned long)skb->data + len);
	
	skb->data = tmp;
}

unsigned long simple_strtoul (const char* cp, char** endp, unsigned int base)
{
	unsigned long	result = 0,value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((toupper (*cp) == 'X') && isxdigit (cp[1])) {
				cp++;
				base = 16;
			}
		}
	}
	else if (base == 16) {
		if (cp[0] == '0' && toupper (cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit (*cp) && (value = isdigit (*cp) ? *cp - '0' : toupper (*cp) - 'A' + 10) < base) {
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char*) cp;
	return result;
}


long simple_strtol (const char* cp, char** endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoul (cp + 1, endp, base);
	return simple_strtoul (cp, endp, base);
}


/*
void RTUSBMlmeUp(PRTMP_ADAPTER pAd)
{
	RTMP_SEM_EVENT_UP(&rMlmeThread);
}

void RTCMDUp(PRTMP_ADAPTER pAd)
{
	RTMP_SEM_EVENT_UP(&rRTUSBCmdThread); 
}
*/

struct sk_buff* skb_clone (struct sk_buff* skb, int gfp_mask)
{}

/*
void get_unaligned(char* data)
{
	return data;
}
*/

void SetAnySsid(IN	PRTMP_ADAPTER	pAd)
{	
	int i=0,j =0;
	int ARssi = 0;

	ARssi = pAd->ScanTab.BssEntry[i].Rssi;
		
	for(i =0 ;i < pAd->ScanTab.BssNr; i++)
	{
		if((pAd->ScanTab.BssEntry[i].AuthMode != Ndis802_11AuthModeOpen)&&(pAd->ScanTab.BssEntry[i].WepStatus == Ndis802_11WEPEnabled))
			continue;
			
		if(ARssi < pAd->ScanTab.BssEntry[i].Rssi)
		{
			ARssi = pAd->ScanTab.BssEntry[i].Rssi;			
			j = i;
		}		
	}
	
	NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->ScanTab.BssEntry[j].Ssid ,pAd->ScanTab.BssEntry[j].SsidLen);
	pAd->MlmeAux.AutoReconnectSsidLen = pAd->ScanTab.BssEntry[j].SsidLen;
	NdisMoveMemory(pAd->CommonCfg.Ssid, pAd->ScanTab.BssEntry[j].Ssid ,pAd->ScanTab.BssEntry[j].SsidLen);
	pAd->CommonCfg.SsidLen = pAd->ScanTab.BssEntry[j].SsidLen;
	pAd->CommonCfg.LastSsidLen= pAd->CommonCfg.SsidLen;
	NdisCopyMemory(pAd->CommonCfg.LastSsid, pAd->CommonCfg.Ssid, pAd->CommonCfg.LastSsidLen);
	pAd->MlmeAux.SsidLen = pAd->CommonCfg.SsidLen;
}

extern PNET_DEV    g_wireless_dev; /* global wireless device */
void emi_task(cyg_addrword_t data)
{
	RTMP_ADAPTER *pAd;	
    GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

	if (mvCTX) {
		RTMPStartEMITx (pAd, mvTxPower, mvChannel, mvTxMode, mvDataRate);
	}else if (mvCRX) {
		RTMPStartEMIRx (pAd, mvDataRate);
	}
}

VOID RTMPStartEMITx (PRTMP_ADAPTER pAdapter, UCHAR txPower, UCHAR channel, UCHAR wireless_mode, UCHAR tx_rate)
{
	/* check EVM & Power
	 * set	ATE			= STASTOP
	 * set	ATEDA		= 00:11:22:33:44:55
	 * set	ATDSA		= 00:aa:bb:cc:dd:ee
	 * set	ATEBSSID 	= 00:11:22:33:44:55
	 * set	ATETXRATE 	= 11;
	 * set	ATECHANNEL 	= 1;
	 * set	ATETXLEN	= 1024;
	 * set	ATETXPOW	= 18;
	 * set	ATETXCNT	= 1000000;
	 * set	ATE			= TXFRAME;
	 *
	 */

	 UCHAR	wlMode = 0;
	 UCHAR  TxMode = 0;
	 int ratevalue = 0;
     char str_buffer[32];
	 
	 Set_ATE_Proc(pAdapter, "ATESTART");
	 
	 if(tx_rate < 12)
	 {
		 switch(tx_rate)
		 {
			case 0:  //bit0 (1M)
		        ratevalue = 0;
				break;
			case 1:  //bit1 (2M)
		        ratevalue = 1;
				break;
			case 2:  //bit2 (5.5M)
		        ratevalue = 2;
				break;
			case 3:  //bit3 (11M)
		        ratevalue = 3;
				break;				
			case 4:  //bit0 (6M)
				ratevalue = 0;
				break;
			case 5:  //bit1 (9M)
				ratevalue = 1;
				break;
			case 6:  //bit1 (12M)
				ratevalue = 2;
				break;
			case 7:  //bit1 (18M)
				ratevalue = 3;
				break;
			case 8:  //bit1 (24M)
				ratevalue = 4;
				break;	
			case 9:  //bit1 (36M)
				ratevalue = 5;
				break;
			case 10:  //bit1 (48M)
				ratevalue = 6;
				break;
			case 11:  //bit1 (54M)
				ratevalue = 7;
				break;						
			default:
		        ratevalue = 11;
				break;
		  }
	 }else{
	   	ratevalue = tx_rate - 12;
	 }	 
		
    /* wireless mode */
    //@> WirelessMode=value
	//value	
    //		0: legacy 11b/g mixed 
    //		1: legacy 11B only 
    //		2: legacy 11A only          //Not support in RfIcType=1(id=RFIC_5225) and RfIcType=2(id=RFIC_5325)
    //		3: legacy 11a/b/g mixed     //Not support in RfIcType=1(id=RFIC_5225) and RfIcType=2(id=RFIC_5325)
    //		4: legacy 11G only
    //		5: 11ABGN mixed
    //		6: 11N only
    //		7: 11GN mixed
    //		8: 11AN mixed
    //		9: 11BGN mixed
    //	   10: 11AGN mixed	
    //
    //	    mvTxMode;		            // 0x00: B-G mixed  0x01: B only  0x02: G only 0x03:B-G-N mixed 

	 if (wireless_mode == 0){
	 	wlMode = PHY_11BG_MIXED;
	 	if(tx_rate > 3)
	 		TxMode = 1;
	 	else
	 		TxMode = 0;	
	} else if (wireless_mode == 1){
	 	wlMode = PHY_11B;
		TxMode = 0;	
		if(tx_rate > 3)	
			ratevalue = 3;		
	} else if (wireless_mode == 2){
	 	wlMode = PHY_11G;
		TxMode = 1;
		if(tx_rate > 11)	
			ratevalue = 7;	
	} else if (wireless_mode == 4){
		wlMode = PHY_11N_2_4G;
		TxMode = 2;
		if(tx_rate > 11)	
			ratevalue = 7;
	} else{ 	
	 	wlMode = PHY_11BGN_MIXED;
	 	if(tx_rate > 11)
	 		TxMode = 2;
	 	else if(tx_rate > 3)
	 		TxMode = 1;	
	 	else
	 		TxMode = 0;
	}
	 	
    /*
    sprintf(str_buffer, "%d\0", wlMode);
    Set_WirelessMode_Proc (pAdapter, str_buffer);
    */
	 	 
    Set_ATE_DA_Proc (pAdapter, "00:11:22:33:44:55");
    Set_ATE_SA_Proc (pAdapter, "00:aa:bb:cc:dd:ee");
    Set_ATE_BSSID_Proc (pAdapter, "00-11-22-33-44-55");

    sprintf(str_buffer, "%d\0", TxMode);
    Set_ATE_TX_MODE_Proc(pAdapter, str_buffer);
    sprintf(str_buffer, "%d\0", channel);
    Set_ATE_CHANNEL_Proc (pAdapter, str_buffer);
    sprintf(str_buffer, "%d\0", mvBandWidth);
    Set_ATE_TX_BW_Proc(pAdapter, str_buffer);
    /*
    sprintf(str_buffer, "%d\0", ratevalue);
    Set_ATE_TX_MCS_Proc (pAdapter, str_buffer);	 	 
    */
	 
    Set_ATE_TX_LENGTH_Proc (pAdapter, "1024");
    //if (txPower > 9) txPower = 9;
    sprintf(str_buffer, "%d\0", txPower);
    Set_ATE_TX_POWER0_Proc (pAdapter, str_buffer);
    Set_ATE_TX_COUNT_Proc (pAdapter, "1000000");
    Set_ATE_Proc(pAdapter, "TXFRAME");		
	 
	Set_ATE_Proc (pAdapter, "TXCONT");
	
}

VOID RTMPStartEMIRx (PRTMP_ADAPTER pAdapter, UCHAR tx_rate)
{
	/* rx follow
	 * set	ATE				= STASTOP;
	 * set	ResetCounter	= 0;
	 * set	ATETXRATE		= 11;
	 * set	ATE				= RXFRAME;
	 */
    char str_buffer[32];
    int ratevalue;
	
	Set_ATE_Proc(pAdapter, "ATESTART");
	Set_ResetStatCounter_Proc (pAdapter, 0);

	 if(tx_rate < 12)
	 {
		 switch(tx_rate)
		 {
			case 0:  //bit0 (1M)
		        ratevalue = 0;
				break;
			case 1:  //bit1 (2M)
		        ratevalue = 1;
				break;
			case 2:  //bit2 (5.5M)
		        ratevalue = 2;
				break;
			case 3:  //bit3 (11M)
		        ratevalue = 3;
				break;				
			case 4:  //bit0 (6M)
				ratevalue = 0;
				break;
			case 5:  //bit1 (9M)
				ratevalue = 1;
				break;
			case 6:  //bit1 (12M)
				ratevalue = 2;
				break;
			case 7:  //bit1 (18M)
				ratevalue = 3;
				break;
			case 8:  //bit1 (24M)
				ratevalue = 4;
				break;	
			case 9:  //bit1 (36M)
				ratevalue = 5;
				break;
			case 10:  //bit1 (48M)
				ratevalue = 6;
				break;
			case 11:  //bit1 (54M)
				ratevalue = 7;
				break;						
			default:
		        ratevalue = 11;
				break;
		  }
    }else{
	   	ratevalue = tx_rate - 12;
    }	
    sprintf(str_buffer, "%d\n", ratevalue);
	Set_ATE_TX_MCS_Proc (pAdapter, str_buffer);
	Set_ATE_Proc (pAdapter, "RXFRAME");

}

void tasklet_init(struct tasklet_struct *t,
                  void (*func)(unsigned long), 
                  unsigned long data)
{
    t->next = NULL;
    t->state = 0;
    atomic_set(&t->count, 0);
    t->func = func;
    t->data = data;
}

void tasklet_schedule(struct tasklet_struct *t)
{
    t->func(t->data);
}

void tasklet_hi_schedule(struct tasklet_struct *t)
{
    t->func(t->data);
}

void tasklet_kill(struct tasklet_struct *t)
{
}

char *
strsep(char **stringp, const char* delim)
{
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

void init_completion(struct completion *x)
{
    x->done = 0;
    /*
    init_waitqueue_head(&x->wait);
    */
    // cgy_semaphore_init(cyg_sem_t *sem, cyg_count32 value);
    spin_lock_init (&x->wait.lock);
    cyg_semaphore_init (&x->wait.semaphore, 0);
    x->wait.test = 0x5A5A;
};

void complete(struct completion *x)
{
    //unsigned long flags;
    //spin_lock_irqsave(&x->wait.lock, flags);
    spin_lock_irq(&x->wait.lock);
    x->done++;

    /*
    __wake_up_locked(&x->wait, TASK_NORMAL, 1);
    */
    spin_unlock_irq(&x->wait.lock);
    //spin_unlock_irqrestore(&x->wait.lock, flags);
    cyg_semaphore_post(&x->wait.semaphore);
    x->wait.test = 0xA5A5;
}

void wait_for_completion(struct completion *x)
{
    spin_lock_irq(&x->wait.lock);
    if (!x->done) {
        spin_unlock_irq(&x->wait.lock);
        cyg_semaphore_wait(&x->wait.semaphore);
    }
    spin_unlock_irq(&x->wait.lock);
}

unsigned long 
wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{

#if 0
    long ltimeout = (long)timeout;

    spin_lock_irq(&x->wait.lock);
    if (!x->done) {
        do {
            #if 0
            spin_unlock_irq(&x->wait.lock);
            cyg_thread_delay(10);
            spin_lock_irq(&x->wait.lock);
            #endif
            #if 0
            spin_unlock_irq(&x->wait.lock);
            mdelay(100);
            spin_lock_irq(&x->wait.lock);
            #endif 
            spin_unlock_irq(&x->wait.lock);
            cyg_semaphore_timed_wait(&x->wait.semaphore,
                                     cyg_current_time()+timeout/MSPTICK);
            spin_lock_irq(&x->wait.lock);

            ltimeout -= 100;
            if (ltimeout < 0)
                ltimeout = 0;
        }
        while(!x->done && ltimeout);
    }
    if(ltimeout)
        x->done --;
    spin_unlock_irq(&x->wait.lock);
    return (unsigned long)ltimeout;
#endif

    spin_lock_irq(&x->wait.lock);
    if (!x->done) {
        spin_unlock_irq(&x->wait.lock);
        cyg_semaphore_timed_wait(&x->wait.semaphore,
                                 cyg_current_time()+timeout/MSPTICK);

        spin_lock_irq(&x->wait.lock);
        if (x->done) {
            x->done --;
            spin_unlock_irq(&x->wait.lock);
            return 1;
        } else {
            spin_unlock_irq(&x->wait.lock);
            return 0;
        }
    }
    x->done --;
    spin_unlock_irq(&x->wait.lock);

    return 1;
}

void complete_and_exit(struct completion *comp, long code)
{
    if (comp)
        complete(comp);
    /*
    do_exit(code);
    */
}

unsigned short eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    return htons(ETH_P_IP);
}

unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
    skb->data -= len;
    skb->len  += len;
    /*
    if (unlikely(skb->data<skb->head))
        skb_under_panic(skb, len, __builtin_return_address(0));
    */
    return skb->data;
}

unsigned char *skb_pull(struct sk_buff *skb, unsigned int len)
{
    if (len > skb->len)
        return NULL;

    skb->len -= len;
    /*
    BUG_ON(skb->len < skb->data_len);
    */
    return skb->data += len;
}

void skb_trim(struct sk_buff *skb, unsigned int len)
{
    if (skb->len > len) {
        skb->len = len;
        skb->tail = skb->data;
        skb->tail += len;
    }
}

void disassoc_cmd_sw(void)
{
#if 0
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)Global_pAd;	

	if (pAd != NULL)
	{
		MLME_DISASSOC_REQ_STRUCT   DisassocReq;
		
		if (INFRA_ON(pAd))
		{
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
						sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, 0);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;

			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
		    pAd->ExtraInfo = GENERAL_LINK_DOWN;
		}	
	}
#endif
}


int pskb_expand_head(struct sk_buff *skb, 
                     int nhead, 
                     int ntail, 
                     gfp_t gfp_mask) {
}

void netif_start_queue(struct net_device *dev) {
}

void netif_stop_queue(struct net_device *dev) {
}

extern unsigned char WirelessLightToggle;
int netif_rx(struct sk_buff *skb) {

    WLanRecv(skb->data, skb->len);
    dev_kfree_skb_any(skb);	

	WirelessLightToggle++;
}

#include "stdarg.h"
#define EOF -1

char *scan_string (const char *str, int base)
{
	char *str_ptr = (char*) str;

	switch (base)
	{
		case 10:
			while (!(isdigit(*str_ptr) || *str_ptr == '-' || *str_ptr == 0x0))
			{
				str_ptr++;
			} 
			break;
		case 16:
			while (!(isxdigit(*str_ptr) || *str_ptr == 0x0))
			{
				str_ptr++;
			} 
			break;
	}

	return str_ptr;
}

int sscanf(const char *str, const char *fmt, ...)
{
	int ret;
	va_list ap;
	char *format_ptr = (char*)fmt;
	char *str_ptr = (char*)str;

	int *p_int;
	long *p_long;

	ret = 0;

	va_start (ap, fmt);

	while ((*format_ptr != 0x0) && (*str_ptr !=	0x0))
	{
		if (*format_ptr == '%')
		{
			format_ptr++;

			if (*format_ptr != 0x0)
			{
				switch (*format_ptr)
				{
				case 'd':	// expect an int
				case 'i':
					p_int = va_arg( ap, int *);
					str_ptr=scan_string(str_ptr, 10);
					if (*str_ptr==0x0) goto end_parse; 
					*p_int = (int)strtol (str_ptr, &str_ptr, 10);
					ret ++;
					break;
				case 'D':
				case 'I': 	// expect a long
					p_long = va_arg( ap, long *);
					str_ptr=scan_string(str_ptr, 10);
					if (*str_ptr==0x0) goto end_parse;
					*p_long = strtol (str_ptr, &str_ptr, 10);
					ret ++;
					break;
				case 'x':	// expect an int in hexadecimal format
					p_int = va_arg( ap, int *);
					str_ptr=scan_string(str_ptr, 16);
					if (*str_ptr==0x0) goto end_parse;
					*p_int = (int)strtol (str_ptr, &str_ptr, 16);
					ret ++;
					break;
				case 'X':  // expect a long in hexadecimal format	
					p_long = va_arg( ap, long *);
					str_ptr=scan_string(str_ptr, 16);
					if (*str_ptr==0x0) goto end_parse;
					*p_long = strtol (str_ptr, &str_ptr, 16);
					ret ++;
					break;
				}
			}
		}
		
		format_ptr++;
	}	

end_parse:	
	va_end (ap);

	if (*str_ptr == 0x0) ret = EOF;
	return ret;
}

char printk_buf[1024];

int printk(const char *fmt, ...)
{
    int n;

    va_list args;
    va_start(args, fmt);
    n = vsprintf(printk_buf, fmt, args);    
    va_end(args);

    serial_puts(printk_buf); 
    return 0;
}

