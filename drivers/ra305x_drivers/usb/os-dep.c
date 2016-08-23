#include "os-dep.h"
#include "asm/irqflags.h"

void usb_test()
{
    unsigned long flags;
    int i = 0;
    
    raw_local_irq_save(flags);   
    i++;
    raw_local_irq_restore(flags);

} /* usb_test */
