
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "prnqueue.h"
#include "star_powermgt.h"	//ZOT716u2

#if defined(ARCH_MIPS)
#define IH_NMLEN    32
typedef struct image_header {
    uint32 ih_magic;   /* Image Header Magic Number */
    uint32 ih_hcrc;    /* Image Header CRC Checksum */
    uint32 ih_time;    /* Image Creation Timestamp  */
    uint32 ih_size;    /* Image Data Size       */
    uint32 ih_load;    /* Data   Load  Address      */
    uint32 ih_ep;      /* Entry Point Address       */
    uint32 ih_dcrc;    /* Image Data CRC Checksum   */
    uint8  ih_os;      /* Operating System      */
    uint8  ih_arch;    /* CPU architecture      */
    uint8  ih_type;    /* Image Type            */
    uint8  ih_comp;    /* Compression Type      */
    uint8  ih_name[IH_NMLEN];  /* Image Name        */
} uboot_image_header_t;
#endif /* ARCH_MIPS */

typedef struct _header_ver{
	
	DWORD crc32_chksum; //crc32 checksum
	char  mark[3]; 	//"XGZ"
	BYTE  bintype; 	//0xB0: All Flash, 0xB1: Code1, 0xB2: Code2, 0xB3: EEPROM, 0xB4: LOADER
	DWORD file_len; //Total Image file lentch
	BYTE  next;
	BYTE  reserved1[19];
	BYTE  MajorVer;
	BYTE  MinorVer;
	BYTE  model; 	//Target model	
	BYTE  ReleaseVer;
	DWORD RDVersion;
	WORD  BuildVer;	
	BYTE  data[60];	
	BYTE  reserved2[154];
} PACK HEADER_VER, *pHEADER_VER;
/**************************************************************/

#define ALLFLASH_TYPE	0xB0   
#define CODE1_TYPE		0xB1   
#define CODE2_TYPE		0xB2   
#define EEPROM_TYPE		0xB3   
#define LOADER_TYPE		0xB4

#define ALLFLASHMARK    0xB0
#define CODE1MARK       0xB1
#define CODE2MARK       0xB2
#define EEPROMMARK      0xB3
#define LOADERMARK      0xB4


//temp extern int flashWriteWord (int offset, unsigned short dat);
//temp extern unsigned int crc32(unsigned char *address, unsigned int size, unsigned int crc); //Ron Add
//temp extern void WatchdogDisable (void);
//temp extern void WatchdogEnable (void);
//temp extern void EraseSectors(int sblk, int nblks);

//from psmain.c
extern uint32 NGET32( uint8 *pSrc );
extern BYTE  PSMode;
extern uint32 get32(uint8 *cp);

// #define PROGRAM_MAX_RETRY_TIMES			3
#define PROGRAM_MAX_RETRY_TIMES			1

#if defined(ARCH_MIPS)
void dump_uboot_image_header(uboot_image_header_t *hdr)
{
    diag_printf("u_boot image header --------------------------------------\n");
    diag_printf("ih_magic:%4x\n", ntohl(hdr->ih_magic));
    diag_printf("ih_hcrc:%4x\n", ntohl(hdr->ih_hcrc));
    diag_printf("ih_time:%4x\n", ntohl(hdr->ih_time));
    diag_printf("ih_size:%d\n", ntohl(hdr->ih_size));
    diag_printf("ih_load:%4x\n", ntohl(hdr->ih_load));
    diag_printf("ih_ep:%4x\n", ntohl(hdr->ih_ep));
    diag_printf("hi_dcrc:%4x\n", ntohl(hdr->ih_dcrc));
    diag_printf("hi_os:%x\n", hdr->ih_os);
    diag_printf("hi_arch:%x\n", hdr->ih_arch);
    diag_printf("hi_type:%x\n", hdr->ih_type);
    diag_printf("hi_comp:%x\n", hdr->ih_comp);
    diag_printf("ih_name:%s\n", hdr->ih_name);
    diag_printf("----------------------------------------------------------\n");
}
#endif /* ARCH_MIPS */

