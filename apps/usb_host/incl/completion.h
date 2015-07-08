#ifndef __LINUX_COMPLETION_H
#define __LINUX_COMPLETION_H

/*
 * (C) Copyright 2001 Linus Torvalds
 *
 * Atomic wait-for-completion handler data structures.
 * See kernel/sched.c for details.
 */

struct __wait_queue_head {
    spinlock_t              lock;
    cyg_sem_t               semaphore;
    struct list_head        task_list;
    int                     test;
};

/*
typedef unsigned int	wait_queue_head_t;
*/
typedef struct __wait_queue_head wait_queue_head_t;


struct completion {
	unsigned int done;
	wait_queue_head_t wait;
};

#define COMPLETION_INITIALIZER(work) \
	{ 0, __WAIT_QUEUE_HEAD_INITIALIZER((work).wait) }

#define DECLARE_COMPLETION(work) \
	struct completion work = COMPLETION_INITIALIZER(work)

#if 0
static inline void init_completion(struct completion *x)
{
	x->done = 0;
	init_waitqueue_head(&x->wait);
}

#define INIT_COMPLETION(x)	((x).done = 0)
#endif

#endif
