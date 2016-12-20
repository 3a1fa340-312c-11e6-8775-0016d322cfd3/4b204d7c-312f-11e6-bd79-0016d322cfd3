/*	$NetBSD: tftpd.c,v 1.11 1997/11/06 00:08:02 lukem Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton
 * <guyton@rand-unix>.
 */

/*
#include <dos.h>

#include "global.h"
#include "psglobal.h"
#include "socket.h"
#include "tftp.h"
#include "swap.h"
#include "tftpd.h"
#include "s3c2510.h"
#include "armisr.h"
*/

#include <cyg/kernel/kapi.h>
#include "network.h"

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "led.h"
#include "ledcode.h"
#include "tftp.h"
#include "tftpd.h"
#include "prnqueue.h"
#include "nps.h"


#ifdef UNIXUTIL_TFTP

extern BYTE *AddrToIP(char *IPAddr);

extern char  Hostname[];
extern uint8 PSMode;
extern uint8 CurSetupPassword[SETUP_PASSWD_LEN+1]; //12/30/99
extern uint8 PSUpgradeMode;

#define IsSpace(x)  ((x) == ' ' || (x) == '\t')

static void tftp(struct tftphdr *tp,int size);
static void nak(int error,int par);
static void SendConfigFile(void);
static void SendZeroByteFile(void);
static void RecvConfigFile(void);
static int16  FillConfigFile(char *configbuf);
static int16  ReadConfigData(char *configbuf,int16 bufsize );
static BYTE *SkipLineTitle(int RowNum,char *LineInfo);
static char IsDHCPOn(char *String);
static char IsPsEnable(char *String);
static void TftpSetEEPROM(void);

#ifdef DEF_PRINTSPEED
char SetPrinterMode(char *String);
char *GetPrinterMode(char PrinterMode);
#endif DEF_PRINTSPEED

static char	buf[PKTSIZE] __attribute__ ((aligned(4)));
static char	ackbuf[PKTSIZE] __attribute__ ((aligned(4)));
static struct	sockaddr_in fromsock;
static int	    fromlen;
static int     lsocket;
//static BYTE   *LineInfo[CONFIG_FILE_LINE_NUM+1];
static BYTE   *LineInfo[CONFIG_FILE_LINE_NUM+2]; //include password
static char    Tftpdprintername[]="Printer1Name";

/*
static const tftpd_errmsg Tftpderrmsgs[] = {
//	{ EUNDEF,   "Undefined error code" },
#ifdef CODE1
	{ EREAD,  	"Invalid filename. (" TFTP_CONFIG_LNAME ")" },
	{ EWRITE,	"Invalid filename. (" TFTP_CONFIG_LNAME ", " TFTP_UPGRADE_LNAME ")"},
	{ EWRITEC1,	"Invalid filename. (" TFTP_CONFIG_LNAME ", " TFTP_UPGRADE_C1LNAME ")"},
#else
 #if (NUM_OF_PRN_PORT >= 3)	//10/28/98	THREE PORT
	{ EREAD,	"Invalid filename. (" TFTP_CONFIG_LNAME ", reset, upgrade, lpt1test, lpt2test, lpt3test)"},
 #elif (NUM_OF_PRN_PORT >= 2) //10/28/98 TWO PORT
	{ EREAD,	"Invalid filename. (" TFTP_CONFIG_LNAME ", reset, upgrade, lpt1test, lpt2test)"},
 #else	//ONE PORT
	{ EREAD,	"Invalid filename. (" TFTP_CONFIG_LNAME ", reset, upgrade, lpt1test)"},
 #endif
	{ EWRITE,	"Invalid filename. (" TFTP_CONFIG_LNAME ", " TFTP_UPGRADE_LNAME ")"},
	{ EWRITEC1,	"Invalid filename. (" TFTP_CONFIG_LNAME ", " TFTP_UPGRADE_C1LNAME ")"},
#endif
//	{ ENOSPACE, "Disk full or allocation exceeded" },
	{ EBADOP,   "Illegal TFTP operation." },
	{ EFFORMAT, "File format error at line %02d !"},
	{ EFTOOBIG,	"File too large."},
	{ EBADMODE, "Invalid TFTP transfer mode."},
	{ EUPGRADE, "BOX is upgrading."},
	{ EWRFLASH, "Upgrade flash error."},
//	{ EBADID,   "Unknown transfer ID" },
//	{ EEXISTS,  "File already exists" },
//	{ ENOUSER,  "No such user" },
	{ EBADPWD,  "Invalid Password." },			//12/31/99
	{ EBADCODE2,"Wrong firmware." },
	{ EERCMD,"Error command." },					//JESSE 6/26/2003
	{ -1,		0 }
};
*/

static const tftpd_errmsg Tftpderrmsgs[] = {
 #if (NUM_OF_PRN_PORT >= 3)	//10/28/98	THREE PORT
	{ EREAD,	"Invalid filename. (" TFTP_CONFIG_LNAME ", reset, upgrade, lpt1test, lpt2test, lpt3test)"},
 #elif (NUM_OF_PRN_PORT >= 2) //10/28/98 TWO PORT
	{ EREAD,	"Invalid filename. (" TFTP_CONFIG_LNAME ", reset, upgrade, lpt1test, lpt2test)"},
 #else	//ONE PORT
	{ EREAD,	"Invalid filename. (" TFTP_CONFIG_LNAME ", reset, upgrade, lpt1test)"},
 #endif
	{ EWRITE,	"Invalid filename. (" TFTP_CONFIG_LNAME ", " TFTP_UPGRADE_LNAME ")"},
	{ EWRITEC1,	"Invalid filename. (" TFTP_CONFIG_LNAME ", " TFTP_UPGRADE_C1LNAME ")"},

//	{ ENOSPACE, "Disk full or allocation exceeded" },
	{ EBADOP,   "Illegal TFTP operation." },
	{ EFFORMAT, "File format error at line %02d !"},
	{ EFTOOBIG,	"File too large."},
	{ EBADMODE, "Invalid TFTP transfer mode."},
	{ EUPGRADE, "BOX is upgrading."},
	{ EWRFLASH, "Upgrade flash error."},
//	{ EBADID,   "Unknown transfer ID" },
//	{ EEXISTS,  "File already exists" },
//	{ ENOUSER,  "No such user" },
	{ EBADPWD,  "Invalid Password." },			//12/31/99
	{ EBADCODE2,"Wrong firmware." },
	{ EERCMD,   "Error command." },					//JESSE 6/26/2003
	{ -1,		0 }
};


