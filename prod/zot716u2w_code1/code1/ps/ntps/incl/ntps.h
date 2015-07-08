#ifndef _NTPS_H
#define _NTPS_H

#define NTDataSocket               	0x6060

#define NT_TIME_OUT         	   	3

//Data Transfer Mode
#define NORMAL_MODE        			0
#define BURST_MODE         			1

#define BLOCK_COUNT  				1
#define NT_MAX_PORT_NAME_LENGTH  	32  //maximun length must = 255,
                                     	//but program only identify 32 bytes
#define NT_MAX_RECV_LEN    			1450

#define NT_CHECK_MARK1 "ZO"	 //old protocol
#define NT_CHECK_MARK2 "NET" //new protocol

#define NT_CHECK_MARK1_LEN (sizeof(NT_CHECK_MARK1)-1)
#define NT_CHECK_MARK2_LEN (sizeof(NT_CHECK_MARK2)-1)

#define NT_END_PRINT  "End Of Printer Job"

typedef struct {
BYTE        BlockNumber;
BYTE        IsFirst;
BYTE       *BlockData;
WORD        IPXFrameType;
WORD        BlockSize;
BYTE        PortName[NT_MAX_PORT_NAME_LENGTH+1]; //ZeroString
BYTE        ImmediateAddress[6];                 // high-low
BYTE        HaveRecvPacket;                      //already receive one packet !
#if defined(IPX)
IPXAddress  Destination;
#endif
#if defined(NTUDP) || defined(NETBEUI)
union      {
#ifdef NTUDP
	struct sockaddr_in	Rsocket;
#endif
#ifdef NETBEUI
	BYTE        SNetBEUIName[16];					 //Netbeui source name
#endif
} s;
#endif

BYTE        Version; //for Print Monitor new and old version 9/6/99

uint8		MaxRecvPackets; //for Print Monitor new and old version 6/20/01

} NT_PORT_BLOCK;

#define RemoteSocket   s.Rsocket
#define SrcNETBEUIName s.SNetBEUIName

typedef struct
{
	BYTE PrinterID;	   //(1 - N)
	BYTE Status;	   //00 - OK , 01 - Busy
	BYTE Mode;         //00
	WORD MaxDataLen;   //Maximum data length
	BYTE MAXPacket;    //Maximum Packet for receive
}PACK NTStartRespData;

typedef struct
{
	BYTE Mark[3];	  //NT Mark "NET"
	BYTE Type;        //60
	BYTE Cmd[2];      //Command
	BYTE PrinterID;	  //(1 - N)
	BYTE BlockNumber; //0x00 - 0xFF , sequence number
	BYTE Mode;		  //0:Normal mode 1:Burst mode
	BYTE BlockCount;  //how many block want to receive !
}PACK NEW_NTQueryAckData;

typedef struct
{
	BYTE Mark[2];	  //NT Mark "ZO"
	BYTE Cmd[2];  //NT connection command
	BYTE PrinterID;	  //(1 - N)
	BYTE BlockNumber; //0x00 - 0xFF , sequence number
	BYTE Mode;		  //0:Normal mode 1:Burst mode
	BYTE BlockCount;  //how many block want to receive !
}PACK OLD_NTQueryAckData;

typedef union
{
	OLD_NTQueryAckData Nt1;
	NEW_NTQueryAckData Nt2;
}PACK NTQueryAckData;

typedef struct
{
	BYTE Cmd[2];                //Command
	BYTE PrinterID;	            //(1 - N)
	BYTE BlockNumber;           //0x00 - 0xFF , sequence number
	BYTE Mode;		            //0:Normal mode 1:Burst mode
	WORD DataLength;            //Data length
	BYTE Data[NT_MAX_RECV_LEN];	//Receive Data
}PACK NTReqData;

typedef struct
{
	BYTE Mark[3];	            //NT Mark "NET"
	BYTE Type;                  //60
	BYTE Cmd[2];                //Command
	BYTE PrinterID;	            //(1 - N)
	BYTE BlockNumber;           //0x00 - 0xFF , sequence number
	BYTE Mode;		            //0:Normal mode 1:Burst mode
	WORD DataLength;            //Data length
	BYTE Data[NT_MAX_RECV_LEN];	//Receive Data
}PACK NEW_NTRequestData;

