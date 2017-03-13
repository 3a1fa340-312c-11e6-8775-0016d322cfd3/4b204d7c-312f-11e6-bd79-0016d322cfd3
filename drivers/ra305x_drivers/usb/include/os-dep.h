
#ifndef _OS_DEP_H_
#define _OS_DEP_H_

#include <cyg/infra/cyg_ass.h>
#include "list.h"
#include "klist.h"
#include "errno.h"

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
#define smp_mb()                __asm__ __volatile__("sync" : : :"memory")
#define smp_mb__before_llsc()   smp_llsc_mb()
#define jiffies                 cyg_current_time()

#define module_param(name, type, perm)
#define MODULE_PARM_DESC(_parm, desc)

/*
 * platform resource
 */
#define IRQ_RT3XXX_USB         18
#define MEM_EHCI_START         0x101c0000
#define MEM_EHCI_END           0x101c0fff
#define MEM_OHCI_START         0x101c1000
#define MEM_OHCI_END           0x101c1fff

#define THIS_MODULE ((struct module *)0)

/* 
 * system spinlocks
 */
#define spin_lock_t                     cyg_spin_lock_t
static int irq_flag;
#define spin_lock_init(x)               cyg_spinlock_init(x, false)
#define spin_lock(x)                    cyg_spinlock_spin(x)
#define spin_unlock(x)                  cyg_spinlock_clear(x)
#define spin_lock_irq(x)                cyg_spinlock_spin_intsave(x, &irq_flag)
#define spin_unlock_irq(x)              cyg_spinlock_clear_intsave(x, irq_flag)
#define spin_lock_irqsave(x, y)         cyg_spinlock_spin_intsave(x, &y)
#define spin_unlock_irqrestore(x, y)    cyg_spinlock_clear_intsave(x, y)

#define mutex_init(x)                   cyg_mutex_init(x)
#define mutex_lock(x)                   cyg_mutex_lock(x)
#define mutex_unlock(x)                 cyg_mutex_unlock(x)
#define mutex_trylock(x)                cyg_mutex_trylock(x)

#ifndef OS_HZ
#define OS_HZ                       100
#endif /* !OS_HZ */

#define HZ            OS_HZ
#define BUG_ON(x)     CYG_ASSERTC(!(x)) 

#define GFP_KERNEL    0
#define GFP_NOIO      ((gfp_t)0x10u)
#define GFP_ATOMIC    ((gfp_t)0x20u)

#ifdef USB_DBG
#define printk        diag_printf
#define pr_debug      diag_printf
#define pr_info       diag_printf
#define TTRACE        \
    diag_printf("%s(%d)\n", __func__, __LINE__)
#define EPDBG         \
    diag_printf("EPDBG: %s(%d)\n", __func__, __LINE__)
