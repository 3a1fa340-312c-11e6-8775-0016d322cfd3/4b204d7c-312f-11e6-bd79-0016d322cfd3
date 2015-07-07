#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "httpd.h"

#define CONST_DATA

#ifdef CONST_DATA
/*
//hardware\prnqueue.c
const char *PrnUsedMessage[]= {
	 " is available !"                  //0
	," is removing current job !"       //1
	," is used by Windows !"            //2  --+--- Swap 5/13/99
	," is used by NetWare !"            //3  --+
	," is used by Unix (LPD), job#=%s"  //4 9/7/98
	," is used by Mac (ATALK) !"        //5 3/25/99
	," is used by IPPD, job#=%d(%d) !" 	//6 8/8/2000
};
*/
#ifdef HTTPD

// HTTP Header lines sent or processed
#define HDR_TYPE	0
#define HDR_LEN	        1
#define HDR_MIME	2
#define HDR_SERVER	3
#define HDR_LOCATION	4
#define HDR_DATE	5
#define HDR_MODIFIED	6
#define HDR_SINCE	7
#define HDR_REF 	8
#define HDR_AGENT	9
#define HDR_FROM        10
#define HDR_AUTHEN      11
#define HDR_AUTH        12
#define HDR_EXPIRES	    13
#define HDR_PRAGMA      14
#define HDR_TARGET      15

const char *  wHdrs[] = {
	"Content-Type:"
	,"Content-Length:"
	,"MIME-version:"
	,"Server:"
	,"Location:"
	,"Date:"
	,"Last-Modified:"
	,"If-Modified-Since:"
	,"Referer:"
	,"User-Agent:"
    ,"From:"
    ,"WWW-Authenticate:"
    ,"Authorization:"
	,"Expires:"	           //Simon 8/10/98
	,"Pragma: no-cache"	   //Simon 8/10/98
	,"Window-Target: _top" //Simon 8/27/98
#ifdef IPPD
	,"Transfer-Encoding:"  //07/19/2000	   16
#endif IPPD
};

// HTTP response codes

const char *  HttpResp[] = {
#define RESP_200    0
    "200 OK"
#define RESP_302	1
	,"302 Moved Temporarily"
#define RESP_304	2
	,"304 Not Modified"
#define RESP_400    3
    ,"400 Bad Request"
#define RESP_401    4
	,"401 Unauthorized"
#define RESP_404    5
    ,"404 Object Not Found"
#define RESP_500    6
    ,"500 Internal Server Error"
#define RESP_501    7
    ,"501 Not Implemented"
};

const char *  HttpMsg[] = {
#define MSG_302		0
	""
#define MSG_400     1
	,"<TITLE>BAD</TITLE><H1>400 Bad Request</H1>Reason: %s."
#define MSG_401     2
    ,"<TITLE>AUTH</TITLE><H1>401 Unauthorized.</H1>"
#define MSG_404     3
	,"<TITLE>ERROR</TITLE><H1>404 Object Not Found.</H1>"
#define MSG_500     4
	,"<TITLE>ERROR</TITLE><H1>500 Internal Server Error</H1>%s."
#define MSG_501     5
	,"<TITLE>ERROR</TITLE><H1>501 Not Implemented</H1>Method \"%s\" is not supported."
};

const char *HttpErrMsg[] = {
#define HTTP_INVALID_REQUEST      HttpErrMsg[0]
	"Invalid HTTP/0.9 Request",
#define HTTP_INVALID_URL          HttpErrMsg[1]
	"Invalid URI for 'HEAD' or 'POST'",
#define	HTTP_INVALID_POST_CONTENT HttpErrMsg[2]
	"POST request Content-Length"

#ifdef IPPD
#define HTTP_INVALID_METHOD       HttpErrMsg[3]
	,"IPP only accept POST method"
#endif IPPD
};

