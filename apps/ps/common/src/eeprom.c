#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "led.h"

#define BOX_EEPROM_ERROR  		"BOX_FLASH_ERROR"

#define FS_NEW_NAME 			"NEW-FIRMWARE\0"
#define DF_PS_NAME  			"DEFAULT_PS_NAME"
#define DF_FS_NAME  			"DEFAULT_FS_NAME"
#define DF_BOX_NAME 			"P_PS-"

#define AT_DEFAULT_NAME 		"ATALK_PS"
#define AT_DEFAULT_TYPE 		"LaserWriter"

uint16  OffsetOfFSName[MAX_FS]; //Offset of FS name from EEPROM.FileServerNames
uint32	PollingTime;
uint8   PSMode;
uint8   PSMode2;
uint8   PSUpgradeMode = WAIT_UPGRADE_MODE;
uint8   NTMaxRecvPacket;
//Jesse uint8   (*pOutputQueue)(uint8 *SrcBuf, uint16 DataSize ,uint8 Port);
uint8   CurSetupPassword[SETUP_PASSWD_LEN+1];
uint8   DefaultNodeID[] = { 0x00, 0x40, 0x01, 0xff, 0xff, 0xff };

#ifdef 	DEF_CENTRAL_PRINT
uint8   CentralPrintID[6];
#endif

BYTE MyPhysNodeAddress[6];	//temp here. (in ipx.c)

char *__BoxIPAddress = &EEPROM_Data.BoxIPAddress;
char *__SubNetMask = &EEPROM_Data.SubNetMask;
char *__GetwayAddress = &EEPROM_Data.GetwayAddress;

char	Hostname[LENGTH_OF_BOX_NAME+1] = {0};

#ifdef RENDEZVOUS
extern BYTE mRENVEnable;
#endif //RENDEZVOUS

// Charles 2001/07/17
EEPROM	EEPROM_Data;
EEPROM	DEFAULT_Data;
//EEPROM	FACTORY_Data;

EEPROM	QC0_Defualt_EEPROM;

#ifdef CFG_EXPIMP
// George added this at build0026 of 716U2 on June 18, 2012.
//	Not knowing why, this varible MUST be declared here.
EEPROM	EEPROM_Data_temp;
#endif	// CFG_EXPIMP

//from psmain.c
extern uint32 msclock( void );
extern uint32 rdclock( void );
extern int urandom(unsigned int n);

//from httpd.c
// Created on 4/26/2000.
// Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
extern BYTE WebLangVersion[];

//from endmark.c
extern unsigned long MyDataSize[4];
extern unsigned char MyData[];
//extern unsigned char *MyData;

//from rwflash.c
extern int ReadFromFlash(EEPROM *RomData);
extern int ReadFromDefault(EEPROM *RomData);
extern int WriteToFlash(EEPROM *RomData);

//temp extern UINT8   mvWDomain; //Ron Add 9/23/04

UINT8    mvAPPTLKEn;
UINT8    mvATPortName[ATALK_PORT_NAME];
UINT8    mvRENVEnable;
UINT8    mvRENVServiceName[64];




uint32 clock()
{
	uint32 pvalue;
	HAL_CLOCK_READ(&pvalue);
	return (pvalue);
}

uint32 free_clock()
{
	uint32 pvalue;
	pvalue = *(uint32 *) (0x79000040);
	return (pvalue);
}

uint32 RANDOMID_32K_ADDRESS = 0x7F8000L;  
uint32 RANDOMID_START_ADDRESS = 0x7FFE00L;

