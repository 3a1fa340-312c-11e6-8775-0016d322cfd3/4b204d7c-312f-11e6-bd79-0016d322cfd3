/******************************************************************************
* mlme_linux.c                                                                                                                                 *
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


#define _MLME_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <mlme_osdep.h>
#if 0 //eason 20100210
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
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif
#include <net/iw_handler.h>
#endif //eason 20100210
extern _adapter *Global_padapter; //eason 20100210
/* eason 20100210 Max number of char in custom event - use multiple of them if needed */
#define IW_CUSTOM_MAX		256	/* In bytes */

#ifdef RTK_DMP_PLATFORM
void Linkup_workitem_callback(struct work_struct *work)
{
	struct mlme_priv *pmlmepriv = container_of(work, struct mlme_priv, Linkup_workitem);
	_adapter *padapter = container_of(pmlmepriv, _adapter, mlmepriv);

_func_enter_;

	RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("+ Linkup_workitem_callback\n"));

	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_LINKUP);

_func_exit_;
}

void Linkdown_workitem_callback(struct work_struct *work)
{
	struct mlme_priv *pmlmepriv = container_of(work, struct mlme_priv, Linkdown_workitem);
	_adapter *padapter = container_of(pmlmepriv, _adapter, mlmepriv);

_func_enter_;

	RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("+ Linkdown_workitem_callback\n"));

	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_LINKDOWN);

_func_exit_;
}
#endif

void sitesurvey_ctrl_handler(void *FunctionContext)
{
	//eason 20100210 _adapter *adapter = (_adapter *)FunctionContext;
	_adapter *adapter = (_adapter *)Global_padapter;

	_sitesurvey_ctrl_handler(adapter);

	_set_timer(&adapter->mlmepriv.sitesurveyctrl.sitesurvey_ctrl_timer, 3000);
}

void join_timeout_handler (void *FunctionContext)
{
	//eason 20100210 _adapter *adapter = (_adapter *)FunctionContext;
	_adapter *adapter = (_adapter *)Global_padapter;
	_join_timeout_handler(adapter);
}


void _scan_timeout_handler (void *FunctionContext)
{
	//eason 20100210 _adapter *adapter = (_adapter *)FunctionContext;
	_adapter *adapter = (_adapter *)Global_padapter;
	scan_timeout_handler(adapter);
}

void dhcp_timeout_handler (void *FunctionContext)
{
	//eason 20100210 _adapter *adapter = (_adapter *)FunctionContext;
	_adapter *adapter = (_adapter *)Global_padapter;
	_dhcp_timeout_handler(adapter);
}

void wdg_timeout_handler (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;
	
	_wdg_timeout_handler(adapter);
	
	_set_timer(&adapter->mlmepriv.wdg_timer, 2000);
}

void init_mlme_timer(_adapter *padapter)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_init_timer(&(pmlmepriv->assoc_timer), padapter->pnetdev, join_timeout_handler, (pmlmepriv->nic_hdl));
	_init_timer(&(pmlmepriv->sitesurveyctrl.sitesurvey_ctrl_timer), padapter->pnetdev, sitesurvey_ctrl_handler, (u8 *)(pmlmepriv->nic_hdl));
	_init_timer(&(pmlmepriv->scan_to_timer), padapter->pnetdev, _scan_timeout_handler, (pmlmepriv->nic_hdl));
#ifdef CONFIG_PWRCTRL
	_init_timer(&(pmlmepriv->dhcp_timer), padapter->pnetdev, dhcp_timeout_handler, (u8 *)(pmlmepriv->nic_hdl));
#endif
	
	_init_timer(&(pmlmepriv->wdg_timer), padapter->pnetdev, wdg_timeout_handler, (u8 *)(pmlmepriv->nic_hdl));
	
		//2010-04-16 by Thomas
	// remove fw detection for hw pbc relative function
	//_init_timer(&(pmlmepriv->hw_pbc_timer), padapter->pnetdev, hw_pbc_timeout_handler, (u8 *)(pmlmepriv->nic_hdl));
	//_init_workitem(&(pmlmepriv->hw_pbc_workitem), hw_pbc_workitem_callback, padapter);

#ifdef RTK_DMP_PLATFORM
	_init_workitem(&(pmlmepriv->Linkup_workitem), Linkup_workitem_callback, padapter);
	_init_workitem(&(pmlmepriv->Linkdown_workitem), Linkdown_workitem_callback, padapter);
#endif
}

extern void indicate_wx_assoc_event(_adapter *padapter);
extern void indicate_wx_disassoc_event(_adapter *padapter);

void os_indicate_connect(_adapter *adapter)
{

_func_enter_;	

        indicate_wx_assoc_event(adapter);
//eason 20100210	netif_carrier_on(adapter->pnetdev);
#ifdef RTK_DMP_PLATFORM
	_set_workitem(&adapter->mlmepriv.Linkup_workitem);
#endif
_func_exit_;	

}

static RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];
void os_indicate_disconnect( _adapter *adapter )
{
   //RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];
   u8              backupPMKIDIndex = 0;
   u8              backupTKIPCountermeasure = 0x00;
      
_func_enter_;

   indicate_wx_disassoc_event(adapter);	
  //eason 20100210 netif_carrier_off(adapter->pnetdev);
#ifdef RTK_DMP_PLATFORM
	_set_workitem(&adapter->mlmepriv.Linkdown_workitem);
#endif

   if(adapter->securitypriv.dot11AuthAlgrthm == 2)//802.1x
   {		 
        // Added by Albert 2009/02/18
        // We have to backup the PMK information for WiFi PMK Caching test item.
        //
        // Backup the btkip_countermeasure information.
        // When the countermeasure is trigger, the driver have to disconnect with AP for 60 seconds.
        
        _memset( &backupPMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );

        _memcpy( &backupPMKIDList[ 0 ], &adapter->securitypriv.PMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
        backupPMKIDIndex = adapter->securitypriv.PMKIDIndex;
        backupTKIPCountermeasure = adapter->securitypriv.btkip_countermeasure;

       	_memset((unsigned char *)&adapter->securitypriv, 0, sizeof (struct security_priv));
       _init_timer(&(adapter->securitypriv.tkip_timer),adapter->pnetdev, use_tkipkey_handler, adapter);

       // Added by Albert 2009/02/18
       // Restore the PMK information to securitypriv structure for the following connection.
       _memcpy( &adapter->securitypriv.PMKIDList[ 0 ], &backupPMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
       adapter->securitypriv.PMKIDIndex = backupPMKIDIndex;
       adapter->securitypriv.btkip_countermeasure = backupTKIPCountermeasure;

   }
   else //reset values in securitypriv 
   {
	struct security_priv *psec_priv=&adapter->securitypriv;

	psec_priv->dot11AuthAlgrthm = 0; //open system
	psec_priv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
	psec_priv->dot11PrivacyKeyIndex = 0;

	psec_priv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psec_priv->dot118021XGrpKeyid = 1;

	psec_priv->ndisauthtype = Ndis802_11AuthModeOpen;
	psec_priv->ndisencryptstatus = Ndis802_11WEPDisabled;
	
	psec_priv->wps_phase = _FALSE;	
   }

_func_exit_;

}


void report_sec_ie(_adapter *adapter,u8 authmode,u8 *sec_ie)
{
		uint len;
		u8 *buff,*p,i;
		union iwreq_data wrqu;

_func_enter_;

	RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("+report_sec_ie, authmode=%d\n", authmode));

	buff = NULL;
	if(authmode==_WPA_IE_ID_)
	{
		RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("report_sec_ie, authmode=%d\n", authmode));
		
		buff = _malloc(IW_CUSTOM_MAX);
		
		if(buff == NULL) {
			RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("report_sec_ie, _malloc fail !\n"));
			return;
		}
		
		_memset(buff,0,IW_CUSTOM_MAX);
		
		p=buff;
		
		p+=sprintf(p,"ASSOCINFO(ReqIEs=");

		len = sec_ie[1]+2;
		len =  (len < IW_CUSTOM_MAX) ? len:IW_CUSTOM_MAX;
			
		for(i=0;i<len;i++){
			p+=sprintf(p,"%02x",sec_ie[i]);
		}

		p+=sprintf(p,")");
		
		_memset(&wrqu,0,sizeof(wrqu));
		
		wrqu.data.length=p-buff;
		
		wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;
		
		wireless_send_event(adapter->pnetdev,IWEVCUSTOM,&wrqu,buff);

		if(buff)
		    _mfree(buff, IW_CUSTOM_MAX);
		
	}

_func_exit_;

}

#ifdef CONFIG_MLME_EXT

void _survey_timer_hdl (void *FunctionContext)
{
	//eason 20100210 _adapter *padapter = (_adapter *)FunctionContext;
	_adapter *padapter = (_adapter *)Global_padapter;
	survey_timer_hdl(padapter);
}

void _reauth_timer_hdl(void *FunctionContext)
{
	//eason 20100210 _adapter *padapter = (_adapter *)FunctionContext;
	_adapter *padapter = (_adapter *)Global_padapter;
	reauth_timer_hdl(padapter);
}

void _reassoc_timer_hdl(void *FunctionContext)
{
	//eason 20100210 _adapter *padapter = (_adapter *)FunctionContext;
	_adapter *padapter = (_adapter *)Global_padapter;
	reassoc_timer_hdl(padapter);
}

void init_mlme_ext_timer(_adapter *padapter)
{	
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	_init_timer(&pmlmeext->survey_timer, padapter->pnetdev, _survey_timer_hdl, padapter);
	_init_timer(&pmlmeext->reauth_timer, padapter->pnetdev, _reauth_timer_hdl, padapter);
	_init_timer(&pmlmeext->reassoc_timer, padapter->pnetdev, _reassoc_timer_hdl, padapter);
}

#endif