const struct FileTypes  HTTPtypes[] = {
#define    T_htm    0
	{ "htm", "text/html"  },
#define    T_gif    1
	{ "gif", "image/gif"  },
#define    T_jpg    2
	{ "jpg", "image/gif"  },
#define    T_css	3
 	{ "css", "text/plain" },
#define    T_js		4
 	{ "js", "text/plain" },
#define	   T_binary 5
	{ "bin", "application/octet-stream" },
#define    T_plain  6
	{ NULL , "text/plain" },
#define	   T_form	7
	{ NULL , "multipart/form-data"},
#ifdef IPPD
#define    T_ipp    8
	{ NULL , "application/ipp"},
#endif IPPD
	{ NULL ,	 NULL     }
};

const char *EchoName[] = {
#define ECHO_NODE_ID            0
	"NodeID"
#define ECHO_VERSION            1
	,"Version"
#define ECHO_NETWARE_CONNECT    2
	,"NetWareConnect"
//port1 - por3 item must continuous //
#define ECHO_PORT1_STATUS       3
	,"Port1"
#define ECHO_PORT2_STATUS       4
	,"Port2"
#define ECHO_PORT3_STATUS       5
	,"Port3"

#define ECHO_DHCP_STATUS		6
	,"CurDHCP"
//CurIP, CurSubNet,	CurGateway must continuous
#define ECHO_IP_STATUS          7
	,"CurIP"
#define ECHO_SUBNET_STATUS      8
	,"CurSubNet"
#define ECHO_GATEWAY_STATUS     9
	,"CurGateway"

#define ECHO_NETWARE_MODE       10
	,"CurNetWareMode"
#define ECHO_FSNAME_STATUS      11
	,"CurFSName"
#define ECHO_HOME_ADDR          12
	,"HomeAddr"
#define ECHO_ERROR_MSG          13
	,"ErrorMsg"
//////////////////////////////////////// also for CGI function ////////
#define ECHO_BOX_NAME           14
	,"BoxName"
#define ECHO_SET_DHCP           15
	,"DHCP"
#define ECHO_SET_NOT_DHCP       16
	,"NotDHCP"

// IP, SubNet, Gateway must continuous
#define ECHO_SET_IP             17
	,"IP"
#define ECHO_SET_SUBNET         18
	,"Subnet"
#define ECHO_SET_GATEWAY        19
	,"Gateway"

#define ECHO_SET_NETWARE_MODE   20
	,"NetWareMode"
#define ECHO_PRINTSERVER_NAME   21
	,"PSName"
#define ECHO_SET_FSNAME         22
	,"FSName"
#define ECHO_POLLING_TIME       23	 //for N6300II only
	,"PollTime"
#define ECHO_PORT_SPEED		    24	 //for N6300II only
    ,"CurSpeed"
#define ECHO_SET_PORT_SPEED		25
	,"PortSpeed"
//////////////////////////////////////////////////////////////////////////
#define ECHO_BOX_UPTIME         26
	,"BoxUpTime"
#define ECHO_SNMP_SYS_CONTACT   27	    //snmp
	,"SnmpSysContact"
#define ECHO_SNMP_SYS_LOCATION  28	    //snmp
	,"SnmpSysLocation"
#define ECHO_SNMP_COMMUNITY1    29	    //snmp
	,"SnmpCommunity1"
#define ECHO_SNMP_COMMUNITY2    30      //snmp
	,"SnmpCommunity2"
#define ECHO_SNMP_COMM1_ACCESS  31      //snmp
	,"SnmpComm1Access"
#define ECHO_SNMP_COMM2_ACCESS  32      //snmp
	,"SnmpComm2Access"
#define ECHO_SNMP_TRAP_ENABLE   33      //snmp
	,"SnmpTrapEnable"
#define ECHO_SNMP_AUTH_TRAP_ENABLE 34   //snmp
	,"SnmpAuthTrapEnable"
#define ECHO_SNMP_TRAP_IP1      35      //snmp
	,"SnmpTrapIP1"
#define ECHO_SNMP_TRAP_IP2      36      //snmp
	,"SnmpTrapIP2"
#define ECHO_SNMP_SET_COMM1_ACCESS 37   //snmp
	,"SnmpSetComm1Access"
#define ECHO_SNMP_SET_COMM2_ACCESS 38   //snmp
	,"SnmpSetComm2Access"
#define ECHO_SNMP_SET_TRAP      39      //snmp
	,"SnmpSetTrapEnable"
#define ECHO_SNMP_SET_AUTH_TRAP 40		//snmp
	,"SnmpSetAuthTrapEnable"

//////////  !!!!    ATALKD     !!!!  ////////// 4/6/99
#define ECHO_ATALK_ZONE_NAME    41
	,"AtalkZoneName"
#define ECHO_ATALK_PORT1_NAME   42
	,"AtalkPort1Name"
#define ECHO_ATALK_PORT2_NAME   43
	,"AtalkPort2Name"
#define ECHO_ATALK_PORT3_NAME   44
	,"AtalkPort3Name"
#define ECHO_ATALK_SET_ZONE_NAME 45
	,"AtalkSetZoneName"
#define ECHO_ATALK_NET_ADDR      46
	,"AtalkNetAddr"
#define ECHO_ATALK_SET_PORT_NAME 47
	,"AtalkSetPortName"
//////////  !!!!    ATALKD     !!!!  //////////

//////////////////////for DEBUG ONLY /////////////////////////////////////
#define ECHO_SET_TIMEOUT_VALUE  48	  //1/25/99
    ,"TimeOutValue"
#define ECHO_SET_NT_MAX_PACKET  49	  //5/18/99
    ,"NTMaxRecvPacket"
#define ECHO_SET_TEST_MODE      50	  //5/19/99
    ,"QTestMode"
#define ECHO_VERSION_STATUS     51
	,"VersionStatus"
#define ECHO_MEMORY_STATUS      52
	,"MemoryStatus"
#define ECHO_PRINT_STATUS		53
	,"PrintStatus"
#define ECHO_PRINT_MODE         54
	,"PrintMode"
#define ECHO_QUEUE_STATUS       55
	,"QueueStatus"
#define ECHO_BUFFER_STATUS      56
	,"BufferStatus"
#define ECHO_ERROR_STATUS       57
	,"ErrorStatus"
#define ECHO_MEDIA_MODE         58
	,"CurMediaMode"
//////////////////////////////////////////////////////////////////////////
///// NDS //////////////////////////////
#define ECHO_NDS_MODE          59
	,"CurNDSMode"
#define ECHO_SET_NDS_MODE      60
	,"SetNDSMode"
#define ECHO_NETWARE_PASSWORD  61
	,"NetwarePassword"
#define ECHO_NDS_TREE_NAME     62
	,"NDSTreeName"
#define ECHO_NDS_CONTEXT       63
	,"NDSContext"
#define ECHO_NDS_CONNECT       64
	,"NDSConnect"
#define ECHO_SET_NDS_TREE_NAME 65
	,"SetNDSTreeName"
#define ECHO_SET_SETUP_PASSWD  66
	,"SetupPWD"
#define ECHO_CONFIRM_PASSWD    67
	,"ConfirmPWD"
#define ECHO_PASSWD_SEED       68
	,"PWDSeed"
///// NDS //////////////////////////////
//////IEEE1284 DEVICE ID ///////////////
#define ECHO_PORT1_DEVICE_ID   69
	,"Port1MDL"
#define ECHO_PORT2_DEVICE_ID   70
	,"Port2MDL"
#define ECHO_PORT3_DEVICE_ID   71
	,"Port3MDL"
#define ECHO_PORT1_MANUFACTURE 72
	,"Port1MFG"
#define ECHO_PORT2_MANUFACTURE 73
	,"Port2MFG"
#define ECHO_PORT3_MANUFACTURE 74
	,"Port3MFG"
#define ECHO_PORT1_COMMAND_SET 75
	,"Port1CMD"
#define ECHO_PORT2_COMMAND_SET 76
	,"Port2CMD"
#define ECHO_PORT3_COMMAND_SET 77
	,"Port3CMD"
#define ECHO_PORT1_PRINTER_MODE 78
	,"Port1MOD"
#define ECHO_PORT2_PRINTER_MODE 79
	,"Port2MOD"
#define ECHO_PORT3_PRINTER_MODE 80
	,"Port3MOD"
#define ECHO_SET_PORT1_BIMODE   81
    ,"SetP1BiMode"
#define ECHO_SET_PORT2_BIMODE   82
    ,"SetP2BiMode"
#define ECHO_SET_PORT3_BIMODE   83
    ,"SetP3BiMode"
#define ECHO_SET_PORT1_PRN_MODE 84
	,"SetP1MOD"
#define ECHO_SET_PORT2_PRN_MODE 85
	,"SetP2MOD"
#define ECHO_SET_PORT3_PRN_MODE 86
	,"SetP3MOD"
//////IEEE1284 DEVICE ID ///////////////

//////////  !!!!    ATALKD 2   !!!!  ////////// 4/18/2000
#define ECHO_ATALK_PORT1_TYPE   87
	,"AtalkPort1Type"
#define ECHO_ATALK_PORT2_TYPE   88
	,"AtalkPort2Type"
#define ECHO_ATALK_PORT3_TYPE   89
	,"AtalkPort3Type"
#define ECHO_ATALK_PORT1_DATA_FORMAT        90
	,"AtalkPort1DataMode"
#define ECHO_ATALK_PORT2_DATA_FORMAT        91
	,"AtalkPort2DataMode"
#define ECHO_ATALK_PORT3_DATA_FORMAT        92
	,"AtalkPort3DataMode"
#define ECHO_SET_ATALK_PORT1_DATA_FORMAT	93
	,"AtalkSetPort1DataMode"
#define ECHO_SET_ATALK_PORT2_DATA_FORMAT	94
	,"AtalkSetPort2DataMode"
#define ECHO_SET_ATALK_PORT3_DATA_FORMAT	95
	,"AtalkSetPort3DataMode"
//////////////////////////////////////////////

#define ECHO_FLASH_WRITE_COUNT  96
    ,"FlashCount"

///////////IPP Jobs Status //////////////////

#define ECHO_IPP_JOBS	        97
	,"IPPJobs"

/////////// Reset & Reboot //////////////////

#define ECHO_SYS_RESET			98
	,"SysReset"

#define ECHO_SYS_REBOOT			99
	,"SysReboot"

#define ECHO_LOAD_DEFAULT		100
	,"LoadDefault"

#define ECHO_SAVE_EEPROM		101
	,"SaveEEPROM"

/////////// Wireless ////////////////

#define ECHO_WLESSID			102
	,"WLESSID"

#define ECHO_WLCHANNEL			103
	,"WLChannel"

#define ECHO_WLWEP_TYPE			104
	,"WLWEPType"

#define ECHO_WLWEP_KEY_SELECT	105
	,"WLWEPKeySel"

#define ECHO_WLWEP_KEY			106
	,"WLWEPKey"

#define ECHO_WLWEP_KEY1			107
	,"WLWEPKey1"

#define ECHO_WLWEP_KEY2			108
	,"WLWEPKey2"

#define ECHO_WLWEP_KEY3			109
	,"WLWEPKey3"

#define ECHO_WLWEP_KEY4			110
	,"WLWEPKey4"

#define ECHO_WLWEP_128KEY		111
	,"WLWEP128Key"

#define ECHO_WLBEACONINTERVAL	112
	,"WLBeaconinterval"

#define ECHO_WLRTSTHRESHOLD		113
	,"WLRTSThreshold"

#define ECHO_WLFRAGMENTATION	114
	,"WLFragmentation"

#define ECHO_WLRATES			115
	,"WLRates"

#define ECHO_WLRATE				116
	,"WLRate"

#define ECHO_WLSHORT_PREAMBLE	117
	,"WLShortPreamble"

#define ECHO_WLAUTH_TYPE		118
	,"WLAuthType"

#define ECHO_WLDTIMINTERVAL		119
	,"WLDtiminterval"

#define ECHO_WLCFPPERIOD		120
	,"WLCfpperiod"

#define ECHO_WLCFPMAXDURATION	121
	,"WLCfpmaxduration"

#define ECHO_WLCRX				122
	,"WLCRX"

#define ECHO_WLCTX				123
	,"WLCTX"

#define ECHO_WLJAPAN			124
	,"WLJapan"

#define ECHO_WLANSIDE			125
	,"WLAnSide"

#define ECHO_WLCOUNTRY			126
	,"WLCountry"

#define ECHO_WLGENERATE			127
	,"WLGenerate"

#define ECHO_WLMODE				128
	,"WLMode"

#define ECHO_WLAPMODE			129
	,"WLAPMode"

#define ECHO_WLTXPOWER			130
	,"WLTxPower"

#define ECHO_WLZONE				131
	,"WLZone"

#define ECHO_WLCURCHANNEL		132
	,"WLCurChannel"

#define ECHO_WLTXRATE			133
	,"WLTxRate"

#define ECHO_WLVERSION			134
	,"WLVersion"

#define ECHO_WEBJETADMIN		135
	,"WebJetAdmin"

#define ECHO_WORKGROUP   		136
	,"SMBWorkGroup"

#define ECHO_SHAREPRINT1		137
    ,"SMBPrint1"

#define ECHO_SHAREPRINT2		138
    ,"SMBPrint2" 

#define ECHO_SHAREPRINT3		139
	,"SMBPrint3"

#define ECHO_SPY				140
	,"Spy"

#define ECHO_SCANAP				141
	,"SCANAP"

#define ECHO_SHOWESSID			142
	,"SHOWESSID"	

#define ECHO_DIAGNOSTIC			143
	,"DIAGNOSTIC"			
	
#define ECHO_CURRBSSID			144
	,"CURRBSSID"

#define ECHO_WLWEPFormat		145
	,"WLWEPFormat"

#define ECHO_WLNonModulate		146
	,"WLNonModulate"
	
#define ECHO_CURRSSID			147
	,"CURRSSID"	

#define ECHO_PrintTest			148
	,"PrintTest"
	
#define ECHO_MAILALERT			149
         ,"ALert"

#define ECHO_SMTPIP		        150
         ,"SMTPIP" 

#define ECHO_SMTPMAIL			151
	,"SMTPMail"

#define ECHO_MAC_RESET_COUNT	152
	,"MACReCount"

#define ECHO_EAP_TYPE			153
	,"EAPType"
	
#define ECHO_EAP_NAME			154
	,"EAPNAME"
	
#define ECHO_EAP_PASSWORD		155
	,"EAPPASSWORD"

#define ECHO_MT_MODE			156
	,"WLMTMode"

#define ECHO_MT_CHANNEL			157
	,"WLMTChannel"
	
#define ECHO_MT_RATE			158
	,"WLMTRate"
	
#define ECHO_MT_PREAMBLE		159
	,"WLMTPreamble"
	
#define ECHO_MT_LENGTH			160
	,"WLMTLength"
	
#define ECHO_MT_SCRAMBLING		161
	,"WLMTScrambling"
	
#define ECHO_MT_FILTER			162
	,"WLMTFilter"
	
#define ECHO_MT_ANTENNA_RX		163
	,"WLMTAntenna_rx"
	
#define ECHO_MT_ANTENNA_TX		164
	,"WLMTAntenna_tx"							
	
#define ECHO_MT_POWER_LOOP		165
	,"WLMTPower_loop"	
	
#define ECHO_MT_KEY_TYPE		166
	,"WLMTKey_type"	
	
#define ECHO_MT_KEY_LENGTH		167
	,"WLMTKey_length"	
	
#define ECHO_MT_KEY				168
	,"WLMTKey"	
	
#define ECHO_MT_CCAMODE			169
	,"WLMTCCAMode"	
	
#define ECHO_MT_AUTORESPOND		170
	,"WLMTAutorespond"							
	
#define ECHO_CURRRATE			171
	,"CURRRATE"	

#define ECHO_LPRQUEUE1			172
	,"LPRQUEUE1"	

#define ECHO_LPRQUEUE2			173
	,"LPRQUEUE2"	
	
#define ECHO_LPRQUEUE3			174
	,"LPRQUEUE3"	
	
#define ECHO_WPA_PASS			175
	,"WPA_Pass"
#define ECHO_CURJOBLIST			176
	,"CurJOBList"
#define ECHO_JOBLIST			177
	,"JOBList"
#define ECHO_PORT1JOBCOUNT		178
	,"Port1JobCount"
#define ECHO_PORT2JOBCOUNT		179
	,"Port2JobCount"
#define ECHO_PORT3JOBCOUNT		180
	,"Port3JobCount"
#define ECHO_PORT1PAGECOUNT		181
	,"Port1PageCount"
#define ECHO_PORT2PAGECOUNT		182
	,"Port2PageCount"
#define ECHO_PORT3PAGECOUNT		183
	,"Port3PageCount"

#define ECHO_VERSION_SHORT		184
	,"Version_Short"

#define ECHO_RANDVOUS			185
	,"Randvous"
#define ECHO_RANDVOUS_NAME		186
	,"RandvousName"

#define ECHO_ATALKSETTINGS		187
	,"AtalkSettings"

// MAC Filtering	// George Add August 11, 2005	
#define ECHO_MF_ENABLED				188
	,"MF_Enabled"
#define ECHO_MF_DENYALL				189
	,"MF_DenyAll"
#define ECHO_MF_CONTROLLISTSIZE		190
	,"MF_ControlListSize"
#define ECHO_MF_NAME				191
	,"MF_Name"
#define ECHO_MF_MACADDRESS			192
	,"MF_MACAddress"
#define ECHO_MF_CONTROLLIST			193
	,"MF_ControlList"
#define ECHO_MF_CONTROLLISTSTATUS	194
	,"MF_ControlListStatus"


// 636U2PW 802.11bg	// Kevin: Apr.1 2007
#define ECHO_WLRXRSSI				195
	,"WLRxRSSI"	
#define ECHO_WLLINKQUALITY			196
	,"WLLinkQuality"	
#define ECHO_WLTXMODE				197
	,"WLTxMode"		
#define ECHO_WLWEPTYPE				198
	,"WLWEPType"
#define ECHO_WLWPATYPE				199
	,"WLWPAType"	
	
//WEP key form 1 to 4 
#define ECHO_WLWEP_128KEY1			200
	,"WLWEP128Key1"

#define ECHO_WLWEP_128KEY2			201
	,"WLWEP128Key2"
	
#define ECHO_WLWEP_128KEY3			202
	,"WLWEP128Key3"
	
#define ECHO_WLWEP_128KEY4			203
	,"WLWEP128Key4"

#define ECHO_WLBANDWIDTH			204
	,"WLBandWidth"	

#define ECHO_WLDATARATE				205
	,"WLDataRate"		
//////////////////////////////////////////////
	// Max Echo Item < (HLINELEN / 4)
	// please search (while( (HLINELEN/4) <= i)) , LINE 26XX
	,NULL
};


const int8 *MediaMode[] = {
"Unknown",
"10M-Half",
"10M-Full",
"100M-Full",
"100M-Half"
};

const int8 basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


//gmtime.c

const char *Weeks[]   = {  "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
const char *Months[] = { "Jan","Feb","Mar","Apr","May","Jun",
				           "Jul","Aug","Sep","Oct","Nov","Dec" };
const char Days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#endif HTTPD


#endif CONST_DATA
