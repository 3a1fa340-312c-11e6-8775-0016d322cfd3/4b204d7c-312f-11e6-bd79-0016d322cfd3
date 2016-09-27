#ifndef _PSGLOBAL_H
#define _PSGLOBAL_H
#include <cyg/kernel/kapi.h>//ZOT==>
/*---------------------------------------------------------------------------*/
/*                             Standard "C" Types                            */
/*---------------------------------------------------------------------------*/
typedef unsigned char		BOOL;
typedef unsigned char		BYTE;
typedef unsigned short		WORD;
#ifndef __RTMP_TYPE_H__
typedef unsigned int		UINT;
#endif
typedef unsigned long		DWORD;

#ifndef  __RTMP_ECOS_TYPE_H_
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
#endif
typedef unsigned char		uint8;
typedef signed   char		int8;
typedef unsigned short		uint16;
typedef signed   short		int16;
typedef unsigned int        uint;
typedef unsigned int		uint32;
typedef signed   int		int32;
typedef unsigned long long	uint64;
typedef signed   long long	int64;

typedef unsigned char		uchar;
typedef unsigned int		time_t;

#ifndef __RTMP_TYPE_H__
typedef unsigned char        UINT8;
typedef unsigned short       UINT16;
typedef unsigned int         UINT32;
#endif

/*---------------------------------------------------------------------------*/
/*                             OS API                                        */
/*---------------------------------------------------------------------------*/
typedef cyg_mutex_t			spinlock_t;//ZOT==>
#define PACK				__attribute__ ((packed))		//615wu //eCos 
#define	MSPTICK				10								//615wu //Milliseconds per tick
#define TICKS_PER_SEC   	100								//615wu //ticks per second 
#define TICKS_PER_SECOND    TICKS_PER_SEC 
#define jiffies				(rdclock() * MSPTICK)			//615wu //in millisecond
#define ppause( x )			cyg_thread_delay( (x / MSPTICK)?(x / MSPTICK):1) 	//615wu //eCos, x in millisecond
#define DATA_ERROR			0xFFFF
#define _TOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )
#define HTONS(x)			(										\
							( ( uint16 )( (x) & 0x00ff ) << 8 ) |	\
							( ( uint16 )( (x) & 0xff00 ) >> 8 )		\
							)

#define HTONL(x)			(											\
							( ( uint32 )( (x) & 0x000000ff ) << 24 ) |	\
							( ( uint32 )( (x) & 0x0000ff00 ) << 8 ) |	\
							( ( uint32 )( (x) & 0x00ff0000 ) >> 8 ) |	\
							( ( uint32 )( (x) & 0xff000000 ) >> 24 )	\
							)			

#define WordSwap(x) 		HTONS((x))
#define DWordSwap(x)		HTONL((x))
#define ntohs(x)			HTONS((x))

#define	min(x,y)			((x)<(y)?(x):(y))
#define huge
#define down				spin_lock //ZOT==> #define down( s )			{ while( *s == 1 ) cyg_thread_yield(); *s = 1; }	//615wu
#define up					spin_unlock //ZOT==> #define up( s )				*s = 0	//615wu
#define spin_lock_init	__spin_lock_init //ZOT==> #define spin_lock_init( s )	*s = 0	//615wu
#define tolower(c)			( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )
#define TRUE 	            1
#define FALSE               0


extern void * kmalloc(size_t nb, int flag);
extern void * kaligned_alloc(size_t nb, size_t align);

extern void kfree(void *block, int flag);
extern void kaligned_free(void *block);

/*---------------------------------------------------------------------------*/
/*                             RWFLASH Define                                */
/*---------------------------------------------------------------------------*/
#define ReadFromEEPROM  	ReadFromFlash
#define WriteToEEPROM   	WriteToFlash
#define DataCopy 			memcpy

