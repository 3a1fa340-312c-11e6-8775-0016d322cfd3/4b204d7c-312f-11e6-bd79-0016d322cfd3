#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "eeprom.h"
#include "socket2.h"
#include "atalkd.h"
#include "atp.h"
#include "atprn.h"

#define MAX_NBP_TUPLE_LEN (ATALK_ZONE_LEN+ATALK_NAME_LEN+ATALK_TYPE_LEN+5)
#define NBP_LOOKUP_LEN    (MAX_NBP_TUPLE_LEN+3)
#define NBP_INPUT_LEN	  (MAX_NBP_TUPLE_LEN*3+3)
//#define AT_PORT_TYPE      "LaserWriter"  //4/19/2000 marked

extern uint16 NGET16( uint8 *pSrc );
extern int memicmp(const void * first, const void * last, unsigned int count);
extern void Node2Num(BYTE *buf,BYTE offset);

static void NCOPY16( uint8 *pDest, uint8 *pSrc )
{
	pDest[0] = pSrc[0];
	pDest[1] = pSrc[1];
}

uint8 nbp_used[NUM_OF_PRN_PORT]; //nbp name have been used or not !

uint8 tmp_obj[32],tmp_type[32],tmp_zone[32];
uint8 olen,tlen,zlen;

static uint8 ATPortName[ATALK_NAME_LEN+1];
static uint8 ATPortNameLength;

static uint16 nbp_set(int16 *len,uint8 *buf);
static int16 nbp_match(uint8 *entity);
static uint8 * strnchr(uint8 *s,int16 len,uint8 c);
static int16 nbpMatch(uint8 *pat,uint16 plen, uint8 *thing);
static int16 nbp_find(int i);
static void SetATPortName(void);

//return code :
//      < 0 : error ,
//      = 0 : ok
int16 nbp_rgstr(void)
{
	uint8 nbp_send[NBP_LOOKUP_LEN],nbp_recv[NBP_LOOKUP_LEN];
	struct sockaddr_at	addr;
	int	   s, length,tries,tries1,rcv_len,i,j;
	uint8  *data, *en;
	struct nbphdr *snh, *rnh;
	struct nbptuple *snt;
	SetATPortName();

	if( (s = socket2( AF_APPLETALK, SOCK_DGRAM, 0 )) < 0) return (-1);

	setsocketopt2(s,SO_RCV_TIMEOUT);

	addr.sat_family = AF_APPLETALK;
	addr.sat_addr   = at_iface.my;
	addr.sat_port   = NBP_SOCKET_NUM;
	addr.sat_len = sizeof(struct sockaddr_at);	//615wu


	if(bind2( s, (struct sockaddr *)&addr,sizeof( struct sockaddr_at )) < 0 ) {

		close_s2(s);
		return( -1 );
	}

	data = nbp_send;
	*data++ = NBP_DDPTYPE;

	snh = (struct nbphdr*) data;
	if(at_iface.zonename[0] == nbpStar)	snh->nh_op = NBPOP_LKUP;	//no router
	else                            snh->nh_op = NBPOP_BRRQ;	//have a router
	snh->nh_cnt = 1;
	data += SZ_NBPHDR;

	snt = (struct nbptuple *)data;
	NCOPY16( &snt->nt_net, &addr.sat_addr.s_net );
	snt->nt_node = addr.sat_addr.s_node;
	snt->nt_port = addr.sat_port;
	data += SZ_NBPTUPLE;

	en = data;	//point to  entity name of NBP tuple

	addr.sat_family = AF_APPLETALK;
	if(at_iface.zonename[0] == nbpStar) {
		addr.sat_addr.s_net = 0;
		addr.sat_addr.s_node = ATADDR_BCAST;
	} else {
		addr.sat_addr = at_iface.gate;
	}
	addr.sat_port = NBP_SOCKET_NUM;

	tries = 3;

	do {
		for (i = 0 ; i < NUM_OF_PRN_PORT; i++) {
			if(nbp_used[i]) continue;

			data = en;
			snh->nh_id = i+1;
			snt->nt_enum = i;

			length = ATPortNameLength;
			*data++ = length;
			memcpy(data,GetATPortName(i),length);
			data += length;

			length = strlen(EEPROM_Data.ATPortType[i]);
//			length = sizeof(AT_PORT_TYPE)-1;
			*data++ = length;
			memcpy(data, EEPROM_Data.ATPortType[i], length);
//			memcpy(data, AT_PORT_TYPE, length);
			data += length;

			length = strlen(at_iface.zonename);
			*data++ = length;
			memcpy(data, at_iface.zonename, length);
			data += length;

			length = data - nbp_send;

			if(sendto2(s, nbp_send, length, 0,(struct sockaddr *)&addr,
			          sizeof( struct sockaddr_at )) < 0 )

			{
				close_s2(s);
				return( -1 );
			}
			ppause(1);
		}
		tries1 = 0;
		while((rcv_len = recvfrom2(s, nbp_recv, sizeof(nbp_recv), 0, NULL, NULL)) > 0 ){
			data = nbp_recv;
			if(*data++ != NBP_DDPTYPE) continue;

			rnh = (struct nbphdr*) data;
			data += SZ_NBPHDR+SZ_NBPTUPLE;
			if ( rnh->nh_op != NBPOP_LKUPREPLY || rnh->nh_cnt != 1) {
				if(++tries1 > 3) { rcv_len = -1; break;}
				else continue;
			}

			olen = *data++;
			memcpy(tmp_obj,data,olen);
			data +=	olen;

			tlen = *data++;
			memcpy(tmp_type,data,tlen);
			data += tlen;

			zlen = *data++;
			memcpy(tmp_zone,data,zlen);
			data += zlen;

			if(rcv_len < (1+SZ_NBPHDR+SZ_NBPTUPLE+olen+tlen+zlen+3)) continue;

			for(j = 0 ; j < NUM_OF_PRN_PORT; j++) {
				if(nbp_used[j]) continue;
				if(nbp_find(j)) nbp_used[j] = 1;
			}
		}
	} while(rcv_len < 0 && --tries > 0);

	close_s2(s);
	return (0);
}

