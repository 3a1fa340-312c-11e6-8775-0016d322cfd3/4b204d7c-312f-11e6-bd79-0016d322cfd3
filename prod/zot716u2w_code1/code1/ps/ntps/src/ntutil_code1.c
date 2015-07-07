#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "ipx.h"
#include "ntps.h"

#define LIST_VIEW_FS_LEN 254
char ANYESS[] = "< ANY >";

extern int diag_flag ;

#define CODE1
#define NDS_PS

//from httpd.c
extern BYTE WebLangVersion; //4/26/2000
extern char Hostname[];

extern int wlan_set_anyssid(char *deststr);

void UtilGetListViewData(LIST_VIEW *BoxInfo)
{
	int16 i;
//Print Monitor -------------------------------------------------------------
	memcpy(_BoxName,BoxInfo->BoxName,LENGTH_OF_BOX_NAME);
	memcpy(Hostname,_BoxName,LENGTH_OF_BOX_NAME);		  //6/28/99
	memcpy(EEPROM_Data.BoxIPAddress,BoxInfo->IP,4);
//System --------------------------------------------------------------------
	EEPROM_Data.PrintServerMode = (EEPROM_Data.PrintServerMode & (PS_DHCP_ON)) | (BoxInfo->ProtocolMode & (uint8)(~PS_DHCP_ON));

	_PrinterSpeed = BoxInfo->PrinterSpeed;
	memcpy(_SetupPassword,BoxInfo->SetupPassword,SETUP_PASSWD_LEN);

//Netware -------------------------------------------------------------------
	EEPROM_Data.PollingTime = BoxInfo->PollingTime;

	memcpy(EEPROM_Data.NovellPassword,BoxInfo->NovellPassword,9);
	memcpy(EEPROM_Data.FileServerNames,BoxInfo->FileServerName,LIST_VIEW_FS_LEN);
	memcpy(_PrintServerName, BoxInfo->PrintServerName,48);

//TCP-IP --------------------------------------------------------------------
	memcpy(EEPROM_Data.SubNetMask,BoxInfo->SubnetMask,4);
	memcpy(EEPROM_Data.GetwayAddress,BoxInfo->GatewayAddress,4);
	EEPROM_Data.PrintServerMode = (EEPROM_Data.PrintServerMode & ((uint8)~(PS_DHCP_ON))) | (BoxInfo->DHCPEnable?PS_DHCP_ON:0x00);

//Snmp ----------------------------------------------------------------------
	memcpy(EEPROM_Data.SnmpSysContact,BoxInfo->SnmpSysContact,SNMP_SYSCONTACT_LEN);
	memcpy(EEPROM_Data.SnmpSysLocation,BoxInfo->SnmpSysLocation,SNMP_SYSLOCATION_LEN);
	memcpy(EEPROM_Data.SnmpCommunityAuthName[0],BoxInfo->SnmpCommunityAuthName[0],SNMP_COMMUNITY_LEN);
	memcpy(EEPROM_Data.SnmpCommunityAuthName[1],BoxInfo->SnmpCommunityAuthName[1],SNMP_COMMUNITY_LEN);
	memcpy(&EEPROM_Data.SnmpAccessFlag,&BoxInfo->SnmpAccessFlag,1);
	memcpy(EEPROM_Data.SnmpTrapTargetIP[0],BoxInfo->SnmpTrapTargetIP[0],4);
	memcpy(EEPROM_Data.SnmpTrapTargetIP[1],BoxInfo->SnmpTrapTargetIP[1],4);

//AppleTalk -----------------------------------------------------------------
	memcpy(EEPROM_Data.ATZoneName,BoxInfo->ATZoneName,ATALK_ZONE_LEN);
	memcpy(EEPROM_Data.ATPortName,BoxInfo->ATPortName,ATALK_PORT_NAME);

	for(i = 0 ; i < NUM_OF_PRN_PORT ; i++) {
		memcpy(EEPROM_Data.ATPortType[i],BoxInfo->ATPortType[i],ATALK_TYPE_LEN);
		EEPROM_Data.ATDataFormat[i]= BoxInfo->ATDataFormat[i];
	}

//NDS -----------------------------------------------------------------------
	memcpy(EEPROM_Data.NDSTreeName, BoxInfo->NDSTreeName,NDS_TREE_LEN);
	memcpy(EEPROM_Data.NDSContext, BoxInfo->NDSContext,NDS_CONTEXT_LEN);

//DEVICE --------------------------------------------------------------------

	EEPROM_Data.Bidirectional[0] = BoxInfo->Bidirectional[0];
	EEPROM_Data.Bidirectional[1] = BoxInfo->Bidirectional[1];
	EEPROM_Data.Bidirectional[2] = BoxInfo->Bidirectional[2];

//Wireless ------------------------------------------------------------------
#if defined(WIRELESS_CARD)	
	//	General
	EEPROM_Data.WLMode = BoxInfo->WLMode;

	if (wlan_set_anyssid(BoxInfo->WLESSID)) //find possible "< ANY >" SSID setting ... Ron 6/24/2003
		memcpy(EEPROM_Data.WLESSID,ANYESS, 32);
	else
		memcpy(EEPROM_Data.WLESSID,BoxInfo->WLESSID, 32);

	EEPROM_Data.WLZone = BoxInfo->WLZone;
	EEPROM_Data.WLChannel = BoxInfo->WLChannel;
	EEPROM_Data.WLRate = BoxInfo->WLRate;
	
	//	Advance  
	EEPROM_Data.WLBeaconinterval = BoxInfo->WLBeaconinterval;
	EEPROM_Data.WLShortPreamble = BoxInfo->WLShortPreamble;
	EEPROM_Data.WLFragmentation = BoxInfo->WLFragmentation;
	EEPROM_Data.WLAuthenticationType = BoxInfo->WLAuthenticationType;	
	
	//	Wep
	EEPROM_Data.WLWEPType = BoxInfo->WLWEPType;
	EEPROM_Data.WLWEPKeyType = BoxInfo->WLWEPKeyType;
	EEPROM_Data.WLWEPKeySel = BoxInfo->WLWEPKeySel;
	memcpy( EEPROM_Data.WLWEPKey1, BoxInfo->WLWEPKey1, sizeof(EEPROM_Data.WLWEPKey1) );
	memcpy( EEPROM_Data.WLWEPKey2, BoxInfo->WLWEPKey2, sizeof(EEPROM_Data.WLWEPKey2) );
	memcpy( EEPROM_Data.WLWEPKey3, BoxInfo->WLWEPKey3, sizeof(EEPROM_Data.WLWEPKey3) );
	memcpy( EEPROM_Data.WLWEPKey4, BoxInfo->WLWEPKey4, sizeof(EEPROM_Data.WLWEPKey4) );
	memcpy( EEPROM_Data.WLWEP128Key, BoxInfo->WLWEP128Key, sizeof(EEPROM_Data.WLWEP128Key) );

#if defined(INCLUDE_AP)
	EEPROM_Data.WLAPMode = BoxInfo->WLAPMode;
#endif
/*Jesse	
#if defined(ISL80211G_EXTRATE)
	EEPROM_Data.WLExtRate = BoxInfo->WLExtRate;
#endif
*/			
#if defined(AUTH_8021X)
//	EEPROM_Data.EAP_Type = BoxInfo->EAP_Type;
//	memcpy( EEPROM_Data.EAP_Name, BoxInfo->EAP_Name, sizeof(EEPROM_Data.EAP_Name) );
//	memcpy( EEPROM_Data.EAP_Password, BoxInfo->EAP_Password, sizeof(EEPROM_Data.EAP_Password) );
#endif

#if defined(WPA_PSK_TKIP)
//	memcpy( EEPROM_Data.wpa_Pass, BoxInfo->wpa_Pass, sizeof(EEPROM_Data.wpa_Pass) );	
#endif
	
#endif

//LPR -----------------------------------------------------------------------
#if defined(LPR_Q_RENAME)
//	for(i = 0 ; i < NUM_OF_PRN_PORT ; i++) {
//		memcpy(EEPROM_Data.LPRQueueName[i],BoxInfo->LPRQueueName[i],LPRQUEUENAME_LENGTH);
//	}
#endif

//EndOfStruct ---------------------------------------------------------------

}



