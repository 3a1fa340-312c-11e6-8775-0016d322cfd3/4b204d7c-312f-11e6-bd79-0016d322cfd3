// Target setup according to H/W

// Now !! ONLY ONE Target :: 615WU used

//******************************************************************************//
//*****************************-  615WU.h  -************************************//
//******************************************************************************//
#ifndef _PSTARGET_H
#define _PSTARGET_H

//------------ OEM ---------------------------------
// #define O_ELEC
// #define O_ZYXEL
// #define O_ZOTCH
// #define O_INTELB
// #define O_LEVELO
// #define O_NETCOR
// #define O_PCI
// #define O_CONRAD
// #define O_ASSMANN

//#define PreTest			// PreTest

#define WIRELESS_SETTING_ONE_PAGE

#define WPA_PSK_TKIP

#ifdef USE_WIRELESS_LIBS
#define WIRELESS_CARD
#else
#undef WIRELESS_CARD
#endif

#define PACK_DATA_EXT			//pack data extern
#define WLWEP128_FOURKEYS
#define ISL80211G_EXTRATE

//#if defined(O_PCI) || defined(O_CONRAD) || defined(O_ASSMANN)
#if  defined(O_PCI) || defined(O_CONRAD)
#define LOADING_MORE_WIRELESS_ENCRYPTION_INFORMATION	// Lance and George added this at build0009 of 716U2W on July 6, 2012.
#endif	//  defined(O_PCI) || defined(O_CONRAD) || defined(O_ASSMANN)

#if defined(O_CONRAD)
#define WPSBUTTON_LEDFLASH_FLICK						// Lance and George added this at build0010 of 716U2W on July 25, 2012.
#endif	// defined(O_CONRAD)

//--- Print Port number -----------------------------------------------
#define NUM_OF_1284_PORT	0
#define NUM_OF_PRN_PORT		1	//615wu //USB Port
#define NUM_PORTS			1	//615wu	// number of USB ports 

/*---------------------------------------------------------------------------*/
/*                             Function support                              */
/*---------------------------------------------------------------------------*/
//Basic
#define WINDOWS_PS
#define NTUDP
#define IPX
#define NETBEUI

#ifdef USE_PS_LIBS
#define NOVELL_PS
#define NDS_PS
#endif /* USE_PS_LIBS */

//Application
#define HTTPD
#define RAWTCPD

#ifdef USE_PS_LIBS
#define IPPD
#define LPD_TXT
#define ATALKD
#define SNMPD
#define UNIXUTIL_TFTP
#endif /* USE_PS_LIBS */

#ifdef USE_NETAPP_LIBS
#define TELNETD
#endif /* USE_NETAPP_LIBS */

#if defined(NDWP2020) && defined(USE_PS_LIBS)
#define RENDEZVOUS //Ron Add 11/24/04
#endif

#ifdef RENDEZVOUS 
#define LINKLOCAL_IP //Ron Add 12/07/04
#endif

//IEEE1284
#ifdef USE_PS_LIBS
#define DEF_IEEE1284
#endif

#define FULL_FIRMWARE_VERSION	// Version + Bulid Number

#if defined(O_ZOTCH) || defined(O_PCI) || defined(O_CONRAD) || defined(O_TPLINK) || defined(O_TPLINM) || defined(O_TPLINS) || defined(O_INTELB) || defined(O_LS)
#define IP_LOAD_DEFAULT
#endif	// defined(O_ZOTCH) || defined(O_PCI) || defined(O_CONRAD) || defined(O_TPLINK) || defined(O_TPLINM) || defined(O_TPLINS) || defined(O_INTELB) || defined(O_LS)

//LPR rename
//#define LPR_Q_RENAME

#if defined(O_ELEC)				// Mark this line if you want to build ELECOM's CODE1
#define PACK_DATA_EXT			// pack data extern
#define ADVANCED_EEP			// Advanced EEP (PS: EEPSize > 1408)
#define MAC_FILTERING			// MAC Filtering
#endif	// defined(O_ELEC)		// Mark this line if you want to build ELECOM's CODE1

#if defined(O_TPLINK)
#define CFG_EXPIMP				// Export current configuration as file
								// Import previous configuration from file
#endif	// defined(O_TPLINK)

//for USB State LED
#define USB_LED

