/* src/wireless/wlanif.c 
*
* wireless lan get network information from different MAC
*
* Create by Ron 6/20/2003
* --------------------------------------------------------------------
*/

//#include "global.h"	//eason
#include <drv_conf.h>		//eason 20100210
#include <osdep_service.h>	//eason 20100210
#include <drv_types.h> 		//eason 20100210
#include "psglobal.h"
#include "pstarget.h"
//#include "hardware.h"	//eason
#include "wlanif.h"
//#include "8021X\eapnet.h"



#define CODE2
extern unsigned char WirelessLightToggle;
#define PrnNoUsed       0x00

#if WLANMAC == RT2561
//#include "os_dep.h"	//eason
#endif

//#ifdef WIRELESS_CARD	//eason

int diag_flag;

BYTE    WirelessInitFailed = 0;
UINT    mvBSSType;			//1:Infrastructure 2:Ad-Hoc
UINT8   mvFirmwareVersion[16];
UINT8	ZRegDomain[12]; 	//Eldan added for test	
UINT8   mvESSID[33]={0};
UINT    mvChannel_start; 
UINT    mvChannel_end; 
UINT    mvChannel; 
UINT    mvWEPType;          // 0 disable, 1 64bit, 2 128 bit
UINT	mvWPAType;			// 0 TKIP, 1 CCMP
UINT    mvWEPKeySel;        // 0,1,2,3
UINT8   mvWirelessMAC[6];   // wireless MAC ID

UINT8   mvEDDMAC[6];
UINT8   mvWEPManual;        //0 Auto, 1 Manual
UINT16  mvWZone;        	//0xA1 USA / CANADA ( Northern American )
							//0xA2 ETSI ( Most of Europe and Australia )
							//0xA3 France
							//0xA4 Japan	1-14
							//0xA5 Spain
							//0xA6 Japan	14
							
UINT8   mvWDomain;          //0x0010 USA    1-11
                            //0x0020 CANADA 1-11
                            //0x0030 ETSI   1-13
                            //0x0031 SPAIN  10-11
                            //0x0032 France 10-13
                            //0x0040 Japan  14
                            //0x0041 Japan  1-14

UINT8   mvWEPKey[32];   	// cannot be NULL
UINT8   mvWEPKey1[6];
UINT8   mvWEPKey2[6];
UINT8   mvWEPKey3[6];
UINT8   mvWEPKey4[6];

UINT8   mvWEP128Key[15];
UINT8   mvWEP128Key2[15];
UINT8   mvWEP128Key3[15];
UINT8   mvWEP128Key4[15];
UINT8	mvWPAPass[64];

UINT    mvBeaconinterval;   //default 100 msec
UINT    mvRTSThreshold;     //default 2432, 0 ~ 3000
UINT    mvFragmentation;    //256 ... 2346, Only EVEN number, default 2346
UINT    mvRates;            // 1 ... 15, 3(1,2) 15(1,2,5.5, 11) 
UINT    mvRate;             //

UINT    mvShortPreamble;    	// 1:short, 0: long, default 0
UINT    mvAuthenticationType;	// 1:open(default), 2: shared, 3:both 4:WPA-PSK 5:WPA2-PSK
UINT    mvDtiminterval;     // 1 ~ 65535, default 1
UINT    mvCfpperiod;        // 1 ~ 65535, default 3
UINT    mvCfpmaxduration;   // 1 ~ 65535, default 100

UINT16	mvTxPower;
UINT    mvCRX=0; //0 or 1 ... continuous RX
UINT    mvCTX=0; //0 or 1 ... continuous RX
UINT8   mvJapan;   //0=usa 1=japan default=0  
UINT8   mvAn_side; //an_side  default=0 
                   //an_side  SPREAD ON  STATE 1=life 2=right 3:diversity
                   //an_side  SPREAD OFF STATE 1=life 2=right 3:diversity 
UINT    mvNonModulate =0; //0 or 1

UINT	mvConfigMode;	//isl3890, 2:INL_MODE_AP, 1:INL_MODE_CLIENT(default)
UINT	mvExtRate;		//isl3890, for 6~54Mb
UINT 	mvWPAauth;		// 0:Open/WEP 1:WPA-PSK 2:WPA2-PSK
UINT8	mvTxMode;		// 0x00: B-G mixed  0x01: B only  0x02: G only 0x03:B-G-N mixed 
UINT	mvBandWidth;	// 0:20MHz 1:40MHz
UINT    mvDataRate;		//0~3:CCK 4~11:OFDM 12~27:MCS0~15

