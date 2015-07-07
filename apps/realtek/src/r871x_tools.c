
#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#include "rtl871x_mp_phy_regdef.h"
#include "wlanif.h"
#include "r871x_tools.h"
#include <rtl8712_cmd.h>
#include <if_ether.h>

void* r871x_pkt_xmit(struct wpsctrl *arg);
int	r8711priv_ioctl(int skfd, unsigned int direction, unsigned int iocode, void *pInBuffer, unsigned int InBufferSize, void *pOutBuffer, unsigned int OutBufferSize);
extern _adapter *Global_padapter; //eason 20100730

void emi_task(cyg_addrword_t data)
{
	_adapter *Adapter = (_adapter *)Global_padapter;
	u32		ratevalue;
	int	mp_mode;
	int 	i, n;
	struct wpsctrl *pwpsc = NULL;
	struct wpsctrl_adapter_ops wpsc_ops;		
//------------------------------------------------------------------
	pwpsc = malloc(sizeof(struct wpsctrl));
	if (pwpsc == NULL) return 0;
	memset(pwpsc, 0, sizeof(struct wpsctrl));
	pwpsc->pwpsc_ops = &wpsc_ops;

	//default
	mp_mode = 1;
	strncpy(pwpsc->ifrn_name, "wlan0", IFNAMSIZ);

	if ((!r871x_mp_init(pwpsc)) || (pwpsc==NULL)) {
	//	printf("mp_init fail\n");
		free(pwpsc);
	}

	if (mp_mode)//r871x_set_mp_start(pwpsc);
		mpstart_hdl(pwpsc, NULL);
//-----------------------------------------------------------------
	Adapter->mppriv.curr_ch = pwpsc->chan_index = mvChannel;
	SetChannel(Adapter);
	
	Adapter->mppriv.curr_txpoweridx = pwpsc->power_index = mvTxPower;
	SetTxPower(Adapter);
	
	if (mvBandWidth != HT_CHANNEL_WIDTH_20)
		mvBandWidth = HT_CHANNEL_WIDTH_40;
	Adapter->mppriv.curr_bandwidth = pwpsc->bandwidth = mvBandWidth;
	SwitchBandwidth(Adapter);	
	
/*	if ( mvExtRate !=0 )
	{
		switch(mvExtRate)
		{
		    case 0x1: //bit0 (6M)
		        ratevalue = 3;
				break;
			case 0x2: //bit1 (9M)
		        ratevalue = 4;
				break;
			case 0x4: //bit2 (12M)
		        ratevalue = 6;
				break;
			case 0x8: //bit3 (18M)
		        ratevalue = 7;
				break;
			case 0x10: //bit4 (24M)
		        ratevalue = 8;
				break;
			case 0x20: //bit5 (36M)
		        ratevalue = 9;
				break;
			case 0x40: //bit6 (48M)
		        ratevalue = 10;
				break;
			case 0x80: //bit7 (54M)
		        ratevalue = 11;
				break;		
			case 0xFF: //auto
		    default:
		        ratevalue = 11;
	    		break;				
		}
	}
	else
	{
		switch(mvRate)
		{
			case 1:  //bit0 (1M)
		        ratevalue = 0;
				break;
			case 3:  //bit1 (2M)
		        ratevalue = 1;
				break;
			case 4:  //bit2 (5.5M)
		        ratevalue = 2;
				break;
			case 8:  //bit3 (11M)
		        ratevalue = 5;
				break;				
			case 15: //bit0, bit1, bit2, bit3 (1,2,5.5,11M) auto
			default:
		        ratevalue = 11;
				break;
		}
	}
	Adapter->mppriv.curr_rateidx = pwpsc->rate_index = ratevalue;*/
	Adapter->mppriv.curr_rateidx = pwpsc->rate_index = mvDataRate;
	SetDataRate(Adapter);
	
	if(mvCTX == 1)	
		set_continuous_tx_hdl(pwpsc, mvCTX);//set_pkt_tx_hdl(pwpsc, mvCTX);
		
	if(mvCRX == 1)	
		get_pkt_phy_rx_hdl(pwpsc, mvCRX);	

	free((void *)pwpsc);

}

//----------------------------------------------------------------------------------------
//MP_START_MODE
int r871x_mp_init(struct wpsctrl *pwpsc)
{
	int skfd, skpkt;

	//eason 20100730	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	skfd = 1;
	if (skfd < 0) return FALSE;

	//eason 20100730	skpkt = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	skpkt = 1;
	if (skpkt < 0) return FALSE;

//	pwpsc = malloc(sizeof(struct wpsctrl));
	pwpsc->enter_mp_test_mode = 0;
	pwpsc->priv_ptr = NULL;
	pwpsc->skfd = skfd;
	pwpsc->skpkt = skpkt;
//eason 20100730	pthread_mutex_init(&pwpsc->iocontrol_mutex, NULL);
	_spinlock_init(&pwpsc->iocontrol_mutex);
	
	//register ops. function
	pwpsc->pwpsc_ops->init = NULL;
	pwpsc->pwpsc_ops->iocontrol = r8711priv_ioctl;

	// basic
	// antenna 1T2R
	SET_TX_ANTENNA(pwpsc->antenna, ANTENNA_A);
	SET_RX_ANTENNA(pwpsc->antenna, ANTENNA_AB);

	pwpsc->bGENTSSI = FALSE;
	pwpsc->bstarttx = FALSE;
	pwpsc->bstartrx = FALSE;

	pwpsc->tx_pkt_cnts = 10;
	pwpsc->rx_pkt_ok_cnts = 0;
	pwpsc->rx_pkt_err_cnts = 0;

	pwpsc->payload_len = 10;
	pwpsc->payload_type = 0;
	pwpsc->dig_mode = 1;

	memset(pwpsc->da, 0xFF, ETH_ALEN);
//	memset(pwpsc->TxDesc, 0, sizeof(tx_desc_8712);

//	memset(pwpsc->tssi_table.cck, 0, sizeof(unsigned char)*4);
//	memset(pwpsc->tssi_table.ofdm, 0, sizeof(unsigned char)*4);

//	memset(pwpsc->eeprom_info.TSSI.CCK, 0, sizeof(unsigned char)*4);
//	memset(pwpsc->eeprom_info.TSSI.OFDM, 0, sizeof(unsigned char)*4);

	memset(pwpsc->efuse_map, 0xFF, EFUSE_MAP_MAX_SIZE);

	return TRUE;
}