#ifdef ARCH_ARM
int vEraseWeb()
{
	return vEraseFlash( WEB_START_ADDRESS, NUM_OF_WEB_SECTOR );
}
#endif /* ARCH_ARM */

int vErase_QC0_Default()
{
	return vEraseFlash( QC0_DEF_FLASH_ADDRESS, NUM_OF_QC0_DEFAULT_SECTOR );
}

int vEraseDefault()
{
	return vEraseFlash( DEFAULT_FLASH_ADDRESS, NUM_OF_DEFAULT_SECTOR );
}

int vEraseEEP()
{
	return vEraseFlash( SYS_FLASH_ADDRESS, NUM_OF_EEP_SECTOR );
}

int vEraseCode2()
{
	return vEraseFlash( CODE2_START_ADDRESS, NUM_OF_CODE2_SECTOR );
}

int vEraseCode1()
{
	return vEraseFlash( CODE1_START_ADDRESS, NUM_OF_CODE1_SECTOR );
}

int vEraseLoader()
{
	return vEraseFlash( LOADER_START_ADDRESS, NUM_OF_LOADER_SECTOR );
}

int vAllocCode2Memory()
{
	uint8 PrintStatus;
	int i;

#ifdef USE_PS_LIBS
	// check print port is busy or not
	PrintStatus = ReadPrintStatus();
	for( i = 0; i < NUM_OF_1284_PORT; i++ )
	{
		if( ( PrintStatus >> ( i * 2 ) ) & 0x03 == 0x03 )
			return 0;
	}

	for( i = 0; i < NUM_OF_PRN_PORT; i++ )
	{
		if( PrnGetPrinterStatus(i) != PrnNoUsed )
			return 0;
	}
	for( i = 0; i < NUM_OF_PRN_PORT; i++ )
		PrnSetUpgradeInUse(i);
#endif

	memset( UPGRADE_TEMP_ADDRESS, 0, PROGRAM_CODE2_KSIZE * 1024 );
	return 1;
}

int vReleaseCode2Memory()
{
#ifdef USE_PS_LIBS
	int i;
	for( i = 0; i < NUM_OF_PRN_PORT; i++ ){
		if (PrnGetPrinterStatus(i) == UpgradeUsed)
		PrnSetNoUse(i);
	}
#endif
}

int ReadFromFlash(EEPROM *RomData)
{
	uint32 CheckSum1, CheckSum2;
	int rc;

//	memcpy( RomData, SYS_FLASH_ADDRESS, sizeof(EEPROM) );
//ZOT716u2
	read_flash(SYS_FLASH_ADDRESS, RomData, sizeof(EEPROM));

	memcpy( &CheckSum1, RomData->CheckSum2, 4 );

	CheckSum2 = crc32( RomData, sizeof(EEPROM)-4, 0xFFFFFFFFL );

	rc = !( CheckSum1 == CheckSum2 );

	if(strcmp(RomData->ZOT_Mark,"ZOT")) rc = 1; //Error !

	return rc;
}

int ReadFromDefault(EEPROM *RomData)
{
	uint32 CheckSum1, CheckSum2;
	int rc;

//	memcpy( RomData, DEFAULT_FLASH_ADDRESS, sizeof(EEPROM) );
//ZOT716u2
	read_flash(DEFAULT_FLASH_ADDRESS, RomData, sizeof(EEPROM));
	
	memcpy( &CheckSum1, RomData->CheckSum2, 4 );

	CheckSum2 = crc32( RomData, sizeof(EEPROM)-4, 0xFFFFFFFFL );

	rc = !( CheckSum1 == CheckSum2 );

	if(strcmp(RomData->ZOT_Mark,"ZOT")) rc = 1; //Error !

	return rc;
}

