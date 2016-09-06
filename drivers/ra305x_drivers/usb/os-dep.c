#include "stddef.h"
#include <cyg/kernel/kapi.h>
#include "os-dep.h"
#include "asm/irqflags.h"
#include "asm/bitops.h"
#include "list.h"


void alarm_callback(cyg_handle_t alarm, cyg_addrword_t data)
{
    static int iloop = 0;
    diag_printf("termy say, alarm_callback ! (%d)\n", iloop++);
}

cyg_handle_t    counter_hdl;
cyg_handle_t    alarm_hdl;
cyg_alarm       alarm_obj;

void usb_test()
{
    // unsigned long bitflags = 0;
    // unsigned long flags, oldflags;
    // int i = 0;
    // 
    // raw_local_irq_save(flags);   
    // i++;
    // raw_local_irq_restore(flags);
    // 
    // oldflags = test_and_set_bit(4, &bitflags);
    // diag_printf("termy say, oldflags = %lu\n", oldflags);
    // diag_printf("termy say, bitflags = %lu\n", bitflags);

    // cyg_handle_t    counter_hdl;
    // cyg_handle_t    alarm_hdl;
    // cyg_alarm       alarm_obj;
    CYG_ADDRWORD    alarm_data;
    unsigned long   alarm_timeout;
    unsigned long   counter_val;
    cyg_tick_count_t    trigger_val;
    cyg_tick_count_t    interval_val;
    struct list_head list_test;

    alarm_timeout = 1;
    
    cyg_clock_to_counter(cyg_real_time_clock(), &counter_hdl);
    cyg_alarm_create (counter_hdl, alarm_callback, 0, &alarm_hdl, &alarm_obj);
    diag_printf("termy say, cyg_hal_clock_period = %lu\n", cyg_hal_clock_period);
    diag_printf("termy say, RTC_PERIOD = %lu, RTC_DENOMINATOR = %lu\n", CYGNUM_HAL_RTC_PERIOD, CYGNUM_HAL_RTC_DENOMINATOR);
    diag_printf("termy say, RTC_NUMERATOR = %lu\n", CYGNUM_HAL_RTC_NUMERATOR);
    diag_printf("termy say, now tick = %llu\n", cyg_current_time());
    cyg_alarm_disable(alarm_hdl);
    cyg_alarm_initialize (alarm_hdl, cyg_current_time() + alarm_timeout, 0);
    cyg_alarm_enable(alarm_hdl);

    for (counter_val = 0; counter_val < 1000000000; ++counter_val) {
        
    }
    cyg_alarm_get_times(alarm_hdl, &trigger_val, &interval_val);
    diag_printf("termy say, trigget_val = %llu, now = %llu\n", trigger_val, cyg_current_time());
    diag_printf("termy say, interval_val = %llu\n", interval_val);

    INIT_LIST_HEAD(&list_test);
} /* usb_test */

void init_timer(struct timer_list *timer)
{
    CYG_ASSERT(timer != NULL, "timer is NULL!");
    if (timer->valid) {
        timer->entry.next = NULL;
        cyg_clock_to_counter (cyg_real_time_clock(), &timer->counter_hdl);
        cyg_alarm_create (timer->counter_hdl, 
                timer->function, 
                timer->data, 
                &timer->alarm_hdl, 
                &timer->alarm_obj);

        if (!timer->alarm_hdl)
            diag_printf("init_timer failed!\n");
        else
            timer->valid = 1;

    }
}

void add_timer(struct timer_list *timer)
{
    mod_timer(timer, timer->expires);
}

void add_timer_on(struct timer_list *timer, int cpu)
{
    mod_timer(timer, timer->expires);
}