//for send mail
#ifdef USE_PS_LIBS
#define Print_ALERT				//check printer is "out of paper"
#define Mail_ALERT			//depend on Print_ALERT

//for printting job log
#define SUPPORT_JOB_LOG

#endif /* USE_PS_LIBS */

//#define SUPPORT_PRN_COUNT

//#define DEBUG_AUSU

//#ifdef O_ZOT
//#if !defined(O_TPLINK)
#ifdef USE_NETAPP_LIBS
#define SMBD
#if defined(NDWP2020) && (defined(O_TPLINK) || defined(O_TPLINA))
#undef SMBD
#endif
#endif /* USE_NETAPP_LIBS */

//#endif	// !defined(O_TPLINK)

//for HP webadmin
#define WEBADMIN
//#endif

#ifdef O_AXIS

//Printting test pige or reset for AXIS
#define NORESETBUTTON

#endif

#define DO_STATUS_PRINT

//#define L2_ZERO_CPY
//#define USB_ZERO_CPY


/***********************************************************************/
/*                          LED Definition                             */
/***********************************************************************/
#if 0	//ZOT716u2
#define Status_Lite			    1 // GPIO1 output

#ifdef USB_LED
#define Usb_Lite			    6 // GPIO6 output
#define Lan_Lite_100			12 // GPIO6 output
#define Lan_Lite_10			    14 // GPIO6 output
#else 
#define Lan_Lite			    6 // GPIO6 output
#endif
#endif	//ZOT716u2
//ZOT716u2 LED
#define LED_MASK	   0x0001E000	//GPIO 13,14,15,16
#define Status_Lite	   0	//GPIO 16; Yellow; Low active
#define Usb11_Lite	   1	//GPIO 15; Green; Low active
#define Usb20_Lite	   2	//GPIO 15; Green; Low active
#define Wireless_Lite  3	//GPIO 14; Yellow; Low active

/***********************************************************************/
/*                          WEB Definition                             */
/***********************************************************************/
#define	WEBPAGE_BASEADDR			0x30090000	//ZOT716u2
#define	WEBPAGE_OFFSET				0x100				

//--- SDROM Memory Mapping allocation -----------------------------------------------
#if defined(ARCH_ARM)
#define PRINT_QUEUE_ADDRESS 		0x00530000	//576k	//ZOT716u2
#define UPGRADE_TEMP_ADDRESS		0x00530000	//576K	//ZOT716u2
#endif /* ARCH_ARM */
#if defined (ARCH_MIPS)
#define PRINT_QUEUE_ADDRESS 		0x80E00000	//576k	//ZOT716u2
#define UPGRADE_TEMP_ADDRESS		0x80E00000	//576K	//ZOT716u2
#endif /* ARCH_MIPS */

//--- SDROM no cache Memory Mapping allocation -----------------------------------------------
#if defined(ARCH_ARM)
#define kHW_RAM						31
//#define kFarHeap_hugebase			( 0x005E0000L )
#define kPktHeap_base               ( 0x005D0000L )
#define kFarHeap_base				( 0x00600000L )
#endif /* ARCH_ARM */

#if defined(ARCH_MIPS)
#define kHW_RAM                     63	
extern unsigned char kFarHeap_base[];
#endif /* ARCH_MIPS */

//--- Print Queue Length per Print Port -----------------------------------------------
#ifndef USB_ZERO_CPY
#define PRNQUEUELEN		10			//ZOT716u2
#else
#define PRNQUEUELEN		6
#endif

//--- MAC memory ----------------------------------
#define MAC_TX_BUF_POOL		0x007E8000
#define MAC_RX_BUF_POOL		0x007F0000
#define MAC_TX_DEC_POOL		0x007E7D00
#define MAC_RX_DEC_POOL		0x007E7E00

//--- USB memory ----------------------------------
#define USB_POOL_BASE		0x007E0000
#define USB_CONTROL_DATA	0x007E4800
#define USBPRN_WRITE_BUFFER 0x007E2800
#define USBPRN_READ_BUFFER	0x007E4808


//--- Number of USB Printer Device ----------------------------------
#define NUM_OF_USB_PRN_DEVICE	6

//--- flash for System Variable -----------------------------
#define ALLFLASHADDROFF        	0
#define LOADERADDROFF			0x00000000
#define WEBOFF					0x00090000

