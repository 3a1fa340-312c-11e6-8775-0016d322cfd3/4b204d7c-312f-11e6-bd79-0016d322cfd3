
#include "star_misc.h"	//ZOT
#include "star_spi.h"
#include "spi_flash.h"

#define far

/*---------------------------------------------------------------------------*/
/*                             Standard "C" Types                            */
/*---------------------------------------------------------------------------*/
typedef unsigned char		u8;
typedef unsigned char		uint8;
typedef signed   char		int8;
typedef unsigned short		u16;
typedef unsigned short		uint16;
typedef signed   short		int16;
typedef unsigned int        uint;
typedef unsigned int		u32;
typedef unsigned char		uchar;

#define FLASHBASE              	(0x30000000)	//ZOT716u2

static uint8 flash_erase( volatile uint16 *fp );



#define STATUS_READY    0       //* ready for action *//
#define STATUS_BUSY     1       //* operation in progress *//
#define STATUS_ERSUSP   2       //* erase suspended *//
#define STATUS_TIMEOUT  3       //* operation timed out *//
#define STATUS_ERROR    4       //* unclassified but unhappy status *//
#define TOTAL_SECTOR          19


/*-----------------------------------------------------------------------
 * return codes from flash_write():
 */
#define ERR_OK				0
#define ERR_TIMOUT			1
#define ERR_NOT_ERASED			2
#define ERR_PROTECTED			4
#define ERR_INVAL			8
#define ERR_ALIGN			16
#define ERR_UNKNOWN_FLASH_VENDOR	32
#define ERR_UNKNOWN_FLASH_TYPE		64
#define ERR_PROG_ERROR			128

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Bus_Idle
 * PURPOSE:
 *
 ******************************************************************************/
static  u32
Spi_Flash_Is_Bus_Idle(void)
{
	/*
	 * Return value :
	 *    1 : Bus Idle
	 *    0 : Bus Busy
	 */
	return ((SPI_SERVICE_STATUS_REG & 0x1) ? 0 : 1);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Tx_Buffer_Empty
 * PURPOSE:
 *
 ******************************************************************************/
static  u32
Spi_Flash_Is_Tx_Buffer_Empty(void)
{
	/*
	 * Return value :
	 *    1 : SPI Tx Buffer Empty
	 *    0 : SPI Tx Buffer Not Empty
	 */
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 3)) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Rx_Buffer_Full
 * PURPOSE:
 *
 ******************************************************************************/
static  u32
Spi_Flash_Is_Rx_Buffer_Full(void)
{
	/*
	 * Return value :
	 *    1 : SPI Rx Buffer Full
	 *    0 : SPI Rx Buffer Not Full
	 */
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 2)) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Buffer_Transmit_Receive
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Buffer_Transmit_Receive(u32 tx_channel, u32 tx_eof_flag, u32 tx_data, u32 * rx_data)
{
	u32 volatile rx_channel;
	u32 volatile rx_eof_flag;

	/*
	 * 1. Wait until SPI Bus is idle, and Tx Buffer is empty
	 * 2. Configure Tx channel and Back-to-Back transmit EOF setting
	 * 3. Write Tx Data 
	 * 4. Wait until Rx Buffer is full
	 * 5. Get Rx channel and Back-to-Back receive EOF setting
	 * 6. Get Rx Data
	 */
	while (!Spi_Flash_Is_Bus_Idle()) ;

	while (!Spi_Flash_Is_Tx_Buffer_Empty()) ;

	SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
	SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3) | ((tx_eof_flag & 0x1) << 2);

	SPI_TRANSMIT_BUFFER_REG = tx_data;

	while (!Spi_Flash_Is_Rx_Buffer_Full()) ;

	rx_channel = (SPI_RECEIVE_CONTROL_REG & 0x3);

	rx_eof_flag = (SPI_RECEIVE_CONTROL_REG & (0x1 << 2)) ? 1 : 0;

	*rx_data = SPI_RECEIVE_BUFFER_REG;

	if ((tx_channel != rx_channel) || (tx_eof_flag != rx_eof_flag)) {
		return 0;	// Failed!!
	} else {
		return 1;	// OK!!
	}
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Read_Status_Register
 * PURPOSE:
 *
 ******************************************************************************/