void nbp_input(cyg_addrword_t _data)
{

	uint8 nbp_data[NBP_INPUT_LEN];
	struct sockaddr_at	lsock, fsock;
	int	   s, fromlen, length,rcv_len;
	struct nbphdr	*nh;
	struct nbptuple	*nt;
	uint8 *data;

//	for(i= 0 ; i < NUM_OF_PRN_PORT; i++) nbp_socket[i] = GetATport();

    while (!ATD_INIT_OK)
        cyg_thread_yield();

	if((s = socket2( AF_APPLETALK, SOCK_DGRAM, 0 )) < 0) {
#ifdef PC_OUTPUT
		printf("\aNBP_INPUT SOCKET() ERROR !\a\n");
#endif PC_OUTPUT
		return;
	}

	lsock.sat_family = AF_APPLETALK;
	lsock.sat_addr   = at_iface.my;
	lsock.sat_port = NBP_SOCKET_NUM;

	if(bind2( s, (struct sockaddr *)&lsock,sizeof( struct sockaddr_at )) < 0 ) {	
		
		close_s2(s);
		return;
	}

	nh = (struct nbphdr*) (nbp_data+1);
	nt = (struct nbptuple *)(nbp_data+1+SZ_NBPHDR);

	for(;;) {
		fromlen = sizeof(fsock);	
		if((rcv_len = recvfrom2(s, nbp_data, sizeof(nbp_data), 0,
		              (struct sockaddr *)&fsock, &fromlen)) > 0 )
		{
			if(nbp_data[0] != NBP_DDPTYPE) continue;

			if( (nh->nh_op != NBPOP_LKUP && nh->nh_op != NBPOP_BRRQ)
			    || nh->nh_cnt != 1)
			{
				continue;
			}

			fsock.sat_addr.s_net = NGET16(&nt->nt_net);
			fsock.sat_addr.s_node = nt->nt_node;
			fsock.sat_port = nt->nt_port;

			//same as my node ??? (loopback)
			if(fsock.sat_addr.s_net  == at_iface.my.s_net &&
			   fsock.sat_addr.s_node == at_iface.my.s_node)
				continue;

			data = ((uint8 *) nt) + SZ_NBPTUPLE;

			olen = *data++;
			memcpy(tmp_obj,data,olen);
			data +=	olen;

			tlen = *data++;
			memcpy(tmp_type,data,tlen);
			data += tlen;

			zlen = *data++;
			memcpy(tmp_zone,data,zlen);
			data += zlen;

			if(rcv_len < (1+SZ_NBPHDR+SZ_NBPTUPLE+olen+tlen+zlen+3)) continue;

			if((nh->nh_cnt = nbp_set(&length,(uint8 *)nt)) > 0) {
				nh->nh_op = NBPOP_LKUPREPLY;
				length += (1+SZ_NBPHDR);
#ifdef PC_OUTPUT
				if(length > NBP_INPUT_LEN) {
					printf("\a\anbp_input() length to small !\n");
					for(;;);
				}
#endif PC_OUTPUT

				sendto2(s, nbp_data, length, 0,(struct sockaddr *)&fsock,
			          sizeof( struct sockaddr_at ) );		          
			}
		}
        sys_check_stack();
	}//for(;;);

}

