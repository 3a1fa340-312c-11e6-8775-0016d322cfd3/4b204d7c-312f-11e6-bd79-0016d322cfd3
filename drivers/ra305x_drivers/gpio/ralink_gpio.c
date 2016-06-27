
#include "ralink_gpio.h"

int ralink_gpio_init (void)
{

    uint32 gpio_mode;

    gpio_mode = le32_to_cpu(*(volatile uint32 *)(RALINK_REG_GPIOMODE));
    gpio_mode |= PS_GPIOMODE_BUTTON2;
    *(volatile uint32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpio_mode);

    gpio_mode = le32_to_cpu(*(volatile uint32 *)(RALINK_REG_GPIOMODE2));
    gpio_mode &= 0x003C;
    gpio_mode |= (PS_GPIOMODE_BUTTON1|PS_GPIOMODE_STATUS|PS_GPIOMODE_WIRLESS|PS_GPIOMODE_USB);
    *(volatile uint32 *)(RALINK_REG_GPIOMODE2) = cpu_to_le32(gpio_mode);

}

int ralink_gpio_ioctl (unsigned int req, unsigned long arg)
{

	unsigned long tmp;
	ralink_gpio_reg_info info;

	req &= RALINK_GPIO_DATA_MASK;

	switch(req) {
	case RALINK_GPIO_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_SET_DIR_IN:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		tmp &= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIODIR) = tmp;
		break;
	case RALINK_GPIO_SET_DIR_OUT:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		tmp |= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIODIR) = tmp;
		break;
	case RALINK_GPIO_READ: //RALINK_GPIO_READ_INT
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO_WRITE: //RALINK_GPIO_WRITE_INT
		*(volatile u32 *)(RALINK_REG_PIODATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_SET: //RALINK_GPIO_SET_INT
		*(volatile u32 *)(RALINK_REG_PIOSET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_CLEAR: //RALINK_GPIO_CLEAR_INT
		*(volatile u32 *)(RALINK_REG_PIORESET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_ENABLE_INTP:
		*(volatile u32 *)(RALINK_REG_INTENA) = cpu_to_le32(RALINK_INTCTL_PIO);
		break;
	case RALINK_GPIO_DISABLE_INTP:
		*(volatile u32 *)(RALINK_REG_INTDIS) = cpu_to_le32(RALINK_INTCTL_PIO);
		break;
	case RALINK_GPIO_REG_IRQ:
		copy_from_user(&info, (ralink_gpio_reg_info *)arg, sizeof(info));
		if (0 <= info.irq && info.irq < RALINK_GPIO_NUMBER) {
			ralink_gpio_info[info.irq].pid = info.pid;

			if (info.irq <= 31) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIORENA));
				tmp |= (0x1 << info.irq);
				*(volatile u32 *)(RALINK_REG_PIORENA) = cpu_to_le32(tmp);
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOFENA));
				tmp |= (0x1 << info.irq);
				*(volatile u32 *)(RALINK_REG_PIOFENA) = cpu_to_le32(tmp);
			} else if (info.irq <= 63) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332RENA));
				tmp |= (0x1 << (info.irq-32));
				*(volatile u32 *)(RALINK_REG_PIO6332RENA) = cpu_to_le32(tmp);
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332FENA));
				tmp |= (0x1 << (info.irq-32));
				*(volatile u32 *)(RALINK_REG_PIO6332FENA) = cpu_to_le32(tmp);
			} else {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564RENA));
				tmp |= (0x1 << (info.irq-64));
				*(volatile u32 *)(RALINK_REG_PIO9564RENA) = cpu_to_le32(tmp);
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564FENA));
				tmp |= (0x1 << (info.irq-64));
				*(volatile u32 *)(RALINK_REG_PIO9564FENA) = cpu_to_le32(tmp);
			}
		}
		else
			printk(KERN_ERR NAME ": irq number(%d) out of range\n",
					info.irq);
		break;

	case RALINK_GPIO6332_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO6332_SET_DIR_IN:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DIR));
		tmp &= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = tmp;
		break;
	case RALINK_GPIO6332_SET_DIR_OUT:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DIR));
		tmp |= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = tmp;
		break;
	case RALINK_GPIO6332_READ:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO6332_WRITE:
		*(volatile u32 *)(RALINK_REG_PIO6332DATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO6332_SET:
		*(volatile u32 *)(RALINK_REG_PIO6332SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO6332_CLEAR:
		*(volatile u32 *)(RALINK_REG_PIO6332SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_SET_DIR_IN:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DIR));
		tmp &= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = tmp;
		break;
	case RALINK_GPIO9564_SET_DIR_OUT:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DIR));
		tmp |= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = tmp;
		break;
	case RALINK_GPIO9564_READ:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO9564_WRITE:
		*(volatile u32 *)(RALINK_REG_PIO9564DATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_SET:
		*(volatile u32 *)(RALINK_REG_PIO9564SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_CLEAR:
		*(volatile u32 *)(RALINK_REG_PIO9564SET) = cpu_to_le32(arg);
		break;

	case RALINK_GPIO_LED_SET:
		printk(KERN_ERR NAME ": gpio led support not built\n");
		break;

	default:
		return -ENOIOCTLCMD;
	}
	return 0;

}