/* To determine whether the setting string (SSID) is "< ANY >" ... Ron 6/24/2003 */
int wlan_set_anyssid(char *deststr){
#if 1	
	char anyssid[][WLAN_SSID_MAXLEN]= {"< ANY >"};
#else
	// next version change
	char anyssid[][WLAN_SSID_MAXLEN]= { "ANY", "<ANY>", "< ANY>", "<ANY >", "< ANY >"};
#endif	
	char tempstr[WLAN_SSID_MAXLEN];
	int i, j = (sizeof(anyssid)/WLAN_SSID_MAXLEN);
	
	memcpy(tempstr, deststr, WLAN_SSID_MAXLEN);
	strupr(tempstr);
	for(i=0; i<j; i++){
		if (strcmp(tempstr, anyssid[i]) ==0)
			return 1; //find one of anyssid[][], possible "< ANY >" SSID setting 
	}
	return 0;			
}

#if	(WLANMAC == RT2561)
extern VOID RTMPStartEMITx (PRTMP_ADAPTER pAdapter, UCHAR txPower, UCHAR channel, UCHAR wireless_mode, UCHAR tx_rate);
extern VOID RTMPStartEMIRx (PRTMP_ADAPTER pAdapter, UCHAR tx_rate);
extern RTMP_ADAPTER _FAR_* 	pAd;
#endif

