
#ifndef __RALINK_GPIO_H__
#define __RALINK_GPIO_H__

#include "rt_mmap.h"

/*
 * ioctl commands
 */
#define	RALINK_GPIO_SET_DIR		    0x01
#define RALINK_GPIO_SET_DIR_IN		0x11
#define RALINK_GPIO_SET_DIR_OUT		0x12
#define	RALINK_GPIO_READ		    0x02
#define	RALINK_GPIO_WRITE		    0x03
#define	RALINK_GPIO_SET			    0x21
#define	RALINK_GPIO_CLEAR		    0x31
#define	RALINK_GPIO_READ_INT		0x02 //same as read
#define	RALINK_GPIO_WRITE_INT		0x03 //same as write
#define	RALINK_GPIO_SET_INT		    0x21 //same as set
#define	RALINK_GPIO_CLEAR_INT		0x31 //same as clear
#define RALINK_GPIO_ENABLE_INTP		0x08
#define RALINK_GPIO_DISABLE_INTP	0x09
#define RALINK_GPIO_REG_IRQ		    0x0A
#define RALINK_GPIO_LED_SET		    0x41

#define	RALINK_GPIO6332_SET_DIR		0x51
#define RALINK_GPIO6332_SET_DIR_IN	0x13
#define RALINK_GPIO6332_SET_DIR_OUT	0x14
#define	RALINK_GPIO6332_READ		0x52
#define	RALINK_GPIO6332_WRITE		0x53
#define	RALINK_GPIO6332_SET		    0x22
#define	RALINK_GPIO6332_CLEAR		0x32

#define	RALINK_GPIO9564_SET_DIR		0x61
#define RALINK_GPIO9564_SET_DIR_IN	0x15
#define RALINK_GPIO9564_SET_DIR_OUT	0x16
#define	RALINK_GPIO9564_READ		0x62
#define	RALINK_GPIO9564_WRITE		0x63
#define	RALINK_GPIO9564_SET		    0x23
#define	RALINK_GPIO9564_CLEAR		0x33

/*
 * Address of RALINK_ Registers
 */
#define RALINK_SYSCTL_ADDR		    RALINK_SYSCTL_BASE	            // system control
#define RALINK_REG_GPIOMODE		    (RALINK_SYSCTL_ADDR + 0x60)
#define RALINK_REG_GPIOMODE2	    (RALINK_SYSCTL_ADDR + 0x64)

#define RALINK_IRQ_ADDR			    RALINK_INTCL_BASE
#define RALINK_PRGIO_ADDR		    RALINK_PIO_BASE                 // Programmable I/O


#define RALINK_REG_PIODIR		    (RALINK_PRGIO_ADDR + 0x00)
#define RALINK_REG_PIO6332DIR		(RALINK_PRGIO_ADDR + 0x04)
#define RALINK_REG_PIO9564DIR		(RALINK_PRGIO_ADDR + 0x08)

#define RALINK_REG_PIOPOL           (RALINK_PRGIO_ADDR + 0x10)
#define RALINK_REG_PIO6332POL       (RALINK_PRGIO_ADDR + 0x14)
#define RALINK_REG_PIO9565POL       (RALINK_PRGIO_ADDR + 0x18)

#define RALINK_REG_PIODATA		    (RALINK_PRGIO_ADDR + 0x20)
#define RALINK_REG_PIO6332DATA		(RALINK_PRGIO_ADDR + 0x24)
#define RALINK_REG_PIO9564DATA		(RALINK_PRGIO_ADDR + 0x28)

#define RALINK_REG_PIOSET		    (RALINK_PRGIO_ADDR + 0x30)
#define RALINK_REG_PIO6332SET		(RALINK_PRGIO_ADDR + 0x34)
#define RALINK_REG_PIO9564SET		(RALINK_PRGIO_ADDR + 0x38)

#define RALINK_REG_PIORESET		    (RALINK_PRGIO_ADDR + 0x40)
#define RALINK_REG_PIO6332RESET		(RALINK_PRGIO_ADDR + 0x44)
#define RALINK_REG_PIO9564RESET		(RALINK_PRGIO_ADDR + 0x48)

#define RALINK_REG_PIORENA		    (RALINK_PRGIO_ADDR + 0x50)
#define RALINK_REG_PIO6332RENA      (RALINK_PRGIO_ADDR + 0x54)
#define RALINK_REG_PIO9564RENA		(RALINK_PRGIO_ADDR + 0x58)

#define RALINK_REG_PIOFENA		    (RALINK_PRGIO_ADDR + 0x60)
#define RALINK_REG_PIO6332FENA		(RALINK_PRGIO_ADDR + 0x64)
#define RALINK_REG_PIO9564FENA		(RALINK_PRGIO_ADDR + 0x68)

#define RALINK_REG_PIOHLINT         (RALINK_PRGIO_ADDR + 0x70)
#define RALINK_REG_PIO6332HLINT     (RALINK_PRGIO_ADDR + 0x74)
#define RALINK_REG_PIO9564HLINT     (RALINK_PRGIO_ADDR + 0x78)
//#define RALINK_REG_INTDIS		    (RALINK_IRQ_ADDR   + 0x78)