int del_timer(struct timer_list *timer)
{
    CYG_ASSERT(timer != NULL, "del_timer: timer is NULL!");

    if (timer->valid && timer->alarm_hdl) {
        cyg_alarm_disable(timer->alarm_hdl);
        // cyg_alarm_delete(timer->alarm_hdl);
        // timer->alarm_hdl = NULL;
        // timer->valid = 0;
        return 1;
    }

    return 0;
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
    CYG_ASSERT(timer != NULL, "mod_timer: timer is NULL!");

    if (timer->valid && timer->alarm_hdl) {
        cyg_alarm_disable(timer->alarm_hdl);
        expires = (expires * OS_HZ) / 1000;
        if (expires > 0) {
            cyg_alarm_initialize(timer->alarm_hdl, cyg_current_timer() + expires, 0);
            cyg_alarm_enable(timer->alarm_hdl);
            return 1;
        }
        else
            diag_printf("mod_timer: expires < 0 !");
    }
    return 0;
}

int mod_timer_pending(struct timer_list *timer, unsigned long expires)
{
    return mod_timer(timer, expires);
}

int mod_timer_pinned(struct timer_list *timer, unsigned long expires)
{
    return mod_timer(timer, expires);
}

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
int timer_pending(struct timer_list *timer)
{
    cyg_tick_count_t    trigger, interval;

    if (timer != NULL && timer->alarm_hdl != NULL) {
        cyg_alarm_get_times(timer->alarm_hdl, &trigger, &interval);
        
        if ((interval != 0) || (trigger > cyg_current_timer()))
            return 1;
    }
    return 0;
}

/*
 * finish_wait - clean up after waiting in a queue
 * @q: waitqueue waited on
 * @wait: wait descriptor
 *
 * Sets current thread back to running state and removes
 * the wait descriptor from the given waitqueue if still
 * queued.
 */
void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
	unsigned long flags;

	// __set_current_state(TASK_RUNNING);
	/*
	 * We can check for list emptiness outside the lock
	 * IFF:
	 *  - we use the "careful" check that verifies both
	 *    the next and prev pointers, so that there cannot
	 *    be any half-pending updates in progress on other
	 *    CPU's that we haven't seen yet (and that might
	 *    still change the stack area.
	 * and
	 *  - all other users take the lock (ie we can only
	 *    have _one_ other CPU that looks at or modifies
	 *    the list).
	 */
	if (!list_empty_careful(&wait->task_list)) {
		spin_lock_irqsave(&q->lock, flags);
		list_del_init(&wait->task_list);
		spin_unlock_irqrestore(&q->lock, flags);
	}
}

// int default_wake_function(wait_queue_t *wait, unsigned mode, int flags, void *key)
// {
// 
// }
// 
// int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key)
// {
//     int ret = default_wake_function(wait, mode, sync, key);
// 
//     if (ret)
//         list_del_init(&wait->task_list);
//     return ret;
// }

void wake_up(wait_queue_head_t* q)
{
    unsigned long flags;
    wait_queue_t *curr, *next;

    spin_lock_irqsave(&q->lock, flags);

    list_for_each_entry_safe(curr, next, &q->task_list, task_list) {
        // if (curr->func(curr, 0, 1, 0, NULL) &&
        //         (cur->flags & WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
        //     break;
        list_del_init(&curr->task_list);
    }
    spin_lock_iqrrestore(&q->lock, flags);
}

/*
 * Note: we use "set_current_state()" _after_ the wait-queue add,
 * because we need a memory barrier there on SMP, so that any
 * wake-function that tests for the wait-queue being active
 * will be guaranteed to see waitqueue addition _or_ subsequent
 * tests in this thread will see the wakeup having taken place.
 *
 * The spin_unlock() itself is semi-permeable and only protects
 * one way (it only protects stuff inside the critical region and
 * stops them from bleeding out - it would still allow subsequent
 * loads to move into the critical region).
 */
void
prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
	unsigned long flags;

	wait->flags &= ~WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave(&q->lock, flags);
	if (list_empty(&wait->task_list))
		__add_wait_queue(q, wait);
	// set_current_state(state);
	spin_unlock_irqrestore(&q->lock, flags);
}