static void
Spi_Flash_Read_Status_Register(u8 spi_flash_channel, u8 * status_reg)
{
	u32 rx_data;

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_RDSR_OPCODE, &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, 0xFF, &rx_data);

	*status_reg = (u8) (rx_data & 0xFF);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Flash_Ready
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Is_Flash_Ready(u8 spi_flash_channel)
{
	u8 status_reg;

	/*
	 * Return value :
	 *    1 : SPI Flash is ready
	 *    0 : SPI Flash is busy
	 */
	Spi_Flash_Read_Status_Register(spi_flash_channel, &status_reg);

	return (status_reg & SPI_FLASH_WIP_BIT) ? 0 : 1;
}



/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Set_Write_Enable
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Set_Write_Enable(u8 spi_flash_channel)
{
	u32 rx_data;
	u8 status_reg;

	// Wait until Flash is ready
	while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, SPI_FLASH_WREN_OPCODE, &rx_data);

	Spi_Flash_Read_Status_Register(spi_flash_channel, &status_reg);

	return ((status_reg & SPI_FLASH_WEL_BIT) ? 1 : 0);
}




/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Sector_Erase
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Sector_Erase(u8 spi_flash_channel, u32 sector_addr)
{
	
	u32 rx_data;

	/*
	 * First, issue "Write Enable" instruction, and then issue "Sector Erase" instruction
	 * Note any address inside the Sector is a valid address of the Sector Erase instruction
	 */
	if (Spi_Flash_Set_Write_Enable(spi_flash_channel)) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_SE_OPCODE, &rx_data);

		/*
		 * Note the sector address is 24-Bit
		 */
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((sector_addr >> 16) & 0xFF), &rx_data);

		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((sector_addr >> 8) & 0xFF), &rx_data);

		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, (u32) ((sector_addr >> 0) & 0xFF), &rx_data);

		return 1;
	} else {
		return 0;
	}
}

int spi_flash_erase( int s_first, int s_last)
{
	
	int prot, sect;


	/* Disable interrupts which might cause a timeout here */
//	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (!Spi_Flash_Sector_Erase(0, sect * 4096)) {
			return ERR_PROG_ERROR;
		} else {
//			printf("Serial Flash Sector %d Erase OK!\n", sect);
			prot = 1;
		}
	}

	/* re-enable interrupts if necessary */
//	if (flag)
//		enable_interrupts();

	return (ERR_OK);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Read_Data_Bytes
 * PURPOSE:
 *
 ******************************************************************************/
void
Spi_Flash_Read_Data_Bytes(u8 spi_flash_channel, u32 address, u8 * read_buffer, u32 len)
{
	u32 rx_data;
	u32 ii;

	// Wait until Flash is ready
	while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

#if 1
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_FAST_READ_OPCODE, &rx_data);
#else
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_READ_OPCODE, &rx_data);
#endif

	/*
	 * Note the address is 24-Bit.
	 * The first byte addressed can be at any location, and the address is automatically
	 * incremented to the next higher address after each byte of the data is shifted-out.
	 * When the highest address is reached, the address counter rolls over to 000000h,
	 * allowing the read sequence to be continued indefinitely.
	 */
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 16) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 8) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 0) & 0xFF), &rx_data);

#if 1
	/*
	 * Dummy Byte - 8bit, only on FAST_READ
	 */
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 0) & 0xFF), &rx_data);
#endif

	/*
	 * Read "len" data bytes
	 */
	for (ii = 0; ii < len - 1; ii++) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, 0xFF, &rx_data);

		*read_buffer++ = (u8) (rx_data & 0xFF);
	}

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, 0xFF, &rx_data);

	*read_buffer = (u8) (rx_data & 0xFF);
}

/*********************************
 *
 * FUNCTION:  read_flash
 * PURPOSE:
 *
*********************************/
void read_flash(u32 address, u8 * read_buffer, u32 len)
{
	Spi_Flash_Read_Data_Bytes( 0, address, read_buffer, len);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Page_Program_Data_Bytes
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Page_Program_Data_Bytes(u8 spi_flash_channel, u32 address, u8 * write_buffer, u32 len)
{
	u32 rx_data;
	u32 ii;
	
	/*
	 * First, issue "Write Enable" instruction, and then issue "Page Program" instruction
	 */
	if (!Spi_Flash_Set_Write_Enable(spi_flash_channel)) {
		return 0;
	}

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_PP_OPCODE, &rx_data);

	/*
	 * Note the address is 24-Bit
	 * If the 8 least significant address bits (A7~A0) are not all zero, all transmitted
	 * data that goes beyond the end of the current page are programmed from the start
	 * address of the same page (from the address whose 8 least significant address bits 
	 * (A7~A0) are all zero.
	 * If more than 256 bytes are sent to the device, previously latched data are discarded
	 * and the last 256 data bytes are guaranteed to be programmed correctly within the
	 * same page.
	 * If less than 256 Data bytes are sent to the device, they are correctly programmed
	 * at the requested addresses without having any effects on the other bytes of the same
	 * page.
	 */
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 16) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 8) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 0) & 0xFF), &rx_data);

	/*
	 * Write "len" data bytes
	 */
	for (ii = 0; ii < len - 1; ii++) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) * write_buffer++, &rx_data);
	}

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, (u32) * write_buffer, &rx_data);

	return 1;
}


