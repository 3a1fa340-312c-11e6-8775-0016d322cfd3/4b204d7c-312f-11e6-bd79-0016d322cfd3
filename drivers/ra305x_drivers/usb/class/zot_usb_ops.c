 
#include "os-dep.h"

#define MAX_USB_NUM_PORTS	8
#define NUM_PORTS			1	//615wu	// number of USB ports 

u32 usb_devices_per_port[MAX_USB_NUM_PORTS] = {0}; // for QC only

u32 OHCI_LINK[MAX_USB_NUM_PORTS] = {0}; 
u32 EHCI_LINK[MAX_USB_NUM_PORTS] = {0}; 

int usb_hub_link_devices( int port )
{
	if( port >= 0 && port < NUM_PORTS )
		return usb_devices_per_port[port];
	return 0;
}

/* Ron Add for QC testing 8/31/04 */
unsigned int usb_device_detect(u8 *PortNum, u8 *PortState)
{
	int port;
	
	*PortNum = NUM_PORTS;
		
	for( port = 0; port < NUM_PORTS ; port++)
	{
//		*PortState = usb_devices_per_port[port];
		if( OHCI_LINK[port] && EHCI_LINK[port])
			return -1;
		if( OHCI_LINK[port] || EHCI_LINK[port])	
			*PortState = 1;
		else
			*PortState = 0;
		PortState++;
	}
	return 0;
}
