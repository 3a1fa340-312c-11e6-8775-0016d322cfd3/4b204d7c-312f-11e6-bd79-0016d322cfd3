/******************************************************************************
* io_linux.c                                                                                                                                 *
*                                                                                                                                          *
* Description :                                                                                                                       *
*                                                                                                                                           *
* Author :                                                                                                                       *
*                                                                                                                                         *
* History :                                                          
*
*                                        
*                                                                                                                                       *
* Copyright 2007, Realtek Corp.                                                                                                  *
*                                                                                                                                        *
* The contents of this file is the sole property of Realtek Corp.  It can not be                                     *
* be used, copied or modified without written permission from Realtek Corp.                                         *
*                                                                                                                                          *
*******************************************************************************/
#define _IO_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

#if 0//eason 20100210	
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/smp_lock.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
#include <linux/usb_ch9.h>
#else
#include <linux/usb/ch9.h>
#endif
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

#include <linux/firmware.h>

#ifdef CONFIG_SDIO_HCI
	#include<linux/mmc/sdio_func.h>
#endif
#endif //eason 20100210	
#ifdef CONFIG_EMBEDDED_FWIMG
#include <farray.h>
#endif


u32 rtl871x_open_fw(_adapter * padapter, void **pphfwfile_hdl, u8 **ppmappedfw)
{
	u32 len;
	
#ifdef  CONFIG_EMBEDDED_FWIMG	

	*ppmappedfw = f_array;
       len = sizeof(f_array);		   
	   
#else

	int rc;		
	const struct firmware **praw = (const struct firmware **)(pphfwfile_hdl);	
	
#ifdef CONFIG_SDIO_HCI		
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)(&padapter->dvobjpriv);
	struct sdio_func *func= pdvobjpriv->func;
	rc = request_firmware(praw, "rtl8712fw.bin",&func->dev);
#endif

#ifdef CONFIG_USB_HCI
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)(&padapter->dvobjpriv);
	struct usb_device *pusbdev=pdvobjpriv->pusbdev;
	rc = request_firmware(praw, "rtl8712fw.bin", &pusbdev->dev);
	RT_TRACE(_module_io_osdep_c_,_drv_info_,("request_firmware: Reason 0x%.8x\n", rc));
#endif

       if (rc < 0) {
		RT_TRACE(_module_io_osdep_c_,_drv_err_,("request_firmware failed: Reason 0x%.8x\n", rc));
		len = 0;
	}else{
	       *ppmappedfw = (u8 *)((*praw)->data);
       	len = (*praw)->size;
		
	}

#endif

 	return len;

}


void rtl871x_close_fw(_adapter *padapter, void *phfwfile_hdl)
{

#ifndef  CONFIG_EMBEDDED_FWIMG		   

	    struct firmware *praw = (struct firmware *)phfwfile_hdl;
	    if(praw)	   
           	release_firmware(praw);
#endif

}



