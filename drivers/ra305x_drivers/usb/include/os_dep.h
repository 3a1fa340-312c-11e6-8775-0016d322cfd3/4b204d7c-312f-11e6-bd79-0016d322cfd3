
#ifndef _OS_DEP_H_
#define _OS_DEP_H_

struct kref {
    atomic_t refcount;
};

struct timer_list {
	struct list_head entry;
	unsigned long expires;
    /*
    sturct tvec_base *base;
    */

	void (*function)(unsigned long);
	unsigned long data;

    int slack;
};

struct dma_pool {
};

struct list_head {
    struct list_head *next, *prev;
};

/**
 * enum irqreturn
 * @IRQ_NONE        interrupt was not from this device
 * @IRQ_HANDLED     interrupt was handled by this device
 * @IRQ_WAKE_THREAD handler requests to wake the handler thread
 */
enum irqreturn {
    IRQ_NONE,
    IRQ_HANDLED,
    IRQ_WAKE_THREAD,
};
typedef enum irqreturn irqreturn_t;

#endif /* end of include guard: _OS_DEP_H_ */
