#include <stdlib.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "ipx.h"
#include "ntps.h"
#include "ntudp.h"
#include "nps.h"


extern int urandom(unsigned int n);

extern void UtilGetListViewData(LIST_VIEW *BoxInfo);
extern void UtilSetListViewData(LIST_VIEW *BoxInfo);
extern void UtilRemoveBusyStatus(uint8 *PrintStatus);
extern void UtilSetListViewData_Adv(LIST_VIEW_EXT *BoxInfo_E);

BYTE gUdpBroadCast;	//8/3/98 Simon

uint16 	t_tran_byte_udp = 0;
BYTE 	Previous_BNo_UDP = 0;
int		NTStartPrintUDP_Finish_Flag = 0;					//os eCos............Schedule Flag
int		util_socket;													//os eCos............

static Utility_Rec NTUDPUtilData;
static void NTDoCommandUDP (struct sockaddr_in *fromsock);
static void UtilConfigBoxUDP (struct sockaddr_in *fromsock);						// (81)
static void UtilViewBoxUDP (BYTE BroadCastMode, struct sockaddr_in *fromsock);		// (84)
static void UtilPrintStatusUDP (struct sockaddr_in *fromsock);						// (88)
static void UtilSendDeviceInfoUDP (BYTE PrinterID,struct sockaddr_in *fromsock);	// (90)

void my_send_view_box( void *dataptr, int size, struct sockaddr_in *fromsock); //615wu
//for PACK_DATA_EXT
BYTE rec_buffer_udp[sizeof(LIST_VIEW) + sizeof(LIST_VIEW_EXT) -1];
BYTE *recbuffer_udp = rec_buffer_udp;

void NTUtilityUDP(cyg_addrword_t data)
{
	struct sockaddr_in util_sockaddr;
	struct sockaddr_in fromsock;
	int fromlen;
	BYTE Version;

	util_sockaddr.sin_family = AF_INET;
	util_sockaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	util_sockaddr.sin_port = htons (IPPORT_NTPS_UTIL);
	util_sockaddr.sin_len = sizeof(util_sockaddr);

	if((util_socket = socket( AF_INET, SOCK_DGRAM, 0 )) < 0) {
		return;
	}
	
	if(bind(util_socket, (struct sockaddr *)&util_sockaddr,sizeof(util_sockaddr)) < 0) {
		return;
	}

	for (;;) {
		fromlen = sizeof(fromsock);
		if(recvfrom(util_socket, &NTUDPUtilData, sizeof(NTUDPUtilData), 0,
		      (struct sockaddr *)&fromsock, &fromlen) < 1) 
		      		continue;
		if(memcmp(NTUDPUtilData.Nt2.Mark,NT_CHECK_MARK2,NT_CHECK_MARK2_LEN) == 0) {
			Version = NT_VERSION_4;
		} else {
			if(memcmp(NTUDPUtilData.Nt1.Mark,NT_CHECK_MARK1,NT_CHECK_MARK1_LEN) == 0) {
				Version = NT_VERSION_2;
			} else
				continue;
		}
		if(Version == NT_VERSION_4) NTDoCommandUDP(&fromsock);
	}
}

void NTDoCommandUDP (struct sockaddr_in *fromsock)
{
	switch(NTUDPUtilData.Nt2.Cmd[0]) {
		case CMD01: // Config Box
//code1 don't save data.
//			UtilConfigBoxUDP(fromsock);
			break;
		case CMD03:	// ReBoot
			if(!memcmp(MyPhysNodeAddress,NTUDPUtilData.Nt2.Data,6))
				Reset(); //os
			break;
		case CMD04: //Send List View Box
			UtilViewBoxUDP(NTUDPUtilData.Nt2.Cmd[1], fromsock);
			break;
		case CMD08: // Request Printer's Status
			UtilPrintStatusUDP(fromsock);
			break;
		case CMD10: //Send Device Info 4/18/2000
			UtilSendDeviceInfoUDP(NTUDPUtilData.Nt2.Data[0],fromsock);
			break;
		default: // If the CMD is UnKnown , Give Up the Packet!
			break;
	} //switch (CMD) {
}

//(81) Write config data into EEPROM.
void UtilConfigBoxUDP (struct sockaddr_in *fromsock)
{
	BYTE Result = 0x00;

	//check this data for myself or not (broadcast) !
	if(memcmp(MyPhysNodeAddress,((LIST_VIEW *)NTUDPUtilData.Nt2.Data)->NodeID,6) != 0) return;

	UtilGetListViewData((LIST_VIEW *)NTUDPUtilData.Nt2.Data);


	if(WriteToEEPROM(&EEPROM_Data) != 0)  Result = 0xff;

	// Setup Utility Protocol
	NTUDPUtilData.Nt2.Cmd[0] = 0x81;
	NTUDPUtilData.Nt2.Cmd[1] = Result;

	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
		(struct sockaddr *)fromsock,sizeof(struct sockaddr));
}