#define TADDR(fmt, addr) diag_printf("%s(%d) %s:%x\n", __func__, __LINE__, fmt, (u32)addr)
#define TVAL(fmt, val) diag_printf("%s(%d) %s:%d\n", __func__, __LINE__, fmt, val)
#define ehci_dbg(ehci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ehci_err(ehci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ehci_info(ehci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ehci_warn(ehci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ohci_dbg(ohci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ohci_err(ohci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ohci_info(ohci, fmt, args...) \
    diag_printf(fmt, ##args)
#define ohci_warn(ohci, fmt, args...) \
    diag_printf(fmt, ##args)
#else /* USB_DBG */

#define printk
#define pr_debug
#define pr_info 
#define TTRACE do{}while(0)
#define EPDBG do{}while(0)
#define TADDR(fmt, addr)
#define TVAL(fmt, val)
#define ehci_dbg(ehci, fmt, args...) \
    do{}while(0) 
#define ehci_err(ehci, fmt, args...) \
    do{}while(0)
#define ehci_info(ehci, fmt, args...) \
    do{}while(0)
#define ehci_warn(ehci, fmt, args...) \
    do{}while(0)
#define ohci_dbg(ohci, fmt, args...) \
    do{}while(0) 
#define ohci_err(ohci, fmt, args...) \
    do{}while(0)
#define ohci_info(ohci, fmt, args...) \
    do{}while(0)
#define ohci_warn(ohci, fmt, args...) \
    do{}while(0)
#endif /* USB_DBG */

#define ehci_vdbg(ehci, fmt, args...)
#define ohci_vdbg(ohci, fmt, args...)
#define dbg_port(ehci, label, port, status)
#define dbg_port(ohci, label, port, status)

#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))

#define BIT(nr)			(1UL << (nr))
#define BITS_PER_LONG 32
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define kstrdup(x,y)        strdup(x)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
// #define ehci_hcd_init       rt3xxx_ehci_probe

// #define kzalloc(x,y)        calloc(x, 1)
// #define kcalloc(x,y,z)      calloc(x, y)

#define KBUILD_MODNAME    "usb-host"
#define __always_inline inline

/*
 * types.h
 */

typedef signed char             s8;
typedef unsigned char           u8;

typedef signed short            s16;
typedef unsigned short          u16;

typedef signed int              s32;
typedef unsigned int            u32;

typedef signed long long        s64;
typedef unsigned long long      u64;

typedef __signed__ char         __s8;
typedef unsigned char           __u8;

typedef __signed__ short        __s16;
typedef unsigned short          __u16;

typedef __signed__ int          __s32;
typedef unsigned int            __u32;

typedef signed long long        __s64;
typedef unsigned long long      __u64;

typedef int                     ssize_t;
#define u_long    unsigned long

#define __bitwise
#define __bitwise__
typedef __u16 __bitwise         __le16;
typedef __u16 __bitwise         __be16;
typedef __u32 __bitwise         __le32;
typedef __u32 __bitwise         __be32;
typedef __u64 __bitwise         __le64;
typedef __u64 __bitwise         __be64;

typedef __u16 __bitwise         __sum16;
typedef __u32 __bitwise         __wsum;

typedef unsigned __bitwise__    gfp_t;
typedef unsigned __bitwise__    fmode_t;

typedef unsigned int            mode_t;
typedef u32                     dma_addr_t;

typedef long                    __kernel_time_t;
typedef long                    __kernel_suseconds_t;
typedef long                    suseconds_t;
typedef long                    time_t;
typedef long long               loff_t;

/* Arbitrary Unicode character */
typedef u32                     unicode_t;

enum dma_data_direction {
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
};

typedef struct {
	unsigned sequence;
	// spinlock_t lock;
    cyg_spinlock_t  lock;
} seqlock_t;

/*
 * NULL descriptor
 */
#define EXPORT_SYMBOL_GPL(x)
#ifdef USB_DBG
#define dev_dbg(dev, format, arg...) diag_printf(format, ##arg)
#define dev_warn(dev, format, arg...) diag_printf(format, ##arg)
#define dev_err(dev, format, arg...) diag_printf(format, ##arg)
#define dev_info(dev, format, arg...) diag_printf(format, ##arg)
#define dbg_status(ehci, label, status) diag_printf("ehci: %s:%x\n", label, status)
#else /* USB_DBG */
#define dev_dbg(dev, format, arg...) 
#define dev_warn(dev, format, arg...) 
#define dev_err(dev, format, arg...) 
#define dev_info(dev, format, arg...) 
#define dbg_status(ehci, label, status) 
#endif /* USB_DBG */

#define dbg_cmd(ehci, label, command)
#define __devinit

#define KERN_WARNING
#define KERN_DEBUG
#define KERN_ERR
#define KERN_EMERG
#define KERN_INFO 
#define WARN_ON
#define WARN_ONCE
#define __iomem
#define __init
#define __exit
#define __read_mostly
#define __force
#define __attribute_const__	
#define __user

#define might_sleep() do{} while(0)
#define bus_get(x)    x
#define bus_put(x)    do{} while(0)
#define get_device(x) x
#define put_device(x)
#define prefetch(x) __builtin_prefetch(x)
static inline void cleanup_device_parent(struct device *dev) {}

static inline int dev_to_node(struct device *dev)
{
	return -1;
}
static inline void set_dev_node(struct device *dev, int node)
{
}

static inline int device_is_registered(struct device *dev)
{
    return (int)dev;
}

/*
 * list
 */
#define LIST_POISON1    ((void *) 0x00100100)
#define LIST_POISON2    ((void *) 0x00200200)


/*
 * wait queue
 */

typedef struct __wait_queue wait_queue_t;
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int flags, void *key);
int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key);

struct __wait_queue {
    unsigned int flags;
    #define WQ_FLAG_EXCLUSIVE   0x01
    void *private;
    wait_queue_func_t func;
    struct list_head task_list;
    cyg_sem_t   wait_sem;
};

struct __wait_queue_head {
    cyg_spinlock_t  lock;
    cyg_sem_t       semaphore;
    struct list_head task_list;
};

typedef struct __wait_queue_head wait_queue_head_t;
int default_wake_function(wait_queue_t *wait, unsigned mode, int flags, void *key);
void finish_wait(wait_queue_head_t *q, wait_queue_t *wait);

#define wake_up(x)  __wake_up(x, 3, 1, NULL)
#define init_waitqueue_head __init_waitqueue_head

#define __WAIT_QUEUE_HEAD_INITIALIZER(name) { \
    .task_list = { &(name).task_list, &(name).task_list } }

#define DECLARE_WAIT_QUEUE_HEAD(name) \
    wait_queue_head_t name = __WAIT_QUEUE_HEAD_INITIALIZER(name)

