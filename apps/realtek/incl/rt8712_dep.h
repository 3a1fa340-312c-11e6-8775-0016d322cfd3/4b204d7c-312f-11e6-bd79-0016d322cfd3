//eason 20100210
#include <stdio.h>
#include "list.h"		
#include "psglobal.h"	
#include "pstarget.h"
#include "usb.h"
#include "wireless_copy.h"
#include "eeprom.h"
#define WLANMAC		RT8188
#define GFP_KERNEL						0
#define GFP_ATOMIC						0
#define ARPHRD_ETHER    		1
#define KERNEL_VERSION(a,b,c)	b
#define LINUX_VERSION_CODE	4
	

//#define le32_to_cpu( z )				( z )
#define time_after(a,b)			(a > b)
#define INIT_WORK rt8712_work
//#define kmalloc(s,t)	malloc(s)
#define free_netdev(x)       kfree(x, 0) 
//#define kfree(p)				free(p)
#define usb_get_dev		usb_inc_dev_use
#define usb_put_dev		usb_dec_dev_use
#define usb_kill_urb(urb)	usb_unlink_urb(urb)
#define SIGUSR1		10

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Software timers
 * There is one of these structures for each simulated timer.
 * Whenever the timer is running, it is on a linked list
 * pointed to by "Timers". The list is sorted in ascending order of
 * expiration, with the first timer to expire at the head. This
 * allows the timer process to avoid having to scan the entire list
 * on every clock tick; once it finds an unexpired timer, it can
 * stop searching.
 *
 * Stopping a timer or letting it expire causes it to be removed
 * from the list. Starting a timer puts it on the list at the right
 * place.
 */
struct timer {
	struct timer *next;	/* Linked-list pointer */
	int32 duration;		/* Duration of timer, in ticks */
	int32 expiration;	/* Clock time at expiration */
	void (*func)(void *);	/* Function to call at expiration */
	void *arg;		/* Arg to pass function */
	char state;		/* Timer state */
#define	TIMER_STOP	0
#define	TIMER_RUN	1
#define	TIMER_EXPIRE	2
};

//ZOT Timer
struct zot_timer {
	struct zot_timer *next;	/* Linked-list pointer */
	cyg_uint32 duration;		/* Duration of timer, in ticks */
	cyg_uint32 expiration;	/* Clock time at expiration */
	void (*func)(void *);	/* Function to call at expiration */
	void *arg;		/* Arg to pass function */
	char state;		/* Timer state */
#define	TIMER_STOP	0
#define	TIMER_RUN	1
#define	TIMER_EXPIRE	2
};

void set_timer(struct zot_timer *t, cyg_uint32 interval);
void start_timer(struct zot_timer *t);
void stop_timer(struct zot_timer *timer);