/*---------------------------------------------------------------------------*/
/*                             TCP/IP Port Number                            */
/*---------------------------------------------------------------------------*/
#define	IPPORT_ECHO	7	/* Echo data port */
#define	IPPORT_DISCARD	9	/* Discard data port */
#define	IPPORT_FTPD	20	/* FTP Data port */
#define	IPPORT_FTP	21	/* FTP Control port */
#define IPPORT_TELNET	23	/* Telnet port */
#define IPPORT_SMTP	25	/* Mail port */
#define	IPPORT_MTP	57	/* Secondary telnet protocol */
#define	IPPORT_FINGER	79	/* Finger port */
#define	IPPORT_TTYLINK	87	/* Chat port */
#define IPPORT_POP	109	/* pop2 port */
#define	IPPORT_NNTP	119	/* Netnews port */
#define	IPPORT_LOGIN	513	/* BSD rlogin port */
#define	IPPORT_TERM	5000	/* Serial interface server port */
//TCP
#define IPPORT_LPD		515	//LPD 	port
#define IPPORT_HTTP		80	//HTTPD port
#define IPPORT_IPPD		631	//IPPD	port
#define IPPORT_RAWTCP		9100 //RAWTCP port
//UDP
#define IPPORT_TFTPD    69 //TFTP port
#define IPPORT_NTPS_UTIL    0x5050 //NTUDP port

/*---------------------------------------------------------------------------*/
/*                                    ERROR                                  */
/*---------------------------------------------------------------------------*/
#define EILSEQ 			 4
#define EFAULT 			 14     /* Unknown error            */
#define EXDEV  			 22     /* Cross-device link        */
#define EFBIG  			 27     /* UNIX - not MSDOS         */
#define ENOSPC           28    	/* No space left on device */
#define EPIPE  			 32     /* UNIX - not MSDOS         */
#define EREMOTEIO		 50		/* UNIX - not MSDOS         */
#define EPROTO			 51		/* UNIX - not MSDOS         */
#define ENOSR  			 52		/* UNIX - not MSDOS         */
#define ETIMEDOUT		 55		/* UNIX - not MSDOS         */
#define EOVERFLOW        56		/* UNIX - not MSDOS         */
#define ECOMM 			 57		/* UNIX - not MSDOS         */
#define ENOLINK 		 60		/* UNIX - not MSDOS         */
#define EINPROGRESS      310   // Operation now in progress
// ipc/network software -- operational errors
#define ENETDOWN         350   // Network is down
#define ENETUNREACH      351   // Network is unreachable
#define ENETRESET        352   // Network dropped connection on reset
#define ECONNABORTED     353   // Software caused connection abort
#define ECONNRESET       354   // Connection reset by peer
#define ENOBUFS          355   // No buffer space available
#define EISCONN          356   // Socket is already connected
#define ENOTCONN         357   // Socket is not connected
#define ESHUTDOWN        358   // Can't send after socket shutdown
#define ETOOMANYREFS     359   // Too many references: can't splice
#define ETIMEDOUT        360   // Operation timed out
#define ECONNREFUSED     361   // Connection refused
#define EHOSTDOWN        364   // Host is down
#define EHOSTUNREACH     365   // No route to host


/*---------------------------------------------------------------------------*/
/*                             System Parameter                              */
/*---------------------------------------------------------------------------*/
//TCP/IP Application
#define BLOCKSIZE			(1460*7)
//For SMB & Netbeui
#define MY_NETBEUI_NAME			"ZOT-N63-"
//Config.h
#define MTHRESH         	512     /* Default memory threshold */
//For Netware PS
#define MAX_QUEUE         	16     //Maximun Queue per Printer
#define MAX_FS            	64     //Maximun File Server


/*---------------------------------------------------------------------------*/
/*                             EEPROM Utility                                */
/*---------------------------------------------------------------------------*/
#define _PrinterSpeed      		(EEPROM_Data.PrinterSpeed)
#define _BoxIPAddress      		(EEPROM_Data.BoxIPAddress)
#define _BoxSubNetMask     		(EEPROM_Data.SubNetMask)
#define _BoxGatewayAddress 		(EEPROM_Data.GetwayAddress)
#define _PrintServerName   		(EEPROM_Data.PrintServerName)
#define _BoxName           		(EEPROM_Data.BoxName)
	// Netware