#define __WAIT_QUEUE_INITIALIZER(name) { \
    .private = cyg_thread_self(), \
    .func = default_wake_function, \
    .task_list = {NULL, NULL} \
}
#define DECLARE_WAITQUEUE(name) \
    wait_queue_t name = __WAIT_QUEUE_INITIALIZER(name)

#define DEFINE_WAIT_FUNC(name, function)                        \
    wait_queue_t name = {                                       \
        .private = cyg_thread_self(),                           \
        .func    = function,                                    \
        .task_list = LIST_HEAD_INIT((name).task_list),          \
    }

#define DEFINE_WAIT(name) DEFINE_WAIT_FUNC(name, autoremove_wake_function)

static inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
    list_add(&new->task_list, &head->task_list);
}

#define __wait_event(wq, condition) \
do {                                \
    DEFINE_WAIT(__wait);            \
                                    \
    for (;;) {                      \
        prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);    \
        if (condition)                                          \
            break;                                              \
        cyg_semaphore_wait(&wq.semaphore);                      \
    }                                                           \
    finish_wait(&wq, &__wait);                                  \    
} while (0)

/**
 * wait_event - sleep until a condition gets true
 * @wq: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 */
#define wait_event(wq, condition)   \
do {                                \
    if (condition)                  \
        break;                      \
    __wait_event(wq, condition);    \
} while(0)

#define __wait_event_timeout(wq, condition, ret)			\
do {									                    \
	DEFINE_WAIT(__wait);						            \
									                        \
	for (;;) {							                    \
		prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);\
		if (condition)						\
			break;						    \
        cyg_semaphore_timed_wait(&wq.semaphore, ret); \
		break;						        \
	}								        \
	finish_wait(&wq, &__wait);				\
} while (0)

/**
 * wait_event_timeout - sleep until a condition gets true or a timeout elapses
 * @wq: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 * @timeout: timeout, in jiffies
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function returns 0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 */
#define wait_event_timeout(wq, condition, timeout)			\
({									\
	long __ret = timeout;						\
	if (!(condition)) 						\
		__wait_event_timeout(wq, condition, __ret);		\
	__ret;								\
})


/*
 * ulitility define
 */
#define DEFINE_MUTEX(mutexname) \
    cyg_mutex_t mutexname;

#define DEFINE_SPINLOCK(lockname) \
    cyg_spinlock_t lockname;

#define __NEW_UTS_LEN       64

#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })

//#define unlikely(cond)  (cond)
# define unlikely(x)	__builtin_expect(!!(x), 0)
# define likely(x)	__builtin_expect(!!(x), 1)

extern u32 __div64_32(u64 *dividend, u32 divisor);

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
# define do_div(n,base) ({				\
	u32 __base = (base);			\
	u32 __rem;					\
	(void)(((typeof((n)) *)0) == ((u64 *)0));	\
	if (likely(((n) >> 32) == 0)) {			\
		__rem = (u32)(n) % __base;		\
		(n) = (u32)(n) / __base;		\
	} else 						\
		__rem = __div64_32(&(n), __base);	\
	__rem;						\
 })

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})

/*
 *	These inlines deal with timer wrapping correctly. You are 
 *	strongly encouraged to use them
 *	1. Because people otherwise forget
 *	2. Because if the timer wrap changes in future you won't have to
 *	   alter your driver code.
 *
 * time_after(a,b) returns true if the time a is after time b.
 *
 * Do this with "<0" and ">=0" to only test the sign of the result. A
 * good compiler would generate better code (and a really good compiler
 * wouldn't care). Gcc is currently neither.
 */
#define time_after(a,b)		\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(b) - (long)(a) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(a) - (long)(b) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define to_device_private_bus(obj)	\
	container_of(obj, struct device_private, knode_bus)

/*
 * unaligned
 */
#include "le_struct.h"
#include "be_byteshift.h"
#define get_unaligned __get_unaligned_le
#define put_unaligned __put_unaligned_le

/*
 * MIPS IO space
 */
// #define mem_map     0x81000000
#define __AC(X,Y)   (X##Y)
#define _AC(X,Y)    __AC(X,Y)

#define PHYS_OFFSET     _AC(0, UL)
#define CAC_BASE        _AC(0x80000000, UL)
// #define IO_BASE         _AC(0xa0000000, UL)
#define UNCAC_BASE      _AC(0xa0000000, UL)

#ifndef PAGE_OFFSET
#define PAGE_OFFSET     (CAC_BASE + PHYS_OFFSET)
#endif /* PAGE_OFFSET */
#define PAGE_SHIFT      12 /* CONFIG_PAGE_SIZE_4KB */
#define PAGE_SIZE       (_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK       (~((1 << PAGE_SHIFT) - 1))
#define PFN_UP(x)       (((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
#define ARCH_PFN_OFFSET PFN_UP(PHYS_OFFSET)
#define UNCAC_ADDR(addr)    ((addr) - PAGE_OFFSET + UNCAC_BASE + PHYS_OFFSET)

