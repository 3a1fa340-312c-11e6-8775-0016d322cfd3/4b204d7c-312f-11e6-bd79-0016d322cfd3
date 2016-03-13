/* src/wireless/wlanif.c

* wireless lan get network information from different MAC
*
* Create by Ron 6/20/2003
* --------------------------------------------------------------------
*/

#include "module_config.h"
#include "psglobal.h"
#include "pstarget.h"
#include "mtk7601_dep.h"
#include "rt_config.h"
#include "wlanif.h"
#include "eeprom.h"
#include "wireless.h"
#include "star_gpio.h"
//#include "8021X\eapnet.h"

extern EEPROM EEPROM_Data;

#define CODE2
extern unsigned char WirelessLightToggle;
#define PrnNoUsed       0x00

int diag_flag;

BYTE    WirelessInitFailed = 1;
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

UINT8   mt7601_countryRegion;
UINT8   mt7601_countryRegionABand;

UINT8   mvWEPKey[32];   	// cannot be NULL
UINT8   mvWEPKey1[6];
UINT8   mvWEPKey2[6];
UINT8   mvWEPKey3[6];
UINT8   mvWEPKey4[6];

UINT8   mvWEP128Key[15];
UINT8   mvWEP128Key2[15];
UINT8   mvWEP128Key3[15];
UINT8   mvWEP128Key4[15];
UINT8	mvWPAPass[65];

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
UINT8	mvTxMode;		// 0x00: B-G mixed  0x01: B only  0x02: G only 0x03:B-G-N mixed 0x04: N only
UINT	mvBandWidth;	// 0:20MHz 1:40MHz
UINT    mvDataRate;		//0~3:CCK 4~11:OFDM 12~27:MCS0~15

extern PNET_DEV    g_wireless_dev; /* global wireless device */

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

//int wlan_route_onoff = 1;

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
#elif 	(WLANMAC == MTK7601) //eason 20110315

	if(EEPROM_Data.WLMode == 2)	// 0:AUTO 1:Infrastructure 2:Ad-Hoc
			mvBSSType = 1;//DOT11_BSSTYPE_IBSS
		else
			mvBSSType = 0;//DOT11_BSSTYPE_INFRA	

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