typedef struct
{
	BYTE Mark[2];	  //NT Mark "ZO"
	BYTE Cmd[2];  //NT connection command
	BYTE PrinterID;	  //(1 - N)
	BYTE BlockNumber; //0x00 - 0xFF , sequence number
	BYTE Mode;		  //0:Normal mode 1:Burst mode
	WORD DataLength;  //Data length
	BYTE Data[NT_MAX_RECV_LEN];	//Data (Max = 1024byte)
}PACK OLD_NTRequestData;

typedef union
{
	OLD_NTRequestData Nt1;
	NEW_NTRequestData Nt2;
}PACK NTRequestData;



#if (LENGTH_OF_FS_NAMES > 254)
#define LIST_VIEW_FS_LEN  254
#else
#define LIST_VIEW_FS_LEN  LENGTH_OF_FS_NAMES
#endif

typedef	struct _UtilFlag {
		BYTE Downloadable:1;
		BYTE DisablePrintSpeed:1;
		BYTE SupportDeviceInfo:1;
		BYTE Support_DiagnosticMode:1;
		BYTE NoUsed:4;
}PACK _UtilFlag ;

typedef struct _SupportFunction{
		BYTE NetWareBindery:1;	  //bit 00: NetWare Bindery printing
		BYTE LPD:1;               //bit 01: LPD/LPR printing
		BYTE PrintMonitor:1;      //bit 02: Print Monitor (peer to peer printing)
		BYTE AppleTalk:1;         //bit 03: AppleTalk printing
		BYTE NetWareNDS:1;        //bit 04: NetWare NDS printing
		BYTE IPP:1;               //bit 05: IPP printing
		BYTE SMB:1;               //bit 06: SMB printing
		BYTE Tftp:1;              //bit 07: Tftp upgrade
		BYTE WEB:1;               //bit 08: Web browser config
		BYTE Telnet:1;            //bit 09: Telnet config
		BYTE SNMP:1;              //bit 10: SNMP
		BYTE JetAdmin:1;          //bit 11: JetAdmin/Web JetAdmin compatible
		BYTE Bidirectional:1;     //bit 12: Bi-directional (Device Info)
		BYTE DisAdvancePage:1;    //bit 13: Enable Advance setting
		BYTE Wireless_sta:1;      //bit 14: Wireless Station support
		BYTE Wireless_ap:1;       //bit 15: Wireless AP support
		BYTE Wireless_11g:1;	  //bit 16: Wireless support 11g
		BYTE Wireless_1x:1;	      //bit 17: Wireless support 802.1x
		BYTE LPRQRename:1;		  //bit 18: LPR Queue Rename
		BYTE Wireless_wpa_psk:1;  //bit 19: Wireless support WPA_PSK
		BYTE Data_Exten:1;		  //bit 20: pack data support external
		BYTE Wireless_wpa2_psk:1; //bit 21: Wireless support WPA2_PSK
		BYTE WL_Transmit_Mode:1;  //bit 22: Wireless support Transmit_Mode
		BYTE Security_Config:1;   //bit 23: Security config Box CFG Seting
		BYTE NoUsed3;
}PACK _SupportFunction;

typedef struct _Port_Model{
	BYTE Model:6;            // Model
	BYTE NumOfPort:2;        // Number Of Port
}PACK _Port_Model;

