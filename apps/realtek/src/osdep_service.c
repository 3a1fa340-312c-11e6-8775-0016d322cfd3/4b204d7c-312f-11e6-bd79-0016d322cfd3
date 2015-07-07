/******************************************************************************
* osdep_service.c                                                                                                                                 *
*                                                                                                                                          *
* Description :                                                                                                                       *
*                                                                                                                                           *
* Author :                                                                                                                       *
*                                                                                                                                         *
* History :                                                          
*
*                                        
*                                                                                                                                       *
* Copyright 2007, Realtek Corp.                                                                                                  *
*                                                                                                                                        *
* The contents of this file is the sole property of Realtek Corp.  It can not be                                     *
* be used, copied or modified without written permission from Realtek Corp.                                         *
*                                                                                                                                          *
*******************************************************************************/


#define _OSDEP_SERVICE_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>

#ifdef RTK_DMP_PLATFORM
#include <auth.h>
#endif

#define RT_TAG	'1178'

//eason 20100210
//#define _malloc(s) kmalloc(s)
//#define _memset(p,c,s) memset(p,c,s)
//#define _memcpy(d,s,z) memcpy(d,s,z)

#if 0 //eason 20100210
u8* _malloc(u32 sz)
{

	u8 	*pbuf;

#ifdef PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		pbuf = dvr_malloc(sz);
	else
#endif
	pbuf = 	kmalloc(sz, /*GFP_KERNEL*/GFP_ATOMIC);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);

#endif

	return pbuf;	
	
}
#endif  //eason 20100210
void	_mfree(u8 *pbuf, u32 sz)
{

#ifdef	PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		dvr_free(pbuf);
	else
#endif
	kfree(pbuf, 0);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisFreeMemory(pbuf,sz, 0);

#endif
	
	
}

#if 0 //eason 20100210
void _memcpy(void* dst, void* src, u32 sz)
{

#ifdef PLATFORM_LINUX

	memcpy(dst, src, sz);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMoveMemory(dst, src, sz);

#endif

}
#endif  //eason 20100210

int	__memcmp(void *dst, void *src, u32 sz)
{

#ifdef PLATFORM_LINUX
//under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
#endif


#ifdef PLATFORM_WINDOWS
//under Windows, the return value of NdisEqualMemory for two same mem. chunk is 1
	
	if (NdisEqualMemory (dst, src, sz))
		return _TRUE;
	else
		return _FALSE;

#endif	
	
	
	
}
#if 0 //eason 20100210
void _memset(void *pbuf, int c, u32 sz)
{

#ifdef PLATFORM_LINUX

        memset(pbuf, c, sz);

#endif

#ifdef PLATFORM_WINDOWS

        NdisZeroMemory(pbuf, sz);

#endif

}
	
#endif //eason 20100210


void    _init_listhead(_list *list)
{

#ifdef PLATFORM_LINUX

        INIT_LIST_HEAD(list);

#endif

#ifdef PLATFORM_WINDOWS

        NdisInitializeListHead(list);

#endif

}


/*
For the following list_xxx operations, 
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	is_list_empty(_list *phead)
{

#ifdef PLATFORM_LINUX

	if (list_empty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif
	

#ifdef PLATFORM_WINDOWS

	if (IsListEmpty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif

	
}


void list_insert_tail(_list *plist, _list *phead)
{

#ifdef PLATFORM_LINUX	
	
	list_add_tail(plist, phead);
	
#endif
	
#ifdef PLATFORM_WINDOWS

  InsertTailList(phead, plist);

#endif		
	
}


/*

Caller must check if the list is empty before calling list_delete

*/


void _init_sema(_sema	*sema, int init_val)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 sema_init(sema, init_val);
	cyg_semaphore_init(sema,init_val);

#endif

#ifdef PLATFORM_OS_XP

	KeInitializeSemaphore(sema, init_val,  SEMA_UPBND); // count=0;

#endif
	
#ifdef PLATFORM_OS_CE
	if(*sema == NULL)
		*sema = CreateSemaphore(NULL, init_val, SEMA_UPBND, NULL);
#endif

}

void _free_sema(_sema	*sema)
{

#ifdef PLATFORM_OS_CE
	CloseHandle(*sema);
#endif

}

void _up_sema(_sema	*sema)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 up(sema);
	cyg_semaphore_post(sema);
#endif	

#ifdef PLATFORM_OS_XP

	KeReleaseSemaphore(sema, IO_NETWORK_INCREMENT, 1,  FALSE );

#endif

#ifdef PLATFORM_OS_CE
	ReleaseSemaphore(*sema,  1,  NULL );
#endif
}

u32 _down_sema(_sema *sema)
{

#ifdef PLATFORM_LINUX
#if 0 //eason 20100210 	
	if (down_interruptible(sema))
		return _FAIL;
    else
    	return _SUCCESS;
#else
	cyg_semaphore_wait(sema); 
	return _SUCCESS;
#endif //eason 20100210
#endif    	

#ifdef PLATFORM_OS_XP

	if(STATUS_SUCCESS == KeWaitForSingleObject(sema, Executive, KernelMode, TRUE, NULL))
		return  _SUCCESS;
	else
		return _FAIL;
#endif

#ifdef PLATFORM_OS_CE
	if(WAIT_OBJECT_0 == WaitForSingleObject(*sema, INFINITE ))
		return _SUCCESS; 
	else
		return _FAIL;
#endif
}



