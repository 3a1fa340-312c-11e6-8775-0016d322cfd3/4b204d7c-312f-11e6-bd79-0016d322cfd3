#include "pstarget.h"
#include "psglobal.h"
#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include "wlanif.h"


#define UBDG_DROP	( (struct netif *)4 )

extern struct netif *Lanface;
extern struct netif *WLanface;
extern int Lanrecvcnt;
struct EAP_mbuf *EapQueue = NULL;
extern struct netif *bridge_in(struct netif *ifp, struct ether *hdr);
void EAP_input(char *data, int length);
spinlock_t WLan_queue_lock; //ZOT==> static unsigned int WLan_queue_lock = 0; //eason 20090828
//ZOT==> #define spin_lock_irqsave( s, f )		{ f = dirps(); *s = 1; }
//ZOT==> #define spin_unlock_irqrestore( s, f )  { *s = 0; restore(f); }

struct EAP_mbuf
{
	struct EAP_mbuf *next;	/* Links packets on queues */
	uint8 *data;
	unsigned int len;
};


void dev_kfree_skb_any (struct sk_buff* skb)
{
	if (skb)
	{ 
		kaligned_free(skb->data); 		
		kfree(skb, 0);
	}	
}

struct sk_buff* skb_clone (struct sk_buff* skb, int gfp_mask)
{}

void skb_reserve (struct sk_buff* skb, unsigned int len)
{
	unsigned char *tmp = skb->data;
	
	tmp = (unsigned char *)((unsigned long)skb->data + len);
	
	skb->data = tmp;
}

unsigned char* skb_put (struct sk_buff* skb, unsigned int len)
{
	skb->len += len;
	return skb->data;
}

struct sk_buff* dev_alloc_skb (unsigned int length)
{
	struct sk_buff* skb; 
	int i_state;

	i_state = dirps();
	skb = (struct sk_buff *)kmalloc(sizeof(struct sk_buff),0);
	
	if(skb != NULL) {
    	/* Clear just the header portion */
		memset(skb,0,sizeof(struct sk_buff));
		if((skb->size = length) != 0) {
			skb->data = kaligned_alloc(length, 4096);
			if(skb->data == NULL)
				return NULL;
			memset(skb->data, 0 ,length);	
			skb->data = ((uint32)skb->data & 0xFFFFF000);
		}
	}else{
		return NULL;	
	}
	restore(i_state);
	return skb;	
}

struct sk_buff* dev_alloc_skb_malloc (unsigned int length)
{
	struct sk_buff* skb; 
	int i_state;

	i_state = dirps();
	skb = (struct sk_buff *)malloc((length + sizeof(struct sk_buff)));
	
	if(skb != NULL) {
    	/* Clear just the header portion */
		memset(skb,0,length + sizeof(struct sk_buff));
		if((skb->size = length) != 0) {
			skb->data = (uint8 *)((uint8*)skb + sizeof(struct sk_buff));			
		}
	}else{
		return NULL;	
	}
	restore(i_state);
	return skb;	
}

void init_timer (struct timer_list* timer)
{
	struct timer* 	new_t;

	new_t = (struct timer *) malloc (sizeof (struct timer));
	memset ((void*) new_t, 0, sizeof (struct timer));
	memset ((void*) timer, 0, sizeof (struct timer_list));
	timer->magic = (unsigned long)new_t;
}

typedef  struct Weth_hdr {
	unsigned char        dest[HW_ADDR_LEN];
	unsigned char        src[HW_ADDR_LEN];
	unsigned short       type;
} PACK Weth_hdr;

extern unsigned char WirelessLightToggle;

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

int del_timer_sync (struct timer_list* timer)
{
	struct timer* 	ptimer;

	ptimer = (struct timer *) timer->magic;

	stop_timer (ptimer);

	return 0;	
}
//---------------------------------no yet
int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	int i, copy;
	int start = skb->len;//eason 20100210 - skb->data_len;

	if (offset > (int)skb->len-len)
		goto fault;

	/* Copy header. */
	if ((copy = start-offset) > 0) {
		if (copy > len)
			copy = len;
		memcpy(to, skb->data + offset, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		to += copy;
	}

/*	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		BUG_TRAP(start <= offset+len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end-offset) > 0) {
			u8 *vaddr;

			if (copy > len)
				copy = len;

			vaddr = kmap_skb_frag(&skb_shinfo(skb)->frags[i]);
			memcpy(to, vaddr+skb_shinfo(skb)->frags[i].page_offset+
			       offset-start, copy);
			kunmap_skb_frag(vaddr);

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			to += copy;
		}
		start = end;
	}

	if (skb_shinfo(skb)->frag_list) {
		struct sk_buff *list;

		for (list = skb_shinfo(skb)->frag_list; list; list=list->next) {
			int end;

			BUG_TRAP(start <= offset+len);

			end = start + list->len;
			if ((copy = end-offset) > 0) {
				if (copy > len)
					copy = len;
				if (skb_copy_bits(list, offset-start, to, copy))
					goto fault;
				if ((len -= copy) == 0)
					return 0;
				offset += copy;
				to += copy;
			}
			start = end;
		}
	}
	if (len == 0)
		return 0;
*/
fault:
	return -EFAULT;
}