int ReadFromQC0_Default(EEPROM *RomData)
{
	uint32 CheckSum1, CheckSum2;
	int rc;

	read_flash(QC0_DEF_FLASH_ADDRESS, RomData, sizeof(EEPROM));
	
	memcpy( &CheckSum1, RomData->CheckSum2, 4 );

	CheckSum2 = crc32( RomData, sizeof(EEPROM)-4, 0xFFFFFFFFL );

	rc = !( CheckSum1 == CheckSum2 );

	if(strcmp(RomData->ZOT_Mark,"ZOT")) rc = 1; //Error !

	return rc;
}

int WriteToFlash(EEPROM *RomData)
{
	uint32 CheckSum;
	int rc = 1;

	cli();
	cyg_scheduler_lock();

	if(RomData != &EEPROM_Data)
		memcpy(&EEPROM_Data,RomData,sizeof(EEPROM));

	if( strcmp(RomData->ZOT_Mark,"ZOT") ) 
	{
		cyg_scheduler_unlock();
		sti();
		return 1;  // EEPROM Data is wrong
	}
	
	EEPROM_Data.EEPROMWriteCount++;	// Write Count

	// current setting CRC
	memset( EEPROM_Data.CheckSum2, 0, 4 );
	CheckSum = crc32( &EEPROM_Data, sizeof(EEPROM)-4, 0xFFFFFFFFL );
	memcpy( EEPROM_Data.CheckSum2, &CheckSum, 4 );

	rc = vEraseEEP();
	
	if(rc == 0){
		/* Now write the EEPROM_Data */
		rc = vProgramFlash( SYS_FLASH_ADDRESS, &EEPROM_Data, sizeof(EEPROM) );
	}

	cyg_scheduler_unlock();
	sti();
		
	return rc;
}


//int ResetToDefalutFlash(void)
int ResetToDefalutFlash(int tmpip, int tmpkey, int nouse)
{
	BYTE  BoxIPAddress[4];
	BYTE  SubNetMask[4];
	BYTE  GetwayAddress[4];
	DWORD EEPROMWriteCount;

	//Save IP
	memcpy(BoxIPAddress, EEPROM_Data.BoxIPAddress, 4);
	memcpy(SubNetMask,   EEPROM_Data.SubNetMask,   4);
	memcpy(GetwayAddress,EEPROM_Data.GetwayAddress,4);

	//Save WriteCount
	EEPROMWriteCount = NGET32(&EEPROM_Data.EEPROMWriteCount);

	memcpy( &EEPROM_Data, &DEFAULT_Data, sizeof(EEPROM) );

#if (!defined(O_PCI) && !defined(O_CONRAD) && !defined(O_ELEC) && !defined(O_PLANET) && !defined(O_LEVELO))
	if (tmpip){
	memcpy(EEPROM_Data.BoxIPAddress, BoxIPAddress, 4);
	memcpy(EEPROM_Data.SubNetMask,   SubNetMask,   4);
	memcpy(EEPROM_Data.GetwayAddress,GetwayAddress,4);

	EEPROM_Data.PrintServerMode =
	    (EEPROM_Data.PrintServerMode & (~PS_DHCP_ON)) | (PSMode & PS_DHCP_ON);
	}
#endif	// (!defined(O_PCI) && !defined(O_CONRAD) && !defined(O_ELEC) && !defined(O_PLANET) && !defined(O_LEVELO))

	NSET32( &EEPROM_Data.EEPROMWriteCount, EEPROMWriteCount );

	return WriteToFlash(&EEPROM_Data);
}

