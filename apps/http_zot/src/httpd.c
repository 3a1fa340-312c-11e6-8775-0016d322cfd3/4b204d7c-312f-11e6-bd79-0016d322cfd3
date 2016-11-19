/* HTTP server
 * Almost completely rewritten. I borrowed extensively from the ideas
 * and code of Brian Lantz' (brian@lantz.com) adaptation of the http
 * server for Tnos which was based loosely on my previous version of
 * the HTTP server which was derived from Ashok Aiyar's server
 * <ashok@biochemistry.cwru.edu>, which was based on
 * Chris McNeil's <cmcneil@mta.ca> Gopher server, which was based in
 * part on fingerd.c
 *
 *                          5/6/1996 Selcuk Ozturk
 *                                   seost2+@pitt.edu
 */
#include <cyg/kernel/kapi.h>
#include <network.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "ippd.h"
#include "httpd.h"
#include "ledcode.h"
#include "nps.h"
#include "pfs.h"
#include "gmtime.h"
#include "prnqueue.h"
#include "prnport.h"
#include "led.h"
#include "joblog.h"

/*Ron Add 11/5/2004 */
extern int	sendack(int s);


#ifndef USE_PS_LIBS
#undef IPPD
#undef LPD_TXT
#undef NOVELL_PS
#undef NDS_PS
#undef ATALKD
#undef DEF_IEEE1284
#undef Mail_ALERT
#undef SUPPORT_JOB_LOG
#endif

#ifdef ATALKD
/*Jesse
#ifdef ATALKD
#include "atp.h"
#include "pap.h"
#include "file.h"
#include "paprint.h"
#endif ATALKD
*/
#include "atalkd.h"
#include "atp.h"
#include "pap.h"
#include "paprint.h"

#define AT_COMM_NONE	  (0)
#define	AT_COMM_TBCP	  (1)
#define AT_COMM_BCP		  (2)
extern struct AT_IFACE at_iface;
static uint8 ATPortName[ATALK_NAME_LEN+1];
static uint8 ATPortNameLength;
extern uint8 *GetATPortName(int i);

#endif /* ATALKD */

#ifdef NDS_PS
#include "ndsqueue.h"
#endif

#ifdef WIRELESS_CARD
#include "wlanif.h" // wireless get information interface ... Ron 6/20/2003
extern int diag_flag;
#endif

#ifndef USE_ADMIN_LIBS
char ANYESS[] = "< ANY >";
#endif
extern uint8    rxStatsRSSI;             /* RF Signal Strength Indicator */
extern uint8    linkQuality;              /* Link Quality */

extern uint8   mvWEPKey[32];   // cannot be NULL
extern uint8   mvWDomain;		//0x0010 USA    1-11
                         		//0x0020 CANADA 1-11
                            	//0x0030 ETSI   1-13
                            	//0x0031 SPAIN  10-11
                            	//0x0032 France 10-13
                            	//0x0040 Japan  14
                            	//0x0041 Japan  1-14

///////////////////////////////////////////////////////////////////////////////

extern int sscanf( const char * /* str */, const char * /* format */, ... );

// Created on 4/26/2000.
// Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
BYTE WebLangVersion[3]={0};

#define CONST_DATA

#ifdef HTTPD
//******************************IP ADDRESS*********
typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
}MIB_DHCP;

extern MIB_DHCP                   *mib_DHCP_p;

#define SOCKSUM		HTTP_MAX_USER
typedef struct u_sock {
	int     num;
	BYTE	flag; 
	int     needrelase;
};
struct u_sock Usedsock[SOCKSUM]={0};

#ifdef IPPD
//IPPD_Start Thread initiation information
#define IPPD_Start_TASK_PRI         	20	//ZOT716u2
#define IPPD_Start_TASK_STACK_SIZE  	8192
static	uint8			IPPD_Start_Stack[IPPD_Start_TASK_STACK_SIZE];
static  cyg_thread		IPPD_Start_Task;
static  cyg_handle_t	IPPD_Start_TaskHdl;
#endif

//Http_Start Thread initiation information
#define Http_Start_TASK_PRI         	20	//ZOT716u2
#define Http_Start_TASK_STACK_SIZE  	8192
static	uint8			Http_Start_Stack[Http_Start_TASK_STACK_SIZE];
static  cyg_thread		Http_Start_Task;
static  cyg_handle_t	Http_Start_TaskHdl;

//Httpd Thread initiation information
#define Httpd_TASK_PRI				20	//ZOT716u2
#define Httpd_TASK_STACK_SIZE    	8192//ZOT716u2 4096
static uint8			Httpd_Stack[SOCKSUM+1][Httpd_TASK_STACK_SIZE];
static cyg_thread		Httpd_Task[SOCKSUM+1];
static cyg_handle_t	Httpd_TaskHdl[SOCKSUM] = {0};


//ksign
cyg_sem_t	 Httpd_sem;

#define		MCBSIZE 						64*1024
#define 	ambufw(x) 						malloc(x)
#define		free_p(x)						free(x)
#define		send_mbuf(a,b,c,d,e)			send(a,b,c,e)

///////////////////////////////////////////////////////////////////////////////
char* __time__ = "(" __DATE__ " " __TIME__ ")";

//endmark.c
extern unsigned long MyDataSize[4];
extern unsigned char MyData[];
//extern unsigned char *MyData;

//from psmain.c
extern BYTE 			PSMode;
extern uint32 rdclock();
extern uint32 msclock();
extern void NSET16( uint8 *pDest, uint16 value );
extern uint32 NGET32( uint8 *pSrc );
extern void NSET32( uint8 *pDest, uint32 value );
extern void NCOPY32( uint8 *pDest, uint8 *pSrc );
extern int urandom(unsigned int n);
extern int strnicmp( char * dst,  char * src ,int n );
extern int stricmp( char * dst,  char * src );
extern void *mallocw(size_t nb);
extern char * strupr ( char * string );

//from prnport.c
extern struct parport PortIO[NUM_OF_PRN_PORT];
extern int PNW_Statu[NUM_OF_PRN_PORT];
extern uint32 	Printing_StartNum[NUM_OF_PRN_PORT];
extern uint32	PageCount[NUM_OF_PRN_PORT];
extern uint8   adjPortType[NUM_OF_PRN_PORT];

//from prnqueue.c
extern char * strdup(const char *str);

//from ntps.c
extern uint8 IsNTEndPrint[NUM_OF_PRN_PORT];	  //for NTUDP.C

//from http_constant.c
extern const char *PrnUsedMessage[];

//from gmtime.c
extern char *gmt_ptime(long t);

//from ntutil.c
//char ANYESS[] = "< ANY >";
extern char ANYESS[];  //Ron 4/1/2003
extern void UtilGetVersionString(BYTE *Version);	  //4/26/2000

//from FILE.H of ARM9PS
#define PF_BUFSIZ( pf )		((pf)->pf_end - (pf)->pf_cur)

//from  eeprom.c 
extern EEPROM			EEPROM_Data;
extern uint8   			NTMaxRecvPacket;
extern uint16			OffsetOfFSName[MAX_FS]; //Offset of FS name from EEPROM.FileServerNames
extern char 			Hostname[LENGTH_OF_BOX_NAME+1];
extern uint8 			CurSetupPassword[SETUP_PASSWD_LEN+1]; //12/30/99

#ifdef CFG_EXPIMP
extern EEPROM			EEPROM_Data_temp;	// George
#endif	// CFG_EXPIMP

//from nps3.c
extern BYTE SearchObjectInit(void);
extern BYTE SearchBinaryObjectName(BYTE *ObjectName,DWORD *LastObjectSeed,WORD ObjectType);

//from lpd.c
#ifdef LPD_TXT
extern int CurJobQueueID[NUM_OF_PRN_PORT]; //6/4/98 added, 12/22/98 set as a global
#define GetUnixJobID(x) CurJobQueueID[(x)]  //12/22/98
#endif

//from rwflash.c
extern int ResetToDefalutFlash(int tmpip, int tmpkey, int nouse);
extern int WriteToEEPROM(EEPROM *RomData);

//from wlanif.c
//extern void wlan_get_currssid(char *ssid);
//extern void wlan_get_currbssid(char *bssid);
extern int wlan_get_currrate(void);
extern int wlan_get_currrate_G(void);
extern int wlan_get_channel(void);


//LED.C of ARM9PS
extern WORD LEDErrorCode;

extern int Need_Rendezous_Reload;

/*Jesse



// alloc.c of ARM9PS
extern unsigned long Availmem;      // Heap memory, ABLKSIZE units 
extern unsigned long mini_Availmem;	//3/29/99

//FORM STUIO.C of ARM9PS
//extern FILE* fdopen(int handle, const unsigned short* mode);

//FORM RWFLASH.C of ARM9PS
extern int vAllocCode2Memory();
extern int vProgramCode2( char *pCode2Data );
extern int vReleaseCode2Memory();

//FORM PCGEN.C of ARM9PS
extern int CheckBIN( uint8 *CodeAddress );

//FORM REBOOT.C of ARM9PS
extern void REBOOT();
extern void BOOTSTRAP();

*/
#ifdef IPPD
//IPPD.C 
extern void ippd(BYTE port, ipp_t *ippObj);
extern BYTE ippCheckPort(BYTE *url, ipp_t *ippObj);
extern int16 ippSetRespBuf(BYTE *buf, ipp_t *ippObj);
extern ipp_t *ippObjList[NUM_OF_PRN_PORT];
extern BYTE  ippJobsNo[NUM_OF_PRN_PORT]; //How many Obj connected for print !

#endif

/* Read a line from a stream into a buffer*/
int zot_recv ( ZOT_FILE *fp , char *buf , int len, int flag )
{
	if( fp->ibuf_cnt ==0 )
	{
		fp->ibuf = fp->ibuf_temp;
//		memset(fp->ibuf ,0 , BLOCKSIZE);
		memset(fp->ibuf ,0 , 1460);
		
//		fp->bufsize = recv( fp->fd , fp->ibuf , BLOCKSIZE , 0  );
		fp->bufsize = recv( fp->fd , fp->ibuf , 1460 , 0  );
		
		fp->ibuf_cnt = fp->bufsize;
		if( fp->bufsize <= 0 )
			return -1;
	}

	len = min( len , fp->ibuf_cnt );	
	memcpy(buf,fp->ibuf,len);
	fp->ibuf +=len;
	fp->ibuf_cnt = fp->ibuf_cnt - len;

	return len;
}

int zot_fflush(ZOT_FILE *fp)
{
	int prev;
	if( fp->offset == 0 )
		return 0;
	prev = send( fp->fd , fp->obuf , fp->offset , 0 );
	fp->offset=0;
	memset(fp->obuf ,0 ,1600);
	return prev;
}

/* send_packet */
int
fmode(ZOT_FILE *fp,int mode)
{
	int prev;
/*Jesse
    if(fp == NULL || fp->cookie != _COOKIE)
            return -1;
    fflush(fp);
    prev = fp->flags.ascii;
    fp->flags.ascii = mode;
*/		
	mode = 0;
	prev = send( fp->fd , fp->obuf , fp->offset , mode );
	fp->offset=0;
	memset(fp->obuf ,0 ,1600);

    return prev;
}

ZOT_FILE* zot_fopen( int handle )
{
	ZOT_FILE* fp;
	
	fp = (ZOT_FILE*)malloc(sizeof(ZOT_FILE));
	if( fp == NULL )
		return NULL;
	else
	{
		memset(fp ,0 ,sizeof(ZOT_FILE));
		fp->fd =  handle;
//		fp->ibuf = (char *)malloc(BLOCKSIZE);
		fp->ibuf = (char *)malloc(1460);
		fp->ibuf_temp = fp->ibuf;
		if(fp->ibuf != NULL)
		{
//			memset(fp->ibuf ,0 ,BLOCKSIZE);
			memset(fp->ibuf ,0 ,1460);
		}
		else
		{
			free(fp);
			return NULL;
		}
		
		fp->obuf = (char *)malloc(1600);
		if(fp->obuf != NULL)
			memset(fp->obuf ,0 ,1600);
		else
		{
			free(fp);
			free(fp->obuf);
			return NULL;
		}
	}
	return fp;
}

int zot_fclose( ZOT_FILE* fp)
{
	int	csd;
	
	csd = fp->fd;	
 	if( fp->offset > 0 )
 		send( fp->fd , fp->obuf , fp->offset , 0 );

 	free( fp->ibuf_temp );
 	free(fp->obuf);
 	free(fp);
 	
     // shutdown(csd,2);
 	close(csd);
 	
 	return 0;
}


int zot_fputs (int8 *buf ,ZOT_FILE *fp )
{
	int len,bytes;
	 
	len = strlen(buf);
	bytes = 1536 - (fp->offset);
	
	if( bytes < len )
	{
		memcpy( fp->obuf + fp->offset , buf , bytes );
		send( fp->fd , fp->obuf ,1536 , 0 );
		fp->offset=0;
		len -= bytes;
		buf += bytes;
		memset(fp->obuf ,0 ,1600);
	}
	memcpy( fp->obuf + fp->offset , buf , len );
	fp->offset += len;
	 
	 return len;
}

void zot_fputc(char data, ZOT_FILE *fp) 		
{
	char buf[10]={0};
	buf[0] = data; 
	zot_fputs(buf,fp);
}

/* Read a line from a stream into a buffer, retaining newline */
char *
zot_fgets(
char *buf,      // User buffer 
int len,        // Length of buffer 
//char *fp        // Input stream 
ZOT_FILE *fp
){
        char *cp;
				
        cp = buf;
        
        if( fp->ibuf_cnt ==0 )
        {
        	fp->ibuf = fp->ibuf_temp;
//			memset(fp->ibuf ,0 ,BLOCKSIZE);
			memset(fp->ibuf ,0 ,1460);
		
//			fp->bufsize = recv( fp->fd , fp->ibuf , BLOCKSIZE , 0  );
			fp->bufsize = recv( fp->fd , fp->ibuf , 1460 , 0  );
			fp->ibuf_cnt = fp->bufsize;
			if( fp->bufsize <= 0 )
				return NULL;
        }
        
        while(len-- > 1){      
                
                if( *(fp->ibuf) == '\r' )
                {
                	(fp->ibuf)++;
                	fp->ibuf_cnt = fp->ibuf_cnt - 1;
                	if( *(fp->ibuf) == '\n')
                		*cp ='\n';
                	(fp->ibuf)++;
                	fp->ibuf_cnt = fp->ibuf_cnt - 1;
                }	
                else
                {
                	*cp = *(fp->ibuf)++;
                	fp->ibuf_cnt = fp->ibuf_cnt - 1;
                }
                
               if( *cp++ == '\n')
                       break;
               if(fp->ibuf_cnt ==0)
              		break;     
        }
        if(buf != NULL)
                *cp = '\0';
        
        return buf;
}

char zot_fgetc(ZOT_FILE *fp)
{
	char cp = 0;
	
	if( fp->ibuf_cnt ==0 )
	{
		fp->ibuf = fp->ibuf_temp;
		memset(fp->ibuf ,0 ,1460);
	
		fp->bufsize = recv( fp->fd , fp->ibuf , 1460 , 0  );
		fp->ibuf_cnt = fp->bufsize;
		if( fp->bufsize <= 0 )
			return NULL;
	}
	cp = *(fp->ibuf);
	(fp->ibuf)++;
	(fp->ibuf_cnt)--;
	
    return cp;
}

#define		fflush(x)				zot_fflush(x)
#define		fputs(x,y) 				zot_fputs(x,y)
#define 	fgets(x,y,z)			zot_fgets(x,y,z)
#define 	fdopen(x,y)				zot_fopen(x)
#define		fclose(x)     			zot_fclose(x)
#define 	FILE 					ZOT_FILE

#if 0 //I implement at socket.c ... Ron 11/5/2004
//10/11/99 Simon
int	sendack(int s)
{
/*Jesse	
	struct usock *up;
	struct tcb *tcb;

	if((up = itop(s)) == NULL || (tcb = up->cb.tcb) == NULL){
		return -1;
	}

	tcb->flags.force = 1;
	tcp_output(tcb);
*/
	char *buf = (char *)mallocw(1514);
	memset(buf,0,1514);
	send(s,buf,6,0);
	free(buf);
	return 0;
}
#endif

uint32 mbufsize(void)
{
	return 0;
}

/* replace terminating end of line marker(s) with null */
void
rip(s)
register char *s;
{
	register char *cp;

	if((cp = strchr(s,'\n')) != NULL)
		*cp = '\0';
	if((cp = strchr(s,'\r')) != NULL)
		*cp = '\0';
}

/* Convert hex-ascii to integer */
int
htoi(s)
char *s;
{
	int i = 0;
	char c;

	while((c = *s++) != '\0'){
		if(c == 'x')
			continue;	/* allow 0x notation */
		if('0' <= c && c <= '9')
			i = (i * 16) + (c - '0');
		else if('a' <= c && c <= 'f')
			i = (i * 16) + (c - 'a' + 10);
		else if('A' <= c && c <= 'F')
			i = (i * 16) + (c - 'A' + 10);
		else
			break;
	}
	return i;
}


/* Return 0 if at least Memthresh memory is available. Return 1 if
 * less than Memthresh but more than Memthresh/2 is available; i.e.,
 * if a yellow garbage collection should be performed. Return 2 if
 * less than Memthresh/2 is available, i.e., a red collection should
 * be performed.
 */
int
availmem()
{
	void *p;
/*Jesse
	if(MEM_IS_ENOUGH) return 0;	// We're clearly OK

#if !defined(LOADER)
	mbuf_garbage();	//garbage collect
#endif
*/
	// Memory < (MTHRESH/3
	if((p = malloc(MTHRESH/3)) != NULL){
		free(p);
		return 0;	/* Okay */
	}

	return 2;		/* Red alert */
}

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

int16 FindIPAddress(BYTE Type)
{
	int rc=1;

	return rc;
}


//Old HTTPD.C member
WORD  AuthSeed;

#define WEB_VERSION    			2
#define SET_AUTH_SEED()  		{AuthSeed = urandom(0xE0E0) + 0x1015;}
#define UPGRADE_BLOCK_SIZE      1024

// function prototypes

void httpFileInfo(FILE *s, int8 *buf,PFS_FILE *fp, int16 vers, int8 type);
long sendfile(PFS_FILE *fp, int16  s);
void sendhtml(PFS_FILE *fp, FILE *s,int8 *InBuf,int16 inbuflen,int8 *OutBuf, struct reqInfo *reqInfo);

#ifdef CFG_EXPIMP
long sendfileCFG(PFS_FILE *fp, int16  s);	// George added this at build0026 of 716U2 on June 7, 2012.
#endif	// CFG_EXPIMP

void httpd(cyg_addrword_t data);
void httpHeaders(FILE *s, int8 *buf, int16 resp, struct reqInfo *rq);

void httpError(FILE *network,int8 *buf,int16 resp,int16 msg,const int8 *str,struct reqInfo *rq);
void process_cgi(FILE *network,int8 *Inbuf, int8 *Outbuf, struct reqInfo *reqInfo);
void EchoOutput(FILE *network,int8 *OutBuf, int8 *EchoItem);
void EchoSetFSName(FILE *network);
int8 IsFSSelected(BYTE *FSName);
void DisplayCGIMsg(char *HtmlName, FILE *network, int8 *inbuf,int8 *outbuf,struct reqInfo *rq);
void DisplayHTML(char *HtmlName, FILE *network, int8 *inbuf,int8 *outbuf,struct reqInfo *rq);

int8 *skipwhite(int8 *cp);
int8 *skipnonwhite(int8 *cp);

//CGI function !
int16 SearchEchoName(BYTE *cp0,BYTE *len);
int CGIBoxSetting(FILE *network,int8 *Inbuf, int8 *OutBuf, struct reqInfo *rq);

int authorization(FILE *netork, int8 *buf, struct reqInfo *reqInfo);
int base64encode(char *encoded, unsigned char *string, int len);
int8 *decode(int8 *string, int16 isPath);

int8 *UptimeString(uint32 timeticks,int8 *buf);
int8 *timeString(uint32 timeticks,int8 *buf);
int8 *portString(BYTE portnum, int8 *buf);

void HttpGetMessage(void);	//9/20/99 added


#ifdef NDS_PS
void EchoSetNDSTreeName(FILE *network, BYTE *TmpBuf);
#endif NDS_PS

BYTE IsSeedPassword(int8 *buf, int8 *pwd);

//BYTE WebLangVersion; //4/26/2000

int8 HttpNeedWriteEEPROM = 0, HttpNeedLoadDefault = 0;
int8 HttpNeedReboot = 0, HttpNeedReset = 0;
int8 WlanNeedServey = 0;
const int8 *Cgi_Error;
int8 errorstr[] = "[SSI error]" ;

int16 HttpUsers = 0;
int16 Httptdisc = 30;	//time out
int32 HttpStartTime;

BYTE  *AuthFileList;

#ifdef O_AXIS
	char* Protocol_Name[] ={"UDP","LPR","NetBEUI","APPTalk","RAWTCP","IPP"};
#else
	char* Protocol_Name[] ={"UDP","LPR","IPX/NetBEUI","APPTalk","RAWTCP","IPP","SMB","NETWARE"};
#endif


#ifdef CONST_DATA
//http_constant.c
extern const int8* wHdrs[];
extern const int8*  HttpResp[];
extern const int8*  HttpMsg[];
extern const int8*  HttpErrMsg[];
extern const struct FileTypes   HTTPtypes[];
extern const int8*  EchoName[];

extern const int8*  MediaMode[];  //9/27/99

// HTTP Header lines sent or processed
#define HDR_TYPE        0
#define HDR_LEN         1
#define HDR_MIME        2
#define HDR_SERVER      3
#define HDR_LOCATION    4
#define HDR_DATE        5
#define HDR_MODIFIED    6
#define HDR_SINCE       7
#define HDR_REF         8
#define HDR_AGENT       9
#define HDR_FROM        10
#define HDR_AUTHEN      11
#define HDR_AUTH        12
#define HDR_EXPIRES     13
#define HDR_PRAGMA      14
#define HDR_TARGET      15
#define HDR_ENCODING    16

#define RESP_200    0
#define RESP_302	1
#define RESP_304	2
#define RESP_400    3
#define RESP_401    4
#define RESP_404    5
#define RESP_500    6
#define RESP_501    7

#define MSG_302		0
#define MSG_400     1
#define MSG_401     2
#define MSG_404     3
#define MSG_500     4
#define MSG_501     5

#define HTTP_INVALID_REQUEST      HttpErrMsg[0]
#define HTTP_INVALID_URL          HttpErrMsg[1]
#define	HTTP_INVALID_POST_CONTENT HttpErrMsg[2]
#define HTTP_INVALID_METHOD       HttpErrMsg[3]

#define T_htm    	0
#define T_gif    	1
#define T_jpg		2
#define T_css		3
#define T_js		4
#define T_binary	5
#define T_plain  	6
#define	T_form	    7
#define T_ipp    	8

#define ECHO_NODE_ID            0
#define ECHO_VERSION            1
#define ECHO_NETWARE_CONNECT    2
#define ECHO_PORT1_STATUS       3
#define ECHO_PORT2_STATUS       4
#define ECHO_PORT3_STATUS       5
#define ECHO_DHCP_STATUS		6
#define ECHO_IP_STATUS          7
#define ECHO_SUBNET_STATUS      8
#define ECHO_GATEWAY_STATUS     9
#define ECHO_NETWARE_MODE       10
#define ECHO_FSNAME_STATUS      11
#define ECHO_HOME_ADDR          12
#define ECHO_ERROR_MSG          13

#define ECHO_BOX_NAME           14
#define ECHO_SET_DHCP           15
#define ECHO_SET_NOT_DHCP       16
#define ECHO_SET_IP             17
#define ECHO_SET_SUBNET         18
#define ECHO_SET_GATEWAY        19
#define ECHO_SET_NETWARE_MODE   20
#define ECHO_PRINTSERVER_NAME   21
#define ECHO_SET_FSNAME         22
#define ECHO_POLLING_TIME       23
#define ECHO_PORT_SPEED		    24	 //for N6300II only
#define ECHO_SET_PORT_SPEED		25	 //for N6300II only

#define ECHO_BOX_UPTIME         26
#define ECHO_SNMP_SYS_CONTACT   27
#define ECHO_SNMP_SYS_LOCATION  28
#define ECHO_SNMP_COMMUNITY1    29
#define ECHO_SNMP_COMMUNITY2    30
#define ECHO_SNMP_COMM1_ACCESS  31
#define ECHO_SNMP_COMM2_ACCESS  32
#define ECHO_SNMP_TRAP_ENABLE   33
#define ECHO_SNMP_AUTH_TRAP_ENABLE 34
#define ECHO_SNMP_TRAP_IP1      35
#define ECHO_SNMP_TRAP_IP2      36
#define ECHO_SNMP_SET_COMM1_ACCESS 37
#define ECHO_SNMP_SET_COMM2_ACCESS 38
#define ECHO_SNMP_SET_TRAP      39
#define ECHO_SNMP_SET_AUTH_TRAP 40

//APPLE-TALK  4/6/99
#define ECHO_ATALK_ZONE_NAME    41
#define ECHO_ATALK_PORT1_NAME   42
#define ECHO_ATALK_PORT2_NAME   43
#define ECHO_ATALK_PORT3_NAME   44
#define ECHO_ATALK_SET_ZONE_NAME 45
#define ECHO_ATALK_NET_ADDR     46
#define ECHO_ATALK_SET_PORT_NAME 47

// for debug only
#define ECHO_SET_TIMEOUT_VALUE  48	 //5/18/99
#define ECHO_SET_NT_MAX_PACKET  49	 //5/18/99
#define ECHO_SET_TEST_MODE      50	 //5/19/99

#define ECHO_VERSION_STATUS     51
#define ECHO_MEMORY_STATUS      52
#define ECHO_PRINT_STATUS		53
#define ECHO_PRINT_MODE         54
#define ECHO_QUEUE_STATUS       55
#define ECHO_BUFFER_STATUS      56
#define ECHO_ERROR_STATUS       57
#define ECHO_MEDIA_MODE         58

//NDS 12/10/99
#define ECHO_NDS_MODE           59
#define ECHO_SET_NDS_MODE       60
#define ECHO_NETWARE_PASSWORD   61
#define ECHO_NDS_TREE_NAME      62
#define ECHO_NDS_CONTEXT        63
#define ECHO_NDS_CONNECT        64
#define ECHO_SET_NDS_TREE_NAME  65
#define ECHO_SET_SETUP_PASSWD   66
#define ECHO_CONFIRM_PASSWD     67
#define ECHO_PASSWD_SEED        68

//IEEE1284 DEVICE ID
#define ECHO_PORT1_DEVICE_ID    69
#define ECHO_PORT2_DEVICE_ID    70
#define ECHO_PORT3_DEVICE_ID    71