//----------------------------------------------------------------------------------------
#if 0	//eason 20100802
int r8711priv_ioctl(int skfd, unsigned int direction, unsigned int iocode, void *pInBuffer, unsigned int InBufferSize,
					void *pOutBuffer, unsigned int OutBufferSize)
{	
	int cmd, ret;
	//eason 20100802	struct iwreq iwr;
	struct iw_point	data;
	ret = 0;

	memset(&iwr, 0, sizeof(struct iwreq));
	//printf("interface=%s\n", pwpsc->ifrn_name);
//	printf("interface=%s\n", interface);
	strncpy(iwr.ifr_ifrn.ifrn_name, interface, sizeof(iwr.ifr_ifrn.ifrn_name));
	//strncpy(iwr.ifr_ifrn.ifrn_name, pwpsc->ifrn_name, sizeof(iwr.ifr_ifrn.ifrn_name));

#ifdef RTL8192SU
	cmd =  SIOCIWFIRSTPRIV + 15;
#else
	cmd =  iocode;//SIOCIWFIRSTPRIV + 0x3
#endif

	iwr.u.data.pointer = pInBuffer;
	iwr.u.data.length = (unsigned short)InBufferSize;
	iwr.u.data.flags = direction;//set/query info

	ret = ioctl(skfd, cmd, &iwr);

	if(direction == 0x00)//query
	{
		if(pOutBuffer)
		{
			memcpy(pOutBuffer, iwr.u.data.pointer, OutBufferSize);
		}
	}
	return ret;
}
#else
int r8711priv_ioctl(int skfd, unsigned int direction, unsigned int iocode, void *pInBuffer, unsigned int InBufferSize,
					void *pOutBuffer, unsigned int OutBufferSize)
{
	int cmd, ret;
	struct iw_point	data;
	ret = 0;

	memset(&data, 0, sizeof(struct iw_point));

#ifdef RTL8192SU
	cmd =  SIOCIWFIRSTPRIV + 15;
#else
	cmd =  iocode;//SIOCIWFIRSTPRIV + 0x3
#endif
	data.pointer = pInBuffer;
	data.length = (unsigned short)InBufferSize;
	data.flags = direction;//set/query info

	ret = r871x_mp_ioctl_hdl(&data);

	if(direction == 0x00)//query
	{
		if(pOutBuffer)
		{
			memcpy(pOutBuffer, data.pointer, OutBufferSize);
		}
	}

	return ret;
}
#endif 	//eason 20100802	
//----------------------------------------------------------------------------------------
void mpstart_hdl(struct wpsctrl *pwpsc, char **param)
{
	if (!r871x_set_mp_start(pwpsc))
		pwpsc->enter_mp_test_mode = 1;
	else {
		//printf("$-mpstart_hdl - fail\n\n");
		//eason 20100730	exit(0);
	}
}

//----------------------------------------------------------------------------------------
int r871x_set_mp_start(struct wpsctrl *pwpsc)
{
	size_t msz;
	int ret;
	struct wpsctrl_adapter_ops	*pwpsc_ops;
	struct mp_ioctl_param		*pmpioctlparam = NULL;
	struct mp_status_param		*pmpstatus = NULL;

	msz = sizeof(struct mp_ioctl_param) + sizeof(struct mp_status_param);
	pmpioctlparam = (struct mp_ioctl_param *)malloc(msz);
	if (pmpioctlparam == NULL) return -1;

	memset(pmpioctlparam, 0, msz);
	pmpioctlparam->subcode = GEN_MP_IOCTL_SUBCODE(MP_START);
	pmpioctlparam->len = sizeof(struct mp_status_param);

	pmpstatus = (struct mp_status_param *)pmpioctlparam->data;
	pmpstatus->mode = _2MAC_MODE_;

	pwpsc_ops = pwpsc->pwpsc_ops;

	//eason 20100730	pthread_mutex_lock(&pwpsc->iocontrol_mutex);
	_spinlock(&pwpsc->iocontrol_mutex);

	ret = pwpsc_ops->iocontrol(pwpsc->skfd, IOCONTROL_SET, SIOCIWFIRSTPRIV + 0x3,
					(unsigned char *)(pmpioctlparam), msz,
					(unsigned char *)(pmpioctlparam), msz);				
	//eason 20100730	pthread_mutex_unlock(&pwpsc->iocontrol_mutex);
	_spinunlock(&pwpsc->iocontrol_mutex);

	free((void *)pmpioctlparam);
//eason 20100730	MPModeParameterAdjustment(pwpsc, TRUE);

	return ret;
}