#define _FileServerName(i) 		(EEPROM_Data.FileServerNames+OffsetOfFSName[i])
#define _SetupPassword     		(EEPROM_Data.Password)
#define _NDSTreeName       		(EEPROM_Data.NDSTreeName)
#define _NDSContext        		(EEPROM_Data.NDSContext)
#define _NovellPassword    		(EEPROM_Data.NovellPassword)
	// AppleTalk
#define _APPTLKEn				(EEPROM_Data.APPTLKEn)
#define _ATalkPortName			(EEPROM_Data.ATPortName)
	// Others
#define AlertActive    			(EEPROM_Data.AlertEnabled)
#define _IPLoadDef    			(EEPROM_Data.IPLoadDef)
#define _SPECOEM				(EEPROM_Data.SPECIAL_OEM)
	// Portface
#define _PORTDESC				(EEPROM_Data.PORTDESC)
#define _PORTTAIL				(EEPROM_Data.PORTTAIL)
	//LPR
#define _LPRQueueName(i)		(EEPROM_Data.LPRQueueName[i])	
	// Wireless
#define _WLZone					(EEPROM_Data.WLZone)
#define _WEPTYPE				(EEPROM_Data.WLWEPType)
#define _WLAuthenticationType	(EEPROM_Data.WLAuthType)
//#define _mib_WPA2Enabled		(EEPROM_Data.mib_WPA2Enabled)
//#define _mib_WPA_encr_algo		(EEPROM_Data.mib_WPA_encr_algo)
	// Rendezvous
#define _RENVEnable				(EEPROM_Data.RENVEnable)
#define _RENVServiceName		(EEPROM_Data.RENVServiceName)


/*---------------------------------------------------------------------------*/
/*                             Version Control                               */
/*---------------------------------------------------------------------------*/
#define MAJOR_VER_OFFSET		(0x20L)
#define MINOR_VER_OFFSET		(0x21L)
#define PS_MODEL_OFFSET			(0x22L)
#define RELEASE_VER_OFFSET		(0x23L)
#define BUILD_VER_OFFSET		(0x28L)
#define FIRMWARE_STRING_OFFSET	(0x2AL)

#define BIN_CHECKSUM_OFFSET		(0x00L)
#define BIN_ID_OFFSET			(0x04L)
#define BIN_MODEL_OFFSET		(0x07L)
#define BIN_LENGTH_OFFSET		(0x08L)
#define BIN_CODE_OFFSET			(0x0CL)

#define RAM_CODE_START_ADDRESS	CODE2_START_ADDRESS
/*
#define CODE1_MAJOR_VER			(*(uint8 *) (CODE1_START_ADDRESS+MAJOR_VER_OFFSET))
#define CODE1_MINOR_VER			(*(uint8 *) (CODE1_START_ADDRESS+MINOR_VER_OFFSET))
#define CODE1_BUILD_LOW_VER		(*(uint8 *) (CODE1_START_ADDRESS+BUILD_VER_OFFSET))
#define CODE1_BUILD_HI_VER		(*(uint8 *) (CODE1_START_ADDRESS+BUILD_VER_OFFSET+1))
#define CODE1_BUILD_VER			( (CODE1_BUILD_HI_VER << 8 )| CODE1_BUILD_LOW_VER)
#define CODE1_FIRMWARE_STRING	(*(uint16 *)(CODE1_START_ADDRESS+BUILD_VER_OFFSET))
#define CODE1_PS_MODEL		    (*(uint8 *) (CODE1_START_ADDRESS+PS_MODEL_OFFSET))

#define CODE2_FIRMWARE_STRING	((uint8 *)  (RAM_CODE_START_ADDRESS+FIRMWARE_STRING_OFFSET))

#define CURRENT_MAJOR_VER		(*(uint8 *) (RAM_CODE_START_ADDRESS+MAJOR_VER_OFFSET))
#define CURRENT_MINOR_VER		(*(uint8 *) (RAM_CODE_START_ADDRESS+MINOR_VER_OFFSET))
#define CURRENT_BUILD_LOW_VER	(*(uint8 *)(RAM_CODE_START_ADDRESS+BUILD_VER_OFFSET))
#define CURRENT_BUILD_HI_VER	(*(uint8 *)(RAM_CODE_START_ADDRESS+BUILD_VER_OFFSET+1))
#define CURRENT_BUILD_VER       ( (CURRENT_BUILD_HI_VER << 8 )| CURRENT_BUILD_LOW_VER)
#define CURRENT_RELEASE_VER		(*(uint16 *)(RAM_CODE_START_ADDRESS+RELEASE_VER_OFFSET))
//#define CURRENT_PS_MODEL		(*(uint8 *) (RAM_CODE_START_ADDRESS+PS_MODEL_OFFSET))
#define CURRENT_PS_MODEL		32    //ZOTIPS
*/