unsigned long find_last_bit(const unsigned long *addr, unsigned long size)
{
	unsigned long words;
	unsigned long tmp;

	/* Start at final word. */
	words = size / BITS_PER_LONG;

	/* Partial final word? */
	if (size & (BITS_PER_LONG-1)) {
		tmp = (addr[words] & (~0UL >> (BITS_PER_LONG
					 - (size & (BITS_PER_LONG-1)))));
		if (tmp)
			goto found;
	}

	while (words) {
		tmp = addr[--words];
		if (tmp) {
found:
			return words * BITS_PER_LONG + __fls(tmp);
		}
	}

	/* Not found */
	return size;
}

/**
 * schedule_delayed_work - put work task in global workqueue after delay
 * @dwork: job to be done
 * @delay: number of jiffies to wait or 0 for immediate execution
 *
 * After waiting for a given time this puts a job in the kernel-global
 * workqueue.
 */
int schedule_delayed_work(struct delayed_work *dwork,
					unsigned long delay)
{
    return 0;
}

/**
 * driver_register - register driver with bus
 * @drv: driver to register
 *
 * We pass off most of the work to the bus_add_driver() call,
 * since most of the things we have to do deal with the bus
 * structures.
 */
int driver_register(struct device_driver *drv)
{
	struct bus_type *bus;
	struct driver_private *priv;
	int error = 0;

	bus = bus_get(drv->bus);
	if (!bus)
		return -EINVAL;

	pr_debug("bus: '%s': add driver %s\n", bus->name, drv->name);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		error = -ENOMEM;
		goto out_put_bus;
	}
	// klist_init(&priv->klist_devices, NULL, NULL);
	priv->driver = drv;
	drv->p = priv;
	// priv->kobj.kset = bus->p->drivers_kset;
	// error = kobject_init_and_add(&priv->kobj, &driver_ktype, NULL,
	//                  "%s", drv->name);
	// if (error)
	//     goto out_unregister;

	if (drv->bus->p->drivers_autoprobe) {
		error = driver_attach(drv);
		if (error)
			goto out_unregister;
	}
	// klist_add_tail(&priv->knode_bus, &bus->p->klist_drivers);
	// module_add_driver(drv->owner, drv);

	// error = driver_create_file(drv, &driver_attr_uevent);
	// if (error) {
	//     printk(KERN_ERR "%s: uevent attr (%s) failed\n",
	//         __func__, drv->name);
	// }
	// error = driver_add_attrs(bus, drv);
	// if (error) {
	//     [> How the hell do we get out of this pickle? Give up <]
	//     printk(KERN_ERR "%s: driver_add_attrs(%s) failed\n",
	//         __func__, drv->name);
	// }

	// if (!drv->suppress_bind_attrs) {
	//     error = add_bind_files(drv);
	//     if (error) {
	//         [> Ditto <]
	//         printk(KERN_ERR "%s: add_bind_files(%s) failed\n",
	//             __func__, drv->name);
	//     }
	// }

	// kobject_uevent(&priv->kobj, KOBJ_ADD);
	return 0;

out_unregister:
	// kobject_put(&priv->kobj);
	kfree(drv->p);
	drv->p = NULL;
out_put_bus:
	bus_put(bus);
	return error;
}

/**
 * device_add - add device to device hierarchy.
 * @dev: device.
 *
 * This is part 2 of device_register(), though may be called
 * separately _iff_ device_initialize() has been called separately.
 *
 * This adds @dev to the kobject hierarchy via kobject_add(), adds it
 * to the global and sibling lists for the device, then
 * adds it to the other relevant subsystems of the driver model.
 *
 * NOTE: _Never_ directly free @dev after calling this function, even
 * if it returned an error! Always use put_device() to give up your
 * reference instead.
 */