//----------------------------------------------------------------------------------------
void set_continuous_tx_hdl(struct wpsctrl *pwpsc, char **param)
{
	int mode = 1;
#if 0	//eason 20100802	
	int mode = 0;
	if (!is_dec_str(param[0])) {
//		printf("%s  %s\n", paramerrstr, continuoustxstr);
		return;
	}

	sscanf(param[0], "%d", &mode);
	if ((mode < 0) ||(mode > 1)) {
//		printf("%s  %s\n", paramerrstr, continuoustxstr);
		return;
	}
#endif	//eason 20100802	
	pwpsc->mp_test_mode = CONTINUOUS_TX;

//	printf("$-set_continuous_tx_hdl - mode:%d\n", mode);
	if (r871x_set_continuous_tx(pwpsc, mode))
		return;//printf("FAIL!!\n");
	else
		r871x_set_pkt_xmit(pwpsc, mode);


}
//------------------------------------------------------------------------------
//This function initializes the DUT to the MP test mode
int mp_start_test(_adapter *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;

	NDIS_WLAN_BSSID_EX bssid;
	struct sta_info *psta;
	unsigned long length;

		_irqL irqL;
	int res = _SUCCESS;


	//3 1. initialize a new NDIS_WLAN_BSSID_EX
//	_memset(&bssid, 0, sizeof(NDIS_WLAN_BSSID_EX));

	_memcpy(bssid.MacAddress, pmppriv->network_macaddr, ETH_ALEN);
	bssid.Ssid.SsidLength = 16;
	_memcpy(bssid.Ssid.Ssid, (unsigned char*)"mp_pseudo_adhoc", bssid.Ssid.SsidLength);
	bssid.InfrastructureMode = Ndis802_11IBSS;
	bssid.NetworkTypeInUse = Ndis802_11DS;
	bssid.IELength = 0;

	length = get_NDIS_WLAN_BSSID_EX_sz(&bssid);
	if (length % 4)
		bssid.Length = ((length >> 2) + 1) << 2; //round up to multiple of 4 bytes.
	else
		bssid.Length = length;

	_enter_critical(&pmlmepriv->lock, &irqL);

	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
		goto end_of_mp_start_test;

	//init mp_start_test status
	pmppriv->prev_fw_state = get_fwstate(pmlmepriv);
	pmlmepriv->fw_state = WIFI_MP_STATE;

	if (pmppriv->mode == _LOOPBOOK_MODE_) {
		set_fwstate(pmlmepriv, WIFI_MP_LPBK_STATE); //append txdesc
		RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_notice_, ("+start mp in Lookback mode\n"));
	} else {
		RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_notice_, ("+start mp in normal mode\n"));
	}

	set_fwstate(pmlmepriv, _FW_UNDER_LINKING);

	//3 2. create a new psta for mp driver
	//clear psta in the cur_network, if any
	psta = get_stainfo(&padapter->stapriv, tgt_network->network.MacAddress);
	if (psta) free_stainfo(padapter, psta);

	psta = alloc_stainfo(&padapter->stapriv, bssid.MacAddress);
	if (psta == NULL) {
		RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_err_, ("mp_start_test: Can't alloc sta_info!\n"));
		res = _FAIL;
		goto end_of_mp_start_test;
	}

	//3 3. join psudo AdHoc
	tgt_network->join_res = 1;
	tgt_network->aid = psta->aid = 1;
	_memcpy(&tgt_network->network, &bssid, length);

	_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);
	os_indicate_connect(padapter);
	set_fwstate(pmlmepriv, _FW_LINKED); // Set to LINKED STATE for MP TRX Testing

end_of_mp_start_test:

	_exit_critical(&pmlmepriv->lock, &irqL);

	return res;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_start_test_hdl(struct oid_par_priv *poid_par_priv)
{
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

	_irqL		oldirql;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;

	u32		mode;
	u8		val8;

_func_enter_;

	RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_notice_, ("+oid_rt_pro_start_test_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return  NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);

	//IQCalibrateBcut(Adapter);

	mode = *((u32*)poid_par_priv->information_buf);
	Adapter->mppriv.mode = mode;// 1 for loopback

	if (mp_start_test(Adapter) == _FAIL)
		status = NDIS_STATUS_NOT_ACCEPTED;

	write8(Adapter, MSR, 1); // Link in ad hoc network, 0x1025004C
	write8(Adapter, RCR, 0); // RCR : disable all pkt, 0x10250048
	write8(Adapter, RCR+2, 0x57); // RCR disable Check BSSID, 0x1025004a

	//disable RX filter map , mgt frames will put in RX FIFO 0
	write16(Adapter, RXFLTMAP0, 0x0); // 0x10250116

	val8 = read8(Adapter, EE_9346CR); // 0x1025000A
	if (!(val8 & _9356SEL))//boot from EFUSE
	{
		efuse_reg_init(Adapter);
		efuse_change_max_size(Adapter);
		efuse_reg_uninit(Adapter);
	}

	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_notice_, ("-oid_rt_pro_start_test_hdl: mp_mode=%d\n", Adapter->mppriv.mode));

_func_exit_;

	return status;
}