#define kmalloc(x, y)   kaligned_alloc(x, 0x20)
#define kfree(x)        kaligned_free(x)
#define copy_from_user  memcpy

/*
 *     virt_to_phys    -       map virtual addresses to physical
 *     @address: address to remap
 *
 *     The returned physical address is the physical (CPU) mapping for
 *     the memory address given. It is only valid to use this function on
 *     addresses directly mapped or allocated via kmalloc.
 *
 *     This function does not give bus mappings for DMA transfers. In
 *     almost all conceivable cases a device driver should not be using
 *     this function
 */
static inline unsigned long virt_to_phys(volatile const void *address)
{
	return (unsigned long)address - PAGE_OFFSET + PHYS_OFFSET;
}

struct device;
static inline dma_addr_t plat_map_dma_mem(struct device *dev, void *addr, size_t size)
{
    return virt_to_phys(addr);
}

// static inline dma_addr_t plat_map_dma_mem_page(struct device *dev, struct page *page)
// {
//     return page_to_phys(page);
// }

static inline unsigned long plat_dma_addr_to_phys(struct device *dev, dma_addr_t dma_addr)
{
    return dma_addr;
}

static inline void plat_unmap_dma_mem(struct device *dev, dma_addr_t dma_addr, 
    size_t size, enum dma_data_direction direction)
{

}

// #define __pfn_to_page(pfn)	(mem_map + ((pfn) - ARCH_PFN_OFFSET))
// #define __page_to_pfn(page)	((unsigned long)((page) - mem_map) + \
//                  ARCH_PFN_OFFSET)
/*
 * Change "struct page" to physical address.
 */
// #define page_to_phys(page)	((dma_addr_t)__page_to_pfn(page) << PAGE_SHIFT)

/*
 *     phys_to_virt    -       map physical address to virtual
 *     @address: address to remap
 *
 *     The returned virtual address is a current CPU mapping for
 *     the memory address given. It is only valid to use this function on
 *     addresses that have a kernel mapping
 *
 *     This function does not handle bus mappings for DMA transfers. In
 *     almost all conceivable cases a device driver should not be using
 *     this function
 */
static inline void * phys_to_virt(unsigned long address)
{
	return (void *)(address + PAGE_OFFSET - PHYS_OFFSET);
}

static inline dma_addr_t dma_map_single(struct device *dev, void *ptr, size_t size,
    enum dma_data_direction direction)
{
    // __dma_sync(virt_to_page(ptr),
    //         (unsigned long)ptr & ~PAGE_MASK, size, direction);
    __dma_sync_virtual(ptr, size, direction);
    return plat_map_dma_mem(dev, ptr, size);
}

#define __sync()				\
	__asm__ __volatile__(			\
		".set	push\n\t"		\
		".set	noreorder\n\t"		\
		".set	mips2\n\t"		\
		"sync\n\t"			\
		".set	pop"			\
		: /* no output */		\
		: /* no input */		\
		: "memory")
#define wmb()   __sync()
#define rmb()   __sync()

// struct new_utsname {
//     char sysname[__NEW_UTS_LEN + 1];
//     char nodename[__NEW_UTS_LEN + 1];
//     char release[__NEW_UTS_LEN +1];
//     char version[__NEW_UTS_LEN + 1];
//     char machine[__NEW_UTS_LEN + 1];
//     char domainname[__NEW_UTS_LEN + 1];
// };
// 
// static inline struct new_utsname *init_utsname(void)
// {
//     return &init_uts_ns.name
// }

// typedef struct {
//     int counter;
// } atomic_t;
// typedef atomic_t atomic_long_t;

// struct kref {
//     atomic_t refcount;
// };

// struct list_head {
//     struct list_head *next, *prev;
// };
// 
// struct hlist_head {
//     struct hlist_node *first;
// };
// 
// struct hlist_node {
//     struct hlist_node *next, **pprev;
// };


struct timer_list {
	struct list_head entry;
	unsigned long expires;
    /*
    sturct tvec_base *base;
    */

	void (*function)(cyg_addrword_t);
	unsigned long data;

