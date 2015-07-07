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
#define USB_INIT_TASK_STACK_SIZE	 512 //ZOT716u2 3072
static uint8 			USB_INIT_Stack[USB_INIT_TASK_STACK_SIZE];
static cyg_thread       USB_INIT_Task;
static cyg_handle_t     USB_INIT_TaskHdl;

void USB_init(cyg_addrword_t data)
{
	cyg_sem_t	sem;

	cyg_semaphore_init(&sem, 0);
	
	ppause(200);	//20 Ticks, 200m seconds
	
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

	ps_init();

//LED
	LED_Init();	

}

externC void
cyg_user_start( void )
{
    zotmain();
}

void perror(const char * str)
{}