const char *EnDisable[] = { "Disable","Enable" };
const char *DHCPMsg[] = {"ON", "OFF"};

#ifdef DEF_PRINTSPEED
const char *PrinterMode[] = {"Fast","Normal","Slow"};
#endif DEF_PRINTSPEED

/*
tftpdreqcmd TftpdRequestCmd[] = {
{ MODE_ASCII | METHOD_RRQ | METHOD_WRQ,	TFTP_CONFIG_UNAME,  NULL  },
{ MODE_BINARY | METHOD_WRQ,				TFTP_UPGRADE_UNAME, TftpDLFlash },
{ MODE_BINARY | METHOD_WRQ,				TFTP_UPGRADE_C1NAME, TftpDLCode1 },
{ MODE_ASCII | METHOD_RRQ | METHOD_WRQ,	TFTP_CONFIG_UNAME,  NULL  },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"ERASEEE",			TftpEraseEE },
#ifndef CODE1
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"ERASEC2",			TftpEraseCode2 },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"REBOOT",      		TftpRebootBox },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"RESET",      		TftpResetBox },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"UPGRADE",    		TftpUpgradeBox },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"LPT1TEST",   		TftpPrinter1Test},
#if (NUM_OF_PRN_PORT >= 2) //10/28/98 TWO PORT
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"LPT2TEST",   		TftpPrinter2Test},
#endif
#if (NUM_OF_PRN_PORT >= 3) //10/28/98 THREE PORT
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"LPT3TEST",   		TftpPrinter3Test},
#endif
#endif
{ 0, NULL, NULL }
};
*/

tftpdreqcmd TftpdRequestCmd[] = {
{ MODE_ASCII | METHOD_RRQ | METHOD_WRQ,	TFTP_CONFIG_UNAME,  NULL  },
{ MODE_BINARY | METHOD_WRQ,				TFTP_UPGRADE_UNAME, TftpDLFlash },
{ MODE_BINARY | METHOD_WRQ,				TFTP_UPGRADE_C1NAME, TftpDLCode1 },
{ MODE_ASCII | METHOD_RRQ | METHOD_WRQ,	TFTP_CONFIG_UNAME,  NULL  },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"ERASEEE",			TftpEraseEE },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"ERASEC2",			TftpEraseCode2 },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"REBOOT",      		TftpRebootBox },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"RESET",      		TftpResetBox },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"UPGRADE",    		TftpUpgradeBox },
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"LPT1TEST",   		TftpPrinter1Test},
#if (NUM_OF_PRN_PORT >= 2) //10/28/98 TWO PORT
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"LPT2TEST",   		TftpPrinter2Test},
#endif
#if (NUM_OF_PRN_PORT >= 3) //10/28/98 THREE PORT
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"LPT3TEST",   		TftpPrinter3Test},
#endif
{ MODE_ASCII | MODE_BINARY | METHOD_RRQ,"BRIDGE",    		TftpBridge },
{ 0, NULL, NULL }
};


const char *Tftpdlinename[]={
//if add/delete any item must modify CONFIG_FILE_NORMAL_NUM (tftpd.h)
	"BoxName",				   //1
	"BoxVersion",			   //2
	"BoxNodeID",			   //3
	"DHCP/BOOTP",              //4
	"BoxIPAddress",			   //5
	"Gateway",				   //6
	"SubnetMask",			   //7
	"NetWare"				   //8
#ifdef DEF_PRINTSPEED
	,"PrinterMode"			   //9
#endif DEF_PRINTSPEED
	,"Password"                //10
//	"UNIX",					   //9
//	"WINDOWS"				   //10
};

static const char *Tftp_config1=
"%02d %-12s : %.18s\n"                                 //BoxName
"%02d %-12s : %X.%02X (fixed)\n"                       //BoxVersion
"%02d %-12s : %02X-%02X-%02X-%02X-%02X-%02X (fixed)\n" //BoxNodeID
"%02d %-12s : %s\n"                                    //DHCP/BOOTP
"%02d %-12s : %d.%d.%d.%d\n"                           //BoxIPAddress
"%02d %-12s : %d.%d.%d.%d\n"                           //Gateway
"%02d %-12s : %d.%d.%d.%d\n"                           //SubnetMask
"%02d %-12s : %s\n"                                    //NetWare
#ifdef DEF_PRINTSPEED
"%02d %-12s : %s (Fast, Normal, Slow)\n"               //PrinterMode
#endif DEF_PRINTSPEED
//"%02d %-12s : %s\n"                                    //UNIX
//"%02d %-12s : %s\n"                                    //WINDOWS
;
#ifdef LPR_Q_RENAME		//2003Dec16
static const char *Tftp_config2="%02d %-12s : %s\n"; //"PrinterName"
#else
static const char *Tftp_config2="%02d %-12s : %s (fixed)\n"; //"PrinterName"
#endif