void get_randnodeid(char *nodeid){
	BYTE rand1, rand2, rand3, rand4;
	uint32 rand_data=0,*rand_addr=0;
	int i,j,k;
	uint32 data=0,zero_data=0;
	uint32 data_temp[14]={0};

	for( i = 0 ; i < 32 ; i++)
	{
		rand_addr = (uint32 *)(RANDOMID_START_ADDRESS - ( i * 0xCC0 + 0x30));

		for( j = 0; j < 128 ; j++)
		{
			data = *(uint32 *)(rand_addr+j);
			
			if( data == 0 )
				zero_data ++;
			
			if(zero_data > 5)
				break ;
			
			for( k = 0; k < 14 ; k++ )
			{
				if( (data_temp[k] == 0) || (data_temp[k] == data) )
				{
					data_temp[k] = data;
					break;
				}			
			}
			
			if( data_temp[13] != 0 )
				break ;
		}
		
		if( data_temp[13] != 0)
			break;
		
		for( k = 0; k < 14 ; k++ )
			data_temp[k] = 0;
		
		zero_data = 0;	
	}
	
	if( i == 32)
	{	
		rand_addr = (uint32 *)RANDOMID_32K_ADDRESS;
		while( rand_addr < 0x7FFFFF)
		{
			rand_data += *(uint32 *)rand_addr;
			rand_addr++;
		}
	}
	else
	{
		for(k = 0; k < 128; k++ )
		{
			rand_data += *(uint32 *)(rand_addr+k);
		}
	}
	
	srand(rand_data);
	
	rand1 = (rand() * (rand() * free_clock()+rand_data))+0x11;
	rand2 = (rand() * (rand() * free_clock()+rand_data))+0xAA;
	rand3 = (rand() * (rand() * free_clock()+rand_data))+0xBB;
	rand4 = (rand() * (rand() * free_clock()+rand_data))+0xCC;

	memcpy(nodeid, DefaultNodeID, 6);
	memcpy(&nodeid[2], &rand1, sizeof(BYTE));
	memcpy(&nodeid[3], &rand2, sizeof(BYTE));
	memcpy(&nodeid[4], &rand3, sizeof(BYTE));
	memcpy(&nodeid[5], &rand4, sizeof(BYTE));
	nodeid[2] = RANDID_SIGNATURE;

}

