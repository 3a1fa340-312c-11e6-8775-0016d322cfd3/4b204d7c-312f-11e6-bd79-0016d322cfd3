
#ifndef _OS_DEP_H_
#define _OS_DEP_H_

#define module_param(name, type, perm)
#define MODULE_PARM_DESC(_parm, desc)

/* 
 * system spinlocks
 */
#define spin_lock_init(x)           cyg_spinlock_init(x, false)
#define spin_lock(x)                cyg_spinlock_spin(x)
#define spin_unlock(x)              cyg_spinlock_clear(x)
#define spin_lock_irqsave(x, y)     cyg_spinlock_spin_intsave(x, &y)
#define spin_unlock_irqstore(x, y)  cyg_spinlock_clear_intsave(x, y)

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

/*
 * os dependent fuction call
 */
bool test_and_set_bit(long nr, volatile unsigned long* addr)
{

}
#endif /* end of include guard: _OS_DEP_H_ */