void	_rwlock_init(_rwlock *prwlock)
{
#ifdef PLATFORM_LINUX

	init_MUTEX(prwlock);

#endif
#ifdef PLATFORM_OS_XP

	KeInitializeMutex(prwlock, 0);

#endif

#ifdef PLATFORM_OS_CE
	*prwlock =  CreateMutex( NULL, _FALSE, NULL);
#endif
}


void	_spinlock_init(_lock *plock)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 spin_lock_init(plock);
	cyg_mutex_t * mut_t = plock;
	cyg_mutex_init(mut_t);	

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisAllocateSpinLock(plock);

#endif
	
}

void	_spinlock_free(_lock *plock)
{

	
#ifdef PLATFORM_WINDOWS

	NdisFreeSpinLock(plock);

#endif
	
}


void	_spinlock(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 spin_lock(plock);
	cyg_mutex_t * mut_t = plock;	
	cyg_mutex_lock(mut_t);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisAcquireSpinLock(plock);

#endif
	
}

void	_spinunlock(_lock *plock)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 spin_unlock(plock);
	cyg_mutex_t * mut_t = plock;
	cyg_mutex_unlock(mut_t);
	
#endif
	
#ifdef PLATFORM_WINDOWS

	NdisReleaseSpinLock(plock);

#endif
}


void	_spinlock_ex(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 spin_lock(plock);
	cyg_mutex_t * mut_t = plock;	
	cyg_mutex_lock(mut_t);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisDprAcquireSpinLock(plock);

#endif
	
}

void	_spinunlock_ex(_lock *plock)
{

#ifdef PLATFORM_LINUX

	//eason 20100210 spin_unlock(plock);
	cyg_mutex_t * mut_t = plock;
	cyg_mutex_unlock(mut_t);

#endif
	
#ifdef PLATFORM_WINDOWS

	NdisDprReleaseSpinLock(plock);

#endif
}
void  _enter_critical(_lock *plock, _irqL *pirqL)
{
	//eason 20100210 spin_lock_irqsave(plock, *pirqL);
	cyg_mutex_t * mut_t = plock;	
	*pirqL = dirps();	
	cyg_mutex_lock(mut_t);
}

void  _exit_critical(_lock *plock, _irqL *pirqL)
{
	//eason 20100210 spin_unlock_irqrestore(plock, *pirqL);
	cyg_mutex_t * mut_t = plock;
	cyg_mutex_unlock(mut_t);
	restore(*pirqL);
}

void  _enter_critical_ex(_lock *plock, _irqL *pirqL)
{
	//eason 20100210 spin_lock_irqsave(plock, *pirqL);
	cyg_mutex_t * mut_t = plock;	
	*pirqL = dirps();	
	cyg_mutex_lock(mut_t);
}

void  _exit_critical_ex(_lock *plock, _irqL *pirqL)
{
	//eason 20100210 spin_unlock_irqrestore(plock, *pirqL);
	cyg_mutex_t * mut_t = plock;
	cyg_mutex_unlock(mut_t);
	restore(*pirqL);
}

void	_init_queue(_queue	*pqueue)
{

	_init_listhead(&(pqueue->queue));

	_spinlock_init(&(pqueue->lock));

}

u32	  _queue_empty(_queue	*pqueue)
{
	return (is_list_empty(&(pqueue->queue)));
}


u32 end_of_queue_search(_list *head, _list *plist)
{

	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
		
}


u32	get_current_time(void)
{
	
#ifdef PLATFORM_LINUX

	return jiffies;

#endif	
	
#ifdef PLATFORM_WINDOWS

	LARGE_INTEGER	SystemTime;
	NdisGetCurrentSystemTime(&SystemTime);
	return (u32)(SystemTime.LowPart);// count of 100-nanosecond intervals 

#endif
	
	
}
#define TASK_INTERRUPTIBLE	1	//eason 20100210
void sleep_schedulable(int ms)	
{

#ifdef PLATFORM_LINUX

    u32 delta;
    
    delta = (ms * HZ)/1000;//(ms)
    if (delta == 0) {
        delta = 1;// 1 ms
    }
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
    return;

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMSleep(ms*1000); //(us)*1000=(ms)

#endif

}


void msleep_os(int ms)
{

#ifdef PLATFORM_LINUX

  	msleep((unsigned int)ms);

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMSleep(ms*1000); //(us)*1000=(ms)

#endif


}
void usleep_os(int us)
{

#ifdef PLATFORM_LINUX
  	
       msleep((unsigned int)us);


#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisMSleep(us); //(us)

#endif


}

void mdelay_os(int ms)
{

#ifdef PLATFORM_LINUX

   	mdelay((unsigned long)ms); 

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisStallExecution(ms*1000); //(us)*1000=(ms)

#endif


}
void udelay_os(int us)
{

#ifdef PLATFORM_LINUX

      udelay((unsigned long)us); 

#endif	
	
#ifdef PLATFORM_WINDOWS

	NdisStallExecution(us); //(us)

#endif

}