void UtilSetListViewData(LIST_VIEW *BoxInfo)
{
	int16 i;
	
	i=sizeof(LIST_VIEW);

//Print Monitor -------------------------------------------------------------
	memcpy(BoxInfo->NodeID, &MyPhysNodeAddress,6);	   //Node ID
	BoxInfo->Version[0]  = CODE1_MAJOR_VER;
	BoxInfo->Version[1]  = CODE1_MINOR_VER;
	BoxInfo->Version[1]  = (BoxInfo->Version[1] % 10) +
		                   ((BoxInfo->Version[1] / 10) << 4);
	
	strcpy(BoxInfo->BoxName,EEPROM_Data.BoxName);      //Box Name
	
	BoxInfo->Port_Model.NumOfPort = EEPROM_Data.NumOfPort;        //Number of port
	BoxInfo->Port_Model.Model     = CODE1_PS_MODEL;			   //Model

	memcpy(BoxInfo->IP,EEPROM_Data.BoxIPAddress,4);	   //IP Address

//System --------------------------------------------------------------------
	BoxInfo->SystemLen = 32;

	BoxInfo->UtilFlag.Downloadable = 1;
#ifdef DEF_PRINTSPEED
	BoxInfo->UtilFlag.DisablePrintSpeed = 0;
#else
	BoxInfo->UtilFlag.DisablePrintSpeed = 1;
#endif //DEF_PRINTSPEED
	BoxInfo->UtilFlag.SupportDeviceInfo = 1;
	
/* Ron Add */
#ifdef PRINT_DIAGNOSTIC	
	BoxInfo->UtilFlag.Support_DiagnosticMode = 1; 
#else	
	BoxInfo->UtilFlag.Support_DiagnosticMode = 0; 
#endif//PRINT_DIAGNOSTIC	
	BoxInfo->UtilFlag.NoUsed = 0;

	BoxInfo->ProtocolMode = EEPROM_Data.PrintServerMode & ((uint8)~PS_DHCP_ON);
	BoxInfo->PrinterSpeed = _PrinterSpeed; // Printer Speed

#ifndef SECURITY_CONFIG
	memcpy(BoxInfo->SetupPassword,_SetupPassword,SETUP_PASSWD_LEN);
	BoxInfo->SetupPassword[SETUP_PASSWD_LEN] = '\0';
#else
	memset(BoxInfo->SetupPassword, 0, SETUP_PASSWD_LEN);
#endif

	UtilGetVersionString(BoxInfo->VersionString);
#ifdef FULL_FIRMWARE_VERSION
	BoxInfo->VersionString[14] = '\0';
#endif FULL_FIRMWARE_VERSION

#if !defined(O_AXIS)

#ifdef NOVELL_PS
#if defined(O_ELEC)
	BoxInfo->SupportFunction.NetWareBindery = 0;
#else
	BoxInfo->SupportFunction.NetWareBindery = 1;
#endif	// defined(O_ELEC)
#else
	BoxInfo->SupportFunction.NetWareBindery = 0;
#endif NOVELL_PS

#ifdef NDS_PS
#if defined(O_ELEC)
	BoxInfo->SupportFunction.NetWareNDS = 0;
#else
	BoxInfo->SupportFunction.NetWareNDS = 1;
#endif	// defined(O_ELEC)
#else
	BoxInfo->SupportFunction.NetWareNDS = 0;
#endif NDS_PS

#endif //!defined(O_AXIS)

#ifdef WINDOWS_PS
	BoxInfo->SupportFunction.PrintMonitor = 1;
#else
	BoxInfo->SupportFunction.PrintMonitor = 0;
#endif WINDOWS_PS

	BoxInfo->SupportFunction.LPD = 1;

#ifdef ATALKD
#if defined(O_ELEC)
	BoxInfo->SupportFunction.AppleTalk = 0;
#else
	BoxInfo->SupportFunction.AppleTalk = 1;
#endif	// defined(O_ELEC)
#else
	BoxInfo->SupportFunction.AppleTalk = 0;
#endif ATALKD

#ifdef IPPD
	BoxInfo->SupportFunction.IPP = 1;
#else
	BoxInfo->SupportFunction.IPP = 0;
#endif IPPD

#ifdef SMBD
	BoxInfo->SupportFunction.SMB = 1;
#else
	BoxInfo->SupportFunction.SMB = 0;
#endif //SMBD

	BoxInfo->SupportFunction.Tftp = 1;
	BoxInfo->SupportFunction.WEB = 1;

#ifdef TELNETD
#if defined(O_ELEC)
	BoxInfo->SupportFunction.Telnet = 0;
#else
	BoxInfo->SupportFunction.Telnet = 1;
#endif	// defined(O_ELEC)
#else
	BoxInfo->SupportFunction.Telnet = 0;
#endif //TELNETD

#ifdef SNMPD
#if defined(O_ELEC)
	BoxInfo->SupportFunction.SNMP = 0;
#else
	BoxInfo->SupportFunction.SNMP = 1;
#endif	// defined(O_ELEC)
#else
	BoxInfo->SupportFunction.SNMP = 0;
#endif SNMPD

#ifdef WEBADMIN
	BoxInfo->SupportFunction.JetAdmin = 1;
#else
	BoxInfo->SupportFunction.JetAdmin = 0;
#endif WEBADMIN

#ifdef DEF_IEEE1284
	BoxInfo->SupportFunction.Bidirectional = 1;
#else
	BoxInfo->SupportFunction.Bidirectional = 0; 
#endif DEF_IEEE1284

	BoxInfo->SupportFunction.Bidirectional = 0; //Ron add for "why DEF_IEEE1284 can not undefine???"

#if defined(WIRELESS_CARD)
	BoxInfo->SupportFunction.Wireless_sta = 1;
#else
	BoxInfo->SupportFunction.Wireless_sta = 0;
#endif

#if defined(INCLUDE_AP)
	BoxInfo->SupportFunction.Wireless_ap = 1;
#else
	BoxInfo->SupportFunction.Wireless_ap = 0;	
#endif	

#if defined(ISL80211G_EXTRATE)
	BoxInfo->SupportFunction.Wireless_11g = 1;
#else
	BoxInfo->SupportFunction.Wireless_11g = 0;	
#endif
		
#ifdef AUTH_8021X
	BoxInfo->SupportFunction.Wireless_1x = 1;
#else
	BoxInfo->SupportFunction.Wireless_1x = 0;	
#endif

#ifdef LPR_Q_RENAME
	BoxInfo->SupportFunction.LPRQRename = 1;
#else
	BoxInfo->SupportFunction.LPRQRename = 0;	
#endif

#ifdef WPA_PSK_TKIP
	BoxInfo->SupportFunction.Wireless_wpa_psk = 1;
#else
	BoxInfo->SupportFunction.Wireless_wpa_psk = 0;	
#endif

#ifdef PACK_DATA_EXT
	BoxInfo->SupportFunction.Data_Exten = 1;
#else
	BoxInfo->SupportFunction.Data_Exten = 0;
#endif //PACK_DATA_EXT

	BoxInfo->SupportFunction.Wireless_wpa2_psk = 1;
	BoxInfo->SupportFunction.WL_Transmit_Mode = 1;

#ifdef SECURITY_CONFIG
//	BoxInfo->SupportFunction.NoUsed2 = 0;
	BoxInfo->SupportFunction.Security_Config =1;
#endif
	BoxInfo->SupportFunction.NoUsed3 = 0;

//Netware -------------------------------------------------------------------
#ifdef NOVELL_PS
	BoxInfo->NetwareLen = 10;
	BoxInfo->PollingTime = EEPROM_Data.PollingTime;
	memcpy(BoxInfo->NovellPassword,EEPROM_Data.NovellPassword,9);

	BoxInfo->NumOfMaxFileServer = MAX_FS;
	BoxInfo->MaxFileServerLen = LIST_VIEW_FS_LEN;
	memcpy(BoxInfo->FileServerName,EEPROM_Data.FileServerNames,LIST_VIEW_FS_LEN);
	BoxInfo->MaxPrintServerLen = 48;   // Maximum Length of Print Server
	memcpy(BoxInfo->PrintServerName,_PrintServerName,48);
#else
	BoxInfo->NetwareLen = 0;
#endif NOVELL_PS

//TCP-IP --------------------------------------------------------------------
	BoxInfo->TCPIPLen = 9;
	memcpy(BoxInfo->SubnetMask,EEPROM_Data.SubNetMask,4);
	memcpy(BoxInfo->GatewayAddress,EEPROM_Data.GetwayAddress,4);
	BoxInfo->DHCPEnable = (EEPROM_Data.PrintServerMode & 0x80?1:0);

//Snmp ----------------------------------------------------------------------
#ifdef	PRNSEV_MODULE	//9312
	BoxInfo->SnmpLen = 0;
#else
	BoxInfo->SnmpLen = (SNMP_SYSCONTACT_LEN+SNMP_SYSLOCATION_LEN+SNMP_COMMUNITY_LEN*2+1+ 2 * 4);

	memcpy(BoxInfo->SnmpSysContact,EEPROM_Data.SnmpSysContact,SNMP_SYSCONTACT_LEN);
	memcpy(BoxInfo->SnmpSysLocation,EEPROM_Data.SnmpSysLocation,SNMP_SYSLOCATION_LEN);
	memcpy(BoxInfo->SnmpCommunityAuthName[0],EEPROM_Data.SnmpCommunityAuthName[0],SNMP_COMMUNITY_LEN);
	memcpy(BoxInfo->SnmpCommunityAuthName[1],EEPROM_Data.SnmpCommunityAuthName[1],SNMP_COMMUNITY_LEN);
	memcpy(&BoxInfo->SnmpAccessFlag,&EEPROM_Data.SnmpAccessFlag,1);
	memcpy(BoxInfo->SnmpTrapTargetIP[0],EEPROM_Data.SnmpTrapTargetIP[0],4);
	memcpy(BoxInfo->SnmpTrapTargetIP[1],EEPROM_Data.SnmpTrapTargetIP[1],4);
#endif	//PRNSEV_MODULE//

//AppleTalk -----------------------------------------------------------------
#ifdef ATALKD
	BoxInfo->ATalkLen = 3 + 2 * ATALK_ZONE_LEN + ATALK_PORT_NAME + 3 * ATALK_TYPE_LEN + 3;
#if defined(CODE1) || !defined(ATALKD)
	BoxInfo->ATCurNet = 0;
	BoxInfo->ATCurNode = 0;
	strcpy(BoxInfo->ATCurZoneName,"*");
#else
	BoxInfo->ATCurNet = ntohs(at_iface.my.s_net);
	BoxInfo->ATCurNode = at_iface.my.s_node;
	strcpy(BoxInfo->ATCurZoneName,at_iface.zonename);
#endif
	memcpy(BoxInfo->ATZoneName,EEPROM_Data.ATZoneName,ATALK_ZONE_LEN);
	memcpy(BoxInfo->ATPortName,EEPROM_Data.ATPortName,ATALK_PORT_NAME);

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		memcpy(BoxInfo->ATPortType[i],EEPROM_Data.ATPortType[i],ATALK_TYPE_LEN);
		BoxInfo->ATDataFormat[i] = EEPROM_Data.ATDataFormat[i];
	}
