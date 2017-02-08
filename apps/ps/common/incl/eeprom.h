#ifndef _EEPROM_H
#define _EEPROM_H

#define LENGTH_OF_BOX_NAME		18	
//Andy:20161229 modify lineup password len to 14 +++
#ifdef O_LINEUP
#define SETUP_PASSWD_LEN        14 //20161102:add for LINEUP password len +++
#define SETUP_USERNAME_LEN		8 //20161102:add for LINEUP Username +++
#else
#define SETUP_PASSWD_LEN		8
#endif
//Andy:20161229 modify lineup password len to 14 ---
//For NDS
#define NDS_TREE_LEN			33
#define NDS_CONTEXT_LEN 		251
#define NOVELL_PASSWORD_LEN 	9	 //include Null character
#define NDS_TOTAL_LEN			(NDS_TREE_LEN+NDS_CONTEXT_LEN+NOVELL_PASSWORD_LEN)

//for Apple Talk
#define ATALK_ZONE_LEN			33
#define ATALK_PORT_NAME 		13
#define ATALK_NAME_LEN			(ATALK_PORT_NAME+1+6+1+1) //(DEVICE_NAME)+(_)+(XXXXXX)+(-)+(1)
#define ATALK_TYPE_LEN			21
#define APPLETALK_EEPROM_LEN	(3+ATALK_ZONE_LEN+ATALK_PORT_NAME) //47

// For SNMPD
#define SNMP_SYSCONTACT_LEN 	16	// string (include zero char) 11/24/98	for SNMPD
#define SNMP_SYSLOCATION_LEN	25	// string (include zero char) 11/24/98	for SNMPD
#define NO_OF_SNMP_COMMUNITY	2
#define SNMP_COMMUNITY_LEN		13	 // string (include zero char)
#define NUM_OF_SNMP_TRAP_IP 	2

// Netware File Server Name Length
#define LENGTH_OF_FS_NAMES		254

#define LENGTH_OF_DEVICE_ID		1024

#define SERVICENAME_LENGTH      13   //SMB Printer Service Name Length ....Ron Add 3/13/2002

#define LPRQUEUENAME_LENGTH		13

#define NCBNAMSZ				16
#define RANDID_SIGNATURE		0x0FF	// RANDOM NODE ID MARK ... Ron 4/3/2003

typedef struct _SnmpAccessFlag {
		BYTE SnmpTrapEnable : 1;	//Enable(1)/Disable(0) Snmp Trap
		BYTE SnmpAuthenTrap : 2;	//Enable(1)/Disable(2) SnmpAuthenTrap
		BYTE SnmpComm0AccessMode : 2; //Snmp Community1 Access Right (01: Read Allow, 10: Write Allow)
		BYTE SnmpComm1AccessMode : 2; //Snmp Community2 Access Right (01: Read Allow, 10: Write Allow)
		BYTE SnmpNoUsed : 1;		//no used
}PACK _SnmpAccessFlag ;

