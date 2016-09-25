#include "stddef.h"
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_cache.h>
#include "os-dep.h"
#include "ktime.h"
// #include "asm/swab.h"
// #include "swab.h"
#include "little_endian.h"
#include "asm/bitops.h"
#include "asm/usb-atomic.h"
#include "scatterlist.h"
#include "asm/r4kcache.h"


void alarm_callback(cyg_handle_t alarm, cyg_addrword_t data)
{
    static int iloop = 0;
    diag_printf("termy say, alarm_callback ! (%d)\n", iloop++);
}

cyg_handle_t    counter_hdl;
cyg_handle_t    alarm_hdl;
cyg_alarm       alarm_obj;

void wq_init (void);

#define USB_THREAD_PRIO 20
#define USB_STACK_SIZE  4096 
static char usb_stack[USB_STACK_SIZE];
cyg_handle_t usb_thread_handle;
cyg_thread usb_thread_data;

void usb_thread (cyg_addrword_t parameter)
{
    cyg_sem_t forever_sem;
    cyg_semaphore_init(&forever_sem, 0);

    /*
     * create work queue thread
     */
    wq_init();

    usb_init();
    ehci_hcd_init();

    usblp_init();
    cyg_semaphore_wait(&forever_sem);
} /* usb_thread */

void usb_test()
{
    cyg_thread_create (USB_THREAD_PRIO,
            usb_thread,
            0,
            "usb_thread",
            usb_stack,
            USB_STACK_SIZE,
            &usb_thread_handle,
            &usb_thread_data
            );
    cyg_thread_resume(usb_thread_handle);
} /* usb_test */

void *kzalloc(size_t size,int gfp)
{
	void *p=malloc(size);
	memset(p,0,size);
	return p;
}

static void timer_handler(cyg_handle_t alarmobj, unsigned long data)
{
    struct timer_list *timer = (struct timer_list *)data;
    
    if (timer->function)
        timer->function(timer->data);

}

