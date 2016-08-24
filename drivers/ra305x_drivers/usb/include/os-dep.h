
#ifndef _OS_DEP_H_
#define _OS_DEP_H_

/*
 * cpu information
 */
#define CONFIG_CPU_MIPS32       1
#define CONFIG_CPU_MIPS32_R2    1
#define kernel_uses_llsc        1
#define BITS_PER_LONG           32
#define cpu_has_clo_clz         1
#define cpu_has_mips64          0
#define __WEAK_LLSC_MB          "       \n"
#define smp_llsc_mb()           __asm__ __volatile__(__WEAK_LLSC_MB : : :"memory")
#define smp_mb__before_llsc()   smp_llsc_mb()

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

typedef struct {
    int counter;
} atomic_t;

struct kref {
    atomic_t refcount;
};

struct list_head {
    struct list_head *next, *prev;
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
// bool test_and_set_bit(long nr, volatile unsigned long* addr)
// {
//
// }

/**
 * timer_pending - is a timer pending?
 * @timer: the timer in question
 *
 * timer_pending will tell whether a given timer is currently pending,
 * or not. Callers must ensure serialization wrt. other operations done
 * to this timer, eg. interrupt contexts, or other CPUs on SMP.
 *
 * return value: 1 if the timer is pending, 0 if not.
 */
static inline int timer_pending(const struct timer_list * timer)
{
	return timer->entry.next != NULL;
}

extern void add_timer_on(struct timer_list *timer, int cpu);
extern int del_timer(struct timer_list * timer);
extern int mod_timer(struct timer_list *timer, unsigned long expires);
extern int mod_timer_pending(struct timer_list *timer, unsigned long expires);
extern int mod_timer_pinned(struct timer_list *timer, unsigned long expires);

#if 0
/**
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * This function is atomic and may not be reordered.  See __set_bit()
 * if you do not require the atomic guarantees.
 *
 * Note: there are no guarantees that this function will not be reordered
 * on non x86 architectures, so if you are writing portable code,
 * make sure not to rely on its reordering guarantees.
 *
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void set_bit(int nr, volatile unsigned long *addr)
{

}

/**
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is atomic and may not be reordered.  However, it does
 * not contain a memory barrier, so if it is used for locking purposes,
 * you should call smp_mb__before_clear_bit() and/or smp_mb__after_clear_bit()
 * in order to ensure changes are visible on other processors.
 */
static inline void clear_bit(int nr, volatile unsigned long *addr)
{

}

/**
 * change_bit - Toggle a bit in memory
 * @nr: Bit to change
 * @addr: Address to start counting from
 *
 * change_bit() is atomic and may not be reordered. It may be
 * reordered on other architectures than x86.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void change_bit(int nr, volatile unsigned long *addr)
{

}

/**
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It may be reordered on other architectures than x86.
 * It also implies a memory barrier.
 */
static inline int test_and_set_bit(int nr, volatile unsigned long *addr)
{

}

/**
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It can be reorderdered on other architectures other than x86.
 * It also implies a memory barrier.
 */
static inline int test_and_clear_bit(int nr, volatile unsigned long *addr)
{

}

/**
 * test_and_change_bit - Change a bit and return its old value
 * @nr: Bit to change
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int test_and_change_bit(int nr, volatile unsigned long *addr)
{

}
#endif /* 0 */
#endif /* end of include guard: _OS_DEP_H_ */