#define ECHO_PORT1_MANUFACTURE  72
#define ECHO_PORT2_MANUFACTURE  73
#define ECHO_PORT3_MANUFACTURE  74

#define ECHO_PORT1_COMMAND_SET  75
#define ECHO_PORT2_COMMAND_SET  76
#define ECHO_PORT3_COMMAND_SET  77

#define ECHO_PORT1_PRINTER_MODE 78
#define ECHO_PORT2_PRINTER_MODE 79
#define ECHO_PORT3_PRINTER_MODE 80

#define ECHO_SET_PORT1_BIMODE   81
#define ECHO_SET_PORT2_BIMODE   82
#define ECHO_SET_PORT3_BIMODE   83

#define ECHO_SET_PORT1_PRN_MODE 84
#define ECHO_SET_PORT2_PRN_MODE 85
#define ECHO_SET_PORT3_PRN_MODE 86

//ATALK 2
#define ECHO_ATALK_PORT1_TYPE   87
#define ECHO_ATALK_PORT2_TYPE   88
#define ECHO_ATALK_PORT3_TYPE   89

#define ECHO_ATALK_PORT1_DATA_FORMAT        90
#define ECHO_ATALK_PORT2_DATA_FORMAT        91
#define ECHO_ATALK_PORT3_DATA_FORMAT        92
#define ECHO_SET_ATALK_PORT1_DATA_FORMAT	93
#define ECHO_SET_ATALK_PORT2_DATA_FORMAT	94
#define ECHO_SET_ATALK_PORT3_DATA_FORMAT	95

#define ECHO_FLASH_WRITE_COUNT  96

#define ECHO_IPP_JOBS	        97

#define ECHO_SYS_RESET			98
#define ECHO_SYS_REBOOT			99
#define ECHO_LOAD_DEFAULT		100
#define ECHO_SAVE_EEPROM		101
#define ECHO_WLESSID			102
#define ECHO_WLCHANNEL			103
#define ECHO_WLWEP_TYPE			104
#define ECHO_WLWEP_KEY_SELECT	105
#define ECHO_WLWEP_KEY			106
#define ECHO_WLWEP_KEY1			107
#define ECHO_WLWEP_KEY2			108
#define ECHO_WLWEP_KEY3			109
#define ECHO_WLWEP_KEY4			110
#define ECHO_WLWEP_128KEY		111
#define ECHO_WLBEACONINTERVAL	112
#define ECHO_WLRTSTHRESHOLD		113
#define ECHO_WLFRAGMENTATION	114
#define ECHO_WLRATES			115
#define ECHO_WLRATE				116
#define ECHO_WLSHORT_PREAMBLE	117
#define ECHO_WLAUTH_TYPE		118
#define ECHO_WLDTIMINTERVAL		119
#define ECHO_WLCFPPERIOD		120
#define ECHO_WLCFPMAXDURATION	121
#define ECHO_WLCRX				122
#define ECHO_WLCTX				123
#define ECHO_WLJAPAN			124
#define ECHO_WLANSIDE			125
#define ECHO_WLCOUNTRY			126
#define ECHO_WLGENERATE			127
#define ECHO_WLMODE				128
#define ECHO_WLAPMODE			129
#define ECHO_WLTXPOWER			130
#define ECHO_WLZONE				131
#define ECHO_WLCURCHANNEL		132
#define ECHO_WLTXRATE			133
#define ECHO_WLVERSION			134
#define ECHO_WEBJETADMIN		135
#define ECHO_WORKGROUP   		136
#define ECHO_SHAREPRINT1		137
#define ECHO_SHAREPRINT2		138
#define ECHO_SHAREPRINT3		139
#define ECHO_SPY				140
#define ECHO_SCANAP				141
#define ECHO_SHOWESSID			142
#define ECHO_DIAGNOSTIC			143
#define ECHO_CURRBSSID			144
#define ECHO_WLWEPFormat		145
#define ECHO_WLNonModulate		146
#define ECHO_CURRSSID			147
#define ECHO_PrintTest			148
#define ECHO_MAILALERT			149
#define ECHO_SMTPIP			    150
#define ECHO_SMTPMAIL			151
#define ECHO_MAC_RESET_COUNT  	152
#define ECHO_EAP_TYPE			153
#define ECHO_EAP_NAME			154
#define ECHO_EAP_PASSWORD		155
#define ECHO_MT_MODE			156
#define ECHO_MT_CHANNEL			157
#define ECHO_MT_RATE			158
#define ECHO_MT_PREAMBLE		159
#define ECHO_MT_LENGTH			160
#define ECHO_MT_SCRAMBLING		161
#define ECHO_MT_FILTER			162
#define ECHO_MT_ANTENNA_RX		163
#define ECHO_MT_ANTENNA_TX		164
#define ECHO_MT_POWER_LOOP		165
#define ECHO_MT_KEY_TYPE		166
#define ECHO_MT_KEY_LENGTH		167
#define ECHO_MT_KEY				168
#define ECHO_MT_CCAMODE			169
#define ECHO_MT_AUTORESPOND		170
#define ECHO_CURRRATE			171
#define ECHO_LPRQUEUE1			172
#define ECHO_LPRQUEUE2			173
#define ECHO_LPRQUEUE3			174
#define ECHO_WPA_PASS			175

#define ECHO_CURJOBLIST			176
#define ECHO_JOBLIST			177
#define ECHO_PORT1JOBCOUNT		178
#define ECHO_PORT2JOBCOUNT		179
#define ECHO_PORT3JOBCOUNT		180
#define ECHO_PORT1PAGECOUNT		181
#define ECHO_PORT2PAGECOUNT		182
#define ECHO_PORT3PAGECOUNT		183

#define ECHO_VERSION_SHORT		184

#define ECHO_RANDVOUS			185
#define ECHO_RANDVOUS_NAME		186
#define ECHO_ATALKSETTINGS		187

// MAC Filtering	// George Add August 11, 2005	
#define ECHO_MF_ENABLED				188
#define ECHO_MF_DENYALL				189
#define ECHO_MF_CONTROLLISTSIZE		190
#define ECHO_MF_NAME				191
#define ECHO_MF_MACADDRESS			192
#define ECHO_MF_CONTROLLIST			193
#define ECHO_MF_CONTROLLISTSTATUS	194

// 636U2PW 802.11bg	// Kevin: Apr.1 2007
#define ECHO_WLRXRSSI				195
#define ECHO_WLLINKQUALITY			196
#define ECHO_WLTXMODE				197
#define ECHO_WLWEPTYPE				198
#define ECHO_WLWPATYPE				199

#define ECHO_WLWEP_128KEY1			200
#define ECHO_WLWEP_128KEY2			201
#define ECHO_WLWEP_128KEY3			202
#define ECHO_WLWEP_128KEY4			203

#define ECHO_WLBANDWIDTH			204
#define ECHO_WLDATARATE				205

#define CGI_IP_ERROR            0
#define CGI_SUBNET_ERROR        1
#define CGI_GATEWAY_ERROR       2
#define CGI_POLLINGTIME_ERROR	3
#define CGI_PSNAME_ERROR	    4
#define CGI_FSNAME_ERROR        5
#define CGI_DHCP_ERROR          6
#define CGI_SNMP_TARP_IP_ERROR	7
#define CGI_PASSWORD_NOT_MATCH  8
#define CGI_UPGRADE_FAILED		9
#define CGI_CFGIMP_FAILED		10	// Failed in importing the CFG file.

#define ERROR_HTM_NAME   		0

#define	MAX_WEB_MESSAGE	  		31
//#define	MAX_WEB_MESSAGE	  		29
#define MAX_ERR_MESSAGE			11
int8 *WebMsg[MAX_WEB_MESSAGE];
int8 *CGI_Msg[MAX_ERR_MESSAGE];

#define WebMsg_PRINTER_WAITING       WebMsg[0]     //"Waiting for job"
#define WebMsg_PRINTER_PAPEROUT      WebMsg[1]     //"Paper Out"
#define WebMsg_PRINTER_OFFLINE       WebMsg[2]     //"Off Line"
#define WebMsg_PRINTER_PRINTING      WebMsg[3]     //"Printing"
#define WebMsg_DHCP_ON		         WebMsg[4]     //"ON"
#define WebMsg_DHCP_OFF              WebMsg[5]     //"OFF"
#define WebMsg_NETWARE_ENABLE        WebMsg[6]     //"Enabled"
#define WebMsg_NETWARE_DISABLE       WebMsg[7]     //"Disabled"
#define WebMsg_NETWARE_CONNECTED     WebMsg[8]     //"Connected"
#define WebMsg_NETWARE_DISCONNECTED  WebMsg[9]     //"Disconnected"
#define WebMsg_PORT_SPEED_INDEX      10
#define WebMsg_PORT_FAST		     WebMsg[WebMsg_PORT_SPEED_INDEX]      //"Fast"
#define WebMsg_PORT_NORMAL		     WebMsg[WebMsg_PORT_SPEED_INDEX+1]    //"Normal"
#define WebMsg_PORT_SLOW		     WebMsg[WebMsg_PORT_SPEED_INDEX+2]    //"Slow"
#define WebMsg_SNMP_ACCESS_READONLY	 WebMsg[13]	   //"Read-Only"
#define WebMsg_SNMP_ACCESS_READWRITE WebMsg[14]	   //"Read/Write"
#define WebMsg_SNMP_TRAP_ENABLE		 WebMsg[15]	   //"Enabled"
#define WebMsg_SNMP_TRAP_DISABLE	 WebMsg[16]	   //"Disabled"
#define WebMsg_SNMP_AUTH_ENABLE		 WebMsg[17]	   //"Enabled"
#define WebMsg_SNMP_AUTH_DISABLE	 WebMsg[18]	   //"Disabled"
#define WebMsg_NETWARE_FS_NOT_FOUND	 WebMsg[19]	   //"File Server not found !"
#define WebMsg_BIMODE_AUTO	         WebMsg[20]	   //"Auto Detect"
#define WebMsg_BIMODE_DISABLE	     WebMsg[21]	   //"Disable"
#define WebMsg_ATALK_TBCP	         WebMsg[22]	   //"TBCP"
#define WebMsg_ATALK_BCP     	     WebMsg[23]	   //"BCP"
#define WebMsg_ATALK_ASCII           WebMsg[24]	   //"ASCII"
#define WebMsg_IPP_PROCESSING        WebMsg[25]    //Processing
#define WebMsg_IPP_PENDING           WebMsg[26]    //Pending
#define WebMsg_LAST_MODIFIED 		 WebMsg[27]	   //"Wed, 25 Jul 2001 01:00:00 GMT"
#define WebMsg_SYSTEM_TIME			 WebMsg[28]    //"Wed, 25 Jul 2001 02:00:00 GMT"
#define WebMsg_Infrastructure		 WebMsg[29]    //Infrastructure
#define WebMsg_802_11AdHoc			 WebMsg[30]    //802.11AdHoc

#define CGI_IP_ERROR            0
#define CGI_SUBNET_ERROR        1
#define CGI_GATEWAY_ERROR       2
#define CGI_POLLINGTIME_ERROR	3
#define CGI_PSNAME_ERROR	    4
#define CGI_FSNAME_ERROR        5
#define CGI_DHCP_ERROR          6
#define CGI_SNMP_TARP_IP_ERROR	7

#endif CONST_DATA

#ifdef CONST_DATA
extern const int8 basis_64[];
#endif CONST_DATA

WORD CurMediaMode = 0;    //for Net Media Type 9/27/99

void httpd_init()
{
	//Create Http_start Thread
	cyg_thread_create(Http_Start_TASK_PRI,
						httpstart,
						IPPORT_HTTP,
						"HttpStart",
						(void *) (Http_Start_Stack),
						Http_Start_TASK_STACK_SIZE,
						&Http_Start_TaskHdl,
						&Http_Start_Task);
	//Start Httpstart Thread
	cyg_thread_resume(Http_Start_TaskHdl);

#ifdef IPPD
	if(PSMode & PS_IPP_MODE)
	{	
		//Create IPPD_Start Thread
		cyg_thread_create(IPPD_Start_TASK_PRI,
							httpstart,
							IPPORT_IPPD,
							"HttpStart",
							(void *) (IPPD_Start_Stack),
							IPPD_Start_TASK_STACK_SIZE,
							&IPPD_Start_TaskHdl,
							&IPPD_Start_Task);
	
		//Start IPPD_start Thread
		// cyg_thread_resume(IPPD_Start_TaskHdl);
	}
#endif
}

#ifdef ARCH_ARM
extern int Network_TCPIP_ON;
#endif /* ARCH_ARM */
// Start up http service
// Usage: "start http [port#] [drive_letter] [root_directory]
void httpstart (cyg_addrword_t data)
{
	int PortNo = data;
	int16 sd,csd;	 //socket
	struct sockaddr_in sa_server;
	struct sockaddr_in sa_client;
	int clen;
	int i=0,j=0;
	
#ifdef ARCH_ARM
    while(Network_TCPIP_ON == 0)
        ppause(100);
#endif /* ARCH_ARM */

	cli();
	HttpGetMessage();  //9/20/99 added
	sti();
	SET_AUTH_SEED();   //1/3/99

	HttpStartTime = HTTP_LAST_MODIFIED_TIME+urandom(1000)+(NGET32(EEPROM_Data.CheckSum2))&0xFFFF;
	memset((int8 *) &sa_server, 0, sizeof(sa_server));
	sa_server.sin_family=AF_INET;
	sa_server.sin_addr.s_addr=  htonl (INADDR_ANY);

#ifdef IPPD
	sa_server.sin_port= htons(PortNo);
#else
	sa_server.sin_port= htons(IPPORT_HTTP);
#endif IPPD

	sd = socket( AF_INET, SOCK_STREAM, 0 );
	
//ZOTIPS	armond_printf("open socket cnt = %d \n",sd);
	
	bind(sd,(struct sockaddr *) &sa_server,sizeof(sa_server));
//Jesse	listen(sd,1);
	listen(sd,8);
//	wndsize(sd,500); //8/4/2000
//Jesse	wndsize(sd,1460); //09/18/2000 fix bug
	                  //for win95 client print mentos.ppt P.7- P.9 ???
	while(1) {
		clen=sizeof(sa_client);
		memset(&sa_client, 0, clen);
		if( (csd = accept(sd,(struct sockaddr *) &sa_client, &clen)) == -1) {			
			
            diag_printf("accept failure\n");
			ppause(10);
			continue;
		}	
		// cyg_scheduler_lock();
		// 
		// for(i=0;i< SOCKSUM ;i++)
		// {
		//     if(Usedsock[i].needrelase	== 1 )
		//     {
        //         cyg_thread_delete(Httpd_TaskHdl[i]);
		//         Usedsock[i].needrelase = 0;
		//     }
		// }
		// cyg_scheduler_unlock();

//		if( HttpUsers >= HTTP_MAX_USER || availmem() != 0) {
		if( HttpUsers >= HTTP_MAX_USER ) {
			// shutdown(csd,2); //8/10/2000 changed
			close(csd);			//Jesse 
////		close_s(csd);

		} else {

/*Jesse
#ifdef IPPD
			newproc("http_child", 1280, httpd, csd, NULL, NULL, 0 );
#else
			newproc("http_child", 1280, httpd, csd, NULL, NULL, 0 );
#endif IPPD
*/			
			cyg_scheduler_lock();
			for(i=0;i< SOCKSUM ;i++)
			{
				if(Usedsock[i].flag == 0 )
				{
					HttpUsers++;
					Usedsock[i].num = csd;
					Usedsock[i].flag = 1;
					
//ZOTIPS					armond_printf("open socket cnt = %d \n",csd);
						
					//Clear Httpd_sem count
//					cyg_semaphore_init(&Httpd_sem,0);
							
					//Create Httpd Thread
					cyg_thread_create(Httpd_TASK_PRI,
										httpd,
										csd,
										"http_child",
										(void *) (Httpd_Stack[i]),
										Httpd_TASK_STACK_SIZE,
										&Httpd_TaskHdl[i],
										&Httpd_Task[i]);
	
					//Start Httpd Thread
					cyg_thread_resume(Httpd_TaskHdl[i]);
					break;
				}
			}
            CYG_ASSERT(i != SOCKSUM, "http:1216, No free socket!");
			cyg_scheduler_unlock();
		}
	}
	
}	

void
httpd (cyg_addrword_t data)
{
	int s = data; //Jesse
	int  length, bread, reject = 0;
	int bin_kind = 0;
	int32 upgrade_maxsize = 0;
	
	int32  tsize = 0;
	int32  bsize = 0;
	int32  qsize = 0;

#ifdef IPPD
	int8 *Inline, *Outline, *cp;

	BYTE port;
	ipp_t *ippObj;
#else
	int8 Inline[HLINELEN], Outline[HLINELEN], *cp;
#endif IPPD
	int8 *tquery, *queryp = NULL;
//Jesse	struct stat sb;
	struct reqInfo rq;
//Jesse	uint8  *CurTempAddress = NEW_CODE2_TEMP_ADDRESS;
	uint8  *CurTempAddress = 0; //Jesse
	PFS_FILE *fp = NULL;
	int8 thetype;
	struct timeval rcv_timeout;
	int i;
	struct linger ling;
	FILE *network =NULL;

	network = fdopen(s,"r+t"); //socket mode : ASCII
	if( network == NULL )
		goto quit;
		
	memset(&rq,0,sizeof(rq));

#ifdef IPPD
	ippObj = calloc(sizeof(ipp_t),1);
	Inline = malloc(HLINELEN);
	Outline = malloc(HLINELEN);
	if(Inline == NULL || Outline == NULL || ippObj == NULL) {
		free(ippObj);
		free(Inline);
		free(Outline);
		fclose(network);
		return;
	}
#endif IPPD

//	if(++HttpUsers > (3*(NUM_OF_PRN_PORT+1)) ) ;
//os	kwait(&HttpUsers); //10/16/2000 changed
//		cyg_semaphore_wait (&Httpd_sem);
	
	rcv_timeout.tv_usec = 65536 * 2;
	rcv_timeout.tv_sec = 3;

	setsockopt (network->fd, 
				SOL_SOCKET, 
				SO_RCVTIMEO,
				(char *)&rcv_timeout,
				sizeof(rcv_timeout));
	
	ling.l_onoff = 1;
	ling.l_linger = 0;
				
	setsockopt (network->fd, 
				SOL_SOCKET,
				SO_LINGER,
				(char *)&ling,
				sizeof(ling));  

//	if( (network->bufsize = recv( network->fd , network->ibuf , 1536 , 0  )) <= 0 ){
//		goto quit;
//	}
//	network->ibuf_cnt = network->bufsize;
//	fgets(Inline, HLINELEN, network);

//os kalarm (Httptdisc * 1000L);	
	if(fgets(Inline, HLINELEN, network) == NULL) {
		goto quit;
	}
//os	kalarm (0L);

	rip(Inline);
	if((cp = strstr(Inline," HTTP/1.")) != NULL) { // allow 1.x not just 1.0 - k2mf
		rq.version = 1;
		*cp = 0;
	}

	// Inline truncated to "METHOD /URI"
	cp = skipnonwhite(Inline); //skip to "/URL"
	if((queryp = strchr(cp,'?')) != NULL)
		*queryp++ = 0;

	// Let's check if we have a query with the GET method
	if(queryp && *queryp) {
		rq.query = decode(queryp,0);
		rq.qsize = strlen(rq.query);
	}

	// only path portion is left at cp
	if(strlen(cp) > 2) {
		*cp++ = 0;
		rq.url = decode(++cp,1); // drop the initial '/'
	} else {
		*cp = 0;
		rq.url = strdup("index.htm");
	}

	length = strlen (rq.url);

	if(!strcmp(Inline,"GET"))
		rq.method = METHOD_GET;
	else if(!strcmp(Inline,"HEAD"))
		rq.method = METHOD_HEAD;
	else if(!strcmp(Inline,"POST"))
		rq.method = METHOD_POST;
	else {
		rq.method = METHOD_UNKNOWN;
		httpError(network,Outline, RESP_501, MSG_501, Inline, &rq);
		goto quit0;                     // what method is this?
	}

	// if HTTP 0.9 (no version string), only 'GET' is allowed
	// also, don't allow query
	if (!rq.version && (rq.method != METHOD_GET || queryp)) {
		httpError(network,Outline, RESP_400, MSG_400,HTTP_INVALID_REQUEST, &rq);
		goto quit0;
	}

    // We don't allow urls with'?'in HEAD & POST methods.
    // Reason: Spec unclear and I'm lazy
	if((rq.method != METHOD_GET) && queryp) {
		httpError(network,Outline, RESP_400,MSG_400,HTTP_INVALID_URL,&rq);
		goto quit0;
	}

	// This is Version 1.x, so get all the header info
	if(rq.version) {
//os		while (kalarm (Httptdisc * 1000L),fgets(Inline,HLINELEN, network) != NULL) 
			while (fgets(Inline,HLINELEN, network) != NULL) 
			{
//os			kalarm (0L);
			rip (Inline);
			if(!*Inline) {
#ifdef IPPD
				if(ippObj->ippreq) {
					rq.version = 2; //ipp mode

					if(!(PSMode & PS_IPP_MODE)) {
						//system disable IPP service
						httpError(network,Outline,RESP_501,MSG_501,rq.url,&rq);
						goto quit0;
					}

					if(rq.method !=	METHOD_POST) {
						httpError(network,Outline, RESP_400, MSG_400,HTTP_INVALID_METHOD, &rq);
						goto quit0;
					}

					// port search
					if((port = ippCheckPort(rq.url, ippObj)) >= NUM_OF_PRN_PORT) {
						httpError(network,Outline,RESP_404,MSG_404,rq.url,&rq);
						goto quit0;
					}

					if(ippObj->chunked) {
						ippObj->qsize = 0;
					} else if(qsize == 0) {
						//no length
						httpError(network,Outline,RESP_400,MSG_400,HTTP_INVALID_POST_CONTENT,&rq);
						goto quit0;
					} else {
						ippObj->qsize = qsize;
					}
					ippObj->s = s;
					ippObj->network = network;
					free(Inline);
					free(Outline);
					Inline = Outline = NULL;
					ippd(port, ippObj);
					goto quit0;
				}
#endif IPPD
				if(qsize && rq.method == METHOD_POST)
				{
                    #ifdef IPPD
					if( rq.boundary )
                    #else
                    if(1)
                    #endif
					{
						if( !vAllocCode2Memory() ) {reject = 1;}
                        #ifdef IPPD
						qsize -= ( strlen( rq.boundary ) + 4 + 4);
                        #endif
						
//						CurTempAddress = mallocw(qsize + 1024);
						CurTempAddress = UPGRADE_TEMP_ADDRESS;
						
//os					while( kalarm(Httptdisc * 1000L), fgets(Inline,HLINELEN, network) != NULL)
						while( fgets(Inline,HLINELEN, network) != NULL)
						{
//os						kalarm(0L);
							tsize += strlen(Inline) + 1;
							rip(Inline);
							if(!*Inline){
								upgrade_maxsize = CODE2_MAX_SIZE * UPGRADE_BLOCK_SIZE;    //ZOTIPS
								qsize -= tsize;
				
								if ( qsize > upgrade_maxsize)
									reject =1;

								rq.binbuf = mallocw(UPGRADE_BLOCK_SIZE);
//os							while( kalarm(Httptdisc * 1000L), 
//Jesse									( bread = recv(network->fd,&rq.binbuf[bsize % UPGRADE_BLOCK_SIZE],
//Jesse									UPGRADE_BLOCK_SIZE - bsize % UPGRADE_BLOCK_SIZE, 0 ) ) )
								while(( bread = zot_recv( network , rq.binbuf,	
									((( qsize - bsize ) >1024) ? 1024 : (qsize - bsize)) , 0 ) ) )
								{
//os									kalarm(0L);
									if( bread < 0 )
										break;
										
									bsize += bread;
									
									if ( bsize > upgrade_maxsize || bsize >= qsize ){
										if( reject == 0 )
										{	
											memcpy( CurTempAddress, rq.binbuf, bread );
										}
											CurTempAddress += bread;
																						
											break;
									}
																		
									if( bsize <= qsize ){
										if( reject == 0 )
										{
											memcpy( CurTempAddress , rq.binbuf, bread );
										}
											CurTempAddress += bread;
									}	
						
								}	

#ifdef CFG_EXPIMP
								// George added this at build0011 of 716U2W on May 21, 2013.
								if( !stricmp(rq.url,"CRESTART.HTM") )
									Cgi_Error = CGI_Msg[CGI_CFGIMP_FAILED];
								else
#endif	// CFG_EXPIMP
									Cgi_Error = CGI_Msg[CGI_UPGRADE_FAILED];

								// Tracing these two variables is recommended.
								//	If you import the CFG file which size are 1408 bytes,
								//	bsize: 1408
								//	CurTempAddress: 
								//	CurTempAddress: 
								CurTempAddress -=  bsize;
								
								if( reject )
								{
									free( rq.url );
									rq.url = strdup( "ERROR.HTM" ); 
								}
								else
								{
#ifdef CFG_EXPIMP
									// George added this at build0011 of 716U2W on May 21, 2013.
									if( !stricmp(rq.url,"CRESTART.HTM") )
									{
										// Tracing these two variables is recommended.
										//	CurTempAddress:
										//	bsize: 1408
										memcpy(&EEPROM_Data_temp, CurTempAddress, sizeof(EEPROM));
										
										if(memcmp(EEPROM_Data_temp.EthernetID, EEPROM_Data.EthernetID, 6) != 0)
										{
											free( rq.url );
											rq.url = strdup( "ERROR.HTM" );
										}
										else
										{
											//if(  ApImportCFG( CurTempAddress , bsize ) == 0 )
											if( WriteToEEPROM(&EEPROM_Data_temp) != 0 )
											{
												free( rq.url );
												rq.url = strdup( "ERROR.HTM" ); 
											}
										}
									}
									else
#endif	// CFG_EXPIMP
									{
										if(  ApUpgradeFirmware( CurTempAddress , bsize ) == 0 )
										{
											free( rq.url );
											rq.url = strdup( "ERROR.HTM" ); 
										}
									}
								}
																
                                /*
								free(CurTempAddress);
                                */

								DisplayCGIMsg(rq.url, network,Inline,Outline,&rq);
								
//								if( !reject ) vReleaseCode2Memory();
								// vReleaseCode2Memory: both CODE 2 and CFG are okay
								vReleaseCode2Memory(); //release memory whatever reject equal 0 or 1 ... Ron 6/23/2003
								goto quit0;
							}
/*Jesse							else
							{
								cp = strstr( Inline, "name=" );
								if( cp )
								{
									cp += strlen( "name=" ) + 1;
									if( !strnicmp( cp, "CODE2BIN", strlen("CODE2BIN") ) )
										rq.type = 1;
									if( !strnicmp( cp, "CFGBIN", strlen("CFGBIN") ) )
										rq.type = 2;
								}
							}
*/							
						}
//						if( !reject ) vReleaseCode2Memory();
						vReleaseCode2Memory(); //release memory whatever reject equal 0 or 1 ... Ron 6/23/2003						
						goto quit0;
					}
					else
					{
						tquery = (int8 *)mallocw(qsize+1);
						*tquery = 0;
//os						kalarm(Httptdisc * 1000L);

						if(fgets(tquery,qsize+1,network)== NULL) {
							free(tquery);
							goto quit0;
						}

//os						kalarm(0L);
						if(*tquery)
							rq.query = decode(tquery,0);
						free(tquery);
						rq.qsize = strlen(rq.query); // correct the query size
					}	
				}
				break;
			}

			if(!strnicmp(Inline,wHdrs[HDR_LEN],strlen(wHdrs[HDR_LEN]))) {
				//"Content-Length:"
#ifdef IPPD
//Jesse				qsize = atol(Inline[strlen(wHdrs[HDR_LEN])]);
				qsize = atol(&Inline[strlen(wHdrs[HDR_LEN])]);
#else
//Jesse				qsize = atoi(Inline[strlen(&wHdrs[HDR_LEN])+1]);
				qsize = atoi(&Inline[strlen(wHdrs[HDR_LEN])+1]);
#endif IPPD
			} else if(!strnicmp(Inline,wHdrs[HDR_SINCE],strlen(wHdrs[HDR_SINCE]))) {
				//"If-Modified-Since:"
				rq.newcheck = strdup(&Inline[strlen(wHdrs[HDR_SINCE])+1]);
			} else if(!strnicmp(Inline,wHdrs[HDR_FROM],strlen(wHdrs[HDR_FROM]))) {
        		//"From:"
				rq.from = strdup(&Inline[strlen(wHdrs[HDR_FROM])+1]);
			} else if(!strnicmp(Inline,wHdrs[HDR_REF],strlen(wHdrs[HDR_REF]))) {
				//"Referer:"
				rq.referer = strdup(&Inline[strlen(wHdrs[HDR_REF])+1]);
			} else if(!strnicmp(Inline,wHdrs[HDR_AGENT],strlen(wHdrs[HDR_AGENT]))) {
				//"User-Agent:"
				rq.agent = strdup(&Inline[strlen(wHdrs[HDR_AGENT])+1]);
			} else if(!strnicmp(Inline,wHdrs[HDR_AUTH],strlen(wHdrs[HDR_AUTH]))) {
				//"Authorization:"
				rq.passwd = strdup(&Inline[strlen(wHdrs[HDR_AUTH])+7]);
			}
#ifdef IPPD
			else if(!strnicmp(Inline,wHdrs[HDR_TYPE],strlen(wHdrs[HDR_TYPE]))) {
				ippObj->ippreq = strstr(&Inline[strlen(wHdrs[HDR_TYPE])],HTTPtypes[T_ipp].type) == NULL?0:1;

				cp = strstr( &Inline[strlen(wHdrs[HDR_TYPE])],HTTPtypes[T_form].type);
				if( cp )
				{
					cp += strlen( HTTPtypes[T_form].type );
					cp = strstr( cp, "boundary=" );
					if( cp )
					{
						cp += strlen( "boundary=" );
						rq.boundary = strdup( cp );
					}
				}
			} else if(!strnicmp(Inline,wHdrs[HDR_ENCODING],strlen(wHdrs[HDR_ENCODING]))) {
				ippObj->chunked = strstr(&Inline[strlen(wHdrs[HDR_ENCODING])],"chunked") == NULL?0:1;
			}
#endif IPPD
			// We don't care about the rest
		}
	}

//Jesse	wndsize(s,0);	//use system defalut size

	rq.myname = Hostname;

//for DLink web if in diagnostic mode disable password
#ifdef WIRELESS_CARD
if (!((EEPROM_Data.SPECIAL_OEM == 0x02) && (diag_flag == 1))) {
#endif
	if(!authorization(network,Outline, &rq)){
		goto quit0;
	}
#ifdef WIRELESS_CARD
}
#endif

//os	kwait(NULL); // Let's be nice to others
	cyg_thread_yield();

	if(rq.query) {
		process_cgi(network,Inline, Outline, &rq);
		goto quit0;
	}

	// The only way we can get to this point with a POST command is through
    // a badly formed POST command
/*
	if(rq.method == METHOD_POST) {
		httpError(network,Outline,RESP_400,MSG_400,HTTP_INVALID_POST_CONTENT,&rq);
		goto quit0;
	}
*/

#ifdef IPPD
	if(ippCheckPort(rq.url, ippObj) < NUM_OF_PRN_PORT) {
		//8/16/2000 added
		free(rq.url);
		rq.url = strdup("ipp.htm");
	}
#endif IPPD
	
	fp = PFSopen(rq.url);
	if(fp != NULL)	{
		if((cp = strrchr(rq.url , '.')) != NULL) {
			cp++;
			for(thetype = 0; HTTPtypes[thetype].type != NULL; thetype++) {
				if(!strnicmp (HTTPtypes[thetype].ext, cp, strlen(HTTPtypes[thetype].ext)))
				break;
			}
		}

		// either had no extension, or extension not found in table
		if(thetype == -1 || HTTPtypes[thetype].type == NULL) {

			thetype = T_plain;
//			thetype = (isbinary(fp)) ? T_binary : T_plain;
		}

		// Charles 2001/07/17
		if( thetype==T_gif || thetype==T_jpg || thetype==T_css )
		{
			if(rq.method && rq.newcheck )
			{
				if( !strnicmp( rq.newcheck, WebMsg_LAST_MODIFIED, strlen(WebMsg_LAST_MODIFIED) ) )
				{
					httpHeaders(network,Outline,RESP_304,&rq);
					fputs("\n",network);
					goto quit0;
				}	
			}
		}

		httpHeaders(network,Outline,RESP_200,&rq);
		httpFileInfo(network,Outline, fp, rq.version, thetype);

		if(rq.method)	{
			if(thetype != T_htm) {// if not .htm, just send
				fflush(network);
				sendfile(fp, s);  //send a binary file
			}
			else	// otherwise, scan for server-side includes
				sendhtml(fp,network,Inline,HLINELEN,Outline,&rq);
		}
	}
#ifdef CFG_EXPIMP
	else if( !stricmp(rq.url,"CFG.BIN") )
	{
		// George added this at build0005 of DWP2020 on May 31, 2012.
		//	Export/Import CFG
		thetype = T_binary;
		
		httpHeaders(network, Outline, RESP_200, &rq);
		httpFileInfo(network, Outline, fp, rq.version, thetype);
		
		fflush(network);
		sendfileCFG(fp, s);  //send a binary file
	}
#endif	// CFG_EXPIMP
	else
	{

		httpError(network,Outline,RESP_404,MSG_404,rq.url,&rq);

	} //if(fp != NULL)	{

quit:
quit0:

	if(fp) PFSclose(fp);
//      free(rq.myname);
    if (rq.url)
	    free(rq.url);
    if (rq.query)
	    free(rq.query);
    if (rq.newcheck)
	    free(rq.newcheck);
    if (rq.from)
	    free(rq.from);
    if (rq.referer)
	    free(rq.referer);
    if (rq.agent)
	    free(rq.agent);
    if (rq.passwd)
	    free(rq.passwd);   // never free rq.arg & rq.file
    if (rq.boundary)
	    free(rq.boundary);
    if (rq.binbuf)
	    free(rq.binbuf);

#ifdef IPPD
	if(Inline) free(Inline);
	if(Outline) free(Outline);
#endif IPPD

#ifdef IPPD
	if(ippObj->ippreq != IPP_PRINT_JOB) {
//		if(ippObj->ippreq == IPP_PRINT_JOB_ERROR) {
//			fclose_buf(network); //only free network buffer !
//			kwait(NULL); //must wait until TCPIN release the TCB block !!!
//			shutdown(s,2);
//		} else {
#endif IPPD
        if (network)
		    fclose(network);
#ifdef IPPD
//		}
		free(ippObj);
	}
#endif IPPD

	if ( WlanNeedServey )
	{
		ppause(50);
		WlanNeedServey = 0;
#if defined(WIRELESS_CARD)
//Jesse		wlan_site_survey();
		wlan_site_survey();	//615wu
//		wlan_scan_bsslist();
#endif		
	}

	if( HttpNeedReboot || HttpNeedReset )
	{
	/* Delay for NDS disconnect, let REBOOT() timer function delay  ... Ron 9/15/04	
		ppause(5000); //12/29/99 for NDS
	*/	

		if( HttpNeedLoadDefault )
		{
			HttpNeedLoadDefault = 0;
			if( ResetToDefalutFlash(1,1,0) != 0 )
				ErrLightOnForever(Status_Lite); 
		}
		else if( HttpNeedWriteEEPROM )
		{
			HttpNeedWriteEEPROM = 0;
			if( WriteToEEPROM(&EEPROM_Data) != 0 )
				ErrLightOnForever(Status_Lite); //6/22/99
		}
		if( HttpNeedReboot )
		{
			HttpNeedReboot = 0;
			HttpNeedReset = 0;
//			Reset(); //os
			REBOOT();
		}
		else
		{
#ifdef RENDEZVOUS
			Need_Rendezous_Reload = 1;
//			WriteToEEPROM(&EEPROM_Data);
#endif RENDEZVOUS
			HttpNeedReboot = 0;
			HttpNeedReset = 0;
//			Reset(); //os
			REBOOT();
//Jesse			BOOTSTRAP();
		}
	}

//os	ksignal(&HttpUsers,1);
//		cyg_semaphore_post (&Httpd_sem);

	cyg_scheduler_lock();
	for(i=0;i<SOCKSUM;i++)
	{
		if(Usedsock[i].num	== s )
		{
			Usedsock[i].num = 0;
			Usedsock[i].flag = 0;
			Usedsock[i].needrelase = 1;
			break;
		}
	}
	HttpUsers--;
	cyg_scheduler_unlock();
	cyg_thread_exit();
}	

