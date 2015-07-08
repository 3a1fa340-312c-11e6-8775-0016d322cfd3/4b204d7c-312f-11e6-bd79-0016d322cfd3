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
#define WIRELESS_CARD
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
#define NOVELL_PS
#define NDS_PS

//Application
#define HTTPD
#define IPPD
#define LPD_TXT
#define ATALKD
#define SNMPD
#define TELNETD

#define RAWTCPD
#define UNIXUTIL_TFTP
//ZOT716u2 Not Yet #define RENDEZVOUS //Ron Add 11/24/04

#ifdef RENDEZVOUS 
#define LINKLOCAL_IP //Ron Add 12/07/04
#endif

//IEEE1284
#define DEF_IEEE1284

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
#define Print_ALERT				//check printer is "out of paper"
#define Mail_ALERT			//depend on Print_ALERT

//for printting job log
#define SUPPORT_JOB_LOG
//#define SUPPORT_PRN_COUNT

//#define DEBUG_AUSU

//#ifdef O_ZOT
//#if !defined(O_TPLINK)
#define SMBD
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
#define LED_MASK	   0x0001C000	//GPIO 14,15,16
#define Status_Lite	0	//GPIO 16; Yellow; Low active
#define Usb11_Lite	   1	//GPIO 15; Green; Low active
#define Usb20_Lite	   2	//GPIO 15; Green; Low active
#define Wireless_Lite  3	//GPIO 14; Yellow; Low active

/***********************************************************************/
/*                          WEB Definition                             */
/***********************************************************************/
#define	WEBPAGE_BASEADDR			0x30090000	//ZOT716u2
#define	WEBPAGE_OFFSET				0x100				

//--- SDROM Memory Mapping allocation -----------------------------------------------
#define PRINT_QUEUE_ADDRESS 		0x00550000	//576k	//ZOT716u2
#define UPGRADE_TEMP_ADDRESS		0x00550000	//576K	//ZOT716u2

//--- SDROM no cache Memory Mapping allocation -----------------------------------------------
#define kHW_RAM						31
//#define kFarHeap_hugebase			( 0x005E0000L )
#define kPktHeap_base               ( 0x005D0000L )
#define kFarHeap_base				( 0x00600000L )

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
#define FLASHSIZE                1//M bytes
#define FLASHBASE              	(0x30000000)	//ZOT716u2
#define SIZEOF_FLASH_SECTOR     16


//--- LOADER Address for System Variable -----------------------------
#define OFFSET_LOADER_ADDRESS	FLASHBASE
#define SECTOR_LOADER_CONFIG	0
#define NUM_OF_LOD_SECTOR		1


//--- EEPROM Address for System Variable -----------------------------
#define SYS_FLASH_ADDRESS 		(0x300FE000)	//ZOT716u2
#define SYS_FLASH_SIZE    		8		//8K
#define NUM_OF_EEP_SECTOR		2

#define OFFSET_EEPROM_CONFIG	0
#define SECTOR_EEPROM_CONFIG	0

//--- Deafult EEP Address for System Variable -----------------------------
#define DEFAULT_FLASH_ADDRESS	(0x300FC000)	//ZOT716u2
#define DEFAULT_FLASH_SIZE  	8		//8K
#define NUM_OF_DEFAULT_SECTOR	2

#define OFFSET_DEFAULT_CONFIG	0
#define SECTOR_DEFAULT_CONFIG	0

//--- QC0 Deafult EEP Address for System Variable -----------------------------
#define QC0_DEF_FLASH_ADDRESS		(0x300FA000)	//ZOT716u2
#define QC0_DEFAULT_FLASH_SIZE  	8		//8K
#define NUM_OF_QC0_DEFAULT_SECTOR	2

//--- LOADER Start Address -------------------------------------------
#define LOADER_START_ADDRESS	(0x30000000)	//ZOT716u2
#define NUM_OF_LOADER_SECTOR	16
#define LOADER_START_ADDRESS	(0x30000000)
#define LOADER_SIZE           	64
#define SECTOR_LOADER_CONFIG	0

//--- CODE1 Start Address -------------------------------------------
#define CODE1_START_ADDRESS		(0x30010000)	//ZOT716u2
#define NUM_OF_CODE1_SECTOR		32
#define CODE1_START_ADDRESS		(0x30010000)
#define CODE1_SIZE           	128
#define SECTOR_CODE1_CONFIG		0

//--- CODE2 Start Address ------------------------------------------------
#define PROGRAM_CODE2_KSIZE		4
#define CODE2_START_ADDRESS		(0x30030000)	//ZOT716u2
#define SECTOR_CODE2_CONFIG		0
#define NUM_OF_CODE2_SECTOR		176//144	//128	eason 20100608
#define CODE2_MAX_SIZE        	704//576	//512	eason 20100608

//--- WEB Start Address ------------------------------------------------
#define PROGRAM_WEB_KSIZE		4
#define WEB_START_ADDRESS		(0x30090000)	//ZOT716u2
#define SECTOR_WEB_CONFIG		0
#define NUM_OF_WEB_SECTOR		6
#define WEB_MAX_SIZE        	384

#endif  /*_PSTARGET_H*/