// Receive BOX (PS) Data Struct for UTILITY packet
typedef struct LIST_VIEW{
//Print Monitor ( 32 ) ------------------------------------------------------
	BYTE NodeID[6];          // Physical Node Address
	BYTE Version[2];         // Software version+ Hardware version
	BYTE BoxName[19];        // Box Name (zero string)
	//Little Enden
	
	_Port_Model Port_Model;
	
	BYTE IP[4];		         // IP address
//System ( 33 ) --------------------------------------------------------------
	BYTE SystemLen;	   // (3:+PrinterSpeed, 32)

	_UtilFlag  UtilFlag;
	BYTE ProtocolMode;
	BYTE PrinterSpeed; // Printer Speed
	BYTE SetupPassword[9];  //Setup Password
	BYTE VersionString[16]; //Version String
	
	_SupportFunction  SupportFunction;

//Netware ( 62 + LIST_VIEW_FS_LEN ) -----------------------------------------
	BYTE NetwareLen;   // (1: Mode+PollingTime, 10: Mode+PollingTime+Password)
	BYTE PollingTime;  // Netware Polling Time
	BYTE NovellPassword[9];  // NDS & Bindery Encrypted password (Zero String)
	BYTE NumOfMaxFileServer; // Number of Maximum File Server
	BYTE MaxFileServerLen;   // Maximum File Server Length
	BYTE FileServerName[LIST_VIEW_FS_LEN];
	BYTE MaxPrintServerLen;   // Maximum Length of Print Server
	BYTE PrintServerName[48]; // Print Server Name (Zero String)

//TCP/IP ( 10 ) -------------------------------------------------------------
	BYTE TCPIPLen;	        // (8: IP only, 9: IP + DHCP)
	BYTE SubnetMask[4];     // Subnet Mask
	BYTE GatewayAddress[4]; // Gateway Address
	BYTE DHCPEnable;        // DHCP Enable|Disable

//SNMP ( 77 ) ---------------------------------------------------------------
	BYTE SnmpLen;	//(76)
	BYTE SnmpSysContact[SNMP_SYSCONTACT_LEN];   //(Zero String)
	BYTE SnmpSysLocation[SNMP_SYSLOCATION_LEN];	//(Zero String)
	BYTE SnmpCommunityAuthName[NO_OF_SNMP_COMMUNITY][SNMP_COMMUNITY_LEN];

	_SnmpAccessFlag SnmpAccessFlag;	//615wu //define in eeprom.h
	
	BYTE SnmpTrapTargetIP[NUM_OF_SNMP_TRAP_IP][4];

//AppleTalk ( 149 ) ----------------------------------------------------------
	BYTE ATalkLen;                       //(148)
	WORD ATCurNet;                       // Current Net (02)
	BYTE ATCurNode;                      // Current Node(01)
	BYTE ATCurZoneName[ATALK_ZONE_LEN];	 // Current Zone Name (zero string)
	BYTE ATZoneName[ATALK_ZONE_LEN];     // Apple ZONE (zero string)
	BYTE ATPortName[ATALK_PORT_NAME];    // Port Object Name (zero string)
	BYTE ATPortType[3][ATALK_TYPE_LEN];  // Port Type Name   (zero string)
	BYTE ATDataFormat[3];				 // Port Data Format

//NDS	(286) ----------------------------------------------------------------
	BYTE NDSLen;						 //(33)
	BYTE NDSTreeName[NDS_TREE_LEN];      // NDS Tree Name
	BYTE NDSContextLen;					 // NDS Context Length (1)
	BYTE NDSContext[NDS_CONTEXT_LEN];    // NDS Context (251)

//DEVICE (4) -----------------------------------------------------------------
	BYTE DEVICELen;
	BYTE Bidirectional[3];  //(3) Bidirectional AUTO/Disable

//Wireless(195) ----------------------------------------------------------------
//for Net
//Wireless(87) ----------------------------------------------------------------
	//	General
	BYTE WirelessLen;
	BYTE WLMode;						 //0x01: Infrastruct, 0x02:Ad-Hoc, 0x03: 802.11b Ad-Hoc	
	BYTE WLESSID[32];			 		 //(32)
	BYTE WLZone;						 //A1: ch1  -  ch11
										 //A2: ch1  -  ch13
										 //A3: ch10 -  ch13		
										 //A4: ch1  -  ch14
										 //A5: ch10 -  ch11
	BYTE WLChannel;
	BYTE WLRate;						 // 0x01: 1,2M; 0x02: 5.5M; 0x03: 11M; 0x04: 1,2,5.5,11M 
	//  Advance 	
	WORD WLBeaconinterval;
	BYTE WLShortPreamble;
	WORD WLFragmentation;
	BYTE WLAuthenticationType;
	
	//	Wep
	BYTE WLWEPType;					     //0: disable, 1: 64bit, 2: 128 bit
	BYTE WLWEPKeySel;
	BYTE WLWEPKey1[6];
	BYTE WLWEPKey2[6];
	BYTE WLWEPKey3[6];
	BYTE WLWEPKey4[6];
	BYTE WLWEP128Key[15];
	BYTE WLWEPKeyType;					 //0: ASCII, 1:HEX
	
	BYTE OperationMode;                 // 0: Normal, 1: diagnostic(factory) mode 
	BYTE WLAPMode;                      // 0: AP mode disable, 1: AP mode enable 		
	                                
// Port Face (4) ------------------------------------------------------------
	BYTE PortFaceLen;
	BYTE PORTDESC;					// The right first two bits: Port 1
									// The right second two bits: Port 2
									// The right third two bits: Port 3
									// The left two bits: undefine 00 default
									// For each port: 00 default 01 Parallel 10 USB 11 undefine
	BYTE PORTTAIL;					// The right first two bits: Sequence of Port 1
									// The right second two bits: Sequence of Port 2
									// The right third two bits: Sequence of Port 3
	BYTE SPECIAL_OEM;				//0:default 1:    2:DLINK
									//3:P1U1U2	4:P1U2U3	5:Port1Port2Port3

//EndOfStruct ( 1 ) ---------------------------------------------------------
	BYTE EOS;
}PACK LIST_VIEW;

