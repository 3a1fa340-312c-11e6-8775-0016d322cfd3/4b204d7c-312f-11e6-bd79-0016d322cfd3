// This module offers structures and consts for all needs menu
// add --- by arius 3/23/2000

#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "httpd.h"

//from httpd.c
extern ZOT_FILE* zot_fopen( int handle );
extern int zot_fclose( ZOT_FILE* fp);
extern char *zot_fgets(char *buf,int len, ZOT_FILE *fp);
extern int zot_fputs (int8 *buf ,ZOT_FILE *fp );
#define fdopen(x,y)		zot_fopen(x)
#define fclose(x) 		zot_fclose(x)
#define fgets(x,y,z)	zot_fgets(x,y,z)
#define	fputs(x,y) 		zot_fputs(x,y)
#define FILE 			ZOT_FILE
extern int availmem();
extern int	sendack(int s);

#include "menu.h"
#include "option.h"

// define System Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems1[] = {
    {"\r\n** System Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   Device Name :  ", PutDeviceName, NULL, MSG, 0, NULL},
    {"   Contact     :  ", PutContact, NULL, MSG, 0, NULL},
    {"   Location    :  ", PutLocation, NULL, MSG, 0, NULL},
    {"   UpTime      :  ", PutUpTime, NULL, MSG, 0, NULL},
    {"   Version     :  ", PutVersion, NULL, MSG, 0, NULL},
    {"   Node ID     :  ", PutNodeID, NULL, MSG, 0, NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU viewmenu1 = { viewitems1, 9 };


// define Printer Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems2[] = {
    {"\r\n ** Printer Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
#ifdef DEF_IEEE1284
    {"   Port1:  \r\n", PutIEEE1284PrinterData, NULL, MSG, 0, NULL},
    {"   Printer Status: ", PutPrinterStatus, NULL, MSG, 0, NULL},
#endif DEF_IEEE1284
#ifdef DEF_PRINTSPEED
    {"   Printer Speed  : ", PutSpeed, NULL, MSG, 0, NULL},
//  {"   Printer Status : ", PutPrinterStatus, NULL, MSG, 0, NULL},
#endif DEF_PRINTSPEED
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};

TMENU viewmenu2 = { viewitems2, sizeof(viewitems2)/sizeof(ITEM) };


// define NetWare Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems3[] = {
    {"\r\n** NetWare Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
#ifdef NDS_PS
    {"   Print Server Name    :  ", PutPrintServerName, NULL, MSG, 0, NULL},
    {"   Polling Time         :  ", PutPollingTime, NULL, MSG, 0, NULL},
    {"   NetWare NDS Mode     :  ", PutNDSStatus, NULL, MSG, 0, NULL},
    {"        NDS Tree Name   :  ", PutNDSTreeName, NULL, MSG, 0, NULL},
    {"        NDS Context     :  ", PutNDSContext, NULL, MSG, 0, NULL},
    {"        Connect Status  :  ", PutNDSConnectStatus, NULL, MSG, 0, NULL},
    {"   NetWare Bindery Mode :  ", PutNetwareStatus, NULL, MSG, 0, NULL},
    {"        File Server Name:  ", PutFileServerName, NULL, MSG, 0, NULL},
    {"        Connect Status  :  ", PutConnectStatus, NULL, MSG, 0, NULL},
#else
    {"   NetWare ", PutNetwareStatus, NULL, MSG, 0, NULL},
    {"     Print Server Name  : ", PutPrintServerName, NULL, MSG, 0, NULL},
    {"     File Server Name   : ", PutFileServerName, NULL, MSG, 0, NULL},
    {"     Polling Time       : ", PutPollingTime, NULL, MSG, 0, NULL},
    {"     Connect Status     : ", PutConnectStatus, NULL, MSG, 0, NULL},
#endif NDS_PS
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};

TMENU viewmenu3 = { viewitems3, sizeof(viewitems3)/sizeof(ITEM) };

// define TCP/IP Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems4[] = {
    {"\r\n** TCP/IP Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   DHCP/BOOTP  : ", PutDhcpAndBootp, NULL, MSG, 0, NULL},
    {"   IP Address  : ", PutIPAddress, NULL, MSG, 0, NULL},
    {"   Subnet Mask : ", PutSubnetMask, NULL, MSG, 0, NULL},
    {"   Gateway     : ", PutGateway, NULL, MSG, 0, NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU viewmenu4 = { viewitems4, 7 };


// define SNMP Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems5[] = {
    {"\r\n** SNMP Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   SNMP Communities: \r\n", NULL, NULL, MSG, 0, NULL},
    {"        Community 1: ", PutCommunity1, NULL, MSG, 0, NULL},
    {"        Community 2: ", PutCommunity2, NULL, MSG, 0, NULL},
    {"   SNMP Traps:   ", PutSNNPTraps, NULL, MSG, 0, NULL},
    {"        Authentication Traps : ", PutAuthenTraps, NULL, MSG, 0, NULL},
    {"                    Trap 1 IP: ", PutTrap1, NULL, MSG, 0, NULL},
    {"                    Trap 2 IP: ", PutTrap2, NULL, MSG, 0, NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU viewmenu5 = { viewitems5, 10 };


#ifdef ATALKD
// define APPLETALK Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems6[] = {
    {"\r\n** Apple Talk Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   Zone Name : ", PutCurrentZoneName, NULL, MSG, 0, NULL},
    {"   Port1:\r\n", PutChooserName, NULL, MSG, 0, NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU viewmenu6 = { viewitems6, 5 };
#endif ATALKD

#ifdef WIRELESS_CARD
// define WIRELESS Memu user chose on "Look at status in Printer Server" menu
ITEM viewitems7[] = {
    {"\r\n** Wireless Information **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   Mode    : ", PutWLANMode, NULL, MSG, 0, NULL},
    {"   ESSID   : ", PutWLANESSID, NULL, MSG, 0, NULL},
    {"   Channel : ", PutWLANChannel, NULL, MSG, 0, NULL},
    {"   WEP Type: ", PutWEPkeyMode, NULL, MSG, 0, NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU viewmenu7 = { viewitems7, 7 };
#endif WIRELESS_CARD

// define "Look at status in Printer Server" menu user chose on main menu
ITEM viewitems[] = {
    {"\r\n** View Data in the Printer Server **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. System   \r\n", NULL, NULL, SUBMENU, '1', &viewmenu1},
    {"   2. Printer  \r\n", NULL, NULL, SUBMENU, '2', &viewmenu2},
    {"   3. Netware  \r\n", NULL, NULL, SUBMENU, '3', &viewmenu3},
    {"   4. TCP/IP   \r\n", NULL, NULL, SUBMENU, '4', &viewmenu4},
    {"   5. SNMP     \r\n", NULL, NULL, SUBMENU, '5', &viewmenu5},
#ifdef ATALKD
    {"   6. APPLETALK\r\n", NULL, NULL, SUBMENU, '6', &viewmenu6},
#endif ATALKD
#ifdef WIRELESS_CARD
    {"   7. Wireless\r\n", NULL, NULL, SUBMENU, '7', &viewmenu7},
#endif WIRELESS_CARD
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU viewmenu = { viewitems, sizeof(viewitems)/sizeof(ITEM) };

// define System Memu user chose on "Setting value in Printer Server" menu
ITEM setitems1[] = {
    {"\r\n** System Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. Device Name    : ", PutDeviceName, GetDeviceName, 0, '1', NULL},
    {"   2. Contact        : ", PutContact, GetContact, 0, '2', NULL},
    {"   3. Location       : ", PutLocation, GetLocation, 0, '3', NULL},
    {"   4. Change Password: ", PutPassword, GetPassword, 0, '4', NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu1 = { setitems1, 7 };


// define Printer Memu user chose on "Setting value in Printer Server" menu
ITEM setitems2[] = {
    {"\r\n** Printer Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
#ifndef NO_SUPPORT_BIDIR
#ifdef DEF_IEEE1284
    {"   1. Port1 Bi-directional :  ", PutBiDirect1, GetBiDirect1, 0, '1', NULL},
#if (NUM_OF_PRN_PORT != 1)
    {"   2. Port2 Bi-directional :  ", PutBiDirect2, GetBiDirect2, 0, '2', NULL},
    {"   3. Port3 Bi-directional :  ", PutBiDirect3, GetBiDirect3, 0, '3', NULL},
#endif
#endif DEF_IEEE1284
#endif NO_SUPPORT_BIDIR
#ifdef DEF_PRINTSPEED
    {"   4. Print Speed :  ", PutSpeed, GetSpeed, 0, '4', NULL},
#endif DEF_PRINTSPEED

    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu2 = { setitems2, sizeof(setitems2)/sizeof(ITEM)};


// define NetWare Memu user chose on "Setting value in Printer Server" menu
ITEM setitems3[] = {
    {"\r\n** Netware Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
#ifdef NDS_PS
    {"   1. Print Server Name   :  ", PutPrintServerName, GetPrintServerName, 0, '1', NULL},
    {"   2. Polling Time        :  ", PutPollingTime, GetPollingTime, 0, '2', NULL},
    {"   3. Encrypted Password  :  ", PutNovellPassword, GetNovellPassword, 0, '3', NULL},
    {"   4. NetWare NDS Mode    :  ", PutNDSStatus, GetNDSStatus, 0, '4', NULL},
    {"   5.   NDS Tree Name     :  ", PutNDSTreeName, GetNDSTreeName, 0, '5', NULL},
    {"   6.   NDS Context       :  ", PutNDSContext, GetNDSContext, 0, '6', NULL},
    {"   7. NetWare Bindery Mode:  ", PutNetwareStatus, GetNetwareStatus, 0, '7', NULL},
    {"   8.   File Server Name  :  ", PutFileServerName, GetFileServerName, 0, '8', NULL},
#else
    {"   1. NetWare Setting    :   ", PutNetwareStatus, GetNetwareStatus, 0, '1', NULL},
    {"   2. Print Server Name  :   ", PutPrintServerName, GetPrintServerName, 0, '2', NULL},
    {"   3. File Server Name   :   ", PutFileServerName, GetFileServerName, 0, '3', NULL},
    {"   4. Polling Time       :   ", PutPollingTime, GetPollingTime, 0, '4', NULL},
#endif NDS_PS
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};

TMENU setmenu3 = { setitems3, sizeof(setitems3)/sizeof(ITEM) };

// define TCP/IP Memu user chose on "Setting value in Printer Server" menu
ITEM setitems4_1[] = {
    {"\r\n** Specify IP **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. IP Address  :    ", PutIPAddress, GetIPAddress, 0, '1', NULL},
    {"   2. Subnet Mask :    ", PutSubnetMask, GetSubnetMask, 0, '2', NULL},
    {"   3. Gateway     :    ", PutGateway, GetGateway, 0, '3', NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu4_1 = { setitems4_1, 6 };

ITEM setitems4[] = {
    {"\r\n** TCP/IP Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. DHCP/BOOTP  : ", PutDhcpAndBootp, GetDhcpAndBootp, 0, '1', NULL},
    {"   2. Specify IP   \r\n", NULL, NULL, SUBMENU, '2', &setmenu4_1},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu4 = { setitems4, 5 };



// define SNMP Memu user chose on "Setting value in Printer Server" menu
ITEM setitems5_1[] = {
    {"\r\n** SNMP Communities **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. Community 1 :  ", PutCommunity1, GetCommunity1, 0, '1', NULL},
    {"   2. Community 2 :  ", PutCommunity2, GetCommunity2, 0, '2', NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu5_1 = { setitems5_1, 5 };

ITEM setitems5_2[] = {
    {"\r\n** SNMP Traps **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. SNMP Traps           :   ", PutSNNPTraps, GetSNNPTraps, 0, '1', NULL},
    {"   2. Authentication Traps :   ", PutAuthenTraps, GetAuthenTraps, 0, '2', NULL},
    {"   3. Trap 1 IP            :   ", PutTrap1, GetTrap1, 0, '3', NULL},
    {"   4. Trap 2 IP            :   ", PutTrap2, GetTrap2, 0, '4', NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu5_2 = { setitems5_2, 7 };

ITEM setitems5[] = {
    {"\r\n** SNMP Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. SNMP Communities  \r\n", NULL, NULL, SUBMENU, '1', &setmenu5_1},
    {"   2. SNMP Traps        \r\n", NULL, NULL, SUBMENU, '2', &setmenu5_2},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu5 = { setitems5, 5 };

#ifdef ATALKD
// define APPLETALK Memu user chose on "Setting value in Printer Server" menu
ITEM setitems6[] = {
    {"\r\n** Apple Talk Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. Zone Name         : ", PutZoneName, GetZoneName, 0, '1', NULL},
    {"   2. Printer Port Name : ", PutApplePortName, GetApplePortName, 0, '2', NULL},
    {"   3. Port1 Printer Type: ", PutPrinterType1, GetPrinterType1, 0, '3', NULL},
    {"   4.        Data Format: ", PutDataFormat1, GetDataFormat1, 0, '4', NULL},
#if (NUM_OF_PRN_PORT != 1)
    {"   5. Port2 Printer Type: ", PutPrinterType2, GetPrinterType2, 0, '5', NULL},
    {"   6.        Data Format: ", PutDataFormat2, GetDataFormat2, 0, '6', NULL},
    {"   7. Port3 Printer Type: ", PutPrinterType3, GetPrinterType3, 0, '7', NULL},
    {"   8.        Data Format: ", PutDataFormat3, GetDataFormat3, 0, '8', NULL},
#endif
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu6 = { setitems6, sizeof(setitems6)/sizeof(ITEM) };

#endif ATALKD

#ifdef WIRELESS_CARD
// define Wireless WEP Memu user chose on "Setting value in Printer Server" menu
ITEM setwepitems[] = {
    {"\r\n** WEP Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. WEP Type      : ", PutWEPkeyMode, GetWEPkeyMode, 0, '1', NULL},
    {"   2. WEP Keyformat : ", PutWEPkeyFormat, GetWEPkeyFormat, 0, '2', NULL},
    {"   3. WEP Key       : ", PutWEPkey, GetWEPkey, 0, '3', NULL},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setwepmenu = { setwepitems, sizeof(setwepitems)/sizeof(ITEM) };

// define Wireless Memu user chose on "Setting value in Printer Server" menu
ITEM setitems7[] = {
    {"\r\n** Wireless Setup **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. Mode        : ", PutWLANMode, GetWLANMode, 0, '1', NULL},
    {"   2. ESSID       : ", PutWLANESSID, GetWLANESSID, 0, '2', NULL},
    {"   3. Channel     : ", PutWLANChannel, GetWLANChannel, 0, '3', NULL},
    {"   4. WEP Setting : ", PutWEPkeyMode, NULL, SUBMENU, '4', &setwepmenu},
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};
TMENU setmenu7 = { setitems7, sizeof(setitems7)/sizeof(ITEM) };

#endif WIRELESS_CARD



// define "Setting value in Printer Server" menu user chose on main menu
ITEM setitems[] = {
    {"\r\n** Setting Data into the Printer Server **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"   1. System   \r\n", NULL, NULL, SUBMENU, '1', &setmenu1},
#if (NUM_OF_1284_PORT > 0)
    {"   2. Printer  \r\n", NULL, NULL, SUBMENU, '2', &setmenu2},
#endif
    {"   3. Netware  \r\n", NULL, NULL, SUBMENU, '3', &setmenu3},
    {"   4. TCP/IP   \r\n", NULL, NULL, SUBMENU, '4', &setmenu4},
    {"   5. SNMP     \r\n", NULL, NULL, SUBMENU, '5', &setmenu5},
#if defined(ATALKD) && defined(WIRELESS_CARD)
    {"   6. APPLETALK\r\n", NULL, NULL, SUBMENU, '6', &setmenu6},
    {"   7. Wireless\r\n", NULL, NULL, SUBMENU, '7', &setmenu7},
    {"   8. Save And Reset\r\n", NULL, SaveDataToPrint, 0, '8', NULL},
#elif defined(ATALKD) && !defined(WIRELESS_CARD)
    {"   6. APPLETALK\r\n", NULL, NULL, SUBMENU, '6', &setmenu6},
    {"   7. Save And Reset\r\n", NULL, SaveDataToPrint, 0, '7', NULL},
#elif !defined(ATALKD) && defined(WIRELESS_CARD)
    {"   6. Wireless\r\n", NULL, NULL, SUBMENU, '6', &setmenu7},
    {"   7. Save And Reset\r\n", NULL, SaveDataToPrint, 0, '7', NULL},
#else
    {"   6. Save And Reset\r\n", NULL, SaveDataToPrint, 0, '6', NULL},
#endif
    {"   0. Back To Last Menu\r\n\r\n",     NULL, NULL, CLOSE, '0', NULL},
    {"Enter your choice ->", NULL, NULL, MSG, 0, NULL}
};

TMENU setmenu = { setitems, sizeof(setitems)/sizeof(ITEM) };


// define main menu
ITEM mainitems[] = {
    {"\r\n** Main Menu on Printer Server **\r\n\r\n", NULL, NULL, MSG, 0, NULL},
    {"  1. Look at status in Print Server\r\n", NULL, NULL, SUBMENU, '1', &viewmenu},
    {"  2. Setting value in Print Server\r\n", NULL, NULL, SUBMENU, '2', &setmenu},
    {"  3. Load Default\r\n", NULL, LoadDefault, 0, '3', NULL},
    {"  4. Reset Print Server\r\n", NULL, ResetPrinter, 0, '4', NULL},
    {"  0. Exit Setup\r\n", NULL, ExitSetup, NULL, '0', NULL},
    {"\r\nEnter your choice ->", NULL, NULL, MSG, 0, NULL}
};

const TMENU mainmenu = { mainitems, 7 };


// command
const unsigned char SayOKSuppressGoAhead[]   = {IAC,WILL,SUPPRESSGOAHEAD,0};
const unsigned char NeedClientTerminalType[] = {IAC,SB,TERMTYPE,SEND,IAC,SE,0};
const unsigned char SayOKEchoDisable[]       = {IAC,WONT,ECHO,0};
const unsigned char DoTerminalOption[]       = {IAC,DO,TERMTYPE,0};
const unsigned char WillEchoOption[]         = {IAC,WILL,ECHO,0};
const unsigned char SayOKTerminalType[]      = {IAC,DO,TERMTYPE,0};

// ansi command
const char ClearScreenCommand[] = "%c[2J";

// message
const char LoginBanner[]   = "\nTelnet Service on the PrintServer\n\n\r";
const char LoginError[]    = "\r\n Login incorrect \r\n";
const char CompanyLogo[]   = "ZeroOne\0";
const char HostName[]      = "Arius\0";
const char TelnetVersion[] = "100\0";
const char ADMINUSER[]   = "ADMIN\0";
const char ADMINPASS[]   = "1:2:3:4:\0";

// message's format
const char StringFormat1[]    = "[%s]\r\n";
const char StringFormat2[]    = "%s\r\n";
const char StringFormat3[]    = "%s ";
const char StringFormat4[]    = "[%s] [%s]\r\n";
const char IntegerFormat1[]   = "%d";
const char IntegerFormat2[]   = "%d\r\n";
const char IntegerFormat3[]   = "[%d]\r\n";
const char IpAddressFormat1[] = "%u.%u.%u.%u\r\n";
const char IpAddressFormat2[] = "%s\r\n";
const char HexFormat1[]       = "%2X";

unsigned char CRLF[3]  = {13, 10, 0};
unsigned char CTRLC[3] = {'^', 'D', 0};
unsigned char CTRLD[3] = {'^', 'C', 0};
unsigned char BACK[2]  = {8, 0};