int WriteToDefault(EEPROM *RomData)
{
	uint32 CheckSum;
	int rc = 1;

	if(RomData != &DEFAULT_Data)
		memcpy(&DEFAULT_Data,RomData,sizeof(EEPROM));

	if( strcmp(RomData->ZOT_Mark,"ZOT") ) return 1;  // EEPROM Data is wrong

	DEFAULT_Data.EEPROMWriteCount++;	// Write Count
	
	cli();
	cyg_scheduler_lock();
	
	memset( DEFAULT_Data.CheckSum2, 0, 4 );
	CheckSum = crc32( &DEFAULT_Data, sizeof(EEPROM)-4, 0xFFFFFFFFL );
	memcpy( DEFAULT_Data.CheckSum2, &CheckSum, 4 );

	rc = vEraseDefault();
	
	if(rc == 0){
		/* Now write the DEFAULT_Data */
		rc = vProgramFlash( DEFAULT_FLASH_ADDRESS, &DEFAULT_Data, sizeof(EEPROM) );
	}
	
	cyg_scheduler_unlock();
	sti();

	return rc;
}

int WriteToQC0_Default(EEPROM *RomData)
{
	uint32 CheckSum;
	int rc = 1;

	if(RomData != &QC0_Defualt_EEPROM)
		memcpy(&QC0_Defualt_EEPROM,RomData,sizeof(EEPROM));

	if( strcmp(RomData->ZOT_Mark,"ZOT") ) return 1;  // EEPROM Data is wrong

	QC0_Defualt_EEPROM.EEPROMWriteCount++;	// Write Count
	
	cli();
	cyg_scheduler_lock();
	
	memset( QC0_Defualt_EEPROM.CheckSum2, 0, 4 );
	CheckSum = crc32( &QC0_Defualt_EEPROM, sizeof(EEPROM)-4, 0xFFFFFFFFL );
	memcpy( QC0_Defualt_EEPROM.CheckSum2, &CheckSum, 4 );

	rc = vErase_QC0_Default();
	
	if(rc == 0){
		/* Now write the DEFAULT_Data */
		rc = vProgramFlash( QC0_DEF_FLASH_ADDRESS, &QC0_Defualt_EEPROM, sizeof(EEPROM) );
	
		if( rc == 0 )
		{
			if( ReadFromQC0_Default(&QC0_Defualt_EEPROM) != 0 )
			{
				rc = 1;
			}
		}
	}
	
	cyg_scheduler_unlock();
	sti();	

	return rc;
}

int DoChecksum( uint8 *Address )
{
	uint32 CheckSum1, CheckSum2, Length;
//ZOT716u2	HEADER_VER *hdr1 = Address;
	HEADER_VER hdr1;
	
	read_flash(Address, &hdr1, sizeof(HEADER_VER));
	
	if( hdr1.mark[0] != 'X' ||
		hdr1.mark[1] != 'G' ||
		hdr1.mark[2] != 'Z' ||
		hdr1.model   != CURRENT_PS_MODEL)		
		return 0; //Mark Error
	
	memcpy( &CheckSum1, &hdr1.crc32_chksum, 4 );
	Length = NGET32((char *)&(hdr1.file_len));
	
//ZOT716u2	CheckSum2 = crc32(Address + sizeof( HEADER_VER ), Length, 0xFFFFFFFFL );	

	memset( UPGRADE_TEMP_ADDRESS, Length + sizeof( HEADER_VER ), 0 );
	read_flash(Address + BIN_ID_OFFSET, UPGRADE_TEMP_ADDRESS, Length + sizeof( HEADER_VER ) - 4);
	CheckSum2 = crc32(UPGRADE_TEMP_ADDRESS, Length + sizeof( HEADER_VER ) - 4, 0xFFFFFFFFL );
//	CheckSum2 = crc32_flash(Address + BIN_ID_OFFSET, Length + sizeof( HEADER_VER ) - 4, 0xFFFFFFFFL );	
	if( CheckSum2 != CheckSum1 )
		return 0;

	return Length;
}