/* wlan interface functions */
void WLan_get_EEPData(void)
{
		memcpy( mvEDDMAC, MyPhysNodeAddress, 6 );
#ifdef CODE1
	if( PSUpgradeMode == NOT_UPGRADE_MODE ){
//#if 0		
//		mvBSSType = P80211ENUM_bsstype_independent;
//		mvChannel = 4;	
//		mvRTSThreshold = 2432;
//		mvFragmentation = 2436;
//		mvRates = 15;
//		mvRate = 15;		
//#endif		
		mvBSSType = P80211ENUM_bsstype_80211ibss;
		strcpy( mvESSID, "WLAN-PS" );
		mvChannel = 6;	
		mvBeaconinterval = 100;		
		mvRTSThreshold = 2432;
		mvFragmentation = 2436;
		mvRates = 15;
		mvRate = 15;		
		mvShortPreamble = 0; // long preamble
		mvAuthenticationType = 3; //both
		mvCfpperiod = 3;
		mvCfpmaxduration = 100;		
	}
	else{
		mvBSSType = P80211ENUM_bsstype_80211ibss;
		strcpy( mvESSID, "WLAN-PS" );
		mvChannel = 6;
		mvBeaconinterval = EEPROM_Data.WLBeaconinterval;
		mvRTSThreshold = EEPROM_Data.WLRTSThreshold;
		mvFragmentation = EEPROM_Data.WLFragmentation;
		mvRates = EEPROM_Data.WLRates;
		mvRate = EEPROM_Data.WLRate;	

		mvShortPreamble = EEPROM_Data.WLShortPreamble;
		mvAuthenticationType = EEPROM_Data.WLAuthType;
		mvDtiminterval = EEPROM_Data.WLDtiminterval;
		mvCfpperiod = EEPROM_Data.WLCfpperiod;
		mvCfpmaxduration = EEPROM_Data.WLCfpmaxduration;
	}
#else // !ifdef CODE1

#ifdef PRINT_DIAGNOSTIC
	if(print_diagnostic())
		diag_flag = 1;
	else 	
		diag_flag = 0;
		
#else
		diag_flag = 0;
#endif
	if(diag_flag){
		mvBSSType = P80211ENUM_bsstype_80211ibss;
		strcpy( mvESSID, "WLAN-PS" );
		mvChannel = 6;
		mvBeaconinterval = 100;
		mvRTSThreshold = 2432;
		mvFragmentation = 2436;
		mvRates = 15;
		mvRate = 15;	
		mvShortPreamble = 0;

		mvTxPower = 0;
		mvCRX = 0;
		mvCTX = 0;
		mvJapan = 0;
		mvAn_side = 0;
		mvNonModulate = 0;		
	}
	else{
#if	(WLANMAC == RT2561) //Kevin:  (WLANMAC == ISL80211G)
#if !(defined CODE1) && (defined INCLUDE_AP)
	if(EEPROM_Data.WLAPMode != 0){
		mvConfigMode = 2;//INL_MODE_AP
		mvBSSType = 1;//DOT11_BSSTYPE_INFRA
	}	
	else
#endif	/*defined INCLUDE_AP*/
	{
		mvConfigMode =	1;//INL_MODE_CLIENT
		
		if(EEPROM_Data.WLMode == 2)	// 0:Infrastructure 1:Ad-Hoc 2:802.11b Ad-Hoc
			mvBSSType = 2;//DOT11_BSSTYPE_IBSS
		else
			mvBSSType = 1;//DOT11_BSSTYPE_INFRA
	}
#elif	(WLANMAC == RT2571)
#if !(defined CODE1) && (defined INCLUDE_AP)
	if(EEPROM_Data.WLAPMode != 0){
		mvConfigMode = 2;//INL_MODE_AP
		mvBSSType = 1;//DOT11_BSSTYPE_INFRA
	}	
	else
#endif	/*defined INCLUDE_AP*/
	{
		mvConfigMode =	1;//INL_MODE_CLIENT
		
		if(EEPROM_Data.WLMode == 2)	// 0:Infrastructure 1:Ad-Hoc 2:802.11b Ad-Hoc
			mvBSSType = 2;//DOT11_BSSTYPE_IBSS
		else
			mvBSSType = 1;//DOT11_BSSTYPE_INFRA
	}	
			
#elif 	(WLANMAC == RT8188) //eason 20100210

	if(EEPROM_Data.WLMode == 2)	// 0:AUTO 1:Infrastructure 2:Ad-Hoc
			mvBSSType = 2;//DOT11_BSSTYPE_IBSS
		else
			mvBSSType = 1;//DOT11_BSSTYPE_INFRA
			
#elif	(WLANMAC == WLZD1211)
	
	switch( EEPROM_Data.WLMode )
	{
		case 0:						// Infrastructure
			mvBSSType = 0x1;//INFRASTRUCTURE_BSS		0x1
			break;
		case 1:						// Pseudo AdHoc
			mvBSSType = 0x3;//PSEUDO_IBSS				0x3
			break;
		case 2:						// 802.11b AdHoc
			mvBSSType = 0x0;//INDEPENDENT_BSS			0x0
			break;
	}
#else	
		switch( EEPROM_Data.WLMode )
		{
		case 0:						// Infrastructure
			mvBSSType = P80211ENUM_bsstype_infrastructure;
			break;
		case 1:						// Pseudo AdHoc
			mvBSSType = P80211ENUM_bsstype_independent;
			break;
		case 2:						// 802.11b AdHoc
			mvBSSType = P80211ENUM_bsstype_80211ibss;
			break;
		}
#endif	/*WLANMAC == RT2561*/		
		
//		strcpy( mvESSID, EEPROM_Data.WLESSID );
		memcpy( mvESSID, EEPROM_Data.WLESSID, 32 );
		mvChannel = EEPROM_Data.WLChannel;
		mvWEPKeySel = EEPROM_Data.WLWEPKeySel;
		memcpy( mvWEPKey1, EEPROM_Data.WLWEPKey1, sizeof(EEPROM_Data.WLWEPKey1) );
		memcpy( mvWEPKey2, EEPROM_Data.WLWEPKey2, sizeof(EEPROM_Data.WLWEPKey2) );
		memcpy( mvWEPKey3, EEPROM_Data.WLWEPKey3, sizeof(EEPROM_Data.WLWEPKey3) );
		memcpy( mvWEPKey4, EEPROM_Data.WLWEPKey4, sizeof(EEPROM_Data.WLWEPKey4) );
		memcpy( mvWEP128Key, EEPROM_Data.WLWEP128Key, sizeof(EEPROM_Data.WLWEP128Key) );
#ifdef WLWEP128_FOURKEYS
		memcpy( mvWEP128Key, EEPROM_Data.WLWEP128Key, sizeof(EEPROM_Data.WLWEP128Key) );
		memcpy( mvWEP128Key2, EEPROM_Data.WLWEP128Key2, sizeof(EEPROM_Data.WLWEP128Key2) );
		memcpy( mvWEP128Key3, EEPROM_Data.WLWEP128Key3, sizeof(EEPROM_Data.WLWEP128Key3) );
		memcpy( mvWEP128Key4, EEPROM_Data.WLWEP128Key4, sizeof(EEPROM_Data.WLWEP128Key4) );
#else
		memcpy( mvWEP128Key, EEPROM_Data.WLWEP128Key, sizeof(EEPROM_Data.WLWEP128Key) );
#endif //WLWEP128_FOURKEYS 			
		memcpy( mvWPAPass, EEPROM_Data.WPA_Pass, sizeof(EEPROM_Data.WPA_Pass));
		
		mvBeaconinterval = EEPROM_Data.WLBeaconinterval;
		mvRTSThreshold = EEPROM_Data.WLRTSThreshold;
		mvFragmentation = EEPROM_Data.WLFragmentation;
		mvRates = EEPROM_Data.WLRates;
		mvRate = EEPROM_Data.WLRate;
		mvAuthenticationType = EEPROM_Data.WLAuthType;
		
		mvShortPreamble = EEPROM_Data.WLShortPreamble;

		mvWEPType 	= EEPROM_Data.WLWEPType;
		mvWPAType 	= EEPROM_Data.WLWPAType;
		mvTxMode 	= EEPROM_Data.WLTxMode;

		mvDtiminterval = EEPROM_Data.WLDtiminterval;
		mvCfpperiod = EEPROM_Data.WLCfpperiod;
		mvCfpmaxduration = EEPROM_Data.WLCfpmaxduration;

		mvTxPower = EEPROM_Data.WLTxPower;
		mvBandWidth = EEPROM_Data.WLBandWidth;
		mvDataRate = EEPROM_Data.WLDataRate;

		mvCRX = EEPROM_Data.WLCRX;
		mvCTX = EEPROM_Data.WLCTX;
		mvJapan = EEPROM_Data.WLJapan;
		mvAn_side = EEPROM_Data.WLAnSide;
		mvNonModulate = EEPROM_Data.WLNonModulate;

#if	  (WLANMAC == RT2571) 
//esaon		mvWZone = EEPROM_Data.WLZone;
//eason		mvExtRate = EEPROM_Data.WLExtRate;
		mvWZone = 0;
		mvExtRate = 0;

		switch( mvWZone )
		{
		case 0xA1:  //USA    1-11
			mvWDomain = 0x0010;
			mvChannel_start =1;
			mvChannel_end =11;
			break;
		case 0xA2:  // ETSI   1-13
			mvWDomain = 0x0030;
			mvChannel_start =1;
			mvChannel_end =13;		
			break;
		case 0xA3:  // France 10-13
			mvWDomain = 0x0032;
			mvChannel_start =10;
			mvChannel_end =13;		
			break;
		case 0xA4:  // Japan  1-14
			mvWDomain = 0x0041;
			mvChannel_start =1;
			mvChannel_end =14;		
			break;
		case 0xA5:  // SPAIN  10-11
			mvWDomain = 0x0031;
			mvChannel_start =10;
			mvChannel_end =11;		
			break;
		case 0xA6:  // Japan  14
			mvWDomain = 0x0040;
			mvChannel_start =14;
			mvChannel_end =14;		
			break;
		default:
			mvWDomain = 0x0010; //USA    1-11
			mvChannel_start =1;
			mvChannel_end =11;		
			break;
		}
		
		if( mvChannel < mvChannel_start )
			mvChannel = mvChannel_start;
		if( mvChannel > mvChannel_end )
			mvChannel = mvChannel_end;

#endif	// #if	  (WLANMAC == RT2571)	

#if	  (WLANMAC == RT8188) 
		mvWZone = EEPROM_Data.WLZone;
		mvExtRate = EEPROM_Data.WLExtRate;
		
		switch( mvWZone )
		{
		case 0xA1:  //USA    1-11
			mvWDomain = 0x0010;
			mvChannel_start =1;
			mvChannel_end =11;
			break;
		case 0xA2:  // ETSI   1-13
			mvWDomain = 0x0030;
			mvChannel_start =1;
			mvChannel_end =13;		
			break;
		case 0xA3:  // France 10-13
			mvWDomain = 0x0032;
			mvChannel_start =10;
			mvChannel_end =13;		
			break;
		case 0xA4:  // Japan  1-14
			mvWDomain = 0x0041;
			mvChannel_start =1;
			mvChannel_end =14;		
			break;
		case 0xA5:  // SPAIN  10-11
			mvWDomain = 0x0031;
			mvChannel_start =10;
			mvChannel_end =11;		
			break;
		case 0xA6:  // Japan  14
			mvWDomain = 0x0040;
			mvChannel_start =14;
			mvChannel_end =14;		
			break;
		default:
			mvWDomain = 0x0010; //USA    1-11
			mvChannel_start =1;
			mvChannel_end =11;		
			break;
		}
		
		if( mvChannel < mvChannel_start )
			mvChannel = mvChannel_start;
		if( mvChannel > mvChannel_end )
			mvChannel = mvChannel_end;

#endif	// #if	  (WLANMAC == RT8188)

#if	  (WLANMAC == WLZD1211)
		mvWZone = EEPROM_Data.WLZone;
		mvExtRate = EEPROM_Data.WLExtRate;

		switch( mvWZone )
		{
		case 0xA1:  //USA    1-11
			mvWDomain = 0x0010;
			mvChannel_start =1;
			mvChannel_end =11;
			break;
		case 0xA2:  // ETSI   1-13
			mvWDomain = 0x0030;
			mvChannel_start =1;
			mvChannel_end =13;		
			break;
		case 0xA3:  // France 10-13
			mvWDomain = 0x0032;
			mvChannel_start =10;
			mvChannel_end =13;		
			break;
		case 0xA4:  // Japan  1-14
			mvWDomain = 0x0041;
			mvChannel_start =1;
			mvChannel_end =14;		
			break;
		case 0xA5:  // SPAIN  10-11
			mvWDomain = 0x0031;
			mvChannel_start =10;
			mvChannel_end =11;		
			break;
		case 0xA6:  // Japan  14
			mvWDomain = 0x0040;
			mvChannel_start =14;
			mvChannel_end =14;		
			break;
		default:
			mvWDomain = 0x0010; //USA    1-11
			mvChannel_start =1;
			mvChannel_end =11;		
			break;
		}
		
		if( mvChannel < mvChannel_start )
			mvChannel = mvChannel_start;
		if( mvChannel > mvChannel_end )
			mvChannel = mvChannel_end;
#endif //(WLANMAC == WLZD1211)

#if	  (WLANMAC == RT2561) //Kevin:	
		mvWZone = EEPROM_Data.WLZone;
		mvExtRate = EEPROM_Data.WLExtRate;

		switch( mvWZone )
		{
		case 0xA1:  //USA    1-11
			mvWDomain = 0x0010;
			mvChannel_start =1;
			mvChannel_end =11;
			break;
		case 0xA2:  // ETSI   1-13
			mvWDomain = 0x0030;
			mvChannel_start =1;
			mvChannel_end =13;		
			break;
		case 0xA3:  // France 10-13
			mvWDomain = 0x0032;
			mvChannel_start =10;
			mvChannel_end =13;		
			break;
		case 0xA4:  // Japan  1-14
			mvWDomain = 0x0041;
			mvChannel_start =1;
			mvChannel_end =14;		
			break;
		case 0xA5:  // SPAIN  10-11
			mvWDomain = 0x0031;
			mvChannel_start =10;
			mvChannel_end =11;		
			break;
		case 0xA6:  // Japan  14
			mvWDomain = 0x0040;
			mvChannel_start =14;
			mvChannel_end =14;		
			break;
		default:
			mvWDomain = 0x0010; //USA    1-11
			mvChannel_start =1;
			mvChannel_end =11;		
			break;
		}
		
		if( mvChannel < mvChannel_start )
			mvChannel = mvChannel_start;
		if( mvChannel > mvChannel_end )
			mvChannel = mvChannel_end;

#endif	// #if	  (WLANMAC == RT2561)	
		
		if(mvCTX == 1 || mvCRX ==1 )
			mvBSSType = P80211ENUM_bsstype_independent;
	}
#endif	// !ifdef CODE1
}