void
httpFileInfo(FILE *network,int8 *buf, PFS_FILE *fp, int16 vers, int8 type)
{
//Jesse	struct stat sb;

	if(!vers) return;		// it is 0.9, no headers sent

	if( type == T_gif || type == T_jpg || type == T_css )
	{
		sprintf(buf, "%s %s\n",
		             wHdrs[HDR_MODIFIED], WebMsg_LAST_MODIFIED );
	  	fputs(buf, network);
	}
	else
	{	
		//LAST_MODIFIED: Sat, 06 Feb 2010 01:50:00 GMT\n
		sprintf(buf, "%s %s\n",
		             wHdrs[HDR_MODIFIED],
		             gmt_ptime(HttpStartTime+(rdclock() /TICKS_PER_SEC))
			);
	  	fputs(buf, network);

		//Expires: Sat, 07 Feb 1998 01:50:00 GMT\nPragma: no-cache\n
		sprintf(buf, "%s %s\n%s\n",wHdrs[HDR_EXPIRES], gmt_ptime(HTTP_EXPIRE_TIME),wHdrs[HDR_PRAGMA]);
		fputs(buf, network);
	}

	//Content-Type: text/html | image/gif
	sprintf(buf, "%s %s\n", wHdrs[HDR_TYPE], HTTPtypes[type&0x7F].type);
	fputs(buf, network);

	//Windows-target: _top
	if(type & 0x80) {
		sprintf(buf, "%s\n", wHdrs[HDR_TARGET]);
		fputs(buf, network);
	}

//	sprintf(buf, "%s %d\n", wHdrs[HDR_LEN],fp->FileSize); //8/31/98	remarked
	if( type == T_gif || type == T_jpg || type == T_css || type == T_js )
	{
		sprintf(buf, "%s %lu\n", wHdrs[HDR_LEN],fp->FileSize); // 2001/07/19 changed
	}
	// Jesse marked this at build0019 in 716U2 on November 22, 2010.
	// George marked this at build0005 in 716U2W on December 13, 2010.
	// Fixed the error that it cannot be browsed by Google Chrome or RockMelt.
	//else
	//	sprintf(buf, "%s %lu\n", wHdrs[HDR_LEN],fp->FileSize+4096); //8/17/2000 changed

	fputs(buf, network);

	//\n
	fputs("\n",network);
}

void httpHeaders(FILE *s,int8 *buf,int16 resp,struct reqInfo *rq)
{
	BYTE *pVersion;
#ifdef IPPD
	BYTE HttpVersion = (rq == NULL || rq->version == 2)?1:0;
#endif IPPD

	if(rq != NULL) {
		rq->response = resp;
		if (!rq->version) return; // it is 0.9, no headers sent
	}

	//HTTP/1.x 200 OK \nDate: Sun, 09 Aug 1998 15:30:00 GMT\r\n
#ifdef IPPD
	sprintf(buf,"HTTP/1.%d %s\r\n%s %s\r\n",HttpVersion, HttpResp[resp], wHdrs[HDR_DATE], WebMsg_SYSTEM_TIME );
#else
	sprintf(buf,"HTTP/1.0 %s\n%s %s\n", HttpResp[resp], wHdrs[HDR_DATE], WebMsg_SYSTEM_TIME );
#endif IPPD
	fputs(buf,s);

	//pVersion = (BYTE *) MK_FP(ROM_BEGIN_SEGMENT,3);
	                                                //pVersion[0] = 12
	                                                //pVersion[1] = 3
													//pVersion[2] = 5
													//pVersion[3] = 20
	//MIME-version:	1.0\nServer: ZOT-PS-06/3.5.2012\n
	sprintf(buf, "%s 1.0\n%s ZOT-PS-%02d/%d.%d.%04d\n",
	            wHdrs[HDR_MIME],
	            wHdrs[HDR_SERVER],
	            (WORD)CURRENT_PS_MODEL,
	            CURRENT_MAJOR_VER,CURRENT_MINOR_VER,CURRENT_BUILD_VER);

	fputs(buf,s);
}

#ifdef IPPD
void ippSendResp(ipp_t *ippObj)
{
	uint16 MaxBufSize, UnSupportSize, RespGroupSize;
//Jesse	struct mbuf *bp;
	char *bp;
	int  cnt;
	BYTE   EndTag;

	//////////// Get Max buffer size //////////////////////////
	UnSupportSize = PF_BUFSIZ(&(ippObj->UnSupportGroup));
	if(ippObj->RetCode < IPP_CLIENT_BAD_REQUEST ||
	   ippObj->RetCode == IPP_SERVER_ERROR_JOB_CANCELLED)
	{
		RespGroupSize = PF_BUFSIZ(&(ippObj->RespGroup));
		if(RespGroupSize == 1) RespGroupSize = 0; //only contain "TAG" value
	} else {
		RespGroupSize = 0;
	}
	MaxBufSize = 512;
	if(UnSupportSize > MaxBufSize) MaxBufSize = UnSupportSize;
	if(RespGroupSize > MaxBufSize) MaxBufSize = RespGroupSize;
	///////////////////////////////////////////////////////////

//Jesse	if((bp = ambufw(MaxBufSize)) == NULL) return;
	if((bp =(char *) ambufw(MaxBufSize)) == NULL) return;
	//response header
//Jesse	httpHeaders(ippObj->network, bp->data, RESP_200, NULL);
		httpHeaders(ippObj->network, bp, RESP_200, NULL);

	//Pragma: no-cache\nCache-Control: no-cache\nConnection: close\n
/*Jesse	sprintf(bp->data, "%s\nCache-Control: no-cache\nConnection: close\n",wHdrs[HDR_PRAGMA]);
	fputs(bp->data, ippObj->network);
*/	sprintf(bp, "%s\r\nCache-Control: no-cache\r\nConnection: close\r\n",wHdrs[HDR_PRAGMA]);
	fputs(bp, ippObj->network);
	
	//Content-Type: application/ipp
/*Jesse	sprintf(bp->data, "%s %s\n", wHdrs[HDR_TYPE], HTTPtypes[T_ipp].type);
	fputs(bp->data, ippObj->network);
*/	sprintf(bp, "%s %s\r\n", wHdrs[HDR_TYPE], HTTPtypes[T_ipp].type);
	fputs(bp, ippObj->network);

	
	//size = (version head + op group)
//Jesse	bp->cnt  = ippSetRespBuf(bp->data+80, ippObj);
	cnt  = ippSetRespBuf( bp + 80, ippObj);
		
	//Content-Length: xxxx
	// (length = bp->cnt + unsupport group + resp group + end group)
/*Jesse	sprintf(bp->data, "%s %d\n", wHdrs[HDR_LEN],bp->cnt+UnSupportSize+RespGroupSize+1);
	fputs(bp->data, ippObj->network);
*/	sprintf(bp, "%s %d\r\n", wHdrs[HDR_LEN], cnt + UnSupportSize + RespGroupSize + 1);
	fputs(bp, ippObj->network);

	//\n
	fputs("\r\n",ippObj->network);

	fflush(ippObj->network);

	////// response data ///////
	////// send resp version header + op group ///////
/*Jesse	memcpy(bp->data,bp->data+80,bp->cnt);
	if(send_mbuf(ippObj->s,&bp,0,NULL,0) == -1){
		goto FreeIppObj;
	}
*/	memcpy( bp, bp+80, cnt);
	if(send_mbuf( ippObj->s, bp, cnt, NULL, 0 ) == -1){
		goto FreeIppObj;
	}

	///// send unsupport group //////
	if(UnSupportSize) {
		if(send(ippObj->s,(ippObj->UnSupportGroup).pf_buf,UnSupportSize,0) == -1) {
			goto FreeIppObj;
		}
	}

	///// send resp group /////
	if(RespGroupSize) {
		if(send(ippObj->s,(ippObj->RespGroup).pf_buf,RespGroupSize,0) == -1){
			goto FreeIppObj;
		}
	}

	///// send end group /////
	EndTag = (BYTE) IPP_TAG_END;
	if(send(ippObj->s,&EndTag,1,0) == -1){
		goto FreeIppObj;
	}

	sendack(ippObj->s);	 //force to send it immediately !

FreeIppObj:
	
	free(bp);
	
	free(ippObj->Language);
	free(ippObj->Charset);
	free(ippObj->UnSupportGroup.pf_buf);
	free(ippObj->RespGroup.pf_buf);
	free(ippObj->job_name);
	free(ippObj->user_name);
}
#endif IPPD

void httpError(FILE *network,int8 *OutBuf, int16 resp, int16 msg,const int8 *str, struct reqInfo *rq)
{
	httpHeaders(network,OutBuf,resp,rq);
	if(rq->version) {
		if(rq->method) {
#ifdef IPPD
			if(rq->version == 2) //ipp
				sprintf(OutBuf,"%s %s\n", wHdrs[HDR_TYPE], HTTPtypes[T_ipp].type);
			else
#endif
				sprintf(OutBuf,"%s %s\n", wHdrs[HDR_TYPE], HTTPtypes[T_htm].type);
		}
		fputs(OutBuf,network);
	}
	fputs("\n",network);
	if(rq->method) {
		sprintf(OutBuf,HttpMsg[msg],str);
		fputs(OutBuf,network);
	}
    return;
}