int ReadFromFactory(EEPROM *Data){
	int i;
	
	memset(Data, 0x00, sizeof(EEPROM));
	strcpy(Data->ZOT_Mark, "ZOT"); //ZOT mark "ZOT"
	Data->EEPROM_Version = 0x0B; //for EEPROM upgrade

//System
	/* Ron pending */
	Data->EthernetID[0] = 0x00; //Physical Node Address.
	Data->EthernetID[1] = 0x40;
	Data->EthernetID[2] = 0x01;
	Data->EthernetID[3] = 0xFF;
	Data->EthernetID[4] = 0xFF;
	Data->EthernetID[5] = 0xFF;

	Data->Version[0] = 0x00;  //Hardware version = Version[0] & 0xF
	Data->Version[1] = 0x06;  //Software version = BYTE[1] * 0x10 + (BYTE[0] >> 4)
	Data->Model = 0x32; 	   //00:N6101EP    01:N6102EP		02:N6200EP
							   //03:N6300EP    04:N6301EP		06:N6300II
							   //07:737,6300+  08:7227			09:PCI520
							   //10:7117	   11:7339,7339+	12:7119
							   //13:5225+      14:7119A			15:7339A
							   //16:7339AW     17:7119AU		18:535U
							   //19:535U-CD    20:535WPI        21:535WUI 
							   //22:525PR      23:535WPID		24:635U2P
							   //25:9312	   26:635U2PW       27:
							   //28:           29:              30:
							   //31:           32:615WU
							   //50:716U2W								   
	memset(Data->BoxName, 0x00, LENGTH_OF_BOX_NAME); //Device Name. (none zero string) 
#ifdef O_AXIS
	strcpy(Data->BoxName, "AXIS"); 
#else
	strcpy(Data->BoxName, "1P_PrintServ"); 
#endif	
	memset(Data->Password, 0x00, SETUP_PASSWD_LEN); //Configuration Device Password.
	Data->NumOfPort = 0x01;    //How many port are this printer support ? //10/29/98
#ifdef O_AXIS
	Data->PrintServerMode = 0xEE;  // 1:Netware, 2:Unix,  4:Windows, 8:Atalk
							        //10:Nds,    20:Ippd, 40:Smb,	80:Dhcp 
#else
	Data->PrintServerMode = 0x6E;  // 1:Netware, 2:Unix,  4:Windows, 8:Atalk
							        //10:Nds,    20:Ippd, 40:Smb,	80:Dhcp 
#endif		
//NetWare (Both)
	memset(Data->PrintServerName, 0x00, 48);  //Print Server Name on File Server.
	strcpy(Data->PrintServerName, "DEFAULT_PS_NAME");
	Data->PollingTime = 0x03;  //Polling Print Server Queue Time.
	
	memset(Data->NovellPassword, 0x00, NOVELL_PASSWORD_LEN); //NDS & Bindery Password for login
	
//NetWare (Bindery)
	memset(Data->FileServerNames, 0x00, LENGTH_OF_FS_NAMES); //Login File Server Names.
	strcpy(Data->FileServerNames, "DEFAULT_FS_NAME");
	
//NetWare (NDS)
	memset(Data->NDSTreeName, 0x00, NDS_TREE_LEN); //NDS Tree Name
	memset(Data->NDSContext, 0x00, NDS_CONTEXT_LEN); //NDS Context

//TCP/IP
#ifdef O_AXIS
	Data->BoxIPAddress[0] = 0x00; //Box IP Address for TCP/IP only
	Data->BoxIPAddress[1] = 0x00; // 
	Data->BoxIPAddress[2] = 0x00; //
	Data->BoxIPAddress[3] = 0x00; //
	
	Data->SubNetMask[0] = 0x00;  //Subnet Mask for TCP/IP only
	Data->SubNetMask[1] = 0x00;
	Data->SubNetMask[2] = 0x00;
	Data->SubNetMask[3] = 0x00;
	
	Data->GetwayAddress[0] = 0x00;  //Getway Address for TCP/IP only
	Data->GetwayAddress[1] = 0x00;
	Data->GetwayAddress[2] = 0x00;
	Data->GetwayAddress[3] = 0x00;		
#else
	Data->BoxIPAddress[0] = 0xC0; //192, Box IP Address for TCP/IP only
	Data->BoxIPAddress[1] = 0xA8; //168 
	Data->BoxIPAddress[2] = 0x64; //100
	Data->BoxIPAddress[3] = 0x05; //5
	
	Data->SubNetMask[0] = 0xFF;  //Subnet Mask for TCP/IP only
	Data->SubNetMask[1] = 0xFF;
	Data->SubNetMask[2] = 0xFF;
	Data->SubNetMask[3] = 0x00;
	
	Data->GetwayAddress[0] = 0xC0;  //Getway Address for TCP/IP only
	Data->GetwayAddress[1] = 0xA8;
	Data->GetwayAddress[2] = 0x64;
	Data->GetwayAddress[3] = 0x01;		
#endif
//SNMP
	memset(Data->SnmpSysContact, 0x00, SNMP_SYSCONTACT_LEN);
	memset(Data->SnmpSysLocation, 0x00, SNMP_SYSLOCATION_LEN);

	for(i=0; i< NO_OF_SNMP_COMMUNITY; i++){
		strcpy(Data->SnmpCommunityAuthName[i], "public");
	}

    Data->SnmpAccessFlag.SnmpTrapEnable = 0;
    Data->SnmpAccessFlag.SnmpAuthenTrap = 2;
    Data->SnmpAccessFlag.SnmpComm0AccessMode = 1;
    Data->SnmpAccessFlag.SnmpComm1AccessMode =1;
    Data->SnmpAccessFlag.SnmpNoUsed= 0;
    
	for(i=0; i< NUM_OF_SNMP_TRAP_IP; i++){
		memset(Data->SnmpTrapTargetIP[i], 0x00, 4);
	}

//AppleTalk
	Data->ATNet = 0x00000;               				//last net (Hi-Low)
	Data->ATNode = 0x00;				 				//last node	
	memset(Data->ATZoneName, 0x00, ATALK_ZONE_LEN);     //Apple ZONE (zero string)
	memset(Data->ATPortName, 0x00, ATALK_PORT_NAME);    //Port Object Name (zero string)
#ifdef O_AXIS
	strcpy(Data->ATPortName, "AXaxaxax_USB");
#else
	strcpy(Data->ATPortName, "ATALK_PS");
#endif	
	for(i=0; i<3; i++){									//Port Type Name	(zero string)
		strcpy(Data->ATPortType[i], "LaserWriter");	
	}
	Data->ATDataFormat[0] =0x01;						//Binary communicate protocol
	Data->ATDataFormat[1] =0x01;					    //0: ASCII, 1:TBCP (Tag Binary Communication Protocol)
	Data->ATDataFormat[2] =0x01;						//2: BCP (Binary Communication Protocol)

//Device
	Data->PrinterSpeed = 0x00;  //0: Fast 1:NORMAL 2:Slow, other : slow speed value
	Data->Bidirectional[0] = 0x00; //1:Auto / 0:Disable Bidirectional
	Data->Bidirectional[1] = 0x00; //1:Auto / 0:Disable Bidirectional
	Data->Bidirectional[2] = 0x00; //1:Auto / 0:Disable Bidirectional
	
	Data->IEEE1284Mode[0] = 0x01; 	//1:Auto / 0:SPP	mode
	Data->IEEE1284Mode[1] = 0x01;   //1:Auto / 0:SPP	mode
	Data->IEEE1284Mode[2] = 0x01;   //1:Auto / 0:SPP	mode

//Test Info
	Data->TimeOutValue = 0x5A;      		//Box Time Out value
	Data->PrnMonMaxPacket = 0x06;    		//0: 1 Packet, 1: 2 Packet (Print Monitor Max Packet)
	Data->PrnTestMode = 0x00;       	   	//0: Normal, 1:Eat queue data, 2: Eat and Compare
	Data->EEPROMWriteCount = 0x00000000;  	//EEPROM Write count !
	Data->PrintServerMode2 = 0x03;//( PS_RAWTCP_MODE | PS_FTP_MODE )
	
//SMB
	memset(Data->WorkGroupName, 0x00, NCBNAMSZ);
	strcpy(Data->WorkGroupName, "WORKGROUP");
	for(i=0; i<3; i++){
		memset(Data->ServiceName[i], 0x00, SERVICENAME_LENGTH);
		sprintf(Data->ServiceName[i], "LP%d", i+1);
	} 

//OEM Model Version	
	Data->SPECIAL_OEM = 0x00;       //0:ZOT  1:ELECOM   2:DLINK 
	
//Mail Alert
	Data->AlertEnabled = 0x00;     		//0:Disable, 1:Enable
	memset(Data->SMTPIP, 0x00, 4);  	//SMTP IP addr
	memset(Data->AlertAddr, 0x00, 32);  //Email address
	Data->IPLoadDef = 0x00;				//0:No, 1:Yes for H/W(code1)
	 
//PORT Descriptor
	Data->PORTDESC = 0x02;		// The right first two bits: Port 1
								// The right second two bits: Port 2
								// The right third two bits: Port 3
								// The left two bits: undefine 00 default
								// For each port: 00 default 01 Parallel 10 USB 11 undefine

	Data->PORTTAIL = 0x01;		// The right first two bits: Sequence of Port 1
								// The right second two bits: Sequence of Port 2
								// The right third two bits: Sequence of Port 
	
//LPR 
	for(i=0; i<3; i++){
		memset(Data->LPRQueueName[i], 0x00, LPRQUEUENAME_LENGTH);
		sprintf(Data->LPRQueueName[i], "lp%d", i+1);	
	}

//Rendezvous 
//    Data->RENVEnable = 1; 						//Ron 12/13/04 
    Data->RENVEnable = 0; 							// Add for Factory 
#ifdef O_AXIS
	strcpy(Data->RENVServiceName, "AXISaxaxax Wireless Print Server"); 	//Ron 12/13/04
#else
    memset(Data->RENVServiceName, 0x00, 64); 	//Ron 12/13/04
#endif

	Data->APPTLKEn = 1;		// 0x00: AppleTalk disable		0x01: AppleTalk enable
	Data->IPXDisable = 0;	// 0x00: IPX enable				0x01: IPX disable

//Wireless
	Data->WLWPAType = 0;	//if (WLAuthenticationType == 4) 0: WPA-PSK(TKIP),  1: WPA-PSK(AES-CCMP),
                            //if (WLAuthenticationType == 5) 0: WPA2-PSK(TKIP), 1: WPA2-PSK(AES-CCMP),

	Data->WLTxMode = 3;		//0:B/G Mixed 1:B Only 2:G Only	3:B/G/N Mixed PS: apmode or WLTxMode

	Data->WLBeaconinterval = 100;
	Data->WLRTSThreshold = 2347;
	Data->WLFragmentation = 2346;

	memset(Data->WLWEP128Key2, 0x00, 15);
	memset(Data->WLWEP128Key3, 0x00, 15);
	memset(Data->WLWEP128Key4, 0x00, 15);
	memset(Data->WPA_Pass, 0x00, 64);

// EAP 
	memset(Data->EAP_Type, 0x00, 1);
	memset(Data->EAP_Name, 0x00, 21);
	memset(Data->EAP_Password, 0x00, 21);


	Data->WLWEPKeyType = 0x00;				//0: ASCII 1: HEX
	Data->WLZone = 0xA1;					//A1-A5, otherwise is autodetect
											//0xA1:  USA      1 - 11
											//0xA2:  ETSI     1 - 13
											//0xA3:  France  11 - 13
											//0xA4:  Japan    1 - 14
											//0xA5:  SPAIN    1 - 13
											
	Data->WLAPMode = 0x00;	
	Data->WLWEPType = 0x00;
	
	Data->WLMode = 2;		// 0x00: Infrastructure	0x01: Ad-Hoc	0x02: 802.11b Ad-Hoc
	strcpy(Data->WLESSID, "WLAN-PS");
	Data->WLChannel = 6;
	Data->WLWEPType = 0;	// 0x00: disable	0x01: 64-bit		0x02: 128-bit
	Data->WLWEPKeySel = 0;
	Data->WLBandWidth = 0;	//0:20MHz 1:40MHz
	Data->WLDataRate = 11;	//0~3:CCK 4~11:OFDM 12~27:MCS0~15
	
	memset(Data->WLWEPKey1, 0x00, 6);
	memset(Data->WLWEPKey2, 0x00, 6);
	memset(Data->WLWEPKey3, 0x00, 6);
	memset(Data->WLWEPKey4, 0x00, 6);
	memset(Data->WLWEP128Key, 0x00, 15);
	
	Data->WLRates = 0x00;				//Basic Rate	... Ron change 2/6/2003
	Data->WLRate = 0x0F;				//TX Rate  bit0:  1M,  bit1:  2M,  bit2:5.5M,   bit3: 11M
	Data->WLExtRate = 0xFF;				// 		   bit0:  6M,  bit1:  9M,  bit2: 12M,   bit3: 18M
	                            		//         bit4: 24M,  bit5: 36M,  bit6: 48M,   bit7: 54M 
	Data->WLShortPreamble = 0x01;   	//0: long, 1:short
	Data->WLAuthType = 0x01; 			//1: Open system, 2: Share Key, 3: Both use, 4: WPA-PSK Enable

}