#if (WLANMAC == WLZD1211)
#if defined(CODE2)
extern void wpa_task(int unused,void *unused1,void *unused2);
extern void wsc_task(int unused,void *unused1,void *unused2);
#endif	//CODE2
#endif

void Wlan_MacInit(void){

	int status;

	WLan_get_EEPData();  // get initial setting ... 7/2/2003
	
#if (WLANMAC == RT8188)			// 716U2W
	WirelessInitFailed = 1;
#endif

#if (WLANMAC == RT2571)			// DWP2010
	WirelessInitFailed = 1;
#endif	

#if (WLANMAC == WLZD1211)		// DWP1000, DWP2000
	WirelessInitFailed = 1;

#if defined(CODE2)
	newproc("WSC", 8192, wsc_task, 0, NULL, NULL, 0);
	
	newproc("WPA", 8192, wpa_task, 0, NULL, NULL, 0);
#endif //CODE2
		
#endif

#if (WLANMAC == RT2561)
#if defined(CODE2)	
	if(rt2561_init())
		Light_Flash(20,0);
#endif
#endif	
	
#if (WLANMAC == PRISM2)
#if defined(CODE2)	
	GSPktDriverInit();
#endif
	
#elif (WLANMAC == PRISM3)	
	GSPktDriverInit();
#elif (WLANMAC == ISL80211G)	
#if defined(CODE2)	
	ISL38XXDriverInit();
#endif	
#elif (WLANMAC == AX81190)	
#if defined(CODE1)	
	wnet_init_code1();
#else	
	wnet_init();
#endif

#else
	return;
#endif	
}

