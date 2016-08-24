#include "stddef.h"
#include "os-dep.h"
#include "asm/irqflags.h"
#include "asm/bitops.h"

void usb_test()
{
    unsigned long bitflags = 0;
    unsigned long flags, oldflags;
    int i = 0;
    
    raw_local_irq_save(flags);   
    i++;
    raw_local_irq_restore(flags);

    oldflags = test_and_set_bit(4, &bitflags);
    diag_printf("termy say, oldflags = %lu\n", oldflags);
    diag_printf("termy say, bitflags = %lu\n", bitflags);

} /* usb_test */

void add_timer_on(struct timer_list *timer, int cpu)
{

}

int del_timer(struct timer_list * timer)
{
    return 0;
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
    return 0;
}

int mod_timer_pending(struct timer_list *timer, unsigned long expires)
{
    return 0;
}

int mod_timer_pinned(struct timer_list *timer, unsigned long expires)
{
    return 0;
}

