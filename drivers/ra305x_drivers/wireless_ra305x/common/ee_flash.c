/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ee_flash.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start);


static USHORT EE_FLASH_ID_LIST[]={
#ifdef RT2880
	0x2880, 
#endif /* RT2880 */
#ifdef RT305x
	0x3052,
	0x3051,
	0x3050,
	0x3350,
#endif /* RT305x */
#ifdef RT3352
	0x3352,
#endif /* RT3352 */
#ifdef RT5350
	0x5350,
#endif /* RT5350 */


#ifdef RT6352
	0x6352,
	0x7620,
#endif /* RT6352 */

};

#define EE_FLASH_ID_NUM  (sizeof(EE_FLASH_ID_LIST) / sizeof(USHORT))



/*******************************************************************************
  *
  *	Flash-based EEPROM read/write procedures.
  *		some chips use the flash memory instead of internal EEPROM to save the 
  *		calibration info, we need these functions to do the read/write.
  *
  ******************************************************************************/
int rtmp_ee_flash_read(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT Offset,
	OUT USHORT *pValue)
{	
	if (!pAd->chipCap.ee_inited)
	{
		*pValue = 0xffff;
	}
	else
	{
		memcpy(pValue, pAd->eebuf + Offset, 2);
	}
	return (*pValue);
}


