#ifndef WLANINFO_H
#define WLANINFO_H
//#include "ctypes.h"

//#include "psglobal.h"
//#include "pstarget.h"
//#include "rt_config.h"

/*WLANMAC */
#define PRISM2                      0x11
#define PRISM3                      0x12
#define AX81190                     0x13
#define ISL80211G                   0x14
#define MTK7601                     0x76
#define RT8188                      0x88
#define RT2561						0x99
#define WLZD1211					0x9A
#define RT2571						0x9B
#define NONWLAN                     0xFE

#define WLAN_BSSID_LEN              6
#define WLAN_SSID_MAXLEN            32
#if 1
#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
//#define BIT8	0x00000100	//Kevin: Conflict w/RT2561 Driver
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
#endif

// ----- for old structure ----
#define P80211ENUM_bsstype_80211ibss		0
#define P80211ENUM_bsstype_infrastructure	1
#define P80211ENUM_bsstype_independent		2
#define P80211ENUM_bsstype_any				3

#define INL_MODE_NONE						-1
#define INL_MODE_PROMISCUOUS				0
#define INL_MODE_CLIENT						1
#define INL_MODE_AP							2
#define INL_MODE_SNIFFER					3
// ----- for old structure ----

#define WLAN_GET_MGMT_CAP_INFO_ESS(n)				((n) & BIT0)
#define WLAN_GET_MGMT_CAP_INFO_IBSS(n)				(((n) & BIT1) >> 1)
#define WLAN_GET_MGMT_CAP_INFO_CFPOLLABLE(n)		(((n) & BIT2) >> 2)
#define WLAN_GET_MGMT_CAP_INFO_CFPOLLREQ(n)			(((n) & BIT3) >> 3)
#define WLAN_GET_MGMT_CAP_INFO_PRIVACY(n)			(((n) & BIT4) >> 4)
#define WLAN_GET_MGMT_CAP_INFO_SHORT_PREAMBLE(n)	(((n) & BIT5) >> 5)
#define WLAN_GET_MGMT_CAP_INFO_PBCC(n)				(((n) & BIT6) >> 6)
#define WLAN_GET_MGMT_CAP_INFO_CHANNEL_AGILITY(n)	(((n) & BIT7) >> 7)

extern unsigned char WirelessInitFailed;

extern UINT    mvBSSType;
extern UINT8   mvFirmwareVersion[16];
extern UINT8	ZRegDomain[12]; //Eldan added for test	
extern UINT8   mvESSID[33];
extern UINT    mvChannel_start; 
extern UINT    mvChannel_end; 
extern UINT    mvChannel; 
extern UINT    mvWEPType;          //0 disable, 1 64bit, 2 128 bit
extern UINT    mvWEPKeySel;        //0,1,2,3
extern UINT8   mvWirelessMAC[6];   // wireless MAC ID
extern UINT	   mvWPAType;
extern UINT8	mvWPAPass[65];

extern UINT8   mvEDDMAC[6];
extern UINT8   mvWEPManual;        //0 Auto, 1 Manual
extern UINT16  mvWZone;        	//0xA1 USA / CANADA ( Northern American )
							//0xA2 ETSI ( Most of Europe and Australia )
							//0xA3 France
							//0xA4 Japan	1-14
							//0xA5 Spain
							//0xA6 Japan	14
							
extern UINT8   mvWDomain;          //0x0010 USA    1-11
                            //0x0020 CANADA 1-11
                            //0x0030 ETSI   1-13
                            //0x0031 SPAIN  10-11
                            //0x0032 France 10-13
                            //0x0040 Japan  14
                            //0x0041 Japan  1-14

extern UINT8   mvWEPKey[32];   // cannot be NULL
extern UINT8   mvWEPKey1[6];
extern UINT8   mvWEPKey2[6];
extern UINT8   mvWEPKey3[6];
extern UINT8   mvWEPKey4[6];

extern UINT8   mvWEP128Key[15];
extern UINT8   mvWEP128Key2[15];
extern UINT8   mvWEP128Key3[15];
extern UINT8   mvWEP128Key4[15];


extern UINT    mvBeaconinterval;   //default 100 msec
extern UINT    mvRTSThreshold;     //default 2432, 0 ~ 3000
extern UINT    mvFragmentation;    //256 ... 2346, Only EVEN number, default 2346
extern UINT    mvRates;            //1 ... 15, 3(1,2) 15(1,2,5.5, 11) 
extern UINT    mvRate;             //

extern UINT    mvShortPreamble;    // 1:short, 0: long, default 0
extern UINT    mvAuthenticationType;  // 1:open, 2: shared, 3:both default 1
extern UINT    mvDtiminterval;     //1 ~ 65535, default 1
extern UINT    mvCfpperiod;        //1 ~ 65535, default 3
extern UINT    mvCfpmaxduration;   //1 ~ 65535, default 100

extern UINT16	mvTxPower;
extern UINT    mvCRX; //0 or 1 ... continuous RX
extern UINT    mvCTX; //0 or 1 ... continuous RX
extern UINT8   mvJapan;   //0=usa 1=japan default=0  
extern UINT8   mvAn_side; //an_side  default=0 
                   //an_side  SPREAD ON  STATE 1=life 2=right 3:diversity
                   //an_side  SPREAD OFF STATE 1=life 2=right 3:diversity 
extern UINT    mvNonModulate; //0 or 1
extern UINT    mvExtRate;
extern UINT    mvConfigMode;
extern UINT	   mvWPAauth;
extern UINT8   mvTxMode;
extern UINT    mvBandWidth;		// 0:20MHz 1:40MHz
extern UINT    mvDataRate;

typedef struct knownbss
{
	/* BSS info */
	UINT8           bssid[WLAN_BSSID_LEN];
	UINT32          channel;
	UINT8   	    rate;
	UINT16          bcn_int;
	UINT16          cap_info;
	UINT8           ssid[WLAN_SSID_MAXLEN+2+1];  /* extra for \0 */

	/* Driver support */
	UINT32          ttl;            /* TODO: add aging of BSS's */
	INT32           rssi;          /* signal strength */ 
	UINT8			sq;            /* signal quality */
	struct knownbss *next;
} knownbss_t;

extern knownbss_t* APList;

int wlan_set_anyssid(char *deststr);

/* wlan interface functions */
void Wlan_MacInit(void);
void Wlan_reset(void);
//eason 20110314	int Wlan_SendPacket(struct sk_buff* pSkb);
int Wlan_Diagonse(void);

/* get wlan management information */
void wlan_site_survey(void);
knownbss_t *wlan_get_scanlist(void);
void wlan_get_currbssid(unsigned char * bssid);
void wlan_get_currssid(unsigned char * ssid);
int wlan_get_channel(void);
int wlan_get_currrate(void);
int wlan_get_rssi(void);
int wlan_get_linkquality(void);

#endif