//---------------------------------------------------------------------------------------------
int r871x_set_continuous_tx(struct wpsctrl *pwpsc, int bstart)
{
	size_t msz;
	int ret;
	struct wpsctrl_adapter_ops *pwpsc_ops;
	struct mp_ioctl_param *pmpioctlparam = NULL;

	msz = sizeof (struct mp_ioctl_param) + sizeof(unsigned int);
	pmpioctlparam = (struct mp_ioctl_param *)malloc(msz);
	if (pmpioctlparam == NULL) {
		return -1;
	}

	memset(pmpioctlparam, 0, msz);
	pmpioctlparam->subcode = GEN_MP_IOCTL_SUBCODE(CNTU_TX);
	pmpioctlparam->len = sizeof(unsigned int);
	*((unsigned int*)pmpioctlparam->data) = (unsigned int)bstart;

	pwpsc_ops = pwpsc->pwpsc_ops;

	//_mutex_lock(&pwpsc->iocontrol_mutex);
	//eason 20100802	pthread_mutex_lock(&pwpsc->iocontrol_mutex);
	_spinlock(&pwpsc->iocontrol_mutex);
	ret = pwpsc_ops->iocontrol(pwpsc->skfd, IOCONTROL_SET, SIOCIWFIRSTPRIV + 0x3,
							(unsigned char *)(pmpioctlparam), msz,
							(unsigned char *)(pmpioctlparam), msz);	
	//_mutex_unlock(&pwpsc->iocontrol_mutex);
	//eason 20100802	pthread_mutex_unlock(&pwpsc->iocontrol_mutex);
	_spinunlock(&pwpsc->iocontrol_mutex);
	
	free((void *)pmpioctlparam);

	return ret;
}

