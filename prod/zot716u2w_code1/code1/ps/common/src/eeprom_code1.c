#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "led.h"

#define BOX_EEPROM_ERROR  		"BOX_FLASH_ERROR"

uint8   PSMode;
uint8   PSMode2;
uint8   PSUpgradeMode = WAIT_UPGRADE_MODE;

uint8   DefaultNodeID[] = { 0x00, 0x40, 0x01, 0xff, 0xff, 0xff };


char *__BoxIPAddress = &EEPROM_Data.BoxIPAddress;
char *__SubNetMask = &EEPROM_Data.SubNetMask;
char *__GetwayAddress = &EEPROM_Data.GetwayAddress;

char	Hostname[LENGTH_OF_BOX_NAME+1] = "PS-TEST";

uint8   CurSetupPassword[SETUP_PASSWD_LEN+1];

// Charles 2001/07/17
EEPROM	EEPROM_Data;
EEPROM	DEFAULT_Data;

//from psmain.c
extern int urandom(unsigned int n);

//from rwflash.c
extern int ReadFromFlash(EEPROM *RomData);
extern int ReadFromDefault(EEPROM *RomData);
extern int WriteToFlash(EEPROM *RomData);

extern unsigned char enaddr[]; //Ron Add 9/22/04
extern UINT8   mvWDomain; //Ron Add 9/23/04

uint32 clock()
{
	uint32 pvalue;
	HAL_CLOCK_READ(&pvalue);
	return (pvalue);
}

void get_randnodeid(char *nodeid){
	BYTE rand1, rand2, rand3, rand4;
	
	rand1 = urandom(clock()+0xAA);
	rand2 = urandom(clock()+0xBB);
	rand3 = urandom(clock()+0xCC);
	rand4 = urandom(clock()+0xFF);

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

//	memset(Data->Reserved, 0x00, 145);  //Reserve bytes, now 145 bytes
//	memset(Data->Reserved, 0x00, 272);  //Reserve bytes, now 272 bytes

/*
// EAP 
	memset(Data->EAP_Type, 0x00, 1);
	memset(Data->EAP_Name, 0x00, 21);
	memset(Data->EAP_Password, 0x00, 21);
*/	
//Wireless
	Data->WLWEPKeyType = 0x00;				//0: ASCII 1: HEX
	Data->WLZone = 0xA1;					//A1-A5, otherwise is autodetect
											//0xA1:  USA      1 - 11
											//0xA2:  ETSI     1 - 13
											//0xA3:  France  11 - 13
											//0xA4:  Japan    1 - 14
											//0xA5:  SPAIN    1 - 13
											
	Data->WLAPMode = 0x00;	
	Data->WLWEPType = 0x00;
	memset(Data->WLWEPKey1, 0x00, 6);
	memset(Data->WLWEPKey2, 0x00, 6);
	memset(Data->WLWEPKey3, 0x00, 6);
	memset(Data->WLWEPKey4, 0x00, 6);
	memset(Data->WLWEP128Key, 0x00, 15);
	
	Data->WLRates = 0x00;				//Basic Rate	... Ron change 2/6/2003
	Data->WLRate = 0x0F;				//TX Rate  bit0:  1M,  bit1:  2M,  bit2:5.5M,   bit3: 11M
	Data->WLExtRate = 0xFF;				// 		   bit0:  6M,  bit1:  9M,  bit2: 12M,   bit3: 18M
	                            		//         bit4: 24M,  bit5: 36M,  bit6: 48M,   bit7: 54M 
	Data->WLShortPreamble = 0x00;   	//0: long, 1:short
	Data->WLAuthenticationType = 0x01; 	//1: Open system, 2: Share Key, 3: Both use, 4: WPA-PSK Enable
	Data->WLWPATYPE = 0x00;				//0: WPA-PSK(TKIP), 1: WPA-PSK(AES), 2: WPA2-PSK(TKIP) 
	
//	memset(Data->Reserved1, 0x00, 6);  //Reserve 6 bytes	
	


}

void EEPROMInit(void)
{
	
	uint16  FSNameLen, FSNameTotalLen;
	uint8   *FileServerNames;
	uint8   NeedWriteEEPROM = 0, LoadDefaultEEPROM = 0;
	uint8   *WebFlash;
	
	if( ReadFromDefault(&DEFAULT_Data) != 0 )
	{
		if( ReadFromDefault(&DEFAULT_Data) != 0 )
		{
			// DEFAULT data is wrong, use ZOT debug settings
			ReadFromFactory(&DEFAULT_Data); //Ron Add 8/30/04			
			LoadDefaultEEPROM = 1;
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
	   
	if( LoadDefaultEEPROM )
	{
		// use default EEPROM settings
		memcpy( &EEPROM_Data, &DEFAULT_Data, sizeof(EEPROM) );

		NeedWriteEEPROM = 1;
	}
	
//code1 not need write eeprom
//	if(NeedWriteEEPROM) {
//		if(WriteToEEPROM(&EEPROM_Data) != 0);	// Write EEPROM Data
//			ErrLightOnForever(Status_Lite); // Write EEPROM Data error
//	}
		
	/* If NODE ID is Default, get random NODE ID ..... Ron 4/3/2003 */
	if( memcmp(DEFAULT_Data.EthernetID,DefaultNodeID,6) == 0 
		|| memcmp(EEPROM_Data.EthernetID,DefaultNodeID,6) == 0 )
	{
		get_randnodeid(EEPROM_Data.EthernetID);
		if (EEPROM_Data.EthernetID[2] == RANDID_SIGNATURE)
		{

			memcpy(MyPhysNodeAddress,EEPROM_Data.EthernetID, 6);
		}
	}
		
	memcpy(Hostname,_BoxName,LENGTH_OF_BOX_NAME); //Jesse

	memcpy(MyPhysNodeAddress,EEPROM_Data.EthernetID, 6); // Copy Phys Address
	PSMode = EEPROM_Data.PrintServerMode; //8/3/99 move from below
	PSMode |= (PS_UNIX_MODE | PS_WINDOWS_MODE); //Unix and Windows always on
	
	//Light_Flash(2,1); //EEPROM Initial OK	
//ZOT716u2		Light_ALL_Flash(2,1); //EEPROM Initial OK
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