#if defined(ARCH_MIPS)
int _DoChecksum( uint8 *Address, unsigned long len )
{
    uint32 checksum;
    uboot_image_header_t hdr;
    HEADER_VER  zhdr;
    read_flash(Address, &hdr, sizeof(uboot_image_header_t));
    read_flash(Address + len - sizeof(HEADER_VER), &zhdr, sizeof(HEADER_VER));
    
	if( zhdr.mark[0] != 'X' ||
		zhdr.mark[1] != 'G' ||
		zhdr.mark[2] != 'Z' ||
        zhdr.model   != CURRENT_PS_MODEL) {
		return 0; //Mark Error
    }	
    memset( UPGRADE_TEMP_ADDRESS, len, 0 );
    read_flash( Address, UPGRADE_TEMP_ADDRESS, len );
    checksum = _crc32(0, UPGRADE_TEMP_ADDRESS + sizeof(uboot_image_header_t), ntohl(hdr.ih_size));
    if (checksum != ntohl(hdr.ih_dcrc)) {
        return 0;
    }
    return len;
}
#endif

int vProgramLoader( char *pCode2Data, uint32 length )
{
	uint8	*CurTempAddress;
	uint8	*CurFlashAddress;
	int		retry, error = 0;
	uint32  i,j=0,write_leng;	
	
	for( retry = 0; retry < PROGRAM_MAX_RETRY_TIMES; retry++ )
	{
		CurTempAddress = pCode2Data;
		CurFlashAddress = LOADER_START_ADDRESS;
		error = 0;
		
		Light_Off( Status_Lite );
		error = vEraseLoader();
		Light_On( Status_Lite );
			
		if(!error)
		{

			for( i = 0; i < length && !error ; i+=write_leng )
			{
				if((j % 2  ) == 0 )	
					Light_Off(Status_Lite);
				else
					Light_On(Status_Lite);
				
				if( (length - i)> PROGRAM_CODE2_KSIZE *1024 )
					write_leng = PROGRAM_CODE2_KSIZE * 1024;
				else
					write_leng = length - i;
				
				error = vProgramFlash( CurFlashAddress, 
					CurTempAddress, write_leng );
				CurTempAddress += write_leng;
				CurFlashAddress += write_leng;
				j++;
			}
		}
	}
	return error;
}

int vProgramCode1( char *pCode2Data, uint32 length )
{
	uint8	*CurTempAddress;
	uint8	*CurFlashAddress;
	int		retry, error = 0;
	uint32  i,j=0,write_leng;
	
	
	
	for( retry = 0; retry < PROGRAM_MAX_RETRY_TIMES; retry++ )
	{
		CurTempAddress = pCode2Data;
		CurFlashAddress = CODE1_START_ADDRESS;
		error = 0;
		
		Light_Off( Status_Lite );
		error = vEraseCode1();
		Light_On( Status_Lite );
			
		if(!error)
		{

			for( i = 0; i < length && !error ; i+=write_leng )
			{
				if((j % 2  ) == 0 )	
					Light_Off(Status_Lite);
				else
					Light_On(Status_Lite);
				
				if( (length - i)> PROGRAM_CODE2_KSIZE *1024 )
					write_leng = PROGRAM_CODE2_KSIZE * 1024;
				else
					write_leng = length - i;
				
				error = vProgramFlash( CurFlashAddress, 
					CurTempAddress, write_leng );
				CurTempAddress += write_leng;
				CurFlashAddress += write_leng;
				j++;
			}
		}

		if(!error)
		{
			if( DoChecksum( CODE1_START_ADDRESS ) == 0 )
				continue;
			break;
		}	
	}
	return error;
}

