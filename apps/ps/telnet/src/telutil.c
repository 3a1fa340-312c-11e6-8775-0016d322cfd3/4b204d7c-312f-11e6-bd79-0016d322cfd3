// This module offers all utilities of telnet, such as keyin checking
// and processing input, menu, and submenu,etc.
// Each item on menu will map to correspond with subroutines.
// add --- by arius 3/16/2000
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ctypes.h"
#include "netuser.h"
#include "usock.h"     // symbol 's_addr'
*/

#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pstarget.h"
#include "psglobal.h"  // structure EEPROM
#include "psdefine.h"
#include "led.h"
#include "ledcode.h"
#include "eeprom.h"

#include "httpd.h"

//from httpd.c
extern ZOT_FILE* zot_fopen( int handle );
extern int zot_fclose( ZOT_FILE* fp);
extern char *zot_fgets(char *buf,int len, ZOT_FILE *fp);
extern int zot_fputs (int8 *buf ,ZOT_FILE *fp );
extern char zot_fgetc(ZOT_FILE *fp);
#define fdopen(x,y)		zot_fopen(x)
#define fclose(x) 		zot_fclose(x)
#define fgets(x,y,z)	zot_fgets(x,y,z)
#define	fgetc(x)		zot_fgetc(x)
#define	fputs(x,y) 		zot_fputs(x,y)
#define	fflush(x)		zot_fflush(x)
#define	Fputc(x,y) 		zot_fputc(x,y)
#define FILE 			ZOT_FILE
extern int availmem();
extern int	sendack(int s);

#ifdef ATALKD
#include "at.h"
#include "atp.h"
#include "pap.h"
#include "file.h"
#include "paprint.h"
#endif ATALKD

#include "option.h"
#include "telutil.h"
#include "menu.h"

#include "prnport.h"
#include "nps.h"

//from prnport.c
extern struct parport PortIO[NUM_OF_PRN_PORT];

typedef struct SNTable
{
  char  Name[LENGTH_OF_FS_NAMES];
  int   Flag;
} ServerNameTable;


WORD   OffsetOfFSNameInTelnet[MAX_FS]; //Offset of FS name from EEPROM.FileServerNames

int    MenuIndex,MenuSel;
int    QuitMenu = 0;

extern int ModifyConfigFlag;
extern int TELNETDUsers;
extern int ServiceFSCountInTelnet;
extern EEPROM  TELNET_EEPROM_Data;
extern int8 *UptimeString(uint32 timeticks, int8 *buf);

#ifdef NDS_PS
extern int8 HasDSExist(BYTE *DSBuf, BYTE *DSName);
#endif NDS_PS

#ifdef NDS_PS
#include "ndsqueue.h"
#else
BYTE	NDSConnectFlag = 0;	//615wu //disable
#endif

// command
extern const unsigned char SayOKSuppressGoAhead[];
extern const unsigned char NeedClientTerminalType[];
extern const unsigned char SayOKEchoDisable[];
extern const unsigned char DoTerminalOption[];
extern const unsigned char WillEchoOption[];
extern const unsigned char SayOKTerminalType[];

// ansi command
extern const char ClearScreenCommand[];

// message
extern const char LoginBanner[];
extern const char CompanyLogo[];
extern const char HostName[];
extern const char TelnetVersion[];
extern const char ADMINUSER[];
extern const char ADMINPASS[];

// message's format
extern const char StringFormat1[];
extern const char StringFormat2[];
extern const char StringFormat3[];
extern const char StringFormat4[];
extern const char IntegerFormat1[];
extern const char IntegerFormat2[];
extern const char IntegerFormat3[];
extern const char IpAddressFormat1[];
extern const char IpAddressFormat2[];
extern const char HexFormat1[];

extern unsigned char CRLF[3];
extern unsigned char CTRLC[3];
extern unsigned char CTRLD[3];
extern unsigned char BACK[2];


// This subroutine will process main menu on the screen when user
// connecting to server will see first menu
// add --- by arius 3/16/2000
void doMenu(FILE *fp, TMENU *menu)
{
   char buffer[10];
   int  select;
   int  strlen = 4;

   buffer[0] =0;

   // It is forever loop.  Just when the user select to end service,
   // the loop will be break.
   while (!QuitMenu)
   {
     // shows each item for choosing the menu
     ShowItems(fp, menu);

     //  wait for user choice the item he wants
     // no.4 parameter is echo fuction
     // TRUE = 1 will do echo
     if (GetInputString(fp, buffer, strlen, TRUE) != INPUT_ERROR)
     {
       buffer[0] = toupper(buffer[0]);
       // Check menu's item key value
       for (select = 0; select <= menu->nitem-2; select++)
       {
         if (buffer[0] == menu->item[select].key)
         {
           MenuSel = select;
           // the program already found it. then execute the function user wants
           if (menu->item[select].fn)
             (*menu->item[select].fn)(fp);

           // If the item user select is submenu, the program call "doMenu" again
           if (menu->item[select].submenu != NULL)
           {
             doMenu(fp, menu->item[select].submenu);
             break;
           }
           else
           {
             // When you choose a item, named "back to last menu".
             if (menu->item[select].type == CLOSE)
               return;
           }
         }
       }   // end of for
     }  // end of if

   } // end of while

   return;
}




// This subroutine will process main menu on the screen when user
// connecting to server will see first menu
// add --- by arius 3/16/2000
void ShowItems(FILE *fp, TMENU *menu)
{
  char  buf[10];
  int   index;

  // send each item on the menu choose by user to end of user.
  // since we start from 0, we should decrease 1 no of items.
  for (index = 0; index <= menu->nitem-1; index++)
  {

     MenuIndex = index;

     // chech whether the item do not show to user or not
     if (menu->item[index].type != DISABLE)
     {
       // Here will actually send items to user
       fputs(menu->item[index].msg,fp);

       // chech whether it will show values or not
       if (menu->item[index].put)
         (*menu->item[index].put)(fp);
     }
     fflush(fp);
  }
}


// return value = (1/0/-1) = ("read error for connect broken"
//   or "CTRL + D"/OK/Press ^C or Input too long)
// This subroutine will process the string user keyin
// add --- by arius 3/16/2000
int GetInputString(FILE *fp, char* buf, int len, int echo)
{
  int   done = 0,index = 0;
  int   data,CRLFCharFlag = 0;

  while (!done)
  {
    // read each character from string
    if ((data = GetCharFromString(fp, echo)) == EOF)
    {
      QuitMenu = 1;
      return INPUT_ERROR;
    }

    switch (data)
    {
// ***** Begin *******
// Modified by ---- Arius     3/21/2000
// since some terminal will send only one character"CR" when you press enter
// key. It will cause some problems in our program.
// Now I modified that when the program see "CR",it will send "CR"+"LF"
// automatically to the client. Then getting LF will be dropped.
      case 10:  // when read "LF" character after receving "CR", give it up.
                // It means the program will not do anything
              break;
      case 13:    /* CR, end of input */
// Setting flag gives a trigger. The program need send two chars, "CR+LF".
              if (index < len)
              {
                buf[index] = 0;
                CRLFCharFlag = 1;
              }
              done = 1;
              break;
// ***** End *******
      case 27:
              buf[0] = 0;
              done = 1;
              break;
      case  8:    /* BackSpace */
              if (index > 0)
              {
                index--;
                if ( echo != 2)
                {
                  Fputc(data, fp);
                  Fputc(TELNETSPACE, fp);
                  Fputc(data, fp);
                }
              }
              else
			  {
			  	data = 0x07;
                Fputc(data, fp);
			  }	
              break;
	  case 0:
	  		// Charles 2001/08/08
			// fix for linux
	  		break;
      default:
              // save data
              if (index < len-1)
                buf[index] = (unsigned char)data;
              else
              {
                if (index == len-1)
                {
                  buf[index] = (unsigned char)data;
                  buf[index+1] = 0;
                  done = 1;
                }
              }
              index++;
    } // end of switch
  }

  // if we set up echo option, we will send data back to clint
  if (echo == 1 && (strlen(buf) || CRLFCharFlag) )
    fputs(CRLF, fp);

//  return buf;
    return 0;
}