/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff( uchar * src, u32 addr, u32 cnt)
{
	u32 prog_size_left;
	u32 prog_size;
	u32 prog_size_total;
	u32 dest = addr ;

	prog_size_left = cnt;
	prog_size_total = 0;
	prog_size = SPI_FLASH_PAGE_SIZE;

	while (prog_size_left) {
		if( prog_size_left <= prog_size)
			prog_size = prog_size_left;
				
		if (!Spi_Flash_Page_Program_Data_Bytes(0, dest + prog_size_total, src + prog_size_total, prog_size)) {
			return ERR_PROG_ERROR;
		}
		prog_size_left -= prog_size;
		prog_size_total += prog_size;
	}


	return (ERR_OK);
}


/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Initialize
 * PURPOSE:
 *
 ******************************************************************************/
static void
Spi_Flash_Initialize(u8 spi_flash_channel)
{
	u32 volatile receive_data;
	u32 value = 0;
 	
//ZOT
	value = MISC_CHIP_CONFIG_REG;
	value &= ~0x10;
	MISC_CHIP_CONFIG_REG = value;

	// Enable SPI pins
	HAL_MISC_ENABLE_SPI_PINS();

	// Disable SPI serial flash access through 0x30000000 region
	HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS();

	/*
	 * Note SPI is NOT enabled after this function is invoked!!
	 */
	SPI_CONFIGURATION_REG =
		(((0x0 & 0x3) << 0) | /* 8bits shift length */
		 (0x0 << 9) | /* general SPI mode */
		 (0x0 << 10) | /* disable FIFO */
		 (0x1 << 11) | /* SPI master mode */
		 (0x0 << 12) | /* disable SPI loopback mode */
		 (0x0 << 13) |
		 (0x0 << 14) |
		 (0x0 << 24) | /* Disable SPI Data Swap */
		 (0x0 << 30) | /* Disable SPI High Speed Read for BootUp */
		 (0x0 << 31)); /* Disable SPI */

	SPI_BIT_RATE_CONTROL_REG = 0x1 & 0x07; // PCLK/8

	// Configure SPI's Tx channel
	SPI_TRANSMIT_CONTROL_REG &= ~(0x03);
	SPI_TRANSMIT_CONTROL_REG |= spi_flash_channel & 0x03;

	// Configure Tx FIFO Threshold
	SPI_FIFO_TRANSMIT_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_TRANSMIT_CONFIG_REG |= ((0x0 & 0x03) << 4);

	// Configure Rx FIFO Threshold
	SPI_FIFO_RECEIVE_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_RECEIVE_CONFIG_REG |= ((0x1 & 0x03) << 4);

	SPI_INTERRUPT_ENABLE_REG = 0;

	// Clear spurious interrupt sources
	SPI_INTERRUPT_STATUS_REG = (0xF << 4);

	receive_data = SPI_RECEIVE_BUFFER_REG;

	// Enable SPI
	SPI_CONFIGURATION_REG |= (0x1 << 31);

	return;
}

void AT91F_SpiInit(void)
{
	Spi_Flash_Initialize(0);
}


int vEraseFlash( volatile uint16 *fp, int nsectors )
{
	int rc = 0;
	u32 add = (u32)fp;
	u32 start_sect,stop_sect;
	
	start_sect = (add - FLASHBASE) / 4096;
	stop_sect = (start_sect + nsectors) - 1;
	
	if( start_sect > 255 || stop_sect > 255)
		rc = -1;
	
	rc = spi_flash_erase(start_sect, stop_sect);
	if(	rc )
		rc = -1;
	return rc;
}