int vProgramCode2( char *pCode2Data, uint32 length )
{
	uint8	*CurTempAddress;
	uint8	*CurFlashAddress;
	int		retry, error = 0;
	uint32  i,j=0,write_leng;
	
    for( retry = 0; retry < PROGRAM_MAX_RETRY_TIMES; retry++ )
    {
        CurTempAddress = pCode2Data;
        CurFlashAddress = CODE2_START_ADDRESS;
        error = 0;

        Light_Off( Status_Lite );
        error = vEraseCode2();
        Light_On( Status_Lite );

        if(!error)
        {

            for( i = 0; i < length && !error ; i+=write_leng )
            {
                if((j % 2  ) == 0 )
                    Light_Off(Status_Lite);
                else
                    Light_On(Status_Lite);

                if( (length - i)> PROGRAM_CODE2_KSIZE *1024 )
                    write_leng = PROGRAM_CODE2_KSIZE * 1024;
                else
                    write_leng = length - i;

                error = vProgramFlash( CurFlashAddress,
                    CurTempAddress, write_leng );
                CurTempAddress += write_leng;
                CurFlashAddress += write_leng;
                j++;
            }
        }

        if(!error)
        {
            #if defined(ARCH_ARM)
            if( DoChecksum( CODE2_START_ADDRESS ) == 0 )
                continue;
            #endif /* ARCH_ARM */
            #if defined(ARCH_MIPS)
            if( _DoChecksum( CODE2_START_ADDRESS, length ) == 0 )
                continue;
            #endif /* ARCH_MIPS */
            break;
        }
    }
    return error;
}

#ifdef ARCH_ARM
int vProgramWeb( char *pWebData, uint32 length)
{
	uint8	*CurTempAddress;
	uint8	*CurFlashAddress;
	int		retry, error = 0;
	uint32  i,write_leng;
	
	for( retry = 0; retry < PROGRAM_MAX_RETRY_TIMES; retry++ )
	{
		CurTempAddress = pWebData;
		CurFlashAddress = WEB_START_ADDRESS;
		error = 0;

		error = vEraseWeb();
			
		if(!error)
		{

			for( i = 0; i < length && !error ; i+=write_leng )
			{
				if( (length - i)> PROGRAM_WEB_KSIZE *1024 )
					write_leng = PROGRAM_WEB_KSIZE * 1024;
				else
					write_leng = length - i;
				
				error = vProgramFlash( CurFlashAddress, 
					CurTempAddress, write_leng );
				CurTempAddress += write_leng;
				CurFlashAddress += write_leng;
			}
		}

		if(!error)
		{
			if( DoChecksum( WEB_START_ADDRESS ) == 0 )
				continue;
			break;
		}	
	}
	return error;
}
#endif /* ARCH_ARM */

int vProgramDef( char *pDefData, uint32 length)
{
	int		rc = 0;

	rc = vEraseDefault();
	
	rc = vEraseEEP();
	
	if(rc == 0){
	/* Now write the DEFAULT_Data */
		rc = vProgramFlash( DEFAULT_FLASH_ADDRESS, &EEPROM_Data, sizeof(EEPROM) );
	}	
	return rc;
}

#ifdef CFG_EXPIMP
// George added this at build0011 of 716U2W on May 17, 2013.
int vProgramCFG( EEPROM *pDefData, uint32 length)
{
	int		rc = 0;

	rc = vEraseEEP();
	
	if(rc == 0){
	/* Now write the EEPROM_Data */
		//rc = vProgramFlash( SYS_FLASH_ADDRESS, &EEPROM_Data, sizeof(EEPROM) );
		rc = vProgramFlash( SYS_FLASH_ADDRESS, &pDefData, sizeof(EEPROM) );
	}
	return rc;
}
#endif	// CFG_EXPIMP

#ifdef ARCH_MIPS
void hal_ra305x_reset(void);
#endif /* ARCH_MIPS */

void Reset(void)
{
#ifdef ARCH_ARM
//	CacheDisable();
	cyg_interrupt_disable();
	HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();	//ZOT716u2
#endif /* ARCH_ARM */
#ifdef ARCH_MIPS
    hal_ra305x_reset();
#endif /* ARCH_MIPS */
}