#else
	BoxInfo->ATalkLen = 0;
#endif ATALKD

//NDS -----------------------------------------------------------------------
#ifdef NDS_PS	
	BoxInfo->NDSLen = NDS_TREE_LEN;
	memcpy(BoxInfo->NDSTreeName,EEPROM_Data.NDSTreeName, NDS_TREE_LEN);
	BoxInfo->NDSContextLen = NDS_CONTEXT_LEN;
	memcpy(BoxInfo->NDSContext,EEPROM_Data.NDSContext,NDS_CONTEXT_LEN);
#else
	BoxInfo->NDSLen = 0;
#endif NDS_PS

//DEVICE --------------------------------------------------------------------
	BoxInfo->DEVICELen = 3;
	BoxInfo->Bidirectional[0] = EEPROM_Data.Bidirectional[0];
	BoxInfo->Bidirectional[1] = EEPROM_Data.Bidirectional[1];
	BoxInfo->Bidirectional[2] = EEPROM_Data.Bidirectional[2];

//Wireless ------------------------------------------------------------------
//  for PSADMIN 
//	BoxInfo->WirelessLen = 194;
	BoxInfo->WirelessLen = 86;
#if defined(WIRELESS_CARD)
	//	General
	BoxInfo->WLMode = EEPROM_Data.WLMode;
	if (!strcmp(EEPROM_Data.WLESSID, ""))
		strcpy(BoxInfo->WLESSID, ANYESS);
	else
	memcpy(BoxInfo->WLESSID, EEPROM_Data.WLESSID, 32);
    // mvWZone will get from Domain reading from Prism2 Card ...//Ron 7/2/2003		
