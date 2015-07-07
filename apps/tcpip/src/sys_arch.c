
#include <cyg/kernel/kapi.h>
#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"

#define MSPTICK 10

struct sys_thread {
  struct sys_thread *next;
  struct sys_timeouts timeouts;
  cyg_handle_t pthread;
};

static struct sys_thread *Threads_List = NULL;

sys_prot_t sys_arch_protect()
{
	sys_prot_t  old_intr;
	HAL_DISABLE_INTERRUPTS(old_intr);
	return old_intr;
}

void sys_arch_unprotect(sys_prot_t lev)
{
	HAL_RESTORE_INTERRUPTS(lev)	;
}

sys_mbox_t 
sys_mbox_new()
{
	struct sys_mbox * mailbox;
	cyg_handle_t * handle;
	cyg_mbox * mbox;
	
	mailbox = malloc(sizeof(struct sys_mbox));
	handle = malloc(sizeof(cyg_handle_t));
	mbox = malloc(sizeof(cyg_mbox));
	
	memset(mailbox, 0x00, sizeof(struct sys_mbox));
	memset(handle, 0x00, sizeof(cyg_handle_t));
	memset(mbox, 0x00, sizeof(cyg_mbox));
	
	cyg_mbox_create(handle, mbox);
	
	mailbox->handle = handle;
	mailbox->mbox = mbox;
	
	return mailbox;
}

void sys_mbox_free(sys_mbox_t mbox)
{
	cyg_handle_t * handle = mbox->handle;
	cyg_mbox * mailbox = mbox->mbox;
	
	cyg_mbox_delete(mailbox);
	
	free(handle);
	free(mailbox);
	free(mbox);
}

void sys_mbox_post(sys_mbox_t mbox, void *msg)
{
	cyg_mbox * mailbox = mbox->mbox;
	
	cyg_mbox_put(mailbox, msg);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
	cyg_mbox * mailbox = mbox->mbox;
	u32_t wtime = 1;
	
	if(timeout != 0)
	{
		*msg = cyg_mbox_timed_get(mailbox, cyg_current_time() + (timeout / MSPTICK));
		
		if (*msg == NULL)
		{
			wtime = SYS_ARCH_TIMEOUT;
		}
		else
		{
			wtime = 0;
		}
	}
	else
	{	
		*msg = cyg_mbox_get(mailbox);
	}
	
	return wtime;
}

sys_sem_t sys_sem_new(u8_t count)
{
	cyg_sem_t * sem;
	cyg_ucount32 val = count;
	
	sem = malloc (sizeof(cyg_sem_t));
	memset(sem, 0x00, sizeof(cyg_sem_t));
	
	cyg_semaphore_init(sem, val);
	
	return sem;
}

void sys_sem_signal(sys_sem_t sem)
{
	cyg_semaphore_post(sem);
}

void sys_sem_free(sys_sem_t sem)
{
	cyg_semaphore_destroy(sem);	
	free(sem);
}

u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
	u32_t wtime = 1, ErrTmp = 0;
	
	if(timeout != 0)
	{
		ErrTmp = cyg_semaphore_timed_wait(sem, cyg_current_time() + (timeout / MSPTICK));
		
		if(ErrTmp == 0)
		{
			wtime = SYS_ARCH_TIMEOUT;
		}
		else
		{
			wtime = 0;
		}
	}
	else	
	{
		cyg_semaphore_wait(sem);
	}
	return wtime;
}

void
sys_init(void)
{
//do nothing.
}
static struct sys_thread *
current_thread(void)
{
  struct sys_thread *st;
  cyg_handle_t pt;
  pt = cyg_thread_self();
  /*  DEBUGF("sys: current_thread: pt %d\n", pt);*/
  for(st = Threads_List; st != NULL; st = st->next) {
    /*    DEBUGF("sys: current_thread: st->pthread %d\n", st->pthread);*/
    if(st->pthread == pt) {
      return st;
    }
  }
  
  return NULL;	//Not find

}
struct sys_timeouts *
sys_arch_timeouts(void)
{
  struct sys_thread *pthread;
  
  pthread = current_thread();
  
  if (pthread == NULL)
  	return NULL;
  else
  	return &pthread->timeouts;  
}

void
sys_arch_thread_new(cyg_handle_t handle)
{
  struct sys_thread *pthread;
  
  if(sys_arch_timeouts() == NULL)
  	{
	  pthread = malloc(sizeof(struct sys_thread));
	  pthread->next = Threads_List;
	  pthread->timeouts.next = NULL;
	  pthread->pthread = handle;
	  Threads_List = pthread;
	}
}

//TCPIP Task create information definition
#define TCPIP_TASK_PRI         6	//ZOT716u2
#define TCPIP_TASK_STACK_SIZE  8192 //ZOT716u2 10240
static u8_t 			TCPIP_Stack[TCPIP_TASK_STACK_SIZE];
static cyg_thread       TCPIP_Task;
static cyg_handle_t     TCPIP_TaskHdl;

sys_thread_t sys_thread_new(void (* thread)(void *arg), void *arg, int prio)
{
	//Create TCPIP Thread
    cyg_thread_create(TCPIP_TASK_PRI,
                  thread,
                  arg,
                  "TCP/IP",
                  (void *) (TCPIP_Stack),
                  TCPIP_TASK_STACK_SIZE,
                  &TCPIP_TaskHdl,
                  &TCPIP_Task);
	
	//Create sys_thread
	sys_arch_thread_new(TCPIP_TaskHdl);
	
	//Start TCPIP Thread
	cyg_thread_resume(TCPIP_TaskHdl);
}