//---------------Utility Printer Test Data Record 1----------------------
const BYTE PrintTestData1[] ="\
******************************************************************************\r\n\
     Device Name: %-18s       Node ID: %02X-%02X-%02X-%02X-%02X-%02X\r\n\
\r\n\
     Flash Version: %02X.%02X                  Release Version: %04d\r\n\
******************************************************************************\r\n\r\n";
#define _PortMask(Port) 		(0x03<<(Port<<1))
#define _PrintPortStatus(Port)	((ReadPrintStatus()&_PortMask(Port)) >> (Port<<1))
//Send Print test data to print queue !
BYTE SendPrintTestData (int Port)
{
	PrnBuf *PrintBuffer;
	BYTE PrintStatus;
	BYTE *pTemp;

	if((PrintStatus = _PrintPortStatus(Port)) != 0) return (PrintStatus);

	if(PrnGetPrinterStatus(Port) != PrnNoUsed) return (PRINTER_BUSY);
	if(PrnGetAvailQueueNO(Port)  < 1) return (PRINTER_BUSY);
	
	PrnSetFtpInUse(Port);
	
	//Print Test Data 1
	PrintBuffer = PrnGetInQueueBuf(Port);

	sprintf(PrintBuffer->data, PrintTestData1,
 	        _BoxName,				//Device Name
	        MyPhysNodeAddress[0],	//Node ID
	        MyPhysNodeAddress[1],
	        MyPhysNodeAddress[2],
	        MyPhysNodeAddress[3],
	        MyPhysNodeAddress[4],
	        MyPhysNodeAddress[5],
	        CURRENT_MAJOR_VER, CURRENT_MINOR_VER,		//Version
	        CURRENT_BUILD_VER	    					//RC version
	);
	pTemp = print_large(PrintBuffer->data+strlen(PrintBuffer->data),"P R I N T",0);
	print_large(pTemp,"T E S T",1);

	PrintBuffer->size = strlen(PrintBuffer->data);
	PrnPutOutQueueBuf(Port,PrintBuffer,PRN_Q_NORMAL);
	
	PrnSetNoUse(Port);
	
	return(PRINTER_FREE);
}

#ifdef ARCH_ARM
extern int Network_TCPIP_ON;
#endif /* ARCH_ARM */

//void tftpd(int nouse,void *nouse1,void *nouse2)
void tftpd(cyg_addrword_t data)
{
	register struct tftphdr *tp;
	register int size;
	struct sockaddr_in tftpd_lsocket;
	int	tftpd_socket;
	char Tmpbuf[3];
	struct timeval rcv_timeout;
	char buf[500] = {0};

#ifdef ARCH_ARM
    while (Netowkr_TCPIP_ON == 0)
        ppause (100);
#endif /* ARCH_ARM */

	tftpd_lsocket.sin_family = AF_INET;
	tftpd_lsocket.sin_addr.s_addr = htonl(INADDR_ANY);
	tftpd_lsocket.sin_port = htons(IPPORT_TFTPD);
	tftpd_lsocket.sin_len = sizeof(tftpd_lsocket);

	tftpd_socket = socket( AF_INET, SOCK_DGRAM, 0 );
//	setsocketopt(tftpd_socket,SO_RCV_BLOCK);

	lsocket = socket( AF_INET, SOCK_DGRAM, 0 );
//	setsocketopt(lsocket,SO_RCV_TIMEOUT);
	rcv_timeout.tv_usec = 500;
	rcv_timeout.tv_sec = 1;
	setsockopt (lsocket, 
				SOL_SOCKET, 
				SO_RCVTIMEO,
				&rcv_timeout,
				sizeof(struct timeval));

	if(bind(tftpd_socket, (struct sockaddr *)&tftpd_lsocket,sizeof(tftpd_lsocket)) < 0) {
//		close_s(tftpd_socket);
//		close_s(lsocket);
		close(tftpd_socket);
		close(lsocket);
		return;
	}

	sprintf(Tmpbuf,"%02d",CURRENT_PS_MODEL);
	TftpdRequestCmd[1].Cmd[3] = Tmpbuf[0];  //MPSXX.BIN
	TftpdRequestCmd[1].Cmd[4] = Tmpbuf[1];  //MPSXX.BIN

	TftpdRequestCmd[2].Cmd[3] = Tmpbuf[0];  //MPSXXLO.BIN
	TftpdRequestCmd[2].Cmd[4] = Tmpbuf[1];  //MPSXXLO.BIN

	for (;;) {
		fromlen = sizeof(fromsock);
		if( (size = recvfrom(tftpd_socket, buf, sizeof(buf), 0,
		      (struct sockaddr *)&fromsock, &fromlen)) < 0) continue;

		tp = (struct tftphdr *)buf;
		tp->th_opcode = ntohs(tp->th_opcode);
		if(tp->th_opcode == RRQ || tp->th_opcode == WRQ) {
			tftp(tp, size);
		}
        sys_check_stack();
	}
}