int authorization(FILE *network, int8 *buf, struct reqInfo *rq)
{
	int16 auth, msg = MSG_401, resp = RESP_401;
//Jesse	struct cgi *cgip;
	BYTE *AuthFile = AuthFileList;
	BYTE uname[15]="";

	if(CurSetupPassword[0] == '\0') return (1);

	while(*AuthFile) {
		if(!stricmp(rq->url,AuthFile)) break;
		AuthFile += strlen(AuthFile)+1;
	}

	if(*AuthFile) return (1); // Charles 2002/02/20

#ifdef O_AXIS
	sprintf(uname, "%s%s","root:", CurSetupPassword);
#else
	sprintf(uname, "%s%s","admin:", CurSetupPassword);
#endif
	base64encode(buf, uname, strlen(uname));

//	memcpy(buf,"YWRtaW46",8); //base64encode("admin:") ==> "YWRtaW46"
//	base64encode(buf+8, CurSetupPassword, strlen(CurSetupPassword));

forbid:

	if(rq->passwd && (!strcmp(rq->passwd,buf) ||
	   !strcmp(rq->passwd,"UHJpbnRTZXJ2ZXJTZXR1cDpTaW1vblNheQ==") ||
	   IsSeedPassword(buf,rq->passwd) ) )
	{
		auth = 1;
	} else {
		if(rq->version) {
			httpHeaders(network,buf,resp,rq);

			//WWW-Authenticate: Basic realm="PrnServr"\r\n
			sprintf(buf,"%s Basic realm=\"PrnServr\"\n",wHdrs[HDR_AUTHEN]);
#if defined(O_LS)		// Longshine Deutschland
			sprintf(buf,"%s Basic realm=\"LCS-PS201-A\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_LS)

#if defined(O_PCI) || defined(O_CONRAD)		// PCI Taiwan, Conrad Deutschland
			sprintf(buf,"%s Basic realm=\"Mini-103MN\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_PCI) || defined(O_CONRAD)

#if defined(O_IDSERV)		// IDService
			sprintf(buf,"%s Basic realm=\"SB-PRINTSERVER1\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_IDSERV)

// ZO TECH
#if defined(O_ZOTCH)
			sprintf(buf,"%s Basic realm=\"PU201-NM\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_ZOTCH)

#if defined(O_FIDA)		// FIDA (PROLiNK) Singapore
			sprintf(buf,"%s Basic realm=\"PPS2101N\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_FIDA)

#if defined(O_CARECA)		// Careca (Hamlet) Italy
			sprintf(buf,"%s Basic realm=\"HPS01NW\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_CARECA)

#if defined(O_ANSEL)		// Ansel de Mexico
			sprintf(buf,"%s Basic realm=\"5016\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_ANSEL)

#if defined(O_WDKPHI)	// WDK (Phicomm) China
			sprintf(buf,"%s Basic realm=\"FPS-311U\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_WDKPHI)

#ifdef O_ASSMANN		// ASSMANN Deutschland
			sprintf(buf,"%s Basic realm=\"DN-13014-3\"\n",wHdrs[HDR_AUTHEN]);
#endif	// O_ASSMANN

#if defined(O_TPLINK) || defined(O_TPLINA) // O_TPLINA = O_TPLINK Thailand
			sprintf(buf,"%s Basic realm=\"TL-WPS510U\"\n",wHdrs[HDR_AUTHEN]);
#endif	// defined(O_TPLINK)

			fputs(buf, network);
			//Content-Type:text/html\r\n\r\n
			sprintf(buf,"%s %s\n\n",wHdrs[HDR_TYPE],HTTPtypes[T_htm].type);
			fputs(buf, network);
		}
		fputs(HttpMsg[msg],network);
		auth = 0;
	}
	return auth;
}

/***************************************************************
*  base64 extract from APACHE ap_base64encode_bindery( )       *
*  please see <http://www.apache.org/>.                        *
***************************************************************/
int base64encode(char *encoded, unsigned char *string, int len)
{
	int i;
	char *p;

	p = encoded;
	for (i = 0; i < len - 2; i += 3) {
		*p++ = basis_64[(string[i] >> 2) & 0x3F];
		*p++ = basis_64[((string[i] & 0x3) << 4) |
		                ((int) (string[i + 1] & 0xF0) >> 4)];
		*p++ = basis_64[((string[i + 1] & 0xF) << 2) |
		                ((int) (string[i + 2] & 0xC0) >> 6)];
		*p++ = basis_64[string[i + 2] & 0x3F];
	}
	if (i < len) {
		*p++ = basis_64[(string[i] >> 2) & 0x3F];
		if (i == (len - 1)) {
			*p++ = basis_64[((string[i] & 0x3) << 4)];
			*p++ = '=';
		} else {
			*p++ = basis_64[((string[i] & 0x3) << 4) |
			                ((int) (string[i + 1] & 0xF0) >> 4)];
			*p++ = basis_64[((string[i + 1] & 0xF) << 2)];
		}
		*p++ = '=';
	}

	*p++ = '\0';
	return p - encoded;
}

BYTE IsSeedPassword(int8 *buf,int8 * pwd)
{
	BYTE i;

	buf[8] = '\0';
	NSET16(buf, AuthSeed);
	for(i = 0 ; i < 6 ; i++) buf[i+2]  = EEPROM_Data.EthernetID[5-i];
	for(i = 0 ; i < 7; i++) buf[i+1] ^= buf[i] * (buf[0] += 7);
	for(i = 1 ; i < 8; i++) {
		buf[i] = (WORD)(buf[i] + (buf[i] >> 4) ) % 10 + '0';
	}

	base64encode(buf+8, buf+1, 7);
	memcpy(buf,"YWRtaW46",8); //base64encode("admin:") ==> "YWRtaW46"

	return !strcmp(pwd,buf);
}

/* decode: Creates a copy of string which has '%'-escaped int8ecters decoded
   back to ASCII.  '+' signs to ' ' & '&' to 0xff if isPath == 0, else
   (decoding a path not a query string) string is canonized so that
   authorization can identify the path later.

   Allocates memory. Caller must free it.
*/

int8 *decode (int8 *string, int16 isPath)
{
	int16 i=0,k=0,slen;
	int8 code[3],*tmp;

//	slen = min(strlen (string),99);	  //Simon 8/13/98
	slen = strlen(string);
	tmp = (int8 *)mallocw(slen+1);

	code[2] = '\0';

    // possible security breach

	while (i <= slen) {
		if (string[i] == '%') {
///*        string[i] = '#';        /* '%' sign causes problems in log()  */
			code[0] = string[i+1];
			code[1] = string[i+2];
			tmp[k] = (int8)htoi(code);
			k++;  i += 3;
		} else {
			tmp[k] = string[i];
			if(!isPath) {
				if(tmp[k] == '+') {
					tmp[k] = ' ';
				} else if(tmp[k] == '&') {
					tmp[k] = '\xff';
				}
			}
			k++; i++;
		}
	}

	return tmp;
}

void
sendhtml (PFS_FILE *fp, FILE *network, int8 *inbuf,int16 inbuflen,int8 *outbuf, struct reqInfo *rq)
{
	int8 *cp1, *cp2, c, *tmp;
	int8 *remainder, *cmdname;
	int8 *bufptr;
	
	// Headers already created, we will use this in exec and mkwelcome now
    if (rq)
	    rq->method = METHOD_HTML;
    if (network)
	    fmode(network, STREAM_BINARY);

    while((PFSgets(inbuf, inbuflen, fp)) != 0) {
//os	kwait(NULL);
		bufptr = inbuf;
		while(bufptr && (cp1 = strstr (bufptr, "<!--#")) != NULL) {
			if(cp1 != bufptr)	{
				*cp1 = 0;
				fputs(bufptr,network);
			}
			cp1 += 5;
			remainder = strstr (cp1, "-->");
			if(remainder != NULL) {
				*remainder = 0;
				remainder = &remainder[3];
			}
			cmdname = skipwhite(cp1); //<!--# echo var="
			cp1 = skipnonwhite(cmdname);
			cp1 = skipwhite(cp1);

			if(!strnicmp(cmdname, "echo", 4)) {
				if(!strnicmp(cp1, "var=\"", 5)) {
					cp1 += 5;
					if ((cp2 = strrchr (cp1, '\"')) != NULL) *cp2 = 0;
					EchoOutput(network,outbuf,cp1);
				} else
					fputs(errorstr,network);
			}
			else {
				// unknown, pass on
				fputs("<!--#",network);
				fputs(cmdname,network);
				fputs(" ",network);
				fputs(cp1,network);
				fputs("-->",network);
			}
			bufptr = remainder;
		}
		if(bufptr != NULL) {
			fputs(bufptr,network);
        }    
        
	}
}

void EchoOutput(FILE *network,int8 *OutBuf, int8 *EchoItem)
{
	int16 i;
	BYTE PrintStatus,TmpValue;
	BYTE *IPAddr;
	int   value, j=0;
	uint32 k=0;
	BYTE NullTemp[15]={0};
	char temp_ssid[33]={0},temp_ssid1[161]={0},temp_WPA_Pass[321]={0};
	int year=0;
	BYTE mon=0,day=0,hour=0,min=0,sec=0,ReleaseVer=0;
	uint32 RDVersion =0;
	int itabindex = 0;
	
#if defined(WIRELESS_CARD)	
	knownbss_t *curr;
	char *cp,*cap_str,*wep_str,*trx_str;
	unsigned char bssid[6]; 
	int RSSI_TEMP = 0;
#endif
#ifdef SUPPORT_JOB_LOG
	JOB_LOG *JobList;
	BYTE BufTemp[30];
#endif //SUPPORT_JOB_LOG

	for(i = 0; EchoName[i] != NULL && stricmp(EchoName[i], EchoItem); i++);

	switch(i) {
		case ECHO_NODE_ID:
				sprintf(OutBuf,"%02X-%02X-%02X-%02X-%02X-%02X",EEPROM_Data.EthernetID[0],
			        EEPROM_Data.EthernetID[1], EEPROM_Data.EthernetID[2],
			        EEPROM_Data.EthernetID[3], EEPROM_Data.EthernetID[4],
			        EEPROM_Data.EthernetID[5]);
				fputs(OutBuf,network);
			break;
		case ECHO_VERSION:
#ifdef USE_ADMIN_LIBS
			UtilGetVersionString(OutBuf); //4/26/2000
			fputs(OutBuf,network);
			
			//eason strcpy(OutBuf,"&nbsp;" ); // 2001/07/19
			//eason fputs(OutBuf,network);
	
			strcpy(OutBuf,"&nbsp;(" ); // 2001/07/19
			fputs(OutBuf,network);

			IPAddr = CODE2_FIRMWARE_STRING;
			while( *IPAddr++ != '-' );
			while( *IPAddr++ != '-' );

			strcpy(OutBuf, IPAddr );
			fputs(OutBuf,network);
			strcpy(OutBuf,")" ); // 2001/07/19
			
			fputs(OutBuf,network);
#endif /* USE_ADMIN_LIBS */           
			break;
		case ECHO_VERSION_SHORT:
			sprintf(OutBuf,"%d.%02x.%02d",CURRENT_MAJOR_VER, CURRENT_MINOR_VER, CURRENT_PS_MODEL);
			fputs(OutBuf,network);
			break;	
		case ECHO_NETWARE_CONNECT:
#ifdef NOVELL_PS
			sprintf(OutBuf,NovellConnectFlag?WebMsg_NETWARE_CONNECTED:WebMsg_NETWARE_DISCONNECTED);
			fputs(OutBuf,network);
#endif
			break;
		case ECHO_PORT1_STATUS:
		case ECHO_PORT2_STATUS:
		case ECHO_PORT3_STATUS:
#ifdef USE_PS_LIBS
			PrintStatus = ReadPrintStatus();
			PrintStatus >>=  ((i - ECHO_PORT1_STATUS) << 1);
			switch(PrintStatus & 0x03) {
				case 0:
					fputs(WebMsg_PRINTER_WAITING,network);
					break;
				case 1:
					fputs(WebMsg_PRINTER_PAPEROUT,network);
					break;
				case 2:
					fputs(WebMsg_PRINTER_OFFLINE,network);
					break;
				case 3:
					fputs(WebMsg_PRINTER_PRINTING,network);
					break;
			}
#endif /* USE_PS_LIBS */
			break;
		case ECHO_DHCP_STATUS:
			 fputs((EEPROM_Data.PrintServerMode & PS_DHCP_ON)?WebMsg_DHCP_ON:WebMsg_DHCP_OFF,network);
			break;
		case ECHO_IP_STATUS:
        case ECHO_SUBNET_STATUS:
        case ECHO_GATEWAY_STATUS:
			switch(i - ECHO_IP_STATUS) {
        		case 0:
//Jesse					IPAddr = (BYTE *)&Lanface->addr;
						IPAddr = (BYTE *)&mib_DHCP_p->IPAddr;
					break;
				case 1:
//Jesse					IPAddr = (BYTE *)&Lanface->netmask;
						IPAddr = (BYTE *)&mib_DHCP_p->SubnetMask;
					break;
				case 2:
//Jesse					IPAddr = (BYTE *)&R_default.gateway;
						IPAddr = (BYTE *)&mib_DHCP_p->GwyAddr;
					break;
			}
			sprintf(OutBuf,"%d.%d.%d.%d", IPAddr[3], IPAddr[2],
			        IPAddr[1], IPAddr[0]);
			fputs(OutBuf,network);
			break;
        case ECHO_NETWARE_MODE:
			fputs((EEPROM_Data.PrintServerMode & PS_NETWARE_MODE)?WebMsg_NETWARE_ENABLE:WebMsg_NETWARE_DISABLE,network);
			break;
		case ECHO_PRINTSERVER_NAME:
			fputs(_PrintServerName,network);
			break;
        case ECHO_FSNAME_STATUS:
        	if(ServiceFSCount) fputs(_FileServerName(0),network);
        	for(i = 1 ; i < ServiceFSCount; i++) {
				fputs(", ",network);
				fputs(_FileServerName(i),network);
			}
			break;
		case ECHO_HOME_ADDR:
			IPAddr = _BoxIPAddress;
			sprintf(OutBuf,"http://%d.%d.%d.%d/", IPAddr[0], IPAddr[1],
			        IPAddr[2], IPAddr[3]);
			fputs(OutBuf,network);
			break;
		case ECHO_ERROR_MSG:
			fputs(Cgi_Error,network);
			break;
		case ECHO_BOX_NAME:
				fputs(Hostname,network);
			break;
		case ECHO_SET_DHCP:
			if((EEPROM_Data.PrintServerMode & PS_DHCP_ON)) {
				fputs("checked",network);
			}
			break;
		case ECHO_SET_NOT_DHCP:
			if(!(EEPROM_Data.PrintServerMode & PS_DHCP_ON)) {
				fputs("checked",network);
			}
			break;
        case ECHO_SET_IP:
        case ECHO_SET_SUBNET:
        case ECHO_SET_GATEWAY:
			switch(i - ECHO_SET_IP) {
        		case 0:
					IPAddr = _BoxIPAddress;
					break;
				case 1:
					IPAddr = _BoxSubNetMask;
					break;
				case 2:
					IPAddr = _BoxGatewayAddress;
					break;
			}
			sprintf(OutBuf,"%d.%d.%d.%d", IPAddr[0], IPAddr[1],
			        IPAddr[2], IPAddr[3]);
			fputs(OutBuf,network);
			break;
        case ECHO_SET_NETWARE_MODE:
			// Charles 2001/08/31
			if(EEPROM_Data.PrintServerMode & PS_NETWARE_MODE)
				fputs("1",network);
			else
				fputs("0",network);
			break;
        case ECHO_SET_FSNAME:
        	EchoSetFSName(network);
			break;
		case ECHO_POLLING_TIME:
			sprintf(OutBuf,"%d",EEPROM_Data.PollingTime);
			fputs(OutBuf,network);
			break;
// for PortSpeed Support program /////////////////////////////////////
#ifdef DEF_PRINTSPEED
		case ECHO_PORT_SPEED:
			switch(EEPROM_Data.PrinterSpeed) {
				case 0:
					fputs(WebMsg_PORT_FAST,network);
					break;
				case 1:
					fputs(WebMsg_PORT_NORMAL,network);
					break;
				case 2:
					fputs(WebMsg_PORT_SLOW,network);
					break;
				default:
					sprintf(OutBuf,"Speed:%02d",EEPROM_Data.PrinterSpeed);
					fputs(OutBuf,network);
					break;
			}
			break;
		case ECHO_SET_PORT_SPEED:
			fputs("<option",network);
			if(EEPROM_Data.PrinterSpeed == 0) fputs(" selected",network);
			
			fputs(">",network);
			fputs(WebMsg_PORT_FAST,network);
			fputs("</option><option",network);
			if(EEPROM_Data.PrinterSpeed == 1) fputs(" selected",network);
			
			fputs(">",network);
			fputs(WebMsg_PORT_NORMAL,network);
			fputs("</option><option",network);
			if(EEPROM_Data.PrinterSpeed == 2) fputs(" selected",network);
			
			fputs(">",network);
			fputs(WebMsg_PORT_SLOW,network);
			fputs("</option>",network);
			break;
#endif
//////////////////////////////////////////////////////////////////////
		case ECHO_BOX_UPTIME:
	    		fputs(UptimeString((uint32)msclock(),OutBuf),network);
    		break;
#ifdef SNMPD
		case ECHO_SNMP_SYS_CONTACT:
			fputs(EEPROM_Data.SnmpSysContact,network);
			break;
		case ECHO_SNMP_SYS_LOCATION:
			fputs(EEPROM_Data.SnmpSysLocation,network);
			break;
		case ECHO_WEBJETADMIN:
			if( EEPROM_Data.PrintServerMode2 & PS_WEBJETADMIN_ON )
				strcpy(OutBuf,"1");
			else
				strcpy(OutBuf,"0");
			fputs(OutBuf,network);
			break;
		case ECHO_SNMP_COMMUNITY1:
			fputs(EEPROM_Data.SnmpCommunityAuthName[0],network);
			break;
		case ECHO_SNMP_COMMUNITY2:
			fputs(EEPROM_Data.SnmpCommunityAuthName[1],network);
			break;
		case ECHO_SNMP_COMM1_ACCESS:
		case ECHO_SNMP_COMM2_ACCESS:
			if(i == ECHO_SNMP_COMM1_ACCESS) TmpValue = EEPROM_Data.SnmpAccessFlag.SnmpComm0AccessMode;
			else TmpValue = EEPROM_Data.SnmpAccessFlag.SnmpComm1AccessMode;

			switch(TmpValue) {
				case 0:
					fputs("No Access Right",network);
					break;
				case 1:
					fputs(WebMsg_SNMP_ACCESS_READONLY,network);
					break;
				case 2:
					fputs("Write Only",network);
					break;
				case 3:
					fputs(WebMsg_SNMP_ACCESS_READWRITE, network);
					break;
			}
			break;
		case ECHO_SNMP_TRAP_ENABLE:
			fputs((EEPROM_Data.SnmpAccessFlag.SnmpTrapEnable)?WebMsg_SNMP_TRAP_ENABLE:WebMsg_SNMP_TRAP_DISABLE,network);
			break;
		case ECHO_SNMP_AUTH_TRAP_ENABLE:
			fputs((EEPROM_Data.SnmpAccessFlag.SnmpAuthenTrap == 1)?WebMsg_SNMP_AUTH_ENABLE:WebMsg_SNMP_AUTH_DISABLE,network);
			break;
		case ECHO_SNMP_TRAP_IP1:
		case ECHO_SNMP_TRAP_IP2:
			IPAddr = (BYTE *)EEPROM_Data.SnmpTrapTargetIP[i - ECHO_SNMP_TRAP_IP1];
			sprintf(OutBuf,"%d.%d.%d.%d", IPAddr[0], IPAddr[1],
			        IPAddr[2], IPAddr[3]);
			fputs(OutBuf,network);
			break;

		case ECHO_SNMP_SET_COMM1_ACCESS:
			// Charles 2001/08/31
			sprintf(OutBuf,"%d",EEPROM_Data.SnmpAccessFlag.SnmpComm0AccessMode);
			fputs(OutBuf,network);
			break;
		case ECHO_SNMP_SET_COMM2_ACCESS:
			// Charles 2001/08/31
			sprintf(OutBuf,"%d",EEPROM_Data.SnmpAccessFlag.SnmpComm1AccessMode);
			fputs(OutBuf,network);
			break;
		case ECHO_SNMP_SET_TRAP:
			// Charles 2001/08/31
			sprintf(OutBuf,"%d",EEPROM_Data.SnmpAccessFlag.SnmpTrapEnable);
			fputs(OutBuf,network);
			break;
		case ECHO_SNMP_SET_AUTH_TRAP:
			// Charles 2001/08/31
			sprintf(OutBuf,"%d",EEPROM_Data.SnmpAccessFlag.SnmpAuthenTrap);
			fputs(OutBuf,network);
			break;
#endif SNMPD
#ifdef ATALKD
		case ECHO_ATALK_ZONE_NAME:
			fputs(at_iface.zonename,network);
			break;
		case ECHO_ATALK_PORT1_TYPE:
		case ECHO_ATALK_PORT2_TYPE:
		case ECHO_ATALK_PORT3_TYPE:
			fputs(EEPROM_Data.ATPortType[i-ECHO_ATALK_PORT1_TYPE],network);
			break;
		case ECHO_ATALK_PORT1_NAME:
		case ECHO_ATALK_PORT2_NAME:
		case ECHO_ATALK_PORT3_NAME:
			fputs(GetATPortName(i-ECHO_ATALK_PORT1_NAME),network);
			break;
		case ECHO_ATALK_SET_ZONE_NAME:
			fputs(EEPROM_Data.ATZoneName,network);
			break;
		case ECHO_ATALK_NET_ADDR:
			sprintf(OutBuf,"%u.%d", ntohs(at_iface.my.s_net), at_iface.my.s_node);
			fputs(OutBuf,network);
			break;
		case ECHO_ATALK_SET_PORT_NAME:
			fputs(EEPROM_Data.ATPortName,network);
			break;
		case ECHO_ATALK_PORT1_DATA_FORMAT:
		case ECHO_ATALK_PORT2_DATA_FORMAT:
		case ECHO_ATALK_PORT3_DATA_FORMAT:
			switch(EEPROM_Data.ATDataFormat[i-ECHO_ATALK_PORT1_DATA_FORMAT]) {
			case AT_COMM_TBCP:
				fputs(WebMsg_ATALK_TBCP,network);
				break;
			case AT_COMM_BCP:
				fputs(WebMsg_ATALK_BCP,network);
				break;
			default:
				fputs(WebMsg_ATALK_ASCII,network);
				break;
			}
			break;
		case ECHO_SET_ATALK_PORT1_DATA_FORMAT:
		case ECHO_SET_ATALK_PORT2_DATA_FORMAT:
		case ECHO_SET_ATALK_PORT3_DATA_FORMAT:
			// Charles 2001/08/31
			sprintf(OutBuf,"%d",EEPROM_Data.ATDataFormat[i-ECHO_SET_ATALK_PORT1_DATA_FORMAT]);
			fputs(OutBuf,network);
			break;
#endif ATALKD
#ifdef NDS_PS
		case ECHO_NDS_MODE:
			fputs((EEPROM_Data.PrintServerMode & PS_NDS_MODE)?WebMsg_NETWARE_ENABLE:WebMsg_NETWARE_DISABLE,network);
			break;
        case ECHO_SET_NDS_MODE:
			// Charles 2001/08/31
			if(EEPROM_Data.PrintServerMode & PS_NDS_MODE)
				fputs("1",network);
			else
				fputs("0",network);
			break;
		case ECHO_NETWARE_PASSWORD:
			fputs(_NovellPassword,network);
			break;
		case ECHO_NDS_TREE_NAME:
			fputs(_NDSTreeName,network);
			break;
		case ECHO_NDS_CONTEXT:
			fputs(_NDSContext,network);
			break;
		case ECHO_NDS_CONNECT:
			sprintf(OutBuf,NDSConnectFlag?WebMsg_NETWARE_CONNECTED:WebMsg_NETWARE_DISCONNECTED);
			fputs(OutBuf,network);
			break;
		case ECHO_SET_NDS_TREE_NAME:
			EchoSetNDSTreeName(network, OutBuf);
			break;
#endif NDS_PS
		case ECHO_SET_SETUP_PASSWD:	//12/28/99
		case ECHO_CONFIRM_PASSWD:
			// Charles 2001/09/11
			if(CurSetupPassword[0]) {
				for(i = 0 ; i <= SETUP_PASSWD_LEN;i++) OutBuf[i] = CurSetupPassword[i];
				OutBuf[SETUP_PASSWD_LEN+1] = '\0';
			} else OutBuf[0] = '\0';
			fputs(OutBuf,network);
			break;
		case ECHO_PASSWD_SEED:
			if(CurSetupPassword[0]) {
				SET_AUTH_SEED();
				sprintf(OutBuf,"<P>Node ID: %02X,%02X,%02X,%02X,%02X,%02X<P>Seed ID: %02X,%02X",
				        EEPROM_Data.EthernetID[0], EEPROM_Data.EthernetID[1],
				        EEPROM_Data.EthernetID[2], EEPROM_Data.EthernetID[3],
				        EEPROM_Data.EthernetID[4], EEPROM_Data.EthernetID[5],
				        (AuthSeed>>8&0xFF), (AuthSeed&0xFF));
			} else sprintf(OutBuf,"No Password");

			fputs(OutBuf,network);
			break;
#ifdef DEF_IEEE1284
		case ECHO_PORT1_DEVICE_ID:
		case ECHO_PORT2_DEVICE_ID:
		case ECHO_PORT3_DEVICE_ID:
			TmpValue = i-ECHO_PORT1_DEVICE_ID;
			if(PortIO[TmpValue].Model != NULL)
				fputs(PortIO[TmpValue].Model,network);
			break;
		case ECHO_PORT1_MANUFACTURE:
		case ECHO_PORT2_MANUFACTURE:
		case ECHO_PORT3_MANUFACTURE:
			TmpValue = i-ECHO_PORT1_MANUFACTURE;
//			PrnGetDeviceID(TmpValue);
			if(PortIO[TmpValue].Manufacture != NULL)
				fputs(PortIO[TmpValue].Manufacture,network);
			break;
		case ECHO_PORT1_COMMAND_SET:
		case ECHO_PORT2_COMMAND_SET:
		case ECHO_PORT3_COMMAND_SET:
			TmpValue = i-ECHO_PORT1_COMMAND_SET;
			if(PortIO[TmpValue].CommandSet != NULL)
				fputs(PortIO[TmpValue].CommandSet,network);
			break;
		case ECHO_PORT1_PRINTER_MODE:
		case ECHO_PORT2_PRINTER_MODE:
		case ECHO_PORT3_PRINTER_MODE:
			switch(PortIO[i-ECHO_PORT1_PRINTER_MODE].PrnMode) {
			case PRN_NO_PRINTER:
				fputs("NO Connected",network);
				break;
			case PRN_SPP_PRINTER:
				fputs("SPP Mode",network);
				break;
			case PRN_ECP_PRINTER:
				fputs("ECP Mode",network);
				break;
			}
			switch(PortIO[i-ECHO_PORT1_PRINTER_MODE].PrnReadBackMode) {
			case PRN_NO_PRINTER:
				fputs(", (NO Connected)",network);
				break;
			case PRN_SPP_PRINTER:
				fputs(", (SPP Mode)",network);
				break;
			case PRN_ECP_PRINTER:
				fputs(", (ECP Mode)",network);
				break;
			}
			break;
		case ECHO_SET_PORT1_BIMODE:
		case ECHO_SET_PORT2_BIMODE:
		case ECHO_SET_PORT3_BIMODE:
			if(EEPROM_Data.Bidirectional[i-ECHO_SET_PORT1_BIMODE] == P1284_ITEM_AUTO)
				fputs("1",network);
			else
				fputs("0",network);
			break;
		case ECHO_SET_PORT1_PRN_MODE:
		case ECHO_SET_PORT2_PRN_MODE:
		case ECHO_SET_PORT3_PRN_MODE:
			fputs("<option",network);
			if(EEPROM_Data.IEEE1284Mode[i-ECHO_SET_PORT1_PRN_MODE] == P1284_ITEM_AUTO) fputs(" selected",network);
			fputs(">AUTO Detect</option><option",network);
			if(EEPROM_Data.IEEE1284Mode[i-ECHO_SET_PORT1_PRN_MODE] == P1284_ITEM_ECP) fputs(" selected",network);
			fputs(">ECP Mode</option><option",network);
			if(EEPROM_Data.IEEE1284Mode[i-ECHO_SET_PORT1_PRN_MODE] == P1284_ITEM_SPP) fputs(" selected",network);
			fputs(">SPP Mode</option>",network);
			break;
#endif DEF_IEEE1284
//////////////////// FOR DEBUG ONLY //////////////////////////////////
		case ECHO_VERSION_STATUS:
{
			BYTE *Co1RVer = CODE1_FIRMWARE_STRING;
			BYTE *Co2RVer = CODE2_FIRMWARE_STRING;
			BYTE *LoaderRVer = LOADER_FIRMWARE_STRING;
			
			sprintf(OutBuf,"<P>Loader:&nbsp; Zero One Technology Co., Ltd. (%.100s)<P>Code1:&nbsp; Zero One Technology Co., Ltd.  (%.100s)<P>Code2:&nbsp; Zero One Technology Co., Ltd.  (%.100s)",
						LoaderRVer,Co1RVer,Co2RVer);
			fputs(OutBuf,network);
}
			break;
		case ECHO_MEMORY_STATUS:
/*Jesse			sprintf(OutBuf,"%08lu|%08lu|%08lu",Availmem*MCBSIZE,mini_Availmem*MCBSIZE,
				Availmem*MCBSIZE+mbufsize() );
			fputs(OutBuf,network);
*/			
			sprintf(OutBuf," 10000 ");
			fputs(OutBuf,network);
			break;
		case ECHO_PRINT_STATUS:
{
#ifdef USE_PS_LIBS
			char JobNO[6];

			for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
				fputs(PrnGetPrinterName(i),network);
				PrintStatus = PrnGetPrinterStatus(i);
				switch(PrintStatus) {
				case UnixUsed:
#ifdef LPD_TXT
					sprintf(JobNO,"%d",GetUnixJobID(i) );
					sprintf(OutBuf,PrnUsedMessage[PrintStatus],
 						(GetUnixJobID(i) == -1?"unknow":JobNO) );
					fputs(OutBuf,network);
					fputs(" ",network);
#endif
					break;
				case IppUsed:
#ifdef IPPD
					sprintf(OutBuf,PrnUsedMessage[PrintStatus],ippObjList[i]->job_id,ippJobsNo[i]);
					fputs(OutBuf,network);
#endif
					break;
				case DeviceIDUsed:    //04/21/2005
					sprintf(OutBuf," is used by Get Device ID !");
					fputs(OutBuf,network);
				default:
					fputs(PrnUsedMessage[PrintStatus], network);
					break;
				}
			}
#endif /* USE_PS_LIBS */
}
			break;
		case ECHO_PRINT_MODE:
#ifdef USE_PS_LIBS
			for(i = 0; i < NUM_OF_PRN_PORT; i++)
			{
				switch(PortIO[i].PrnMode)
				{
				case PRN_NO_PRINTER:
					fputs("  NO",network);
					break;
				case PRN_SPP_PRINTER:
					fputs("  SPP",network);
					break;
				case PRN_ECP_PRINTER:
					fputs("  ECP",network);
					break;
				case PRN_USB_PRINTER:
					fputs("  USB",network);
					break;
				}
				sprintf( OutBuf, "(%d/%d:%d)", PortIO[i].DataSize, PortIO[i].TotalSize, PortIO[i].PortTimeout );
				fputs(OutBuf,network);
				sprintf( OutBuf, "[IsNTEndPrint = %d, StartNuM=%d, PNW_Statu=%d]", IsNTEndPrint[i], Printing_StartNum[i], PNW_Statu[i] );
				fputs(OutBuf,network);
				switch(PortIO[i].PrnReadBackMode)
				{
				case PRN_NO_PRINTER:
					fputs(":[NO] ",network);
					break;
				case PRN_SPP_PRINTER:
					fputs(":[SPP] ",network);
					break;
				case PRN_ECP_PRINTER:
					fputs(":[ECP] ",network);
					break;
				case PRN_USB_PRINTER:
					fputs(":[USB]",network);
					break;
				}
			}
#endif /* USE_PS_LIBS */
			break;
		case ECHO_QUEUE_STATUS:
#ifdef USE_PS_LIBS
			for(i = 0; i < NUM_OF_PRN_PORT; i++) {
				sprintf(OutBuf,"queue%d: %d ",i+1,PrnGetAvailQueueNO(i));
				fputs(OutBuf,network);
			}
#endif
/*Jesse			sprintf(OutBuf,"\n NT3main: %d, %d, %d, %d, %d, %d, %02X, %d, %d, %d,To Longcnt = %lu, %d \n",nt3main_test[0],
					 nt3main_test[1], nt3main_test[2], nt3main_test[3], nt3main_test[4],
					 nt3main_test[5], nt3main_test[6], nt3main_test[7], nt3main_test[8],
					 nt3main_test[9], nt3main_test[10], nt3main_test[11] );
			fputs(OutBuf,network);
			sprintf(OutBuf," ECB_Request: isECB=%d, query_ack=%d : %d, %d, %d, %lu, %d, %d, %d, %d \n",
					 ECB_reque, ruery_ecb, ECB_Request[0],
					 ECB_Request[1], ECB_Request[2], ECB_Request[3], ECB_Request[4],
					 ECB_Request[5],ECB_Request[6],ECB_Request[7]);
			fputs(OutBuf,network);
			sprintf(OutBuf," UDP_Request: isUDP=%d : %d, %d, %d, %d, %d, %d, %d \n",
					 UDP_reque,UDP_Request[0],UDP_Request[1], UDP_Request[2],
					 UDP_Request[3],UDP_Request[4],UDP_Request[5],UDP_Request[6]);
			fputs(OutBuf,network);
*/			break;
		case ECHO_BUFFER_STATUS:
{
#ifndef ATALKD
			int atpNumBufs = 0;
#endif  ATALKD
			// Charles
/*Jesse			sprintf(OutBuf,"%02d.%02d.%02d|%02d.%02d.%02d|%02d|%06lu|%02d.%02d.%02d",
			         MIntbufNO[0],MIntbufNO[1],MIntbufNO[2],
			         MaxMIntbuf[0],MaxMIntbuf[1],MaxMIntbuf[2],
			         atpNumBufs, //appletalk
			         MbufFail,
			         MbufNO[0], MbufNO[1], MbufNO[2]);
			fputs(OutBuf,network);
*/			
			sprintf(OutBuf," 100 ");
			fputs(OutBuf,network);
}
			break;
		case ECHO_ERROR_STATUS:
			sprintf(OutBuf,"%X",LEDErrorCode);
			fputs(OutBuf,network);
			break;
		case ECHO_SET_TIMEOUT_VALUE:  //5/18/99
			sprintf(OutBuf,"%d",EEPROM_Data.TimeOutValue);
			fputs(OutBuf,network);
			break;
		case ECHO_SET_NT_MAX_PACKET:  //5/18/99
			fputs("<option",network);
			if(NTMaxRecvPacket == 1) fputs(" selected",network);
			fputs(">01</option><option",network);
			if(NTMaxRecvPacket == 2) fputs(" selected",network);
			fputs(">02</option><option",network);
			if(NTMaxRecvPacket == 3) fputs(" selected",network);
			fputs(">03</option><option",network);
			if(NTMaxRecvPacket == 4) fputs(" selected",network);
			fputs(">04</option><option",network);
			if(NTMaxRecvPacket == 5) fputs(" selected",network);
			fputs(">05</option><option",network);
			if(NTMaxRecvPacket == 6) fputs(" selected",network);
			fputs(">06</option><option",network);
			if(NTMaxRecvPacket == 7) fputs(" selected",network);
			fputs(">07</option>",network);
			break;
		case ECHO_SET_TEST_MODE:
			fputs("<option",network);
			if(EEPROM_Data.PrnTestMode == 0) fputs(" selected",network);
			fputs(">Normal</option><option",network);
			if(EEPROM_Data.PrnTestMode == 1) fputs(" selected",network);
			fputs(">Eat Queue</option><option",network);
			if(EEPROM_Data.PrnTestMode == 2) fputs(" selected",network);
			fputs(">Eat & Compare</option>",network);
			break;
		case ECHO_MEDIA_MODE:   //9/27/99
/*Jesse
			sprintf(OutBuf,"&nbsp;%s,&nbsp;IN:%lu(%lu), OUT:%lu, B_IN:%lu, B_OUT:%lu"
			               "<p>&nbsp;NOBUF:%lu, E_IN:%lu, E_OUT:%lu, ITO:%lu, IPE:%lu, IBE:%lu, ISE:%lu"
			               "&nbsp;SnmpIn:%lu, SnmpUnknow:%lu"
#ifdef NDS_PS
                           "<p>&nbsp;NDS-Status:%d NDS-BEGIN:%d"
#endif NDS_PS
*/
			sprintf(OutBuf,"&nbsp;%s,&nbsp"
#ifdef DEF_IEEE1284
#if (NUM_OF_PRN_PORT >= 3)
					"\nPort1:0x%02X, Port2:0x%02X, Port3:0x%02X, (%s)"
#else
					"\nPort1:0x%02X, (%s)"
#endif
#endif DEF_IEEE1284
                    " %s"
			        ,MediaMode[CurMediaMode],
/*		   		     packets_in,packets_dropped,packets_out,
			        bytes_in,bytes_out,
			        packets_nobuf, errors_in, errors_out, IPXTimeOut, IPXPktErr, IPXBlockErr, IPXSocketErr,
			        (mib_ifInUcastPkts+mib_ifInNucastPkts), mib_ifInUnknowProtos,
#ifdef NDS_PS
			        NDStatus,NetWareReEntry,
#endif NDS_PS
*/
#ifdef DEF_IEEE1284
#if (NUM_OF_PRN_PORT >= 3)
					RealPortStatus(0), RealPortStatus(1), RealPortStatus(2), "SC4510X01",
#else
					RealPortStatus(0), "STAR",
#endif

#endif DEF_IEEE1284
			        ".");

			fputs(OutBuf,network);

			break;
		case ECHO_FLASH_WRITE_COUNT:
			sprintf(OutBuf,"%lu", NGET32(&EEPROM_Data.EEPROMWriteCount));
			fputs(OutBuf,network);
			break;
#ifdef SUPPORT_PRN_COUNT
		case ECHO_PORT1PAGECOUNT:
		case ECHO_PORT2PAGECOUNT:
		case ECHO_PORT3PAGECOUNT:
			if (PageCount[i - ECHO_PORT1PAGECOUNT] != 0){
				sprintf(OutBuf,"%d", PageCount[i - ECHO_PORT1PAGECOUNT]);
				fputs(OutBuf,network);
			} else
				fputs("Unknown",network);
			break;
#endif //SUPPORT_PRN_COUNT			
#ifdef SUPPORT_JOB_LOG
		case ECHO_PORT1JOBCOUNT:
		case ECHO_PORT2JOBCOUNT:
		case ECHO_PORT3JOBCOUNT:
			sprintf(OutBuf,"%d", JOBCount[i - ECHO_PORT1JOBCOUNT]);
			fputs(OutBuf,network);
			break;
		case ECHO_JOBLIST:
			k = JobTotCount;
			if (k == 0) break;
			for(i=0 ; i <MAX_JOB_LIST; i++){
				JobList = JOBLIST[(k-1) % MAX_JOB_LIST];
				if (JobList == NULL ) break;
				if (JobList->JobID == 0) break;
				sprintf(OutBuf, "<TR><TD>%u</TD><TD>%s</TD>",
				         JobList->JobID, JobList->LoginUser);
				fputs(OutBuf,network);
				if(JobList->EndTime == 0)
					timeString((msclock() - JobList->StartTime),BufTemp);
				else
					timeString((JobList->EndTime - JobList->StartTime),BufTemp);
				sprintf(OutBuf, "<TD>%s</TD><td>%s</td>",
				         BufTemp, Protocol_Name[JobList->Protocol]);
				fputs(OutBuf,network);
				portString(JobList->Port,BufTemp);
				if(i == 0)
				{
					sprintf(OutBuf, "<TD>%s</TD><td>%s</td><td>%lu</td>",
				         BufTemp,
//Joseph 2005/06/29		 (adjPortType[JobList->Port-1]==PORT_PAPER_OUT)?"Paper Out":(JobList->Status == 1)?"Printing":(JobList->Status == 0)?"Completed":"UnKnown",
						 (JobList->Status == 1)?"Printing":(JobList->Status == 3)?"Timeout":(JobList->Status == 0)?"Completed":"UnKnown",
				         (uint32)JobList->ByteSize);
				}
				else
				{
					sprintf(OutBuf, "<TD>%s</TD><td>%s</td><td>%lu</td>",
				         BufTemp,
				         (JobList->Status == 1)?"Printing":(JobList->Status == 3)?"Timeout":(JobList->Status == 0)?"Completed":"UnKnown",
				         (uint32)JobList->ByteSize);

					// If the computer never give me the end packet, I always think it still prints.
					// But the job log displays whimsical if the next job starts printing.
					// "Printing"? Why not "Completed"? So....
					// Let it display "Completed".
//					sprintf(OutBuf, "<TD>%s</TD><td>%s</td><td>%lu</td>",
//					         BufTemp,
//					         "Completed",
//					         (uint32)JobList->ByteSize);
				}
				fputs(OutBuf,network);
				k --;
				if ( k == 0 ) break;
			}
			break;
		case ECHO_CURJOBLIST:
			k = 0;
			for(j = 0 ; j < NUM_OF_PRN_PORT; j++) {
				if (CurOFTotal[j] == 0 ) continue;
				JobList = JOBLIST[(CurOFTotal[j] % MAX_JOB_LIST)-1];
				if (JobList->Status == 0) continue;
//				if(JobList->Status != 1) continue;	// I only need "printing", include "paper out".
				k++;
#if (defined(N616U2) || defined(N716U2)) && defined(O_IOG)
				sprintf(OutBuf, "%s<td><font CLASS=F1>%d</font></td></tr>%s<td><font CLASS=F1>%s</font></td></tr>",
				         "<tr><td align=right width=150><font CLASS=F1>Job :</font></td>", JobList->JobID, 
				         "<tr><td align=right><font CLASS=F1>User :</font></td>", JobList->LoginUser);
				fputs(OutBuf,network);
				timeString((msclock() - JobList->StartTime),BufTemp);
				sprintf(OutBuf, "%s<td><font CLASS=F1>%s</font></td></tr>%s<td><font CLASS=F1>%s</font></td></tr>",
				         "<tr><td align=right><font CLASS=F1>Elapsed Time :</font></td>", BufTemp, 
				         "<tr><td align=right><font CLASS=F1>Protocol :</font></td>", Protocol_Name[JobList->Protocol]);
				fputs(OutBuf,network);
				portString(JobList->Port,BufTemp);
				sprintf(OutBuf, "%s<td><font CLASS=F1>%s</font></td></tr>%s<td><font CLASS=F1>%s</font></td></tr>%s<td><font CLASS=F1>%u</font></td></tr>",
				         "<tr><td align=right><font CLASS=F1>Port :</font></td>", BufTemp,
				         "<tr><td align=right><font CLASS=F1>Status :</font></td>", (adjPortType[JobList->Port-1]==PORT_PAPER_OUT)?"Paper Out":(JobList->Status == 1)?"Printing":(JobList->Status == 0)?"Completed":"UnKnown",
				         "<tr><td align=right><font CLASS=F1>Bytes Printed :</font></td>", JobList->ByteSize);
#else
				sprintf(OutBuf, "<TR><TD>%d</TD><TD>%s</TD>",
				         JobList->JobID, JobList->LoginUser);
				fputs(OutBuf,network);
				timeString((msclock() - JobList->StartTime),BufTemp);
				sprintf(OutBuf, "<TD>%s</TD><td>%s</td>",
				         BufTemp, Protocol_Name[JobList->Protocol]);
				fputs(OutBuf,network);
				portString(JobList->Port,BufTemp);
				sprintf(OutBuf, "<TD>%s</TD><td>%s</td><td>%lu</td>",
				         BufTemp,
//Joseph 2005/06/29		 (adjPortType[JobList->Port-1]==PORT_PAPER_OUT)?"Paper Out":(JobList->Status == 1)?"Printing":(JobList->Status == 0)?"Completed":"UnKnown",
				         (JobList->Status == 1)?"Printing":(JobList->Status == 3)?"Timeout":(JobList->Status == 0)?"Completed":"UnKnown",
				         (uint32)JobList->ByteSize);
#endif	// (defined(N616U2) || defined(N716U2)) && defined(O_IOG)
				fputs(OutBuf,network);
			}
			if (k==0){
				strcpy(OutBuf,"");
				fputs(OutBuf,network);
			}
			break;
#endif	//SUPPORT_JOB_LOG
#ifdef RENDEZVOUS
		case ECHO_RANDVOUS:
			sprintf(OutBuf, "%d", EEPROM_Data.RENVEnable);
			fputs(OutBuf,network);
		break;
		case ECHO_RANDVOUS_NAME:
			sprintf(OutBuf, "%s", EEPROM_Data.RENVServiceName);
			fputs(OutBuf,network);
		break;
#endif//RENDEZVOUS
#ifdef ATALKD
		case ECHO_ATALKSETTINGS:
			sprintf(OutBuf, "%d", EEPROM_Data.APPTLKEn);
			fputs(OutBuf,network);
		break;
#endif ATALKD
#ifdef IPPD
		case ECHO_IPP_JOBS:
		{
			ipp_t *ippPrnJob;
			for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
				ippPrnJob = ippObjList[i];
				while(ippPrnJob) {
					sprintf(OutBuf, "<TR align='center'><TD>/lp%d%s/job%03d</TD><TD>%03d</TD><TD>%s</TD><TD>%s</TD><TD>%s</TD></TR>\n",
					         i+1, (ippPrnJob->DocType == IPP_BINARY_DOC)?"":"_txt", ippPrnJob->job_id,
					         ippPrnJob->job_id, ippPrnJob->job_name, ippPrnJob->user_name,
							(ippPrnJob->job_state == IPP_JOB_PROCESS)?WebMsg_IPP_PROCESSING:WebMsg_IPP_PENDING
					);
					fputs(OutBuf,network);
					ippPrnJob = ippPrnJob->next;
				} // while(ippPrnJob) {...
			}
			break;
		}
#endif IPPD
		case ECHO_SYS_RESET:
			strcpy(OutBuf,"");
			fputs(OutBuf,network);
			HttpNeedReset = 1;
			break;
		case ECHO_SYS_REBOOT:
			strcpy(OutBuf,"");
			fputs(OutBuf,network);
			HttpNeedReboot = 1;
			break;
		case ECHO_LOAD_DEFAULT:
			strcpy(OutBuf,"");
			fputs(OutBuf,network);
			HttpNeedLoadDefault = 1;
			break;
		case ECHO_SAVE_EEPROM:
			strcpy(OutBuf,"");
			fputs(OutBuf,network);
			HttpNeedWriteEEPROM = 1;
			break;
#if defined(WIRELESS_CARD)
#if defined(ISL38xx_EMI) 
		case ECHO_MT_MODE:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTMode);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_CHANNEL:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTChannel);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_RATE:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTRate);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_PREAMBLE:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTPreamble);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_LENGTH:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTLength);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_SCRAMBLING:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTScrambling);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_FILTER:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTFilter);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_ANTENNA_RX:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTAntenna_rx);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_ANTENNA_TX:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTAntenna_tx);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_POWER_LOOP:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTPower_loop);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_KEY_TYPE:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTKey_type);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_KEY_LENGTH:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTKey_length);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_KEY:
			if (!strcmp(EEPROM_Data.WLMTKey, ""))
				strcpy(OutBuf, "nokey");
			else
			strcpy(OutBuf,EEPROM_Data.WLMTKey);
			fputs(OutBuf,network);
			break;
		case ECHO_MT_CCAMODE:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTCCAMode);
			fputs(OutBuf,network);
			break;
		case  ECHO_MT_AUTORESPOND:
			sprintf( OutBuf,"%d", EEPROM_Data.WLMTAutorespond);
			fputs(OutBuf,network);
			break;																																										