// This subroutine will process each character from string user keyin
// add --- by arius 3/16/2000
int GetCharFromString(FILE *fp, int echo)
{
  int  data;
  int  par[10];

loop:
  fflush(fp);

  // It is a counter. aboout 180 seconds
  // After that time, the sysetm will be time
  // out. since user doesn't keyin any data.
//Jesse  kalarm(180000L);

  // read a char from stdin
  if ((data = fgetc(fp)) == EOF)
  {
    // no data will directly go to lable error
//Jesse    kalarm(0L);
    goto error;
  }

  // setting 0, since user already keyin data
//Jesse  kalarm(0L);

  switch (data)
  {
    // Telnet protocol command's structure
    // IAC + Fuction or IAC + Option Code + Option ID
    // Four Option Codes
    //       WILL:  The sender wants to enable the option itself
    //       DO  :  The sender wants the receiver to enable the option
    //       WONT:  The sender wants to disable the option itself
    //       DONT:  The sender wants the receiver to disable the option
    case IAC:
             if ((par[0] = fgetc(fp)) == EOF)
               goto error;

             if ((par[1] = fgetc(fp)) == EOF)
               goto error;

             switch((unsigned char)par[0])
             {
               // for "WILL" option, now we only support terminal type.
               // others will not support.
               // so when client request other option types, server
               // will not response.
               case WILL:
                         switch((unsigned char)par[1])
                         {
                           case TERMTYPE:
                                         // reply OK message
                                         // IAC + DO + TERMINAL TYPE
//                                         fwrite(SayOKTerminalType,1,3,fp);
                                         // request client to send terminal information
//Jesse                                         fwrite(NeedClientTerminalType,1,6,fp);
										fputs(NeedClientTerminalType,fp);
                                         break;
                         }
                         break;
               // for "WONT" option, now we don't support.
               case WONT:
                         break;

               // for "DO" option, now we only support SUPPRESS GO AHEAD
               // and ECHO type.
               // others will not support.
               // so when client request other option types, server
               // will not response.
               case DO:
                        switch((unsigned char)par[1])
                        {
                          case SUPPRESSGOAHEAD:
                                         // reply OK message
                                         // IAC + WILL + SUPPRESS GO AHEAD
//Jesse                                         fwrite(SayOKSuppressGoAhead,1,3,fp);
											fputs(SayOKSuppressGoAhead,fp);
                              				break;
                          case ECHO:
                                			break;
                        }
                        break;

               // for "DONT" option, now we only support ECHO type.
               // others will not support.
               // so when client request other option types, server
               // will not response.
               case DONT:
                        switch((unsigned char)par[1])
                        {
                          case ECHO:
                                    // reply OK message
                                    // IAC + WONT + ECHO
//Jesse                                    fwrite(SayOKEchoDisable,1,3,fp);
									fputs(SayOKEchoDisable,fp);
                                    break;
                        }
                        break;

               // SB will be suboption.  The string begins "SB" and ends "SE"
               // we will only support terminal type.
               case SB:
                        switch((unsigned char)par[1])
                        {
                          case TERMTYPE:
                                        for (;;)
                                        {
                                          if ((data = fgetc(fp)) == EOF)
                                            goto error;
                                          if (data == SE)
                                            break;
                                        }
                                        break;
                        }
                        break;
             }
             break;
    case 3:     // ^C , interrupt the command
           fputs(CTRLC, fp);
           goto error;
    case 4:     // ^D , close the connection
           fputs(CTRLD, fp);
           fputs(CRLF, fp);
           goto error;
    default:
           // testing the char whether can show or not
           if ((int)isprint(data))
           {
             switch (echo)
             {
               case 1: // already enable echo option
                      Fputc(data, fp);
                      break;
               case 2: // already disable echo option
                       // don't send anything to client
                      break;
             }
           }
           return data;
  }
  goto loop;

error:
  QuitMenu = 1;
  TELNETDUsers--;
  return EOF;
}




// This subroutine will be called when user wants to exit the telent service
// add --- by arius 3/20/2000
void ExitSetup(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];

  if (ModifyConfigFlag)
  {
    fputs("\r\nData already change. Do you want to exit console?(Y/N)",fp);
    // length -1: 1 byte saves null char
    if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
    {
      switch(Buffer[0])
      {
        case  'y':
        case  'Y':
                  fputs("\r\n Quit Print Server Console \r\n",fp);
                  TELNETDUsers--;
                  QuitMenu = 1;
                  break;
      }
    }
  }
  else
  {
    fputs("\r\n Quit Print Server Console \r\n",fp);
    TELNETDUsers--;
    QuitMenu = 1;
  }

}



// This subroutine will process user input string
// add --- by arius 3/17/2000
int InputString(FILE *fp, char *str, int Strlen)
{
  int echomode = 1;       // 1: means the server will echo to client

  if (GetInputString(fp, str, Strlen, echomode) != INPUT_ERROR)
    return INPUT_OK;

  return INPUT_ERROR;
}


// This subroutine will process user input password
// add --- by arius 3/17/2000
int InputPassword(FILE *fp, char *str, int len)
{
  int echomode = 2;        // 2: means the server does not respond

  if (GetInputString(fp, str, len, echomode) != INPUT_ERROR)
    return INPUT_OK;

  return INPUT_ERROR;
}




// This subroutine for logging in wiil check name and password user input.
// The return value is the permissions field or -1 if the login failed.
// add --- by arius 3/17/2000
int UsersLogin(char *name, char *pass)
{
  char Temp[LENGTH_OF_PASSWORD+1];

  if ( stricmp(name,ADMINUSER) != NULL )
    return -1;

  memcpy(Temp,EEPROM_Data.Password,LENGTH_OF_PASSWORD);
  Temp[LENGTH_OF_PASSWORD] = 0;
  if ( stricmp(pass,Temp) != NULL )
    return -1;

  return 1;
}




// This subroutine will check IP Address user keyin including format, range. etc.
// The return 1 mean OK. Otherwise(Value = 0) has a data error.
// add --- by arius 3/24/2000
int CheckInputIPAddress(char *InputData,int Type)
{
  long Num;
  int  iReturn,Counter,NumCounter,DotCounter,TotalDot,MaxDot;
  int  index;
  char temp[40],StopChar,*token,*stop;

  // what kind of type do we want check?
  switch(Type)
  {
    // check whether it is a string or not
    case DATA_STRING:
                     iReturn = 1;
                     break;
    // check whether it is a number or not
    case DATA_NUMBER:
                     for ( index = 0; index < strlen(InputData); index++ )
                     {
                       if ( InputData[index] < '0' || InputData[index] > '9' )
                         if ( InputData[index] != ' ' )
                           iReturn = 0;   // not number since it involves space
                     }
                     iReturn = 1;
                     break;
    // check whether it is a IP address or not
    case DATA_IP:
    case DATA_NODE:
                    if ( Type == DATA_IP)
                    {
                      StopChar = '.';
                      MaxDot = 3;
                    }
                    else
                    {
                      StopChar = ':';
                      MaxDot = 5;
                    }
                    iReturn = 0;
                    Counter = 0;
                    NumCounter = 0;
                    DotCounter = 0;
                    TotalDot = 0;
                    if ( strlen(InputData) > 0 )
                    {
                      for ( index = 0; index < strlen(InputData); index++ )
                      {
                        if ( isxdigit(InputData[index]) != 0 )
                        {
                          if (Type == DATA_IP)
                          {
                            if (InputData[index] >= '0' && InputData[index] <= '9')
                            {
                              if (NumCounter == DotCounter)
                                NumCounter++;
                              Counter++;
                            }
                          }
                          else
                          {
                            if (NumCounter == DotCounter)
                              NumCounter++;
                            Counter++;
                          }
                        }
                        else if (InputData[index] == ' ')
                               Counter++;
                             else if (InputData[index] == StopChar)
                                  {
                                    if (NumCounter > DotCounter)
                                      DotCounter++;
                                    TotalDot++;
                                    Counter++;
                                  }
                      }  // end of for

                      if ( (strlen(InputData) == Counter) &&
                           (DotCounter == MaxDot) &&
                           (TotalDot == DotCounter) &&
                           (NumCounter == (MaxDot+1)) )
                      {
                        iReturn = 1;
                        strcpy(temp,InputData);

                        // Now we check range of the value
                        token = strtok(temp,&StopChar);
                        while (token != NULL)
                        {
                          if (Type == DATA_IP)
                            Num = atol(token);

                          // range drops between 0 and 255
                          if (Num < 0 || Num > 255)
                            iReturn = 0;
                          token = strtok(NULL,&StopChar);
                        }
                      }
                    }  // end of the most outer if
                    break;
  }
  return iReturn;
}



// This subroutine will show "Device Name" user wants.
// add --- by arius 3/23/2000
void PutDeviceName(FILE *fp)
{
  char temp[LENGTH_OF_BOX_NAME+1];
  char Buffer[LENGTH_OF_BOX_NAME+4];

  // Get Device Name from EEPROM
  memcpy(temp,_BoxNameInTelnet,LENGTH_OF_BOX_NAME);
  temp[LENGTH_OF_BOX_NAME] = 0;

  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);

}



// This subroutine offers users to input device name they want
// add --- by arius 3/23/2000
void GetDeviceName(FILE *fp)
{
  char Buffer[LENGTH_OF_BOX_NAME+1];
  char Message[50];

  sprintf(Message,"Input Device Name(Max %d chars): ",LENGTH_OF_BOX_NAME);
  fputs(Message,fp);

  // get device name
  if ( InputString(fp, Buffer, LENGTH_OF_BOX_NAME ) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      if (strlen(Buffer) != LENGTH_OF_BOX_NAME)
        strcpy(_BoxNameInTelnet,Buffer);
      else
        memcpy(_BoxNameInTelnet,Buffer,LENGTH_OF_BOX_NAME);
      ModifyConfigFlag = 1;     // setting flag
    }
  }
}


// This subroutine will show "Contact" user wants.
// add --- by arius 3/23/2000
void PutContact(FILE *fp)
{
  char temp[SNMP_SYSCONTACT_LEN];
  char Buffer[SNMP_SYSCONTACT_LEN+4];

  // get Contact Data
  strcpy(temp,TELNET_EEPROM_Data.SnmpSysContact);

  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}