//
// Handle initial connection protocol.
//
void tftp(struct tftphdr *tp,int size)
{
	register char *cp;
	int first = 1, ecode;
	char *filename, *mode = NULL; /* XXX gcc */
	int CmdIndex;

	filename = cp = tp->th_stuff;
again:
	while (cp < buf + size) {
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
		nak(EBADOP,0);
		return;
	}

	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}

	//mode string
	strupr(mode);

	strupr(filename);
	for( CmdIndex=0; TftpdRequestCmd[CmdIndex].Cmd; CmdIndex++ ) {
		if( !strcmp( TftpdRequestCmd[CmdIndex].Cmd, filename ) )
			break;
	}

	// tftp put command function use get command , it is error need to return. 
	if( !(TftpdRequestCmd[CmdIndex].attrib & tp->th_opcode) )		//JESSE 6/26/2003
	{		
		nak( EERCMD ,0);
		return;
	}
	

	if( TftpdRequestCmd[CmdIndex].Cmd == NULL )
	{
		if( tp->th_opcode == WRQ ){
			if(!strncmp(filename, "MPS", 3))
				nak( EWRITE, CURRENT_PS_MODEL );
			else
				nak( EWRITEC1, CURRENT_PS_MODEL );
		}
		else
			nak( EREAD, 0 );
		return;
	}

	// check access mode
	if( strcmp( ASCII_TRANSFER_MODE, mode ) == 0 )
	{
		if( ( TftpdRequestCmd[CmdIndex].attrib & MODE_ASCII ) == 0 )
		{
			nak( EBADMODE, 0 );
			return;
		}
	}
	else
	{
		if( ( TftpdRequestCmd[CmdIndex].attrib & MODE_BINARY ) == 0 )
		{
			nak( EBADMODE, 0 );
			return;
		}
	}

	if( CmdIndex == 0 ) 			// CONFIG.TXT
	{
		if( tp->th_opcode == WRQ )
			RecvConfigFile();
		else
			SendConfigFile();
	}
	else
	{
		if( tp->th_opcode == RRQ )
			SendZeroByteFile();
		(TftpdRequestCmd[CmdIndex].Func)();
	}
	return;
}

//*
//* Send the requested file.
//*
void SendConfigFile(void)
{
	struct tftphdr *dp;
	register struct tftphdr *ap;    /* ack packet */
	register int size, n;
	volatile int block;
	int16 TryCount;

	dp = (struct tftphdr *)buf;
	size = FillConfigFile(dp->th_data);
	ap = (struct tftphdr *)ackbuf;

	block = 1;
	TryCount = 0;
	dp->th_opcode = htons((WORD)DATA);
	dp->th_block = htons((WORD)block);

	do{
		if(sendto(lsocket,
		          dp,
		          size + 4,
		          0,
		          (struct sockaddr *)&fromsock,
		          sizeof(fromsock)) < 0)  return;
//		kwait(NULL);
		cyg_thread_yield();
		fromlen = sizeof(fromsock);
		n = recvfrom(lsocket,
		              ackbuf,
		              sizeof(ackbuf),
		              0,
		              (struct sockaddr *)&fromsock,
		              &fromlen );
	}while((n < 0 || ntohs(ap->th_opcode) != ACK) && TryCount++ < TFTPD_TRY_TIMES);
}

void SendZeroByteFile(void)
{
	register int size, n;
	struct tftphdr dp;
	struct tftphdr *ap;
	int16 TryCount;

	ap = (struct tftphdr *)ackbuf;

	dp.th_opcode = htons((WORD)DATA);
	dp.th_block = htons((WORD)1);

	TryCount = 0;
	do{
		if(sendto(lsocket,&dp,4,0,
		          (struct sockaddr *)&fromsock,
		          sizeof(fromsock)) < 0) return;
		n = recvfrom(lsocket,
		              ackbuf,
		              sizeof(ackbuf),
		              0,
		              (struct sockaddr *)&fromsock,
		              &fromlen );
	}while((n < 0 || ntohs(ap->th_opcode) != ACK) && TryCount++ < TFTPD_TRY_TIMES);
}

//*
//* Receive a file.
//*
void RecvConfigFile(void)
{
	struct tftphdr *dp;
	register struct tftphdr *ap;    /* ack buffer */
	register int16 n;
	int16 block;
	int16 TryCount;

	dp = (struct tftphdr *)buf;
	ap = (struct tftphdr *)ackbuf;

	block = 0;
	TryCount = 0;
	do {
		ap->th_opcode = htons((WORD)ACK);
		ap->th_block = htons((WORD)block);
		if(sendto(lsocket,
		          ackbuf,
	    	      4,
		          0,
		          (struct sockaddr *)&fromsock,
		          sizeof(fromsock)) < 0)  return;
//		kwait(NULL);
		cyg_thread_yield();
		fromlen = sizeof(fromsock);
		n = recvfrom(lsocket,
    		          dp,
        	    	  PKTSIZE,
        		      0,
	            	  (struct sockaddr *)&fromsock,
		              &fromlen );
	} while (n < 0 && ++TryCount < TFTPD_TRY_TIMES);
	if(n < 0 ) return;

	if(n >= PKTSIZE) {
		nak(EFTOOBIG,0);
		return;
	}

	if((n = ReadConfigData(dp->th_data,n-4)) != 0) {
//		nak(EFFORMAT,n); move to ReadConfigData() 12/31/99
		return;
	}

	block++;
	ap->th_opcode = htons((WORD)ACK);    // send the "final" ack
	ap->th_block = htons((WORD)(block));
	sendto(lsocket,
	       ackbuf,
	   	   PKTSIZE,
	       0,
	       (struct sockaddr *)&fromsock,
	       sizeof(fromsock));

	TftpSetEEPROM();
}