#ifdef ARCH_ARM
#define QC0ADDROFF              0x000FA000
#define DEFAULTADDROFF         	0x000FC000
#define EEPROMADDROFF         	0x000FE000
#define MAXFLASHOFF             0x00100000
#define FLASHSIZE                1//M bytes
#define FLASHBASE              	(0x30000000)	//ZOT716u2
#define FLASHBASE_2             (0x30000000)
#define LOADER_SIZE           	64
#define CODE1ADDROFF           	0x00010000
#define CODE1_SIZE              128	
#define CODE2ADDROFF           	0x00030000
#define CODE2_SIZE              768

#define SIZEOF_FLASH_SECTOR     16
#endif /* ARCH_ARM */

#ifdef ARCH_MIPS
#define QC0ADDROFF              0x001FA000
#define DEFAULTADDROFF         	0x001FC000
#define EEPROMADDROFF         	0x001FE000
#define MAXFLASHOFF             0x00200000
#define FLASHSIZE               2//M bytes
#define FLASHBASE               (0xBC000000)
#define FLASHBASE_2             (0xBC100000)

#define LOADER_SIZE           	192
#define CODE1ADDROFF           	0x00030000
#define CODE1_SIZE              128	
#define CODE2ADDROFF           	0x00050000
#define CODE2_SIZE              1024

#endif /* ARCH_MIPS */


//--- LOADER Address for System Variable -----------------------------
#define OFFSET_LOADER_ADDRESS	FLASHBASE
#define SECTOR_LOADER_CONFIG	0
#ifdef ARCH_ARM
#define NUM_OF_LOADER_SECTOR	1
#endif /* ARCH_ARM */
#ifdef ARCH_MIPS
#define NUM_OF_LOADER_SECTOR    (LOADER_SIZE / 4)
#endif /* ARCH_MIPS */


//--- EEPROM Address for System Variable -----------------------------
#define SYS_FLASH_ADDRESS 		FLASHBASE_2 + EEPROMADDROFF
#define SYS_FLASH_SIZE    		8		//8K
#define NUM_OF_EEP_SECTOR		2

#define OFFSET_EEPROM_CONFIG	0
#define SECTOR_EEPROM_CONFIG	0

//--- Deafult EEP Address for System Variable -----------------------------
#define DEFAULT_FLASH_ADDRESS	FLASHBASE_2 + DEFAULTADDROFF
#define DEFAULT_FLASH_SIZE  	8		//8K
#define NUM_OF_DEFAULT_SECTOR	2

#define OFFSET_DEFAULT_CONFIG	0
#define SECTOR_DEFAULT_CONFIG	0

//--- QC0 Deafult EEP Address for System Variable -----------------------------
#define QC0_DEF_FLASH_ADDRESS   FLASHBASE_2 + QC0ADDROFF
#define QC0_DEFAULT_FLASH_SIZE  	8		//8K
#define NUM_OF_QC0_DEFAULT_SECTOR	2

//--- LOADER Start Address -------------------------------------------
#define LOADER_START_ADDRESS	FLASHBASE
#define NUM_OF_LOADER_SECTOR	16
#define SECTOR_LOADER_CONFIG	0

//--- CODE1 Start Address -------------------------------------------
#define CODE1_START_ADDRESS		FLASHBASE + CODE1ADDROFF
#define NUM_OF_CODE1_SECTOR	    32	
#define SECTOR_CODE1_CONFIG		0

//--- CODE2 Start Address ------------------------------------------------
#define PROGRAM_CODE2_KSIZE		4
#define CODE2_START_ADDRESS		FLASHBASE + CODE2ADDROFF
#define SECTOR_CODE2_CONFIG		0
#define NUM_OF_CODE2_SECTOR		(CODE2_SIZE / PROGRAM_CODE2_KSIZE) //192//176//144
#define CODE2_MAX_SIZE        	CODE2_SIZE

//--- WEB Start Address ------------------------------------------------
#define PROGRAM_WEB_KSIZE		4
#define WEB_START_ADDRESS		(0x30090000)	//ZOT716u2
#define SECTOR_WEB_CONFIG		0
#define NUM_OF_WEB_SECTOR		6
#define WEB_MAX_SIZE        	384

#endif  /*_PSTARGET_H*/