// This subroutine offers users to input contact they want
// add --- by arius 3/23/2000
void GetContact(FILE *fp)
{
  char Buffer[SNMP_SYSCONTACT_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Contact(Max %d chars): ",SNMP_SYSCONTACT_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, SNMP_SYSCONTACT_LEN-1 ) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.SnmpSysContact,Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }

}




// This subroutine will show "Location" user wants.
// add --- by arius 3/23/2000
void PutLocation(FILE *fp)
{
  char temp[SNMP_SYSLOCATION_LEN];
  char Buffer[SNMP_SYSLOCATION_LEN+4];

  // get Location Data
  strcpy(temp,TELNET_EEPROM_Data.SnmpSysLocation);

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input Location they want
// add --- by arius 3/23/2000
void GetLocation(FILE *fp)
{
  char Buffer[SNMP_SYSLOCATION_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Location(Max %d chars): ",SNMP_SYSLOCATION_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, SNMP_SYSLOCATION_LEN-1) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.SnmpSysLocation,Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }
}



// This subroutine will show "Password" user wants.
// add --- by arius 4/12/2000
void PutPassword(FILE *fp)
{
  char temp[LENGTH_OF_PASSWORD+1];
  char Buffer[LENGTH_OF_PASSWORD+1];

  // Get Device Name from EEPROM
  memcpy(temp,_SetupPasswordInTelnet,LENGTH_OF_PASSWORD);
  temp[LENGTH_OF_PASSWORD] = 0;
  if (strlen(temp))
  {
    memset(Buffer,'*',strlen(temp));
    Buffer[strlen(temp)] =0;
    // send to client
    fputs(Buffer,fp);
  }
  fputs("\r\n",fp);
}



// This subroutine offers users to input "Password" they want
// add --- by arius 4/12/2000
void GetPassword(FILE *fp)
{
  char Buffer[LENGTH_OF_PASSWORD+1];
  char Message[50];
  char Temp[LENGTH_OF_PASSWORD+1];


  memcpy(Temp,_SetupPasswordInTelnet,LENGTH_OF_PASSWORD);
  Temp[LENGTH_OF_PASSWORD] = 0;

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Old Password(Max %d chars): ",LENGTH_OF_PASSWORD);
  fputs(Message,fp);
  // length -1: 1 byte saves null char
  if ( InputPassword(fp, Buffer, LENGTH_OF_PASSWORD) != INPUT_ERROR )
  {
    if (!strcmp(Buffer,Temp))
    {
       // Keyin New Password
       // length -1: 1 byte saves null char
      sprintf(Message,"\r\nInput New Password(Max %d chars): ",LENGTH_OF_PASSWORD);
      fputs(Message,fp);
      // length -1: 1 byte saves null char
      if ( InputPassword(fp, Buffer, LENGTH_OF_PASSWORD) != INPUT_ERROR )
      {
         // Keyin New Password again
         // length -1: 1 byte saves null char
        sprintf(Message,"\r\nInput Confirm New Password(Max %d chars): ",LENGTH_OF_PASSWORD);
        fputs(Message,fp);
        if ( InputPassword(fp, Temp, LENGTH_OF_PASSWORD) != INPUT_ERROR )
        {
          if (!strcmp(Temp,Buffer))
          {
            // Save new password
            if (strlen(Temp) != LENGTH_OF_PASSWORD)
              strcpy(_SetupPasswordInTelnet,Temp);
            else
              memcpy(_SetupPasswordInTelnet,Temp,LENGTH_OF_PASSWORD);
            ModifyConfigFlag = 1;     // setting flag
          }
          else
            fputs("\r\nTwo Times Password are not same! \r\n",fp);
        }
      }
    }
    else
      fputs("\r\nOld Password is not correct! \r\n",fp);
  }

}




#ifdef DEF_PRINTSPEED
// This subroutine will show "Printer Speed" user wants.
// add --- by arius 3/23/2000
void PutSpeed(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Printer Speed Data
  switch(_PrinterSpeedInTelnet)
  {
    case 0:
             strcpy(temp,"High");
             break;
    case 1:
             strcpy(temp,"Normal");
             break;
    case 2:
             strcpy(temp,"SLow");
             break;
    default:
             strcpy(temp,"SLow");
  }

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}


// This subroutine offers users to "Printer Speed" they want
// add --- by arius 3/23/2000
void GetSpeed(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;


  fputs("Input Speed(1.High 2.Normal 3.Slow): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:     // High
              _PrinterSpeedInTelnet = 0;
              ModifyConfigFlag = 1;
              break;
      case  2:     // Normal
              _PrinterSpeedInTelnet = 1;
              ModifyConfigFlag = 1;
              break;
      case  3:     // Slow
              _PrinterSpeedInTelnet = 2;
              ModifyConfigFlag = 1;
              break;
      default:
              // will not change previous value
              ;
    }
  }
}
#endif DEF_PRINTSPEED


#ifdef DEF_IEEE1284

// add --- by arius 5/03/2000
void PutIEEE1284PrinterData(FILE *fp)
{
  char Buffer[50];
  int  i;

  for (i = 0; i < NUM_OF_PRN_PORT; i++)
  {
    if (i != 0)
    {
      sprintf(Buffer,"   Port%d: \r\n",i+1);
      fputs(Buffer,fp);
    }
    fputs("      Manufacture: ",fp);
    if (PortIO[i].Manufacture)
      fputs(PortIO[i].Manufacture,fp);
    fputs("\r\n",fp);
    fputs("      Command Set: ",fp);
    if (PortIO[i].CommandSet)
      fputs(PortIO[i].CommandSet,fp);
    fputs("\r\n",fp);
    fputs("            Model: ",fp);
    if (PortIO[i].Model)
      fputs(PortIO[i].Model,fp);
    fputs("\r\n",fp);
  }
}

// This subroutine will show "Bi-directional" user wants.
// add --- by arius 5/03/2000
void PutBiDirect1(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Bi-Directional Data
  switch(TELNET_EEPROM_Data.Bidirectional[0])
  {
    case P1284_ITEM_AUTO:
             strcpy(temp,"Auto");
             break;
    case P1284_ITEM_DISABLE:
             strcpy(temp,"Disable");
             break;
    default:
             strcpy(temp,"Auto");
  }

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}

// This subroutine offers users to "Bi-directional" they want
// add --- by arius 5/03/2000
void GetBiDirect1(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Input Port1 Bi-Directional(1.Auto Dectect 2.Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:     // Auto Dectect
              TELNET_EEPROM_Data.Bidirectional[0] = P1284_ITEM_AUTO;
              ModifyConfigFlag = 1;
              break;
      case  2:     // Disable
              TELNET_EEPROM_Data.Bidirectional[0] = P1284_ITEM_DISABLE;
              ModifyConfigFlag = 1;
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

#if (NUM_OF_PRN_PORT != 1)

// This subroutine will show "Bi-directional" user wants.
// add --- by arius 5/03/2000
void PutBiDirect2(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Bi-Directional Data
  switch(TELNET_EEPROM_Data.Bidirectional[1])
  {
    case P1284_ITEM_AUTO:
             strcpy(temp,"Auto");
             break;
    case P1284_ITEM_DISABLE:
             strcpy(temp,"Disable");
             break;
    default:
             strcpy(temp,"Auto");
  }

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}


// This subroutine offers users to "Bi-directional" they want
// add --- by arius 5/03/2000
void GetBiDirect2(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Input Port2 Bi-Directional(1.Auto Dectect 2.Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:     // Auto Dectect
              TELNET_EEPROM_Data.Bidirectional[1] = P1284_ITEM_AUTO;
              ModifyConfigFlag = 1;
              break;
      case  2:     // Disable
              TELNET_EEPROM_Data.Bidirectional[1] = P1284_ITEM_DISABLE;
              ModifyConfigFlag = 1;
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

// This subroutine will show "Bi-directional" user wants.
// add --- by arius 5/03/2000
void PutBiDirect3(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Bi-Directional Data
  switch(TELNET_EEPROM_Data.Bidirectional[2])
  {
    case P1284_ITEM_AUTO:
             strcpy(temp,"Auto");
             break;
    case P1284_ITEM_DISABLE:
             strcpy(temp,"Disable");
             break;
    default:
             strcpy(temp,"Auto");
  }

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}


// This subroutine offers users to "Bi-directional" they want
// add --- by arius 5/03/2000
void GetBiDirect3(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Input Port3 Bi-Directional(1.Auto Dectect 2.Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:     // Auto Dectect
              TELNET_EEPROM_Data.Bidirectional[2] = P1284_ITEM_AUTO;
              ModifyConfigFlag = 1;
              break;
      case  2:     // Disable
              TELNET_EEPROM_Data.Bidirectional[2] = P1284_ITEM_DISABLE;
              ModifyConfigFlag = 1;
              break;
      default:
              // will not change previous value
              ;
    }
  }
}
#endif

#endif DEF_IEEE1284



// This subroutine will show "Netware(Bindery Mode) Status" user wants.
// add --- by arius 3/24/2000
void PutNetwareStatus(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Netware Status Data
  if (TELNET_EEPROM_Data.PrintServerMode & PS_NETWARE_MODE)
    strcpy(temp,"Enable");
  else
    strcpy(temp,"Disable");

  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Netware(Bindery Mode) Status" they want
// add --- by arius 3/24/2000
void GetNetwareStatus(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("NetWare Setting(1.Enable 2.Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // enable
              TELNET_EEPROM_Data.PrintServerMode |= PS_NETWARE_MODE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // disbale first bit = 11111110 = 0xFE
              TELNET_EEPROM_Data.PrintServerMode &= ~PS_NETWARE_MODE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}



// This subroutine will show "Print Server Name" user wants.
// add --- by arius 3/23/2000
void PutPrintServerName(FILE *fp)
{
  char temp[LENGTH_OF_PS_NAMES];
  char Buffer[LENGTH_OF_PS_NAMES+4];

  // get Print Server name
  strcpy(temp,_PrintServerNameInTelnet);

  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}


// This subroutine offers users to input Speed they want
// add --- by arius 3/23/2000
void GetPrintServerName(FILE *fp)
{
  char Buffer[LENGTH_OF_PS_NAMES];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Print Server Name(Max %d chars): ",LENGTH_OF_PS_NAMES-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_PS_NAMES-1) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      strcpy(_PrintServerNameInTelnet,Buffer);
      ModifyConfigFlag = 1;     // setting flag
    }
  }
}


// This subroutine will show "File Server Name" user wants.
// add --- by arius 3/23/2000
void PutFileServerName(FILE *fp)
{
  int index;
  char Message[LENGTH_OF_FS_NAMES];

  if (ServiceFSCountInTelnet) fputs(_FileServerNameInTelnet(0),fp);
  for (index = 1; index < ServiceFSCountInTelnet; index++ )
  {
    sprintf(Message,",%s",_FileServerNameInTelnet(index));
    fputs(Message,fp);
  }
  fputs("\r\n",fp);
}



// This subroutine will show "Polling Time" user wants.
// add --- by arius 3/23/2000
void PutPollingTime(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];

  // give message format, such as "%d \r\n"
  sprintf(Buffer,"%d \r\n",TELNET_EEPROM_Data.PollingTime);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Polling Time" they want
// add --- by arius 3/23/2000
void GetPollingTime(FILE *fp)
{
  char Buffer[LENGTH_OF_POLLING_TIME];
  int  temp;

  fputs("Input Polling Time(3-29 seconds): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_POLLING_TIME-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    // the value is legal or illegal
    // illegal will be dropped it
    if ( temp <= 29 && temp >= 3)
    {
      TELNET_EEPROM_Data.PollingTime = temp;
      ModifyConfigFlag = 1;    // setting flag
    }
  }
}



#ifdef NDS_PS
// This subroutine will show "Novell Password" user wants.
// add --- by arius 4/12/2000
void PutNovellPassword(FILE *fp)
{
  char Buffer[NOVELL_PASSWORD_LEN];

  if (strlen(_NovellPasswordInTelnet))
  {
    memset(Buffer,'*',strlen(_NovellPasswordInTelnet));
    Buffer[strlen(_NovellPasswordInTelnet)] = 0;
    fputs(Buffer,fp);
  }
  fputs("\r\n",fp);
}



// This subroutine offers users to input "Novell Password" they want
// add --- by arius 4/12/2000
void GetNovellPassword(FILE *fp)
{
  char Buffer[NOVELL_PASSWORD_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Password(Max %d chars): ",NOVELL_PASSWORD_LEN-1);
  fputs(Message,fp);
  // length -1: 1 byte saves null char
  if ( InputPassword(fp, Buffer, NOVELL_PASSWORD_LEN-1) != INPUT_ERROR )
  {
    // length is not zero
//    if (strlen(Buffer))
//    {
      strcpy(_NovellPasswordInTelnet,Buffer);
      strupr(_NovellPasswordInTelnet);
      ModifyConfigFlag = 1;     // setting flag
//    }
  }
}


// This subroutine will show "NDS enable/disable" user wants.
// add --- by arius 4/12/2000
void PutNDSStatus(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Netware Status Data
  if (TELNET_EEPROM_Data.PrintServerMode & PS_NDS_MODE)
    strcpy(temp,"Enable");
  else
    strcpy(temp,"Disable");

  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);

}




// This subroutine offers users to input "NDS enable/disable" they want
// add --- by arius 4/12/2000
void GetNDSStatus(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("NDS Mode Setting(1.Enable 2.Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // enable
              TELNET_EEPROM_Data.PrintServerMode |= PS_NDS_MODE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // disbale
              TELNET_EEPROM_Data.PrintServerMode &= ~PS_NDS_MODE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}




// This subroutine will show "NDS Tree Name" user wants.
// add --- by arius 4/12/2000
void PutNDSTreeName(FILE *fp)
{
  char Buffer[NDS_TREE_LEN];

  // get Print Server name
  strcpy(Buffer,_NDSTreeNameInTelnet);

  // send to client
  fputs(Buffer,fp);
  fputs("\r\n",fp);
}


// This subroutine offers users to input "NDS Tree Name" they want
// add --- by arius 4/12/2000
void GetNDSTreeName(FILE *fp)
{
  unsigned long LastObjectSeed = 0xFFFFFFFF;
  BYTE  DSName[48],rc;
  int16 DSOffset,DSCount = 0;
  BYTE  TmpBuf[50];
  int   Value;
  ServerNameTable NTables[100];
  char Message[NDS_TREE_LEN+4];
  char Buffer[NDS_TREE_LEN];

  TmpBuf[0] = '\0';

  if ((rc = SearchObjectInit()) == 0)
  {
    while (!SearchBinaryObjectName(DSName,&LastObjectSeed,DS_SERVER_OBJECT))
    {
      DSOffset = 32;
      do {
              DSName[DSOffset--] = '\0';
      } while(DSOffset && DSName[DSOffset] == '_');

      if (HasDSExist(TmpBuf,DSName)) continue;
      strcpy(NTables[DSCount].Name,DSName);
      NTables[DSCount].Flag = 0;
      DSCount++;

      sprintf(Message,"%d. %s\r\n",DSCount,DSName);
      fputs(Message,fp);
    }
  }

  if (DSCount)
  {
    fputs("Please Select one number: ",fp);
    // length -1: 1 byte saves null char
    if ( InputString(fp, Buffer,LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
    {
      if (strlen(Buffer))
      {
        Value = atoi(Buffer);
        if ( (Value >= 1) && (Value <= DSCount) )
        {
          strcpy(_NDSTreeNameInTelnet,NTables[Value-1].Name);
          ModifyConfigFlag = 1;       // setting flag
        }
      }
      else
        ; // do not chnage any data
    }
  }
  else
    fputs("\r\nTree Name can not Found on the network\r\n",fp);
}


// This subroutine will show "NDS Context" user wants.
// add --- by arius 4/12/2000
void PutNDSContext(FILE *fp)
{
  char Buffer[NDS_CONTEXT_LEN];

  // get Print Server name
  strcpy(Buffer,_NDSContextInTelnet);

  // send to client
  fputs(Buffer,fp);
  fputs("\r\n",fp);

}


// This subroutine offers users to input "NDS Context" they want
// add --- by arius 4/12/2000
void GetNDSContext(FILE *fp)
{
  char Buffer[NDS_CONTEXT_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input NDS Context(Max %d chars): ",NDS_CONTEXT_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, NDS_CONTEXT_LEN-1) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      strcpy(_NDSContextInTelnet,Buffer);
      ModifyConfigFlag = 1;     // setting flag
    }
  }

}
#endif NDS_PS


// This subroutine will show "DHCP/BOOTP" user wants.
// add --- by arius 3/23/2000
void PutDhcpAndBootp(FILE *fp)
{
  char temp[LENGTH_OF_BUFFER];
  char Buffer[LENGTH_OF_BUFFER+4];

  // get Netware Status Data
  if (TELNET_EEPROM_Data.PrintServerMode & PS_DHCP_ON)
    strcpy(temp,"Enable");
  else
    strcpy(temp,"Disable");

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "DHCP/BOOTP" they want
// add --- by arius 3/23/2000
void GetDhcpAndBootp(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Input(1.Enable 2.Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // enable
              TELNET_EEPROM_Data.PrintServerMode |= PS_DHCP_ON;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // disbale first bit = 11111101
              TELNET_EEPROM_Data.PrintServerMode &= ~PS_DHCP_ON;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }

}



// This subroutine will show "IP Address" user wants.
// add --- by arius 3/23/2000
void PutIPAddress(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE *IPAddr;

  // give message format, such as "[%u.%u.%u.%u]\r\n"
  IPAddr = _BoxIPAddressInTelnet;
  sprintf(Buffer,IpAddressFormat1,IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3]);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "IP Address" they want
// add --- by arius 3/23/2000
void GetIPAddress(FILE *fp)
{
  char Buffer[LENGTH_OF_IPADDRESS];

  fputs("Input IP Address(x.x.x.x): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_IPADDRESS-1) != INPUT_ERROR )
  {
    // Now checking data(IP address) user keyin
    if (CheckInputIPAddress(Buffer,DATA_IP))
    {
		//Charles
		NSET32(_BoxIPAddressInTelnet, DWordSwap(aton(Buffer)) );
      ModifyConfigFlag = 1;      // setting flag
    }
    else
    ;  // data error we drop it
  }

}



// This subroutine will show "Subnet Mask" user wants.
// add --- by arius 3/23/2000
void PutSubnetMask(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE *IPAddr;

  // give message format, such as "[%u.%u.%u.%u]\r\n"
  IPAddr = _BoxSubNetMaskInTelnet;
  sprintf(Buffer,IpAddressFormat1,IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3]);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Subnet Mask" they want
// add --- by arius 3/23/2000
void GetSubnetMask(FILE *fp)
{
  char Buffer[LENGTH_OF_IPADDRESS];

  fputs("Input Subnet Mask Address(x.x.x.x): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_IPADDRESS-1) != INPUT_ERROR )
  {
    // Now checking data(IP address) user keyin
    if (CheckInputIPAddress(Buffer,DATA_IP))
    {
		//Charles
		NSET32( _BoxSubNetMaskInTelnet, DWordSwap(aton(Buffer)) );
      ModifyConfigFlag = 1;       // setting flag
    }
    else
    ;  // data error we drop it
  }
}



// This subroutine will show "Gateway" user wants.
// add --- by arius 3/23/2000
void PutGateway(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE *IPAddr;

  // give message format, such as "[%u.%u.%u.%u]\r\n"
  IPAddr = _BoxGatewayAddressInTelnet;
  sprintf(Buffer,IpAddressFormat1,IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3]);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Gateway" they want
// add --- by arius 3/23/2000
void GetGateway(FILE *fp)
{
  char Buffer[LENGTH_OF_IPADDRESS];

  fputs("Input Gateway Address(x.x.x.x): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_IPADDRESS-1) != INPUT_ERROR )
  {
    // Now checking data(IP address) user keyin
    if (CheckInputIPAddress(Buffer,DATA_IP))
    {
		//Charles
	   NSET32(_BoxGatewayAddressInTelnet, DWordSwap(aton(Buffer)) );
      ModifyConfigFlag = 1;     // setting flag
    }
    else
    ;  // data error we drop it
  }
}




// This subroutine will show "Community1" user wants.
// add --- by arius 3/24/2000
void PutCommunity1(FILE *fp)
{
  char Buffer[SNMP_COMMUNITY_LEN];
  char Buffer1[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  // give message format, such as "%s "
  sprintf(Buffer,StringFormat3,TELNET_EEPROM_Data.SnmpCommunityAuthName[0]);

  // send to client
  fputs(Buffer,fp);

  TmpValue = TELNET_EEPROM_Data.SnmpAccessFlag.SnmpComm0AccessMode;
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case 0:
            sprintf(Buffer1,StringFormat2,"No Access Right");
            break;
    case 1:
            sprintf(Buffer1,StringFormat2,"Read-Only");
            break;
    case 2:
            sprintf(Buffer1,StringFormat2,"Write Only");
            break;
    case 3:
            sprintf(Buffer1,StringFormat2,"Read-Write");
            break;
  }

  // send to client
  fputs(Buffer1,fp);
}




// This subroutine offers users to input "Community1" they want
// add --- by arius 3/24/2000
void GetCommunity1(FILE *fp)
{
  char Buffer[SNMP_COMMUNITY_LEN];
  char Buffer1[LENGTH_OF_MULTI_CHOICE];
  char Message[50];
  int  temp;

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Community1 Name(Max %d chars):",SNMP_COMMUNITY_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, SNMP_COMMUNITY_LEN-1) != INPUT_ERROR )
  {
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.SnmpCommunityAuthName[0],Buffer);
      ModifyConfigFlag = 1;      // setting flag
    }
  }
  fputs("Input Community1(1. Read-Only  2. Read-Write): ",fp);
  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer1, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer1);

    switch(temp)
    {
      case  1:    // Read-Only
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpComm0AccessMode = 1;
              ModifyConfigFlag = 1;      // setting flag
              break;
      case  2:    // Read-Write
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpComm0AccessMode = 3;
              ModifyConfigFlag = 1;      // stting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}




// This subroutine will show "Community2" user wants.
// add --- by arius 3/24/2000
void PutCommunity2(FILE *fp)
{
  char Buffer[SNMP_COMMUNITY_LEN];
  char Buffer1[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  // give message format, such as "%s "
  sprintf(Buffer,StringFormat3,TELNET_EEPROM_Data.SnmpCommunityAuthName[1]);

  // send to client
  fputs(Buffer,fp);

  TmpValue = TELNET_EEPROM_Data.SnmpAccessFlag.SnmpComm1AccessMode;
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case 0:
            sprintf(Buffer1,StringFormat2,"No Access Right");
            break;
    case 1:
            sprintf(Buffer1,StringFormat2,"Read-Only");
            break;
    case 2:
            sprintf(Buffer1,StringFormat2,"Write Only");
            break;
    case 3:
            sprintf(Buffer1,StringFormat2,"Read-Write");
            break;
  }

  // send to client
  fputs(Buffer1,fp);
}




// This subroutine offers users to input "Community2" they want
// add --- by arius 3/24/2000
void GetCommunity2(FILE *fp)
{
  char Buffer[SNMP_COMMUNITY_LEN];
  char Buffer1[LENGTH_OF_MULTI_CHOICE];
  char Message[50];
  int  temp;

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Community2 Name(Max %d chars): ",SNMP_COMMUNITY_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, SNMP_COMMUNITY_LEN-1) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.SnmpCommunityAuthName[1],Buffer);
      ModifyConfigFlag = 1;      // setting flag
    }
  }

  fputs("Input Community2(1. Read-Only  2. Read-Write): ",fp);
  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer1, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer1);

    switch(temp)
    {
      case  1:    // Read-Only
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpComm1AccessMode = 1;
              ModifyConfigFlag = 1;      // setting flag
              break;
      case  2:    // Read-Write
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpComm1AccessMode = 3;
              ModifyConfigFlag = 1;      // stting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}




// This subroutine will show "SNNP Traps" user wants.
// add --- by arius 3/24/2000
void PutSNNPTraps(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  TmpValue = TELNET_EEPROM_Data.SnmpAccessFlag.SnmpTrapEnable;
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case 0:
            sprintf(Buffer,StringFormat2,"Disable");
            break;
    case 1:
            sprintf(Buffer,StringFormat2,"Enable");
            break;
  }

  // send to client
  fputs(Buffer,fp);
}




// This subroutine offers users to input "SNNP Traps" they want
// add --- by arius 3/24/2000
void GetSNNPTraps(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Input SNNP Traps(1. Enable  2. Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:   // Enable
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpTrapEnable = 1;
              ModifyConfigFlag = 1;     // setting flag
              break;
      case  2:   // Disable
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpTrapEnable = 0;
              ModifyConfigFlag = 1;     // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}



// This subroutine will show "Authentication Traps" user wants.
// add --- by arius 3/24/2000
void PutAuthenTraps(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  TmpValue = TELNET_EEPROM_Data.SnmpAccessFlag.SnmpAuthenTrap;
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case 2:
            sprintf(Buffer,StringFormat2,"Disable");
            break;
    case 1:
            sprintf(Buffer,StringFormat2,"Enable");
            break;
  }

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Authentication Traps" they want
// add --- by arius 3/24/2000
void GetAuthenTraps(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Input Authentication Traps(1. Enable  2. Disable): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:   // Enable
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpAuthenTrap = 1;
              ModifyConfigFlag = 1;     // setting flag
              break;
      case  2:   // Disable
              TELNET_EEPROM_Data.SnmpAccessFlag.SnmpAuthenTrap = 2;
              ModifyConfigFlag = 1;     // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }

}




// This subroutine will show "Trap1" user wants.
// add --- by arius 3/24/2000
void PutTrap1(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE *IPAddr;

  // give message format, such as "[%u.%u.%u.%u]\r\n"
  IPAddr = TELNET_EEPROM_Data.SnmpTrapTargetIP[0];
  sprintf(Buffer,IpAddressFormat1,IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3]);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Trap1" they want
// add --- by arius 3/24/2000
void GetTrap1(FILE *fp)
{
  char Buffer[LENGTH_OF_IPADDRESS];
  BYTE IPAddr[5];

  fputs("Input Snmp Trap1 IP Address(x.x.x.x): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_IPADDRESS-1) != INPUT_ERROR )
  {
    // Now checking data(IP address) user keyin
    if (CheckInputIPAddress(Buffer,DATA_IP))
    {
		NSET32(IPAddr, aton(Buffer));
      TELNET_EEPROM_Data.SnmpTrapTargetIP[0][0] = IPAddr[3];
      TELNET_EEPROM_Data.SnmpTrapTargetIP[0][1] = IPAddr[2];
      TELNET_EEPROM_Data.SnmpTrapTargetIP[0][2] = IPAddr[1];
      TELNET_EEPROM_Data.SnmpTrapTargetIP[0][3] = IPAddr[0];
      ModifyConfigFlag = 1;     // setting flag
    }
    else
    ;  // data error we drop it
  }
}


// This subroutine will show "Trap2" user wants.
// add --- by arius 3/24/2000
void PutTrap2(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE *IPAddr;

  // give message format, such as "[%u.%u.%u.%u]\r\n"
  IPAddr = TELNET_EEPROM_Data.SnmpTrapTargetIP[1];
  sprintf(Buffer,IpAddressFormat1,IPAddr[0],IPAddr[1],IPAddr[2],IPAddr[3]);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Trap2" they want
// add --- by arius 3/24/2000
void GetTrap2(FILE *fp)
{
  char Buffer[LENGTH_OF_IPADDRESS];
  BYTE IPAddr[5];

  fputs("Input Snmp Trap2 IP Address(x.x.x.x): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_IPADDRESS-1) != INPUT_ERROR )
  {
    // Now checking data(IP address) user keyin
    if (CheckInputIPAddress(Buffer,DATA_IP))
    {
		NSET32(IPAddr, aton(Buffer));
      TELNET_EEPROM_Data.SnmpTrapTargetIP[1][0] = IPAddr[3];
      TELNET_EEPROM_Data.SnmpTrapTargetIP[1][1] = IPAddr[2];
      TELNET_EEPROM_Data.SnmpTrapTargetIP[1][2] = IPAddr[1];
      TELNET_EEPROM_Data.SnmpTrapTargetIP[1][3] = IPAddr[0];
      ModifyConfigFlag = 1;     // setting flag
    }
    else
    ;  // data error we drop it
  }
}




#ifdef ATALKD
// This subroutine will show "Zone Name" user wants.
// add --- by arius 3/24/2000
void PutZoneName(FILE *fp)
{
  char temp[ATALK_ZONE_LEN];
  char Buffer[ATALK_ZONE_LEN+5];

  // get Zone name
  strcpy(temp,TELNET_EEPROM_Data.ATZoneName);

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Zone Name" they want
// add --- by arius 3/24/2000
void GetZoneName(FILE *fp)
{
  char Buffer[ATALK_ZONE_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Zone Name(Max %d chars): ",ATALK_ZONE_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, ATALK_ZONE_LEN-1) != INPUT_ERROR )
  {
    // length is not zero
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.ATZoneName,Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }
}



// This subroutine will show "Port Name" user wants.
// add --- by arius 4/13/2000
void PutApplePortName(FILE *fp)
{
  char temp[ATALK_PORT_NAME];
  char Buffer[ATALK_PORT_NAME+5];

  // get port name
  strcpy(temp,TELNET_EEPROM_Data.ATPortName);

  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}



// This subroutine offers users to input "Port Name" they want
// add --- by arius 4/13/2000
void GetApplePortName(FILE *fp)
{
  char Buffer[ATALK_PORT_NAME];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input APPLE Port Name(Max %d chars): ",ATALK_PORT_NAME-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, ATALK_PORT_NAME-1) != INPUT_ERROR )
  {
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.ATPortName,Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }
}


// This subroutine will show "Printer Type" user wants.
// add --- by arius 3/24/2000
void PutPrinterType1(FILE *fp)
{
  char temp[ATALK_TYPE_LEN];
  char Buffer[ATALK_TYPE_LEN+5];

  // get port name
  strcpy(temp,TELNET_EEPROM_Data.ATPortType[0]);

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}
// This subroutine offers users to input "Printer Type" they want
// add --- by arius 3/24/2000
void GetPrinterType1(FILE *fp)
{
  char Buffer[ATALK_TYPE_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Printer Port Type  Name(Max %d chars): ",ATALK_TYPE_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, ATALK_TYPE_LEN-1) != INPUT_ERROR )
  {
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.ATPortType[0],Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }
}

// This subroutine will show "Data Format" user wants.
// add --- by arius 6/02/2000
void PutDataFormat1(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  TmpValue = TELNET_EEPROM_Data.ATDataFormat[0];
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case AT_COMM_NONE:
            sprintf(Buffer,StringFormat2,"ASCII");
            break;
    case AT_COMM_TBCP:
            sprintf(Buffer,StringFormat2,"TBCP");
            break;
    case AT_COMM_BCP:
            sprintf(Buffer,StringFormat2,"BCP");
            break;
  }

  // send to client
  fputs(Buffer,fp);
}

// This subroutine offers users to input "Data Format" they want
// add --- by arius 6/02/2000
void GetDataFormat1(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Data Format Setting(1.ASCII 2.TBCP 3.BCP): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // ASCII
              TELNET_EEPROM_Data.ATDataFormat[0] =  AT_COMM_NONE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // TBCP
              TELNET_EEPROM_Data.ATDataFormat[0] =  AT_COMM_TBCP;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  3:  // BCP
              TELNET_EEPROM_Data.ATDataFormat[0] =  AT_COMM_BCP;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

#if (NUM_OF_PRN_PORT != 1)
// This subroutine will show "Printer Type" user wants.
// add --- by arius 5/03/2000
void PutPrinterType2(FILE *fp)
{
  char temp[ATALK_TYPE_LEN];
  char Buffer[ATALK_TYPE_LEN+5];

  // get port name
  strcpy(temp,TELNET_EEPROM_Data.ATPortType[1]);

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}
// This subroutine offers users to input "Printer Type" they want
// add --- by arius 5/03/2000
void GetPrinterType2(FILE *fp)
{
  char Buffer[ATALK_TYPE_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Printer Port Type  Name(Max %d chars): ",ATALK_TYPE_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, ATALK_TYPE_LEN-1) != INPUT_ERROR )
  {
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.ATPortType[1],Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }
}

// This subroutine will show "Data Format" user wants.
// add --- by arius 6/02/2000
void PutDataFormat2(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  TmpValue = TELNET_EEPROM_Data.ATDataFormat[1];
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case AT_COMM_NONE:
            sprintf(Buffer,StringFormat2,"ASCII");
            break;
    case AT_COMM_TBCP:
            sprintf(Buffer,StringFormat2,"TBCP");
            break;
    case AT_COMM_BCP:
            sprintf(Buffer,StringFormat2,"BCP");
            break;
  }

  // send to client
  fputs(Buffer,fp);
}

// This subroutine offers users to input "Data Format" they want
// add --- by arius 6/02/2000
void GetDataFormat2(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Data Format Setting(1.ASCII 2.TBCP 3.BCP): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // ASCII
              TELNET_EEPROM_Data.ATDataFormat[1] =  AT_COMM_NONE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // TBCP
              TELNET_EEPROM_Data.ATDataFormat[1] =  AT_COMM_TBCP;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  3:  // BCP
              TELNET_EEPROM_Data.ATDataFormat[1] =  AT_COMM_BCP;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

// This subroutine will show "Printer Type" user wants.
// add --- by arius 5/03/2000
void PutPrinterType3(FILE *fp)
{
  char temp[ATALK_TYPE_LEN];
  char Buffer[ATALK_TYPE_LEN+5];

  // get port name
  strcpy(temp,TELNET_EEPROM_Data.ATPortType[2]);

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}

// This subroutine offers users to input "Printer Type" they want
// add --- by arius 5/03/2000
void GetPrinterType3(FILE *fp)
{
  char Buffer[ATALK_TYPE_LEN];
  char Message[50];

  // length -1: 1 byte saves null char
  sprintf(Message,"Input Printer Port Type  Name(Max %d chars): ",ATALK_TYPE_LEN-1);
  fputs(Message,fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, ATALK_TYPE_LEN-1) != INPUT_ERROR )
  {
    if (strlen(Buffer))
    {
      strcpy(TELNET_EEPROM_Data.ATPortType[2],Buffer);
      ModifyConfigFlag = 1;   // setting flag
    }
  }
}

// This subroutine will show "Data Format" user wants.
// add --- by arius 6/02/2000
void PutDataFormat3(FILE *fp)
{
  char Buffer[LENGTH_OF_BUFFER];
  BYTE TmpValue;

  TmpValue = TELNET_EEPROM_Data.ATDataFormat[2];
  switch(TmpValue)
  {
    // give message format, such as "%s\r\n"
    case AT_COMM_NONE:
            sprintf(Buffer,StringFormat2,"ASCII");
            break;
    case AT_COMM_TBCP:
            sprintf(Buffer,StringFormat2,"TBCP");
            break;
    case AT_COMM_BCP:
            sprintf(Buffer,StringFormat2,"BCP");
            break;
  }

  // send to client
  fputs(Buffer,fp);
}

// This subroutine offers users to input "Data Format" they want
// add --- by arius 6/02/2000
void GetDataFormat3(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];
  int  temp;

  fputs("Data Format Setting(1.ASCII 2.TBCP 3.BCP): ",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // ASCII
              TELNET_EEPROM_Data.ATDataFormat[2] =  AT_COMM_NONE;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // TBCP
              TELNET_EEPROM_Data.ATDataFormat[2] =  AT_COMM_TBCP;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  3:  // BCP
              TELNET_EEPROM_Data.ATDataFormat[2] =  AT_COMM_BCP;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}
#endif
#endif ATALKD




// This subroutine offers users to input "Reset Printer" they want
// add --- by arius 3/24/2000
void ResetPrinter(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];

  fputs("Are you sure that you want reset Print Server?(Y/N)",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    switch(Buffer[0])
    {
      case  'y':
      case  'Y':
#ifdef _PC
                fputs(" \r\nReset successful \r\n",fp);
#else
                // reset
//Jesse                BOOTSTRAP();
				//Reset();
				REBOOT();
				fputs("\r\n Quit Print Server Console \r\n",fp);
                TELNETDUsers--;
                QuitMenu = 1;
				
#endif _PC
                break;
      default:
              // will not change previous value
              ;
    }
  }
}




// add --- by arius 5/03/2000
void LoadDefault(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];

  fputs("Are you sure that you want load default value?(Y/N)",fp);

  // length -1: 1 byte saves null char
  if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
  {
    switch(Buffer[0])
    {
      case  'y':
      case  'Y':
#ifdef _PC
                fputs(" \r\nReset successful \r\n",fp);
#else
                // load default
                ResetToDefalutFlash(1,1,0);
                // reset
//Jesse                BOOTSTRAP();
				REBOOT();
				fputs("\r\n Quit Print Server Console \r\n",fp);
                TELNETDUsers--;
                QuitMenu = 1;
				//Reset();
#endif _PC
                break;
      default:
              // will not change previous value
              ;
    }
  }
}


// This subroutine offers users to save data already changed into print server
// add --- by arius 3/24/2000
void SaveDataToPrint(FILE *fp)
{
  char Buffer[LENGTH_OF_MULTI_CHOICE];

  if (!ModifyConfigFlag)
  {
    fputs("\r\n No any data was changed! \r\n",fp);
    // length -1: 1 byte saves null char
    if ( InputString(fp, Buffer, LENGTH_OF_MULTI_CHOICE-1) != INPUT_ERROR )
      ;
  }
  else
  {
#ifdef _PC
    fputs("\r\n Data already save into print server. \r\n",fp);
    ModifyConfigFlag = 0;
#else
    if (WriteToEEPROM(&TELNET_EEPROM_Data) != 0)
      ErrLightOnForever(LED_WR_EEPROM); // Write EEPROM Data

    // reset print server
//Jesse    BOOTSTRAP();
	// Reset();
	REBOOT();
	fputs("\r\n Quit Print Server Console \r\n",fp);
    TELNETDUsers--;
    QuitMenu = 1;
	
#endif _PC

  }
}



// This subroutine will show "Up time" user wants.
// add --- by arius 3/24/2000
void PutUpTime(FILE *fp)
{
  char Buffer[50];

  // send to client
  fputs(UptimeString((uint32)msclock(),Buffer), fp);
  fputs("\r\n",fp);
}


// This subroutine will show "Version" user wants.
// add --- by arius 3/24/2000
void PutVersion(FILE *fp)
{
  char Buffer[30];

  UtilGetVersionString(Buffer); //4/26/2000
  // send to client
  fputs(Buffer,fp);
  fputs("\r\n",fp);
}


// This subroutine will show "Node ID" user wants.
// add --- by arius 3/24/2000
void PutNodeID(FILE *fp)
{
  char Buffer[20];
  BYTE *IPAddr;

  IPAddr = TELNET_EEPROM_Data.EthernetID;
  sprintf(Buffer,"%02X-%02X-%02X-%02X-%02X-%02X\r\n",IPAddr[0],IPAddr[1],IPAddr[2],
                  IPAddr[3],IPAddr[4],IPAddr[5]);

  // send to client
  fputs(Buffer,fp);
}


// This subroutine will show "Printer Status" user wants.
// add --- by arius 3/24/2000
void PutPrinterStatus(FILE *fp)
{
  BYTE PrintStatus, ECHO_PORT1_STATUS = 0;
  char Buffer[50];
  int  i;

  for (i = 0; i < NUM_OF_PRN_PORT; i++)
  {
#if (NUM_OF_PRN_PORT != 1)
     sprintf(Buffer,"\r\n           Port %d: ",i+1);
     fputs(Buffer,fp);
#endif
     PrintStatus = ReadPrintStatus();

     PrintStatus >>= ((i- ECHO_PORT1_STATUS) << 1);
     switch(PrintStatus & 0x03)
     {
       // give message format, such as "%s"
       case 0:    // Wating for job
               sprintf(Buffer,StringFormat3,"Waiting For Job");
               break;
       case 1:    // Paper out
               sprintf(Buffer,StringFormat3,"Paper Out");
               break;
       case 2:    // OFF Line
               sprintf(Buffer,StringFormat3,"OFF Line");
               break;
       case 3:    // Printing
               sprintf(Buffer,StringFormat3,"Printing");
               break;
     }
     fputs(Buffer,fp);
  }
  fputs("\r\n",fp);
}



// This subroutine will show "Bindery mode Connect Status" user wants.
// add --- by arius 3/24/2000
void PutConnectStatus(FILE *fp)
{
  char temp[20];
  char Buffer[25];

  if (NovellConnectFlag)
    strcpy(temp,"Connected");
  else
    strcpy(temp,"Disconnected");
  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}



#ifdef NDS_PS
// This subroutine will show "NDS mode Connect Status" user wants.
// add --- by arius 4/12/2000
void PutNDSConnectStatus(FILE *fp)
{
  char temp[20];
  char Buffer[25];

  if (NDSConnectFlag)
    strcpy(temp,"Connected");
  else
    strcpy(temp,"Disconnected");
  // give message format, such as "%s\r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}
#endif NDS_PS


#ifdef ATALKD
// This subroutine will show "Choose Name" user wants.
// add --- by arius 3/24/2000
void PutChooserName(FILE *fp)
{
  char Buffer[100];
  BYTE ECHO_ATALK_PORT1_NAME = 0;
  int  i;

  for (i = 0; i < NUM_OF_PRN_PORT; i++)
  {
    if (i != 0)
    {
      sprintf(Buffer,"   Port%d: \r\n",i+1);
      fputs(Buffer,fp);
    }
    sprintf(Buffer,"      Chooser Name: %s\r\n",GetATPortName(i-ECHO_ATALK_PORT1_NAME));
    fputs(Buffer,fp);
    sprintf(Buffer,"      Printer Type: %s\r\n",TELNET_EEPROM_Data.ATPortType[i-ECHO_ATALK_PORT1_NAME]);
    fputs(Buffer,fp);
             fputs("       Data Format: ",fp);
    switch( TELNET_EEPROM_Data.ATDataFormat[i-ECHO_ATALK_PORT1_NAME] )
    {
      case AT_COMM_NONE:
                        sprintf(Buffer,"ASCII\r\n");
                        break;
      case AT_COMM_TBCP:
                        sprintf(Buffer,"TBCP\r\n");
                        break;
      case AT_COMM_BCP:
                        sprintf(Buffer,"BCP\r\n");
                        break;
    }
    fputs(Buffer,fp);
  }
}



// This subroutine will show "Current Zone Name" user wants.
// add --- by arius 4/13/2000
void PutCurrentZoneName(FILE *fp)  // Now using zone name
{
//  fputs(at_iface.zonename,fp);
  fputs(TELNET_EEPROM_Data.ATZoneName,fp);

  fputs("\r\n",fp);
}
#endif ATALKD


#ifdef WIRELESS_CARD
// This subroutine will show "Wireless Mode" user wants.
// add --- by Jerry 7/17/2003
void PutWLANMode(FILE *fp)
{
  char Buffer[25];

     switch(TELNET_EEPROM_Data.WLMode)
     {
       // give message format, such as "%s"
       case 0:    // Infrastructure
               sprintf(Buffer,StringFormat3,"Infrastructure");
               break;
       case 1:    // Pseudo Ad-Hoc
               sprintf(Buffer,StringFormat3,"Ad-Hoc(peer to peer)");
               break;
       case 2:    // 802.11Ad-Hoc
               sprintf(Buffer,StringFormat3,"802.11 Ad-Hoc");
               break;
     }
      fputs(Buffer,fp);   
  
  fputs("\r\n",fp);
}

// This subroutine offers users to input "Wireless Mode" they want
// add --- by Jerry 7/17/2003
void GetWLANMode(FILE *fp)
{
  char Buffer[10];
  int  temp;

  fputs("Mode Setting(1.Infrastructure 2.802.11Ad-Hoc 3.Ad-Hoc(peer to peer)): ",fp);

  //  1 byte saves null char
  if ( InputString(fp, Buffer, 9) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // Infrastructure
              TELNET_EEPROM_Data.WLMode =  0;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // 802.11Ad-Hoc
              TELNET_EEPROM_Data.WLMode =  2;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  3:  // Ad-Hoc(peer to peer)
              TELNET_EEPROM_Data.WLMode =  1;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

// This subroutine will show "ESSID" user wants.
// add --- by Jerry 7/17/2003
void PutWLANESSID(FILE *fp)
{
	char temp[33]={0};
  char Buffer[32+5];

  // get ESSID
	//strcpy(temp,TELNET_EEPROM_Data.WLESSID);
	memcpy(temp, TELNET_EEPROM_Data.WLESSID, 32);

  // give message format, such as "%s \r\n"
  sprintf(Buffer,StringFormat2,temp);

  // send to client
  fputs(Buffer,fp);
}

// This subroutine offers users to input "ESSID" they want
// add --- by Jerry 7/17/2003
void GetWLANESSID(FILE *fp)
{
	char Buffer[33]={0};
  char Message[50];

  //  1 byte saves null char
	sprintf(Message,"Input ESSID(Max %d chars): ",32);
  fputs(Message,fp);

  // 1 byte saves null char
	if ( InputString(fp, Buffer, 32) != INPUT_ERROR )
  {
	//      strcpy(TELNET_EEPROM_Data.WLESSID,Buffer);
		memcpy(TELNET_EEPROM_Data.WLESSID, Buffer, 32);
      ModifyConfigFlag = 1;   // setting flag
  }
}


// This subroutine will show "Wireless Channel" user wants.
// add --- by Jerry 7/17/2003
void PutWLANChannel(FILE *fp)  
{
//  fputs(TELNET_EEPROM_Data.WLChannel,fp);
  int Buffer[3];

   // give message format, such as "%d \r\n"
  sprintf(Buffer,IntegerFormat2,TELNET_EEPROM_Data.WLChannel);

  // send to client
  fputs(Buffer,fp);

}

// This subroutine offers users to input "Wireless Channel" they want
// add --- by Jerry 7/17/2003
void GetWLANChannel(FILE *fp)
{
	char	Buffer[5];
	int 	temp;
	uint16	country;
	country = TELNET_EEPROM_Data.WLZone;
	switch( country )
	{
		case 0xA1:  //USA    1-11
			fputs("Input Channel(1-11): ",fp);
			if ( InputString(fp, Buffer, 4) != INPUT_ERROR )
			{
				temp = atoi(Buffer);
				if ( temp <= 11 && temp >= 1)
				{
		  			TELNET_EEPROM_Data.WLChannel = temp;
		  			ModifyConfigFlag = 1;    // setting flag
				}
			}
			break;
		case 0xA2:  //ETSI    1-13
			fputs("Input Channel(1-13): ",fp);
			if ( InputString(fp, Buffer, 4) != INPUT_ERROR )
			{
				temp = atoi(Buffer);
				if ( temp <= 13 && temp >= 1)
				{
		  			TELNET_EEPROM_Data.WLChannel = temp;
		  			ModifyConfigFlag = 1;    // setting flag
				}
			}
			break;			
		case 0xA4:  // Japan  1-14
			fputs("Input Channel(1-14): ",fp);
			if ( InputString(fp, Buffer, 4) != INPUT_ERROR )
			{
				temp = atoi(Buffer);
				if ( temp <= 14 && temp >= 1)
				{
		  			TELNET_EEPROM_Data.WLChannel = temp;
		  			ModifyConfigFlag = 1;    // setting flag
				}
			}
			break;					
		default:
			fputs("Input Channel(1-11): ",fp);
			if ( InputString(fp, Buffer, 4) != INPUT_ERROR )
			{
				temp = atoi(Buffer);
				if ( temp <= 11 && temp >= 1)
				{
		  			TELNET_EEPROM_Data.WLChannel = temp;
		  			ModifyConfigFlag = 1;    // setting flag
				}
			}
			break;		
	}
}	

// This subroutine will show "WEP Type" user wants.
// add --- by Jerry 7/17/2003
void PutWEPkeyMode(FILE *fp)
{
	char Buffer[15];
#ifdef WPA_PSK_TKIP
	if ( EEPROM_Data.WLAuthType == 4 )
	{
		sprintf(Buffer,StringFormat3,"Enable (TKIP)");
		fputs(Buffer,fp);   
		fputs("\r\n",fp);
		return;
	}
#endif	
	switch(TELNET_EEPROM_Data.WLWEPType)
    {
       // give message format, such as "%s"
    	case 0:    // Disable WEP 
               sprintf(Buffer,StringFormat3,"Disable");
               break;
    	case 1:    // 64 bit
               sprintf(Buffer,StringFormat3,"64(40)-bit");
               break;
    	case 2:    // 128 bit
               sprintf(Buffer,StringFormat3,"128-bit");
               break;
     }
   
    fputs(Buffer,fp);   
	fputs("\r\n",fp);
}

// This subroutine offers users to input "WEP Key Mode" they want
// add --- by Jerry 7/17/2003
void GetWEPkeyMode(FILE *fp)
{
  char Buffer[10];
  int  temp;

  fputs("WEP Key Setting(1.Disable 2.64(40)-bit 3.128-bit): ",fp);

  //  1 byte saves null char
  if ( InputString(fp, Buffer, 9) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // Disable
              TELNET_EEPROM_Data.WLWEPType =  0;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // 64(40)-bit
              TELNET_EEPROM_Data.WLWEPType =  1;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  3:  // 128-bit
              TELNET_EEPROM_Data.WLWEPType =  2;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

// This subroutine will show "WEP Keyformat" user wants.
// add --- by Jerry 7/17/2003
void PutWEPkeyFormat(FILE *fp)
{
	char Buffer[15];

#ifdef WPA_PSK_TKIP
	if ( EEPROM_Data.WLAuthType == 4 )
	{
		sprintf(Buffer,StringFormat3,"No Format");
		fputs(Buffer,fp);   
  		fputs("\r\n",fp);
  		return;
	}
#endif	
	switch(TELNET_EEPROM_Data.WLWEPKeyType)
	{
		// give message format, such as "%s"
		case 0:    // ASCII 
		       sprintf(Buffer,StringFormat3,"Alphanumeric");
		       break;
		case 1:    // Hex
		       sprintf(Buffer,StringFormat3,"Hexadeciaml");
		       break;
    }
    fputs(Buffer,fp);   
  
	fputs("\r\n",fp);
}

// This subroutine offers users to input "WEP Key Format" they want
// add --- by Jerry 7/17/2003
void GetWEPkeyFormat(FILE *fp)
{
  char Buffer[10];
  int  temp;

  fputs("WEP Key Format(1.Alphanumeric 2.Hexadeciaml): ",fp);

  //  1 byte saves null char
  if ( InputString(fp, Buffer, 9) != INPUT_ERROR )
  {
    // convert data into integer
    temp = atoi(Buffer);

    switch(temp)
    {
      case  1:  // Alphanumeric
              TELNET_EEPROM_Data.WLWEPKeyType =  0;
              ModifyConfigFlag = 1;         // setting flag
              break;
      case  2:  // Hexadeciaml
              TELNET_EEPROM_Data.WLWEPKeyType =  1;
              ModifyConfigFlag = 1;         // setting flag
              break;
      default:
              // will not change previous value
              ;
    }
  }
}

// This subroutine will show "WEP Key" user wants.
// add --- by Jerry 7/17/2003
void PutWEPkey(FILE *fp)
{
	char temp[32]="";
	char Buffer[32+5]="";
	char *nul="";
	int i,weplen;

#ifdef WPA_PSK_TKIP
	if ( EEPROM_Data.WLAuthType == 4 )
	{
		sprintf(Buffer,StringFormat3,"Not set");
		fputs(Buffer,fp);   
  		fputs("\r\n",fp);
  		return;
	}
#endif		
	if(TELNET_EEPROM_Data.WLWEPType == 1){
		strcpy(temp,TELNET_EEPROM_Data.WLWEPKey1);
		weplen=5;
	}	
	else if(TELNET_EEPROM_Data.WLWEPType == 2){
		strcpy(temp,TELNET_EEPROM_Data.WLWEP128Key);
		weplen=13;
	}	
	else
		strcpy(temp,nul);
			
	if(TELNET_EEPROM_Data.WLWEPKeyType == 1){
		for(i=0;i<weplen;i++)
			sprintf(&Buffer[i*2],HexFormat1,temp[i]); 
		
		fputs(Buffer,fp);
		fputs("\r\n",fp);
	}
	else{
	  sprintf(Buffer,StringFormat2,temp);
	  // send to client
	  fputs(Buffer,fp);
	}
}


// This subroutine offers users to input "WEP Key" they want
// add --- by Jerry 7/17/2003
void GetWEPkey(FILE *fp)
{
	char Buffer[30];
	char Message[50];
	int i,nValue;
	
	if((TELNET_EEPROM_Data.WLWEPType == 1) && (TELNET_EEPROM_Data.WLWEPKeyType == 0))		
	{	
		sprintf(Message,"Input WEP Key(Max %d chars): ",5);
		fputs(Message,fp);

		// get wep key
		if ( InputString(fp, Buffer, 6 ) != INPUT_ERROR )
		{
			// length is not zero
			if (strlen(Buffer))
			{
				strcpy(TELNET_EEPROM_Data.WLWEPKey1,Buffer);
				ModifyConfigFlag = 1;     // setting flag
			}
		}
	}	
	if((TELNET_EEPROM_Data.WLWEPType == 1) && (TELNET_EEPROM_Data.WLWEPKeyType == 1))		
	{	
		sprintf(Message,"Input WEP Key(Max %d hex digitals): ",10);
		fputs(Message,fp);

		// get wep key
		if ( InputString(fp, Buffer, 12 ) != INPUT_ERROR )
		{
			// length is not zero
			if (strlen(Buffer))
			{
				for( i = 0; i < 10; i++ )
				{
					sscanf( &Buffer[i], "%1x", &nValue );
					nValue &= 0x0F;
					if( i % 2 == 0 )
						TELNET_EEPROM_Data.WLWEPKey1[i/2] = nValue << 4;
					else
						TELNET_EEPROM_Data.WLWEPKey1[i/2] |= nValue;
				}
				ModifyConfigFlag = 1;     // setting flag
			}
		}
	}	

	if((TELNET_EEPROM_Data.WLWEPType == 2) && (TELNET_EEPROM_Data.WLWEPKeyType == 0))
	{	
		sprintf(Message,"Input WEP Key(Max %d chars): ",13);
		fputs(Message,fp);

		// get wep key
		if ( InputString(fp, Buffer, 14 ) != INPUT_ERROR )
		{
			// length is not zero
			if (strlen(Buffer))
			{
				strcpy(TELNET_EEPROM_Data.WLWEP128Key,Buffer);
				ModifyConfigFlag = 1;     // setting flag
			}
		}
	}	
	if((TELNET_EEPROM_Data.WLWEPType == 2) && (TELNET_EEPROM_Data.WLWEPKeyType == 1))		
	{	
		sprintf(Message,"Input WEP Key(Max %d hex digitals): ",26);
		fputs(Message,fp);

		// get wep key
		if ( InputString(fp, Buffer, 30 ) != INPUT_ERROR )
		{
			// length is not zero
			if (strlen(Buffer))
			{
				for( i = 0; i < 26; i++ )
				{
					sscanf( &Buffer[i], "%1x", &nValue );
					nValue &= 0x0F;
					if( i % 2 == 0 )
						TELNET_EEPROM_Data.WLWEP128Key[i/2] = nValue << 4;
					else
						TELNET_EEPROM_Data.WLWEP128Key[i/2] |= nValue;
				}
				ModifyConfigFlag = 1;     // setting flag
			}
		}
	}	

}

#endif WIRELESS_CARD

// This subroutine offers users to input "File Server Names" they want
// add --- by arius 3/23/2000
void GetFileServerName(FILE *fp)
{
	char Buffer[LENGTH_OF_FS_NAMES];
	char Message[LENGTH_OF_FS_NAMES+4];
	unsigned long LastObjectSeed = 0xFFFFFFFF;
	BYTE            FSName[49],rc;
	BYTE            *FSName1,*cp,*cp1;
	int16           FSCount = 0, count = 0;
	ServerNameTable NTables[100];
	int             Value;
	WORD            FSNameLen,FSNameTotalLen = 0;

	FSName[48] = '\0';

	if ((rc = SearchObjectInit()) == 0)
	{
		fputs("Found out File Server on the network ...\r\n",fp);
		while (!SearchBinaryObjectName(FSName,&LastObjectSeed,FILE_SERVER_OBJECT))
		{
			strcpy(NTables[FSCount].Name,FSName);
			NTables[FSCount].Flag = 0;
			FSCount++;
			sprintf(Message,"%d. %s\r\n",FSCount,FSName);
			fputs(Message,fp);
		}
	}

	if (FSCount)
	{
    	fputs("You can give multi-choice, such as 1 2 \r\n",fp);
    	fputs("Please input numbers: ",fp);

		// length -1: 1 byte saves null char
		if ( InputString(fp, Buffer,LENGTH_OF_FS_NAMES-1) != INPUT_ERROR )
		{
	    	if (strlen(Buffer))
			{
	        	FSName1 = TELNET_EEPROM_Data.FileServerNames;
	        	cp  = Buffer;
	        	cp1 = Buffer;
	        	while(cp != NULL)
	        	{
	          		if ( (cp = strchr(cp,' ')) != NULL )
	            		*cp = '\0';
	          		Value = atoi(cp1);
	
	          		if ( (Value >= 1) && (Value <= FSCount) && !NTables[Value-1].Flag )
	          		{
	            		FSNameLen = strlen(NTables[Value-1].Name);
	            		memcpy(FSName1,NTables[Value-1].Name,FSNameLen);
	            		NTables[Value-1].Flag = 1;  // setting flag
	            		*(FSName1+FSNameLen) = '\0';
	            		OffsetOfFSNameInTelnet[count] = FSNameTotalLen;
	            		FSName1 += FSNameLen+1;
	            		FSNameTotalLen += FSNameLen+1;
//	            		count++;
	          		}
	
	          		if (cp == NULL)
	          			break;
	          		cp++;
	        		cp1 = cp;
	        	}

	       		if (count)
	        	{
	          		*(TELNET_EEPROM_Data.FileServerNames+FSNameTotalLen) = '\0';
	
	          		ModifyConfigFlag = 1;       // setting flag
	          		ServiceFSCountInTelnet = count;
	        	}
			}  // end of if
			else
			{
	        	; // do not chnage any data
	    	}
		}
	}
	else
    	fputs("NetWare File Server Can Not Found on the network\r\n",fp);

}