void TftpSetEEPROM(void)
{
	BYTE NewPSMode = EEPROM_Data.PrintServerMode; //Charles 2001/03/19
	BYTE NeedReboot = 0;
	int i;

	memcpy(_BoxName,LineInfo[0],LENGTH_OF_BOX_NAME);
	memcpy(Hostname,_BoxName,LENGTH_OF_BOX_NAME); //9/2/99 Simon added

	NCOPY32( _BoxIPAddress, LineInfo[4] );
	NCOPY32( _BoxGatewayAddress, LineInfo[5] );
	NCOPY32( _BoxSubNetMask, LineInfo[6] );
	if(LineInfo[7][0])
		NewPSMode |= PS_NETWARE_MODE;
	else
		NewPSMode &= ~(PS_NETWARE_MODE);
//	if(LineInfo[8][0]) NewPSMode  |= PS_UNIX_MODE;
//	if(LineInfo[9][0]) NewPSMode  |= PS_WINDOWS_MODE;

#ifdef DEF_PRINTSPEED
	if(EEPROM_Data.PrinterSpeed != LineInfo[8][0])
	{
//		NeedReboot = 1;
		EEPROM_Data.PrinterSpeed = LineInfo[8][0];
		if( EEPROM_Data.PrinterSpeed > 2 )
			EEPROM_Data.PrinterSpeed = 2;
	}
#endif DEF_PRINTSPEED

	if(LineInfo[3][0] != 0) // Charles 2001/09/05
	{
		NeedReboot = 1;
		NewPSMode |= PS_DHCP_ON;
	}
	else
		NewPSMode &= ~(PS_DHCP_ON);

	if(NewPSMode != EEPROM_Data.PrintServerMode) {
		EEPROM_Data.PrintServerMode = NewPSMode; //5/8/98
		NeedReboot = 1;
	}

#ifdef LPR_Q_RENAME
	for (i=0; i<NUM_OF_PRN_PORT; i++)
		memcpy(_LPRQueueName(i),LineInfo[9+i],LPRQUEUENAME_LENGTH);
#endif

	ppause(10);

	UseEEPROMIP();
	if(WriteToEEPROM(&EEPROM_Data) != 0) ErrLightOnForever(LED_WR_EEPROM);
	if(NeedReboot) {
//		BOOTSTRAP();
		Reset();
	}
}

void TftpEraseEE(void)
{
	cli();
    #if defined(N716U2W)
	vErase_QC0_Default();
    #endif
	vEraseEEP();
	vEraseDefault();
	sti();
}

void TftpEraseCode2(void)
{
	cli();
	vEraseCode2();
	sti();
}

void TftpRebootBox(void)
{
//	REBOOT();
	Reset();
}

void TftpResetBox(void)
{
//	BOOTSTRAP();
	Reset();
}

void TftpUpgradeBox(void)
{
}

void TftpPrinter1Test(void)
{
	SendPrintTestData(0);
}

void TftpPrinter2Test(void)
{
#if (NUM_OF_PRN_PORT >= 2)
	SendPrintTestData(1);
#endif
}

void TftpPrinter3Test(void)
{
#if (NUM_OF_PRN_PORT >= 3)
	SendPrintTestData(2);
#endif
}

//temp extern char BridgeMode;

void TftpBridge(void)
{
//temp	BridgeMode = 1;
}

//
// * Send a nak packet (error message).
// * Error code passed in is one of the
// * standard TFTP codes, or a UNIX errno
// * offset by 100.
//
void nak(int error,int par)
{
	register struct tftphdr *tp;
	int length;
	tftpd_errmsg far* pe;

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((WORD)ERROR);
	tp->th_code = htons((WORD)error);
	for(pe = Tftpderrmsgs; pe->e_code >= 0; pe++)
		if(pe->e_code == error)
			break;
	sprintf(tp->th_msg,pe->e_msg,par);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;

	sendto(lsocket,
	       tp,
	       length,
	       0,
	       (struct sockaddr *)&fromsock,
	       sizeof(fromsock));
}

int16 FillConfigFile(char *configbuf)
{
	BYTE IPAddress[4];
	BYTE SubnetMask[4];
	BYTE Gateway[4];
	int16 buflen,i;

	memcpy( IPAddress, EEPROM_Data.BoxIPAddress, 4 );
	memcpy( SubnetMask, EEPROM_Data.SubNetMask, 4 );
	memcpy( Gateway, EEPROM_Data.GetwayAddress, 4 );

	sprintf(configbuf,Tftp_config1,
	        1,Tftpdlinename[0],EEPROM_Data.BoxName,
	        2,Tftpdlinename[1],CURRENT_MAJOR_VER,CURRENT_MINOR_VER,
	        3,Tftpdlinename[2],EEPROM_Data.EthernetID[0],EEPROM_Data.EthernetID[1],
	                           EEPROM_Data.EthernetID[2],EEPROM_Data.EthernetID[3],
	                           EEPROM_Data.EthernetID[4],EEPROM_Data.EthernetID[5],
			4,Tftpdlinename[3],(PSMode & PS_DHCP_ON ? DHCPMsg[0]:DHCPMsg[1]),
	        5,Tftpdlinename[4],IPAddress[0],IPAddress[1],
	                           IPAddress[2],IPAddress[3],
	        6,Tftpdlinename[5],Gateway[0],Gateway[1],
	                           Gateway[2],Gateway[3],
			7,Tftpdlinename[6],SubnetMask[0],SubnetMask[1],
	                           SubnetMask[2],SubnetMask[3],
	        8,Tftpdlinename[7],EnDisable[EEPROM_Data.PrintServerMode & PS_NETWARE_MODE ?1:0]
#ifdef DEF_PRINTSPEED
			,
			9,Tftpdlinename[8],GetPrinterMode(EEPROM_Data.PrinterSpeed)
#endif DEF_PRINTSPEED
//	        9,Tftpdlinename[8],EnDisable[EEPROM_Data.PrintServerMode & PS_UNIX_MODE    ?1:0],
//	       10,Tftpdlinename[9],EnDisable[EEPROM_Data.PrintServerMode & PS_WINDOWS_MODE ?1:0]
	);

	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		Tftpdprintername[7] = i+'1';
		buflen = strlen(configbuf);
		sprintf(configbuf+buflen,Tftp_config2,i+1+CONFIG_FILE_NORMAL_NUM,
		        Tftpdprintername,PrnGetPrinterName(i));
	}
	if(CurSetupPassword[0]) {
		buflen = strlen(configbuf);
		sprintf(configbuf+buflen,"%02d %-12s : [",
		        (CONFIG_FILE_NORMAL_NUM+NUM_OF_PRN_PORT+1)
		        ,Tftpdlinename[CONFIG_FILE_NORMAL_NUM]);
		buflen = strlen(configbuf);
		for(i = 0; i < SETUP_PASSWD_LEN; i++) configbuf[buflen+i] = '*';
		strcpy((configbuf+buflen+SETUP_PASSWD_LEN),"]\n");
	}
