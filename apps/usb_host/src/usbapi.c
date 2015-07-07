#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "usb.h"
#include "hub.h"

struct pci_usb_desc
{
	uint8 data[95];
	uint8 occupy;
};

#define USB_PRINTER_WRITE_BUFFER	USBPRN_WRITE_BUFFER	//ZOT716u2 //8K
#define PCI_BAR_POOL_SIZE		0x5000	//615wu available size 20K Bytes

#define Bar0_Internal2PCI(x)			x
#define Bar0_PCI2Internal(x)			x
#define Bar1_Internal2PCI(x)			x
#define Bar1_PCI2Internal(x)			x

#define PCI_DESC_OCCUPY		1
#define PCI_DESC_AVAILABLE		0

#define OHCI_HCCA_SIZE			0x100   //256 byte,256-byte aligned.
#define OHCI_DEV_SIZE			0x700	//1.75K byte,16-byte aligned.

#define USB_DESC_SIZE			0x60	
#define USB_DESC_NUMBER			85

#define USB_CONTROL_DATA_BASE 	USB_CONTROL_DATA	//ZOT716u2
#define USB_CONTL_SETUP_ADDR	USB_CONTROL_DATA_BASE
#define USB_CONTL_SETUP_BASE	(USB_CONTL_SETUP_ADDR + sizeof(uint32))
#define USB_CONTL_BUFFER_ADDR	((USB_CONTL_SETUP_ADDR + 0x400) - sizeof(uint32))
#define USB_CONTL_BUFFER_BASE	(USB_CONTL_SETUP_ADDR + 0x400)		//Size:1K Bytes

/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
#define PCI_DMA_NONE		3

uint32 *PCI_BAR0_POOL_BASE;

uint32 pci_usb_pool_init()
{
	PCI_BAR0_POOL_BASE = USB_POOL_BASE;	//ZOT716u2
	memset(PCI_BAR0_POOL_BASE, 0x00, PCI_BAR_POOL_SIZE);
	return 0;
}

uint32 * pci_alloc_hcca(uint32 * framelist_dma)
{
	*framelist_dma = Bar0_Internal2PCI(PCI_BAR0_POOL_BASE);
	return PCI_BAR0_POOL_BASE;
}

uint32 * pci_alloc_ohci_dev(uint32 * dma)	//Printer and Hub share here, but Hub useless.
{
	*dma = Bar0_Internal2PCI((uint32)PCI_BAR0_POOL_BASE + OHCI_HCCA_SIZE );
	return ((uint32)PCI_BAR0_POOL_BASE + OHCI_HCCA_SIZE);;
}

void pci_free_ohci_dev(void)	
{	
	//Only free second OHCI device (Printer)
	uint32 * start_addr	= ((uint32)PCI_BAR0_POOL_BASE + OHCI_HCCA_SIZE ); //skip OHCI HCCA 256 Bytes
	memset(start_addr, 0x00, OHCI_DEV_SIZE);	
}

uint32 * pci_alloc_usb_desc(uint32 * dma_addr)
{
	uint32 * start_addr;
	uint32 * desc_addr;
	struct pci_usb_desc * pci_usb_desc;
	int i;
	int i_state = dirps();
	
	start_addr = (uint32)PCI_BAR0_POOL_BASE + OHCI_HCCA_SIZE + OHCI_DEV_SIZE;	//skip OHCI HCCA 256 Bytes, OHCI device and reserved memory
	//found an available space
	for ( i = 0; i < USB_DESC_NUMBER; i++)
    {
    	pci_usb_desc = desc_addr = (uint32)start_addr + ( i * USB_DESC_SIZE );
        if ( pci_usb_desc -> occupy == PCI_DESC_AVAILABLE)
        {
            break;
        }
    }
	
    if ( i == USB_DESC_NUMBER )   // out of space
   	{
   		restore(i_state);
   		return -1;
   	}
   	
   	memset(pci_usb_desc, 0x00,95);
   	//Set Occupy
   	pci_usb_desc -> occupy = PCI_DESC_OCCUPY;
   	//PCI remap
	*dma_addr = Bar0_Internal2PCI(desc_addr);
	
	restore(i_state);
	return desc_addr;
}