int rtmp_ee_flash_write(PRTMP_ADAPTER pAd, USHORT Offset, USHORT Data)
{
	if (pAd->chipCap.ee_inited)
	{
		memcpy(pAd->eebuf + Offset, &Data, 2);
		/*rt_nv_commit();*/
		/*rt_cfg_commit();*/
#ifdef MULTIPLE_CARD_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("rtmp_ee_flash_write:pAd->MC_RowID = %d\n", pAd->MC_RowID));
		DBGPRINT(RT_DEBUG_TRACE, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
		if ((pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x48000) || (pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x40000))
			RtmpFlashWrite(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
		RtmpFlashWrite(pAd->eebuf, RF_OFFSET, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */
	}
	return 0;
}


VOID rtmp_ee_flash_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{	
	if (!pAd->chipCap.ee_inited)
		return;
		
	memcpy(Data, pAd->eebuf, EEPROM_SIZE);
}


VOID rtmp_ee_flash_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	if (!pAd->chipCap.ee_inited)
		return;
	memcpy(pAd->eebuf, Data, EEPROM_SIZE);
#ifdef MULTIPLE_CARD_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("rtmp_ee_flash_write_all:pAd->MC_RowID = %d\n", pAd->MC_RowID));
	DBGPRINT(RT_DEBUG_TRACE, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
	if ((pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x48000) || (pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x40000))
		RtmpFlashWrite(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
	RtmpFlashWrite(pAd->eebuf, RF_OFFSET, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */
}


static NDIS_STATUS rtmp_ee_flash_reset(
	IN RTMP_ADAPTER *pAd, 
	IN PUCHAR start)
{
	PUCHAR				src;
	RTMP_OS_FS_INFO		osFsInfo;
	RTMP_OS_FD			srcf;
	INT 					retval;

	src = EEPROM_DEFAULT_FILE_PATH;

	RtmpOSFSInfoChange(&osFsInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> Error opening file %s\n", src));
			return NDIS_STATUS_FAILURE;
		}
		else 
		{
			/* The object must have a read method*/
			NdisZeroMemory(start, EEPROM_SIZE);
			
			retval = RtmpOSFileRead(srcf, start, EEPROM_SIZE);
			if (retval < 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> Read %s error %d\n", src, -retval));
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("--> rtmp_ee_flash_reset copy %s to eeprom buffer\n", src));
			}

			retval = RtmpOSFileClose(srcf);
			if (retval)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
			}
		}
	}

	RtmpOSFSInfoChange(&osFsInfo, FALSE);

	return NDIS_STATUS_SUCCESS;
}



static BOOLEAN  validFlashEepromID(RTMP_ADAPTER *pAd)
{
	USHORT eeFlashId;
	int listIdx;
	
	rtmp_ee_flash_read(pAd, 0, &eeFlashId);

	for(listIdx =0 ; listIdx < EE_FLASH_ID_NUM; listIdx++)
	{
		if (eeFlashId == EE_FLASH_ID_LIST[listIdx])
			return TRUE;
	}
	return FALSE;
}


static NDIS_STATUS rtmp_ee_flash_init(PRTMP_ADAPTER pAd, PUCHAR start)
{
	pAd->chipCap.ee_inited = 1;

	if (validFlashEepromID(pAd) == FALSE)
	{
		if (rtmp_ee_flash_reset(pAd, start) != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rtmp_ee_init(): rtmp_ee_flash_init() failed\n"));
			return NDIS_STATUS_FAILURE;
		}

		/* Random number for the last bytes of MAC address*/
		{
			USHORT  Addr45;
			
			rtmp_ee_flash_read(pAd, 0x08, &Addr45);
			Addr45 = Addr45 & 0xff;
			Addr45 = Addr45 | (RandomByte(pAd)&0xf8) << 8;
			
			rtmp_ee_flash_write(pAd, 0x08, Addr45);
			DBGPRINT(RT_DEBUG_ERROR, ("The EEPROM in Flash is wrong, use default\n"));
		}

		if (validFlashEepromID(pAd) == FALSE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rtmp_ee_flash_init(): invalid eeprom\n"));
			return NDIS_STATUS_FAILURE;
		}
	}
	
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_nv_init(PRTMP_ADAPTER pAd)
{
#ifdef MULTIPLE_CARD_SUPPORT
	UCHAR *eepromBuf;
#endif /* MULTIPLE_CARD_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("--> rtmp_nv_init\n"));
	if (pAd->chipCap.eebuf == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pAd->chipCap.eebuf == NULL!!!\n"));
		return NDIS_STATUS_FAILURE;
	}
	
/*	ASSERT((pAd->eebuf == NULL)); */
	pAd->eebuf = pAd->chipCap.eebuf;

#ifdef __ECOS
	int stat;
	if ((stat = cyg_flash_init(NULL)) != 0) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("FLASH: driver init failed: %s\n", cyg_flash_errmsg(stat)));
        	return -1;
   	}

	{
		UCHAR *eepromBuf;

		if (os_alloc_mem(pAd, &eepromBuf, EEPROM_SIZE) == NDIS_STATUS_SUCCESS)
		{
			NdisZeroMemory(eepromBuf, EEPROM_SIZE);
			NdisMoveMemory(eepromBuf, pAd->eebuf, EEPROM_SIZE);
			RtmpFlashRead(pAd->eebuf, RF_OFFSET, EEPROM_SIZE);
			pAd->chipCap.ee_inited = 1;
			if (validFlashEepromID(pAd) == FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("The EEPROM in Flash are wrong, use default\n"));
				NdisMoveMemory(pAd->eebuf, eepromBuf, EEPROM_SIZE);
			}
			os_free_mem(pAd, eepromBuf);
		}
	}
	return NDIS_STATUS_SUCCESS;
#else

#ifdef MULTIPLE_CARD_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("rtmp_nv_init:pAd->MC_RowID = %d\n", pAd->MC_RowID));
	os_alloc_mem(pAd, &eepromBuf, EEPROM_SIZE);
	if (eepromBuf)
	{	
		pAd->eebuf = eepromBuf;
		NdisMoveMemory(pAd->eebuf, pAd->chipCap.eebuf, EEPROM_SIZE);
		}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("rtmp_nv_init:Alloc memory for pAd->MC_RowID[%d] failed! used default one!\n", pAd->MC_RowID));
	}
	DBGPRINT(RT_DEBUG_OFF, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
	RtmpFlashRead(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
	RtmpFlashRead(pAd->eebuf, RF_OFFSET, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */

	return rtmp_ee_flash_init(pAd, pAd->eebuf);
#endif /* __ECOS */
}