    int slack;
    cyg_handle_t    counter_hdl;
    cyg_handle_t    alarm_hdl;
    cyg_alarm       alarm_obj;
    cyg_spinlock_t  lock;
    bool            valid;
    bool            pending;
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

extern void init_timer(struct timer_list *timer);
extern void add_timer_on(struct timer_list *timer, int cpu);
extern void add_timer(struct timer_list *timer);
extern int del_timer(struct timer_list *timer);
extern int mod_timer(struct timer_list *timer, unsigned long expires);
extern int mod_timer_pending(struct timer_list *timer, unsigned long expires);
extern int mod_timer_pinned(struct timer_list *timer, unsigned long expires);
# define del_timer_sync(t)		del_timer(t)

/*
 * complete.h
 */

struct usb_completion {
    unsigned int done;
    wait_queue_head_t wait;
};

extern void usb_init_completion(struct usb_completion *x);
extern void usb_wait_for_completion(struct usb_completion *);
extern unsigned long usb_wait_for_completion_timeout(struct usb_completion *x,
						   unsigned long timeout);
extern void usb_complete(struct usb_completion *);

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

/*
 * device.h
 */
#define platform_get_drvdata(_dev) dev_get_drvdata(&(_dev)->dev)

struct kobject {
	const char		*name;
	struct list_head	entry;
	struct kobject		*parent;
	// struct kset		*kset;
	// struct kobj_type	*ktype;
	// struct sysfs_dirent	*sd;
	// struct kref		kref;
	unsigned int state_initialized:1;
	unsigned int state_in_sysfs:1;
	unsigned int state_add_uevent_sent:1;
	unsigned int state_remove_uevent_sent:1;
	unsigned int uevent_suppress:1;
};

struct bus_type_private {
	// struct kset subsys;
	// struct kset *drivers_kset;
	// struct kset *devices_kset;
    struct klist klist_devices;
    struct klist klist_drivers;
	// struct blocking_notifier_head bus_notifier;
	unsigned int drivers_autoprobe:1;
	struct bus_type *bus;
};

struct driver_private {
	struct kobject kobj;
    struct klist klist_devices;
    struct klist_node knode_bus;
	// struct module_kobject *mkobj;
	struct device_driver *driver;
};
#define to_driver(obj) container_of(obj, struct driver_private, kobj)

typedef struct pm_message {
	int event;
} pm_message_t;

struct attribute {
	const char		*name;
	mode_t			mode;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lock_class_key	*key;
	struct lock_class_key	skey;
#endif
};

/* interface for exporting device attributes */
struct device_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct device *dev, struct device_attribute *attr,
			char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count);
};

#define DEVICE_ATTR(_name, _mode, _show, _store) \
struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, _show, _store)

struct bus_type {
	const char		*name;
	// struct bus_attribute	*bus_attrs;
	// struct device_attribute	*dev_attrs;
	// struct driver_attribute	*drv_attrs;

	int (*match)(struct device *dev, struct device_driver *drv);
	// int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
	void (*shutdown)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);

	const struct dev_pm_ops *pm;

	struct bus_type_private *p;
};

struct device_driver {
	const char		*name;
	struct bus_type		*bus;

	struct module		*owner;
	const char		*mod_name;	/* used for built-in modules */

	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */

#if defined(CONFIG_OF)
	const struct of_device_id	*of_match_table;
#endif

	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
	void (*shutdown) (struct device *dev);
	int (*suspend) (struct device *dev, pm_message_t state);
	int (*resume) (struct device *dev);
	const struct attribute_group **groups;

	const struct dev_pm_ops *pm;

	struct driver_private *p;
};

enum dpm_state {
	DPM_INVALID,
	DPM_ON,
	DPM_PREPARING,
	DPM_RESUMING,
	DPM_SUSPENDING,
	DPM_OFF,
	DPM_OFF_IRQ,
};

struct dev_pm_info {
	pm_message_t		power_state;
	unsigned int		can_wakeup:1;
	unsigned int		should_wakeup:1;
	unsigned		async_suspend:1;
	enum dpm_state		status;		/* Owned by the PM core */
#ifdef CONFIG_PM_SLEEP
	struct list_head	entry;
	struct usb_completion	completion;
	unsigned long		wakeup_count;
#endif
#ifdef CONFIG_PM_RUNTIME
	struct timer_list	suspend_timer;
	unsigned long		timer_expires;
	struct work_struct	work;
	wait_queue_head_t	wait_queue;
	// spinlock_t		lock;
    cyg_spinlock_t      lock;
	atomic_t		usage_count;
	atomic_t		child_count;
	unsigned int		disable_depth:3;
	unsigned int		ignore_children:1;
	unsigned int		idle_notification:1;
	unsigned int		request_pending:1;
	unsigned int		deferred_resume:1;
	unsigned int		run_wake:1;
	unsigned int		runtime_auto:1;
	enum rpm_request	request;
	enum rpm_status		runtime_status;
	int			runtime_error;
	unsigned long		active_jiffies;
	unsigned long		suspended_jiffies;
	unsigned long		accounting_timestamp;
#endif
};

/**
 * struct device_private - structure to hold the private to the driver core portions of the device structure.
 *
 * @klist_children - klist containing all children of this device
 * @knode_parent - node in sibling list
 * @knode_driver - node in driver list
 * @knode_bus - node in bus list
 * @driver_data - private pointer for driver specific info.  Will turn into a
 * list soon.
 * @device - pointer back to the struct class that this structure is
 * associated with.
 *
 * Nothing outside of the driver core should ever touch these fields.
 */
