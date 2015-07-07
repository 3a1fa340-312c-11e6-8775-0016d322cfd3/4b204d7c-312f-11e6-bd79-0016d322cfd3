#ifndef _NTUDP_H
#define _NTUDP_H

#define TCPUDP  0x1010	   //for ECB FrameType
#define NTUDP_RETRY_COUNT  4
#define NTUDP_BROADCAST_IP 0x01020304

// Receive BOX (PS) Data Struct for NETBEUI UTILITY packet
typedef struct
{
	BYTE  Version[2];     // BOX Version
	BYTE  Name[18];       // BOX (PS) Name
	DWORD IP;             // IP Address
	DWORD SubnetMask;     // Subnet Mask
	DWORD Gateway;        // Gateway
	BYTE  PSMode;         // Print Server Mode
} NTUDP_Config;

typedef struct
{
	BYTE  Name[18];   // BOX (PS) Name
	BYTE  node[6];    // BOX's Network Address
	BYTE  ver[2];     // *Not same as EEPROM version*
	WORD  Model;      // System Type  UNIX(01) or NW (02) , 03 UNIX+NW, 04 NT single 05 NT MULTI
	DWORD IP;         // BOX's IP Address
	DWORD SubnetMask; // Subnet Mask
	DWORD Gateway;    // Gateway
} NTUDP_Bname;


#endif  _NTUDP_H
