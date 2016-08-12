
#include "pstarget.h"
#include "psglobal.h"

#include "ralink_gpio.h"
#include "eth_ra305x/if_ra305x.h"


int ralink_gpio_init (void)
{

    uint32 gpio_mode, io_value;

    //
    // set GPIO_MODE
    // use LINK0 as N/A         (GPIO43, input, reserve)
    // use LINK1 as N/A         (GPIO42, input, reserve)
    // use LINK2 as STATUS_LED  (GPIO41, output)
    // use LINK3 as USB_LED     (GPIO40, output)
    // use LINK4 as SW_RESET    (GPIO39, input)
    // use WDT_RST_N as WPS_KEY (GPIO38, input)
    //
    REG(RALINK_REG_GPIOMODE)  |= RALINK_GPIOMODE_WDT;
    REG(RALINK_REG_GPIOMODE2) |= (RALINK_GPIOMODE_EPHY0|
                                  RALINK_GPIOMODE_EPHY1|
                                  RALINK_GPIOMODE_EPHY2|
                                  RALINK_GPIOMODE_EPHY3|
                                  RALINK_GPIOMODE_EPHY4);
    // use wireless led as GPIO
    REG(RALINK_REG_GPIOMODE2) |= RALINK_GPIOMODE_WLED;
    //
    // set GPIO direction
    // 1 -> output
    // 0 -> input
    //
    REG(RALINK_REG_PIO6332DIR) = 0;
    REG(RALINK_REG_PIO6332DIR) |= (1 << PS_GPIO_POS_USB | 1 << PS_GPIO_POS_STATUS | 1 << PS_GPIO_POS_WIRELESS);

    // 
    // test function
    // 
    // light_usb_on();
    // light_status_on();
    // light_wireless_on();
    // if (get_reset_input())
    //     diag_printf("reset key press !\n");
    // if (get_wps_input())
    //     diag_printf("wps key press !\n");
}

inline void light_usb_on(void)
{
    REG(RALINK_REG_PIO6332DATA) &= (~(1 << PS_GPIO_POS_USB)); 
}

inline void light_usb_off(void)
{
    REG(RALINK_REG_PIO6332DATA) |= (1 << PS_GPIO_POS_USB);
}

inline void light_status_on(void)
{
    REG(RALINK_REG_PIO6332DATA) &= (~(1 << PS_GPIO_POS_STATUS));
}

inline void light_status_off(void)
{
    REG(RALINK_REG_PIO6332DATA) |= (1 << PS_GPIO_POS_STATUS);
}

inline void light_wireless_on(void)
{
    REG(RALINK_REG_PIO6332DATA) &= (~(1 << PS_GPIO_POS_WIRELESS));   
}

inline void light_wireless_off(void)
{
    REG(RALINK_REG_PIO6332DATA) |= (1 << PS_GPIO_POS_WIRELESS);
    
}

//
// return value > 0 indicate key press down
//
inline int get_reset_input(void)
{
    return REG(RALINK_REG_PIO6332DATA) & (1 << PS_GPIO_POS_RESET);   
}

//
// return value > 0 indicate key press down
//
inline int get_wps_input(void)
{
    return REG(RALINK_REG_PIO6332DATA) & (1 << PS_GPIO_POS_WPS); 
}