extern uint8	code1_major_ver;
extern uint8	code1_minor_ver;
extern uint8	code1_build_low_ver;
extern uint8	code1_build_hi_ver;
extern uint8	code1_ps_model;
extern uint8	code1_firmware_string[60];

#define CODE1_FIRMWARE_STRING	code1_firmware_string

#define CODE1_MAJOR_VER			code1_major_ver
#define CODE1_MINOR_VER			code1_minor_ver
#define CODE1_BUILD_LOW_VER		code1_build_low_ver
#define CODE1_BUILD_HI_VER		code1_build_hi_ver
#define CODE1_BUILD_VER			( (CODE1_BUILD_HI_VER << 8 )| CODE1_BUILD_LOW_VER)	
#define CODE1_PS_MODEL		    code1_ps_model

extern uint8	code2_major_ver;
extern uint8	code2_minor_ver;
extern uint8	code2_ps_model;
extern uint8	code2_build_low_ver;
extern uint8	code2_build_hi_ver;
extern uint8	code2_release_ver;
extern uint8	code2_firmware_string[60];

#define CODE2_FIRMWARE_STRING	code2_firmware_string

#define CURRENT_MAJOR_VER		code2_major_ver
#define CURRENT_MINOR_VER		code2_minor_ver
#define CURRENT_BUILD_LOW_VER	code2_build_low_ver
#define CURRENT_BUILD_HI_VER	code2_build_hi_ver
#define CURRENT_BUILD_VER		( (CURRENT_BUILD_HI_VER << 8 )| CURRENT_BUILD_LOW_VER)
#define CURRENT_RELEASE_VER		code2_release_ver
#define CURRENT_PS_MODEL		code2_ps_model

extern uint8	Loader_firmware_string[60];
#define LOADER_FIRMWARE_STRING	Loader_firmware_string
/*---------------------------------------------------------------------------*/
/*                             Extern Functions                               */
/*---------------------------------------------------------------------------*/
extern uint16 ServiceFSCount;       //How many FS need service
extern uint8  NovellConnectFlag;    //Novell PS connect or disconnect
extern BYTE MyPhysNodeAddress[];
extern uint16  OffsetOfFSName[MAX_FS];

#ifdef WEBADMIN	 //6/26/2000
extern uint32 IPXENIIRecv;
extern uint32 IPX8023Recv;
extern uint32 IPX8022Recv;
extern uint32 IPXSNAPRecv;
#endif /*WEBADMIN*/

/*---------------------------------------------------------------------------*/
/*                        SMB  Functions                                     */
/*---------------------------------------------------------------------------*/
#define pstrcpy(d,s) strcpy((d),(s))
#define pstrcat(d,s) strcat((d),(s))
#define fstrcpy(d,s) strcpy((d),(s))
#define fstrcat(d,s) strcat((d),(s))

//From STDIO.H of ARM9PS
#define		STREAM_BINARY		0
#define		STREAM_ASCII		1

#define TTRACE        \
    diag_printf("%s(%d)\n", __func__, __LINE__)

#endif  /*_PSGLOBAL_H*/
