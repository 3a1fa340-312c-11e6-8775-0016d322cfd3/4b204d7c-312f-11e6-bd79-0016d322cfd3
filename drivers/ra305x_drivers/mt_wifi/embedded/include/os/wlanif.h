#ifndef WLANINFO_H
#define WLANINFO_H
//#include "ctypes.h"

#include "psglobal.h"
#include "pstarget.h"
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

typedef struct knownbss
{
	/* BSS info */
	uint8           bssid[WLAN_BSSID_LEN];
	uint32          channel;
	uint8   	    rate;
	uint16          bcn_int;
	uint16          cap_info;
	uint8           ssid[WLAN_SSID_MAXLEN+2+1];  /* extra for \0 */

	/* Driver support */
	uint32          ttl;            /* TODO: add aging of BSS's */
	int32           rssi;          /* signal strength */ 
	uint8			sq;            /* signal quality */
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
void wlan_get_currbssid(unsigned char huge * bssid);
void wlan_get_currssid(unsigned char huge * ssid);
int wlan_get_channel(void);
int wlan_get_currrate(void);
int wlan_get_rssi(void);
int wlan_get_linkquality(void);

#endif
