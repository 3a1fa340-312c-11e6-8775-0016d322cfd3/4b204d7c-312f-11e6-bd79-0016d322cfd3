#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "socket2.h"
#include "atalkd.h"
#include "zip.h"

extern void LightOnForever(unsigned char LED);

int zip_getnetinfo(void);
void zip_info_query(cyg_addrword_t data)
{
	uint16 net;
	uint8 *needwait =data;	//615wu

    while(!ATD_INIT_OK && (*needwait)) 
        cyg_thread_yield();

	do {
		if(*needwait) ppause(60*1000L);	 //wait 1 min
	
		if(zip_getnetinfo()) {
			net = ntohs(at_iface.my.s_net);
			if(  (net < at_iface.netrange.first) ||
			     (net > at_iface.netrange.last) )
			{
				at_iface.status = AT_PROBING;
				aarp_probe(); //Find a Zone Name
			}
#ifndef _PC
			if(at_iface.my.s_net != EEPROM_Data.ATNet ||
			   at_iface.my.s_node != EEPROM_Data.ATNode) {
				EEPROM_Data.ATNet = at_iface.my.s_net;
				EEPROM_Data.ATNode = at_iface.my.s_node;
				if(WriteToEEPROM(&EEPROM_Data) != 0) LightOnForever(Status_Lite); // Write EEPROM Data
			}
#endif !_PC

		}
		else {
			at_iface.gate.s_net     = 0;
			at_iface.gate.s_node    = 0;
			at_iface.netrange.first = 0;
			at_iface.netrange.last  = STARTUP_LASTNET;
			memcpy(at_iface.zonename,"*",2); //Zone not found
		}
        sys_check_stack();
	}while(*needwait);
}


//return code : != 0 : found a Zone Name
int zip_getnetinfo(void)
{
	struct sockaddr_at	lsock,fsock;
	uint8  *data, sendpkt[40], recvpkt[80], flags;
	int16  s, retry, fromlen,sendsize, recvsize;
	data = sendpkt;
	*data++ = ZIP_DDPTYPE;	//DDP type
	*data++ = ZIPOP_GNI;	//GetNetInfo
	memset(data,'\0',5);
	data += 5;

	//*
	//* Set our requesting zone to NULL, so the response will contain
	//* the default zone.
	//*
//	*data = strlen(at_iface.zonename);
//	memcpy(data+1, at_iface.zonename,*data);
	*data = strlen(EEPROM_Data.ATZoneName);
	memcpy(data+1, EEPROM_Data.ATZoneName, *data);
	data += *data + 1;

	lsock.sat_family = AF_APPLETALK;
	lsock.sat_addr.s_net = ATADDR_ANYNET;
	lsock.sat_addr.s_node = ATADDR_ANYNODE;
	lsock.sat_port = ZIP_SOCKET_NUM;
	lsock.sat_len = sizeof(struct sockaddr_at);	//615wu

	fsock.sat_family = AF_APPLETALK;
	fsock.sat_addr.s_net = 0;
	fsock.sat_addr.s_node = ATADDR_BCAST;
	fsock.sat_port = ZIP_SOCKET_NUM;
	fsock.sat_len = sizeof(struct sockaddr_at);	//615wu

	s = socket2( AF_APPLETALK, SOCK_DGRAM, 0 );	
	
	setsocketopt2(s,SO_RCV_TIMEOUT);

	if(bind2(s, (struct sockaddr *)&lsock,sizeof(lsock)) < 0) {

		close_s2(s);
		return 0;
	}
	sendsize = data - sendpkt;

	retry = 0;
	do {
		if(sendto2( s, sendpkt, sendsize, 0, (struct sockaddr *)&fsock,
		   sizeof( struct sockaddr_at )) < 0 ) {

#ifdef PC_OUTPUT
			printf("Send ZIP GNI Error !\n");
			return 0;
#endif PC_OUTPUT
		}

//ZIP GetNetInfo Reply

		fromlen = sizeof(fsock);		
		recvsize = recvfrom2(s, recvpkt, sizeof(recvpkt), 0,(struct sockaddr *)&fsock, &fromlen );	
		if(recvsize > 8 && recvpkt[0] == ZIP_DDPTYPE && recvpkt[1] == ZIPOP_GNIREPLY) break;
	} while(++retry < 3 );

	close_s2(s);

	if(retry >= 3) return 0;

	flags = recvpkt[2];	//flags bit 7:zone invalid, 6:use broadcast, 5:only one zone

	data = recvpkt + 7;	//point to zone name length

	//get zone name	6/17/99 enable
	if(!(flags & ZIPGNI_INVALID)) {
		if( *data == 0) return 0; //something wrong !!
		memcpy(at_iface.zonename,data+1,*data);
		at_iface.zonename[*data] = '\0';
	}
	data += *data + 1;

	//get multicast address
	if(!(flags & ZIPGNI_USEBROADCAST)) {
		if(*data == 6) {
#ifndef _PC
			// Charles
			// AsmAddMultiAddr(data+1);
#endif  _PC
		}
	}
	data += *data + 1;

	if((flags & ZIPGNI_INVALID)) {
		if(*data == 0) return 0; //something wrong !!
		memcpy(at_iface.zonename,data+1,*data);
		at_iface.zonename[*data] = '\0';
	}

	data = recvpkt + 3; //point to Network number range start

	at_iface.netrange.first = ntohs(NGET16(data));
	data += sizeof(uint16);
	at_iface.netrange.last  = ntohs(NGET16(data));
	data += sizeof(uint16);

	//get the gateway address
	at_iface.gate = fsock.sat_addr;

	return 1;
}