void Wlan_reset(void){

#if (WLANMAC == RT2561)
#if defined(CODE2)	
	RT61_reset();
#endif	

#elif	(WLANMAC == WLZD1211)
	ZYDAS_1211_reset();

#else
	return;
#endif	
	
}

#if (WLANMAC == WLZD1211)
extern struct net_device *g_dev;

//Jesse
int send_sem = 0;
//int send_pkt_cnt = 0;

#endif
//eason 20100210 extern PRTMP_ADAPTER       Global_pAd;
int Wlan_SendPacket(struct sk_buff* pSkb){
//not yet	struct mbuf* skbuf;

	if( WirelessInitFailed )
		return -1;

//Jesse		
//	if( send_pkt_cnt > 1000	)
//		return ;
//	send_pkt_cnt++;
#if (WLANMAC == RT8188)
	xmit_entry(pSkb);
	WirelessLightToggle++;	
#endif

#if (WLANMAC == RT2571)
#if defined(CODE2)	
#if 1
//not yet	skbuf = alloc_int_mbuf (1514);
//not yet	skbuf->cnt = len;
//not yet	memcpy(skbuf->data, buf, len);
//not yet	RTSendPackets(skbuf);
RTMPSendPackets(Global_pAd, pSkb);
WirelessLightToggle++;	
#endif	
#endif 
#endif	

#if (WLANMAC == WLZD1211)

	while(send_sem == 1)
	{
		kwait(NULL);
	}
	send_sem = 1;
	
	if(len > 1536)
	{
//Jesse
#ifdef CODE2
Printf("Send pkt len over 1536\n");
#endif //CODE2
		send_sem = 0;
		return -1;
	}
//Jesse 	skbuf = alloc_int_mbuf (1514);
	skbuf = alloc_mbuf (1536);
	if( skbuf == NULL )
	{
//Jesse
#ifdef CODE2
Printf("Send = NULL\n");
#endif //CODE2
		send_sem = 0;
		return -1;
	}
	skbuf->cnt = len;
	memcpy(skbuf->data, buf, len);
	zd1205_xmit_frame(skbuf, g_dev);
	send_sem = 0;
	WirelessLightToggle++;	
#endif

#if (WLANMAC == RT2561)
#if defined(CODE2)	
#if 1
	//Kevin:
//ZOT 2007	

	skbuf = __dev_alloc_skb(1600, 0xFF);

	skbuf->cnt = len;
	memcpy(skbuf->data, buf, len);
	RTMPSendPackets (pAd, skbuf);
	WirelessLightToggle++;	
#endif	
#endif 
#endif	

#if (WLANMAC == PRISM2)
#if defined(CODE2)	
	GSPktSendPacket(buf, len);
#endif	
#elif (WLANMAC == PRISM3)
	GSPktSendPacket(buf, len);
#elif (WLANMAC == ISL80211G)	
#if defined(CODE2)	
	islpci_eth_transmit(buf, len);
#endif	
#elif (WLANMAC == AX81190)
#if defined(CODE1)	
	wlanc1_send(buf, len);
#else	
	wnet_send(buf, len);
#endif	
#else
	return -1;
#endif	
	
}