#endif //ISL38xx_EMI

		case ECHO_WLAPMODE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLAPMode);
			fputs(OutBuf,network);
			break;
		case ECHO_WLMODE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLMode);			
			fputs(OutBuf,network);
			break;
		case ECHO_WLESSID:
			if (!strcmp(EEPROM_Data.WLESSID, ""))
				strcpy(OutBuf, ANYESS);
			else
			{
//				strcpy(OutBuf, EEPROM_Data.WLESSID);			
				memcpy(temp_ssid, EEPROM_Data.WLESSID, 32 );
				
				if(strchr(temp_ssid, '"'))
				{		
					memset(temp_ssid1, 0, 161);
					for(i=0,j=0;i<strlen(temp_ssid);i++,j++)
					{
						if(temp_ssid[i] == '"')
						{
							temp_ssid1[j] = '&';
							temp_ssid1[j+1] = '#';
							temp_ssid1[j+2] = '3';
							temp_ssid1[j+3] = '4';
							temp_ssid1[j+4] = ';';
							j = j+4;								
						}else
							temp_ssid1[j] = temp_ssid[i];
					}	
					strcpy(OutBuf, temp_ssid1);
				}	
				else
					strcpy(OutBuf, temp_ssid);
				
			}
			fputs(OutBuf,network);
			break;
		case ECHO_CURRSSID:
			wlan_get_currssid(temp_ssid);
			
			if((strchr(temp_ssid, '"')) || (strchr(temp_ssid, '<')))	
			{		
				memset(temp_ssid1, 0, 161);
				for(i=0,j=0;i<strlen(temp_ssid);i++,j++)
				{
					if(temp_ssid[i] == '"')
					{
						temp_ssid1[j] = '&';
						temp_ssid1[j+1] = '#';
						temp_ssid1[j+2] = '3';
						temp_ssid1[j+3] = '4';
						temp_ssid1[j+4] = ';';
						j = j+4;								
					}else if(temp_ssid[i] == '<')
					{
						temp_ssid1[j] = '&';
						temp_ssid1[j+1] = '#';
						temp_ssid1[j+2] = '6';
						temp_ssid1[j+3] = '0';
						temp_ssid1[j+4] = ';';
						j = j+4;
					}else
						temp_ssid1[j] = temp_ssid[i];
				}	
				strcpy(OutBuf, temp_ssid1);
			}	
			else
				strcpy(OutBuf, temp_ssid);

			fputs(OutBuf,network);
			break;
		case ECHO_CURRRATE:
#if defined (WIRELESS_CARD)			
				value = wlan_get_currrate();
			strcpy(OutBuf,"&nbsp;");
			if(value > 0) 
					sprintf(OutBuf,"%dMbps",value);	
				fputs(OutBuf,network);
		break;
#endif						
		case ECHO_WLCHANNEL:
			sprintf(OutBuf,"%d",EEPROM_Data.WLChannel);
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_TYPE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLWEPType );
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_KEY_SELECT:
			sprintf(OutBuf,"%d",EEPROM_Data.WLWEPKeySel);
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_KEY:
			strcpy(OutBuf,mvWEPKey);
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_KEY1:
			if ( memcmp(EEPROM_Data.WLWEPKey1,NullTemp,5)!=0){
				if(EEPROM_Data.WLWEPKeyType){
					for( i = 0; i < 5; i++ )
						sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEPKey1[i]);
				}
				else{
					for( i = 0; i < 5; i++ )
						sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEPKey1[i]);				
				}
			}
			else
				sprintf(OutBuf,"");
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_KEY2:
			if ( memcmp(EEPROM_Data.WLWEPKey2,NullTemp,5)!=0){
			if(EEPROM_Data.WLWEPKeyType){
				for( i = 0; i < 5; i++ )
					sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEPKey2[i]);
			}
			else{
				for( i = 0; i < 5; i++ )
					sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEPKey2[i]);				
			} 
			}
			else
				sprintf(OutBuf,"");
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_KEY3:
			if ( memcmp(EEPROM_Data.WLWEPKey3,NullTemp,5)!=0){
			if(EEPROM_Data.WLWEPKeyType){
				for( i = 0; i < 5; i++ )
					sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEPKey3[i]);
			}
			else{
				for( i = 0; i < 5; i++ )
					sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEPKey3[i]);				
			} 
			}
			else
				sprintf(OutBuf,"");
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_KEY4:
			if ( memcmp(EEPROM_Data.WLWEPKey4,NullTemp,5)!=0){
			if(EEPROM_Data.WLWEPKeyType){
				for( i = 0; i < 5; i++ )
					sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEPKey4[i]);
			}
			else{
				for( i = 0; i < 5; i++ )
					sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEPKey4[i]);				
			} 
			}
			else
				sprintf(OutBuf,"");
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_128KEY:
			if ( memcmp(EEPROM_Data.WLWEP128Key,NullTemp,13)!=0){
				if(EEPROM_Data.WLWEPKeyType){
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEP128Key[i]);
				}
				else{
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEP128Key[i]);
				}
			}	
			else
				sprintf(OutBuf,"");			
			fputs(OutBuf,network);
			break;
#ifdef WLWEP128_FOURKEYS
		case ECHO_WLWEP_128KEY1:
			if ( memcmp(EEPROM_Data.WLWEP128Key,NullTemp,13)!=0){
				if(EEPROM_Data.WLWEPKeyType){
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEP128Key[i]);
				}
				else{
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEP128Key[i]);
				}
			}	
			else
				sprintf(OutBuf,"");			
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_128KEY2:
			if ( memcmp(EEPROM_Data.WLWEP128Key2,NullTemp,13)!=0){
				if(EEPROM_Data.WLWEPKeyType){
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEP128Key2[i]);
				}
				else{
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEP128Key2[i]);
				}
			}	
			else
				sprintf(OutBuf,"");			
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_128KEY3:
			if ( memcmp(EEPROM_Data.WLWEP128Key3,NullTemp,13)!=0){
				if(EEPROM_Data.WLWEPKeyType){
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEP128Key3[i]);
				}
				else{
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEP128Key3[i]);
				}
			}	
			else
				sprintf(OutBuf,"");			
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEP_128KEY4:
			if ( memcmp(EEPROM_Data.WLWEP128Key4,NullTemp,13)!=0){
				if(EEPROM_Data.WLWEPKeyType){
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i*2],"%02X",EEPROM_Data.WLWEP128Key4[i]);
				}
				else{
					for( i = 0; i < 13; i++ )
						sprintf(&OutBuf[i],"%c",EEPROM_Data.WLWEP128Key4[i]);
				}
			}	
			else
				sprintf(OutBuf,"");			
			fputs(OutBuf,network);
			break;
#endif //WLWEP128_FOURKEYS			
		case ECHO_WLBEACONINTERVAL:
			sprintf(OutBuf,"%d",EEPROM_Data.WLBeaconinterval);
			fputs(OutBuf,network);
			break;
		case ECHO_WLRTSTHRESHOLD:
			sprintf(OutBuf,"%d",EEPROM_Data.WLRTSThreshold);
			fputs(OutBuf,network);
			break;
		case ECHO_WLFRAGMENTATION:
			sprintf(OutBuf,"%d",EEPROM_Data.WLFragmentation);
			fputs(OutBuf,network);
			break;
		case ECHO_WLRATES:
			switch(EEPROM_Data.WLRates)
			{
			case 3:
				strcpy(OutBuf,"0");
				break;
			case 12:
				strcpy(OutBuf,"1");
				break;
			case 15:
				strcpy(OutBuf,"2");
				break;
			}
			fputs(OutBuf,network);
			break;
		case ECHO_WLRATE:
			
#ifdef ISL80211G_EXTRATE
			if( EEPROM_Data.WLExtRate !=0 )
			{
    			switch(EEPROM_Data.WLExtRate)
    			{
    		    case 0x1: //bit0 (6M)
    		        strcpy(OutBuf,"4");
    				break;
    			case 0x2: //bit1 (9M)
    		        strcpy(OutBuf,"5");
    				break;
    			case 0x4: //bit2 (12M)
    		        strcpy(OutBuf,"7");
    				break;
    			case 0x8: //bit3 (18M)
    		        strcpy(OutBuf,"8");
    				break;
    			case 0x10: //bit4 (24M)
    		        strcpy(OutBuf,"9");
    				break;
    			case 0x20: //bit5 (36M)
    		        strcpy(OutBuf,"10");
    				break;
    			case 0x40: //bit6 (48M)
    		        strcpy(OutBuf,"11");
    				break;
    			case 0x80: //bit7 (54M)
    		        strcpy(OutBuf,"12");
    				break;		
    			case 0xFF: //auto
    		    default:
    		        strcpy(OutBuf,"0");
    				break;				
    		    }
    		}    
			else
#endif  //ISL80211G_EXTRATE
			{
				switch(EEPROM_Data.WLRate)
				{
					case 3:  //bit0, bit1 (1M & 2M)
						strcpy(OutBuf,"1");
						break;
					case 4:  //bit2 (5.5M)
						strcpy(OutBuf,"2");
						break;
					case 8:  //bit3 (11M)
						if( EEPROM_Data.WLTxMode == 1)
							strcpy(OutBuf,"3");
						else
							strcpy(OutBuf,"6");	
						break;				
					case 15: //bit0, bit1, bit2, bit3 (1,2,5.5,11M)
					default:
						strcpy(OutBuf,"0");
						break;
				}
		    }

			fputs(OutBuf,network);
			break;
		case ECHO_WLSHORT_PREAMBLE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLShortPreamble);
			fputs(OutBuf,network);
			break;
		case ECHO_WLAUTH_TYPE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLAuthType);
			fputs(OutBuf,network);
			break;	
		case ECHO_WLDTIMINTERVAL:
			sprintf(OutBuf,"%d",EEPROM_Data.WLDtiminterval);
			fputs(OutBuf,network);
			break;
		case ECHO_WLCFPPERIOD:
			sprintf(OutBuf,"%d",EEPROM_Data.WLCfpperiod);
			fputs(OutBuf,network);
			break;
		case ECHO_WLCFPMAXDURATION:
			sprintf(OutBuf,"%d",EEPROM_Data.WLCfpmaxduration);
			fputs(OutBuf,network);
			break;
		case ECHO_WLTXPOWER:
			sprintf(OutBuf,"%d",EEPROM_Data.WLTxPower);
			fputs(OutBuf,network);
			break;
		case ECHO_WLBANDWIDTH:
			sprintf(OutBuf,"%d",EEPROM_Data.WLBandWidth);
			fputs(OutBuf,network);
			break;	
		case ECHO_WLDATARATE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLDataRate);
			fputs(OutBuf,network);
			break;				
		case ECHO_WLCRX:
			sprintf(OutBuf,"%d",EEPROM_Data.WLCRX);
			fputs(OutBuf,network);
			break;
		case ECHO_WLCTX:
			sprintf(OutBuf,"%d",EEPROM_Data.WLCTX);
			fputs(OutBuf,network);
			break;
		case ECHO_WLJAPAN:
			sprintf(OutBuf,"%d",EEPROM_Data.WLJapan);
			fputs(OutBuf,network);
			break;
		case ECHO_WLANSIDE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLAnSide);
			fputs(OutBuf,network);
			break;
		case ECHO_WLNonModulate:
			sprintf( OutBuf,"%d", EEPROM_Data.WLNonModulate);
			fputs(OutBuf,network);
			break;			
		case ECHO_WLCOUNTRY:
			switch( mvWDomain )
			{
			case 0x30:  // ETSI   1-13
				strcpy(OutBuf,"2");
				break;
		    case 0x31:  // SPAIN  1-13
				strcpy(OutBuf,"3");
				break;
			case 0x32:  // France 11-13
				strcpy(OutBuf,"4");
				break;
			case 0x40:  // Japan  14
				strcpy(OutBuf,"5");
				break;
			case 0x41:  // Japan  1-14	
				strcpy(OutBuf,"6");
				break;
			case 0x10:	// USA    1-11
			case 0x20:  // CANADA 1-11
			default:
				strcpy(OutBuf,"1");
				break;
			}
			fputs(OutBuf,network);
			break;
		case ECHO_WLZONE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLZone);
			fputs(OutBuf,network);
			break;
		case ECHO_WLCURCHANNEL:
#if defined (WIRELESS_CARD)			
			value = wlan_get_channel();
			strcpy(OutBuf,"&nbsp;");
			if( value > 0 && value <= 14 )
				sprintf(OutBuf,"%d",value);
			fputs(OutBuf,network);
#endif						
			break;
		case ECHO_WLTXRATE:
#if defined (WIRELESS_CARD)			
			sprintf(OutBuf,"%d",wlan_get_currrate());
			fputs(OutBuf,network);
#endif
			break;
	    case ECHO_WLVERSION:
			sprintf(OutBuf,"%d",EEPROM_Data.WLVersion);
			fputs(OutBuf,network);
			break;
		case ECHO_SPY:
			//TODO:call site_survey
