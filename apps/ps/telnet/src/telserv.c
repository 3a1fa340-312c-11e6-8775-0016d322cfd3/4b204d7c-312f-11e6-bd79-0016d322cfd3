// This module works for telnet server on printer server
// When server starts, the module was called.
// The function of the module will start telnet server, stop telnet server,
// begin telnet's parameter for user login such as open socket, bind socket,
// and listen etc, and deal with each coming packet.
// add --- by arius 3/15/2000

/*
#include "stdio.h"
#include <ctype.h>
#include "ctypes.h"
#include "socket.h"    // AF_INET IPPORT_TELNET SOCK_STREAM AF_LOCAL
#include "usock.h"     // structure 'sp' symbol 'sa' 'refcnt'
*/

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

#include "telserv.h"
#include "option.h"
#include "telutil.h"
#include "menu.h"

#define	AF_LOCAL	3	//form PSRDC

union sp {
        struct sockaddr *sa;
        struct sockaddr_in *in;
};

#define	fileno(fp)	((fp) != NULL ? (fp)->fd : -1)

//TELNET_SERVER_MAIN Thread initiation information
#define TELNET_SERVER_MAIN_TASK_PRI         	20	//ZOT716u2
#define TELNET_SERVER_MAIN_TASK_STACK_SIZE  	4096
static	uint8			TELNET_SERVER_MAIN_Stack[TELNET_SERVER_MAIN_TASK_STACK_SIZE];
static  cyg_thread		TELNET_SERVER_MAIN_Task;
static  cyg_handle_t	TELNET_SERVER_MAIN_TaskHdl;

extern int Network_TCPIP_ON;

// ********* External variable **********
extern  int  QuitMenu;
extern WORD  OffsetOfFSNameInTelnet[MAX_FS]; //Offset of FS name from EEPROM.FileServerNames

extern const TMENU mainmenu;

// send telnet's option
extern const unsigned char DoTerminalOption[];
extern const unsigned char WillEchoOption[];

// ansi command
extern const char ClearScreenCommand[];

// message
extern const char LoginBanner[];
extern const char LoginError[];
extern const char CompanyLogo[];
extern const char HostName[];
extern const char TelnetVersion[];
extern const char ADMINUSER[];
extern const char ADMINPASS[];

#ifdef WIRELESS_CARD
extern diag_flag;
#endif


// ********** GLOBAL VARIABLE ***********
static  int telnetlink = -1;
int     TestingFlag = 0;
EEPROM  TELNET_EEPROM_Data;
int     ModifyConfigFlag;
int     TELNET_MAX_USER = 1;
int     TELNETDUsers = 0;
int     ServiceFSCountInTelnet;
//long    CurrentIP;
struct in_addr CurrentIP;

// Adding  --- By Arius  3/15/2000
// This function will begin service of telnet when printer server was started.
// It will prepare parameter for telnet connection.
void telnetstart(cyg_addrword_t data)
{
	struct sockaddr_in lsocket;
	struct sockaddr_in sa_client;
	int    s;
	FILE   *network;
	char   Buffer[50];
	int16  clen;
	
	while( Network_TCPIP_ON == 0 )
		ppause(100);
	
	if (telnetlink != -1)
	 return;

	lsocket.sin_family = AF_INET;
	lsocket.sin_addr.s_addr = htonl(INADDR_ANY);
	
	lsocket.sin_port = htons(IPPORT_TELNET);
	
	telnetlink = socket(AF_INET,SOCK_STREAM,0);
	bind(telnetlink,(struct sockaddr *)&lsocket,sizeof(lsocket));
	
	listen(telnetlink,1);
	for (;;)
	{
		clen=sizeof(sa_client);
		memset(&sa_client, 0, clen);
		if ((s = accept(telnetlink,(struct sockaddr *) &sa_client, &clen)) == -1)
			break;  /* Service is shutting down */
	
		if (TELNETDUsers >= TELNET_MAX_USER || availmem() != 0)
		{
			// open file will prepare to send message to client
		    network = fdopen(s,"r+t");
		
		    fputs("\r\n This service will offer one user to use it. \r\n",network);
		    sprintf(Buffer," The Current User is [IP:%s]\r\n",inet_ntoa(CurrentIP));
		    fputs(Buffer,network);
		    fclose(network);   // close network
		
//Jesse		    shutdown(s,1);
//Jesse		    close_s(s);
		}
		else
		{
		    CurrentIP.s_addr = sa_client.sin_addr.s_addr;
		    // Spawn a child process
		//        newproc("ServicedForTelnet",1024,TServerMainFunction,s,NULL,NULL,0);
				
			if( TELNET_SERVER_MAIN_TaskHdl != 0 )
					cyg_thread_delete(TELNET_SERVER_MAIN_TaskHdl);
			
			//Create TELNET_SERVER_MAIN Thread
			cyg_thread_create(TELNET_SERVER_MAIN_TASK_PRI,
			              TServerMainFunction,
			              s,
			              "ServicedForTelnet",
			              (void *) (TELNET_SERVER_MAIN_Stack),
			              TELNET_SERVER_MAIN_TASK_STACK_SIZE,
			              &TELNET_SERVER_MAIN_TaskHdl,
			              &TELNET_SERVER_MAIN_Task);
			
			//Start TELNET_SERVER_MAIN Thread
			cyg_thread_resume(TELNET_SERVER_MAIN_TaskHdl);	
		
		}
	}
	
	return;
}