#if	  (WLANMAC == MTK7601) 
		mvWZone = EEPROM_Data.WLZone;
		mvExtRate = EEPROM_Data.WLExtRate;	// RT3070
		
		switch( mvWZone )
		{
		case 0xA1:  //USA    1-11
			mvWDomain = 0x0010;
			mvChannel_start =1;
			mvChannel_end =11;
            mt7601_countryRegion = 0;
            mt7601_countryRegionABand = 7;//0;
			break;
		case 0xA2:  // ETSI   1-13
			mvWDomain = 0x0030;
			mvChannel_start =1;
			mvChannel_end =13;		
            mt7601_countryRegion = 1;
            mt7601_countryRegionABand = 7;//1;
			break;
		case 0xA3:  // France 10-13
			mvWDomain = 0x0032;
			mvChannel_start =10;
			mvChannel_end =13;		
            mt7601_countryRegion = 3;
            mt7601_countryRegionABand = 7;//3;
			break;
		case 0xA4:  // Japan  1-14
			mvWDomain = 0x0041;
			mvChannel_start =1;
			mvChannel_end =14;		
            mt7601_countryRegion = 5;
            mt7601_countryRegionABand = 7;//5;
			break;
		case 0xA5:  // SPAIN  10-11
			mvWDomain = 0x0031;
			mvChannel_start =10;
			mvChannel_end =11;		
            mt7601_countryRegion = 2;
            mt7601_countryRegionABand = 7;//2;
			break;
		case 0xA6:  // Japan  14
			mvWDomain = 0x0040;
			mvChannel_start =14;
			mvChannel_end =14;		
            mt7601_countryRegion = 4;
            mt7601_countryRegionABand = 7;//4;
			break;
		default:
			mvWDomain = 0x0010; //USA    1-11
			mvChannel_start =1;
			mvChannel_end =11;		
            mt7601_countryRegion = 0;
            mt7601_countryRegionABand = 7;//0;
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

//#if (WLANMAC == MTK7601)
char* WLan_config () 
{
    char* config_buf = malloc (4096);
    char  str_buf[14];
    int   offset = 0, i;
    int   wirelessMode = 9;

    /* MAC address */
    if (config_buf) {
        sprintf(str_buf, "%2X:%2X:%2X:%2X:%2X:%2X", mvEDDMAC[0],
                                                    mvEDDMAC[1],
                                                    mvEDDMAC[2],
                                                    mvEDDMAC[3],
                                                    mvEDDMAC[4],
                                                    mvEDDMAC[5]);
        for (i = 0; i < 17; i++) {
            if (str_buf[i] == 0x20)
                str_buf[i] = 0x30;
        }
        offset = sprintf(config_buf, "Default\n");
        offset += sprintf(config_buf+offset, "MacAddress=%s\n", str_buf);
    }
    
    /* channel */
    offset += sprintf(config_buf+offset, "Channel=%d\n", mvChannel);
 
    /* wireless mode */
    //@> WirelessMode=value
	//value	
    //		0: legacy 11b/g mixed 
    //		1: legacy 11B only 
    //		2: legacy 11A only          //Not support in RfIcType=1(id=RFIC_5225) and RfIcType=2(id=RFIC_5325)
    //		3: legacy 11a/b/g mixed     //Not support in RfIcType=1(id=RFIC_5225) and RfIcType=2(id=RFIC_5325)
    //		4: legacy 11G only
    //		5: 11ABGN mixed
    //		6: 11N only
    //		7: 11GN mixed
    //		8: 11AN mixed
    //		9: 11BGN mixed
    //	   10: 11AGN mixed	
    //
    //	    mvTxMode;		            // 0x00: B-G mixed  0x01: B only  0x02: G only 0x03:B-G-N mixed 
    switch (mvTxMode) {
        case 0:
            wirelessMode = 0;
            break;
        case 1:
            wirelessMode = 1;
            break;
        case 2:
            wirelessMode = 4;
            break;
        case 4:
            wirelessMode = 6;
            break;
        case 3:
        default:
            wirelessMode = 9;
            break;
    }
    offset += sprintf(config_buf+offset, "WirelessMode=%d\n", wirelessMode);

    //offset += sprintf(config_buf+offset, "CountryCode=TW\n");
    /* country region */
    //offset += sprintf(config_buf+offset, "CountryRegion=%d\n", mvWDomain);
    offset += sprintf(config_buf+offset, "CountryRegion=%d\n", mt7601_countryRegion);

    /* country region aband */
    //@> CountryRegionABand=value      							
    //	value	
    //		0: use 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165 Channel
    //		1: use 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 Channel
    //		2: use 36, 40, 44, 48, 52, 56, 60, 64 Channel
    //		3: use 52, 56, 60, 64, 149, 153, 157, 161 Channel
    //		4: use 149, 153, 157, 161, 165 Channel
    //		5: use 149, 153, 157, 161 Channel
    //		6: use 36, 40, 44, 48 Channel
    //		7: use 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165 Channel
    //		8: use 52, 56, 60, 64 Channel
    //		9: use 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165 Channel
    //	   10: use 36, 40, 44, 48, 149, 153, 157, 161, 165 Channel
    //	   11: use 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 149, 153, 157, 161 Channel

    offset += sprintf(config_buf+offset, "CountryRegionABand=%d\n", mt7601_countryRegionABand);
    
    offset += sprintf(config_buf+offset, "ChannelGeography=1\n");
    /* SSID */
    #if 1
    offset += sprintf(config_buf+offset, "SSID=%s\n", mvESSID);
    #else
    offset += sprintf(config_buf+offset, "SSID=TStudio\n");
    //offset += sprintf(config_buf+offset, "SSID=TOTOLINK iPuppy III\n");
    //offset += sprintf(config_buf+offset, "SSID=ADDAP\n");
    #endif

    /* network type */
    //@> NetworkType=value	    		
    //	value 
    //		Infra: infrastructure mode
    //     	Adhoc: adhoc mode
    
    #if 1
    if (mvBSSType == 0)
        offset += sprintf(config_buf+offset, "NetworkType=Infra\n");
    else {
        offset += sprintf(config_buf+offset, "NetworkType=Adhoc\n");
        //wlan_route_onoff = 2;
    }
    #else
    offset += sprintf(config_buf+offset, "NetworkType=Infra\n");
    #endif

    /* beacon period */
    offset += sprintf(config_buf+offset, "BeaconPeriod=%d\n", mvBeaconinterval);

    /* tx power default 100*/
    #if 1
    #ifdef WIFI_CERTIFICATION
    if (mvTxPower > 70) mvTxPower = 70;
    #endif
    offset += sprintf(config_buf+offset, "TxPower=%d\n", mvTxPower);
    //offset += sprintf(config_buf+offset, "TxPower=100\n");
    #else
    offset += sprintf(config_buf+offset, "TxPower=100\n");
    #endif

    /* bg protection */
    //@> BGProtection=value
    //	value
    //		0: Auto 
    //		1: Always on 
    //		2: Always off
    offset += sprintf(config_buf+offset, "BGProtection=0\n");

    /* tx preamble */
    //@> TxPreamble=value
    //  	value
    //		0:Preamble Long
    //		1:Preamble Short 
    //		2:Auto
    offset += sprintf(config_buf+offset, "TxPreamble=0\n");

    /* rts threshold */
    //@> RTSThreshold=value
    //	value
    //		1~2347 
    offset += sprintf(config_buf+offset, "RTSThreshold=2347\n");

    /* frag threshold */
    //@> FragThreshold=value
    //	value       	
    //		256~2346
    offset += sprintf(config_buf+offset, "FragThreshold=2346\n");

    /* tx burst */
    //@> TxBurst=value
    //	value
    //		0: Disable
    //		1: Enable
    offset += sprintf(config_buf+offset, "TxBurst=1\n");

    /* wmm capable */
    //@> WmmCapable=value
    //	value
    //		0: Disable WMM
    //		1: Enable WMM
    offset += sprintf(config_buf+offset, "WmmCapable=1\n");

    /* ack policy */
    offset += sprintf(config_buf+offset, "AckPolicy=0;0;0;0\n");

    /* auth mode */
    //@> AuthMode=value
    //	value
    //		OPEN	 	For open system	
    //		SHARED	  	For shared key system	
    //		WEPAUTO     Auto switch between OPEN and SHARED
    //		WPAPSK      For WPA pre-shared key  (Infra)
    //		WPA2PSK     For WPA2 pre-shared key (Infra)
    //		WPANONE		For WPA pre-shared key  (Adhoc)
    //		WPA         Use WPA-Supplicant
    //		WPA2        Use WPA-Supplicant
    //
    //      mvWPAauth;		        // 0:Open/WEP 1:WPA-PSK 2:WPA2-PSK
    //      mvAuthenticationType;	// 1:open(default), 2: shared, 3:both 4:WPA-PSK 5:WPA2-PSK
    
    //@> EncrypType=value
    //	value
    //		NONE		For AuthMode=OPEN                    
    //		WEP			For AuthMode=OPEN or AuthMode=SHARED 
    //		TKIP		For AuthMode=WPAPSK or WPA2PSK                    
    //		AES			For AuthMode=WPAPSK or WPA2PSK                     

    //      mvWEPType;  // 0 disable, 1 64bit, 2 128 bit
    //      mvWPAType;	// 0 TKIP, 1 CCMP

    //      mvWEPType : 1 -> 64-bit  , key at mvWEPKEY1-4
    //      mvWEPType : 2 -> 128-bit , key at mvWEP128KEY1-4
    #if 1
    switch(mvAuthenticationType) {
        case 1: /* open */
        case 2: /* shared */
        default:
            if (mvWEPType == 0) {
                offset += sprintf(config_buf+offset, "AuthMode=OPEN\n");
                offset += sprintf(config_buf+offset, "EncrypType=NONE\n");
            }
            else {
                offset += sprintf(config_buf+offset, "AuthMode=WEPAUTO\n");
                offset += sprintf(config_buf+offset, "EncrypType=WEP\n");
            }
            break;
        case 3: /* both */
            break;
        case 4: /* WPA PSK */
            if (mvWPAType == 0)
                offset += sprintf(config_buf+offset, "EncrypType=TKIP\n");
            else
                offset += sprintf(config_buf+offset, "EncrypType=AES\n");

            offset += sprintf(config_buf+offset, "AuthMode=WPAPSK\n");
            break;
        case 5: /* WPA2 PSK */
            if (mvWPAType == 0)
                offset += sprintf(config_buf+offset, "EncrypType=TKIP\n");
            else
                offset += sprintf(config_buf+offset, "EncrypType=AES\n");

            offset += sprintf(config_buf+offset, "AuthMode=WPA2PSK\n");
            break;
    }
    #else
    //offset += sprintf(config_buf+offset, "AuthMode=OPEN\n");
    //offset += sprintf(config_buf+offset, "EncrypType=WEP\n");
    //offset += sprintf(config_buf+offset, "AuthMode=WPANONE\n");
    //offset += sprintf(config_buf+offset, "EncrypType=AES\n");
    offset += sprintf(config_buf+offset, "AuthMode=WPAPSK\n");
    offset += sprintf(config_buf+offset, "EncrypType=AES\n");
    /*
    offset += sprintf(config_buf+offset, "AuthMode=\n");
    offset += sprintf(config_buf+offset, "EncrypType=\n");
    */
    #endif

    /* default key index */
    //@> DefaultKeyID=value
    //	value
    //		1~4
    offset += sprintf(config_buf+offset, "DefaultKeyID=%d\n", mvWEPKeySel+1);

    //@> Key1Type=vaule
    //    Key2Type=value
    //    Key3Type=vaule
    //    Key4Type=vaule
    //    value
    //		0   hexadecimal type
    //		1   assic type
    //    (usage : reading profile only)
    //    //@> Key1Str=value
    //    Key2Str=value
    //    Key3Str=vaule
    //    Key4Str=vaule
    //    value
    //		10 or 26 characters (key type=0)
    //		5 or 13 characters  (key type=1)
    //    (usage : reading profile only)	
    
    offset += sprintf(config_buf+offset, "Key1Type=1\n");
    offset += sprintf(config_buf+offset, "Key2Type=1\n");
    offset += sprintf(config_buf+offset, "Key3Type=1\n");
    offset += sprintf(config_buf+offset, "Key4Type=1\n");

    #if 1
    if ((mvAuthenticationType == 1) || (mvAuthenticationType == 2)) {
        if (mvWEPType == 1) {
                offset += sprintf(config_buf+offset, "Key1Str=%s\n", mvWEPKey1);
                offset += sprintf(config_buf+offset, "Key2Str=%s\n", mvWEPKey2);
                offset += sprintf(config_buf+offset, "Key3Str=%s\n", mvWEPKey3);
                offset += sprintf(config_buf+offset, "Key4Str=%s\n", mvWEPKey4);
        } 
        if (mvWEPType == 2) {
                offset += sprintf(config_buf+offset, "Key1Str=%s\n", mvWEP128Key);
                offset += sprintf(config_buf+offset, "Key2Str=%s\n", mvWEP128Key2);
                offset += sprintf(config_buf+offset, "Key3Str=%s\n", mvWEP128Key3);
                offset += sprintf(config_buf+offset, "Key4Str=%s\n", mvWEP128Key4);
        }
    }

    if ((mvAuthenticationType == 4) || (mvAuthenticationType == 5)) {
        offset += sprintf(config_buf+offset, "WPAPSK=%s\n", mvWPAPass); 
    }

    #else
    offset += sprintf(config_buf+offset, "WPAPSK=termy22688953\n");
    //offset += sprintf(config_buf+offset, "Key1Str=termy22688953\n");
    //offset += sprintf(config_buf+offset, "WPAPSK=0287971234\n");
    //offset += sprintf(config_buf+offset, "WPAPSK=d4607bd36975b7c5272d4cf498cd95acac31e90134b9da663342bf8bdf0f62fb\n");
    //offset += sprintf(config_buf+offset, "Key1Str=termy22688953\n");
    #endif

    //@> PSMode=value
    //    value
    //    	CAM			    Constantly Awake Mode
    //		Max_PSP		    Max Power Savings
    //		Fast_PSP		Power Save Mode
    offset += sprintf(config_buf+offset, "PSMode=CAM\n");

    //@> FastRoaming=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "FastRoaming=0\n");

    //@> RoamThreshold=value
    //	value
    //		Positive Interger(dBm)
    offset += sprintf(config_buf+offset, "RoamThreshold=70\n");

    //@> HT_RDG=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "HT_RDG=1\n");

    //@> HT_EXTCHA=value (Extended Channel Switch Announcement)
    //	value
    //		0				Below
    //		1 				Above
    offset += sprintf(config_buf+offset, "HT_EXTCHA=0\n");

    //@> HT_OpMode=value
    //	value
    //		0				HT mixed format
    //		1				HT greenfield format
    offset += sprintf(config_buf+offset, "HT_OpMode=0\n");

    //@> HT_MpduDensity=value
    //	value (based on 802.11n D2.0)
    //		0: no restriction
    //		1: 1/4 £gs
    //		2: 1/2 £gs
    //		3: 1 £gs
    //		4: 2 £gs
    //		5: 4 £gs
    //		6: 8 £gs
    //		7: 16 £gs
    offset += sprintf(config_buf+offset, "HT_MpduDensity=4\n");

    //@> HT_BW=value
    //	value
    //		0				20MHz
    //		1				40MHz
    offset += sprintf(config_buf+offset, "HT_BW=%d\n", mvBandWidth);

    //@> HT_AutoBA=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "HT_AutoBA=1\n");

    //@> HT_BADecline
    //	value
    //		0				Disabled
    //		1			    Enabled <Reject BA request from AP>
    offset += sprintf(config_buf+offset, "HT_BADecline=0\n");

    //@> HT_AMSDU=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "HT_AMSDU=0\n");

    //@> HT_BAWinSize=value
    //	value
    //		1 ~ 64
    offset += sprintf(config_buf+offset, "HT_BAWinSize=64\n");
    
    //@> HT_GI=value
    //	value
    //		0				long GI
    //		1				short GI
    offset += sprintf(config_buf+offset, "HT_GI=1\n");

    //@> HT_MCS=value
    //	value
    //		0 ~ 15
    //		33: auto
    offset += sprintf(config_buf+offset, "HT_MCS=33\n");

    //@> HT_MIMOPSMode=value
    //	value (based on 802.11n D2.0)
    //		0				Static SM Power Save Mode
    //		1				Dynamic SM Power Save Mode
    //		2				Reserved
    //		3				SM enabled
    //	(not fully support yet)
    offset += sprintf(config_buf+offset, "HT_MIMOPSMode=3\n");
    
    //@> IEEE80211H=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "IEEE80211H=0\n");

    //@> TGnWifiTest=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "TGnWifiTest=0\n");

    //@> WirelessEvent=value
    //	value
    //		0				Disabled
    //		1				Enabled <send custom wireless event>
    offset += sprintf(config_buf+offset, "WirelessEvent=0\n");

    //@> MeshId=value
    //	value
    //		Length 1~32 ascii characters
    offset += sprintf(config_buf+offset, "MeshId=MESH\n");

    //@> MeshAutoLink=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "MeshAutoLink=1\n");

    //@> MeshEncrypType=value
    //	value
    //		NONE		For MeshAuthMode=OPEN                    
    //		WEP			For MeshAuthMode=OPEN
    //		TKIP		For MeshAuthMode=WPANONE
    //		AES			For MeshAuthMode=WPANONE
    offset += sprintf(config_buf+offset, "MeshAuthMode=OPEN\n");
    offset += sprintf(config_buf+offset, "MeshEncrypType=NONE\n");

    //@> MeshDefaultkey=value
    //	value
    //		1~4
    offset += sprintf(config_buf+offset, "MeshDefaultkey=1\n");

    //@> CarrierDetect=value
    //	value
    //		0				Disabled
    //		1				Enabled
    offset += sprintf(config_buf+offset, "CarrierDetect=0\n");

    if (mvDataRate <= 3) {
        offset += sprintf(config_buf+offset, "FixedTxMode=CCK\n");
        offset += sprintf(config_buf+offset, "HT_MCS=%d\n", mvDataRate);
    }
    if (mvDataRate >= 4 && mvDataRate <= 11) {
        offset += sprintf(config_buf+offset, "FixedTxMode=OFDM\n");
        offset += sprintf(config_buf+offset, "HT_MCS=%d\n", mvDataRate - 4);
    }
    if (mvDataRate >= 12 && mvDataRate <= 27) {
        offset += sprintf(config_buf+offset,"FixedTxMode=HT\n");
        offset += sprintf(config_buf+offset,"HT_MCS=%d\n", mvDataRate - 12);
    }

    offset += sprintf(config_buf+offset, "BeaconLostTime=4\n");
    offset += sprintf(config_buf+offset, "RaidoOn=1\n");
    offset += sprintf(config_buf+offset, "WIDIEnable=1\n");
    offset += sprintf(config_buf+offset, "PktAggregate=0");

    /* for EDCCA */
    offset += sprintf(config_buf+offset, "EDCCA_ED_TH=95\n");
    offset += sprintf(config_buf+offset, "EDCCA_FALSE_CCA_TH=200\n");
    offset += sprintf(config_buf+offset, "EDCCA_BLOCK_CHECK_TH=2\n");
    offset += sprintf(config_buf+offset, "ED_MODE=1\n");
    offset += sprintf(config_buf+offset, "ED_LEARN_TH=50\n");
    offset += sprintf(config_buf+offset, "EDCCA_STA_RSSI_TH=-80\n");

    printk("%s\n", config_buf);
    return config_buf;
}
//#endif

#if (WLANMAC == WLZD1211)
#if defined(CODE2)
extern void wpa_task(int unused,void *unused1,void *unused2);
extern void wsc_task(int unused,void *unused1,void *unused2);
#endif	//CODE2
#endif

void Wlan_MacInit(void){

	int status;

	WLan_get_EEPData();  // get initial setting ... 7/2/2003
	return;
}

void Wlan_reset(void){
    
	unsigned int value;
    int i;

	value = GPIOA_DATA_INPUT_REG;
	value &= ~GPIO_13_MASK;
	GPIOA_DATA_OUTPUT_REG = value;

    cyg_thread_delay(20000);
    for (i = 0; i < 0x1ffffff; i++);

	value = GPIOA_DATA_INPUT_REG;
	value |= GPIO_13_MASK;
	GPIOA_DATA_OUTPUT_REG = value;
    
}

//extern PRTMP_ADAPTER       Global_pAd;
int Wlan_SendPacket(struct sk_buff* pSkb){
//not yet	struct mbuf* skbuf;

    if( WirelessInitFailed ) {
        if (pSkb)
            dev_kfree_skb_any(pSkb);
        return -1;
    } 

    /*
    if(pSkb) {
        dev_kfree_skb_any(pSkb);
    }
    return 0;
    */
        
    /*
    if (wlan_route_onoff) {
	    rt28xx_send_packets(pSkb, g_wireless_dev);
	    WirelessLightToggle++;	
    }
    else
        dev_kfree_skb_any(pSkb);
    */
    rt28xx_send_packets(pSkb, g_wireless_dev);
    WirelessLightToggle++;	

    return 0;	
}

int Wlan_Diagonse(void){

	if( WirelessInitFailed )
		return 1;
    printk("%s\n", __FUNCTION__);
	return 1;
}

void wlan_site_survey(void){
	int Port = 0;
	WORD PrinterStatus;
	
	if( WirelessInitFailed )
		return;

    printk("%s\n", __FUNCTION__);

#if defined(CODE2)
#ifdef USE_PS_LIBS
	for(Port = 0 ; Port < NUM_OF_PRN_PORT ; Port++) {
		PrinterStatus = PrnGetPrinterStatus(Port);
		if (PrinterStatus != PrnNoUsed)
			return;
	}
#endif
#endif //CODE2

    //rt_ioctl_siwscan(g_wireless_dev, NULL, NULL, NULL);
	return;
	
}

knownbss_t* APList = NULL;
knownbss_t *wlan_get_scanlist(void){
		
	if( WirelessInitFailed )
		return NULL;
#if 1
    RTMP_ADAPTER *pAd;
    knownbss_t *pKnown_bss = NULL;
    int i;

    printk("%s\n", __FUNCTION__);

	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

	// Clean APList
	for ( ; APList != NULL; APList = pKnown_bss) {
		pKnown_bss = APList->next;
		free(APList);
	}

	// Put all list to APList
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) {
		
		if (pAd->ScanTab.BssEntry[i].Channel == 0)
			break;
		
		pKnown_bss = malloc(sizeof(knownbss_t));

		if (pKnown_bss == NULL)
			continue;
		memset(pKnown_bss, 0, sizeof(knownbss_t));

		memcpy(pKnown_bss->bssid, pAd->ScanTab.BssEntry[i].Bssid, MAC_ADDR_LEN);
		pKnown_bss->channel = pAd->ScanTab.BssEntry[i].Channel;

		pKnown_bss->cap_info = pAd->ScanTab.BssEntry[i].BssType;	// 0:Ad-Hoc 1:Infra

		if(pAd->ScanTab.BssEntry[i].Privacy) {
			
			pKnown_bss->cap_info |= 1 << 4;
			if(pAd->ScanTab.BssEntry[i].AuthMode > 2)
				pKnown_bss->cap_info |= 1 << 8;
		}
		
		memset(pKnown_bss->ssid,0,WLAN_SSID_MAXLEN+2+1);
		memcpy(pKnown_bss->ssid, pAd->ScanTab.BssEntry[i].Ssid, pAd->ScanTab.BssEntry[i].SsidLen); 
		pKnown_bss->rssi = pAd->ScanTab.BssEntry[i].Rssi;

		if (APList == NULL)
			APList = pKnown_bss;
		else {
			pKnown_bss->next = APList;
			APList = pKnown_bss;
		}        
		
	}		

	return APList;
#else
    return NULL;
#endif

#if 0
	VOID *pAd = NULL;


    struct iw_point iw_p;

    char* user_data = malloc(4096);
    iw_p.pointer = NULL;
    iw_p.length = 4096;

	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);


    //StaSiteSurvey(pAd, NULL, SCAN_ACTIVE);

    rt_ioctl_siwscan(g_wireless_dev, NULL, NULL, NULL);
    mdelay(5000);
	rt_ioctl_giwscan(g_wireless_dev, NULL, &iw_p, user_data);
    
    rt_ioctl_iwaplist(g_wireless_dev, NULL, &iw_p, user_data);

    free(user_data);