int device_add(struct device *dev)
{
	struct device *parent = NULL;
	struct class_interface *class_intf;
	int error = -EINVAL;

	dev = get_device(dev);
	if (!dev)
		goto done;

	if (!dev->p) {
		error = device_private_init(dev);
		if (error)
			goto done;
	}

	/*
	 * for statically allocated devices, which should all be converted
	 * some day, we need to initialize the name. We prevent reading back
	 * the name, and force the use of dev_name()
	 */
	if (dev->init_name) {
		dev_set_name(dev, "%s", dev->init_name);
		dev->init_name = NULL;
	}

	if (!dev_name(dev)) {
		error = -EINVAL;
		goto name_error;
	}

	pr_debug("device: '%s': %s\n", dev_name(dev), __func__);

	parent = get_device(dev->parent);
	setup_parent(dev, parent);

	/* use parent numa_node */
	if (parent)
		set_dev_node(dev, dev_to_node(parent));

	error = bus_add_device(dev);
	if (error)
		goto BusError;
	//device_pm_add(dev);

	bus_probe_device(dev);
	if (parent)
		klist_add_tail(&dev->p->knode_parent,
			       &parent->p->klist_children);

	if (dev->class) {
		mutex_lock(&dev->class->p->class_mutex);
		/* tie the class to the device */
		klist_add_tail(&dev->knode_class,
			       &dev->class->p->class_devices);

		/* notify any interfaces that the device is here */
		list_for_each_entry(class_intf,
				    &dev->class->p->class_interfaces, node)
			if (class_intf->add_dev)
				class_intf->add_dev(dev, class_intf);
		mutex_unlock(&dev->class->p->class_mutex);
	}
done:
	put_device(dev);
	return error;
 BusError:
	cleanup_device_parent(dev);
	if (parent)
		put_device(parent);
name_error:
	kfree(dev->p);
	dev->p = NULL;
	goto done;
}

static struct device *next_device(struct klist_iter *i)
{
    struct klist_node *n = klist_next(i);
    struct device *dev = NULL;
    struct device_private *dev_prv;

    if (n) {
        dev_prv = to_device_private_bus(n);
        dev = dev_prv->device;
    }
    return dev;
}

/**
 * bus_for_each_dev - device iterator.
 * @bus: bus type.
 * @start: device to start iterating from.
 * @data: data for the callback.
 * @fn: function to be called for each device.
 *
 * Iterate over @bus's list of devices, and call @fn for each,
 * passing it @data. If @start is not NULL, we use that device to
 * begin iterating from.
 *
 * We check the return of @fn each time. If it returns anything
 * other than 0, we break out and return that value.
 *
 * NOTE: The device that returns a non-zero value is not retained
 * in any way, nor is its refcount incremented. If the caller needs
 * to retain this data, it should do so, and increment the reference
 * count in the supplied callback.
 */
int bus_for_each_dev(struct bus_type *bus, struct device *start,
             void *data, int (*fn)(struct device *, void *))
{
    struct klist_iter i;
    struct device *dev;
    int error = 0;

    if (!bus)
        return -EINVAL;

    klist_iter_init_node(&bus->p->klist_devices, &i,
                 (start ? &start->p->knode_bus : NULL));
    while ((dev = next_device(&i)) && !error)
        error = fn(dev, data);
    klist_iter_exit(&i);
    return error;
}

static int __driver_attach(struct device *dev, void *data)
{
    struct device_driver *drv = data;

	/*
	 * Lock device and try to bind to it. We drop the error
	 * here and always return 0, because we need to keep trying
	 * to bind to devices and some drivers will return an error
	 * simply if it didn't support the device.
	 *
	 * driver_probe_device() will spit a warning if there
	 * is an error.
	 */

    if (!driver_match_device(drv, dev))
        return 0;

    if (dev->parent)	/* Needed for USB */
        device_lock(dev->parent);
    device_lock(dev);
    if (!dev->driver)
        driver_probe_device(drv, dev);
    device_unlock(dev);
    if (dev->parent)
        device_unlock(dev->parent);

    return 0;
}