//#define RALINK_REG_INTENA		    (RALINK_IRQ_ADDR   + 0x80)
#define RALINK_REG_PIOLLINT         (RALINK_PRGIO_ADDR + 0x80)
#define RALINK_REG_PIO6332LLINT     (RALINK_PRGIO_ADDR + 0x84)
#define RALINK_REG_PIO9564LLINT     (RALINK_PRGIO_ADDR + 0x88)

#define RALINK_REG_PIOINTATA	    (RALINK_PRGIO_ADDR + 0x90)
#define RALINK_REG_PIO6332INTSTA	(RALINK_PRGIO_ADDR + 0x94)
#define RALINK_REG_PIO9564INTSTA	(RALINK_PRGIO_ADDR + 0x98)

#define RALINK_REG_PIOEDGE		    (RALINK_PRGIO_ADDR + 0xA0)
#define RALINK_REG_PIO6332EDGE		(RALINK_PRGIO_ADDR + 0xA4)
#define RALINK_REG_PIO9564EDGE		(RALINK_PRGIO_ADDR + 0xA8)

// GPIO1_Mode Register (0x10000060) Value
#define RALINK_GPIOMODE_GPIO		0x1
#define RALINK_GPIOMODE_SPI_SLAVE	0x4             // (0x1 << 2)
#define RALINK_GPIOMODE_SPI_CS1		0x10            // (0x1 << 4)
#define RALINK_GPIOMODE_I2S		    0x40            // (0x1 << 6)
#define RALINK_GPIOMODE_UART0		0x100           // (0x1 << 8)
#define RALINK_GPIOMODE_SDXC		0x400           // (0x1 << 10)
#define RALINK_GPIOMODE_SPI		    0x1000          // (0x1 << 12)
#define RALINK_GPIOMODE_WDT		    0x4000          // (0x1 << 14)
#define RALINK_GPIOMODE_PERST		0x10000         // (0x1 << 16)
#define RALINK_GPIOMODE_REFCLK		0x40000         // (0x1 << 18)
#define RALINK_GPIOMODE_I2C		    0x100000        // (0x1 << 20)
#define RALINK_GPIOMODE_UART1       0x1000000       // (0x1 << 24)
#define RALINK_GPIOMODE_UART2       0x4000000       // (0x1 << 26)
#define RALINK_GPIOMODE_PWM0		0x10000000      // (0x1 << 28)
#define RALINK_GPIOMODE_PWM1		0x40000000      // (0x1 << 30)

// GPIO2_Mode Register (0x10000064) Value
#define RALINK_GPIOMODE_WLED        0x1
#define RALINK_GPIOMODE_EPHY0       0x4             // (0x1 << 2)
#define RALINK_GPIOMODE_EPHY1       0x10            // (0x1 << 4)
#define RALINK_GPIOMODE_EPHY2       0x40            // (0x1 << 6)
#define RALINK_GPIOMODE_EPHY3       0x100           // (0x1 << 8)
#define RALINK_GPIOMODE_EPHY4       0x400           // (0x1 << 10)

#define PS_GPIOMODE_BUTTON1         RALINK_GPIOMODE_EPHY4   // pin 139, GPIO39, mix EPHY_LED4_N_JTRST_N        
#define PS_GPIOMODE_BUTTON2         RALINK_GPIOMODE_WDT     // pin 137, GPIO38, mix WDT_RST_N
#define PS_GPIOMODE_STATUS          RALINK_GPIOMODE_EPHY2   // pin 141, GPIO41, mix EPHY_LED2_N_JTMS
#define PS_GPIOMODE_WIRELESS        RALINK_GPIOMODE_WLED    // pin 144, GPIO44, mix WLED_N
#define PS_GPIOMODE_USB             RALINK_GPIOMODE_EPHY3   // pin 140, GPIO40, mix EPHY_LED3_N_JTCLK

#define PS_BUTTON_1                 7
#define PS_BUTTON_2                 6
#define PS_LED_STATUS               9
#define PS_LED_WIRELESS             12
#define PS_LED_USB                  8

#define PS_GPIO_POS_WPS             6
#define PS_GPIO_POS_RESET           7
#define PS_GPIO_POS_USB             8               // (40 - 32)
#define PS_GPIO_POS_STATUS          9               // (41 - 32) 
#define PS_GPIO_POS_REV1            10              // (42 - 32) 
#define PS_GPIO_POS_REV2            11              // (43 - 32) 
#define PS_GPIO_POS_WIRELESS        12              // (44 - 32) 

#define RALINK_GPIO_NUMBER		    73

#define RALINK_GPIO_DATA_MASK		0xFFFFFFFF
#define RALINK_GPIO_DIR_IN		    0
#define RALINK_GPIO_DIR_OUT		    1
#define RALINK_GPIO_DIR_ALLIN		0
#define RALINK_GPIO_DIR_ALLOUT		0xFFFFFFFF
#define RALINK_GPIO_LED_INFINITY	4000

#define RALINK_GPIO(x)			    (1 << x)
#define REG(x)                      *(volatile u32 *)(x)

//
// Function List
//
int ralink_gpio_init(void);
//int ralink_gpio_ioctl (unsigned int req, unsigned long arg);
extern inline void light_usb_on(void);
extern inline void light_usb_off(void);
extern inline void light_status_on(void);
extern inline void light_status_off(void);
extern inline void light_wirless_on(void);
extern inline void light_wireless_off(void);
extern inline int get_reset_input(void);
extern inline int get_wps_input(void);
#endif
