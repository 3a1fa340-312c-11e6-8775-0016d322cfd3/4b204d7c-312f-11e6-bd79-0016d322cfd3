/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_rbs.h

    Abstract:
 	Ralink SoC Internal Bus related definitions and data dtructures

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RTMP_RBUS_H__
#define __RTMP_RBUS_H__

#include "iface/rtmp_inf_pcirbs.h"

/*************************************************************************
  *
  *	Device hardware/ Interface related definitions.
  *
  ************************************************************************/

#define RTMP_MAC_IRQ_NUM		4


/*************************************************************************
  *
  *	EEPROM Related definitions
  *
  ************************************************************************/
#ifdef RT305x
#if defined(CONFIG_RALINK_RT3050_1T1R)
#if defined(CONFIG_RALINK_RT3350)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3350_AP_1T1R_V1_0.bin"
#else
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3050_AP_1T1R_V1_0.bin"
#endif /* CONFIG_RALINK_RT3350 */
#endif /* CONFIG_RALINK_RT3050_1T1R */

#if defined(CONFIG_RALINK_RT3051_1T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3051_AP_1T2R_V1_0.bin"
#endif /* CONFIG_RALINK_RT3051_1T2R */

#if defined(CONFIG_RALINK_RT3052_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3052_AP_2T2R_V1_1.bin"
#endif /* CONFIG_RALINK_RT3052_2T2R */
#endif /* RT305x */


#ifdef RT3352
#if defined(CONFIG_RALINK_RT3352_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3352_AP_2T2R-4L_V12.BIN"
#endif /* CONFIG_RALINK_RT3352_2T2R */
#endif /* RT3352 */

#ifdef RT5350
#if defined(CONFIG_RALINK_RT5350_1T1R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT5350_AP_1T1R_V1_0.bin"
#endif // CONFIG_RALINK_RT5350_1T1R //
#endif /* RT5350 */




#if defined (CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif


#define RALINK_SYSCTL_BASE			0xb0000000


#ifdef LINUX
/*************************************************************************
  *
  *	Device Tx/Rx related definitions.
  *
  ************************************************************************/
#ifdef DFS_SUPPORT
/* TODO: Check these functions. */
#ifdef RTMP_RBUS_SUPPORT
extern void unregister_tmr_service(void);
extern void request_tmr_service(int, void *, void *);
#endif /* RTMP_RBUS_SUPPORT */

#endif /* DFS_SUPPORT */

#endif /* LINUX */

#endif /* __RTMP_RBUS_H__ */