/**
 * driver_attach - try to bind driver to devices.
 * @drv: driver.
 *
 * Walk the list of devices that the bus has on it and try to
 * match the driver with each one.  If driver_probe_device()
 * returns 0 and the @dev->driver is set, we've found a
 * compatible pair.
 */
int driver_attach(struct device_driver *drv)
{
    return bus_for_each_dev(drv->bus, NULL, drv, __driver_attach);
}

static void driver_bound(struct device *dev)
{
	if (klist_node_attached(&dev->p->knode_driver)) {
		printk(KERN_WARNING "%s: device %s already bound\n",
			__func__, kobject_name(&dev->kobj));
		return;
	}

	pr_debug("driver: '%s': %s: bound to device '%s'\n", dev_name(dev),
		 __func__, dev->driver->name);

	klist_add_tail(&dev->p->knode_driver, &dev->driver->p->klist_devices);

	// if (dev->bus)
	//     blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
	//                      BUS_NOTIFY_BOUND_DRIVER, dev);
}

static int really_probe(struct device *dev, struct device_driver *drv)
{
	int ret = 0;

	// atomic_inc(&probe_count);
	pr_debug("bus: '%s': %s: probing driver %s with device %s\n",
		 drv->bus->name, __func__, drv->name, dev_name(dev));
	WARN_ON(!list_empty(&dev->devres_head));

	dev->driver = drv;
	// if (driver_sysfs_add(dev)) {
	//     printk(KERN_ERR "%s: driver_sysfs_add(%s) failed\n",
	//         __func__, dev_name(dev));
	//     goto probe_failed;
	// }

	if (dev->bus->probe) {
		ret = dev->bus->probe(dev);
		if (ret)
			goto probe_failed;
	} else if (drv->probe) {
		ret = drv->probe(dev);
		if (ret)
			goto probe_failed;
	}

	driver_bound(dev);
	ret = 1;
	pr_debug("bus: '%s': %s: bound device %s to driver %s\n",
		 drv->bus->name, __func__, dev_name(dev), drv->name);
	goto done;

probe_failed:
	// devres_release_all(dev);
	// driver_sysfs_remove(dev);
	dev->driver = NULL;

	if (ret != -ENODEV && ret != -ENXIO) {
		/* driver matched but the probe failed */
		printk(KERN_WARNING
		       "%s: probe of %s failed with error %d\n",
		       drv->name, dev_name(dev), ret);
	}
	/*
	 * Ignore errors returned by ->probe so that the next driver can try
	 * its luck.
	 */
	ret = 0;
done:
	// atomic_dec(&probe_count);
	// wake_up(&probe_waitqueue);
	return ret;
}

/**
 * driver_probe_device - attempt to bind device & driver together
 * @drv: driver to bind a device to
 * @dev: device to try to bind to the driver
 *
 * This function returns -ENODEV if the device is not registered,
 * 1 if the device is bound successfully and 0 otherwise.
 *
 * This function must be called with @dev lock held.  When called for a
 * USB interface, @dev->parent lock must be held as well.
 */
int driver_probe_device(struct device_driver *drv, struct device *dev)
{
	int ret = 0;

	// if (!device_is_registered(dev))
	//     return -ENODEV;
    // 
	// pr_debug("bus: '%s': %s: matched device %s with driver %s\n",
	//      drv->bus->name, __func__, dev_name(dev), drv->name);
    // 
	// pm_runtime_get_noresume(dev);
	// pm_runtime_barrier(dev);
	ret = really_probe(dev, drv);
	// pm_runtime_put_sync(dev);

	return ret;
}

static int __device_attach(struct device_driver *drv, void *data)
{
	struct device *dev = data;

	if (!driver_match_device(drv, dev))
		return 0;

	return driver_probe_device(drv, dev);
}

static struct device_driver *next_driver(struct klist_iter *i)
{
	struct klist_node *n = klist_next(i);
	struct driver_private *drv_priv;