#ifdef PC_OUTPUT
	while(strlen(configbuf) >= PKTSIZE) {
		printf("(TFTPD.C) Error: bufsize < PKTSIZE !\n");
	}
#endif PC_OUTPUT
	return (strlen(configbuf));
}

int16 ReadConfigData(char *configbuf,int16 bufsize )
{
	WORD RowNum,ColNum,CurBufPos;
	BYTE *CurDataPos,*DHCPPos;
	BYTE *IPAddr;
	WORD i;
	int8 rc;
	BYTE NeedCheckPwd = (CurSetupPassword[0]?1:0);

	if(*configbuf == '\0') return 1;  //data error

	RowNum = 0;
	CurBufPos = 0;
	while(CurBufPos < bufsize && RowNum <= (CONFIG_FILE_LINE_NUM+1)) {
		LineInfo[RowNum] = configbuf+CurBufPos;
		while(configbuf[CurBufPos] != 0x0d && configbuf[CurBufPos] != 0x0a)
			CurBufPos++;
		while(configbuf[CurBufPos] == 0x0d || configbuf[CurBufPos] == 0x0a) {
			configbuf[CurBufPos++] = '\0';
		}
		RowNum++;
	}

	RowNum -= NeedCheckPwd;
	rc = 0;
	for( i = 0 ; i < RowNum; i++) {
		if((CurDataPos = SkipLineTitle(i+1,LineInfo[i])) == NULL) {
			rc = i+1;
			goto TftpRowErr;
		}
		switch(i) {
			case 0: //BoxName
				ColNum = 0;
			//	while(!IsSpace(*CurDataPos) && *CurDataPos != '\0') {
				while(*CurDataPos != '\0') {
					LineInfo[i][ColNum++] = *(CurDataPos++);
				}
				LineInfo[i][ColNum] = '\0';
				break;
			case 1:	//BoxVersion
			case 2:	//BoxNodeID
				break;
			case 3: //DHCP/BOOTP
				if((LineInfo[i][0] = IsDHCPOn(CurDataPos)) < 0) {
					rc = i+1;
					goto TftpRowErr;
				}
				break;
			case 4:	//BoxIPAddress
			case 5: //Gateway
			case 6:	//Mask
				if((IPAddr = AddrToIP(CurDataPos)) == NULL) {
					rc = i+1;
					goto TftpRowErr;
				}
				memcpy( LineInfo[i], IPAddr, 4 );
				break;
			case 7: //Netware
//			case 8: //Unix
//			case 9: //Windows
				if((LineInfo[i][0] = IsPsEnable(CurDataPos)) < 0) {
					rc = i+1;
					goto TftpRowErr;
				}
				break;
#ifdef DEF_PRINTSPEED
			case 8: //Printer Mode
				if((LineInfo[i][0] = SetPrinterMode(CurDataPos)) < 0) {
					rc = i+1;
					goto TftpRowErr;
				}
				break;
			case 9: //Printer1Name
			case 10: //Printer2Name
			case 11: //Printer3Name
#ifdef LPR_Q_RENAME
				ColNum = 0;
				while(*CurDataPos != '\0') {
					LineInfo[i][ColNum++] = *(CurDataPos++);
				}
				LineInfo[i][ColNum] = '\0';
#endif				
				break;
#else
			case 8: //Printer1Name
			case 9: //Printer2Name
			case 10: //Printer3Name
#ifdef LPR_Q_RENAME
				while(*CurDataPos != '\0') {
					LineInfo[i][ColNum++] = *(CurDataPos++);
				}
				LineInfo[i][ColNum] = '\0';
#endif				
				break;
#endif DEF_PRINTSPEED
		}
	}

	if(RowNum != CONFIG_FILE_LINE_NUM) {
		rc = (RowNum < CONFIG_FILE_LINE_NUM?RowNum+1:RowNum);
		goto TftpRowErr;
	}

	if(NeedCheckPwd) {
		if((CurDataPos = SkipLineTitle(i+1,LineInfo[i])) == NULL ||
		   *CurDataPos != '[')
		{
			rc = i+1;
			goto TftpRowErr;
		}
		CurBufPos = strlen(++CurDataPos)-1;
		while((int16)CurBufPos >= 0 && CurDataPos[CurBufPos] == ' ') CurBufPos--;
		if((int16)CurBufPos < 0 || CurDataPos[CurBufPos] != ']') {
			//no end mark ']'
			rc = i+1;
			goto TftpRowErr;
		}
		CurDataPos[CurBufPos] = '\0';
		for(i = 0 ; i < CurBufPos; i++) {
			if(CurDataPos[i] == ' ') {
				CurDataPos[i] = '\0';
				break;
			}
		}
		if(strcmp(CurSetupPassword,CurDataPos)) {
			nak(EBADPWD,i+1);
			return 1;
		}
	}
TftpRowErr:
	if(rc) nak(EFFORMAT,rc);

	return rc;
}

