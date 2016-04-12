
#include "wlanif.h"

int diag_flag;

void wlan_site_survey(void)
{

}

void wlan_get_currssid(unsigned char *ssid) 
{
}

void wlan_get_currbssid(unsigned char *bssid) 
{
}

int  wlan_get_currrate(void)
{
    return 0;
}

int wlan_get_channel(void)
{
    return 0;
}

knownbss_t* APList = NULL;
knownbss_t *wlan_get_scanlist(void)
{
    return NULL;
}

int wlan_get_rssi(void) 
{
    return 0;
}

int wlan_get_linkquality(void)
{
    return -1;
}

int wlan_set_anyssid(char *deststr)
{
    return 0;
}