typedef struct EEPROM {        
// Total 5 = 0 + 5
// Start 5 = 4 + 1
	BYTE ZOT_Mark[4];		   //ZOT mark "ZOT"
	BYTE EEPROM_Version;	   //for EEPROM upgrade

// Total	42 = 5 + 37
// System	37 = 6 + 2 + 1 + 18 + 8 + 1 + 1
	BYTE EthernetID[6];			//Physical Node Address.
	BYTE Version[2];		 	//Hardware version = Version[0] & 0xF
								//Software version = BYTE[1] * 0x10 + (BYTE[0] >> 4)
	BYTE Model; 				//00:N6101EP    01:N6102EP		02:N6200EP
								//03:N6300EP    04:N6301EP		06:N6300II
								//07:737,6300+  08:7227			09:PCI520
								//10:7117		11:7339,7339+	12:7119
								//13:5225+      14:7119A		15:7339A
								//16:7339AW     17:7119AU		18:535U
								//19:535U-CD    20:535WPI		21:535WUI 
								//22:525PR      23:535WPID		24:635U2P
								//25:9312		26:635U2PW		27:535WU+
								//28:			29:615P			30:7119A+
								//31:615U2		32:615WU		33:615U
								//34:7339A+		35:616U2		36:636U2P
								//37:MFPU1		38:MFPU2		39:636U2P
								//40:636U2PW	41:8636U2		42:P101S
								//43:DWP1000	44:DWM1000		45:iN-210
								//46:PU201S
                                //47:716U2
								//50:716U2W
                                //55:DWP2020
                                //56(0x38):716U2  (MT7688)
                                //57(0x39):716U2W (MT7688)
                                //58(0x3A):DWP2021(MT7688)

	BYTE BoxName[LENGTH_OF_BOX_NAME]; //Device Name. (none zero string)
	BYTE Password[SETUP_PASSWD_LEN];	//Configuration Device Password.
	BYTE NumOfPort; 		   //How many port are this printer support ? //10/29/98
	BYTE PrintServerMode;				//0x01:Netware, 0x02:Unix, 0x04:Windows, 0x08:Apple
										//0x10:NDS, 0x20:IPP , 0x40:SMB , 0x80:DHCP

// Total			100 = 42 + 58
// NetWare (Both)	58 = 48 + 1 + 9
	BYTE PrintServerName[48];  //Print Server Name on File Server.
	BYTE PollingTime;		   //Polling Print Server Queue Time.
	BYTE NovellPassword[NOVELL_PASSWORD_LEN];//NDS & Bindery Password for login

// Total			354 = 100 + 254
// NetWare (Bindery)	254
	BYTE FileServerNames[LENGTH_OF_FS_NAMES]; //Login File Server Names.

// Total			638 = 354 + 284
// NetWare (NDS)	284 = 33 + 251
	BYTE NDSTreeName[NDS_TREE_LEN]; 	   //NDS Tree Name
	BYTE NDSContext[NDS_CONTEXT_LEN];	   //NDS Context

// Total	650 = 638 + 12
// TCP/IP	12 = 4 + 4 + 4
	BYTE BoxIPAddress[4];		//Box IP Address for TCP/IP only
	BYTE SubNetMask[4]; 		//Subnet Mask for TCP/IP only
	BYTE GetwayAddress[4];		//Getway Address for TCP/IP only

// Total	726 = 650 + 76
// SNMP		76 = 16 + 25 + 2 * 13 + 1 + 2 * 4
	BYTE SnmpSysContact[SNMP_SYSCONTACT_LEN];
	BYTE SnmpSysLocation[SNMP_SYSLOCATION_LEN];
	BYTE SnmpCommunityAuthName[NO_OF_SNMP_COMMUNITY][SNMP_COMMUNITY_LEN];
	_SnmpAccessFlag SnmpAccessFlag;
	BYTE SnmpTrapTargetIP[NUM_OF_SNMP_TRAP_IP][4];

// Total		841 = 726 + 115
// AppleTalk	115 = 2 + 1 + 33 + 13 + 3 * 21 + 3
	WORD ATNet;						 	 //last net (Hi-Low)
	BYTE ATNode;						 //last node
	BYTE ATZoneName[ATALK_ZONE_LEN];	 //Apple ZONE (zero string)
	BYTE ATPortName[ATALK_PORT_NAME];	 //Port Object Name (zero string)
	BYTE ATPortType[3][ATALK_TYPE_LEN];  //Port Type Name	(zero string)
	BYTE ATDataFormat[3];				 //Binary communicate protocol
										// 0x00: ASCII
										// 0x01: TBCP (Tag Binary Communication Protocol)
										// 0x02: BCP (Binary Communication Protocol)

// Total		848 = 841 + 7
// Device		7 = 1 + 3 + 3
	BYTE PrinterSpeed;			//0: Fast 1:NORMAL 2:Slow, other : slow speed value
	BYTE Bidirectional[3];		//1:Auto / 0:Disable Bidirectional
	BYTE IEEE1284Mode[3];		//1:Auto / 0:SPP	mode

// Total		856 = 848 + 8
// Test Info	8 = 1 + 1 + 1 + 4 + 1
	BYTE TimeOutValue;			//Box Time Out value
	BYTE PrnMonMaxPacket;		//0: 1 Packet, 1: 2 Packet (Print Monitor Max Packet)
	BYTE PrnTestMode;			//0: Normal, 1:Eat queue data, 2: Eat and Compare
	DWORD EEPROMWriteCount; 	//EEPROM Write count !
	BYTE PrintServerMode2;

// Total		911 = 856 + 55
// SMB			55 = 16 + 3 * 13
	char WorkGroupName[NCBNAMSZ];
	char ServiceName[3][SERVICENAME_LENGTH];	

// Total				912 = 911 + 1
// OEM Model Version	1
	BYTE SPECIAL_OEM;			// 0x00: default	0x01: Custom	0x02: D-LINK
								// 0x03: P1U1U2		0x04: P1U2U3	0x05:Port1Port2Port3
	
// Total				950 = 912 + 38
// Mail Alert			38 = 1 + 4 + 32 + 1
	BYTE AlertEnabled;			// 0x00: Disable	0x01: Enable
	BYTE SMTPIP[4];				// SMTP server IP address
	BYTE AlertAddr[32];			//Email address
	
	BYTE IPLoadDef;				//0:No, 1:Yes for H/W(code1)

// Total				952 = 950 + 2
// PORT Descriptor		2 = 1 + 1
	BYTE PORTDESC;				// The right first two bits: Port 1
								// The right second two bits: Port 2
								// The right third two bits: Port 3
								// The left two bits: undefine 00 default
								// For each port: 00 default 01 Parallel 10 USB 11 undefine

	BYTE PORTTAIL;				// The right first two bits: Sequence of Port 1
								// The right second two bits: Sequence of Port 2
								// The right third two bits: Sequence of Port 
	
// Total	991 = 952 + 39
// LPR		39 = 3 * 13
	BYTE LPRQueueName[3][LPRQUEUENAME_LENGTH];

// Total		1056 = 991 + 65
// Randezvous	65 = 1 + 64
	BYTE RENVEnable;			// 0x00: Disable	0x01: Enable
	BYTE RENVServiceName[64];

// Total		1060 = 1056 + 1 + 1 + 1 + 1
// Others		4 = 1 + 1 + 1 + 1
	BYTE APPTLKEn;				// 0x00: AppleTalk disable		0x01: AppleTalk enable
	BYTE IPXDisable;			// 0x00: IPX enable				0x01: IPX disable

	BYTE WLWPAType;				//if (WLAuthenticationType == 4) 0: WPA-PSK(TKIP),  1: WPA-PSK(AES-CCMP),
                                //if (WLAuthenticationType == 5) 0: WPA2-PSK(TKIP), 1: WPA2-PSK(AES-CCMP),

	BYTE WLTxMode;              ////0:B/G Mixed 1:B Only 2:G Only 3:B/G Mixed	PS: apmode or WLTxMode

	BYTE WLDataRate;			// 0~3:CCK 4~11:OFDM 12~27:MCS0~15
	BYTE WLBandWidth;			// 0:20MHz 1:40MHz
	
	BYTE WPSButtonPressedCount;	// WPS button pressed count: 0 - 255

// Total		1408 = 1369 + 84
// Reserved		36
	BYTE Reserved[36];

// Total	1105 = 1060 + 45
// WPA		45 = 15 + 15 +15
	BYTE WLWEP128Key2[15];
	BYTE WLWEP128Key3[15];
	BYTE WLWEP128Key4[15];

// Total	1169 = 1105 + 64
// WPA		64
	BYTE WPA_Pass[64];
	
// Total	1170 = 1169 + 1
// ISL38xx	1
	BYTE ISLFlashError;			// 0x00: No Error	0x01: Error

// For EMI Testing
// Total		1220 = 1170 + 50
// Wireless		50 = 1 + 4 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 32 + 1 + 1 + 1
	BYTE WLMTMode;
	DWORD WLMTChannel;
	BYTE WLMTRate;
	BYTE WLMTPreamble;
	BYTE WLMTLength;
	BYTE WLMTScrambling;
	BYTE WLMTFilter;
	BYTE WLMTAntenna_rx;
	BYTE WLMTAntenna_tx;
	BYTE WLMTPower_loop;
	BYTE WLMTKey_type;
	BYTE WLMTKey_length;
	BYTE WLMTKey[32];
	BYTE WLMTCCAMode;
	BYTE WLMTAutorespond;
	
	BYTE WLExtRate; 		    // bit0:  6M,  bit1:  9M,  bit2: 12M,   bit3: 18M
	                            // bit4: 24M,  bit5: 36M,  bit6: 48M,   bit7: 54M 
	                            
// Total		1263 = 1220 + 43
// 802.1X EAP Setting   3 item 43 bytes....
	BYTE EAP_Type;				// 0x00: NONE	0x01: MD5	0x02: CHAPV2
	BYTE EAP_Name[21];			//EAP Account Name
	BYTE EAP_Password[21];		//EAP password
	
// Total		1269 = 1263 + 6
// Wireless		6 = 1 + 1 + 1 + 2 + 1
	BYTE WLNonModulate;			// 0x00: Modulation		0x01: Non-Modulation
	BYTE WLWEPKeyType;          // 0x00: ASCII			0x01: HEX
	BYTE WLVersion;				// 0x00: Auto			0x01: 2.0		0x02: 2.5
	WORD WLTxPower;				//TX power control
	BYTE WLZone;				// 0xA1: US			0xA2: Europe		0xA3: France
								// 0xA4: Japan		0xA5: Spain		otherwise is autodetect
								
// Total		1345 = 1269 + 76
// Wireless		76 = 1 + 1 + 32 + 1 + 1 + 1 + 6 + 6 + 6 + 6 + 15
	BYTE WLAPMode;				// 0x00: WLAPMode disable	0x01: WLAPMode enable
	BYTE WLMode;				// 0x00: Infrastructure	0x01: Ad-Hoc	0x02: 802.11b Ad-Hoc
	BYTE WLESSID[32];
	BYTE WLChannel;
	BYTE WLWEPType;				// 0x00: disable	0x01: 64-bit		0x02: 128-bit
	BYTE WLWEPKeySel;

	BYTE WLWEPKey1[6];
	BYTE WLWEPKey2[6];
	BYTE WLWEPKey3[6];
	BYTE WLWEPKey4[6];
	BYTE WLWEP128Key[15];		// This item is equal to WLWEP128Key1

// Total		1370 = 1345 + 25
// Wireless		25 = 2 + 2 + 2 + 1 + 1 + 1 + 1 + 2 + 2 + 2 + 1 + 1 + 1 + 1 + 1 + 4
	WORD WLBeaconinterval;
	WORD WLRTSThreshold;
	WORD WLFragmentation;
	BYTE WLRates;				//Basic Rate	... Ron change 2/6/2003
	BYTE WLRate;				//TX Rate

	BYTE WLShortPreamble;
	BYTE WLAuthType;			// 0x01: Open system		0x02: Share Key
								// 0x03: Both use			0x04: WPA-PSK
								// 0x05: WPA2-PSK Enable	0x06:WPA-802.1x
								// 0x07: WPA2-802.1x

	WORD WLDtiminterval;
	WORD WLCfpperiod;
	WORD WLCfpmaxduration;

	BYTE WLCRX;
	BYTE WLCTX;
	BYTE WLJapan;
	BYTE WLAnSide;

	BYTE CheckSum2[4]; 			//Any Value
#ifdef ADVANCED_EEP
// All EEP Total	3500 = 1408 + 2092
// Enhance Part

#if defined(MAC_FILTERING)
	// Enhance Part Total	1819 = 1408 + 411
	// MAC Filtering		411 = 1 + 1 + 1 + 24 * 11 + 24 * 6
	BYTE MF_Enabled;					// 0x00: Disabled	0x01: Enabled
	BYTE MF_DenyAll;					// 0x00: Allow all	0x01: Deny all
	BYTE MF_ControlListSize;			// Control List size
	BYTE MF_Name[MF_CONTROLLISTMAXSIZE][MF_NAMELEN];
	BYTE MF_MACAddress[MF_CONTROLLISTMAXSIZE][6];

	// Enhance Part Total	3496 = 1819 + 1677
	// Reserved				1677
	BYTE Reserved_Adv[1677];
#else
	// Enhance Part Total	1468 = 1408 + 60
	// Wireless WEP 128-bit	60 = 15 + 15 + 15 + 15
	BYTE WLWEP128Key1[15];		// This item is equal to WLWEP128Key
	BYTE WLWEP128Key2[15];
	BYTE WLWEP128Key3[15];
	BYTE WLWEP128Key4[15];

	// Enhance Part Total	3496 = 1468 + 2028
	// Reserved				2028
	BYTE Reserved_Adv[2028];
#endif	// defined(MAC_FILTERING)	
	// Enhance Part Total	3501 = 3497 + 4
	// Checksum				4
	BYTE CheckSum3[4]; 			//Any Value
#endif ADVANCED_EEP
	
}PACK	EEPROM ;

extern EEPROM	EEPROM_Data;
extern EEPROM	DEFAULT_Data;
extern EEPROM	QC0_Defualt_EEPROM;

/* Ron Add 8/27/2004 */
int ReadFromFactory(EEPROM *Data);

#endif /*_EEPROM_H*/
