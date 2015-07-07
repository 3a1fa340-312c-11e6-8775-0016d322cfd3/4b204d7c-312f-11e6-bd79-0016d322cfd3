#ifndef _IPX_H
#define _IPX_H

struct e_ipxaddress
 {	// = IPXAddress =
	BYTE	network[4]; 					/* High-Low */
	BYTE	node[6];						/* high-low */
	BYTE	socket[2];						/* high-low */
}__attribute__ ((aligned(1), packed));

typedef struct e_ipxaddress IPXAddress;

struct e_ipxheader
{	// = IPXPacket =
	WORD		checkSum;					/* high-low */
	WORD		length; 					/* high-low */
	BYTE		transportControl;
	BYTE		packetType;
	IPXAddress	destination;
	IPXAddress	source;
}__attribute__ ((aligned(1), packed));

typedef struct e_ipxheader IPXHeader;

typedef struct ECBFragment {
	void		*address;
	WORD		size;						/* low-high */
	WORD		recv_size;					//receive size //6/27/2000
} ECBFragment;

typedef struct ECB {
	void		*linkAddress;
	void		(*ESRAddress)(void);
	BYTE		inUseFlag;
	BYTE		completionCode;
	WORD		socketNumber;				// high-low
	WORD		IPXFrameType;				//3/16/98 Simon
//	BYTE		IPXWorkspace[4];			/* N/A */ //remarked by Simon 3/16/98
//	BYTE		driverWorkspace[12];		/* N/A */ //remarked by Simon 3/16/98

	BYTE		immediateAddress[6];		// high-low
	WORD		fragmentCount;				// low-high
	ECBFragment fragmentDescriptor[2];
#ifdef NETBEUI
	BYTE		IPXSrcNETBEUIName[16];		//NetBEUI source Name, NetBEUI over IPX, 5/4/98
#endif
}ECB;

typedef struct SocketNumberTable {
	WORD  socketNumber; 					/* high-low */
	BYTE  socketType;
	void  *ECBLinkList;
} SocketNumberTable;

struct e_etherhead
{
	BYTE dest[6];							/* where the packet is going */
	BYTE   me[6];							/* who am i to send this packet */
	BYTE lentyp[2];							/* Length or Ethernet packet type  */
}__attribute__ ((aligned(1), packed));

typedef struct e_etherhead ETHER_HEAD;

typedef struct ipx8023 {
	ETHER_HEAD	   Etherh;
} IPXPKT8023;

typedef struct ipx8022 {
	ETHER_HEAD Etherh;
	BYTE	  NOVELL_DSAP;
	BYTE	  NOVELL_SSAP;
	BYTE	  CONTROL_8022_UI;
} IPXPKT8022;

typedef struct ipxsnap {
	ETHER_HEAD Etherh;
	BYTE	  NOVELL_DSAP;
	BYTE	  NOVELL_SSAP;
	BYTE	  CONTROL_SNAP_UI;
	BYTE	  ORGANIZATION_CODE[3];
	WORD	  ETHERNETTYPE;
} IPXPKTSNAP;

typedef struct ipxenii {
	ETHER_HEAD Etherh;
} IPXPKTENII;

typedef union PKT_BUFFER {
	ETHER_HEAD Etherh;
	IPXPKT8023 IPX_8023;
	IPXPKT8022 IPX_8022;
	IPXPKTSNAP IPX_SNAP;
	IPXPKTENII IPX_ENII;
	BYTE  PKT_BUFFER[1514];    //packet send buffer
} PKTSENDBUFFER;

#define  SOCKET_TABLE_LENGTH				15		 // Charles 2001/09/11

#define  SHORT_LIVED						0x00
#define  LONG_LIVED 						0xFF
#define  SUCCESSFUL 						0x00

#define  NETBEUI_8022_DSAP					0xF0	 //5/5/98
#define  NETBEUI_8022_SSAP					0xF0	 //5/5/98

#define  NOVELL_8022_DSAP					0xE0
#define  NOVELL_8022_SSAP					0xE0
#define  NOVELL_ETHN_TYPE					0x3781
#define  NOVELL_SNAP_DSAP					0xAA
#define  NOVELL_SNAP_SSAP					0xAA
#define  CONTROL_UI 						0x03
#define  VENDERID1							0
#define  VENDERID2							0
#define  VENDERID3							0


#define  NO_SUCH_SOCKET 					0xFE
#define  NO_LISTEN_ECB						0xFF
#define  MALFORM_PACKET 					0xFD
#define  SOCKET_NOT_OPEN					0xFF
#define  SOCKET_NOT_BUFFER					0xF0
#define  SOCKET_TABLE_FULL					0xFE
#define  SOCKET_ALREADY_OPEN				0xFF
#define  SOCKET_SEND_PACKET_ERROR			0xFC

#define  ANY								0x0000

//#define  MAX_IPX_PACKET_SIZE				1054
#define  MAX_IPX_PACKET_SIZE				1470	 //11/25/99 for NDS
#define  MIN_IPX_PACKET_SIZE				30


#define  OKAY								0x00
#define  FREEZE 							0x00
#define  SENDING							0xFF
#define  RECEIVING							0xFE
#define  PROCESSING 						0xFA

#define  MaxFrameType						4

//protocol
#define  IPX8023 0xFFFF
#define  IPX8022 0xE0E0
#define  IPXSNAP 0xAAAA
#define  IPXENII 0x8137

#define  IPXBEUI 0xF0F0  //5/5/98

//IPX Global variable
extern BYTE MyPhysNodeAddress[];
extern BYTE MyNetworkAddress[];
extern WORD MyIntNO;

//IPX current frame type for NOVELL only (NPS3.C)
extern WORD frameType;

//IPX API
WORD GetIPXFrameType(WORD i); //0:IPX8022, 1:IPX8023, 2:IPXENII, 3:IPXSNAP

int  IPXInitialize(void);
void IPXGetInternetworkAddress(BYTE *Address);
WORD IPXOpenSocket(BYTE *socketNumber,BYTE socketType);
void IPXSendPacket(ECB *sendECB);
void IPXListenForPacket(ECB *listenECB);
int  IPXCancelEvent(ECB *listenECB);
WORD IPXInput(WORD InputFrameType, BYTE *SrcAddress,IPXHeader *p,int16 DataLen);

//IPX Upgrade
#define UpgradeSocket            	0x1840		// 4018

#define UPGRADE_BLOCK_SIZE       	1024
#define UPGRADE_MAX_RETRY_TIMES		4			//4 times
#define UPGRADE_TIME_OUT_SEC	 	3000L		//3 sec

//Netware Upgrade-Flash data structure
typedef struct
{
	BYTE Mark[3];    // User Define Protocol :"NET"
	BYTE Type;       //50, 60 or 70
	BYTE Cmd[2];	 //command
	WORD BlockNumber;
	BYTE Data[UPGRADE_BLOCK_SIZE];
} IPX_Upgrade_Rec;

#endif _IPX_H