void pci_free_usb_desc(uint32 addr)
{
	struct pci_usb_desc * pci_usb_desc = Bar0_PCI2Internal(addr);
	//Set Available
	pci_usb_desc -> occupy = PCI_DESC_AVAILABLE;
}

uint32 pci_data_map(uint32 * addr, uint32 len, uint32 direc, uint32 setup)
{
	int i_state = dirps();
	uint32 *setup_addr = USB_CONTL_SETUP_ADDR;
	uint32 *setup_base = USB_CONTL_SETUP_BASE;
	uint32 *buffer_addr = USB_CONTL_BUFFER_ADDR;
	uint32 *buffer_base = USB_CONTL_BUFFER_BASE;
	
	if (((uint32)addr > (USB_PRINTER_WRITE_BUFFER + 0x2000))||((uint32)addr < USB_PRINTER_WRITE_BUFFER))
	{
		//Not Print Data (Control type used)
		if (setup)
		{
			*setup_addr = (uint32)addr;
			memcpy(setup_base, addr, len);
			restore(i_state);

			return Bar0_Internal2PCI(setup_base);
		}else{
			*buffer_addr = (uint32)addr;
			if (direc == PCI_DMA_TODEVICE)		
				memcpy(buffer_base, addr, len);
			restore(i_state);

			return Bar0_Internal2PCI(buffer_base);
		}	
	}else{
		//Print Data
		restore(i_state);

		return (Bar0_Internal2PCI(addr));
	}
}

uint32 pci_data_unmap(uint32 dma, uint32 len, uint32 direc, uint32 setup)
{
	uint32 *addr = Bar0_PCI2Internal(dma);
	uint32 *buffer_addr = USB_CONTL_BUFFER_ADDR;
	uint32 *buffer_base = USB_CONTL_BUFFER_BASE;
	uint32 *org_addr;
	

	if (((uint32)addr > USB_CONTROL_DATA_BASE)||((uint32)addr < (USB_CONTROL_DATA_BASE + 0x800)))
	{
		//Not Print Data (Control type used)
		if (setup)
			return 0;
		else{
			if (direc == PCI_DMA_FROMDEVICE)	
			{
				org_addr = (uint32)(*buffer_addr);
				memcpy(org_addr, buffer_base, len);
			}
			return 0;
		}
	}else{
		//Print Data
		return 0;
	}
}

void pci_data_sync(uint32 dma, uint32 len, uint32 direc, uint32 setup)
{
	uint32 * addr = Bar0_PCI2Internal(dma);
	uint32 *setup_addr = USB_CONTL_SETUP_ADDR;
	uint32 *setup_base = USB_CONTL_SETUP_BASE;
	uint32 *buffer_addr = USB_CONTL_BUFFER_ADDR;
	uint32 *buffer_base = USB_CONTL_BUFFER_BASE;
	uint32 *setup_org_addr, *buffer_org_addr;
	
	if (((uint32)addr > USB_CONTROL_DATA_BASE)||((uint32)addr < (USB_CONTROL_DATA_BASE + 0x800)))
	{
		//Not Print Data (Control type used)
		if (setup)
		{
			setup_org_addr = (uint32)(*setup_addr);
			memcpy(setup_base, setup_org_addr, len);
		}else{
			if (direc == PCI_DMA_FROMDEVICE)	
			{	
				buffer_org_addr = (uint32)(*buffer_addr);
				memcpy(buffer_org_addr, buffer_base, len);
			}else if (direc == PCI_DMA_TODEVICE)		
			{
				buffer_org_addr = (uint32)(*buffer_addr);
				memcpy(buffer_base, buffer_org_addr, len);
			}
		}
	}
}