#endif
    return APList;
}

void wlan_get_currbssid(unsigned char huge * bssid){
	
    void *pAd = NULL;
    struct sockaddr ap_addr;

	if( WirelessInitFailed ){
		memset(bssid, 0 ,WLAN_BSSID_LEN);
		return;
	}	

    if (!rt_ioctl_giwap(g_wireless_dev, NULL, &ap_addr, NULL))
        memcpy(bssid, ap_addr.sa_data, WLAN_BSSID_LEN);
}

void wlan_get_currssid(unsigned char huge * ssid){
	
    struct iw_point iw_p;

	if( WirelessInitFailed ){
		memset(ssid, 0 ,WLAN_SSID_MAXLEN);
		return;
	}

    rt_ioctl_giwessid (g_wireless_dev, NULL, &iw_p, ssid);
}

int wlan_get_channel(void){
	
    RTMP_ADAPTER *pAd = NULL;

	if( WirelessInitFailed )
		return -1;

	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

    if (pAd)
        return pAd->MlmeAux.Channel;
    else
        return -1;
}

int wlan_get_currrate(void){
	union iwreq_data wrqu;

	if( WirelessInitFailed )
		return -1;

    rt_ioctl_giwrate(g_wireless_dev, NULL, &wrqu, NULL);
	return wrqu.bitrate.value/1000000;
}

