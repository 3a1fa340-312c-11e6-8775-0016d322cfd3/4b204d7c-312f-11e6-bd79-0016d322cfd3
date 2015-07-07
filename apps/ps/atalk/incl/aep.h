#ifndef _AEP_H
#define _AEP_H

#define MAX_AEP_LEN			587
#define AEPOP_REQUEST		1
#define AEPOP_REPLY			2

#define AEP_DDPTYPE			4
#define AEP_SOCKET_NUM		4

void aep_input(cyg_addrword_t data);

#endif  _AEP_H
