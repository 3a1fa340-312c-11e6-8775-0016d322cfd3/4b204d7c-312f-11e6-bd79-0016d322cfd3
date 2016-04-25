#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "atalkd.h"
#include "eeprom.h"

extern void papd_init(void);
uint8 ZIP_NEED_WAIT = 0;	//615wu

//atalkd_init();
//aarp_probe();
//zip();
//nbp_lookup();

struct AT_IFACE at_iface;

void atalkd_init(cyg_addrword_t data)
{
	uint8 probe_ok = 0;
//Jesse modify 2008/2/18
    ppause(5000);	//615wu

//	strcpy(at_iface.zonename, EEPROM_Data.ATZoneName);	//4/23/99 get from ZIP.C

	at_iface.my.s_net = EEPROM_Data.ATNet;
	at_iface.my.s_node = EEPROM_Data.ATNode;
	at_iface.status         = AT_PROBING;

	if(at_iface.my.s_net != 0 && at_iface.my.s_node != 0) {
		at_iface.netrange.first = at_iface.netrange.last  = htons(at_iface.my.s_net);
		probe_ok = probe_last_addr();	//prob last address
	}

	if(!probe_ok)
	{
		at_iface.netrange.first = STARTUP_FIRSTNET;
		at_iface.netrange.last  = STARTUP_LASTNET;
		aarp_probe();
	}

	ZIP_NEED_WAIT = 0;					//615wu
	zip_info_query(&ZIP_NEED_WAIT);

	papd_init();
    cyg_semaphore_post(&ATD_INIT_OK);
}