int Wlan_Diagonse(void){

	if( WirelessInitFailed )
		return 1;

#if (WLANMAC == PRISM2)
	return prism2diagnose();
	
#elif (WLANMAC == PRISM3)
	return prism2diagnose();

#elif (WLANMAC == ISL80211G)
#ifdef CODE2
	return isl80211g_get_diagnose();	
#endif	
#elif (WLANMAC == AX81190)	
	
	
#else
	return 1;
#endif	
}


void wlan_site_survey(void){
	int Port = 0;
	WORD PrinterStatus;
	
	if( WirelessInitFailed )
		return;

#if defined(CODE2)
	for(Port = 0 ; Port < NUM_OF_PRN_PORT ; Port++) {
		PrinterStatus = PrnGetPrinterStatus(Port);
		if (PrinterStatus != PrnNoUsed)
			return;
	}
#endif //CODE2

#if (WLANMAC == RT2561)
	RTMP_site_survey();
#elif (WLANMAC == RT2571)
	RTMP_site_survey();
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)
	ZYDAS_1211_site_survey();
#endif //CODE2
#elif	(WLANMAC == RT8188)	
	REALTEK_site_survey();
#else
	return;
#endif
	
}

knownbss_t *wlan_get_scanlist(void){
		
	if( WirelessInitFailed )
		return NULL;
	
#if (WLANMAC == RT2561)
	return RTMP_get_scanlist();
#elif (WLANMAC == RT2571)
	return RTMP_get_scanlist();
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)
	ZYDAS_1211_get_scanlist();