//	BoxInfo->WLZone = EEPROM_Data.WLZone; //Ron 7/2/2003
//Jesse	BoxInfo->WLZone = mvWZone;
	BoxInfo->WLZone = EEPROM_Data.WLZone;
	BoxInfo->WLChannel = EEPROM_Data.WLChannel;
	BoxInfo->WLRate = EEPROM_Data.WLRate;
	
	//	Advance  
	BoxInfo->WLBeaconinterval = EEPROM_Data.WLBeaconinterval;
	BoxInfo->WLShortPreamble = EEPROM_Data.WLShortPreamble;
	BoxInfo->WLFragmentation = EEPROM_Data.WLFragmentation;
	BoxInfo->WLAuthenticationType = EEPROM_Data.WLAuthenticationType;	
	
	//	Wep
	BoxInfo->WLWEPType = EEPROM_Data.WLWEPType;
	BoxInfo->WLWEPKeySel = EEPROM_Data.WLWEPKeySel;
	memcpy( BoxInfo->WLWEPKey1, EEPROM_Data.WLWEPKey1, sizeof(EEPROM_Data.WLWEPKey1) );
	memcpy( BoxInfo->WLWEPKey2, EEPROM_Data.WLWEPKey2, sizeof(EEPROM_Data.WLWEPKey2) );
	memcpy( BoxInfo->WLWEPKey3, EEPROM_Data.WLWEPKey3, sizeof(EEPROM_Data.WLWEPKey3) );
	memcpy( BoxInfo->WLWEPKey4, EEPROM_Data.WLWEPKey4, sizeof(EEPROM_Data.WLWEPKey4) );
	memcpy( BoxInfo->WLWEP128Key, EEPROM_Data.WLWEP128Key, sizeof(EEPROM_Data.WLWEP128Key) );
	BoxInfo->WLWEPKeyType = EEPROM_Data.WLWEPKeyType;

	BoxInfo->OperationMode =0;