void wireless_send_event(struct net_device *dev,unsigned int cmd, union iwreq_data *wrqu,char *	extra)
{}



//---------------------------------no yet
void msleep (unsigned int msecs)
{
	ppause(msecs);
}

//skb_queue
void skb_queue_head_init(struct sk_buff_head *list)
{
	_spinlock_init(&list->lock);
	list->prev = (struct sk_buff *)list;
	list->next = (struct sk_buff *)list;
	list->qlen = 0;	
}

void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;
	
	spin_lock_irqsave(&WLan_queue_lock, &flags); //ZOT==>spin_lock_irqsave(&WLan_queue_lock, flags);//eason no yet	_enter_critical(&list->lock, flags);
	__skb_queue_tail(list, newsk);
	spin_unlock_irqrestore(&WLan_queue_lock, &flags);//ZOT==> spin_unlock_irqrestore(&WLan_queue_lock, flags);//eason no yet	_exit_critical(&list->lock, flags);	
}

void __skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
	struct sk_buff *prev, *next;

	newsk->list = list;
	list->qlen++;
	next = (struct sk_buff *)list;
	prev = next->prev;
	newsk->next = next;
	newsk->prev = prev;
	next->prev = newsk;
	prev->next = newsk;
}

void skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;
	while ((skb=skb_dequeue(list))!=NULL)
		kfree_skb(skb);
}

/**
 *	skb_dequeue - remove from the head of the queue
 *	@list: list to dequeue from
 *
 *	Remove the head of the list. The list lock is taken so the function
 *	may be used safely with other locking list functions. The head item is
 *	returned or %NULL if the list is empty.
 */
struct sk_buff * skb_dequeue(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&WLan_queue_lock, &flags);//ZOT==> spin_lock_irqsave(&WLan_queue_lock, flags);//eason no yet	_enter_critical(&list->lock, flags);
	result = __skb_dequeue(list);
	spin_unlock_irqrestore(&WLan_queue_lock, &flags);//ZOT==>	spin_unlock_irqrestore(&WLan_queue_lock, flags);//eason no yet	_exit_critical(&list->lock, flags);
	return result;
}

struct sk_buff *__skb_dequeue(struct sk_buff_head *list)
{
	struct sk_buff *next, *prev, *result;