#endif //CODE2
	return APList;	
#elif	(WLANMAC == RT8188)
	return	REALTEK_get_scanlist();
#else
	return NULL;
#endif
}

void wlan_get_currbssid(unsigned char huge * bssid){
	
	if( WirelessInitFailed ){
		memset(bssid, 0 ,WLAN_BSSID_LEN);
		return;
	}	
#if	  (WLANMAC == RT2561)
#ifdef CODE2
	RTMP_get_currbssid(bssid);
#endif
#elif	  (WLANMAC == RT2571)
#ifdef CODE2
	RTMP_get_currbssid(bssid);
#endif
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)
	ZYDAS_1211_get_currbssid(bssid);
#endif //CODE2
#elif	(WLANMAC == RT8188)
	REALTEK_get_currbssid(bssid);
#else
	memset(bssid, 0 ,WLAN_BSSID_LEN);
#endif
}

void wlan_get_currssid(unsigned char huge * ssid){
	
	if( WirelessInitFailed ){
		memset(ssid, 0 ,WLAN_SSID_MAXLEN);
		return;
	}

#if	  (WLANMAC == RT2561)
#ifdef CODE2
	RTMP_get_currssid(ssid);
#endif
#elif	  (WLANMAC == RT2571)
#ifdef CODE2
	RTMP_get_currssid(ssid);
#endif
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)	
	ZYDAS_1211_get_currssid(ssid);
#endif //CODE2
#elif	(WLANMAC == RT8188) 
	REALTEK_get_currssid(ssid);
#else
	memset(ssid, 0 ,WLAN_SSID_MAXLEN);
#endif

}

int wlan_get_channel(void){
	
	if( WirelessInitFailed )
		return -1;

#if	  (WLANMAC == RT2561)
#ifdef CODE2
	return RTMP_get_channel();
#endif	
#elif	  (WLANMAC == RT2571)
#ifdef CODE2
	return RTMP_get_channel();
#endif
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)
	return ZYDAS_1211_get_channel();
#endif //CODE2
#elif	(WLANMAC == RT8188)
	return ERALTEK_get_channel();
#else
	return -1;
#endif
}

int wlan_get_currrate(void){
	
	if( WirelessInitFailed )
		return -1;

#if	  (WLANMAC == RT2561)
#ifdef CODE2
	return RTMP_get_currrate();
#endif
#elif	  (WLANMAC == RT2571)
#ifdef CODE2
	return RTMP_get_currrate();
#endif	
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)	
	return ZYDAS_1211_get_currrate();
#endif //CODE2
#elif	(WLANMAC == RT8188)
	return	REALTEK_get_currrate();
#else
	return -1;
#endif
}

int wlan_get_rssi(void) {
	
	if( WirelessInitFailed )
		return -1;

#if	  (WLANMAC == RT2561)
#ifdef CODE2
	return RTMP_get_rssi();
#endif
#elif	  (WLANMAC == RT2571)
#ifdef CODE2
	return RTMP_get_rssi();
#endif
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)
	return ZYDAS_1211_get_rssi();
#endif //CODE2
#elif	(WLANMAC == RT8188)
	return REALTEK_get_rssi();
#else
	return -1;
#endif	
}

