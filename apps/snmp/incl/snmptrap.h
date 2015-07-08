#ifndef _SNMP_TRAP_H
#define _SNMP_TRAP_H

#include "asn1.h"
#include "snmp_api.h"

extern CommunityAuth SnmpCommunityAuth[NO_OF_SNMP_COMMUNITY];
void SendTrap(int generic_trap,int specific_trap,char *description);

#endif  _SNMP_TRAP_H