NDIS_STATUS oid_rt_pro_set_continuous_tx_hdl(struct oid_par_priv *poid_par_priv)
{
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

	_irqL		oldirql;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;

	u32		bStartTest;

_func_enter_;

	RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_notice_, ("+oid_rt_pro_set_continuous_tx_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	bStartTest = *((u32*)poid_par_priv->information_buf);

	_irqlevel_changed_(&oldirql, LOWER);
	SetContinuousTx(Adapter,(u8)bStartTest);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}


//pthread_t pkt_xmit_thread;
//pthread_attr_t attr;

void r871x_set_pkt_xmit(struct wpsctrl *pwpsc,int bstart)
{
	pwpsc->bstarttx = bstart;
	if (bstart == TRUE) {
#if	0//eason 20100802		
		int res = 0;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

#ifdef _PTM_SET_AUTO_
		if((pwpsc->bGENTSSI == FALSE) && (pwpsc->mp_test_mode == PACKET_TX))
		{
			r871x_set_ptm(pwpsc,_POWERTRACK_PKTTX_);
		}
#endif
		res = pthread_create(&pkt_xmit_thread, &attr, r871x_pkt_xmit, (void *)pwpsc);
		if (res) {
			//printf("Thread creation failed!!\n");
			//exit(EXIT_FAILURE);
		}
#else
		r871x_pkt_xmit(pwpsc);
#endif //eason 20100802		
	} else {
#ifdef _PTM_SET_AUTO_
		if((pwpsc->bGENTSSI == FALSE) && (pwpsc->mp_test_mode == PACKET_TX))
		{
			r871x_set_ptm(pwpsc,_POWERTRACK_OFF_);
		}
#endif
#ifdef _FORCE_CANCEL_
		if (!pthread_cancel(pkt_xmit_thread)) {
			//printf("$-pkt xmit pthread_cancel OK\n");
		}
#else
#if	0	//eason 20100802
		if (!pthread_join(pkt_xmit_thread, NULL)) {
			//printf("$-pkt tx stop\n");
			//abort();
		}
#endif	//eason 20100802		
#endif
	}
}

const int MAX_PKT_BUF_SIZE = 2048;
const int ETHER_HEADER_LEN = ETH_HLEN;	// 14
const int TxEthHeaderOffset = 0;
const int TxDescOffset = ETH_HLEN;//ETHER_HEADER_LEN = 2*ETH_ALEN + 2
const int TxDescSize = 32;
const int TxPayloadOffset = 46;//TxDescOffset + TxDescSize;
const int PAYLOAD_HEADER_LEN = 17;
const int CRC_LEN = 1;
const int PAYLOAD_FIX_LEN = 18;//PAYLOAD_HEADER_LEN + CRC_LEN;
const unsigned char HeaderSID[6] = {0x00, 0xE0, 0x4C, 0x12, 0x34, 0x56};
static const unsigned char payloaddata[] = {0x00,0x5a,0xa5,0xff};

const unsigned int crc_ta[16] = {	// CRC餘式表
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
};
//eason 20100802	void* r871x_pkt_xmit(void* arg)
void* r871x_pkt_xmit(struct wpsctrl *arg)
{
#ifdef _FORCE_CANCEL_
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

	int bContinuous = TRUE;
	int i;
	unsigned int Xmited = 0, pktlen = 0, datalen = 0;
	unsigned char pxmitbuff[MAX_PKT_BUF_SIZE], *ppayload;
	unsigned char crc = 0;

	struct wpsctrl *pwpsc = (struct wpsctrl *)arg;

	/*prepare sockaddr_ll*/
#if 0	//eason 20100804
	/*RAW communication*/
	pwpsc->socket_address.sll_family   = PF_PACKET;
	/*we don't use a protocoll above ethernet layer
	  ->just use anything here*/
	pwpsc->socket_address.sll_protocol = htons(ETH_P_IP);

	/*index of the network device
	see full code later how to retrieve it*/
	pwpsc->socket_address.sll_ifindex  = if_nametoindex(interface);

	/*ARP hardware identifier is ethernet*/
	pwpsc->socket_address.sll_hatype   = ARPHRD_ETHER;

	/*target is another host*/
	pwpsc->socket_address.sll_pkttype  = PACKET_OTHERHOST;

	/*address length*/
	pwpsc->socket_address.sll_halen    = ETH_ALEN;

	/*MAC - begin*/
	for (i = 0; i < ETH_ALEN; i++)
		pwpsc->socket_address.sll_addr[i] = pwpsc->da[i];

	/*MAC - end*/
	pwpsc->socket_address.sll_addr[ETH_ALEN]  = 0x00;/*not used*/
	pwpsc->socket_address.sll_addr[ETH_ALEN+1]  = 0x00;/*not used*/
#endif	//eason 20100804
	/* end of prepare sockaddr_ll */

	/* initial TxDesc */
	// in driver, only DWORD 3,5,6 used
	pwpsc->TxDesc.rty_lmt_en = 0;	// MAC dont retry
	pwpsc->TxDesc.dis_fb = 1;	// no rate fallback during retry

	pwpsc->TxDesc.use_rate = 1;
	if (pwpsc->rate_index > 0xB)
		pwpsc->TxDesc.tx_ht = 1;
	else
		pwpsc->TxDesc.tx_ht = 0;
	pwpsc->TxDesc.tx_rate = pwpsc->rate_index;

	pwpsc->TxDesc.tx_sc = 0;//Dont care
	pwpsc->TxDesc.tx_bw = pwpsc->bandwidth;

	pwpsc->TxDesc.txpktsize = PAYLOAD_FIX_LEN + pwpsc->payload_len; // not used
	/* end of initial TxDesc */

	memset(pxmitbuff, 0, sizeof(unsigned char)*MAX_PKT_BUF_SIZE);

	//ether 2 header - 14 bytes
	for(i=0; i<ETH_ALEN; i++) *(pxmitbuff + i)= pwpsc->da[i]; //DA, 0~5
	for(i=0; i<ETH_ALEN; i++) *(pxmitbuff + ETH_ALEN + i) = HeaderSID[i];//SA, 6~11
	*(pxmitbuff + (ETH_HLEN - 2)) = 0x87;	// 12
	*(pxmitbuff + (ETH_HLEN - 1)) = 0x12;	// 13

	// Tx Desc
	memcpy(pxmitbuff + TxDescOffset, &pwpsc->TxDesc, sizeof(struct _tx_desc_8712));

	// payload
	ppayload = pxmitbuff + TxPayloadOffset;

	// sequence number, 1~4
	*ppayload = 0x0;
	*(ppayload + 1) = 0x0;
	*(ppayload + 2) = 0x0;
	*(ppayload + 3) = 0x0;

	// data type, 5~6
	*(ppayload + 4) = pwpsc->payload_type & 0xFF;
	*(ppayload + 5) = (pwpsc->payload_type >> 8) & 0xFF;

	// Packet Len, 7~8
	datalen = pwpsc->payload_len;
	pktlen = PAYLOAD_FIX_LEN + datalen;
	*(ppayload + 6) = pktlen & 0xFF;
	*(ppayload + 7) = (pktlen >> 8) & 0xFF;

	// undefined, 9~17
	for (i = 8; i < PAYLOAD_HEADER_LEN; i++)
		*(ppayload + i) = (unsigned char)0xFF;

	// stuff
	ppayload += PAYLOAD_HEADER_LEN;
#if 1
	if (pwpsc->payload_type < 4)
		memset(ppayload, payloaddata[pwpsc->payload_type], datalen);
#else
	for (count = 0; count < datalen; count++)
		if (pwpsc->payload_type < 4)
			*(ppayload + count) = payloaddata[pwpsc->payload_type];
#endif
	// insert crc in last byte
	ppayload += datalen;
	crc = cal_crc(pxmitbuff + TxPayloadOffset, PAYLOAD_HEADER_LEN + datalen);
	*ppayload = crc;

	if (pwpsc->mp_test_mode != PACKET_TX) {
//		if (pwpsc->rate_index < 4)//CCK
//			pwpsc->tx_pkt_cnts = 4;
//		else
			pwpsc->tx_pkt_cnts = 1;
	}
/*eason 20100804	if (pwpsc->tx_pkt_cnts == 0)
		printf("$-TH:pkt xmit continued\n");
	else
		printf("$-TH:pkt_cnts:%d\n", pwpsc->tx_pkt_cnts);*/

	pktlen += ETHER_HEADER_LEN + TxDescSize;

	while (bContinuous)
	{
//		memcpy(pxmitbuff + TxPayloadOffset, &Xmited, 4);

		for (i = 0; i < 4; i++)
			*(pxmitbuff + TxPayloadOffset + i) = (Xmited >> (8*i)) & 0xFF;
		crc = cal_crc(pxmitbuff + TxPayloadOffset, PAYLOAD_HEADER_LEN + datalen);
		*ppayload = crc;

		//eason 20100804	if (packet_send(pwpsc, pxmitbuff, pktlen) == -1)
		if (send_eap_pkt(0, pxmitbuff, pktlen) < 0)
			return -1;//eason 20100804	printf("send packet error\n");

		Xmited++;
#ifdef _DBG_MSG_
	//	printf("$-TH:pkt %d sended\n",Xmited);
#endif

		if (pwpsc->mp_test_mode != PACKET_TX)
			ppause(2);//eason 20100804	sleep(2);
		
#ifdef _FORCE_CANCEL_
		pthread_testcancel();
#endif
		if (pwpsc->tx_pkt_cnts == 0) {
			bContinuous = pwpsc->bstarttx;
		} else {
			pwpsc->tx_pkt_cnts--;
			bContinuous = (pwpsc->bstarttx && pwpsc->tx_pkt_cnts);
		}
	}

#ifdef _DBG_MSG_
	//printf("$-TH:pkt thread exit~\n");
#endif
	return NULL;
}

static unsigned char cal_crc(const unsigned char *ptr, unsigned int len)
{
	unsigned int icrc = 0;
	unsigned short scrc = 0;
	unsigned char da;

	if (!ptr) return 0;

	while (len-- != 0) {
		da = ((unsigned char)(icrc/256))/16;	// save the high bits of CRC
		icrc <<= 4;			// right shift CRC 4 bits = take low 12 bits of CRC
		icrc ^= crc_ta[da^(*ptr/16)];	// CRC的高4位和本字的前半字元相加後查表算CRC，然後加上上一次CRC的餘數
		da = ((unsigned char)(icrc/256))/16;	// save the high bits of CRC
		icrc <<= 4;			// right shift CRC 4 bits is equal to take low 12 bits of CRC
		icrc ^= crc_ta[da^(*ptr&0x0f)];	// CRC的高4位和本字元的後半字元相加後查表算CRC,然後再加上上一次CRC的餘數
		ptr++;
	}
	scrc = ((icrc&0xFF000000)>>24)+((icrc&0x00FF0000)>>16)+((icrc&0x0000FF00)>>8)+(icrc&0x000000FF);
	return ((unsigned char)(scrc &0xFF));
}

int packet_send(struct wpsctrl *pwpsc, unsigned char *packet, int pktlen)
{
#if 0
	struct sockaddr *paddr = (struct sockaddr*)&pwpsc->socket_address;
	return sendto(pwpsc->skpkt, packet, pktlen, 0, paddr, sizeof(struct sockaddr_ll));
#endif
}

//----------------------------------------------------------------------------------
void get_pkt_phy_rx_hdl(struct wpsctrl *pwpsc, unsigned int param)
{
	int bstart = 0;
	unsigned int rx_ant = 0;
	bstart = param;	//eason 20100804
#if 0	//eason 20100804
	if (!is_dec_str(param[0])) {
	//	printf("%s  %s\n", paramerrstr, pktrxstr);
		return;
	}
	sscanf(param[0], "%d", &bstart);
	if ((bstart < 0) || (bstart > 1)) {
//		printf("%s  %s\n", paramerrstr, pktrxstr);
		return;
	}

	if ((param[1] != NULL) && is_dec_str(param[1])) {
		sscanf(param[1], "%d", &rx_ant);
		if ((rx_ant >= ANTENNA_A) && (rx_ant <= ANTENNA_AB)) {
			if (GET_RX_ANTENNA(pwpsc->antenna) != rx_ant) {
				unsigned int antenna = pwpsc->antenna;
				SET_RX_ANTENNA(pwpsc->antenna, rx_ant);
				if (r871x_set_antenna(pwpsc, pwpsc->antenna)) {
					pwpsc->antenna = antenna;
			//		printf("$-get_pkt_phy_rx_hdl - switch RX antenna FAIL!!\n");
					return;
				}
			}
		}
	}
#endif //eason 20100804
	pwpsc->mp_test_mode = PACKET_RX;

	rx_ant = GET_RX_ANTENNA(pwpsc->antenna); // update to new rx antenna
//	printf("$-get_pkt_phy_rx_hdl - mode:%d antenna:%d\n", bstart, rx_ant);
	r871x_get_pkt_recv(pwpsc, bstart);
//	printf("\n");
}

//----------------------------------------------------------------------------------
void r871x_get_pkt_recv(struct wpsctrl *pwpsc, int bstart)
{
	int res = 0;

	if (bstart)
	{
		pwpsc->bstartrx = TRUE;

		pwpsc->record_rx_pkt_ok_cnts = 0;
		pwpsc->record_rx_pkt_err_cnts = 0;

		pwpsc->rx_pkt_ok_cnts = 0;
		pwpsc->rx_pkt_err_cnts = 0;

		r871x_pkt_recv_reset(pwpsc);

		r871x_get_phy_rx_pkts_ok(pwpsc, &pwpsc->record_rx_pkt_ok_cnts);	
		r871x_get_phy_rx_pkts_error(pwpsc, &pwpsc->record_rx_pkt_err_cnts);

//eason 20100804		res = pthread_create(&pkt_recv_thread, NULL, phy_pkt_recv_thread, (void *)pwpsc);
		res = phy_pkt_recv_thread(pwpsc);
		if (res)
			return -1;	//eason 20100804	printf("rx thread creation failed!!\n");
		else {
		//	printf("press any key to quit\n");
		//eason20100804	getchar_nodelay();
			bstart = 0;
		}

	}

	if (!bstart) {
		pwpsc->bstartrx = FALSE;
/*eason 20100804		if (!pthread_join(pkt_recv_thread, NULL)) {
			printf("$-pkt rx stop.\n");
		}*/
	}
}

int r871x_pkt_recv_reset(struct wpsctrl *pwpsc)
{
	size_t msz;
	int ret;
	struct wpsctrl_adapter_ops *pwpsc_ops;
	struct mp_ioctl_param *pmpioctlparam = NULL;

	msz = sizeof(struct mp_ioctl_param);
	pmpioctlparam = (struct mp_ioctl_param *)malloc(msz);
	if (pmpioctlparam == NULL) {
		return -1;
	}
	memset(pmpioctlparam, 0, msz);

	pmpioctlparam->subcode = GEN_MP_IOCTL_SUBCODE(RESET_PHY_RX_PKT_CNT);
	pmpioctlparam->len = 0;

	pwpsc_ops = pwpsc->pwpsc_ops;

	//eason 20100802	pthread_mutex_lock(&pwpsc->iocontrol_mutex);
	_spinlock(&pwpsc->iocontrol_mutex);
	ret = pwpsc_ops->iocontrol(pwpsc->skfd, IOCONTROL_SET, SIOCIWFIRSTPRIV + 0x3,
							(unsigned char *)(pmpioctlparam), msz,
							(unsigned char *)(pmpioctlparam), msz);
	//_mutex_unlock(&pwpsc->iocontrol_mutex);
	//eason 20100802	pthread_mutex_unlock(&pwpsc->iocontrol_mutex);
	_spinunlock(&pwpsc->iocontrol_mutex);
	
	free((void *)pmpioctlparam);

	return ret;
}

//----------------------------------------------------------------------------------
int r871x_get_phy_rx_pkts_ok(struct wpsctrl *pwpsc,unsigned int* count)
{
#if 1
	size_t msz;
	int ret;
	struct wpsctrl_adapter_ops *pwpsc_ops;
	struct mp_ioctl_param *pmpioctlparam = NULL;

	msz = sizeof(struct mp_ioctl_param) + sizeof(unsigned int);
	pmpioctlparam = (struct mp_ioctl_param *)malloc(msz);
	if (pmpioctlparam == NULL) {
		return -1;
	}
	memset(pmpioctlparam, 0, msz);

	pmpioctlparam->subcode = GEN_MP_IOCTL_SUBCODE(GET_PHY_RX_PKT_RECV);
	pmpioctlparam->len = sizeof(unsigned int);

	pwpsc_ops = pwpsc->pwpsc_ops;

	//_mutex_lock(&pwpsc->iocontrol_mutex);
	_spinlock(&pwpsc->iocontrol_mutex);
	ret = pwpsc_ops->iocontrol(pwpsc->skfd, IOCONTROL_QUERY, SIOCIWFIRSTPRIV + 0x3,
							(unsigned char *)(pmpioctlparam), msz,
							(unsigned char *)(pmpioctlparam), msz);
	//_mutex_unlock(&pwpsc->iocontrol_mutex);
	_spinunlock(&pwpsc->iocontrol_mutex);

	*count = *(unsigned int*)pmpioctlparam->data;

	free((void *)pmpioctlparam);

	return ret;
#else
	unsigned int OFDM_cnt = 0, CCK_cnt = 0, HT_cnt = 0;

	GetRxPhyPacketCounts(pwpsc, OFDM_MPDU_OK_BIT, &OFDM_cnt);
	GetRxPhyPacketCounts(pwpsc, CCK_MPDU_OK_BIT, &CCK_cnt);
	GetRxPhyPacketCounts(pwpsc, HT_MPDU_OK_BIT, &HT_cnt);

	*count = OFDM_cnt + CCK_cnt + HT_cnt;
	return TRUE;
#endif
}

//----------------------------------------------------------------------------------
int r871x_get_phy_rx_pkts_error(struct wpsctrl *pwpsc, unsigned int *count)
{
#if 1
	size_t msz;
	int ret;
	struct wpsctrl_adapter_ops *pwpsc_ops;
	struct mp_ioctl_param *pmpioctlparam = NULL;

	msz = sizeof(struct mp_ioctl_param) + sizeof(unsigned int);
	pmpioctlparam = (struct mp_ioctl_param *)malloc(msz);
	if (pmpioctlparam == NULL) {
		return -1;
	}
	memset(pmpioctlparam, 0, msz);

	pmpioctlparam->subcode = GEN_MP_IOCTL_SUBCODE(GET_PHY_RX_PKT_ERROR);
	pmpioctlparam->len = sizeof(unsigned int);

	pwpsc_ops = pwpsc->pwpsc_ops;

	//_mutex_lock(&pwpsc->iocontrol_mutex);
	_spinlock(&pwpsc->iocontrol_mutex);
	ret = pwpsc_ops->iocontrol(pwpsc->skfd, IOCONTROL_QUERY, SIOCIWFIRSTPRIV + 0x3,
							(unsigned char *)(pmpioctlparam), msz,
							(unsigned char *)(pmpioctlparam), msz);
	//_mutex_unlock(&pwpsc->iocontrol_mutex);
	_spinunlock(&pwpsc->iocontrol_mutex);

	*count = *(unsigned int*)pmpioctlparam->data;

	free((void *)pmpioctlparam);

	return ret;
#else
	unsigned int OFDM_cnt = 0, CCK_cnt = 0, HT_cnt = 0;

	GetRxPhyPacketCounts(pwpsc, OFDM_MPDU_FAIL_BIT, &OFDM_cnt);
	GetRxPhyPacketCounts(pwpsc, CCK_MPDU_FAIL_BIT, &CCK_cnt);
	GetRxPhyPacketCounts(pwpsc, HT_MPDU_FAIL_BIT, &HT_cnt);

	*count = OFDM_cnt + CCK_cnt + HT_cnt;
	return TRUE;
#endif
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_reset_phy_rx_packet_count_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);
	_irqL		oldirql;

_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}

	_irqlevel_changed_(&oldirql, LOWER);
	ResetPhyRxPktCount(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}

