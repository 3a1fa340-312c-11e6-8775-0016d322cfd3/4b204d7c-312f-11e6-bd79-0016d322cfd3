// This module offers functions prototype all subroutines need
// add --- by arius 3/20/2000

#include <stdio.h>
//#include "ctypes.h"
#include "menu.h"

#ifndef _INC_TELUTIL_H_
#define _INC_TELUTIL_H_

// define length
#define LENGTH_OF_USERNAME      48 // string (include zero char)
#define LENGTH_OF_PASSWORD       8 // string (include zero char)
#define LENGTH_OF_PS_NAMES      48 // string (include zero char)
#define LENGTH_OF_MULTI_CHOICE  10 // string (include zero char)
#define LENGTH_OF_BUFFER        30
#define LENGTH_OF_POLLING_TIME  10 // string (include zero char)
#define LENGTH_OF_IPADDRESS    100 // string (include zero char)
// #define TELNETBEEP             '' // 0x07
#define TELNETSPACE            ' '
#define TELNETSTAR             '*'


// We keep one copy of EEPROM Data,since user will usually change it.
// After User press "save", then program will use data we keep to restore to box.

extern WORD   OffsetOfFSNameInTelnet[MAX_FS]; //Offset of FS name from EEPROM.FileServerNames

#define _PrinterSpeedInTelnet      (TELNET_EEPROM_Data.PrinterSpeed)
#define _BoxIPAddressInTelnet      (TELNET_EEPROM_Data.BoxIPAddress)
#define _BoxSubNetMaskInTelnet     (TELNET_EEPROM_Data.SubNetMask)
#define _BoxGatewayAddressInTelnet (TELNET_EEPROM_Data.GetwayAddress)
#define _PrintServerNameInTelnet   (TELNET_EEPROM_Data.PrintServerName)
#define _BoxNameInTelnet           (TELNET_EEPROM_Data.BoxName)
#define _FileServerNameInTelnet(i) (TELNET_EEPROM_Data.FileServerNames+OffsetOfFSNameInTelnet[i])

#define _SetupPasswordInTelnet     (TELNET_EEPROM_Data.Password)

#define _NDSTreeNameInTelnet       (TELNET_EEPROM_Data.NDSTreeName)
#define _NDSContextInTelnet        (TELNET_EEPROM_Data.NDSContext)
#define _NovellPasswordInTelnet    (TELNET_EEPROM_Data.NovellPassword)

int32 aton(register char *s);
//Jesse char *inet_ntoa(int32 a);
void doMenu(FILE *fp, TMENU *menu);
void ShowItems(FILE *fp, TMENU *menu);
int  GetInputString(FILE *fp, char* buf, int len, int echo);
int  GetCharFromString(FILE *fp, int echo);
int  InputString(FILE *fp, char *str, int len);
int  InputPassword(FILE *fp, char *str, int len);
int  UsersLogin(char *name, char *pass);

#endif