BYTE *SkipLineTitle(int RowNum,char *LineInfo)
{
	char TmpBuf[3],TitleLen;
	char *TmpLineName;

	sprintf(TmpBuf,"%02d",RowNum);
	if(RowNum > CONFIG_FILE_NORMAL_NUM) {
		if(RowNum == CONFIG_FILE_LINE_NUM+1) {
			//Password
			TmpLineName = Tftpdlinename[CONFIG_FILE_NORMAL_NUM];
		} else {
			TmpLineName = Tftpdprintername;
			TmpLineName[7] = (RowNum - CONFIG_FILE_NORMAL_NUM)+'0';
		}
	}
	else TmpLineName = Tftpdlinename[RowNum-1];

	TitleLen = strlen(TmpLineName);
	if(LineInfo[0] != TmpBuf[0] ||
		LineInfo[1] != TmpBuf[1] ||
		LineInfo[2] != ' ' ||
		memcmp(TmpLineName,LineInfo+3,TitleLen) != 0
	) return NULL;
	LineInfo += TitleLen+3;
	while(IsSpace(*LineInfo)) LineInfo++;
	if(*(LineInfo++) != ':') return NULL;
	while(IsSpace(*LineInfo)) LineInfo++;
	return LineInfo;
}
/*
BYTE *AddrToIP(char *IPAddr)
{
	static char IP[4];
	char *Startp,tmp;
	BYTE EndIP = 0,IPCount = 0;
	int16 TempIP;

	do {
		Startp = IPAddr;
		while(*IPAddr != '.' && *IPAddr != ' ' && *IPAddr != '\0' ) {
			if(*IPAddr < '0' || *IPAddr > '9') return NULL;	//8/14/98
		    IPAddr++;
		}
		if(*IPAddr != '.') EndIP = 1;
		tmp = *IPAddr;	//8/14/98
		*IPAddr = '\0';
		if(Startp == IPAddr || (TempIP = atoi(Startp)) < 0 || TempIP > 255) {
			*IPAddr = tmp; //8/14/98, don't change the input value
		    return NULL;
		}
		*IPAddr = tmp;	//8/14/98, don't change the input value
		IP[IPCount++] = TempIP;
		if(IPCount != 4) IPAddr++;
	} while(!EndIP && IPCount < 4);
	if(!EndIP || IPCount != 4 ) return NULL;
	while(*IPAddr == ' ') IPAddr++;
	if(*IPAddr != '\0') return NULL;
	return (IP);
}
*/
char IsDHCPOn(char *String)
{
	char *TmpStr = String;

	while(!IsSpace(*TmpStr) && *TmpStr != '\0') TmpStr++;
	*TmpStr = '\0';
	strupr(String);
	if(strcmp(String,DHCPMsg[0]) == 0) return 1;//ON
	if(strcmp(String,DHCPMsg[1]) == 0) return 0;//OFF
	return (-1);
}

char IsPsEnable(char *String)
{
	char *TmpStr = String;
	int StrLen;

	while(!IsSpace(*TmpStr) && *TmpStr != '\0') TmpStr++;
	*TmpStr = '\0';

	StrLen = strlen(EnDisable[0]);
	if(strnicmp(String,EnDisable[0],StrLen) == 0 && String[StrLen] == '\0')
		return 0;//disable

	StrLen = strlen(EnDisable[1]);
	if(strnicmp(String,EnDisable[1],StrLen) == 0 && String[StrLen] == '\0')
		return 1;//enable

	return (-1);
}

#ifdef DEF_PRINTSPEED
char *GetPrinterMode(char Mode)
{
	static char OtherMode[6];

	if(Mode < 3) return PrinterMode[Mode];
	else {
		sprintf(OtherMode,"V:%d",(WORD)Mode);
	}
	return OtherMode;
}

char SetPrinterMode(char *String)
{
	char *TmpStr = String;
	int StrLen, i ;

	while(!IsSpace(*TmpStr) && *TmpStr != '\0') TmpStr++;
	*TmpStr = '\0';

	for(i = 0 ; i < 3; i++) { //fast, normal, slow
		StrLen = strlen(PrinterMode[i]);
		if(strnicmp(String,PrinterMode[i],StrLen) == 0 && String[StrLen] == '\0')
		return i;
	}
	//Backdoor V:Value V:10 V:20
	if((String[0] == 'V' || String[0] == 'v') && String[1] == ':')
	    return((BYTE)atoi(String+2));

	return (-1); //error
}
#endif DEF_PRINTSPEED

/*
#ifdef CODE1
char *PrnGetPrinterName(int PortNumber)
{
	static BYTE PortName[]= G_PRN_PORT_NAME;

	PortName[G_PRN_PORT_POS] = PortNumber+'1';

	return PortName;
}
#endif
*/