unsigned long PSCheckImageIntegrity(HEADER_VER *hdr1){
	unsigned long offset = MAXFLASHOFF;
	unsigned int checksum1, checksum2;
	unsigned int imagelen;
				
	if( hdr1->mark[0] != 'X' ||
#if defined(N716U2S)
		hdr1->mark[1] != 'X' ||
#else
		hdr1->mark[1] != 'G' ||
#endif	// defined(N716U2S)
		hdr1->mark[2] != 'Z' ||
		hdr1->model   != CURRENT_PS_MODEL )			
		return offset; //Mark Error

	memcpy( &checksum1, &hdr1->crc32_chksum, 4 );
	imagelen = NGET32((char *)&(hdr1->file_len));
	
//	checksum2 = crc32((char *)hdr1 + sizeof( HEADER_VER ), imagelen, 0xFFFFFFFFL );	
	checksum2 = crc32((char *)hdr1 + BIN_ID_OFFSET, imagelen + (sizeof(HEADER_VER) -4), 0xFFFFFFFFL );	
	if (checksum2 != checksum1)
		return offset;
		
	switch (hdr1->bintype){
	case ALLFLASHMARK:
		offset = 	ALLFLASHADDROFF;	
		break;
	case LOADERMARK:
		offset = 	LOADERADDROFF;	
		break;	
	case CODE1MARK:
		offset = 	CODE1ADDROFF;				
		break;
	case CODE2MARK:
		offset = 	CODE2ADDROFF;	
		break;
	case EEPROMMARK:
		offset = 	DEFAULTADDROFF;	
		break;		
	default:
		offset = 	MAXFLASHOFF;	//fail
		break;						
	}	
	return offset; //correct Image file		
}

#ifdef CFG_EXPIMP
// George added this at build0011 of 716U2W on May 17, 2013.
unsigned long PSCheckCFGIntegrity(EEPROM *hdr1){
	unsigned long offset = MAXFLASHOFF;
	unsigned int checksum1, checksum2;
	unsigned int imagelen;
	
	if( strcmp(hdr1->ZOT_Mark,"ZOT") )
		return offset;  // EEPROM Data is wrong
				
	if( hdr1->Model != CURRENT_PS_MODEL )
		return offset; //Mark Error

	memcpy( &checksum1, &hdr1->CheckSum2, 4 );
	//imagelen = NGET32((char *)&(hdr1->file_len));
	imagelen = 1408;
	
	checksum2 = crc32((char *)hdr1, sizeof(EEPROM)-4, 0xFFFFFFFFL );	
	if (checksum2 != checksum1)
		return offset;
		
	offset = EEPROMADDROFF;

	return offset; //correct Image file		
}
#endif	// CFG_EXPIMP