	if (n) {
		drv_priv = container_of(n, struct driver_private, knode_bus);
		return drv_priv->driver;
	}
	return NULL;
}

/**
 * bus_for_each_drv - driver iterator
 * @bus: bus we're dealing with.
 * @start: driver to start iterating on.
 * @data: data to pass to the callback.
 * @fn: function to call for each driver.
 *
 * This is nearly identical to the device iterator above.
 * We iterate over each driver that belongs to @bus, and call
 * @fn for each. If @fn returns anything but 0, we break out
 * and return it. If @start is not NULL, we use it as the head
 * of the list.
 *
 * NOTE: we don't return the driver that returns a non-zero
 * value, nor do we leave the reference count incremented for that
 * driver. If the caller needs to know that info, it must set it
 * in the callback. It must also be sure to increment the refcount
 * so it doesn't disappear before returning to the caller.
 */
int bus_for_each_drv(struct bus_type *bus, struct device_driver *start,
		     void *data, int (*fn)(struct device_driver *, void *))
{
	struct klist_iter i;
	struct device_driver *drv;
	int error = 0;

	if (!bus)
		return -EINVAL;

	klist_iter_init_node(&bus->p->klist_drivers, &i,
			     start ? &start->p->knode_bus : NULL);
	while ((drv = next_driver(&i)) && !error)
		error = fn(drv, data);
	klist_iter_exit(&i);
	return error;
}

/**
 * device_attach - try to attach device to a driver.
 * @dev: device.
 *
 * Walk the list of drivers that the bus has and call
 * driver_probe_device() for each pair. If a compatible
 * pair is found, break out and return.
 *
 * Returns 1 if the device was bound to a driver;
 * 0 if no matching driver was found;
 * -ENODEV if the device is not registered.
 *
 * When called for a USB interface, @dev->parent lock must be held.
 */
int device_attach(struct device *dev)
{
	int ret = 0;

	device_lock(dev);
	if (dev->driver) {
		// ret = device_bind_driver(dev);
		// if (ret == 0)
		//     ret = 1;
		// else {
		//     dev->driver = NULL;
		//     ret = 0;
		// }

        driver_bound(dev);
        ret = 1;
	} else {
		// pm_runtime_get_noresume(dev);
		ret = bus_for_each_drv(dev->bus, NULL, dev, __device_attach);
		// pm_runtime_put_sync(dev);
	}
	device_unlock(dev);
	return ret;
}

/*
 * kernl / completion
 */

void init_completion(struct completion *x)
{
    x->done = 0;
    spin_lock_init (&x->wait.lock);
    cyg_semaphore_init (&x->wait.semaphore, 0);
}

void wait_for_completion(struct completion *)
{
    spin_lock_irq (&x->wait.locK);
    if (!x->done) {
        spin_unlock (&x->wait.locK);
        cyg_semaphore_wait (&x->wait.semaphore);
    }
    spin_unlock_irq (&x->wait.lock);
}

unsigned long wait_for_completion_timeout(struct completion *x,
						   unsigned long timeout)
{
    long ltimeout = (long)timeout;

    spin_lock_irq (&x->wait.lock);
    if (!x->done) {
        spin_unlock_irq (&x->wait.lock);
        cyg_thread_delay (10);
        spin_lock_irq (&x->wait.lock);

        ltimeout -= 10;
        if (ltimeout < 0)
            ltimeout = 0;
    }
    while (!x->donw && ltimeout);

    x->done --;
    spin_unlock_irq (&x->wait.lock);
    
    return (unsigned long)ltimeout;
}

void complete(struct completion *)
{
    unsigned long flags;
    spin_lock_irqsave (&x->wait.lock, flags);
    x->done ++;
    cyg_semaphore_post (&x->wait.semaphore);
    spin_unlock_irqrestore (&x->wait.lock, flags);
}

/*
 * nls_base
 */