int wlan_get_linkquality(void) {
	
	if( WirelessInitFailed )
		return -1;

#if	  (WLANMAC == RT2561)
#ifdef CODE2
	return RTMP_get_linkquality();
#endif
#elif	  (WLANMAC == RT2571)
#ifdef CODE2
	return RTMP_get_linkquality();
#endif
#elif	(WLANMAC == WLZD1211)
#if defined(CODE2)
	return ZYDAS_1211_get_linkquality();
#endif //CODE2	
#elif	(WLANMAC == RT8188)
	return REALTEK_get_linkquality();
#else
	return -1;
#endif	
}

extern int rtl871x_get_linkup();

//1:link up 0: link down
int wlan_get_linkup ()
{
	int state = 0;
	state = rtl871x_get_linkup();
	
	return state;
}


#ifdef AUTH_8021X
int wlan_set_encrypt(uint32 action){
#if	  (WLANMAC == ISL80211G)
	return islpci_set_encrypt(action);

#elif (WLANMAC == PRISM2)
	return prism2sta_set_encrypt(action);

#elif (WLANMAC == PRISM3)
	return prism2sta_set_encrypt(action);
	
#elif (WLANMAC == AX81190)	
	
#else
	return -1;
#endif		
}

int wlan_set_exunencrypt(uint32 action){
#if	  (WLANMAC == ISL80211G)
	return islpci_set_exunencrypt(action);

#elif (WLANMAC == PRISM2)
	return prism2sta_set_exunencrypt(action);

#elif (WLANMAC == PRISM3)
	return prism2sta_set_exunencrypt(action);
	
#elif (WLANMAC == AX81190)	
	
#else
	return -1;
#endif		
}

void wlan_eap_get_frame()
{
#if	  (WLANMAC == ISL80211G)
	ksignal((struct mbuf *)eap_get_frame, 1);
	
#endif	
}

#endif

#ifdef WPA_PSK_TKIP		//WPA2004Jun
int wlan_set_stakey(int id, char *address, char *wpa_key1, char *wpa_key2){
#if	  (WLANMAC == ISL80211G)
	return islpci_set_stakey(id, address, wpa_key1, wpa_key2);
#endif	
}

int wlan_set_stasc(int id, char *address, int tx_sc, unsigned long sc_high, unsigned short sc_low){
#if	  (WLANMAC == ISL80211G)
	return islpci_set_stasc(id, address, tx_sc, sc_high, sc_low);
#endif	
}

void wlan_request_key(int req)
{
#if	  (WLANMAC == ISL80211G)
#ifdef WPA_PSK_TKIP
	wpa_request_key(req);
#endif	
#endif
}
#endif

#if 0
void wlan_wsc_save_eep(void)
{

#if	(WLANMAC == RT2561)	
	
	EEPROM_Data.WLMode = 0;
	memset(EEPROM_Data.WLESSID,0,32);
	memcpy(EEPROM_Data.WLESSID,pAd->pPortCfg->Ssid, pAd->pPortCfg->SsidLen);
	
	if(pAd->pPortCfg->AuthMode == Ndis802_11AuthModeWPAPSK)
		EEPROM_Data.WLAuthType = 4;
	else if(pAd->pPortCfg->AuthMode = Ndis802_11AuthModeWPA2PSK)
		EEPROM_Data.WLAuthType = 5;
	
	
	if(pAd->pPortCfg->WepStatus == Ndis802_11Encryption2Enabled)
		EEPROM_Data.WLWPAType = 0;
	if(pAd->pPortCfg->WepStatus == Ndis802_11Encryption3Enabled)
		EEPROM_Data.WLWPAType = 1;
	
	memset(EEPROM_Data.WPA_Pass,0,64);
	memcpy(EEPROM_Data.WPA_Pass,pAd->pPortCfg->WscControl.WscKey, pAd->pPortCfg->WscControl.WscKeyLen);

#elif	(WLANMAC == WLZD1211)

	EEPROM_Data.WLMode = 0;
	memset(EEPROM_Data.WLESSID,0,32);
	memcpy(EEPROM_Data.WLESSID,mvESSID, strlen(mvESSID));	
            
    EEPROM_Data.WLAuthType = mvAuthenticationType;
    
   	EEPROM_Data.WLWPAType = mvWPAType;
	   
	memset(EEPROM_Data.WPA_Pass,0,64);
	memcpy(EEPROM_Data.WPA_Pass,mvWPAPass,64);
	
	WriteToEEPROM(&EEPROM_Data);

#endif

}
#endif
//#endif //define WIRELESS_CARD	//eason