struct device_private {
	struct klist klist_children;
	struct klist_node knode_parent;
	struct klist_node knode_driver;
	struct klist_node knode_bus;
	void *driver_data;
	struct device *device;
};

#define to_device_private_parent(obj)	\
	container_of(obj, struct device_private, knode_parent)
#define to_device_private_driver(obj)	\
	container_of(obj, struct device_private, knode_driver)
#define to_device_private_bus(obj)	\
	container_of(obj, struct device_private, knode_bus)

struct class_private {
	// struct kset class_subsys;
	struct klist class_devices;
	struct list_head class_interfaces;
	// struct kset class_dirs;
	// struct mutex class_mutex;
    cyg_mutex_t     class_mutex;
	struct class *class;
};
#define to_class(obj)	\
	container_of(obj, struct class_private, class_subsys.kobj)

/*
 * device classes
 */
struct class {
	const char		*name;
	struct module		*owner;

	// struct class_attribute		*class_attrs;
	// struct device_attribute		*dev_attrs;
	// struct kobject			*dev_kobj;

	// int (*dev_uevent)(struct device *dev, struct kobj_uevent_env *env);
	// char *(*devnode)(struct device *dev, mode_t *mode);

	void (*class_release)(struct class *class);
	void (*dev_release)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);

	// const struct kobj_ns_type_operations *ns_type;
	const void *(*namespace)(struct device *dev);

	// const struct dev_pm_ops *pm;

	struct class_private *p;
};

struct class_interface {
	struct list_head	node;
	struct class		*class;

	int (*add_dev)		(struct device *, struct class_interface *);
	void (*remove_dev)	(struct device *, struct class_interface *);
};

struct device {
	struct device		*parent;

	struct device_private	*p;

	// struct kobject kobj;
	const char		*init_name; /* initial name of the device */
	struct device_type	*type;

	// struct mutex		mutex;	[> mutex to synchronize calls to
	//                  * its driver.
	//                  */
    cyg_mutex_t         mutex;

	struct bus_type	*bus;		    /* type of bus device is on */
	struct device_driver *driver;	/* which driver has allocated this device */
	void   *platform_data;	        /* Platform specific data, device core doesn't touch it */
	struct dev_pm_info	power;

#ifdef CONFIG_NUMA
	int		numa_node;	            /* NUMA node this device is close to */
#endif
	u64		*dma_mask;	            /* dma mask (if dma'able device) */
	u64		coherent_dma_mask;      /* Like dma_mask, but for
                                       alloc_coherent mappings as
                                       not all hardware supports
                                       64 bit addresses for consistent
                                       allocations such descriptors. */

	struct device_dma_parameters *dma_parms;

	struct list_head	dma_pools;	/* dma pools (if dma'ble) */

	struct dma_coherent_mem	*dma_mem; /* internal for coherent mem
                                         override */
    /* arch specific additions */
	// struct dev_archdata	archdata;
#ifdef CONFIG_OF
	struct device_node	*of_node;
#endif

	//dev_t			devt;	/* dev_t, creates the sysfs "dev" */

	// spinlock_t		    devres_lock;
    cyg_spinlock_t      devres_lock;
	struct list_head	devres_head;

	struct klist_node	knode_class;
	struct class		*class;
	const struct attribute_group **groups;	/* optional groups */

	void	(*release)(struct device *dev);
};

/*
 * The type of device, "struct device" is embedded in. A class
 * or bus can contain devices of different types
 * like "partitions" and "disks", "mouse" and "event".
 * This identifies the device type and carries type-specific
 * information, equivalent to the kobj_type of a kobject.
 * If "name" is specified, the uevent will contain it in
 * the DEVTYPE variable.
 */
struct device_type {
	const char *name;
	// const struct attribute_group **groups;
	// int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
	// char *(*devnode)(struct device *dev, mode_t *mode);
	void (*release)(struct device *dev);

	// const struct dev_pm_ops *pm;
};

struct platform_device {
    const char	* name;
    int		id;
    struct device	dev;
    u32		num_resources;
    struct resource	* resource;

    // const struct platform_device_id	*id_entry;

    /* arch specific additions */
    // struct pdev_archdata	archdata;
};
#define to_platform_device(x) container_of((x), struct platform_device, dev)

struct platform_driver {
    int (*probe)(struct platform_device *);
    int (*remove)(struct platform_device *);
    void (*shutdown)(struct platform_device *);
    // int (*suspend)(struct platform_device *, pm_message_t state);
    int (*resume)(struct platform_device *);
    struct device_driver driver;
    // const struct platform_device_id *id_table;
};
#define to_platform_driver(x) container_of((x), struct platform_driver, driver)

