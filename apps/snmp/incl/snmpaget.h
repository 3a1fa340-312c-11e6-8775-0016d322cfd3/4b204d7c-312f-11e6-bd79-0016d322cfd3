#ifndef _SNMPAGENT_H
#define _SNMPAGENT_H

void SnmpAgent(cyg_addrword_t data);	//615wu
int snmp_agent_parse(snmp_session *session, uint8 *indata,int inlength,uint8 *outdata,int *outlength);

#endif  _SNMPAGENT_H