	prev = (struct sk_buff *) list;
	next = prev->next;
	result = NULL;
	if (next != prev) {
		result = next;
		next = next->next;
		list->qlen--;
		next->prev = prev;
		prev->next = next;
		result->next = NULL;
		result->prev = NULL;
		result->list = NULL;
	}
	return result;
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

//Timer
#define ZOT_Timer_TASK_PRI         20
#define ZOT_Timer_TASK_STACK_SIZE	 8192
static cyg_uint8 			ZOT_Timer_Stack[ZOT_Timer_TASK_STACK_SIZE];
static cyg_thread       ZOT_Timer_Task;
static cyg_handle_t     ZOT_Timer_TaskHdl;

struct zot_timer *zTimers = NULL;
cyg_mutex_t zot_timer_mt;

void zot_timer_task(cyg_addrword_t arg)
{
	register struct zot_timer *t;
	register struct zot_timer *expired;
//	void (** vf)(void);
//	int tmp;
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


/* Append packet to end of packet queue */
void
enqueue(
struct EAP_mbuf **q,
struct EAP_mbuf **bpp
){
	struct EAP_mbuf *p;
	int i_state;

	if(q == NULL || bpp == NULL || *bpp == NULL)
		return;
	i_state = dirps();
	if(*q == NULL){
		/* List is empty, stick at front */
		*q = *bpp;
	} else {
		for(p = *q ; p->next != NULL ; p = p->next)
			;
		p->next = *bpp;
	}
	*bpp = NULL;	/* We've consumed it */
	restore(i_state);
	
}

/* Unlink a packet from the head of the queue */
struct EAP_mbuf *
dequeue(struct EAP_mbuf **q)
{
	struct EAP_mbuf *bp;
	int i_state;

	if(q == NULL)
		return NULL;
	i_state = dirps();
	if((bp = *q) != NULL){
		*q = bp->next;
		bp->next = NULL;
	}
	restore(i_state);
	return bp;
}

void EAP_input(char *data, int length)
{
	struct EAP_mbuf *mbuf;
	
	mbuf = malloc(sizeof(struct EAP_mbuf));
	if (!mbuf)
		return;
	memset(mbuf, 0, sizeof(struct EAP_mbuf));	
	mbuf->data = malloc(length);
	if (!mbuf->data)
	{
		free(mbuf);
		return;	
	}
	memset(mbuf->data, 0, length);
	memcpy(mbuf->data, data, length);
	mbuf->len = length;

	enqueue(&EapQueue, &mbuf);
	
}

extern _adapter *Global_padapter; //eason 20100210
void set__adapter_mlmepriv(char *ssid , int len)
{
	_adapter *padapter = (_adapter *)Global_padapter;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	
	if(len == 0)
		memset(&pmlmepriv->assoc_ssid.Ssid, 0, 32);
	else
	memcpy(&pmlmepriv->assoc_ssid.Ssid, ssid, len);
	pmlmepriv->assoc_ssid.SsidLength = len;
}

void wlan_save_eep(void)
{
	EEPROM_Data.WLMode = 0;
	
	strcpy(EEPROM_Data.WLESSID,  mvESSID);
	EEPROM_Data.WLAuthType = mvAuthenticationType;
	EEPROM_Data.WLWEPType = mvWEPType;
	
	EEPROM_Data.WLWEPType = mvWEPType ;
	EEPROM_Data.WLWPAType = mvWPAType;
	EEPROM_Data.WLWEPKeySel = mvWEPKeySel;
	
	memset( EEPROM_Data.WLWEPKey1, 0, sizeof(EEPROM_Data.WLWEPKey1) );
	memset( EEPROM_Data.WLWEPKey2, 0, sizeof(EEPROM_Data.WLWEPKey2) );
	memset( EEPROM_Data.WLWEPKey3, 0, sizeof(EEPROM_Data.WLWEPKey3) );
	memset( EEPROM_Data.WLWEPKey4, 0, sizeof(EEPROM_Data.WLWEPKey4) );
	memset( EEPROM_Data.WLWEP128Key, 0, sizeof(EEPROM_Data.WLWEP128Key) );
	memset( EEPROM_Data.WLWEP128Key2, 0, sizeof(EEPROM_Data.WLWEP128Key2) );
	memset( EEPROM_Data.WLWEP128Key3, 0, sizeof(EEPROM_Data.WLWEP128Key3) );
	memset( EEPROM_Data.WLWEP128Key4, 0, sizeof(EEPROM_Data.WLWEP128Key4) );
	
	memcpy( EEPROM_Data.WLWEPKey1, mvWEPKey1, sizeof(EEPROM_Data.WLWEPKey1) );
	memcpy( EEPROM_Data.WLWEPKey2, mvWEPKey2, sizeof(EEPROM_Data.WLWEPKey2) );
	memcpy( EEPROM_Data.WLWEPKey3, mvWEPKey3, sizeof(EEPROM_Data.WLWEPKey3) );
	memcpy( EEPROM_Data.WLWEPKey4, mvWEPKey4, sizeof(EEPROM_Data.WLWEPKey4) );
	memcpy( EEPROM_Data.WLWEP128Key, mvWEP128Key, sizeof(EEPROM_Data.WLWEP128Key) );
	memcpy( EEPROM_Data.WLWEP128Key2, mvWEP128Key2, sizeof(EEPROM_Data.WLWEP128Key2) );
	memcpy( EEPROM_Data.WLWEP128Key3, mvWEP128Key3, sizeof(EEPROM_Data.WLWEP128Key3) );
	memcpy( EEPROM_Data.WLWEP128Key4, mvWEP128Key4, sizeof(EEPROM_Data.WLWEP128Key4) );
		
	memset( EEPROM_Data.WPA_Pass, 0, 64 );
	memcpy( EEPROM_Data.WPA_Pass, mvWPAPass, sizeof(EEPROM_Data.WPA_Pass));
	
	// George added this at build0008 of 716U2W on May 10, 2012.
	EEPROM_Data.WPSButtonPressedCount += 1;

	WriteToEEPROM(&EEPROM_Data);
}

void disassoc_cmd_sw(void)
{
	_adapter *padapter = (_adapter *)Global_padapter;


	if ((if_up(padapter)) == _FALSE) {
		return;
	}

	disassoc_cmd(padapter);
			
	if (check_fwstate(&padapter->mlmepriv, _FW_LINKED) == _TRUE)
		indicate_disconnect(padapter);
		
//		cyg_thread_yield();
}