/*
 * pm_wakeup.h
 */

#ifdef CONFIG_PM

/* Changes to device_may_wakeup take effect on the next pm state change.
 *
 * By default, most devices should leave wakeup disabled.  The exceptions
 * are devices that everyone expects to be wakeup sources: keyboards,
 * power buttons, possibly network interfaces, etc.
 */
static inline void device_init_wakeup(struct device *dev, bool val)
{
	dev->power.can_wakeup = dev->power.should_wakeup = val;
}

static inline void device_set_wakeup_capable(struct device *dev, bool capable)
{
	dev->power.can_wakeup = capable;
}

static inline bool device_can_wakeup(struct device *dev)
{
	return dev->power.can_wakeup;
}

static inline void device_set_wakeup_enable(struct device *dev, bool enable)
{
	dev->power.should_wakeup = enable;
}

static inline bool device_may_wakeup(struct device *dev)
{
	return dev->power.can_wakeup && dev->power.should_wakeup;
}

#else /* !CONFIG_PM */

/* For some reason the following routines work even without CONFIG_PM */
static inline void device_init_wakeup(struct device *dev, bool val)
{
	dev->power.can_wakeup = val;
}

static inline void device_set_wakeup_capable(struct device *dev, bool capable)
{
	dev->power.can_wakeup = capable;
}

static inline bool device_can_wakeup(struct device *dev)
{
	return dev->power.can_wakeup;
}

static inline void device_set_wakeup_enable(struct device *dev, bool enable)
{
}

static inline bool device_may_wakeup(struct device *dev)
{
	return false;
}

static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	return *(const volatile u32 __force *) addr;
}
#define readl(addr) __le32_to_cpu(__raw_readl(addr))

static inline void __raw_writel(u32 b, volatile void __iomem *addr)
{
	*(volatile u32 __force *) addr = b;
}
#define writel(b,addr) __raw_writel(__cpu_to_le32(b),addr)

#endif /* !CONFIG_PM */

/*
 * usbdevice_fs
 */
/* You can do most things with hubs just through control messages,
 * except find out what device connects to what port. */
struct usbdevfs_hub_portinfo {
	char nports;		/* number of downstream ports in this hub */
	char port [127];	/* e.g. port 3 connects to device 27 */
};

#define USBDEVFS_HUB_PORTINFO      _IOR('U', 19, struct usbdevfs_hub_portinfo)

/*
 * workqueue.h
 */

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

/*
 * initialize a work item's function pointer
 */

struct work_struct {
    atomic_long_t data;
    struct list_head entry;
    work_func_t func;
#ifdef CONFIG_LOCKDEP
    struct lockdep_map lockdep_map;
#endif
    unsigned long delay;
};
 
struct delayed_work {
    struct work_struct work;
    struct timer_list timer;
};

static inline void __init_work(struct work_struct *work, int onstack)
{
}

static int cancel_delayed_work_sync(struct delayed_work *dwork){ return 0; }

#define PREPARE_WORK(_work, _func)				\
    do {							\
        (_work)->func = (_func);			\
    } while (0)

#define PREPARE_DELAYED_WORK(_work, _func)			\
    PREPARE_WORK(&(_work)->work, (_func))

#define __INIT_WORK(_work, _func, _onstack)              \
    do {                                                 \
        __init_work((_work), _onstack);                  \
        INIT_LIST_HEAD(&(_work)->entry);                 \
        PREPARE_WORK((_work), (_func));                  \
    } while (0)

#define INIT_WORK(_work, _func)           \
    do {                                  \
        __INIT_WORK((_work), (_func), 0); \
    } while (0)

#define INIT_DELAYED_WORK(_work, _func)				\
    do {							\
        INIT_WORK(&(_work)->work, (_func));		\
        init_timer(&(_work)->timer);			\
    } while (0)

int schedule_delayed_work(struct delayed_work *dwork,
                    unsigned long delay);

unsigned long find_last_bit(const unsigned long *addr, unsigned long size);
unsigned long msecs_to_jiffies(const unsigned int m);

/*
 * nlf_base.h
 */

/*
 * Sample implementation from Unicode home page.
 * http://www.stonehand.com/unicode/standard/fss-utf.html
 */
struct utf8_table {
	int     cmask;
	int     cval;
	int     shift;
	long    lmask;
	long    lval;
};

/* Byte order for UTF-16 strings */
enum utf16_endian {
	UTF16_HOST_ENDIAN,
	UTF16_LITTLE_ENDIAN,
	UTF16_BIG_ENDIAN
};

