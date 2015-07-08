/*===========================================================================
//        zotmain.c
//	Copy Righti(2006) ZOT.
//=========================================================================*/
#include <cyg/kernel/kapi.h>	// Kernel API.
#include <cyg/infra/diag.h>		// For diagnostic printing.

#include "pstarget.h"
#include "psglobal.h"
#include "star_intc.h"
#include "star_misc.h"



extern void LanPktInit();
extern void usbnet_init();

#define	SYSPA_USB11_OPERATION_BASE_ADDR		0xC4000000
#define	SYSPA_USB20_OPERATION_BASE_ADDR		0xCC000000

//ZOT716u2
#if	0
cyg_handle_t USB_INIT_SysClk;
cyg_handle_t USB_INIT_Counter;
cyg_handle_t USB_INIT_Alarm;
cyg_alarm USB_INIT_timerAlarm;

void star_usb_init(cyg_handle_t handle, cyg_addrword_t ptr)
{
	//Stop timer
	cyg_alarm_delete(USB_INIT_Alarm);
	cyg_clock_delete(USB_INIT_SysClk);
	cyg_counter_delete(USB_INIT_Counter);
	//init memory
	pci_usb_pool_init();
	//usb
	usb_init();
	//ohci
	ohci_hcd_init(0x5C, SYSPA_USB11_OPERATION_BASE_ADDR);
	//ehci
	ehci_hcd_init(0x6B, SYSPA_USB20_OPERATION_BASE_ADDR);	
	//Printer Class
	usblp_init();
	
}
#endif

#define USB_INIT_TASK_PRI         20	//ZOT716u2
#define USB_INIT_TASK_STACK_SIZE	 4096//512 //ZOT716u2 3072
static uint8 			USB_INIT_Stack[USB_INIT_TASK_STACK_SIZE];
static cyg_thread       USB_INIT_Task;
static cyg_handle_t     USB_INIT_TaskHdl;
extern int r8712u_drv_entry();
extern int rtusb_init();

void USB_init(cyg_addrword_t data)
{
	cyg_sem_t	sem;

	cyg_semaphore_init(&sem, 0);
	
	ppause(200);	//20 Ticks, 200m seconds
	
	//init memory
//ZOT==>	pci_usb_pool_init();
	//usb
	usb_init();
	//ohci
	ohci_hcd_init(0x5C, SYSPA_USB11_OPERATION_BASE_ADDR);
	//ehci
	ehci_hcd_init(0x6B, SYSPA_USB20_OPERATION_BASE_ADDR);	
	//Printer Class
	usblp_init();	
	
	ZOT_Timer_init();	//eason 20100210
	
	// Wireless initialization
	Wlan_MacInit();							// This line is important. wlanif.c
#ifdef MTK7601
	rtusb_init();	//rt3070
#else
	r8712u_drv_entry();
#endif
	cyg_semaphore_wait(&sem);
	cyg_semaphore_destroy(&sem);	
}

void zotmain( void )
{
	unsigned int i;
	
//Cache
	CacheEnable();
	
//IO
	IOInit();
	
//UART
	serial_init();
	serial_puts("ZOT 716u2W...\n");
	
//SPI FLASH INIT	
	AT91F_SpiInit();

//READ F/W Version
	read_version();
			
//EEPROM
	EEPROMInit();

//LWIP
	zot_network_init();

//MAC
	LanPktInit();
	star_nic_lan_init();
	LanPktStart();
	
//Print Server module
	IPXInitialize();
	NETBEUInit();
	Spooler_init();
	ps_init();

//LED
	LED_Init();	

//USB
	(*((u32 volatile *)(0xC0000004))) = 0x146;
	(*((u32 volatile *)(0xC0000044))) = 0x200;

	(*((u32 volatile *)(0xC8000004))) = 0x106;
	(*((u32 volatile *)(0xCC000040))) = (3 << 5) | 0x20000;	

//USB Ethernet Class
//#ifdef PreTest
	usbnet_init();//ax8817x
//#endif //PreTest

#if 0
    USB_INIT_SysClk = cyg_real_time_clock();

    cyg_clock_to_counter(USB_INIT_SysClk, &USB_INIT_Counter);
    cyg_alarm_create(USB_INIT_Counter, (cyg_alarm_t *)star_usb_init,
                     (cyg_addrword_t) 0,
                     &USB_INIT_Alarm, &USB_INIT_timerAlarm);

    /* This creates a periodic timer */
    cyg_alarm_initialize(USB_INIT_Alarm, cyg_current_time() +  20, 0); //20 Ticks, 200m seconds	   
#endif

	// USB initialization, including wireless initialization
    cyg_thread_create(USB_INIT_TASK_PRI,
                  USB_init,
                  0,
                  "USB-INIT",
                  (void *) (USB_INIT_Stack),
                  USB_INIT_TASK_STACK_SIZE,
                  &USB_INIT_TaskHdl,
                  &USB_INIT_Task);

	cyg_thread_resume(USB_INIT_TaskHdl);    

//IDLE task
	zot_idle_task_init();	

}

externC void
cyg_user_start( void )
{
    zotmain();
}

void perror(const char * str)
{}