int wlan_get_rssi(void) {

    RTMP_ADAPTER *pAd = NULL;
    CHAR AvgRssi0;

	if( WirelessInitFailed )
		return -1;

	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);
    RtmpIoctl_rt_ioctl_rssi(pAd, (void *)&AvgRssi0, 0);

	return (int)AvgRssi0;
}

int wlan_get_linkquality(void) {
	
    RTMP_ADAPTER *pAd = NULL;

	if( WirelessInitFailed )
		return -1;

	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);
    if (pAd)
        return pAd->Mlme.ChannelQuality;
    else return -1;
}

//eason 20110314	extern int rtl871x_get_linkup();

//1:link up 0: link down
int wlan_get_linkup ()
{
	PWSC_CTRL    pWscControl;
    RTMP_ADAPTER *pAd;
	int state = 0;

	if( WirelessInitFailed )
		return -1;

	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

    pWscControl = &pAd->StaCfg.WscControl;

    /*
	state = rtl871x_get_linkup();
    */
	if ((pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
	    (pAd->StaCfg.WscControl.bWscTrigger
	    )) {
        //if ((pWscControl->WscStatus == STATUS_WSC_LINK_UP) && wlan_route_onoff)
        if (pWscControl->WscStatus == STATUS_WSC_LINK_UP)
            state = 1;
    } else {
        //if ((pAd->ExtraInfo == GENERAL_LINK_UP) && wlan_route_onoff)
        if (pAd->ExtraInfo == GENERAL_LINK_UP)
            state = 1;
    } 

	return state;
}


#ifdef AUTH_8021X
int wlan_set_encrypt(uint32 action){
	return -1;
}

int wlan_set_exunencrypt(uint32 action){
	return -1;
}

void wlan_eap_get_frame()
{
}
#endif /* AUTH_8021X */

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
}
#endif /* WPA_PSK_TKIP */