static const struct utf8_table utf8_table[] =
{
    {0x80,  0x00,   0*6,    0x7F,           0,         /* 1 byte sequence */},
    {0xE0,  0xC0,   1*6,    0x7FF,          0x80,      /* 2 byte sequence */},
    {0xF0,  0xE0,   2*6,    0xFFFF,         0x800,     /* 3 byte sequence */},
    {0xF8,  0xF0,   3*6,    0x1FFFFF,       0x10000,   /* 4 byte sequence */},
    {0xFC,  0xF8,   4*6,    0x3FFFFFF,      0x200000,  /* 5 byte sequence */},
    {0xFE,  0xFC,   5*6,    0x7FFFFFFF,     0x4000000, /* 6 byte sequence */},
    {0,						       /* end of table    */}
};

#define UNICODE_MAX	0x0010ffff
#define PLANE_SIZE	0x00010000

#define SURROGATE_MASK	0xfffff800
#define SURROGATE_PAIR	0x0000d800
#define SURROGATE_LOW	0x00000400
#define SURROGATE_BITS	0x000003ff

static inline int driver_match_device(struct device_driver *drv,
				      struct device *dev)
{
	return drv->bus->match ? drv->bus->match(dev, drv) : 1;
}

static inline void device_lock(struct device *dev)
{
	mutex_lock(&dev->mutex);
}

static inline void device_unlock(struct device *dev)
{
	mutex_unlock(&dev->mutex);
}

static inline int device_trylock(struct device *dev)
{
	return mutex_trylock(&dev->mutex);
}

#define LONG_MAX                ((long)(~0UL >> 1))
#define MAX_SCHEDULE_TIMEOUT    LONG_MAX
#define INT_MAX                 ((int)(~0U >> 1))

/*
 * Task state bitmask. NOTE! These bits are also
 * encoded in fs/proc/array.c: get_task_state().
 *
 * We have two separate sets of flags: task->state
 * is about runnability, while task->exit_state are
 * about the task exiting. Confusing, but this way
 * modifying one set can't modify the other one by
 * mistake.
 */
#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define __TASK_STOPPED          4
#define __TASK_TRACED           8
/* in tsk->exit_state */
#define EXIT_ZOMBIE             16
#define EXIT_DEAD               32
/* in tsk->state again */
#define TASK_DEAD               64
#define TASK_WAKEKILL           128
#define TASK_WAKING             256
#define TASK_STATE_MAX          512

/*
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.
 */
#define atomic_read(v)    (*(volatile int *)&(v)->counter)

/*
 * These flags used only by the kernel as part of the
 * irq handling routines.
 *
 * IRQF_DISABLED - keep irqs disabled when calling the action handler.
 *                 DEPRECATED. This flag is a NOOP and scheduled to be removed
 * IRQF_SAMPLE_RANDOM - irq is used to feed the random generator
 * IRQF_SHARED - allow sharing the irq among several devices
 * IRQF_PROBE_SHARED - set by callers when they expect sharing mismatches to occur
 * IRQF_TIMER - Flag to mark this interrupt as timer interrupt
 * IRQF_PERCPU - Interrupt is per cpu
 * IRQF_NOBALANCING - Flag to exclude this interrupt from irq balancing
 * IRQF_IRQPOLL - Interrupt is used for polling (only the interrupt that is
 *                registered first in an shared interrupt is considered for
 *                performance reasons)
 * IRQF_ONESHOT - Interrupt is not reenabled after the hardirq handler finished.
 *                Used by threaded interrupts which need to keep the
 *                irq line disabled until the threaded handler has been run.
 * IRQF_NO_SUSPEND - Do not disable this IRQ during suspend
 *
 */
#define IRQF_DISABLED         0x00000020
#define IRQF_SAMPLE_RANDOM    0x00000040
#define IRQF_SHARED           0x00000080
#define IRQF_PROBE_SHARED     0x00000100
#define __IRQF_TIMER          0x00000200
#define IRQF_PERCPU           0x00000400
#define IRQF_NOBALANCING      0x00000800
#define IRQF_IRQPOLL          0x00001000
#define IRQF_ONESHOT          0x00002000
#define IRQF_NO_SUSPEND       0x00004000
#define IRQF_NOINT            0x00008000

# define __acquires(x)
# define __releases(x)
# define __acquire(x) (void)0
# define __release(x) (void)0

#define unreachable()   do{}while(1)
#define BRK_BUG		512	/* Used by BUG() */
static inline void  BUG(void)
{
	__asm__ __volatile__("break %0" : : "i" (BRK_BUG));
	unreachable();
}

static inline const char *dev_name(const struct device *dev)
{
	/* Use the init name until the kobject becomes available */
	if (dev->init_name)
		return dev->init_name;

	return NULL;
}

extern int usb_init(void);

#include "asm/irqflags.h"
#include "asm/bitops.h"
#include "generic.h"
#include "little_endian.h"
#endif /* end of include guard: _OS_DEP_H_ */
