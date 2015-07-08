#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "star_powermgt.h"	//ZOT716u2

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

//#define FLASHSIZE               1 //M bytes, define on Target.def
#define ALLFLASHADDROFF        	0
#define LOADERADDROFF			0x00000000
#define CODE1ADDROFF           	0x00010000
#define CODE2ADDROFF           	0x00030000
#define WEBOFF					0x00090000
#define DEFAULTADDROFF         	0x000FC000
#define EEPROMADDROFF         	0x000FE000
#define MAXFLASHOFF             0x00100000

//temp extern int flashWriteWord (int offset, unsigned short dat);
//temp extern unsigned int crc32(unsigned char *address, unsigned int size, unsigned int crc); //Ron Add
//temp extern void WatchdogDisable (void);
//temp extern void WatchdogEnable (void);
//temp extern void EraseSectors(int sblk, int nblks);

//from psmain.c
extern uint32 NGET32( uint8 *pSrc );
extern BYTE  PSMode;
extern uint32 get32(uint8 *cp);

#define PROGRAM_MAX_RETRY_TIMES			1

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

	memset( UPGRADE_TEMP_ADDRESS, 0, PROGRAM_CODE2_KSIZE * 1024 );
	return 1;
}

int vReleaseCode2Memory()
{

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

int WriteToFlash(EEPROM *RomData)
{
	uint32 CheckSum;
	int rc = 1;
    int nbytes;
    unsigned short *src = NULL;
    int offset;

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

int WriteToDefault(EEPROM *RomData)
{
	uint32 CheckSum;
	int rc = 1;
    int nbytes;
    unsigned short *src = NULL;
    int offset;

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

int DoChecksum( uint8 *Address )
{
	uint32 CheckSum1, CheckSum2, Length;
//ZOT716u2	HEADER_VER *hdr1 = Address;
	HEADER_VER hdr1;
	
	read_flash(Address, &hdr1, sizeof(HEADER_VER));
	
	if( hdr1.mark[0] != 'X' ||
		hdr1.mark[1] != 'G' ||
		hdr1.mark[2] != 'Z' ||
		hdr1.model   != CODE1_PS_MODEL)		
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
			if( DoChecksum( CODE2_START_ADDRESS ) == 0 )
				continue;
			break;
		}	
	}
	return error;
}

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

void Reset(void)
{
	cyg_interrupt_disable();
	HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();	//ZOT716u2
}

unsigned long PSCheckImageIntegrity(HEADER_VER *hdr1){
	unsigned long offset = MAXFLASHOFF;
	unsigned int checksum1, checksum2;
	unsigned int imagelen;
				
	if( hdr1->mark[0] != 'X' ||
		hdr1->mark[1] != 'G' ||
		hdr1->mark[2] != 'Z' )		
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


int
ApUpgradeFirmware(char *pData, unsigned long len)
{
    unsigned long nbytes;
    unsigned short *src = NULL;
    unsigned long offset;
    unsigned long StartSector;
    int tmp,i=0;
    unsigned long sector_counter;
    char 		  *mfgptr = NULL;	
    char          *pread = pData;
	HEADER_VER *hdr1 = (HEADER_VER *)pread;
	HEADER_VER *prehdr1 = NULL;
        
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
				nbytes += sizeof(HEADER_VER);
				cyg_scheduler_lock();
				vProgramCode1( hdr1, nbytes);
				cyg_scheduler_unlock();			
				break;
			case CODE2ADDROFF:
				nbytes += sizeof(HEADER_VER);
				cyg_scheduler_lock();
				vProgramCode2( hdr1, nbytes);
				cyg_scheduler_unlock();
				break;
			case DEFAULTADDROFF:
				vProgramDef( hdr1, nbytes);
				break;
	
		}
	
		sti();
    
		if( hdr1->mark[0] == 'X' &&
			hdr1->mark[1] == 'G' &&
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