void TftpDLFlash(void)
{
	struct tftphdr *dp;
	struct tftphdr *ap;    /* ack buffer */
	uint16 block;
	int    nRecvLen, retry;
//	uint8  *CurTempAddress = NEW_CODE2_TEMP_ADDRESS;
	uint8  *CurTempAddress = UPGRADE_TEMP_ADDRESS;
	uint8  *TempAddress ;
	uint32	bsize=0;
	TempAddress = CurTempAddress;
	
	if( !vAllocCode2Memory() )
	{
		nak(EUPGRADE,0);
		return;
	}

	if( PSUpgradeMode != WAIT_UPGRADE_MODE )
	{
		vReleaseCode2Memory();
		nak(EUPGRADE,0);
		return;
	}

	PSUpgradeMode = TFTP_UPGRADE_MODE;

	dp =  (struct tftphdr *)buf;
	ap = (struct tftphdr *)ackbuf;

	block = 0;
	
	do {
		retry = 0;
		do {
			ap->th_opcode = htons((uint16)ACK);
			ap->th_block = htons((uint16)block);
			if(sendto(lsocket,
			          ackbuf,
	    		      4,
		        	  0,
			          (struct sockaddr *)&fromsock,
			          sizeof(fromsock)) < 0)
			{			  
				vReleaseCode2Memory();
				PSUpgradeMode = WAIT_UPGRADE_MODE;
				return;
			}		  
			//kwait(NULL);   //1/29/99 remarked
            cyg_thread_yield();
			fromlen = sizeof(fromsock);
			nRecvLen = recvfrom(lsocket,
    			          dp,
        		    	  PKTSIZE,
        			      0,
		            	  (struct sockaddr *)&fromsock,
			              &fromlen );
			
		} while( (nRecvLen < 0 || htons(dp->th_block) != block+1)
		          && ++retry < TFTPD_TRY_TIMES );

		if( nRecvLen < 0 ) break;
		if( nRecvLen > 4 )
		{
			if( block > ( CODE2_MAX_SIZE << 1 ) )
			{ 
				block++;
				break;
			}

			memcpy( CurTempAddress, dp->th_data, nRecvLen-4 );

			StatusLightToggle = 5;

/*			if( block == 0 )
			{
				if( CurTempAddress[BIN_ID_OFFSET] != 'X' ||
					CurTempAddress[BIN_ID_OFFSET+1] != 'G' ||
					CurTempAddress[BIN_ID_OFFSET+2] != 'Z' ||
					CurTempAddress[BIN_MODEL_OFFSET] != CURRENT_PS_MODEL )
				{
					nak( EBADCODE2, 0 );
					break;
				}
			}
*/
			CurTempAddress += SEGSIZE;
			bsize += SEGSIZE;
		}
		block++;
	} while	(nRecvLen == PKTSIZE);

	if( block > ((CODE2_MAX_SIZE<<1)+1) )
	{
		vReleaseCode2Memory();
		nak( EBADCODE2, 0 );
		PSUpgradeMode = WAIT_UPGRADE_MODE;
		return;
	}

/*	if( vProgramCode2( NEW_CODE2_TEMP_ADDRESS ) )
	{
		vReleaseCode2Memory();
		nak( EWRFLASH, 0 );
		PSUpgradeMode = WAIT_UPGRADE_MODE;
		return;
	}
*/
	if(ApUpgradeFirmware( TempAddress , bsize ) == 0)
	{
		vReleaseCode2Memory();
		nak( EWRFLASH, 0 );
		PSUpgradeMode = WAIT_UPGRADE_MODE;
		return;
	}
	
	ap->th_opcode = htons((uint16)ACK);    // send the "final" ack
	ap->th_block = htons((uint16)(block));
	sendto(lsocket,
	       ackbuf,
	       4,
	       0,
	       (struct sockaddr *)&fromsock,
	       sizeof(fromsock));

	ppause(10);

//	REBOOT();
	Reset();

	PSUpgradeMode = WAIT_UPGRADE_MODE;
}

void TftpDLCode1(void)
{
	struct tftphdr *dp;
	struct tftphdr *ap;    /* ack buffer */
	uint16 block;
	int    nRecvLen, retry;
//	uint8  *CurTempAddress = NEW_CODE2_TEMP_ADDRESS; // use temp block to store BIN file
	uint8  *CurTempAddress = UPGRADE_TEMP_ADDRESS; // use temp block to store BIN file
	uint8  *TempAddress ;
	uint32	bsize=0;
	TempAddress = CurTempAddress;


	if( !vAllocCode2Memory() )
	{
		nak(EUPGRADE,0);
		return;
	}

	dp =  (struct tftphdr *)buf;
	ap = (struct tftphdr *)ackbuf;

	block = 0;

	do {
		retry = 0;
		do {
			ap->th_opcode = htons((uint16)ACK);
			ap->th_block = htons((uint16)block);
			if(sendto(lsocket,
			          ackbuf,
	    		      4,
		        	  0,
			          (struct sockaddr *)&fromsock,
			          sizeof(fromsock)) < 0)
			{			  
				vReleaseCode2Memory();
				return;
			}		  
			//kwait(NULL);   //1/29/99 remarked
//			cyg_thread_yield();			
			fromlen = sizeof(fromsock);
			nRecvLen = recvfrom(lsocket,
    			          dp,
        		    	  PKTSIZE,
        			      0,
		            	  (struct sockaddr *)&fromsock,
			              &fromlen );
		} while( (nRecvLen < 0 || htons(dp->th_block) != block+1)
		          && ++retry < TFTPD_TRY_TIMES );

		if( nRecvLen < 0 ) break;
		if( nRecvLen > 4 )
		{
			if( block > ( CODE1_SIZE << 1 ) )
			{ 
				block++;
				break;
			}

			memcpy( CurTempAddress, dp->th_data, nRecvLen-4 );

			StatusLightToggle = 5;

			CurTempAddress += SEGSIZE;
			bsize += SEGSIZE;
		}
		block++;
	} while	(nRecvLen == PKTSIZE);

	if( block > ((CODE1_SIZE<<1)+1) )
	{
		vReleaseCode2Memory();
		nak( EBADCODE2, 0 );
		return;
	}

//	SupervisorMode();
//	CacheDisable();
//	DisableDCache();
//	DisableICache();
//	cli();

//	if( vProgramCode1( NEW_CODE2_TEMP_ADDRESS ) )
/*	if( vPCode1( NEW_CODE2_TEMP_ADDRESS ) )
	{
		vReleaseCode2Memory();
		nak( EWRFLASH, 0 );
		return;
	}
*/
	if(ApUpgradeFirmware( TempAddress , bsize ) == 0)
	{
		vReleaseCode2Memory();
		nak( EWRFLASH, 0 );
		return;
	}
	
//	sti();

	ap->th_opcode = htons((uint16)ACK);    // send the "final" ack
	ap->th_block = htons((uint16)(block));
	sendto(lsocket,
	       ackbuf,
	       4,
	       0,
	       (struct sockaddr *)&fromsock,
	       sizeof(fromsock));

	ppause(10);

//	REBOOT();
	Reset();
}

#endif UNIXUTIL_TFTP