//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_phy_rx_packet_received_hdl(struct oid_par_priv *poid_par_priv)
{
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

	_irqL		oldirql;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;

_func_enter_;

	RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_info_, ("+oid_rt_get_phy_rx_packet_received_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len != sizeof(ULONG))
		return NDIS_STATUS_INVALID_LENGTH;

	_irqlevel_changed_(&oldirql, LOWER);
	*(ULONG*)poid_par_priv->information_buf = GetPhyRxPktReceived(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;

	RT_TRACE(_module_rtl871x_mp_ioctl_c_, _drv_notice_, ("-oid_rt_get_phy_rx_packet_received_hdl: recv_ok=%d\n", *(ULONG*)poid_par_priv->information_buf));

_func_exit_;

	return status;
}

//----------------------------------------------------------------------------------
void* phy_pkt_recv_thread(void* arg)
{
//	unsigned int current_pkt_ok = 0, current_pkt_err = 0;
	struct wpsctrl *pwpsc = (struct wpsctrl *)arg;

	while (pwpsc->bstartrx == TRUE)
	{
		//printf("$-recv record packet ok cnts:%d\n",pwpsc->record_rx_pkt_ok_cnts);
		//printf("$-recv record packet err cnts:%d\n",pwpsc->record_rx_pkt_err_cnts);

		ppause(2);	//eason 20100804	sleep(2);
#if 1
		r871x_get_phy_rx_pkts_ok(pwpsc, &pwpsc->rx_pkt_ok_cnts);
		r871x_get_phy_rx_pkts_error(pwpsc, &pwpsc->rx_pkt_err_cnts);
#else
		r871x_get_phy_rx_pkts_ok(pwpsc, &current_pkt_ok);
		r871x_get_phy_rx_pkts_error(pwpsc, &current_pkt_err);

		//printf("$-recv current packet err cnts:%d\n",current_pkt_err);

		pwpsc->rx_pkt_ok_cnts = Calculatediff(pwpsc->record_rx_pkt_ok_cnts, current_pkt_ok);
		pwpsc->rx_pkt_err_cnts = Calculatediff(pwpsc->record_rx_pkt_err_cnts, current_pkt_err);
#endif
//		printf("$-recv packet ok cnts:%d\n", pwpsc->rx_pkt_ok_cnts);
//		printf("$-recv packet error cnts:%d\n\n", pwpsc->rx_pkt_err_cnts);
//		printf("\r$-recv packet cnts ok:%d error:%d", pwpsc->rx_pkt_ok_cnts, pwpsc->rx_pkt_err_cnts);
		//fflush(stdout);
	}
//	printf("\n");

	return NULL;
}

void set_pkt_tx_hdl(struct wpsctrl *pwpsc, unsigned int param)
{
	unsigned int bstarttx = 0;
	unsigned int pkt_cnts = 0 ;
	unsigned int payload_len = 0;//max:2018 2048-14-16
	unsigned int payload_type = 0;//0 1 2 3

	char *param5 = NULL;
	int i;
#if 0	//eason 20100804
	// paramer 1 : bstarttx
	if (!is_dec_str(param[0])) goto _ERROR_EXIT_FUNC_;
	sscanf(param[0], "%d", &bstarttx);
	if((bstarttx<0) || (bstarttx>1)) goto _ERROR_EXIT_FUNC_;
	pwpsc->bstarttx = bstarttx;
	if ((bstarttx==0) || (param[1]==NULL))
		goto _EXECUTE_FUNC_;

	// paramer 2 : pkt cnts
	if (!is_dec_str(param[1])) goto _ERROR_EXIT_FUNC_;
	sscanf(param[1], "%d", &pkt_cnts);
	pwpsc->tx_pkt_cnts = pkt_cnts;
	if (param[2] == NULL)
		goto _EXECUTE_FUNC_;

	// paramer 3 : payload len
	if (!is_dec_str(param[2])) goto _ERROR_EXIT_FUNC_;
	sscanf(param[2], "%d", &payload_len);
	if ((payload_len<0) || (payload_len>2018)) goto _ERROR_EXIT_FUNC_;
	pwpsc->payload_len = payload_len;
	if (param[3] == NULL)
		goto _EXECUTE_FUNC_;

	// paramer 4 : payload type
	if (!is_dec_str(param[3])) goto _ERROR_EXIT_FUNC_;
	sscanf(param[3], "%d", &payload_type);
	if((payload_type<0) || (payload_type>4)) goto _ERROR_EXIT_FUNC_;
	pwpsc->payload_type = payload_type;
	if (param[4] == NULL)
		goto _EXECUTE_FUNC_;

	// paramer 5 : destination address
	if ((param[4][1]|0x20) == 'x')
		param5 = param[4] + 2;
	else
		param5 = param[4];

	if (strlen(param5) < (2 * ETH_ALEN)) goto _ERROR_EXIT_FUNC_;

	for (i = 0; i < ETH_ALEN; i++) {
		unsigned char CurByte;
		CurByte = Asc2Hex(param5[2 * i]);
		CurByte <<= 4;
		CurByte += Asc2Hex(param5[2 * i + 1]);
		pwpsc->da[i] = CurByte;
	}
#endif	//eason 20100804
_EXECUTE_FUNC_:
	pwpsc->bstarttx = bstarttx = param;	//eason 20100804
	pwpsc->tx_pkt_cnts = pkt_cnts;		//eason 20100804
	if (pwpsc->bstarttx == 1) {
/*		printf("$-set_pkt_tx_hdl - bstarttx:%x pkt_cnts:%d len:%d type:0x%02x da:0x%s \n",\
			pwpsc->bstarttx, pwpsc->tx_pkt_cnts, pwpsc->payload_len,\
			payloaddata[pwpsc->payload_type], pwpsc->da);*/
	}

	pwpsc->mp_test_mode = PACKET_TX;

	r871x_set_pkt_xmit(pwpsc, bstarttx);

	return;

_ERROR_EXIT_FUNC_:
//	printf("%s  %s\n", paramerrstr, pkttxstr);
	return;

}