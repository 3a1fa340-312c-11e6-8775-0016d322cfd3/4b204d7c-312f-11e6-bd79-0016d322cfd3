// This module offers prototypr of menu's function.
// It will define structure of menu and variables.
// add --- by arius 3/23/2000

#ifndef _INC_MENU_H_
#define _INC_MENU_H_

#define INPUT_ERROR     -1
#define INPUT_NO_CHANGE  0
#define INPUT_OK         2

/* define data type */
#define DATA_STRING (0)
#define DATA_NUMBER (1)
#define DATA_IP     (2)
#define DATA_NODE   (3)

// define const variable
//#define NORMAL    0x0000      Simon
#define GRAY      0x0001
#define DISABLE   0x0002
#define CLOSE     0x0004
#define MSG       0x0008
#define SUBMENU   0x0010

#define DTEMP_LEN 64
#define TEMP_LEN  64
#define STR_LEN   64
#define DSTR_LEN  24

#define FILE 			ZOT_FILE

// prototype
void ExitSetup(FILE *fp);

void PutDeviceName(FILE *fp);
void GetDeviceName(FILE *fp);
void PutContact(FILE *fp);
void GetContact(FILE *fp);
void PutLocation(FILE *fp);
void GetLocation(FILE *fp);
void PutPassword(FILE *fp);
void GetPassword(FILE *fp);

#ifdef DEF_PRINTSPEED
void PutSpeed(FILE *fp);
void GetSpeed(FILE *fp);
#endif DEF_PRINTSPEED

#ifdef DEF_IEEE1284
void PutIEEE1284PrinterData(FILE *fp);
#endif DEF_PRINTSPEED


#ifdef DEF_IEEE1284
void PutBiDirect1(FILE *fp);
void GetBiDirect1(FILE *fp);
#if (NUM_OF_PRN_PORT != 1)
void PutBiDirect2(FILE *fp);
void GetBiDirect2(FILE *fp);
void PutBiDirect3(FILE *fp);
void GetBiDirect3(FILE *fp);
#endif
#endif DEF_IEEE1284


#ifdef NDS_PS
void PutNovellPassword(FILE *fp);
void GetNovellPassword(FILE *fp);
void PutNDSStatus(FILE *fp);
void GetNDSStatus(FILE *fp);
void PutNDSTreeName(FILE *fp);
void GetNDSTreeName(FILE *fp);
void PutNDSContext(FILE *fp);
void GetNDSContext(FILE *fp);
#endif NDS_PS
void PutNetwareStatus(FILE *fp);   // for bindery status
void GetNetwareStatus(FILE *fp);   // for bindery status
void PutPrintServerName(FILE *fp);
void GetPrintServerName(FILE *fp);
void PutFileServerName(FILE *fp);
void GetFileServerName(FILE *fp);
void PutPollingTime(FILE *fp);
void GetPollingTime(FILE *fp);


void PutDhcpAndBootp(FILE *fp);
void GetDhcpAndBootp(FILE *fp);

void PutIPAddress(FILE *fp);
void GetIPAddress(FILE *fp);
void PutSubnetMask(FILE *fp);
void GetSubnetMask(FILE *fp);
void PutGateway(FILE *fp);
void GetGateway(FILE *fp);

void PutCommunity1(FILE *fp);
void GetCommunity1(FILE *fp);
void PutCommunity2(FILE *fp);
void GetCommunity2(FILE *fp);

void PutSNNPTraps(FILE *fp);
void GetSNNPTraps(FILE *fp);
void PutAuthenTraps(FILE *fp);
void GetAuthenTraps(FILE *fp);
void PutTrap1(FILE *fp);
void GetTrap1(FILE *fp);
void PutTrap2(FILE *fp);
void GetTrap2(FILE *fp);

#ifdef ATALKD
void PutZoneName(FILE *fp);        // User modified, but not use it.
void GetZoneName(FILE *fp);
void PutApplePortName(FILE *fp);
void GetApplePortName(FILE *fp);
void PutPrinterType1(FILE *fp);
void GetPrinterType1(FILE *fp);
void PutDataFormat1(FILE *fp);
void GetDataFormat1(FILE *fp);
#if (NUM_OF_PRN_PORT != 1)
void PutPrinterType2(FILE *fp);
void GetPrinterType2(FILE *fp);
void PutPrinterType3(FILE *fp);
void GetPrinterType3(FILE *fp);
void PutDataFormat2(FILE *fp);
void GetDataFormat2(FILE *fp);
void PutDataFormat3(FILE *fp);
void GetDataFormat3(FILE *fp);
#endif
#endif ATALKD

void ResetPrinter(FILE *fp);
void LoadDefault(FILE *fp);

void SaveDataToPrint(FILE *fp);

void PutUpTime(FILE *fp);
void PutVersion(FILE *fp);
void PutNodeID(FILE *fp);
void PutPrinterStatus(FILE *fp);
void PutConnectStatus(FILE *fp);    // for bindery
#ifdef NDS_PS
void PutNDSConnectStatus(FILE *fp);
#endif

#ifdef ATALKD
void PutChooserName(FILE *fp);
void PutCurrentZoneName(FILE *fp);  // Now using zone name
#endif ATALKD

#undef WIRELESS_CARD	//eason 20100210

#ifdef WIRELESS_CARD
void PutWLANMode(FILE *fp);
void PutWLANESSID(FILE *fp);
void PutWLANChannel(FILE *fp);
void PutWEPkeyMode(FILE *fp);
void PutWEPkeyFormat(FILE *fp);
void PutWEPkey(FILE *fp);
void GetWLANMode(FILE *fp);
void GetWLANESSID(FILE *fp);
void GetWLANChannel(FILE *fp);
void GetWEPkeyMode(FILE *fp);
void GetWEPkeyFormat(FILE *fp);
void GetWEPkey(FILE *fp);
#endif WIRELESS_CARD


// define menu sturcture
struct _TMENU;
typedef struct {
        char   *msg;              // show message
        void   (*put)(FILE *fp);  // each item wants the variable from this fuction
        void   (*fn)(FILE *fp);   //
        int    type;              // what kind of menu type
        char   key;               // short key
        struct _TMENU  *submenu;
} ITEM;

typedef struct _TMENU {
        ITEM *item;      // main body of menu
        int  nitem;      // number of items on this menu
} TMENU;

#endif