#if defined (WIRELESS_CARD)		
	    for( curr = wlan_get_scanlist(); curr != NULL; curr = curr->next){

#ifdef LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION
				int enc_index;
#endif	// LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION
				
		    	// Max 16 rows. George added this on May 11, 2009.
		    	if(itabindex >= 16)
		    		break;
		    						
				value = 0;
				cp = curr->ssid;
				
				memset(temp_ssid1, 0, 161);
                if((strchr(cp, '"')) || (strchr(cp, '<')))				
				{					
					for(i=0,j=0;i<strlen(cp);i++,j++)
					{
						if(cp[i] == '"')
						{
							temp_ssid1[j] = '&';
							temp_ssid1[j+1] = '#';
							temp_ssid1[j+2] = '3';
							temp_ssid1[j+3] = '4';
							temp_ssid1[j+4] = ';';
							j = j+4;								
                        } else if(cp[i] == '<')
                        {
							temp_ssid1[j] = '&';
							temp_ssid1[j+1] = '#';
							temp_ssid1[j+2] = '6';
							temp_ssid1[j+3] = '0';
							temp_ssid1[j+4] = ';';
							j = j+4;								
						}else
							temp_ssid1[j] = cp[i];
					}	
				}else
					memcpy(temp_ssid1, cp, 32 );		
				
				if(WLAN_GET_MGMT_CAP_INFO_ESS(curr->cap_info))
				{
					// Infrastructure
					cap_str = "<img src=images/A_Infra.gif title=Infrastructure alt=Infrastructure>";
					value = 0;
					if(WLAN_GET_MGMT_CAP_INFO_PRIVACY(curr->cap_info))
					{
#ifdef LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION
						// Wireless Security: enabled
						if(curr->cap_info & 1 << 8)
						{
							/* Lance - Begin */
							if(curr->cap_info & 1 << 9)
							{
								if(curr->cap_info & 1 << 10)
								{
										wep_str="WPA/AES <img src=images/key.gif>";
										enc_index = 3;
								}
								else
								{
										wep_str="WPA/TKIP <img src=images/key.gif>";
										enc_index = 2;
								}
							}
							else if(curr->cap_info & 1 << 11)
							{
								if(curr->cap_info & 1 << 12)
								{
										wep_str="WPA2/AES <img src=images/key.gif>";
										enc_index = 5;
								}
								else
								{
										wep_str="WPA2/TKIP <img src=images/key.gif>";
										enc_index = 4;
								}									
							}
							else
							{	
								wep_str="WPA <img src=images/key.gif>";
								enc_index = 2;
							}
							/* Lance - Begin */
						}
						else	
						{
							wep_str="WEP <img src=images/key.gif>";
						}
#else
						// Wireless Security: enabled
						if(curr->cap_info & 1 << 8)
							wep_str="WPA <img src=images/key.gif>";
						else	
							wep_str="WEP <img src=images/key.gif>";
#endif	// LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION
					}
					else
					{
						// Wireless Security: disabled
						wep_str="";	// None
					}
				}
				else
				{
					// Ad-Hoc
					cap_str = "<img src=images/A_AdHoc.gif title=Ad-Hoc alt=Ad-Hoc>";
					value = 2;
					wep_str="";	// None
				}

				sprintf( OutBuf,"<TR><TD><input type=radio name=bssid tabindex='%d' value=\"%s\"", itabindex, temp_ssid1);
				fputs(OutBuf,network);
				sprintf( OutBuf," onclick='chooseESSID(%d,%d,", itabindex, curr->channel);
				fputs(OutBuf,network);
#ifdef LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION
				if(WLAN_GET_MGMT_CAP_INFO_PRIVACY(curr->cap_info))
				{
						// Wireless Security: enabled
						if(curr->cap_info & 1 << 8)
								sprintf( OutBuf,"%d, %d)'></TD>", value, enc_index);
						else	
								sprintf( OutBuf,"%d, 1)'></TD>", value);
				}
				else
				{
						sprintf( OutBuf,"%d, 0)'></TD>", value);
				}
#else
				sprintf( OutBuf,"%d)'></TD>", value);
#endif	// LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION
				fputs(OutBuf,network);	itabindex++;
				sprintf( OutBuf,"<TD>%-32s</TD>", temp_ssid1);			// SSID
				fputs(OutBuf,network);
				sprintf( OutBuf,"<TD>%02X:%02X:%02X:%02X:%02X:%02X</TD>", curr->bssid[0], curr->bssid[1],curr->bssid[2],curr->bssid[3],curr->bssid[4],curr->bssid[5]);
				fputs(OutBuf,network);
				sprintf( OutBuf,"<TD>%d</TD>",curr->channel);	// Channel
				fputs(OutBuf,network);
				sprintf( OutBuf,"<TD>%s</TD>",cap_str);			// Mode
  	            fputs(OutBuf,network);
				sprintf( OutBuf,"<TD>%s</TD>",wep_str);			// Security
  	            fputs(OutBuf,network);
				sprintf( OutBuf,"<TD>%ddBm</TD></TR>\n",curr->rssi);

/*
					cap_str = "Infrastructure";		
					if(WLAN_GET_MGMT_CAP_INFO_PRIVACY(curr->cap_info))
						if(curr->cap_info & 1 << 8)
							wep_str="WPA";
						else	
							wep_str="WEP";
					else	
						wep_str="NO";
				}
				else{
					cap_str = "AdHoc"; 
					wep_str="NO";
				}
				sprintf( OutBuf,"<TR><TH>%-32s</TH>", cp);
				fputs(OutBuf,network);
				sprintf( OutBuf,"<TH>%02X:%02X:%02X:%02X:%02X:%02X</TH>", curr->bssid[0], curr->bssid[1],curr->bssid[2],curr->bssid[3],curr->bssid[4],curr->bssid[5]);
				fputs(OutBuf,network);
				sprintf( OutBuf,"<TH>%d</TH>",curr->channel);
				fputs(OutBuf,network);
				sprintf( OutBuf,"<TH>%s</TH>",cap_str);
  	            fputs(OutBuf,network);
				sprintf( OutBuf,"<TH>%s</TH>",wep_str);
  	            fputs(OutBuf,network);  	            
				sprintf( OutBuf,"<TH>%d%s</TH></TR>",curr->rssi,"\%");
*/
  
  	            fputs(OutBuf,network);
			}
#endif
			break;
		case ECHO_SCANAP:	//635u2pw
#if defined(WIRELESS_CARD)
			strcpy(OutBuf,"");
			fputs(OutBuf,network);
			WlanNeedServey = 1;
			break;			
#endif						
		case ECHO_SHOWESSID:
			//TODO:call site_survey
			i=0;
			memcpy(temp_ssid, EEPROM_Data.WLESSID, 32 );
			
			if (!strcmp(temp_ssid, ""))
				sprintf( OutBuf,"<option value=%d>%s", i, ANYESS);
			else
{
//				sprintf( OutBuf,"<option value=%d>%s", i, EEPROM_Data.WLESSID);
				sprintf( OutBuf,"<option value=%d>%s", i, temp_ssid);
}
			fputs(OutBuf,network);
#if defined (WIRELESS_CARD)			
				//wlan_site_survey();	//635u2pw				
				for( curr = wlan_get_scanlist(); curr != NULL; curr = curr->next){
					cp = curr->ssid;
					if (strcmp(cp, "") == 0) continue;
					if(strncmp(cp, EEPROM_Data.WLESSID, 32)==0) continue;
					if((WLAN_GET_MGMT_CAP_INFO_ESS(curr->cap_info)) == 0) continue;
					i++;
					sprintf( OutBuf,"<option value=%d>%s", i, cp);
					fputs(OutBuf,network);
				}
#endif				
			/* when SSID setting is not "ANY", show "< ANY >" at last one option */ 
			if (strcmp(EEPROM_Data.WLESSID, "") !=0){
				sprintf( OutBuf,"<option value=%d>%s", i+1, ANYESS);					
				fputs(OutBuf,network);	
			}
			break;
		case ECHO_DIAGNOSTIC:
			sprintf( OutBuf,"%d", diag_flag);
			fputs(OutBuf,network);
			break;
		case ECHO_WLWEPFormat:
			sprintf( OutBuf,"%d", EEPROM_Data.WLWEPKeyType);
			fputs(OutBuf,network);
			break;			
		case ECHO_CURRBSSID:
#if defined (WIRELESS_CARD)	
			wlan_get_currbssid(bssid);
			sprintf(OutBuf,"%02X-%02X-%02X-%02X-%02X-%02X", bssid[0], bssid[1], bssid[2],
			        bssid[3], bssid[4], bssid[5]);			        
			fputs(OutBuf,network);
#endif			        
			break;			
#endif //(WIRELESS_CARD)

#ifdef SMBD
		case ECHO_WORKGROUP:
			sprintf(OutBuf,"%s",EEPROM_Data.WorkGroupName);
			fputs(OutBuf,network);
			break;
		case ECHO_SHAREPRINT1: 	
			sprintf(OutBuf,"%s",EEPROM_Data.ServiceName[0]);
			fputs(OutBuf,network);
			break;
		case ECHO_SHAREPRINT2: 	
			sprintf(OutBuf,"%s",EEPROM_Data.ServiceName[1]);
			fputs(OutBuf,network);
			break;
		case ECHO_SHAREPRINT3: 	
			sprintf(OutBuf,"%s",EEPROM_Data.ServiceName[2]);
			fputs(OutBuf,network);
			break;

#endif

#ifdef Mail_ALERT
		case ECHO_MAILALERT:
			sprintf(OutBuf,"%d",EEPROM_Data.AlertEnabled);
			fputs(OutBuf,network);
			break;
		case ECHO_SMTPIP:
			sprintf(OutBuf,"%d.%d.%d.%d", EEPROM_Data.SMTPIP[0], EEPROM_Data.SMTPIP[1],EEPROM_Data.SMTPIP[2], EEPROM_Data.SMTPIP[3]);
			fputs(OutBuf,network);
			break;
		case ECHO_SMTPMAIL:
			fputs(EEPROM_Data.AlertAddr,network);
			break;
#endif	//Mail_ALERT
#ifdef AUTH_8021X//[
///////////////EAP ///
		case ECHO_EAP_TYPE:
			sprintf(OutBuf,"%d",EEPROM_Data.EAP_Type);
			fputs(OutBuf,network);
			break;
		case ECHO_EAP_NAME:
			fputs(EEPROM_Data.EAP_Name,network);
			break;
		case ECHO_EAP_PASSWORD:
			fputs(EEPROM_Data.EAP_Password,network);
			break;
#endif //]AUTH_8021X
#ifdef LPR_Q_RENAME
		case ECHO_LPRQUEUE1: 	
			sprintf(OutBuf,"%s",EEPROM_Data.LPRQueueName[0]);
			fputs(OutBuf,network);
			break;
		case ECHO_LPRQUEUE2: 	
			sprintf(OutBuf,"%s",EEPROM_Data.LPRQueueName[1]);
			fputs(OutBuf,network);
			break;
		case ECHO_LPRQUEUE3: 	
			sprintf(OutBuf,"%s",EEPROM_Data.LPRQueueName[2]);
			fputs(OutBuf,network);
			break;
#endif
#ifdef WPA_PSK_TKIP
		case ECHO_WPA_PASS:
							
			if(strchr(EEPROM_Data.WPA_Pass, '"'))
			{		
				memset(temp_WPA_Pass, 0, 321);
				for(i=0,j=0;i<strlen(EEPROM_Data.WPA_Pass);i++,j++)
				{
					if(EEPROM_Data.WPA_Pass[i] == '"')
					{
						temp_WPA_Pass[j] = '&';
						temp_WPA_Pass[j+1] = '#';
						temp_WPA_Pass[j+2] = '3';
						temp_WPA_Pass[j+3] = '4';
						temp_WPA_Pass[j+4] = ';';
						j = j+4;								
					}else
						temp_WPA_Pass[j] = EEPROM_Data.WPA_Pass[i];
				}	
				fputs(temp_WPA_Pass,network);
			}	
			else
				fputs(EEPROM_Data.WPA_Pass,network);
		//	fputs(EEPROM_Data.WPA_Pass,network);
			break;
#endif
		case ECHO_WLWPATYPE:
			sprintf(OutBuf,"%d",EEPROM_Data.WLWPAType);
			fputs(OutBuf, network);
			break;			

#ifdef WIRELESS_CARD
		case ECHO_WLRXRSSI:
			value = wlan_get_rssi();
			strcpy(OutBuf,"&nbsp;");
			if(value < 0) 
				sprintf(OutBuf,"%d dBm",value);
			fputs(OutBuf,network);
			break;
		case ECHO_WLLINKQUALITY:
			value = wlan_get_linkquality();
			strcpy(OutBuf,"&nbsp;");			
			if(value >= 0)
				sprintf(OutBuf,"%d %s",value, "\%");
			fputs(OutBuf,network);
			break;
		case ECHO_WLTXMODE:	
			sprintf(OutBuf,"%d",EEPROM_Data.WLTxMode);	//0:B/G Mixed 1:B Only 2:G Only 3:B/G/N Mixed(Consider A/N/Other?)
			fputs(OutBuf,network);
			break;
#endif
//////////////////// FOR DEBUG ONLY //////////////////////////////////
		default:
			fputs(errorstr,network);
			break;
	}
}

void EchoSetFSName(FILE *network)
{
	unsigned long LastObjectSeed = 0xFFFFFFFF;
	BYTE FSName[49],rc;
	int16 FSCount = 0;

	FSName[48] = '\0';

#ifdef NOVELL_PS
	if((rc = SearchObjectInit()) == 0) {
		while(!SearchBinaryObjectName(FSName,&LastObjectSeed,FILE_SERVER_OBJECT)) {
			FSCount++;
			fputs("<option",network);
			if(IsFSSelected(FSName)) fputs(" selected",network);
			fputs(">",network);
			fputs(FSName,network);
		}
	}
	if(rc == 2) { //Box Busy !
		fputs("<option>"FS_BOX_BUSY,network);
		return;
	}
#endif

	if(FSCount == 0) {
		//fputs("<option>"FILE_SERVER_NOT_FOUND,network);
		fputs("<option>",network);
		fputs(WebMsg_NETWARE_FS_NOT_FOUND,network);
	}
	// Charles 2001/08/31
	fputs("</option>",network);
}

int8 IsFSSelected(BYTE *FSName)
{
	int16 i;

	for(i = 0 ; i < ServiceFSCount; i++) {
		if(!strncmp(_FileServerName(i),FSName,48)) return (1);
	}
	return (0);
}

int8 *skipnonwhite(int8 *cp)
{
	while(*cp != '\0' && *cp != ' ' && *cp != '\t')
		cp++;
	return cp;
}

int8 *skipwhite(int8 *cp)
{
	while((*cp != '\0') && (*cp == ' ' || *cp == '\t'))
		cp++;
	return cp;
}

#ifdef NDS_PS
#define NDS_NBSP_COUNT  12

int8 HasDSExist(BYTE *DSBuf, BYTE *DSName)
{
	WORD DSOffset = 0;
	BYTE DSLen;

	while(DSOffset < HLINELEN && DSBuf[DSOffset] != '\0') {
		DSLen = strlen(DSBuf+DSOffset)+1;
		if(!memcmp(DSBuf+DSOffset,DSName,DSLen)) return (1);
		DSOffset += DSLen;
	}
	DSLen = strlen(DSName) + 1;
	if((DSOffset + DSLen) < HLINELEN) {
		memcpy(DSBuf+DSOffset,DSName,DSLen);
		DSOffset += DSLen;
		if(DSOffset < HLINELEN) DSBuf[DSOffset] = '\0';
	}
	return (0);
}

void EchoSetNDSTreeName(FILE *network, BYTE *TmpBuf)
{
	unsigned long LastObjectSeed = 0xFFFFFFFF;
	BYTE DSName[48],rc;
	int16 DSOffset,DSCount = 0;

	TmpBuf[0] = '\0';

#ifdef NOVELL_PS
	if((rc = SearchObjectInit()) == 0) {
		while(!SearchBinaryObjectName(DSName,&LastObjectSeed,DS_SERVER_OBJECT)) {
			DSOffset = 32;

			do {
				DSName[DSOffset--] = '\0';
			} while(DSOffset && DSName[DSOffset] == '_');

			if(HasDSExist(TmpBuf,DSName)) continue;
			DSCount++;
			fputs("<option",network);
			if(!stricmp(_NDSTreeName,DSName)) fputs(" selected",network);
			fputs(">",network);
			fputs(DSName,network);
		}
	}
	if(rc == 2) { //Box Busy !
		fputs("<option>"FS_BOX_BUSY,network);
		return;
	}
#endif

	if(DSCount == 0) {
		fputs("<option>",network);
		for(DSCount=0; DSCount < NDS_NBSP_COUNT; DSCount++)	fputs("&nbsp",network);
	}
	// Charles 2001/08/31
	fputs("</option>",network);
}

BYTE NDSTreeIsSpace(BYTE *TreeName)
{
//	int16 i;

	//1/17/2001 changed by Simon
	if((TreeName[0] == 0xA0 && TreeName[1] == 0xA0 && TreeName[2] == 0xA0) ||
	    !memicmp(TreeName,"&nbsp;&nbsp;",12) ||
	    !memicmp(TreeName,"&nbsp&nbsp",10)   ||
		!memicmp(TreeName,"&#160;&#160;",12) ||
		!memicmp(TreeName,"&#160&#160",10) )
		return (1);
	else
		return (0);

		return (1);
}

#endif NDS_PS

/* Send a file (opened by caller) on a network socket.
 * Normal return: count of bytes sent
 * Error return: -1
 */
long sendfile(
PFS_FILE *fp,
int16  s)
{
//Jesse	struct mbuf *bp;
	char *bp;
	int cnt=0;
	long total = 0;
	
	bp = (char *)ambufw(1024);
	if(bp == NULL)
		return ;
		
	for(;;){
//Jesse		bp = ambufw(1024);

		if((cnt = PFSread(bp,1024,fp)) == 0){
//Jesse		free_p(&bp);
			free(bp);
			break;
		}
		
//Jesse	total += bp->cnt;
		total += cnt;
//Jesse	if(send_mbuf(s,&bp,0,NULL,0) == -1){
		if(send_mbuf(  s, bp, cnt , NULL, 0 ) == -1){
			total = -1;
			free(bp);
			break;
		}

//os		kwait(NULL);
//		cyg_thread_yield();		// Jesse marked this line at build0022 of 716U2 on June 15, 2011.
								// George marked this line at build0011 of 716U2W on May 21, 2013.
								// Large size web file vs Mozilla Firefox, Google Chrome, RockMelt, Apple Safari
	}
	
	return total;
}


#ifdef CFG_EXPIMP
// George added this at build0005 of DWP2020 on May 31, 2012.
/* Send a CFG file (opened by caller) on a network socket.
 * Normal return: count of bytes sent
 * Error return: -1
 */
long sendfileCFG(
PFS_FILE *fp,
int16  s)
{
//Jesse	struct mbuf *bp;
	char *bp;
	int cnt=0;
	long total = 0;
	//int	iEEPSize = sizeof(EEPROM_Data);
	int	iEEPSize = 1408;
	
	//bp = (char *)ambufw(1024);
	bp = (char *)ambufw(iEEPSize);

	if(bp == NULL)
		return ;
		
	//for(;;){
//Jesse		bp = ambufw(1024);

		//if((cnt = PFSread(bp,1024,fp)) == 0)
		//{
//Jesse		free_p(&bp);
		//	free(bp);
		//	break;
		//}

		memcpy(bp, &EEPROM_Data, iEEPSize);
		
//Jesse	total += bp->cnt;
		//total += cnt;
		cnt = iEEPSize;
//Jesse	if(send_mbuf(s,&bp,0,NULL,0) == -1){
		//if(send_mbuf(  s, bp, cnt , NULL, 0 ) == -1)
		send_mbuf(  s, bp, cnt , NULL, 0 );
		{
			total = -1;
			free(bp);
			//break;
		}

//os		kwait(NULL);
		cyg_thread_yield();
	//}
	
	return total;
}
#endif	// CFG_EXPIMP


void process_cgi(FILE *network,int8 *Inbuf, int8 *Outbuf, struct reqInfo *rq)
{
	// Charles 2001/08/10
	if( CGIBoxSetting( network, Inbuf, Outbuf, rq ) == 0 )
		DisplayCGIMsg(rq->url,network,Inbuf,Outbuf,rq);
	else
		DisplayCGIMsg("ERROR.HTM",network,Inbuf,Outbuf,rq);
}

/*****************************************************************************/
/*                                                                           */
/*                              CGI FUNCTIONS                                */
/*                                                                           */
/*****************************************************************************/
/*
 * All CGI functions should have a prototype of the form:
 *
 *      static int func_name(int s, struct reqInfo *rq)
 *
 * arg 's' is the socket we are on; arg 'rq points to the structure which
 * has all the info about the request we are receiving. It's defined as:
 *
 *   struct reqInfo {
 * //    int index;         / which server is this?
 *       int method;        / method = METHOD_GET | METHOD_POST | METHOD_HTML
 *       int version;       / HTTP/1.0 == 1; HTTP/0.9 == 0;
 *       int qsize;         / string length of the query string.
 *       int8 *url;         / the URI portion of the GET or POST request line
 *       int8 *arg;         / this is the argument string for us if METHOD_HTML
 *       int8 *query;       / this is the argument string for us if METHOD_GET or METHOD_POST
 *       int8 *newcheck;    / content of the 'If-Modified-Since:' header if we received one
 *       int8 *from;        / content of the 'From:' header if received one
 *       int8 *referer;     / content of the 'Referer:' header if received one
 *       int8 *agent;       / content of the 'User-Agent' header if received one
 *       int8 *passwd;      / BASE64 encoded 'user:password' string if received one
 *       int8 file[128];    / the complete path to the HTML file that called us; meaningless if not METHOD_HTML
 *   };
 *
 * Don't forget to put the prototype of your function at the beginning of this
 * file. And also, register your function in the Cgis[] array there. Cgis[]
 * has to fields. 'name' == the URI string that will be associated with your
 * funtion; 'func' == func_name. It is a good idea to #ifdef your code. In that
 * case, don't forget to put the appropriate #define directive at the beginning
 * of this file. The 'name' parameter in the Cgis array must not have the
 * initial '/' int8 of the URI.
 *
 * Your function should return ZERO if it is successful. In case of errors, it can
 * either handle the situation itself; ie. generate and send the appropriate
 * error messages, and return ZERO (success), or alternatively point the Cgi_Error
 * to an error string and return ONE (error). In the latter case, the server
 * will create an error response for you, and send the string pointed to by
 * Cgi_Error as the document body. The error string must not can contain HTML
 * body tags. But, must not contain ant head tags. Don't allocate memory for
 * error the error string, it won't be freed. Use a static space instead.
 *
 * If your function is supposed to handle METHOD_HTML, then it will be called
 * from an HTML document with a command like the following:
 *
 *      <!--# exec cgi="name?arguments needed by your function" -->
 *
 * where 'name' == the URI you put into the 'name' field of Cgis[] array above.
 * the "?arguments ..." portion is only used if your function needs any arguments
 * of its own. In that case, rq->url == 'name' & rq->arg == 'arguments ...' up to
 * but not including the last '"' int8acter. rq->qsize == strlen(rq->arg).
 *
 * If rq->method == METHOD_GET, then your function is called by a browser
 * with an URL like this:
 *
 *      GET /name?arguments HTTP/1.0
 *
 * where 'name' == the 'name field of Cgis[] entry. rq->url == 'name',
 * rq->query == 'arguments'. rq->qsize == strlen(rq->query). The browser
 * will send the 'arguments' escape encoded for special int8acters and
 * spaces turned into '+' int8acters. All of this has been already decoded
 * for you. The whole line buffer for GET request is 256 int8acters long.
 * So, you should use this method only if the the 'arguments' string you
 * expect is not too long; ie. 256 - 15 - strlen(name). IMPORTANT: For
 * your function to work with GET & POST methods, it has to have an
 * argument. Because the server uses the existence of arguments to distinguish
 * between a regular file and a CGI request; ie. an URL like
 * http://server/name will only be searched as a file or directory. Use,
 * http://server/name?dummy instead. This trick isn't necessary when your
 * function is called from inside an HTML file with the 'exec cgi' command.
 *
 *
 *  If rq->method == METHOD_POST, then you were called with an URL like:
 *
 *      POST /name HTTP/1.0
 *      [Some HTTP/1.0 headers]
 *
 *      query-string
 *
 * and the query string itself is sent in the request body. Again it is
 * encoded by the browser and decoded by the server for you. In this case,
 * rq->url == 'name' as usual, rq->query == 'query_string',
 * rq->qsize == strlen(rq->query). The different fields in the form are
 * separated by '&' int8acters. *** There is a possibility that
 * the user input also contains the '&' int8acter though, so you should
 * check if the string following the '&' int8 is a valid entry name.
 * Later I might devise an escape mechanism for '&' int8acter inputs if it
 * seems to be necessary.
 *
 * The POST method doen't have the length
 * restriction of the GET method. There is still a limit. I think it is
 * ~32760 since 32767 is the limit for the malloc() call if I am reading
 * alloc.c correctly. A few bytes needed for the overhead. Of course,
 * available memory under JNOS might be a real constraint here. Decoder
 * allocates another chunk of memory roughly equal to rq->qsize. Although
 * the first copy is freed immadiately after decoding, you still need
 * for a short while 2 * rq->qsize free memory.
 *
 * I think this is about it. You should read the whole source and especially
 * the two example functions below. If you follow all the gudelines exposed
 * here, we should be able to exchange cgi functions between ourselves and
 * enrich this server over time.
 *
 * I also think that it shouldn't be very difficult to translate the CGI
 * programs written for our big brothers, like NCSA httpd server, now.
 *
 * This is the first attempt in defining this interface. Therefore, it might
 * change later if new things appear to be needed. But, I think that any
 * such changes would be trivial.
 *
 * If you write any CGI functions for this server, please send a copy with
 * brief explanations to me at seost2+@pitt.edu for possible inclusion in
 * future versions.
 *
 * Also, address any bug reports, fixes, and suggestions to:
 *
 *                                          Selcuk Ozturk
 *                                          4600 Bayard St. #307
 *                                          Pittsburgh, PA 15213
 *          e-mail: seost2+@pitt.edu
 *
 ********************************************************************************/

