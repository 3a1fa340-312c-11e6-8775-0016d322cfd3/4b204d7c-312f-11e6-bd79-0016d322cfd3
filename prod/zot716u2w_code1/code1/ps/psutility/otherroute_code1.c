#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>

#include "pstarget.h"
#include "psglobal.h"
#include "ipx.h"
#include "netbeui.h"
#include "ewb_packet.h"

extern uint16 NGET16( uint8 *pSrc );
extern WORD NT_NETBEUItoIPXInput(WORD InputFrameType, BYTE *SrcAddress,NETBEUIHeader *p,int16 DataLen);

WORD MyIntNO;
extern WORD IPXInput(WORD InputFrameType, BYTE *SrcAddress,IPXHeader *p,int16 DataLen);
//Recognize Other packet for eCos
int ProcessOtherPckt(unsigned char *pFrame, unsigned int lenFrame)
{
	eth_hdr *eth;
	unsigned short etherType;
	uint16 FrameType;
	
	eth = (eth_hdr *)pFrame;
	//check Ethernet type
    etherType = ntohs(eth->type);
	
	if(etherType == IPXENII)
		FrameType = IPXENII;
	else 
		FrameType = NGET16(&pFrame[14]);
		
	switch(FrameType) {
		case IPX8023:
		case IPXENII:
			IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[14],(lenFrame-14));
			break;
		case IPX8022:
			IPXInput(FrameType,eth->src,(IPXHeader *)&pFrame[17],(lenFrame-17));
			break;
		case IPXSNAP:
			*((char*)&FrameType+1) = pFrame[20]; //ethernet type
			*(char*)&FrameType     = pFrame[21];
				
			if(FrameType == IPXENII) {
				if (lenFrame > 22)
				    IPXInput(IPXSNAP,eth->src,(IPXHeader *)&pFrame[22],(lenFrame-22));
			}
			break;
#ifdef NETBEUI
		case IPXBEUI:
			if(!memcmp(eth->dest,NETBEUI_MULTICAST,6)){
				if (lenFrame > 17)
				    NT_NETBEUItoIPXInput(FrameType,eth->src,(NETBEUIHeader *)&pFrame[17],(lenFrame-17));
			}
			else return -1;
		break;
#endif
		default:
			return -1;
	}
	return 0;
}