typedef struct LIST_VIEW_EXT
{
//Wireless(111) ----------------------------------------------------------------
	//	General
	BYTE WirelessExtLen;			//lenth==110
	
	BYTE WLExtRate; 		        // bit0:  6M,  bit1:  9M,  bit2: 12M,   bit3: 18M
	                                // bit4: 24M,  bit5: 36M,  bit6: 48M,   bit7: 54M 
	// EAP Setting   3 item 43 bytes....
	BYTE EAP_Type;					//0:NONE  1:MD5   2:CHAPV2
	BYTE EAP_Name[21];				//EAP Account Name
	BYTE EAP_Password[21];			//EAP password	

	// WPA_PSK_TKIP
	BYTE wpa_Pass[64];				//WPA_PSK password
	
	// Transmit Mode
	BYTE WLTransMode;				//0:AUTO, 1:B only, 2:G only
	
	// WPA TYPE
	BYTE WLWPATYPE;					//if (WLAuthenticationType == 4) 0: WPA-PSK(TKIP),  1: WPA-PSK(AES), 
									//if (WLAuthenticationType == 5) 0: WPA2-PSK(TKIP), 1: WPA2-PSK(AES),
		
//LPR	(40) ----------------------------------------------------------------
	BYTE LPRLen;						//(39)
	BYTE LPRQueueName[3][LPRQUEUENAME_LENGTH];	//LPR Queue name
	                               
// RSA Public Key (257) ------------------------------------
	BYTE rsa_public_key_len;
	BYTE rsa_public_key_value[256]; // Reserve to 2048 bit even if we currently choose 1024 bit 	
	                                
//EndOfStruct ( 1 ) ---------------------------------------------------------
	BYTE EOS;
}PACK LIST_VIEW_EXT;

typedef struct
{
	uint32 StartTime;
	uint16 HoldTime;
}NTHoldTimeData;

typedef struct      //Ron define 10/13/2003
{
	BYTE Mark[3];	            //NT Mark "NET"
	BYTE Type;                  //60	
	BYTE Cmd[2];                //Command
	BYTE PrinterID;	            //(1 - N)
	BYTE BlockNumber;           //0x00 - 0xFF , sequence number
	BYTE Mode;		            //0:Normal mode 1:Burst mode
	WORD DataLength;            //Data length
}PACK NTReqCmdHeader;

#endif  _NTPS_H
