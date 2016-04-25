#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "socket2.h"
#include "atalkd.h"

uint8 aep_packet[MAX_AEP_LEN];

void aep_input(cyg_addrword_t data)
{
	struct sockaddr_at	addr;
	int	   s,fromlen,rcv_len;

    cyg_semaphore_wait(&ATD_INIT_OK);

	if((s = socket2( AF_APPLETALK, SOCK_DGRAM, 0 )) < 0) {
#ifdef PC_OUTPUT
		printf("\aAEP SOCKET() ERROR !\a\n");
#endif PC_OUTPUT
		return;
	}

	addr.sat_family = AF_APPLETALK;
	addr.sat_addr   = at_iface.my;
	addr.sat_port = AEP_SOCKET_NUM;
	addr.sat_len = sizeof(struct sockaddr_at);


	if(bind2( s, (struct sockaddr *)&addr,sizeof( struct sockaddr_at )) < 0 ) {
#ifdef PC_OUTPUT
		printf("\aAEP BIND() ERROR !\a\n");
#endif PC_OUTPUT
		close_s2(s);
		return;
	}
	for(;;) {
		fromlen = sizeof(addr);	
		if((rcv_len = recvfrom2(s, aep_packet, sizeof(aep_packet), 0,
		              (struct sockaddr *)&addr, &fromlen)) > 0 )
		{
			if ( rcv_len < 2 || aep_packet[0] != AEP_DDPTYPE ||
			    aep_packet[1] != AEPOP_REQUEST ) {
				continue;
			}
			aep_packet[1] = AEPOP_REPLY;			
			sendto2( s,aep_packet,rcv_len, 0, (struct sockaddr *)&addr,
			        sizeof( struct sockaddr_at ));		        
		}
	} //for(;;);
}