void init_timer(struct timer_list *timer)
{
    CYG_ASSERT(timer != NULL, "timer is NULL!");
    if (!timer->valid) {
        timer->entry.next = NULL;
        cyg_clock_to_counter (cyg_real_time_clock(), &timer->counter_hdl);
        cyg_alarm_create (timer->counter_hdl, 
                timer_handler, 
                timer, 
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
        // expires = (expires * OS_HZ) / 1000;
        if (expires > 0) {
            // cyg_alarm_initialize(timer->alarm_hdl, cyg_current_time() + expires, 0);
            cyg_alarm_initialize(timer->alarm_hdl, expires, 0);
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
        
        if ((interval != 0) || (trigger > cyg_current_time()))
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
int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key)
 {
//     int ret = default_wake_function(wait, mode, sync, key);
// 
//     if (ret)
//         list_del_init(&wait->task_list);
//     return ret;
}

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
    spin_unlock_irqrestore(&q->lock, flags);
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

#define QUEUE_WORK_PRIO 20
#define WORK_QUEUE_STACK_SIZE   2048
static char wq_stack[WORK_QUEUE_STACK_SIZE];
cyg_handle_t wq_thread_handle;
cyg_handle_t wq_mbox_handle;
cyg_thread wq_thread_data;
cyg_mbox wq_mbox_data;

void wq_thread (cyg_addrword_t parameter)
{
    struct work_struct *work;
    cyg_handle_t *mbox_handle = (cyg_handle_t)parameter;
    
    while (true) {
        work = cyg_mbox_get (mbox_handle);
        if (work->delay)
            cyg_thread_delay(work->delay);
        if (work) {
            work->func(work);
        }
    } /* while (true) */
}

void wq_init (void)
{
    cyg_mbox_create (&wq_mbox_handle, &wq_mbox_data);

    cyg_thread_create (QUEUE_WORK_PRIO,
        wq_thread,
        (cyg_addrword_t)wq_mbox_handle,
        "queue work thread",
        wq_stack, 
        WORK_QUEUE_STACK_SIZE,
        &wq_thread_handle,
        &wq_thread_data
        );
    cyg_thread_resume(wq_thread_handle);
}

/**
 * schedule_work - put work task in global workqueue
 * @work: job to be done
 *
 * Returns zero if @work was already on the kernel-global workqueue and
 * non-zero otherwise.
 *
 * This puts a job in the kernel-global workqueue if it was not already
 * queued and leaves it in the same position on the kernel-global
 * workqueue otherwise.
 */
int schedule_work(struct work_struct *work)
{
    work->delay = 0;
    return cyg_mbox_put(wq_mbox_handle, (void *)work);
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
    dwork->work.delay = delay;
    return cyg_mbox_put(wq_mbox_handle, (void *)&dwork->work);
}

static void klist_devices_get(struct klist_node *n)
{
	struct device_private *dev_prv = to_device_private_bus(n);
	struct device *dev = dev_prv->device;

	get_device(dev);
}

static void klist_devices_put(struct klist_node *n)
{
	struct device_private *dev_prv = to_device_private_bus(n);
	struct device *dev = dev_prv->device;

	put_device(dev);
}

/**
 * bus_register - register a bus with the system.
 * @bus: bus.
 *
 * Once we have that, we registered the bus with the kobject
 * infrastructure, then register the children subsystems it has:
 * the devices and drivers that belong to the bus.
 */
int bus_register(struct bus_type *bus)
{
	int retval;
	struct bus_type_private *priv;

	// priv = kzalloc(sizeof(struct bus_type_private), GFP_KERNEL);
	priv = kmalloc(sizeof(struct bus_type_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->bus = bus;
	bus->p = priv;

	// BLOCKING_INIT_NOTIFIER_HEAD(&priv->bus_notifier);
    // 
	// retval = kobject_set_name(&priv->subsys.kobj, "%s", bus->name);
	// if (retval)
	//     goto out;
    // 
	// priv->subsys.kobj.kset = bus_kset;
	// priv->subsys.kobj.ktype = &bus_ktype;
	priv->drivers_autoprobe = 1;

	// retval = kset_register(&priv->subsys);
	// if (retval)
	//     goto out;
    // 
	// retval = bus_create_file(bus, &bus_attr_uevent);
	// if (retval)
	//     goto bus_uevent_fail;
    // 
	// priv->devices_kset = kset_create_and_add("devices", NULL,
	//                      &priv->subsys.kobj);
	// if (!priv->devices_kset) {
	//     retval = -ENOMEM;
	//     goto bus_devices_fail;
	// }
    // 
	// priv->drivers_kset = kset_create_and_add("drivers", NULL,
	//                      &priv->subsys.kobj);
	// if (!priv->drivers_kset) {
	//     retval = -ENOMEM;
	//     goto bus_drivers_fail;
	// }

	klist_init(&priv->klist_devices, klist_devices_get, klist_devices_put);
	klist_init(&priv->klist_drivers, NULL, NULL);

	// retval = add_probe_files(bus);
	// if (retval)
	//     goto bus_probe_files_fail;
    // 
	// retval = bus_add_attrs(bus);
	// if (retval)
	//     goto bus_attrs_fail;

	pr_debug("bus: '%s': registered\n", bus->name);
	return 0;

// bus_attrs_fail:
//     remove_probe_files(bus);
// bus_probe_files_fail:
//     kset_unregister(bus->p->drivers_kset);
// bus_drivers_fail:
//     kset_unregister(bus->p->devices_kset);
// bus_devices_fail:
//     bus_remove_file(bus, &bus_attr_uevent);
// bus_uevent_fail:
//     kset_unregister(&bus->p->subsys);
// out:
//     kfree(bus->p);
//     bus->p = NULL;
//     return retval;
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

    BUG_ON(!drv->bus->p);

	bus = bus_get(drv->bus);
	if (!bus)
		return -EINVAL;

	pr_debug("bus: '%s': add driver %s\n", bus->name, drv->name);

	// priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	priv = kmalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		error = -ENOMEM;
		goto out_put_bus;
	}
	klist_init(&priv->klist_devices, NULL, NULL);
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
	klist_add_tail(&priv->knode_bus, &bus->p->klist_drivers);
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
 * bus_probe_device - probe drivers for a new device
 * @dev: device to probe
 *
 * - Automatically probe for a driver if the bus allows it.
 */
void bus_probe_device(struct device *dev)
{
	struct bus_type *bus = dev->bus;
	int ret;

	if (bus && bus->p->drivers_autoprobe) {
		ret = device_attach(dev);
		WARN_ON(ret < 0);
	}
}

/**
 * dev_set_name - set a device name
 * @dev: device
 * @fmt: format string for the device's name
 */
int dev_set_name(struct device *dev, const char *fmt, ...)
{
	// va_list vargs;
	// int err;
    // 
	// va_start(vargs, fmt);
	// err = kobject_set_name_vargs(&dev->kobj, fmt, vargs);
	// va_end(vargs);
	// return err;
    return 0;
}

/**
 * bus_add_device - add device to bus
 * @dev: device being added
 *
 * - Add device's bus attributes.
 * - Create links to device's bus.
 * - Add the device to its bus's list of devices.
 */
int bus_add_device(struct device *dev)
{
	struct bus_type *bus = bus_get(dev->bus);
	// int error = 0;

	if (bus) {
		pr_debug("bus: '%s': add device %s\n", bus->name, dev_name(dev));
		// error = device_add_attrs(bus, dev);
		// if (error)
		//     goto out_put;
		// error = sysfs_create_link(&bus->p->devices_kset->kobj,
		//                 &dev->kobj, dev_name(dev));
		// if (error)
		//     goto out_id;
		// error = sysfs_create_link(&dev->kobj,
		//         &dev->bus->p->subsys.kobj, "subsystem");
		// if (error)
		//     goto out_subsys;
		// error = make_deprecated_bus_links(dev);
		// if (error)
		//     goto out_deprecated;
		klist_add_tail(&dev->p->knode_bus, &bus->p->klist_devices);
	}
	return 0;

// out_deprecated:
//     sysfs_remove_link(&dev->kobj, "subsystem");
// out_subsys:
//     sysfs_remove_link(&bus->p->devices_kset->kobj, dev_name(dev));
// out_id:
//     device_remove_attrs(bus, dev);
// out_put:
//     bus_put(dev->bus);
//     return error;
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
 * bus_find_device - device iterator for locating a particular device.
 * @bus: bus type
 * @start: Device to begin with
 * @data: Data to pass to match function
 * @match: Callback function to check device
 *
 * This is similar to the bus_for_each_dev() function above, but it
 * returns a reference to a device that is 'found' for later use, as
 * determined by the @match callback.
 *
 * The callback should return 0 if the device doesn't match and non-zero
 * if it does.  If the callback returns non-zero, this function will
 * return to the caller and not iterate over any more devices.
 */
struct device *bus_find_device(struct bus_type *bus,
			       struct device *start, void *data,
			       int (*match)(struct device *dev, void *data))
{
	struct klist_iter i;
	struct device *dev;

	if (!bus)
		return NULL;

	klist_iter_init_node(&bus->p->klist_devices, &i,
			     (start ? &start->p->knode_bus : NULL));
	while ((dev = next_device(&i)))
		if (match(dev, data) && get_device(dev))
			break;
	klist_iter_exit(&i);
	return dev;
}

/*
 * __device_release_driver() must be called with @dev lock held.
 * When called for a USB interface, @dev->parent lock must be held as well.
 */
static void __device_release_driver(struct device *dev)
{
	struct device_driver *drv;

	drv = dev->driver;
	if (drv) {
		// pm_runtime_get_noresume(dev);
		// pm_runtime_barrier(dev);
        // 
		// driver_sysfs_remove(dev);

		// if (dev->bus)
		//     blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
		//                      BUS_NOTIFY_UNBIND_DRIVER,
		//                      dev);

		if (dev->bus && dev->bus->remove)
			dev->bus->remove(dev);
		else if (drv->remove)
			drv->remove(dev);
		// devres_release_all(dev);
		dev->driver = NULL;
		klist_remove(&dev->p->knode_driver);
		// if (dev->bus)
		//     blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
		//                      BUS_NOTIFY_UNBOUND_DRIVER,
		//                      dev);
        // 
		// pm_runtime_put_sync(dev);
	}
}

/**
 * device_release_driver - manually detach device from driver.
 * @dev: device.
 *
 * Manually detach device from driver.
 * When called for a USB interface, @dev->parent lock must be held.
 */
void device_release_driver(struct device *dev)
{
	/*
	 * If anyone calls device_release_driver() recursively from
	 * within their ->remove callback for the same device, they
	 * will deadlock right here.
	 */
	device_lock(dev);
	__device_release_driver(dev);
	device_unlock(dev);
}

/**
 * bus_remove_device - remove device from bus
 * @dev: device to be removed
 *
 * - Remove symlink from bus's directory.
 * - Delete device from bus's list.
 * - Detach from its driver.
 * - Drop reference taken in bus_add_device().
 */
void bus_remove_device(struct device *dev)
{
	if (dev->bus) {
		// sysfs_remove_link(&dev->kobj, "subsystem");
		// remove_deprecated_bus_links(dev);
		// sysfs_remove_link(&dev->bus->p->devices_kset->kobj,
		//           dev_name(dev));
		// device_remove_attrs(dev->bus, dev);
		if (klist_node_attached(&dev->p->knode_bus))
			klist_del(&dev->p->knode_bus);

		pr_debug("bus: '%s': remove device %s\n",
			 dev->bus->name, dev_name(dev));
		device_release_driver(dev);
		bus_put(dev->bus);
	}
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
	// if (dev->init_name) {
	//     dev_set_name(dev, "%s", dev->init_name);
	//     dev->init_name = NULL;
	// }

	// if (!dev_name(dev)) {
	//     error = -EINVAL;
	//     goto name_error;
	// }

    parent = get_device(dev->parent);
	// setup_parent(dev, parent);

	// pr_debug("device: '%s': %s addr:%x\n", dev_name(dev), __func__, (u32)dev);
    // if (parent)
    //     pr_debug("%s(%d) device parent:%x\n", __func__, __LINE__, (u32)parent);


	/* use parent numa_node */
	// if (parent)
	//     set_dev_node(dev, dev_to_node(parent));

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

/**
 * device_initialize - init device structure.
 * @dev: device.
 *
 * This prepares the device for use by other layers by initializing
 * its fields.
 * It is the first half of device_register(), if called by
 * that function, though it can also be called separately, so one
 * may use @dev's fields. In particular, get_device()/put_device()
 * may be used for reference counting of @dev after calling this
 * function.
 *
 * NOTE: Use put_device() to give up your reference instead of freeing
 * @dev directly once you have called this function.
 */
void device_initialize(struct device *dev)
{
	// dev->kobj.kset = devices_kset;
	// kobject_init(&dev->kobj, &device_ktype);
	INIT_LIST_HEAD(&dev->dma_pools);
	mutex_init(&dev->mutex);
	// lockdep_set_novalidate_class(&dev->mutex);
	spin_lock_init(&dev->devres_lock);
	INIT_LIST_HEAD(&dev->devres_head);
	// device_pm_init(dev);
	// set_dev_node(dev, -1);
}

/**
 * device_register - register a device with the system.
 * @dev: pointer to the device structure
 *
 * This happens in two clean steps - initialize the device
 * and add it to the system. The two steps can be called
 * separately, but this is the easiest and most common.
 * I.e. you should only call the two helpers separately if
 * have a clearly defined need to use and refcount the device
 * before it is added to the hierarchy.
 *
 * NOTE: _Never_ directly free @dev after calling this function, even
 * if it returned an error! Always use put_device() to give up the
 * reference initialized in this function instead.
 */
int device_register(struct device *dev)
{
	device_initialize(dev);
	return device_add(dev);
}

/**
 * device_del - delete device from system.
 * @dev: device.
 *
 * This is the first part of the device unregistration
 * sequence. This removes the device from the lists we control
 * from here, has it removed from the other driver model
 * subsystems it was added to in device_add(), and removes it
 * from the kobject hierarchy.
 *
 * NOTE: this should be called manually _iff_ device_add() was
 * also called manually.
 */
void device_del(struct device *dev)
{
	struct device *parent = dev->parent;
	struct class_interface *class_intf;

	/* Notify clients of device removal.  This call must come
	 * before dpm_sysfs_remove().
	 */
	// if (dev->bus)
	//     blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
	//                      BUS_NOTIFY_DEL_DEVICE, dev);
	// device_pm_remove(dev);
	// dpm_sysfs_remove(dev);
    pr_debug("%s(%d) dev:%x\n", __func__, __LINE__, (u32)dev);
	if (parent)
		klist_del(&dev->p->knode_parent);
	// if (MAJOR(dev->devt)) {
	//     devtmpfs_delete_node(dev);
	//     device_remove_sys_dev_entry(dev);
	//     device_remove_file(dev, &devt_attr);
	// }
	if (dev->class) {
		// device_remove_class_symlinks(dev);

		mutex_lock(&dev->class->p->class_mutex);
		/* notify any interfaces that the device is now gone */
		list_for_each_entry(class_intf,
				    &dev->class->p->class_interfaces, node)
			if (class_intf->remove_dev)
				class_intf->remove_dev(dev, class_intf);
		/* remove the device from the class list */
		klist_del(&dev->knode_class);
		mutex_unlock(&dev->class->p->class_mutex);
	}
	// device_remove_file(dev, &uevent_attr);
	// device_remove_attrs(dev);
	bus_remove_device(dev);

	/*
	 * Some platform devices are driven without driver attached
	 * and managed resources may have been acquired.  Make sure
	 * all resources are released.
	 */
	// devres_release_all(dev);

	/* Notify the platform of the removal, in case they
	 * need to do anything...
	 */
	// if (platform_notify_remove)
	//     platform_notify_remove(dev);
	// kobject_uevent(&dev->kobj, KOBJ_REMOVE);
	// cleanup_device_parent(dev);
	// kobject_del(&dev->kobj);
	// put_device(parent);
}

/**
 * device_unregister - unregister device from system.
 * @dev: device going away.
 *
 * We do this in two parts, like we do device_register(). First,
 * we remove it from all the subsystems with device_del(), then
 * we decrement the reference count via put_device(). If that
 * is the final reference count, the device will be cleaned up
 * via device_release() above. Otherwise, the structure will
 * stick around until the final reference to the device is dropped.
 */
void device_unregister(struct device *dev)
{
	pr_debug("device: '%s': %s\n", dev_name(dev), __func__);
	device_del(dev);
	put_device(dev);
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
		// printk(KERN_WARNING "%s: device %s already bound\n",
		//     __func__, kobject_name(&dev->kobj));
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
 * These exports can't be _GPL due to .h files using this within them, and it
 * might break something that was previously working...
 */
void *dev_get_drvdata(const struct device *dev)
{
	if (dev && dev->p)
		return dev->p->driver_data;
	return NULL;
}
EXPORT_SYMBOL(dev_get_drvdata);

void dev_set_drvdata(struct device *dev, void *data)
{
	int error;

	if (!dev)
		return;
	if (!dev->p) {
		error = device_private_init(dev);
		if (error)
			return;
	}
	dev->p->driver_data = data;
}

/*
 * kernl / completion
 */

void usb_init_completion(struct usb_completion *x)
{
    x->done = 0;
    spin_lock_init (&x->wait.lock);
    cyg_semaphore_init (&x->wait.semaphore, 0);
}

void usb_wait_for_completion(struct usb_completion *x)
{
    spin_lock_irq (&x->wait.lock);
    if (!x->done) {
        spin_unlock (&x->wait.lock);
        cyg_semaphore_wait (&x->wait.semaphore);
    }
    spin_unlock_irq (&x->wait.lock);
}

unsigned long usb_wait_for_completion_timeout(struct usb_completion *x,
						   unsigned long timeout)
{
    return cyg_semaphore_timed_wait(&x->wait.semaphore,
                cyg_current_time() + timeout);
}

void usb_complete(struct usb_completion *x)
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

int utf8s_to_utf16s(const u8 *s, int len, u16 *pwcs)
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
				*op++ = (u16) (SURROGATE_PAIR |
						((u >> 10) & SURROGATE_BITS));
				*op++ = (u16) (SURROGATE_PAIR |
						SURROGATE_LOW |
						(u & SURROGATE_BITS));
			} else {
				*op++ = (u16) u;
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

int utf16s_to_utf8s(const u16 *pwcs, int len, enum utf16_endian endian,
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

/*
 * When we convert to jiffies then we interpret incoming values
 * the following way:
 *
 * - negative values mean 'infinite timeout' (MAX_JIFFY_OFFSET)
 *
 * - 'too large' values [that would result in larger than
 *   MAX_JIFFY_OFFSET values] mean 'infinite timeout' too.
 *
 * - all other values are converted to jiffies by either multiplying
 *   the input value by a factor or dividing it with a factor
 *
 * We must also be careful about 32-bit overflows.
 */
unsigned long msecs_to_jiffies(const unsigned int m)
{
    return (m + 9)/10;
}

/**
 * getnstimeofday - Returns the time of day in a timespec
 * @ts:		pointer to the timespec to be set
 *
 * Returns the time of day in a timespec.
 */
void getnstimeofday(struct timespec *ts)
{
	// unsigned long seq;
	s64 nsecs;

	// WARN_ON(timekeeping_suspended);
    // 
	// do {
	//     seq = read_seqbegin(&xtime_lock);
    // 
	//     *ts = xtime;
	//     nsecs = timekeeping_get_ns();
    // 
	//     [> If arch requires, add in gettimeoffset() <]
	//     nsecs += arch_gettimeoffset();
    // 
	// } while (read_seqretry(&xtime_lock, seq));

    nsecs = (s64)cyg_current_time();
	timespec_add_ns(ts, nsecs);
}

/**
 * ktime_get_real - get the real (wall-) time in ktime_t format
 *
 * returns the time in ktime_t format
 */
ktime_t ktime_get_real(void)
{
	struct timespec now;

	getnstimeofday(&now);

	return timespec_to_ktime(now);
}

static void klist_children_put(struct klist_node *n)
{
	struct device_private *p = to_device_private_parent(n);
	struct device *dev = p->device;

	put_device(dev);
}

static void klist_children_get(struct klist_node *n)
{
	struct device_private *p = to_device_private_parent(n);
	struct device *dev = p->device;

	get_device(dev);
}

int device_private_init(struct device *dev)
{
	// dev->p = kzalloc(sizeof(*dev->p), GFP_KERNEL);
	dev->p = kmalloc(sizeof(*dev->p), GFP_KERNEL);
	if (!dev->p)
		return -ENOMEM;
	dev->p->device = dev;
	klist_init(&dev->p->klist_children, klist_children_get,
		   klist_children_put);
	return 0;
}

/**
 * msleep - sleep safely even with waitqueue interruptions
 * @msecs: Time in milliseconds to sleep for
 */
void msleep(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs) + 1;

    if (timeout)
        cyg_thread_delay(timeout);
}

/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
void kref_init(struct kref *kref)
{
    atomic_set(&kref->refcount, 1);
    smp_mb();
}

/**
 * kref_get - increment refcount for object.
 * @kref: object.
 */
void kref_get(struct kref *kref)
{
    WARN_ON(!atomic_read(&kref->refcount));
    atomic_inc(&kref->refcount);
    smp_mb__after_atomic_inc();
}

/**
 * kref_put - decrement refcount for object.
 * @kref: object.
 * @release: pointer to the function that will clean up the object when the
 *	     last reference to the object is released.
 *	     This pointer is required, and it is not acceptable to pass kfree
 *	     in as this function.
 *
 * Decrement the refcount, and if 0, call release().
 * Return 1 if the object was removed, otherwise return 0.  Beware, if this
 * function returns 0, you still can not count on the kref from remaining in
 * memory.  Only use the return value if you want to see if the kref is now
 * gone, not present.
 */
extern void kaligned_free(void *block);
int kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
    WARN_ON(release == NULL);
    WARN_ON(release == (void (*)(struct kref *))kaligned_free);

    if (atomic_dec_and_test(&kref->refcount)) {
        release(kref);
        return 1;
    }
    return 0;
}

inline void __dma_sync_virtual(void *addr, size_t size,
	enum dma_data_direction direction)
{
    unsigned long lsize = 32;
    unsigned long almask = ~(lsize - 1);

	switch (direction) {
	case DMA_TO_DEVICE:
		// dma_cache_wback((unsigned long)addr, size);
        blast_dcache_range((unsigned long)addr, (unsigned long)addr + size);
		break;

	case DMA_FROM_DEVICE:
		// dma_cache_inv((unsigned long)addr, size);
		cache_op(Hit_Writeback_Inv_D, (unsigned long)addr & almask);
		cache_op(Hit_Writeback_Inv_D, ((unsigned long)addr + size - 1)  & almask);
		blast_inv_dcache_range((unsigned long)addr, (unsigned long)addr + size);
		break;

	case DMA_BIDIRECTIONAL:
		// dma_cache_wback_inv((unsigned long)addr, size);
        blast_dcache_range((unsigned long)addr, (unsigned long)addr + size);
		break;

	default:
		BUG();
	}
    __sync();
}

/*
 * A single sg entry may refer to multiple physically contiguous
 * pages. But we still need to process highmem pages individually.
 * If highmem is not configured then the bulk of this loop gets
 * optimized out.
 */
// static inline void __dma_sync(struct page *page,
//     unsigned long offset, size_t size, enum dma_data_direction direction)
// {
//     size_t left = size;
// 
//     do {
//         size_t len = left;
// 
//         __dma_sync_virtual(page_address(page) + offset,
//                 size, direction);
// 
//         offset = 0;
//         page++;
//         left -= len;
//     } while (left);
// }

void dma_unmap_single(struct device *dev, dma_addr_t dma_addr, size_t size,
	enum dma_data_direction direction)
{
	// if (cpu_is_noncoherent_r10000(dev))
	//     __dma_sync(dma_addr_to_page(dev, dma_addr),
	//            dma_addr & ~PAGE_MASK, size, direction);
    // 
	// plat_unmap_dma_mem(dev, dma_addr, size, direction);
}

int dma_mapping_error(struct device *dev, dma_addr_t dma_addr)
{
    return 0;
}

// void udelay(int delay)
// {
//     CYGACC_CALL_IF_DELAY_US(delay);
// }

void mdelay(int ms)
{
    udelay(ms * 1000);
}

/*
 * This implementation of find_{first,next}_zero_bit was stolen from
 * Linus' asm-alpha/bitops.h.
 */
#define BITOP_WORD(nr)		((nr) / BITS_PER_LONG)
#define ffz(x)  __ffs(~(x))

unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
				 unsigned long offset)
{
	const unsigned long *p = addr + BITOP_WORD(offset);
	unsigned long result = offset & ~(BITS_PER_LONG-1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset %= BITS_PER_LONG;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (BITS_PER_LONG - offset);
		if (size < BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG-1)) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
	if (tmp == ~0UL)	/* Are any bits zero? */
		return result + size;	/* Nope. */
found_middle:
	return result + ffz(tmp);
}

void *
aligned_alloc(size_t nb, size_t align)
{
	u32 *cp;
	char *p;

	if( align < sizeof(u32) )
		align = sizeof(u32);
	align--;

	p = malloc( nb + sizeof(u32) + align );

	if( p== NULL ) return NULL;
	
	memset(p, 0, nb + sizeof(u32) + align);
	cp = ((u32)( p + sizeof(u32) + align ) & ~((u32)align));
	cp[-1] = p;

	return cp;
}

void
aligned_free(void *block)
{
	char *cp = block;
	char *p;

	if( block == NULL ) return;

	p = *(u32 *)( cp - sizeof(u32) );

	free( p );
}

/**
 * strlcpy - Copy a %NUL terminated string into a sized buffer
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @size: size of destination buffer
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like strncpy() does.
 */
size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}

/**
 * ktime_add_ns - Add a scalar nanoseconds value to a ktime_t variable
 * @kt:		addend
 * @nsec:	the scalar nsec value to add
 *
 * Returns the sum of kt and nsec in ktime_t format
 */
ktime_t ktime_add_ns(const ktime_t kt, u64 nsec)
{
	ktime_t tmp;

	if (likely(nsec < NSEC_PER_SEC)) {
		tmp.tv64 = nsec;
	} else {
		unsigned long rem = do_div(nsec, NSEC_PER_SEC);

		tmp = ktime_set((long)nsec, rem);
	}

	return ktime_add(kt, tmp);
}

u32  __div64_32(u64 *n, u32 base)
{
	u64 rem = *n;
	u64 b = base;
	u64 res, d = 1;
	u32 high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (u64) high << 32;
		rem -= (u64) (high*base) << 32;
	}

	while ((s64)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}

/**
 * sg_next - return the next scatterlist entry in a list
 * @sg:		The current sg entry
 *
 * Description:
 *   Usually the next entry will be @sg@ + 1, but if this sg element is part
 *   of a chained scatterlist, it could jump to the start of a new
 *   scatterlist array.
 *
 **/
struct scatterlist *sg_next(struct scatterlist *sg)
{
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sg->sg_magic != SG_MAGIC);
#endif
	if (sg_is_last(sg))
		return NULL;

	sg++;
	if (unlikely(sg_is_chain(sg)))
		sg = sg_chain_ptr(sg);

	return sg;
}