// Adding  --- By Arius  3/15/2000
// This fuction will end service of telnet on that printer server
// It will release all resources telnet server occupied.
int telnetstop(argc,argv,p)
int argc;
char *argv[];
void *p;
{
//Jesse   close_s(telnetlink);
   close(telnetlink);
   telnetlink = -1;
   return 0;
}





// Adding  --- By Arius  3/15/2000
// This fuction will check name, password ,and rights
int CheckNameRights(FILE *network)
{
   char   *cp;
   union  sp sp;
   struct sockaddr tmp;
   char   username[LENGTH_OF_USERNAME],password[LENGTH_OF_PASSWORD+1];
   char   counter = 1;

   username[0] = NULL;
   password[0] = NULL;

   sp.sa = &tmp;
   sp.sa->sa_family = AF_LOCAL;    /* default to AF_LOCAL */

   /* This is one of the two parts of the mbox code that depends on the
    * underlying protocol. We have to figure out the name of the
    * calling station. This is only practical when AX.25 or NET/ROM is
    * used. Telnet users have to identify themselves by a login procedure.
    */
   switch (sp.sa->sa_family)
   {
     case AF_LOCAL:
     case AF_INET:
                  fmode(network,STREAM_BINARY);
                  // give telnet option
                  // setting echo option and terminal type option
                  // IAC + WILL +ECHO
//Jesse                  fwrite(WillEchoOption,1,3,network);
                  fputs(WillEchoOption,network);
                  
                  // IAC + DO + TERMINALTYPE
//Jesse                  fwrite(DoTerminalOption,1,3,network);
					fputs(DoTerminalOption,network);

                  // Prompt system information
//                  fprintf(network,(char *)ClearScreenCommand,0x1b);  // clear screen
//Jesse                  fwrite(LoginBanner,1,strlen(LoginBanner),network);
					fputs(LoginBanner,network);
				
                  // check user information
                  for (;;)
                  {
//                     fputs("login: ",network);
//                       // -1: save a null byte
//                     if (InputString(network,username,LENGTH_OF_USERNAME-1) == EOF)
//                       return -1;
//                     if (username == '\0')
//                       continue;
					 strcpy(username,"ADMIN");
                     // Does counter exceed 3?
                     if (counter <= MAXCounterForKeyinPassword)
                     {

#if defined(WIRELESS_CARD)
	if((diag_flag == 1)  && (EEPROM_Data.SPECIAL_OEM == 0x02))return 0;
#endif

                       fputs("Password: ",network);
                       if (InputPassword(network,password,LENGTH_OF_PASSWORD) == EOF)
                         return -1;

                       if ( UsersLogin(username,password) != -1)
                         return 0;

//Jesse                       fwrite(LoginError,1,strlen(LoginError),network);
						fputs(LoginError,network);
                       counter++;
                     }
                     else
                     {
                       TELNETDUsers--;
                       return -1;
                     }
                  }
   }
   return 0;
}



// Adding  --- By Arius  3/15/2000
// This fuction's job will process packets from end of client.
// It also includes checking user name and password , rights, etc.
void TServerMainFunction(cyg_addrword_t data)
{
//Jesse   struct usock *up;
   FILE   *network;
   struct timeval rcv_timeout;
   int 	s =	data;
   

//Jesse   if (p == NULL)
     network = fdopen(s,"r+t");
//Jesse   else
//Jesse     network = (FILE *)p;

   TELNETDUsers++;

	rcv_timeout.tv_usec = 0;
	rcv_timeout.tv_sec = 180;

	setsockopt (network->fd, 
				SOL_SOCKET, 
				SO_RCVTIMEO,
				(char *)&rcv_timeout,
				sizeof(rcv_timeout));

   // check name, password, and rights of the remote station
   if (CheckNameRights(network) == -1)
   {
     fclose(network);
     return;
   }

   // copy content from original EEPROM
   memcpy(&TELNET_EEPROM_Data,&EEPROM_Data,sizeof(EEPROM_Data));
   ServiceFSCountInTelnet = ServiceFSCount;
   memcpy(OffsetOfFSNameInTelnet,OffsetOfFSName,sizeof(OffsetOfFSName));

   ModifyConfigFlag = 0;

   // offered a menu, all of fuctions, to select for user
   QuitMenu = 0;           // setting flag
   doMenu(network,&mainmenu);

   /* nasty hack! we may have screwed up reference count */
   /* by invoking newproc("smtp_send",....); Fudge it!   */
//Jesse   if ((up = itop(fileno(network))) != NULL)
//Jesse     up->refcnt = 1;
   fclose(network);
   
   cyg_thread_exit();
}