int
ApUpgradeFirmware(char *pData, unsigned long len)
{
    unsigned long nbytes;
    unsigned short *src = NULL;
    unsigned long offset;
    char          *pread = pData;
	HEADER_VER *hdr1 = (HEADER_VER *)pread;
	HEADER_VER *prehdr1 = NULL;
    uboot_image_header_t *uheader_ptr;
    unsigned long zhdr_offset;
    unsigned long crclen;
        
    if ( pData == NULL)
    {
        return FALSE;
    }
    
    cli();        

	do {
		if ((offset = PSCheckImageIntegrity(hdr1)) == MAXFLASHOFF)
		{
			sti();
			return FALSE;
		}
	
		if (hdr1->bintype == ALLFLASHMARK){
			if (hdr1->next == 1){ 
				prehdr1 = hdr1;   // Temp hdr1, to change hdr1 soon
				pread = pread + sizeof(HEADER_VER); //Jump chain header
				hdr1 = (HEADER_VER *)pread; //next
				continue;
			}
		}
				
		nbytes = hdr1->file_len;	
		prehdr1 = hdr1;
				
		
		switch(offset)
		{
			case LOADERADDROFF:
                cyg_scheduler_lock();
                src = (char *)hdr1 + sizeof(HEADER_VER);
                vProgramLoader( src, nbytes);
                cyg_scheduler_unlock();
				break;

			case CODE1ADDROFF:
#if defined(ARCH_ARM)
				nbytes += sizeof(HEADER_VER);
                cyg_scheduler_lock();
                vProgramCode1( hdr1, nbytes);
                cyg_scheduler_unlock();
#endif /*ARCH_ARM */
				break;

			case CODE2ADDROFF:
#if defined (ARCH_MIPS)
                /*
                 * copy zot header to end
                 */
                zhdr_offset = ((nbytes + (sizeof(HEADER_VER)-1)) / sizeof(HEADER_VER)) * sizeof(HEADER_VER);
                hdr1->crc32_chksum = htonl(0);
                hdr1->crc32_chksum = _crc32(0, hdr1, sizeof(HEADER_VER));
                memcpy((void *)hdr1 + zhdr_offset + sizeof(HEADER_VER), hdr1, sizeof(HEADER_VER));
                /*
                 * update date length and let pointer cross zot header
                 */
                nbytes = zhdr_offset + sizeof(HEADER_VER);
                hdr1++;
#endif /* ARCH_MIPS */
#if defined (ARCH_ARM)
                nbytes += sizeof(HEADER_VER);
#endif /* ARCH_ARM */
                cyg_scheduler_lock();
                vProgramCode2( hdr1, nbytes);
                cyg_scheduler_unlock();
                break;

                #if defined(ARCH_ARM)
                case WEBOFF:
                    nbytes += sizeof(HEADER_VER);
                    vProgramWeb(hdr1, nbytes);
                    break;

                #endif /* ARCH_ARM */
                case DEFAULTADDROFF:
                    vProgramDef( hdr1, nbytes);
                    break;
	
		}
	
		sti();
    
		if( hdr1->mark[0] == 'X' &&
#if defined(N716U2S)
			hdr1->mark[1] == 'X' &&
#else
			hdr1->mark[1] == 'G' &&
#endif	// defined(N716U2S)
			hdr1->mark[2] == 'Z' )
		{
			// jump its header length
			pread = pread + sizeof(HEADER_VER); 
		}
		
		pread = pread + hdr1->file_len;
		hdr1 = (HEADER_VER *)pread;					
	} while(prehdr1!= NULL && prehdr1->next == 1);

    return TRUE;
}

#ifdef CFG_EXPIMP
int
ApImportCFG(char *pData, unsigned long len)
{
    unsigned long 	nbytes;
    unsigned short 	*src = NULL;
    unsigned long 	offset;

    char          	*pread = pData;
	EEPROM			*hdr1 = (EEPROM *)pread;
	EEPROM 			*prehdr1 = NULL;
        
    if ( pData == NULL)
    {
        return FALSE;
    }

    cli();        

	//do
	//{
		// Check CFG Integrity
		if ((offset = PSCheckCFGIntegrity(hdr1)) == MAXFLASHOFF)
		{
			sti();
			return FALSE;
		}

		//nbytes = hdr1->file_len;
		nbytes = 1408;
		prehdr1 = hdr1;
		
		if(offset == EEPROMADDROFF)
			vProgramCFG( hdr1, nbytes);
	
		sti();

#if 0
		if( hdr1->mark[0] == 'X' &&
			hdr1->mark[1] == 'G' &&
			hdr1->mark[2] == 'Z' )
		{
			// jump its header length
			pread = pread + sizeof(HEADER_VER); 
		}
		
		pread = pread + hdr1->file_len;
		hdr1 = (HEADER_VER *)pread;
#endif 0
	//} while(prehdr1!= NULL && prehdr1->next == 1);
	//} while(prehdr1!= NULL);

    return TRUE;
}
#endif	// CFG_EXPIMP