void wlan_set_wps_on()
{
    char* config_buffer = NULL; 

 	if( WirelessInitFailed )
		return NULL;

    RTMP_ADAPTER *pAd;
	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);
   
    mvBSSType = 0;
    EEPROM_Data.WLMode = 0;
    mvTxMode 	= 9;
    EEPROM_Data.WLTxMode = 9;

    config_buffer = WLan_config();
    if (config_buffer)
        RTMPSetProfileParameters (pAd, config_buffer);

    printk("wps on\n");
    //pAd->StaCfg.WscControl.WscDriverAutoConnect = 0x01;
    Set_WscConfMode_Proc(pAd, "1");  /* enrollee */
    Set_WscMode_Proc(pAd, "2");      /* PBC Mode */
    Set_WscGetConf_Proc(pAd, "1");   /* wsc_start */

    if (config_buffer)
        free(config_buffer);
}

/*
void wlan_connect()
{
    
  	if( WirelessInitFailed )
		return;

    if (wlan_route_onoff)
        return;

    RTMP_ADAPTER *pAd;
    GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

    printk("wlan_connect\n");

    wlan_route_onoff = 1;
   
}

void wlan_disconnect()
{
    
 	if( WirelessInitFailed )
		return;

    
    if ((wlan_route_onoff == 0) || (wlan_route_onoff == 2))
        return;

    RTMP_ADAPTER *pAd;
	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

    printk("wlan_disconnect\n");

    wlan_route_onoff = 0;
    
}
*/

void wlan_check_edcca()
{
#if 1
 	if( WirelessInitFailed )
		return;

    RTMP_ADAPTER *pAd;
	GET_PAD_FROM_NET_DEV(pAd, g_wireless_dev);

#ifdef ED_MONITOR
	if(pAd->ed_chk != EDCCA_OFF)
		ed_status_read(pAd);
#ifdef ED_SMART
	if(pAd->ed_chk == EDCCA_SMART)
		ed_state_judge(pAd);
#endif /* ED_SMART */
#endif /* ED_MONITOR */
#endif
}