int utf8_to_utf32(const u8 *s, int len, unicode_t *pu)
{
	unsigned long l;
	int c0, c, nc;
	const struct utf8_table *t;
  
	nc = 0;
	c0 = *s;
	l = c0;
	for (t = utf8_table; t->cmask; t++) {
		nc++;
		if ((c0 & t->cmask) == t->cval) {
			l &= t->lmask;
			if (l < t->lval || l > UNICODE_MAX ||
					(l & SURROGATE_MASK) == SURROGATE_PAIR)
				return -1;
			*pu = (unicode_t) l;
			return nc;
		}
		if (len <= nc)
			return -1;
		s++;
		c = (*s ^ 0x80) & 0xFF;
		if (c & 0xC0)
			return -1;
		l = (l << 6) | c;
	}
	return -1;
}

int utf32_to_utf8(unicode_t u, u8 *s, int maxlen)
{
	unsigned long l;
	int c, nc;
	const struct utf8_table *t;

	if (!s)
		return 0;

	l = u;
	if (l > UNICODE_MAX || (l & SURROGATE_MASK) == SURROGATE_PAIR)
		return -1;

	nc = 0;
	for (t = utf8_table; t->cmask && maxlen; t++, maxlen--) {
		nc++;
		if (l <= t->lmask) {
			c = t->shift;
			*s = (u8) (t->cval | (l >> c));
			while (c > 0) {
				c -= 6;
				s++;
				*s = (u8) (0x80 | ((l >> c) & 0x3F));
			}
			return nc;
		}
	}
	return -1;
}

int utf8s_to_utf16s(const u8 *s, int len, wchar_t *pwcs)
{
	u16 *op;
	int size;
	unicode_t u;

	op = pwcs;
	while (*s && len > 0) {
		if (*s & 0x80) {
			size = utf8_to_utf32(s, len, &u);
			if (size < 0)
				return -EINVAL;

			if (u >= PLANE_SIZE) {
				u -= PLANE_SIZE;
				*op++ = (wchar_t) (SURROGATE_PAIR |
						((u >> 10) & SURROGATE_BITS));
				*op++ = (wchar_t) (SURROGATE_PAIR |
						SURROGATE_LOW |
						(u & SURROGATE_BITS));
			} else {
				*op++ = (wchar_t) u;
			}
			s += size;
			len -= size;
		} else {
			*op++ = *s++;
			len--;
		}
	}
	return op - pwcs;
}

static inline unsigned long get_utf16(unsigned c, enum utf16_endian endian)
{
	switch (endian) {
	default:
		return c;
	case UTF16_LITTLE_ENDIAN:
		return __le16_to_cpu(c);
	case UTF16_BIG_ENDIAN:
		return __be16_to_cpu(c);
	}
}

int utf16s_to_utf8s(const wchar_t *pwcs, int len, enum utf16_endian endian,
		u8 *s, int maxlen)
{
	u8 *op;
	int size;
	unsigned long u, v;

	op = s;
	while (len > 0 && maxlen > 0) {
		u = get_utf16(*pwcs, endian);
		if (!u)
			break;
		pwcs++;
		len--;
		if (u > 0x7f) {
			if ((u & SURROGATE_MASK) == SURROGATE_PAIR) {
				if (u & SURROGATE_LOW) {
					/* Ignore character and move on */
					continue;
				}
				if (len <= 0)
					break;
				v = get_utf16(*pwcs, endian);
				if ((v & SURROGATE_MASK) != SURROGATE_PAIR ||
						!(v & SURROGATE_LOW)) {
					/* Ignore character and move on */
					continue;
				}
				u = PLANE_SIZE + ((u & SURROGATE_BITS) << 10)
						+ (v & SURROGATE_BITS);
				pwcs++;
				len--;
			}
			size = utf32_to_utf8(u, op, maxlen);
			if (size == -1) {
				/* Ignore character and move on */
			} else {
				op += size;
				maxlen -= size;
			}
		} else {
			*op++ = (u8) u;
			maxlen--;
		}
	}
	return op - s;
}


