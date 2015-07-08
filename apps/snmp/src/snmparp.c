#include <string.h>
#include "psglobal.h"	//615wu
#include "snmparp.h"

static int ArpHashIndex;
static struct arp_tab *ap;

//615wu
#if 0
void ARP_Scan_Init(void)
{
	ArpHashIndex = 0;
	ap = Arp_tab[0];
}

int ARP_Scan_Next(uint32 *Addr,uint8 *PhysAddr)
{
	while(ArpHashIndex < HASHMOD) {
		while(ap != NULL){
			*Addr = ap->ip_addr;
			memcpy(PhysAddr,ap->hw_addr,6);
			ap = ap->next;
			return 1;
		}
		ap = Arp_tab[++ArpHashIndex];
	}
	return 0;
}
#endif