#if defined(INCLUDE_AP)
	BoxInfo->WLAPMode = EEPROM_Data.WLAPMode;
#else
	BoxInfo->WLAPMode = 0;	
#endif	
/*Jesse
#if defined(ISL80211G_EXTRATE)
	BoxInfo->WLExtRate = EEPROM_Data.WLExtRate;
#else
	BoxInfo->WLExtRate = 0;	
#endif
*/		
#if defined(AUTH_8021X)
//	BoxInfo->EAP_Type = EEPROM_Data.EAP_Type;
//	memcpy( BoxInfo->EAP_Name, EEPROM_Data.EAP_Name, sizeof(EEPROM_Data.EAP_Name) );
//	memcpy( BoxInfo->EAP_Password, EEPROM_Data.EAP_Password, sizeof(EEPROM_Data.EAP_Password) );
#endif

#if defined(WPA_PSK_TKIP)
//	memcpy( BoxInfo->wpa_Pass, EEPROM_Data.wpa_Pass, sizeof(EEPROM_Data.wpa_Pass) );	
#endif

#endif	//WIRELESS_CARD

// Port Face ----------------------------------------------------------------
	BoxInfo->PortFaceLen = 3;
	BoxInfo->PORTDESC = EEPROM_Data.PORTDESC;
	BoxInfo->PORTTAIL = EEPROM_Data.PORTTAIL;
	BoxInfo->SPECIAL_OEM = EEPROM_Data.SPECIAL_OEM;

//EndOfStruct ---------------------------------------------------------------
	BoxInfo->EOS = 0xFF;

}

void UtilRemoveBusyStatus(uint8 *PrintStatus)
{
	if((*PrintStatus & 0x03) == 0x03) *PrintStatus &= 0xFC;
	if((*PrintStatus & 0x0C) == 0x0C) *PrintStatus &= 0xF3;
    if((*PrintStatus & 0x30) == 0x30) *PrintStatus &= 0xCF;
}

void UtilGetVersionString(uint8 *Version)
{
	//EEPROM_Data.SPECIAL_OEM               0x02 for D_Link version defined in EEPROM.H
	if(EEPROM_Data.SPECIAL_OEM == 0x02){
		sprintf(Version,"%d.%02x",
			CODE1_MAJOR_VER, CODE1_MINOR_VER);
	}else{
	sprintf(Version,"%d.%02x.%02d",
		CODE1_MAJOR_VER, CODE1_MINOR_VER, CODE1_PS_MODEL);
	}		

}