// Extra post-settings from customer -- TP-Link
// George created this sub-function at build0006 of 716U2W on April 28, 2011.
// #if defined(O_TPLINK)

// Extra post-settings default from customer -- TP-Link
// int EEPROMInit_DEFAULT_O_TPLINK()
// {
// }
// #endif	// defined(O_TPLINK)

// Extra post-settings from customer -- ZO TECH
// George created this sub-function at build0006 of 716U2W on April 28, 2011.
// #if defined(O_ZOTCH) || defined(O_ZOTCHW) || defined(O_ZOTCHS)

// Extra post-settings default from customer -- ZO TECH
// int EEPROMInit_DEFAULT_O_ZOTCH()
// {
// }

// Extra post-settings current from customer -- ZO TECH
// int EEPROMInit_CURRENT_O_ZOTCH()
// {
// }
// #endif	// defined(O_ZOTCH) || defined(O_ZOTCHW) || defined(O_ZOTCHS)

void EEPROMInit(void)
{
	
	uint16  FSNameLen, FSNameTotalLen;
	uint8   *FileServerNames;
	uint8   NeedWriteEEPROM = 0, LoadDefaultEEPROM = 0, NeedWriteDEFAULT = 0;
	uint8   QCDefaultEEPROM = 1;
	uint8   *WebFlash;

#if defined(O_AXIS)
	BYTE	NullArray[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	BYTE	AXIS_ATPortName[13] = "AXaxaxax_USB";
	BYTE	AXIS_RendezvousServiceName[33] = "AXISaxaxax Wireless Print Server";
#endif	// defined(O_AXIS)	

#ifdef PreTest
	BYTE	BoxIPAddress_PreTest[4] = {0xC0, 0xA8, 0x00, 0xC8};		// 192.168.0.200
	BYTE	SubNetMask_PreTest[4] = {0xFF, 0xFF, 0xFF, 0x00};		// 255.255.255.0
	BYTE	GetwayAddress_PreTest[4] = {0xC0, 0xA8, 0x00, 0x01};	// 192.168.0.1
#endif	// PreTest

	if( ReadFromQC0_Default(&QC0_Defualt_EEPROM) != 0 )
	{
		if( ReadFromQC0_Default(&QC0_Defualt_EEPROM) != 0 )
		{
			QCDefaultEEPROM = 0;
		}
	}

	if( ReadFromDefault(&DEFAULT_Data) != 0 )
	{
		if( ReadFromDefault(&DEFAULT_Data) != 0 )
		{
			// DEFAULT data is wrong, use ZOT debug settings
			ReadFromFactory(&DEFAULT_Data); //Ron Add 8/30/04			
//			NeedWriteEEPROM = 1;
			NeedWriteDEFAULT = 1;
		}
	}
	
	if(QCDefaultEEPROM)
	{
		if( memcmp(QC0_Defualt_EEPROM.EthernetID, DEFAULT_Data.EthernetID, 6) != 0 )
		{
			memcpy( &DEFAULT_Data, &QC0_Defualt_EEPROM, sizeof(EEPROM) );
			NeedWriteDEFAULT = 1;
		}
	}
			
	if( ReadFromEEPROM(&EEPROM_Data) != 0 )
	{
		if( ReadFromEEPROM(&EEPROM_Data) != 0 )
		{
			// use default EEPROM settings
			memcpy( &EEPROM_Data, &DEFAULT_Data, sizeof(EEPROM) );
			NeedWriteEEPROM = 1;
			// If DEFAULT_Data and EEPROM_Data are no data, it isn't QC0.
			if(LoadDefaultEEPROM)
				memcpy(_BoxName,BOX_EEPROM_ERROR,LENGTH_OF_BOX_NAME);
		}	
	}

	// Charles 2001/10/31, we support RawTCP & FTP printing from version 11
	// George modified this at build0006 of 716U2W on April 28, 2011.
	if(((unsigned char)EEPROM_Data.EEPROM_Version == 0x0A) || ((unsigned char)DEFAULT_Data.EEPROM_Version == 0x0A) )
	{
		EEPROM_Data.EEPROM_Version = 11;
		DEFAULT_Data.EEPROM_Version = 11;

		EEPROM_Data.PrintServerMode2 |= ( PS_RAWTCP_MODE | PS_FTP_MODE );
		DEFAULT_Data.PrintServerMode2 |= ( PS_RAWTCP_MODE | PS_FTP_MODE );
		
//		LoadDefaultEEPROM = 1;
		NeedWriteEEPROM = 1;
		NeedWriteDEFAULT = 1;
	}
	
	// WPS -- RT8188
	if(EEPROM_Data.WLMode > 0)
	{
		// If the wireless mode is not Infrastructure, set this value as 0.
		EEPROM_Data.WPSButtonPressedCount = 0;
		
		NeedWriteEEPROM = 1;
	}
	
	//======================================================================================//
	// Extra post-settings from customer													//
	//======================================================================================//
	
		// TP-Link
// #if defined(O_TPLINK)
//	if(EEPROMInit_DEFAULT_O_TPLINK())
//		NeedWriteDEFAULT = 1;
// #endif	// defined(O_TPLINK)

		// ZO TECH
//#if defined(O_ZOTCH) || defined(O_ZOTCHW) || defined(O_ZOTCHS)
		// George modified this at build0021 of 716U2 on March 23, 2011.
//		if(EEPROMInit_CURRENT_O_ZOTCH())
//			NeedWriteEEPROM = 1;

//		if(EEPROMInit_DEFAULT_O_ZOTCH())
//			NeedWriteDEFAULT = 1;
//#endif	// defined(O_ZOTCH) || defined(O_ZOTCHW) || defined(O_ZOTCHS)

	// Charles 2002/02/06
	if( DEFAULT_Data.NumOfPort != NUM_OF_PRN_PORT
		|| DEFAULT_Data.Model != CURRENT_PS_MODEL )
	{
		// read default EEPROM from code
		ReadFromFactory(&DEFAULT_Data); //Ron Add 8/30/04
		// update EEPROM_Data from DEFAULT_Data
		memcpy( &EEPROM_Data, &DEFAULT_Data, sizeof(EEPROM) );
//		LoadDefaultEEPROM = 1;
		NeedWriteDEFAULT = 1;
		NeedWriteEEPROM = 1;
	}
	
#ifdef IP_LOAD_DEFAULT
	if( (DEFAULT_Data.IPLoadDef != 1) || (EEPROM_Data.IPLoadDef != 1) ){
		DEFAULT_Data.IPLoadDef = 1;
		EEPROM_Data.IPLoadDef = 1;
		NeedWriteDEFAULT = 1;
		NeedWriteEEPROM = 1;
	}
#endif IP_LOAD_DEFAULT
	
	memcpy(Hostname,_BoxName,LENGTH_OF_BOX_NAME); //Jesse
    
	memcpy(MyPhysNodeAddress,EEPROM_Data.EthernetID, 6); // Copy Phys Address
	
#if defined(O_AXIS)
	if( memcmp(DEFAULT_Data.ATPortName, AXIS_ATPortName, 13) == 0 )
	{
		Node2Num(AXIS_ATPortName + 2, 3);
		memcpy(&DEFAULT_Data.ATPortName, AXIS_ATPortName, 13);
		memcpy(&EEPROM_Data.ATPortName, AXIS_ATPortName, 13);
//		LoadDefaultEEPROM = 1;
		NeedWriteDEFAULT = 1;
		NeedWriteEEPROM = 1;
	}
	
	if( DEFAULT_Data.RENVEnable == 0 )
	{
		DEFAULT_Data.RENVEnable = 1;
		EEPROM_Data.RENVEnable = 1;
//		LoadDefaultEEPROM = 1;
		NeedWriteDEFAULT = 1;
		NeedWriteEEPROM = 1;
	}
	
	if( (memcmp(DEFAULT_Data.RENVServiceName, NullArray, 8) == 0) || 
		(memcmp(DEFAULT_Data.RENVServiceName, AXIS_RendezvousServiceName, 33) == 0) )
	{
		Node2Num(AXIS_RendezvousServiceName + 4, 3);
		memcpy(&DEFAULT_Data.RENVServiceName, AXIS_RendezvousServiceName, 33);
		memcpy(&EEPROM_Data.RENVServiceName, AXIS_RendezvousServiceName, 33);
//		LoadDefaultEEPROM = 1;
		NeedWriteDEFAULT = 1;
		NeedWriteEEPROM = 1;
	}
		
	if( DEFAULT_Data.APPTLKEn == 0 )
	{
		DEFAULT_Data.APPTLKEn = 1;
		EEPROM_Data.APPTLKEn = 1;
//		LoadDefaultEEPROM = 1;
		NeedWriteDEFAULT = 1;
		NeedWriteEEPROM = 1;
	}
#endif	// defined(O_AXIS)

#ifdef PreTest
	DEFAULT_Data.PrintServerMode &= 0x7F;
	EEPROM_Data.PrintServerMode &= 0x7F;
	memcpy(&DEFAULT_Data.BoxIPAddress, BoxIPAddress_PreTest, 4);
	memcpy(&EEPROM_Data.BoxIPAddress, BoxIPAddress_PreTest, 4);
	memcpy(&DEFAULT_Data.SubNetMask, SubNetMask_PreTest, 4);
	memcpy(&EEPROM_Data.SubNetMask, SubNetMask_PreTest, 4);
	memcpy(&DEFAULT_Data.GetwayAddress, GetwayAddress_PreTest, 4);
	memcpy(&EEPROM_Data.GetwayAddress, GetwayAddress_PreTest, 4);
#endif	// PreTest
	
//	if( LoadDefaultEEPROM )
	if( NeedWriteDEFAULT )
	{
		if(WriteToDefault(&DEFAULT_Data) != 0)	// Write DEFAULT Data	
			ErrLightOnForever(Status_Lite); // Write DEFAULT Data error
		NeedWriteEEPROM = 1;
	}
	
	if(NeedWriteEEPROM) {
		if(WriteToEEPROM(&EEPROM_Data) != 0)	// Write EEPROM Data
			ErrLightOnForever(Status_Lite); // Write EEPROM Data error
	}

	/* If NODE ID is Default, get random NODE ID ..... Ron 4/3/2003 */
	if( memcmp(EEPROM_Data.EthernetID,DefaultNodeID,6) == 0 )
	{
		get_randnodeid(EEPROM_Data.EthernetID);
		if (EEPROM_Data.EthernetID[2] == RANDID_SIGNATURE)
		{
			memcpy(MyPhysNodeAddress,EEPROM_Data.EthernetID, 6);				
		}
	}
	
	// Charles 20001/08/09
	WebFlash = MyData + MyDataSize[1];
//	WebFlash = MyData;
	// Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
	WebLangVersion[0] = *(WebFlash +1);
	WebLangVersion[1] = *(WebFlash +2);

	FSNameTotalLen = 0;
	ServiceFSCount = 0;
	FileServerNames = EEPROM_Data.FileServerNames;

	while(*FileServerNames != '\0') {
		FSNameLen = strlen(FileServerNames);
		OffsetOfFSName[ServiceFSCount] = FSNameTotalLen;
		FileServerNames += FSNameLen+1;
		FSNameTotalLen += FSNameLen+1;
		ServiceFSCount++;                //  Service Count
	}

	PSMode = EEPROM_Data.PrintServerMode; //8/3/99 move from below
	PSMode2 = EEPROM_Data.PrintServerMode2; // 2001/10/31

	if(ServiceFSCount == 0)  {
		//disable netware PS mode !
        PSMode &= ~((uint8)PS_NETWARE_MODE); //8/3/99 changed
	}

#ifdef RENDEZVOUS
	mRENVEnable = EEPROM_Data.RENVEnable; 
#endif //RENDEZVOUS

	PollingTime = (EEPROM_Data.PollingTime) * TICKS_PER_SEC; //Polling Print Server Queue Time.

	if(EEPROM_Data.TimeOutValue < 10) EEPROM_Data.TimeOutValue = 90; //1/25/99

	NTMaxRecvPacket = EEPROM_Data.PrnMonMaxPacket + 1; //5/18/99, 8/19/99 changed

#ifdef USB_ZERO_CPY           	
/* Since one Print Queue size is 8K, and each NT Printing data size is 1460 bytes.
   1460 *6 =  8760 will more than 8K, NTMaxRecvPacket needs to be <=5.  ==>Ron 8/2/2005
*/	
	if (NTMaxRecvPacket > 5)
		NTMaxRecvPacket =5;
#endif

	memcpy(CurSetupPassword,_SetupPassword,SETUP_PASSWD_LEN);  //12/30/99
	CurSetupPassword[SETUP_PASSWD_LEN] = '\0';


	//Light_Flash(2,1); //EEPROM Initial OK	
//ZOT716u2	Light_ALL_Flash(2,1); //EEPROM Initial OK
	
}


char *get_rootpass(void){
	return 	EEPROM_Data.Password;
}

int get_rootpass_len(void){
	return 	sizeof(EEPROM_Data.Password);
}

uint8	code1_major_ver=0;
uint8	code1_minor_ver=0;
uint8	code1_build_low_ver=0;
uint8	code1_build_hi_ver=0;
uint8	code1_ps_model=0;
uint8	code1_firmware_string[60]={0};

uint8	code2_major_ver=0;
uint8	code2_minor_ver=0;
uint8	code2_ps_model=0;
uint8	code2_build_low_ver=0;
uint8	code2_build_hi_ver=0;
uint8	code2_release_ver=0;
uint8	code2_firmware_string[60]={0};

uint8	Loader_firmware_string[60]={0};

void read_version()
{
	
	read_flash((CODE1_START_ADDRESS+MAJOR_VER_OFFSET), &code1_major_ver, 1);
	read_flash((CODE1_START_ADDRESS+MINOR_VER_OFFSET), &code1_minor_ver, 1);
	read_flash((CODE1_START_ADDRESS+PS_MODEL_OFFSET), &code1_ps_model, 1);
	read_flash((CODE1_START_ADDRESS+BUILD_VER_OFFSET), &code1_build_low_ver, 1);
	read_flash((CODE1_START_ADDRESS+BUILD_VER_OFFSET+1), &code1_build_hi_ver, 1);
	read_flash((CODE1_START_ADDRESS+FIRMWARE_STRING_OFFSET), &code1_firmware_string, 60);

	
	read_flash((RAM_CODE_START_ADDRESS+MAJOR_VER_OFFSET), &code2_major_ver, 1);
	read_flash((RAM_CODE_START_ADDRESS+MINOR_VER_OFFSET), &code2_minor_ver, 1);
	read_flash((RAM_CODE_START_ADDRESS+PS_MODEL_OFFSET), &code2_ps_model, 1);
	read_flash((RAM_CODE_START_ADDRESS+BUILD_VER_OFFSET), &code2_build_low_ver, 1);
	read_flash((RAM_CODE_START_ADDRESS+BUILD_VER_OFFSET+1), &code2_build_hi_ver, 1);
	read_flash((RAM_CODE_START_ADDRESS+RELEASE_VER_OFFSET), &code2_release_ver, 1);
	read_flash((RAM_CODE_START_ADDRESS+FIRMWARE_STRING_OFFSET), &code2_firmware_string, 60);

	read_flash((LOADER_START_ADDRESS+FIRMWARE_STRING_OFFSET), &Loader_firmware_string, 60);
}