uint16 nbp_set(int16 *len,uint8 *buf)
{
	uint16 cnt,i;
	struct nbptuple	*nt;
	uint8  *data;

	cnt = 0;
	data = buf;
	for(i = 0 ;	i < NUM_OF_PRN_PORT; i++) {
		if(nbp_used[i]) continue;
		if(nbp_find(i)) {
			nt = (struct nbptuple *)data;

			nt->nt_net = at_iface.my.s_net;
			nt->nt_node = at_iface.my.s_node;
			nt->nt_port = (pr[i].p_atp)->atph_saddr.sat_port;
			nt->nt_enum = i;
			data += SZ_NBPTUPLE;

			*data = ATPortNameLength;
			memcpy(data+1,GetATPortName(i),*data);
			data += *data + 1;

			*data = strlen(EEPROM_Data.ATPortType[i]);
			memcpy(data+1,EEPROM_Data.ATPortType[i],*data);
//			*data = sizeof(AT_PORT_TYPE)-1;
//			memcpy(data+1,AT_PORT_TYPE,*data);
			data += *data + 1;

			*data = strlen(at_iface.zonename);
			memcpy(data+1,at_iface.zonename,*data);
			data += *data + 1;

			cnt++;
		}
	}
	*len = data - buf;
	return cnt;
}


int16 nbp_find(int i)
{
	// If not in local zone, then forget it
	if(!( (tmp_zone[0] == nbpStar && zlen == 1) ||
	       (tmp_zone[0] == '\0') || /* zero length string is "*" */
	       ( strlen(at_iface.zonename)== zlen && memicmp(tmp_zone,at_iface.zonename,zlen) == 0)
	  ) )
	{
		return (0);
	}

	if(!nbpMatch(tmp_obj,olen,GetATPortName(i))) {
		return (0);
    }

	if(!nbpMatch(tmp_type,tlen,EEPROM_Data.ATPortType[i])) {
//	if(!nbpMatch(tmp_type,tlen,AT_PORT_TYPE)) {
		return (0);
    }
	return (1);
}

int16 nbpMatch(uint8 *pat,uint16 plen, uint8 *thing)
{
	uint8 *p;
	int16 pl, tl, hl;

	if ((pat[0] == nbpEquals || pat[0] == nbpWild) && plen == 1)
		return (1);

	tl = strlen((char *)thing);

	if( (p = (uint8 *)strnchr(pat,plen,nbpWild)) != NULL) {
		hl = p - pat;
		if(hl && memicmp(pat, thing, hl) != 0) return (0);
		pl = plen - 1;
		if (tl < pl) return (0);

		if (hl < pl && memicmp(p+1,thing+(tl-(pl-hl)),pl-hl) != 0)
			return (0);
		return (1);
	}

	if( (tl != plen) || memicmp(pat,thing,plen) != 0) return (0);

	return (1);
}

uint8 * strnchr(uint8 *s,int16 len,uint8 c)
{
	while(len--) {
		if(*s == c) return s;
		s++;
	}
	return NULL;
}

void SetATPortName(void)
{
//	ATPortNameLength = strlen(Hostname);
//	memcpy(ATPortName, Hostname, ATPortNameLength);
	ATPortNameLength = strlen(EEPROM_Data.ATPortName);
	memcpy(ATPortName, EEPROM_Data.ATPortName, ATPortNameLength);
#if !defined(O_AXIS)
	if(ATPortNameLength) ATPortName[ATPortNameLength++] = '-';
	Node2Num(ATPortName+ATPortNameLength,3);
	ATPortNameLength += 6;
	ATPortName[ATPortNameLength++] = '-';
	ATPortName[ATPortNameLength++] = '1';
#endif	// !defined(O_AXIS)
	ATPortName[ATPortNameLength]   = '\0';
}

uint8 *GetATPortName(int i)
{
#if !defined(O_AXIS)
	ATPortName[ATPortNameLength-1] = i + '1';
#endif	// !defined(O_AXIS)
	return ATPortName;
}