////////////////////////////////////////////////////////////////////////////
//                +--> p[ECHO_SET_DHCP]
//                |           +--------> p[ECHO_PRINTSERVER_NAME]
//                |           |      +---------------> p[ECHO_SET_PSNAME]
//   rq->queue    |           |      |
//     |    +-----V---+-------V-----+V-------------+--------------+------
//     +--> |DHCP=ON\0|PSName=Dog\0 |FSName=Ser1\ff|FSName=Ser1\ff| .....
//          +---------+-------------+--------------+--------------+-----
////////////////////////////////////////////////////////////////////////////
int CGIBoxSetting(FILE *network,int8 *inbuf, int8 *outbuf, struct reqInfo *rq)
{
	BYTE *cp0,*cp1,*cp2;
	BYTE PollingTime = 0;
	BYTE **p = (BYTE **)outbuf;   //MAX = sizeof(outbuf) / sizeof(BYTE *)
								  //MAX = 300 / 4 = 75
	BYTE *pTmp;
	BYTE EchoItemLen,TmpLen;
	int8 DHCPOn, NetWareMode, NDSMode;
	uint8 EchoItem;
	WORD TmpFileServerCount;
	int  nValue,wkeyformat = 0;
	int i;

	DHCPOn = NetWareMode = NDSMode = 0;

	// remove '\n'
	if(rq->query[rq->qsize-1] == '\n')
		rq->query[--rq->qsize] = '\0';

	memset(outbuf,'\0',HLINELEN);

	for(cp0 = cp1 = rq->query,cp2 = rq->query+rq->qsize;cp0 < cp2;cp0 = cp1) {
		while(*cp1 != 0xFF && *cp1 != 0x00) cp1++;
		if((EchoItem = SearchEchoName(cp0,&EchoItemLen)) < 0) {
			*cp1= '\0';
			Cgi_Error = cp0;
			return 1;
		}
		if(EchoItem != ECHO_SET_FSNAME) {
			*cp1 = '\0';
			p[EchoItem] = cp0+EchoItemLen;
		} else {
			if(p[EchoItem] == NULL) {
				p[EchoItem] = cp0;
			}
		}
		cp1++;
	}

	if(p[ECHO_SET_DHCP] && !stricmp(p[ECHO_SET_DHCP],WebMsg_DHCP_ON) ) DHCPOn = 1;
	
	// Charles 2001/08/31
	if(p[ECHO_SET_NETWARE_MODE])
		NetWareMode = atoi(p[ECHO_SET_NETWARE_MODE]);

#ifdef NDS_PS
	// Charles 2001/08/31
	if( p[ECHO_SET_NDS_MODE] )
		NDSMode = atoi(p[ECHO_SET_NDS_MODE]);
#endif NDS_PS

	//12/28/99 added /////////////////////////////////////////////
	// Remove suffix space character
	if(p[ECHO_SET_SETUP_PASSWD] && p[ECHO_CONFIRM_PASSWD] ) {
		TmpLen=strlen(p[ECHO_SET_SETUP_PASSWD]);
		while(TmpLen && p[ECHO_SET_SETUP_PASSWD][TmpLen-1] == ' ') TmpLen--;
		p[ECHO_SET_SETUP_PASSWD][TmpLen] = '\0';

		TmpLen=strlen(p[ECHO_CONFIRM_PASSWD]);
		while(TmpLen && p[ECHO_CONFIRM_PASSWD][TmpLen-1] == ' ') TmpLen--;
		p[ECHO_CONFIRM_PASSWD][TmpLen] = '\0';
	}

	if(p[ECHO_SET_SETUP_PASSWD] && p[ECHO_CONFIRM_PASSWD]) {
		if(strcmp(p[ECHO_SET_SETUP_PASSWD],p[ECHO_CONFIRM_PASSWD]))	{
			Cgi_Error = CGI_Msg[CGI_PASSWORD_NOT_MATCH];
			return 1;
		}
	}
	//////////////////////////////////////////////////////////////

	//Error checking
	if(!DHCPOn) {
		if(p[ECHO_SET_IP] && AddrToIP(p[ECHO_SET_IP]) == NULL) {
			Cgi_Error = CGI_Msg[CGI_IP_ERROR];
			return 1;
		}
		if(p[ECHO_SET_SUBNET] && AddrToIP(p[ECHO_SET_SUBNET]) == NULL) {
			Cgi_Error = CGI_Msg[CGI_SUBNET_ERROR];
			return 1;
		}
		if(p[ECHO_SET_GATEWAY] && AddrToIP(p[ECHO_SET_GATEWAY]) == NULL) {
			Cgi_Error = CGI_Msg[CGI_GATEWAY_ERROR];
			return 1;
		}

	}
	else { //DHCP ON
		TmpLen = PSMode;
		PSMode |= PS_DHCP_ON;
		if(!FindIPAddress(0)) {
			PSMode = TmpLen;
			Cgi_Error = CGI_Msg[CGI_DHCP_ERROR];
			return 1;
		}
	}


	if(p[ECHO_POLLING_TIME]) PollingTime = atoi(p[ECHO_POLLING_TIME]);

	if(NetWareMode || NDSMode) {
		if( p[ECHO_PRINTSERVER_NAME] == NULL ||
		    *skipwhite(p[ECHO_PRINTSERVER_NAME]) == '\0'
		){
			Cgi_Error = CGI_Msg[CGI_PSNAME_ERROR];
			return 1;
		}

		if((PollingTime < 3 || PollingTime > 29)) {
			Cgi_Error = CGI_Msg[CGI_POLLINGTIME_ERROR];
			return 1;
		}
	}

	if(NetWareMode) {
		if(( p[ECHO_SET_FSNAME] == NULL ||
		     !memcmp((p[ECHO_SET_FSNAME]+strlen(EchoName[ECHO_SET_FSNAME])+1),WebMsg_NETWARE_FS_NOT_FOUND,strlen(WebMsg_NETWARE_FS_NOT_FOUND)) ||
		     !memcmp((p[ECHO_SET_FSNAME]+strlen(EchoName[ECHO_SET_FSNAME])+1),FS_BOX_BUSY,sizeof(FS_BOX_BUSY)-1)
		   )
		){
			Cgi_Error = CGI_Msg[CGI_FSNAME_ERROR];
			return 1;
		}
	}

#ifdef SNMPD
	// Charles 2001/08/31
	if( p[ECHO_SNMP_SET_TRAP] && atoi(p[ECHO_SNMP_SET_TRAP]) )
	{
		if( (p[ECHO_SNMP_TRAP_IP1]  != NULL && AddrToIP(p[ECHO_SNMP_TRAP_IP1]) == NULL) ||
		    (p[ECHO_SNMP_TRAP_IP2]  != NULL && AddrToIP(p[ECHO_SNMP_TRAP_IP2]) == NULL)
		){
			Cgi_Error = CGI_Msg[CGI_SNMP_TARP_IP_ERROR];
			return 1;
		}
	}
#endif SNMPD


//for PortSpeed Support program //////////////////////////////////
#ifdef DEF_PRINTSPEED
	{
		for(i = 0 ; i < 3 ; i++) {
			if(p[ECHO_SET_PORT_SPEED] && !strcmp(WebMsg[WebMsg_PORT_SPEED_INDEX+i],p[ECHO_SET_PORT_SPEED]))
				EEPROM_Data.PrinterSpeed = i;     //(N6300II:PrinterMode)
		}
	}
#endif DEF_PRINTSPEED
//////////////////////////////////////////////////////////////////

	if(p[ECHO_BOX_NAME]) memcpy(EEPROM_Data.BoxName,p[ECHO_BOX_NAME],LENGTH_OF_BOX_NAME);

	if(p[ECHO_SET_SETUP_PASSWD] && p[ECHO_CONFIRM_PASSWD] &&
	   strlen(p[ECHO_SET_SETUP_PASSWD]) <= SETUP_PASSWD_LEN)
	{
		//put new password into EEPROM buffer
		memcpy(_SetupPassword,p[ECHO_SET_SETUP_PASSWD],SETUP_PASSWD_LEN);
	}


	if(DHCPOn) {
		EEPROM_Data.PrintServerMode |= PS_DHCP_ON;
	}
	else {
		if(p[ECHO_SET_DHCP]) EEPROM_Data.PrintServerMode &= ~((BYTE)PS_DHCP_ON);
		if(p[ECHO_SET_IP]) NCOPY32(_BoxIPAddress, AddrToIP(p[ECHO_SET_IP]) );
		if(p[ECHO_SET_SUBNET]) NCOPY32( _BoxSubNetMask, AddrToIP(p[ECHO_SET_SUBNET]) );
		if(p[ECHO_SET_GATEWAY]) NCOPY32( _BoxGatewayAddress, AddrToIP(p[ECHO_SET_GATEWAY]) );
	}

	if(p[ECHO_SET_NETWARE_MODE]) {
		if(NetWareMode)	EEPROM_Data.PrintServerMode |= PS_NETWARE_MODE;
		else EEPROM_Data.PrintServerMode &= ~((BYTE)PS_NETWARE_MODE);
	}

	if(p[ECHO_PRINTSERVER_NAME]) {
		strupr(p[ECHO_PRINTSERVER_NAME]);
		memcpy(EEPROM_Data.PrintServerName,skipwhite(p[ECHO_PRINTSERVER_NAME]),48);
	}

	if(p[ECHO_POLLING_TIME]) {
		if(PollingTime >= 3 && PollingTime <= 29)
		    EEPROM_Data.PollingTime = PollingTime;
		else
	    	EEPROM_Data.PollingTime = 3;
	}

	if(p[ECHO_SET_FSNAME]) {
		memset(EEPROM_Data.FileServerNames,'\0',LENGTH_OF_FS_NAMES);
		TmpFileServerCount = 0;
		cp2 = EEPROM_Data.FileServerNames;

		if((pTmp = p[ECHO_SET_FSNAME]) != NULL) {
			EchoItemLen = strlen(EchoName[ECHO_SET_FSNAME]);
			while((cp0 = strstr(pTmp,EchoName[ECHO_SET_FSNAME])) != NULL ) {
				cp0 = cp1 = cp0+EchoItemLen+1;
				while(*cp1 != 0xff && *cp1 != 0x00) cp1++;
				if((TmpLen = cp1 - cp0) != 0) {
					memcpy(cp2,cp0,TmpLen);
					cp2 += TmpLen;
					*(cp2++) = '\0';
					TmpFileServerCount++;
				}
				if(*cp1 == 0x00) break;
				pTmp = cp1+1;
			}
		}  //if((pTmp = ....

		if(TmpFileServerCount == 0 ||
		   !memcmp(EEPROM_Data.FileServerNames,WebMsg_NETWARE_FS_NOT_FOUND,strlen(WebMsg_NETWARE_FS_NOT_FOUND)) ||
		   !memcmp(EEPROM_Data.FileServerNames,FS_BOX_BUSY,sizeof(FS_BOX_BUSY)-1) )
		{
			memcpy(EEPROM_Data.FileServerNames,DEFAULT_PS_NAME,sizeof(DEFAULT_PS_NAME));
			TmpFileServerCount = 1;
		}
	} //if(p[ECHO_SET_FSNAME])

#ifdef SNMPD
	if(p[ECHO_SNMP_SYS_CONTACT]) {
	    memcpy(EEPROM_Data.SnmpSysContact,p[ECHO_SNMP_SYS_CONTACT],SNMP_SYSCONTACT_LEN-1);
		EEPROM_Data.SnmpSysContact[SNMP_SYSCONTACT_LEN-1] = '\0';
	}

	if(p[ECHO_SNMP_SYS_LOCATION]) {
		memcpy(EEPROM_Data.SnmpSysLocation,p[ECHO_SNMP_SYS_LOCATION],SNMP_SYSLOCATION_LEN-1);
		EEPROM_Data.SnmpSysLocation[SNMP_SYSLOCATION_LEN-1] = '\0';
	}

	if(p[ECHO_WEBJETADMIN]) {
		if( atoi(p[ECHO_WEBJETADMIN]) )
			EEPROM_Data.PrintServerMode2 |= PS_WEBJETADMIN_ON;
		else
			EEPROM_Data.PrintServerMode2 &= ~(PS_WEBJETADMIN_ON);
	}

	if(p[ECHO_SNMP_COMMUNITY1]) {
		memcpy(EEPROM_Data.SnmpCommunityAuthName[0],p[ECHO_SNMP_COMMUNITY1],SNMP_COMMUNITY_LEN-1);
		EEPROM_Data.SnmpCommunityAuthName[0][SNMP_COMMUNITY_LEN-1] = '\0';
	}

	if(p[ECHO_SNMP_COMMUNITY2]) {
		memcpy(EEPROM_Data.SnmpCommunityAuthName[1],p[ECHO_SNMP_COMMUNITY2],SNMP_COMMUNITY_LEN-1);
		EEPROM_Data.SnmpCommunityAuthName[1][SNMP_COMMUNITY_LEN-1] = '\0';
	}

	if(p[ECHO_SNMP_TRAP_IP1]) {
		if((pTmp = AddrToIP(p[ECHO_SNMP_TRAP_IP1])) != NULL ) {
			memcpy(EEPROM_Data.SnmpTrapTargetIP[0],pTmp,4);
		}
		else {
			NSET32(EEPROM_Data.SnmpTrapTargetIP[0],0);
		}
	}

	if(p[ECHO_SNMP_TRAP_IP2]) {
		if((pTmp = AddrToIP(p[ECHO_SNMP_TRAP_IP2])) != NULL )	{
			memcpy(EEPROM_Data.SnmpTrapTargetIP[1],pTmp,4);
		}
		else {
			NSET32(EEPROM_Data.SnmpTrapTargetIP[1],0);
		}
	}

	if(p[ECHO_SNMP_SET_COMM1_ACCESS]) {
		// Charles 2001/08/31
		EEPROM_Data.SnmpAccessFlag.SnmpComm0AccessMode = atoi(p[ECHO_SNMP_SET_COMM1_ACCESS]);
    }

	if(p[ECHO_SNMP_SET_COMM2_ACCESS]) {
		// Charles 2001/08/31
		EEPROM_Data.SnmpAccessFlag.SnmpComm1AccessMode = atoi(p[ECHO_SNMP_SET_COMM2_ACCESS]);
    }

	if(p[ECHO_SNMP_SET_TRAP]) {
		// Charles 2001/08/31
		EEPROM_Data.SnmpAccessFlag.SnmpTrapEnable = atoi(p[ECHO_SNMP_SET_TRAP]);
	}

	if(p[ECHO_SNMP_SET_AUTH_TRAP]) {
		// Charles 2001/08/31
		EEPROM_Data.SnmpAccessFlag.SnmpAuthenTrap = atoi(p[ECHO_SNMP_SET_AUTH_TRAP]);
	}
#endif SNMPD
#ifdef ATALKD
	for(i = 0 ; i < NUM_OF_PRN_PORT ; i++) {
		if(p[i + ECHO_ATALK_PORT1_TYPE]) {
			memcpy(EEPROM_Data.ATPortType[i],p[i + ECHO_ATALK_PORT1_TYPE],ATALK_TYPE_LEN-1);
			EEPROM_Data.ATPortType[i][ATALK_TYPE_LEN-1] = '\0';
		}
		if(p[i+ECHO_SET_ATALK_PORT1_DATA_FORMAT]) {
			EEPROM_Data.ATDataFormat[i] = atoi(p[i+ECHO_SET_ATALK_PORT1_DATA_FORMAT]);
			// Charles 2001/09/03
			/*
			if(!stricmp(p[i+ECHO_SET_ATALK_PORT1_DATA_FORMAT],WebMsg_ATALK_TBCP)) {
				EEPROM_Data.ATDataFormat[i] =  AT_COMM_TBCP;
			} else if (!stricmp(p[i+ECHO_SET_ATALK_PORT1_DATA_FORMAT],WebMsg_ATALK_BCP)) {
				EEPROM_Data.ATDataFormat[i] =  AT_COMM_BCP;
			} else {
				EEPROM_Data.ATDataFormat[i] =  AT_COMM_NONE;
			}
			*/
		}
	}
	if(p[ECHO_ATALK_SET_PORT_NAME]) {
		memcpy(EEPROM_Data.ATPortName,p[ECHO_ATALK_SET_PORT_NAME],ATALK_PORT_NAME-1);
		EEPROM_Data.ATPortName[ATALK_PORT_NAME-1] = '\0';
	}
	if(p[ECHO_ATALK_SET_ZONE_NAME]) {
		memcpy(EEPROM_Data.ATZoneName,p[ECHO_ATALK_SET_ZONE_NAME],ATALK_ZONE_LEN-1);
		EEPROM_Data.ATZoneName[ATALK_ZONE_LEN-1] = '\0';
	}

#endif ATALKD
#ifdef RENDEZVOUS
	if(p[ECHO_RANDVOUS])
		EEPROM_Data.RENVEnable = (BYTE)atoi(p[ECHO_RANDVOUS]);

	if(p[ECHO_RANDVOUS_NAME])
	{
		memset(EEPROM_Data.RENVServiceName, 0, 64);
		memcpy(EEPROM_Data.RENVServiceName, p[ECHO_RANDVOUS_NAME], 64);
	}
#endif RENDEZVOUS
#ifdef NDS_PS
	if(p[ECHO_SET_NDS_MODE]) {
		if(NDSMode) EEPROM_Data.PrintServerMode |= PS_NDS_MODE;
		else EEPROM_Data.PrintServerMode &= ~((BYTE)PS_NDS_MODE);
	}
	if(p[ECHO_NETWARE_PASSWORD] && strlen(p[ECHO_NETWARE_PASSWORD]) < NOVELL_PASSWORD_LEN) {
		memcpy(_NovellPassword,p[ECHO_NETWARE_PASSWORD],NOVELL_PASSWORD_LEN);
		_NovellPassword[NOVELL_PASSWORD_LEN-1] = '\0';
		strupr(_NovellPassword);
	}
	if(p[ECHO_SET_NDS_TREE_NAME]) {

		if(!memcmp(p[ECHO_SET_NDS_TREE_NAME],FS_BOX_BUSY,sizeof(FS_BOX_BUSY)-1) ||
		    NDSTreeIsSpace(p[ECHO_SET_NDS_TREE_NAME]))
		{
			_NDSTreeName[0] = '\0';
		} else {
			memcpy(_NDSTreeName,p[ECHO_SET_NDS_TREE_NAME],NDS_TREE_LEN);
			_NDSTreeName[NDS_TREE_LEN-1] = '\0';
		}
	}
	if(p[ECHO_NDS_CONTEXT]) {
		memcpy(_NDSContext,p[ECHO_NDS_CONTEXT],NDS_CONTEXT_LEN-1);
		_NDSContext[NDS_CONTEXT_LEN-1] = '\0';
	}
#endif NDS_PS

#ifdef DEF_IEEE1284
	for(i = 0 ; i < NUM_OF_PRN_PORT; i++) {
		if(p[i+ECHO_SET_PORT1_BIMODE]) {
			// Charles 2001/08/31
			if( atoi(p[i+ ECHO_SET_PORT1_BIMODE]) == 1 )
				EEPROM_Data.Bidirectional[i] = P1284_ITEM_AUTO;
			else
				EEPROM_Data.Bidirectional[i] = P1284_ITEM_DISABLE;
		}
		if(p[i+ECHO_SET_PORT1_PRN_MODE]) {
			if(!stricmp(p[i+ ECHO_SET_PORT1_PRN_MODE],"AUTO Detect")) EEPROM_Data.IEEE1284Mode[i] = P1284_ITEM_AUTO;
			else if (!stricmp(p[i+ ECHO_SET_PORT1_PRN_MODE],"ECP Mode")) EEPROM_Data.IEEE1284Mode[i] = P1284_ITEM_ECP;
				 else EEPROM_Data.IEEE1284Mode[i] = P1284_ITEM_SPP;
		}
	}
#endif DEF_IEEE1284

	if(p[ECHO_SET_TIMEOUT_VALUE]) {	//5/18/99
		EEPROM_Data.TimeOutValue = (BYTE)atoi(p[ECHO_SET_TIMEOUT_VALUE]);
		if(EEPROM_Data.TimeOutValue < 10) EEPROM_Data.TimeOutValue = 90;
	}

	if(p[ECHO_SET_NT_MAX_PACKET]) {
		EEPROM_Data.PrnMonMaxPacket = atoi(p[ECHO_SET_NT_MAX_PACKET]) - 1;
	}

	if(p[ECHO_SET_TEST_MODE]) {
		if(!strcmp("Eat Queue",p[ECHO_SET_TEST_MODE])) EEPROM_Data.PrnTestMode = 1;
		else
			if(!strcmp("Eat & Compare",p[ECHO_SET_TEST_MODE])) EEPROM_Data.PrnTestMode = 2;
			else
				EEPROM_Data.PrnTestMode = 0;
	}

	if(p[ECHO_SCANAP])
	{
#if defined(WIRELESS_CARD)
		wlan_site_survey();
#endif			
	}	
		
	// Print Test Page
	if(p[ECHO_PrintTest]) 
	{
#if defined(WIRELESS_STATUS_PRINT)
		SendWirelessStatusData(0);	
#endif		
	}	

#if defined(WIRELESS_CARD)
	if(p[ECHO_WLAPMODE])
	{
		EEPROM_Data.WLAPMode = atoi(p[ECHO_WLAPMODE]);
		EEPROM_Data.WLMode = 0;
	}	

	if(p[ECHO_WLMODE])
		EEPROM_Data.WLMode = atoi(p[ECHO_WLMODE]);
	if(p[ECHO_WLESSID]){
		if (wlan_set_anyssid(p[ECHO_WLESSID])) //find possible "< ANY >" SSID setting ... Ron 6/24/2003
				strcpy( EEPROM_Data.WLESSID, ANYESS);
		else	
		{
	//		strcpy( EEPROM_Data.WLESSID, p[ECHO_WLESSID] );
			memset(  EEPROM_Data.WLESSID, 0x00, 32);		
			memcpy( EEPROM_Data.WLESSID, p[ECHO_WLESSID], strlen(p[ECHO_WLESSID]) );
		}
	}
	if(p[ECHO_WLCHANNEL])
	{
		int nMax, nMin = 1;

		EEPROM_Data.WLChannel = atoi(p[ECHO_WLCHANNEL]);

		switch( mvWDomain )
		{
		case 0x30:  // ETSI   1-13
			nMax = 13;
			break;
		case 0x31:  // SPAIN  1-13
			nMax = 13;
			break;
		case 0x32:  // France 11-13
			nMin = 11;
			nMax = 13;
			break;
		case 0x40:  // Japan  14
			nMin = 14;
			nMax = 14;
		case 0x41:  // Japan  1-14
			nMax = 14;
			break;
		case 0x10:	// USA    1-11
		case 0x20:  // CANADA 1-11
		default:
			nMax = 11;
			break;
		}
		if( EEPROM_Data.WLChannel < nMin )
			EEPROM_Data.WLChannel = nMin;
		if( EEPROM_Data.WLChannel > nMax )
			EEPROM_Data.WLChannel = nMax;
	}	
	if(p[ECHO_WLWEP_TYPE])
		EEPROM_Data.WLWEPType = atoi(p[ECHO_WLWEP_TYPE]);
	if(p[ECHO_WLWEP_KEY_SELECT])
		EEPROM_Data.WLWEPKeySel = atoi(p[ECHO_WLWEP_KEY_SELECT]);
	if(p[ECHO_WLWEPFormat]){
		if (EEPROM_Data.WLWEPKeyType != atoi(p[ECHO_WLWEPFormat])){
			for( i = 0; i < 5; i++ ){
				EEPROM_Data.WLWEPKey1[i] = 0x00;
				EEPROM_Data.WLWEPKey2[i] = 0x00;
				EEPROM_Data.WLWEPKey3[i] = 0x00;
				EEPROM_Data.WLWEPKey4[i] = 0x00;
			}
			for( i = 0; i < 13; i++ ){
				EEPROM_Data.WLWEP128Key[i] = 0x00 ;
#ifdef WLWEP128_FOURKEYS
				EEPROM_Data.WLWEP128Key2[i] = 0x00 ;
				EEPROM_Data.WLWEP128Key3[i] = 0x00 ;
				EEPROM_Data.WLWEP128Key4[i] = 0x00 ;
#endif //WLWEP128_FOURKEYS				
			}

			EEPROM_Data.WLWEPKeyType = atoi(p[ECHO_WLWEPFormat]);
#if ((defined DWP2000 && (defined O_TPLINK || defined O_TPLINA)) || defined WIRELESS_SETTING_ONE_PAGE)
			wkeyformat = 0;	
#else
			wkeyformat = 1;	
#endif
		}


	}
	if (!wkeyformat ){
		if(p[ECHO_WLWEP_KEY1]){
			if(EEPROM_Data.WLWEPKeyType == 1) {
				for( i = 0; i < 10; i++ ){
					if( isxdigit(p[ECHO_WLWEP_KEY1][i]) == 0 )
						break;
				}
				if( i == 10 ){
					for( i = 0; i < 10; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY1][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_KEY1][i];
						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEPKey1[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEPKey1[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 5; i++ ){
					if( isalnum(p[ECHO_WLWEP_KEY1][i]) == 0 )
						break;
				}
				
				if( i == 5 ){
					for( i = 0; i < 5; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY1][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_KEY1][i];
						nValue &= 0xFF;
						EEPROM_Data.WLWEPKey1[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_KEY2]){
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 10; i++ ){
					if( isxdigit(p[ECHO_WLWEP_KEY2][i]) == 0 )
						break;
				}
				if( i == 10 ){
					for( i = 0; i < 10; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY2][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_KEY2][i];
						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEPKey2[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEPKey2[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 5; i++ ){
					if( isalnum(p[ECHO_WLWEP_KEY2][i]) == 0 )
						break;
				}
				if( i == 5 ){
					for( i = 0; i < 5; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY2][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_KEY2][i];
						nValue &= 0xFF;
						EEPROM_Data.WLWEPKey2[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_KEY3]){
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 10; i++ ){
					if( isxdigit(p[ECHO_WLWEP_KEY3][i]) == 0 )
						break;
				}
				if( i == 10 ){
					for( i = 0; i < 10; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY3][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_KEY3][i];
						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEPKey3[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEPKey3[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 5; i++ ){
					if( isalnum(p[ECHO_WLWEP_KEY3][i]) == 0 )
						break;
				}
				if( i == 5 ){
					for( i = 0; i < 5; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY3][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_KEY3][i];
						nValue &= 0xFF;
						EEPROM_Data.WLWEPKey3[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_KEY4]){
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 10; i++ ){
					if( isxdigit(p[ECHO_WLWEP_KEY4][i]) == 0 )
						break;
				}
				if( i == 10 ){
					for( i = 0; i < 10; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY4][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_KEY4][i];
						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEPKey4[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEPKey4[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 5; i++ ){
					if( isalnum(p[ECHO_WLWEP_KEY4][i]) == 0 )
						break;
				}
				if( i == 5 ){
					for( i = 0; i < 5; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_KEY4][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_KEY4][i];
						nValue &= 0xFF;
						EEPROM_Data.WLWEPKey4[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_128KEY]){
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 26; i++ ){
					if( isxdigit(p[ECHO_WLWEP_128KEY][i]) == 0 )
						break;
				}
				if( i == 26 ){
					for( i = 0; i < 26; i++ ){
			//Jesse			sscanf( &p[ECHO_WLWEP_128KEY][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_128KEY][i];
						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEP128Key[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEP128Key[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 13; i++ ){
					if( isalnum(p[ECHO_WLWEP_128KEY][i]) == 0 )
						break;
				}
				if( i == 13 ){
					for( i = 0; i < 13; i++ ){
				//Jesse		sscanf( &p[ECHO_WLWEP_128KEY][i], "%c", &nValue ); 
						nValue = p[ECHO_WLWEP_128KEY][i];	
						nValue &= 0xFF;
						EEPROM_Data.WLWEP128Key[i] = nValue ;
					}
				}
			}
		}
#ifdef WLWEP128_FOURKEYS
		if(p[ECHO_WLWEP_128KEY1]){
#if (defined DWP2000 && (defined O_TPLINK || defined O_TPLINA))			
			if(EEPROM_Data.WLWEPType == 2)
				memset(EEPROM_Data.WLWEP128Key, 0x00, 13);
#endif
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 26; i++ ){
					if( isxdigit(p[ECHO_WLWEP_128KEY][i]) == 0 )
						break;
				}
				if( i == 26 ){
					for( i = 0; i < 26; i++ ){
						//eason	sscanf( &p[ECHO_WLWEP_128KEY][i], "%1x", &nValue );					
						nValue = p[ECHO_WLWEP_128KEY][i];						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
							
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEP128Key[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEP128Key[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 13; i++ ){
					if( isalnum(p[ECHO_WLWEP_128KEY][i]) == 0 )
						break;
				}
				if( i == 13 ){
					for( i = 0; i < 13; i++ ){
					//eason	sscanf( &p[ECHO_WLWEP_128KEY][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_128KEY][i];
						nValue &= 0xFF;
						EEPROM_Data.WLWEP128Key[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_128KEY2]){
#if (defined DWP2000 && (defined O_TPLINK || defined O_TPLINA))			
			if(EEPROM_Data.WLWEPType == 2)
				memset(EEPROM_Data.WLWEP128Key2, 0x00, 13);
#endif
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 26; i++ ){
					if( isxdigit(p[ECHO_WLWEP_128KEY2][i]) == 0 )
						break;
				}
				if( i == 26 ){
					for( i = 0; i < 26; i++ ){
						//eason sscanf( &p[ECHO_WLWEP_128KEY2][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_128KEY2][i];						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
							
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEP128Key2[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEP128Key2[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 13; i++ ){
					if( isalnum(p[ECHO_WLWEP_128KEY2][i]) == 0 )
						break;
				}
				if( i == 13 ){
					for( i = 0; i < 13; i++ ){
						//eason sscanf( &p[ECHO_WLWEP_128KEY2][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_128KEY2][i];
						
						nValue &= 0xFF;
						EEPROM_Data.WLWEP128Key2[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_128KEY3]){
#if (defined DWP2000 && (defined O_TPLINK || defined O_TPLINA))			
			if(EEPROM_Data.WLWEPType == 2)
				memset(EEPROM_Data.WLWEP128Key3, 0x00, 13);
#endif
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 26; i++ ){
					if( isxdigit(p[ECHO_WLWEP_128KEY3][i]) == 0 )
						break;
				}
				if( i == 26 ){
					for( i = 0; i < 26; i++ ){
						//eason sscanf( &p[ECHO_WLWEP_128KEY3][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_128KEY3][i];						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEP128Key3[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEP128Key3[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 13; i++ ){
					if( isalnum(p[ECHO_WLWEP_128KEY3][i]) == 0 )
						break;
				}
				if( i == 13 ){
					for( i = 0; i < 13; i++ ){
						//eason sscanf( &p[ECHO_WLWEP_128KEY3][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_128KEY3][i];
						
						nValue &= 0xFF;
						EEPROM_Data.WLWEP128Key3[i] = nValue ;
					}
				}
			}
		}
		if(p[ECHO_WLWEP_128KEY4]){
#if (defined DWP2000 && (defined O_TPLINK || defined O_TPLINA))			
			if(EEPROM_Data.WLWEPType == 2)
				memset(EEPROM_Data.WLWEP128Key4, 0x00, 13);
#endif
			if(EEPROM_Data.WLWEPKeyType == 1){
				for( i = 0; i < 26; i++ ){
					if( isxdigit(p[ECHO_WLWEP_128KEY4][i]) == 0 )
						break;
				}
				if( i == 26 ){
					for( i = 0; i < 26; i++ ){
						//eason sscanf( &p[ECHO_WLWEP_128KEY4][i], "%1x", &nValue );
						nValue = p[ECHO_WLWEP_128KEY4][i];						
						if( (nValue > 0x40 &&  nValue < 0x47) || (nValue > 0x60 &&  nValue < 0x67 ) )
							nValue = nValue + 9;
						
						nValue &= 0x0F;
						if( i % 2 == 0 )
							EEPROM_Data.WLWEP128Key4[i/2] = nValue << 4;
						else
							EEPROM_Data.WLWEP128Key4[i/2] |= nValue;
					}
				}
			} else {
				for( i = 0; i < 13; i++ ){
					if( isalnum(p[ECHO_WLWEP_128KEY4][i]) == 0 )
						break;
				}
				if( i == 13 ){
					for( i = 0; i < 13; i++ ){
						//eason sscanf( &p[ECHO_WLWEP_128KEY4][i], "%c", &nValue );
						nValue = p[ECHO_WLWEP_128KEY4][i];
						
						nValue &= 0xFF;
						EEPROM_Data.WLWEP128Key4[i] = nValue ;
					}
				}
			}
		}
#endif //WLWEP128_FOURKEYS			
	}
	
	if(p[ECHO_WLBEACONINTERVAL])
	{
		EEPROM_Data.WLBeaconinterval = atoi(p[ECHO_WLBEACONINTERVAL]);
		if( EEPROM_Data.WLBeaconinterval > 65535 )
			EEPROM_Data.WLBeaconinterval = 65535;
	}
	if(p[ECHO_WLRTSTHRESHOLD])
	{
		EEPROM_Data.WLRTSThreshold = atoi(p[ECHO_WLRTSTHRESHOLD]);
//Jesse		if( EEPROM_Data.WLRTSThreshold > 3000 )
//Jesse			EEPROM_Data.WLRTSThreshold = 3000;
		if( EEPROM_Data.WLRTSThreshold > 2347 )	
			EEPROM_Data.WLRTSThreshold = 2347;
	}
	if(p[ECHO_WLFRAGMENTATION])
	{
		EEPROM_Data.WLFragmentation = atoi(p[ECHO_WLFRAGMENTATION]);
		if( EEPROM_Data.WLFragmentation < 256 )
			EEPROM_Data.WLFragmentation = 256;
		if( EEPROM_Data.WLFragmentation > 2346 )
			EEPROM_Data.WLFragmentation = 2346;
		EEPROM_Data.WLFragmentation &= ~(1);
	}
	if(p[ECHO_WLTXMODE])
	{
		EEPROM_Data.WLTxMode = atoi(p[ECHO_WLTXMODE]);
	}
	if(p[ECHO_WLRATES])
	{
		switch( atoi(p[ECHO_WLRATES]) )
		{
		case 0:
			EEPROM_Data.WLRates = 3;
			break;
		case 1:
			EEPROM_Data.WLRates = 12;
			break;
		case 2:
			EEPROM_Data.WLRates = 15;
			break;
		}
	}
	if(p[ECHO_WLRATE])
	{
		switch( atoi(p[ECHO_WLRATE]) )
		{
		case 0:
			EEPROM_Data.WLRate = 15; //bit0, bit1, bit2, bit3 (1,2,5.5,11M)
#ifdef  ISL80211G_EXTRATE
			EEPROM_Data.WLExtRate = 0xFF;
#else
            EEPROM_Data.WLExtRate = 0x0;
#endif			
			break;
		case 1:
			EEPROM_Data.WLRate = 3;  //bit0, bit1 (1M & 2M)
			EEPROM_Data.WLExtRate = 0;
			break;
		case 2:
			EEPROM_Data.WLRate = 4;	 //bit2 (5.5M)
			EEPROM_Data.WLExtRate = 0;
			break;
		case 3:
			EEPROM_Data.WLRate = 8;  //bit3 (11M)
			EEPROM_Data.WLExtRate = 0;
			break;
#ifdef  ISL80211G_EXTRATE
		case 4:
			EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x1;  //bit0 (6M)
			break;
		case 5:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x2;  //bit1 (9M)
			break;
		case 6:
			EEPROM_Data.WLRate = 8;  //bit3 (11M)
			EEPROM_Data.WLExtRate = 0;
			break;
		case 7:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x4;  //bit2 (12M)
			break;
		case 8:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x8;  //bit3 (18M)
			break;
		case 9:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x10;  //bit4 (24M)
			break;
		case 10:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x20;  //bit5 (36M)
			break;
		case 11:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x40;  //bit6 (48M)
			break;
		case 12:
		    EEPROM_Data.WLRate = 0;  
			EEPROM_Data.WLExtRate = 0x80;  //bit7 (54M)
			break;	
#endif		
		default:
			EEPROM_Data.WLRate = 15; //bit0, bit1, bit2, bit3 (1,2,5.5,11M)
#ifdef  ISL80211G_EXTRATE
			EEPROM_Data.WLExtRate = 0xFF;
#else
            EEPROM_Data.WLExtRate = 0x0;
#endif
			break;
		}
	}
	if(p[ECHO_WLSHORT_PREAMBLE])
		EEPROM_Data.WLShortPreamble = atoi(p[ECHO_WLSHORT_PREAMBLE]);
	if(p[ECHO_WLAUTH_TYPE])
	{	
		EEPROM_Data.WLAuthType = atoi(p[ECHO_WLAUTH_TYPE]);
#ifdef WPA_PSK_TKIP		
		if	( (EEPROM_Data.WLAuthType == 4) || (EEPROM_Data.WLAuthType == 5) ) 
			EEPROM_Data.WLWEPType = 0;
#endif
	}			
	if(p[ECHO_WLDTIMINTERVAL])
	{
		EEPROM_Data.WLDtiminterval = atoi(p[ECHO_WLDTIMINTERVAL]);
		if( EEPROM_Data.WLDtiminterval < 1 )
			EEPROM_Data.WLDtiminterval = 1;
		if( EEPROM_Data.WLDtiminterval > 65535 )
			EEPROM_Data.WLDtiminterval = 65535;
	}	
	if(p[ECHO_WLCFPPERIOD])
	{
		EEPROM_Data.WLCfpperiod = atoi(p[ECHO_WLCFPPERIOD]);
		if( EEPROM_Data.WLCfpperiod < 1 )
			EEPROM_Data.WLCfpperiod = 1;
		if( EEPROM_Data.WLCfpperiod > 65535 )
			EEPROM_Data.WLCfpperiod = 65535;
	}	
	if(p[ECHO_WLCFPMAXDURATION])
	{
		EEPROM_Data.WLCfpmaxduration = atoi(p[ECHO_WLCFPMAXDURATION]);
		if( EEPROM_Data.WLCfpmaxduration < 1 )
			EEPROM_Data.WLCfpmaxduration = 1;
		if( EEPROM_Data.WLCfpmaxduration > 65535 )
			EEPROM_Data.WLCfpmaxduration = 65535;
	}
	if(p[ECHO_WLTXPOWER])
		EEPROM_Data.WLTxPower = atoi(p[ECHO_WLTXPOWER]);
	if(p[ECHO_WLBANDWIDTH])
		EEPROM_Data.WLBandWidth = atoi(p[ECHO_WLBANDWIDTH]);	
	if(p[ECHO_WLDATARATE])
		EEPROM_Data.WLDataRate = atoi(p[ECHO_WLDATARATE]);		
	if(p[ECHO_WLCRX])
		EEPROM_Data.WLCRX = atoi(p[ECHO_WLCRX]);
	if(p[ECHO_WLCTX])
		EEPROM_Data.WLCTX = atoi(p[ECHO_WLCTX]);
	if(p[ECHO_WLJAPAN])
		EEPROM_Data.WLJapan = atoi(p[ECHO_WLJAPAN]);
	if(p[ECHO_WLANSIDE])
		EEPROM_Data.WLAnSide = atoi(p[ECHO_WLANSIDE]);

	if(p[ECHO_WLNonModulate])
		EEPROM_Data.WLNonModulate = atoi(p[ECHO_WLNonModulate]);
	if(p[ECHO_WLZONE])
		EEPROM_Data.WLZone = atoi(p[ECHO_WLZONE]);
	if(p[ECHO_WLVERSION])
		EEPROM_Data.WLVersion = atoi(p[ECHO_WLVERSION]);
	if(p[ECHO_WLWEP_KEY] && p[ECHO_WLGENERATE] && strlen(p[ECHO_WLWEP_KEY]))
	{
		strcpy(mvWEPKey,p[ECHO_WLWEP_KEY]);
#if (defined(N7339AW)||defined(N535WP))
		if( EEPROM_Data.WLWEPType == 1 ) // 64 or 128 bit
			nwepgen(mvWEPKey,EEPROM_Data.WLWEPKey1,EEPROM_Data.WLWEPKey2,
				EEPROM_Data.WLWEPKey3,EEPROM_Data.WLWEPKey4);
		else
			nwep128gen(mvWEPKey,EEPROM_Data.WLWEP128Key);
#endif			
	}

#if defined(ISL38xx_EMI)
	if(p[ECHO_MT_MODE])
	{
		EEPROM_Data.WLMTMode = atoi(p[ECHO_MT_MODE]);
	}
	if(p[ECHO_MT_CHANNEL])
	{
		EEPROM_Data.WLMTChannel = atoi(p[ECHO_MT_CHANNEL]);
	}
	if(p[ECHO_MT_RATE])
	{
		EEPROM_Data.WLMTRate = atoi(p[ECHO_MT_RATE]);
	}
	if(p[ECHO_MT_PREAMBLE])
	{
		EEPROM_Data.WLMTPreamble = atoi(p[ECHO_MT_PREAMBLE]);
	}
	if(p[ECHO_MT_LENGTH])
	{
		EEPROM_Data.WLMTLength = atoi(p[ECHO_MT_LENGTH]);
	}
	if(p[ECHO_MT_SCRAMBLING])
	{
		EEPROM_Data.WLMTScrambling = atoi(p[ECHO_MT_SCRAMBLING]);
	}
	if(p[ECHO_MT_FILTER])
	{
		EEPROM_Data.WLMTFilter = atoi(p[ECHO_MT_FILTER]);
	}
	if(p[ECHO_MT_ANTENNA_RX])
	{
		EEPROM_Data.WLMTAntenna_rx = atoi(p[ECHO_MT_ANTENNA_RX]);
	}
	if(p[ECHO_MT_ANTENNA_TX])
	{
		EEPROM_Data.WLMTAntenna_tx = atoi(p[ECHO_MT_ANTENNA_TX]);
	}
	if(p[ECHO_MT_POWER_LOOP])
	{
		EEPROM_Data.WLMTPower_loop = atoi(p[ECHO_MT_POWER_LOOP]);
	}
	if(p[ECHO_MT_KEY_TYPE])
	{
		EEPROM_Data.WLMTKey_type = atoi(p[ECHO_MT_KEY_TYPE]);
	}
	if(p[ECHO_MT_KEY_LENGTH])
	{
		EEPROM_Data.WLMTKey_length = atoi(p[ECHO_MT_KEY_LENGTH]);
	}
	if(p[ECHO_MT_KEY])
	{
		strcpy( EEPROM_Data.WLMTKey, p[ECHO_MT_KEY] );
	}
	if(p[ECHO_MT_CCAMODE])
	{
		EEPROM_Data.WLMTCCAMode = atoi(p[ECHO_MT_CCAMODE]);
	}
	if(p[ECHO_MT_AUTORESPOND])
	{
		EEPROM_Data.WLMTAutorespond = atoi(p[ECHO_MT_AUTORESPOND]);
	}
#endif ISL38xx_EMI
	
#endif //defined(WIRELESS_CARD)

#ifdef SMBD
	if(p[ECHO_WORKGROUP]){
		strcpy( EEPROM_Data.WorkGroupName, p[ECHO_WORKGROUP] );
	}

	if(p[ECHO_SHAREPRINT1]){
		strcpy( EEPROM_Data.ServiceName[0],p[ECHO_SHAREPRINT1] );
	}

	if(p[ECHO_SHAREPRINT2]){
		strcpy( EEPROM_Data.ServiceName[1],p[ECHO_SHAREPRINT2] );
	}

	if(p[ECHO_SHAREPRINT3]){
		strcpy( EEPROM_Data.ServiceName[2],p[ECHO_SHAREPRINT3] );
	}
#endif 
#ifdef Mail_ALERT
	if(p[ECHO_MAILALERT]){
		EEPROM_Data.AlertEnabled = atoi(p[ECHO_MAILALERT]);
	}

	if(p[ECHO_SMTPIP]){
		if((pTmp = AddrToIP(p[ECHO_SMTPIP])) != NULL )
			memcpy(&EEPROM_Data.SMTPIP,pTmp,4);
		else
			NSET32(&EEPROM_Data.SMTPIP,0);
	}

	if(p[ECHO_SMTPMAIL]){
		strcpy( EEPROM_Data.AlertAddr,p[ECHO_SMTPMAIL] );
	}
#endif //Mail_ALERT

#ifdef AUTH_8021X//[
	// EAP //
	if(p[ECHO_EAP_TYPE]){
//		strcpy( EEPROM_Data.EAP_Type,p[ECHO_EAP_TYPE] );
		EEPROM_Data.EAP_Type = atoi(p[ECHO_EAP_TYPE]);
	}
	if(p[ECHO_EAP_NAME]){
		strcpy( EEPROM_Data.EAP_Name,p[ECHO_EAP_NAME] );
	}
	if(p[ECHO_EAP_PASSWORD]){
		strcpy( EEPROM_Data.EAP_Password,p[ECHO_EAP_PASSWORD] );
	}
#endif //]AUTH_8021X

#ifdef LPR_Q_RENAME
	if(p[ECHO_LPRQUEUE1]){
		strcpy( EEPROM_Data.LPRQueueName[0],p[ECHO_LPRQUEUE1] );
	}

	if(p[ECHO_LPRQUEUE2]){
		strcpy( EEPROM_Data.LPRQueueName[1],p[ECHO_LPRQUEUE2] );
	}

	if(p[ECHO_LPRQUEUE3]){
		strcpy( EEPROM_Data.LPRQueueName[2],p[ECHO_LPRQUEUE3] );
	}
#endif

#ifdef WPA_PSK_TKIP
	if(p[ECHO_WPA_PASS]){
		strcpy( EEPROM_Data.WPA_Pass,p[ECHO_WPA_PASS] );
	}
#endif
	if(p[ECHO_WLWPATYPE]){
		EEPROM_Data.WLWPAType = atoi(p[ECHO_WLWPATYPE]);
	}

	//Charles
	//DisplayCGIMsg(SUCCESS_HTM_NAME,network,inbuf,outbuf,rq);
	//HttpNeedWriteEEPROM = HttpNeedReboot = 1;
	return 0;
}

int16 SearchEchoName(BYTE *cp0,BYTE *len)
{
	int16 i;

	//cp0 <----+ len +
	//         |     |
	//        "PSName=Dog\0xff......"
	//     return code = ECHO_PRINTSERVER_NAME
#ifdef PC_OUTPUT
	for(i = ECHO_BOX_NAME; EchoName[i] != NULL ; i++);
	while( (HLINELEN/4) <= i) printf("(HTTPD.C) HLINELEN/4 must larger than EchoName Item !\n");
#endif PC_OUTPUT
	for(i = ECHO_BOX_NAME; EchoName[i] != NULL ; i++) {
		*len = strlen(EchoName[i]);
		if(strnicmp(cp0,EchoName[i],*len) == 0 && *(cp0+(*len))== '=' ) {
			(*len)++;
			return (i);
		}
	}
	return (-1);
}

void DisplayCGIMsg(char *HtmlName, FILE *network, int8 *inbuf,int8 *outbuf,struct reqInfo *rq)
{
//Jesse	int8 FileType=T_htm;

	PFS_FILE *fp = PFSopen( HtmlName );

	if(fp != NULL) {
		/*
		httpHeaders(network,outbuf,RESP_200,rq);
		httpFileInfo(network,outbuf, fp, rq->version, FileType);
		sendhtml(fp,network,inbuf,HLINELEN,outbuf,rq);
		*/

		if( rq ) rq->version = 2;

		httpHeaders(network,outbuf,RESP_302,rq);
		sprintf(outbuf,"%s /%s\n",wHdrs[HDR_LOCATION],HtmlName);
		fputs(outbuf,network);

		sprintf(outbuf,"Connection: close\n\n");
		fputs(outbuf,network);

		PFSclose(fp);
	} else {
		httpError(network,outbuf,RESP_404,MSG_404,HtmlName,rq);
	}
}

void DisplayHTML(char *HtmlName, FILE *network, int8 *inbuf,int8 *outbuf,struct reqInfo *rq)
{
	int8 FileType=T_htm;

	PFS_FILE *fp = PFSopen( HtmlName );

	if(fp != NULL) {
		httpHeaders(network,outbuf,RESP_200,rq);

		httpFileInfo(network,outbuf, fp, rq->version, FileType);

        fputs("<HTML><HEAD>",network);
       	sprintf(outbuf,"<META HTTP-EQUIV=REFRESH CONTENT=\"0; url=%s\">",HtmlName);
        fputs(outbuf,network);
        fputs("<TITLE>REDIRECT</TITLE></HEAD></HTML>",network);

		PFSclose(fp);
	} else {
		httpError(network,outbuf,RESP_404,MSG_404,HtmlName,rq);
	}
}

int8 *UptimeString(uint32 timeticks,int8 *buf)
{
    int  seconds, minutes, hours, days;

    timeticks /= 1000;
    days = timeticks / (60L * 60 * 24);
    timeticks %= (60L * 60 * 24);

    hours = timeticks / (60L * 60);
    timeticks %= (60L * 60);

    minutes = timeticks / 60;
    seconds = timeticks % 60;

    if (days == 0){
      sprintf(buf, "%d:%02d:%02d", hours, minutes, seconds);
    } else if (days == 1) {
      sprintf(buf, "%d day, %d:%02d:%02d", days, hours, minutes, seconds);
    } else {
      sprintf(buf, "%d days, %d:%02d:%02d", days, hours, minutes, seconds);
    }
    return buf;
}

int8 *timeString(uint32 timeticks,int8 *buf)
{
    int  seconds, minutes, hours, days;

    timeticks /= 1000;
    days = timeticks / (60L * 60 * 24);
    timeticks %= (60L * 60 * 24);

    hours = timeticks / (60L * 60);
    timeticks %= (60L * 60);

    minutes = timeticks / 60;
    seconds = timeticks % 60;

    if (days == 0){
    	if (hours == 0)
      		sprintf(buf, "%dmin %dsec", minutes, seconds);
      	else
      		sprintf(buf, "%dhour %dmin %dsec", hours, minutes, seconds);
    } else {
      sprintf(buf, "%ddays, %dhour %dmin %dsec", days, hours, minutes, seconds);
    }
    return buf;
}

int8 *portString(BYTE portnum, int8 *buf)
{
	BYTE PDesc[4]={'P'};
	BYTE tail = 0,sPDesc = 0;
	
	sPDesc = (_PORTDESC >> (2 *(portnum-1))) & 0x03;
	
	if ( sPDesc == 2)
		PDesc[0] ='U';

	tail = ( _PORTTAIL >> (2 *(portnum-1)) ) & 0x03;

#if ( defined (O_AXIS) )		//&&target
	sprintf(buf, "%s", "USB");
#else
	switch(_SPECOEM)
	{
		case 0:		// default: display PID
			sprintf(buf, "%d", portnum);
			break;
		case 1:		// Custom
		case 3:		// P1U1U2
		case 4:		// P1U2U3
			sprintf(buf, "%s%d", PDesc, tail);
			break;
		default:
			sprintf(buf, "%d", portnum);
		break;
	}
#endif

    return buf;
}


//*************************************************************************
//
//			       W3Offset ----+
//					            |
//                +------------+V--------+--------------------------------+
// code2 ---+---> |0 1 2 3....6| 7 8 9 A |                                |
//          |     +------------+---------+--------------------------------+
//			|     |                                                       |
//    (W3Offset)  |             +------------ AuthFileLen ----------------+
//          |     +---------------+----------------+----------------------+
// WebFlash +---> | WebVersion(1) | AuthFileLen(2) |AuthFileList.....\0\0 |
//                +---------------+----------------+-----------+----------+
// WebMsg ------> | WebMsgCount(1)| ErrMsgCount(1) | WebMsg1 \0|WebMsg2\0 |
//                +----+----------+-+------------+-+---------+-+--+-------+
//                | ...|ErrMsg1...\0|ErrMsg2...\0| ..........|0xFF|       |
//                +----+------------+------------+-----------+----+-------+
// PFS_DIR_LST--> |                                                       |
// (HttpInfo)     +-------------------------------------------------------+
//
//*************************************************************************

void HttpGetMessage(void)	//9/20/99 added
{
	BYTE *WebFlash;
//Jesse	DWORD W3Offset;
	BYTE WebMsgCount, ErrMsgCount;
	BYTE i, CurCount;

	/*
	if((W3Offset = *(DWORD *) MK_FP(ROM_BEGIN_SEGMENT,7)) == 0) return;

	WebFlash = (BYTE *)((BYTE huge *)MK_FP(ROM_BEGIN_SEGMENT,0)+W3Offset);
	*/
	WebFlash = MyData + MyDataSize[1];
//	WebFlash = MyData;


//mark 08/03/2005	WebLangVersion = 'X'; //5/12/2000

	// 12/29/99 Get Authorization File List ////////
	if(*(WebFlash++) != WEB_VERSION) {
		return;
	}

//mark 08/03/2005	WebLangVersion = *WebFlash; //4/26/2000
	WebFlash += 3;              //4/26/2000

	AuthFileList = (WebFlash + 2);	//skip AuthFileLen(2)
	WebFlash += *(WORD *)WebFlash;	//WebMsg address = WebFlash + AuthFileLen
	////////////////////////////////////////////////

	WebMsgCount = *(WebFlash++);
	ErrMsgCount = *WebFlash;
   
	if(WebMsgCount != MAX_WEB_MESSAGE || ErrMsgCount != MAX_ERR_MESSAGE) {
		return;
	}
    
	CurCount = 0;
	for(i = 0 ; i < WebMsgCount; i++) {
		WebMsg[CurCount++] = ++WebFlash;
		while(*WebFlash != '\0') WebFlash++;
	}
	CurCount = 0;
	for(i = 0 ; i < ErrMsgCount; i++) {
		CGI_Msg[CurCount++] = ++WebFlash;
		while(*WebFlash != '\0') WebFlash++;
	}
    if( *(++WebFlash) != 0xFF) return;//end mark error

	pDirList = (PFS_DIR_LIST*)(++WebFlash);
}

#endif HTTPD
