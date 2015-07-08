#ifndef _SNMPARP_H
#define _SNMPARP_H

void ARP_Scan_Init(void);
int ARP_Scan_Next(uint32 *Addr,uint8 *PhysAddr);

#endif  _SNMPARP_H