extern struct netif *Lanface; 
extern err_t SendPacket(struct netif *netif, struct pbuf *p);
extern err_t ethernetif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
//(84) Send Box data for utility view & config it.
void UtilViewBoxUDP (BYTE BroadCastMode, struct sockaddr_in *fromsock)
{
	char *p;
	int i;
	
	NTUDPUtilData.Nt2.Cmd[0]=0x84;
	NTUDPUtilData.Nt2.Cmd[1]=0xFF;	//Waiting for D/L

	memset(NTUDPUtilData.Nt2.Data, 0, 1024);
	
	UtilSetListViewData((LIST_VIEW *)NTUDPUtilData.Nt2.Data);
	
	if( BroadCastMode == 0 )
	{	
		my_send_view_box( &NTUDPUtilData, sizeof(NEW_NT_Utility_Rec),(struct sockaddr *)fromsock);
	}
	else{
		sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
	}

}

//(88) Send Print HW Status.
void UtilPrintStatusUDP (struct sockaddr_in *fromsock)
{
	BYTE PortNumber;

	NTUDPUtilData.Nt2.Cmd[0] = 0x88;
	NTUDPUtilData.Nt2.Cmd[1] = 0xFF;  //Download mode

	//print status
	NTUDPUtilData.Nt2.Data[0] = 0;


	UtilRemoveBusyStatus(&NTUDPUtilData.Nt2.Data[0]);

//	NTUDPUtilData.Nt2.Data[1] = NovellConnectFlag;
	NTUDPUtilData.Nt2.Data[1] = 0;

	//Using Status
	for(PortNumber = 0; PortNumber < NUM_OF_PRN_PORT; PortNumber++) {
		NTUDPUtilData.Nt2.Data[PortNumber+2] = 0;
	}
	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
}

//-------------------------------------------------------------------
//(90) Send Device Info. 4/18/2000 added
void UtilSendDeviceInfoUDP (BYTE PrinterID,struct sockaddr_in *fromsock)
{
	BYTE *p;
	WORD len;

	NTUDPUtilData.Nt2.Cmd[0] = 0x90;
	NTUDPUtilData.Nt2.Cmd[1] = 0x00;

	//Printer Status
	NTUDPUtilData.Nt2.Data[0] = PrinterID;
	memset(&NTUDPUtilData.Nt2.Data[1],'\0',10);	
	
	sendto(util_socket,&NTUDPUtilData,sizeof(NEW_NT_Utility_Rec), 0, 
			(struct sockaddr *)fromsock,sizeof(struct sockaddr));
}

void my_send_view_box( void *dataptr, int size, struct sockaddr_in *fromsock)
{
	struct pbuf *p=NULL,*q=NULL;
	struct ip_addr *ipaddr=NULL;
	struct netif *temp_netif=NULL;
	struct udp_hdr *udphdr=NULL;
	int i=0;
	
	// Creat a temp IP:1.2.3.4
	ipaddr = (struct ip_addr *)malloc(sizeof(struct ip_addr));
	if( ipaddr == NULL )
		goto end;
	ipaddr->addr = htonl(0x01020304);
	
	// alloc a new pbuf
	p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_REF);
	if( p == NULL)
		goto end;

	p->payload = dataptr;
	p->len = p->tot_len = size;

    /* allocate udp header in a seperate new pbuf */
    q = pbuf_alloc(PBUF_IP, UDP_HLEN, PBUF_RAM);
    /* new header pbuf could not be allocated? */
    if (q == NULL) {
    	goto end;
     }
    /* chain header q in front of given pbuf p */
    pbuf_chain(q, p);
	
	// fill udp header 	
	udphdr = q->payload;
	udphdr->src = htons(0x5050);
	udphdr->dest = (fromsock->sin_port);
	/* in UDP, 0 checksum means 'no checksum' */
	udphdr->chksum = 0x0000; 
	udphdr->len = htons(q->tot_len);
	  		
	udphdr->chksum = inet_chksum_pseudo(q, ipaddr, &(fromsock->sin_addr),
    	 IP_PROTO_UDP, q->tot_len);
	
	if (udphdr->chksum == 0x0000) udphdr->chksum = 0xffff;
	
	// Creat a temp netif for IP:1.2.3.4 MASK:0.0.0.0		
	temp_netif = (struct netif *)malloc(sizeof(struct netif));
	if( temp_netif == NULL )
		goto end;			
	memset( temp_netif, 0, sizeof(struct netif));
	temp_netif->ip_addr.addr = 0x01020304;
	temp_netif->netmask.addr = 0x0;
	temp_netif->output = ethernetif_output;
	temp_netif->linkoutput = SendPacket;
	
	/* set MAC hardware address length */
	temp_netif->hwaddr_len = 6;
	
	/* set MAC hardware address */
	for ( i = 0; i < 6; ++i)
    {
        temp_netif->hwaddr[i] = MyPhysNodeAddress[i];
    }
	
	/* maximum transfer unit */
	temp_netif->mtu = 1500;	
	
	// send to ip layer	
	ip_output_if (q, ipaddr, &(fromsock->sin_addr), 255, 0, 17, temp_netif);    
		
end:		
	pbuf_free(q);
	pbuf_free(p);
	free(temp_netif);
	free(ipaddr);
	

}