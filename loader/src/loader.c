#include "global.h"
#include "star_intc.h"
#include "star_timer.h"
#include "star_powermgt.h"

#define __PACKED__   __packed

typedef __PACKED__ struct _header_ver1{
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
} HEADER_VER1, *pHEADER_VER1;

#define FLASH_START					0x30000000
#define SDRAM_START					0x20000000
#define ZOT716U2_SDRAM_COPY_OFF		0x00550000
#define FL_ZOT716U2_CODE1_OFF		0x00010000
#define FL_ZOT716U2_CODE2_OFF		0x00030000
#define FL_ZOT716U2_CODE1_START		(FLASH_START + FL_ZOT716U2_CODE1_OFF)
#define FL_ZOT716U2_CODE2_START		(FLASH_START + FL_ZOT716U2_CODE2_OFF)
#define ZOT716U2_CRC_OFF			0x00
#define ZOT716U2_LEN_OFF			0x08
#define ZOT716U2_BIN_OFF			0x100
#define FL_ZOT716U2_CODE1_CRC		(FL_ZOT716U2_CODE1_START + ZOT716U2_CRC_OFF)
#define FL_ZOT716U2_CODE1_LEN		(FL_ZOT716U2_CODE1_START + ZOT716U2_LEN_OFF)
#define FL_ZOT716U2_CODE2_CRC		(FL_ZOT716U2_CODE2_START + ZOT716U2_CRC_OFF)
#define FL_ZOT716U2_CODE2_LEN		(FL_ZOT716U2_CODE2_START + ZOT716U2_LEN_OFF)

#define ZOT716U2_FLASH_START		(FL_ZOT716U2_CODE2_START + ZOT716U2_BIN_OFF)
#define ZOT716U2_SDRAM_COPY_START	(SDRAM_START + ZOT716U2_SDRAM_COPY_OFF)
#define ZOT716U2_SDRAM_COPY_CRC		(ZOT716U2_SDRAM_COPY_START + ZOT716U2_CRC_OFF)
#define ZOT716U2_SDRAM_COPY_LEN		(ZOT716U2_SDRAM_COPY_START + ZOT716U2_LEN_OFF)
#define ZOT716U2_SDRAM_COPY_BIN		(ZOT716U2_SDRAM_COPY_START + ZOT716U2_BIN_OFF)
#define ZOT716U2_SDRAM_START		SDRAM_START

unsigned int NGET32( unsigned char *pSrc )
{
	return (unsigned int)( pSrc[0] | pSrc[1]<<8 | pSrc[2]<<16 | pSrc[3]<<24 );
}

#define TIMER_COUNTER_VAL	100

void enable_interrupts (void)
{
	__asm
	{
		mrs r0, CPSR
		bic r0, r0, #0x80
		msr CPSR_c, r0
	}
}

str8131_cpu_enter_idle_mode(void)
{
	__asm
	{
		mcr	p15, 0, r1, c7, c0, 4
	}
}

#define	SYSPA_DDRC_SDRC_BASE_ADDR		0x72000000
static void str8131_adjust_dram_auto_refresh_interval(unsigned int old_ahb_clock, unsigned int new_ahb_clock)
{
	unsigned int dramc_timing_parameter2;
	unsigned int old_auto_refresh_interval;
	unsigned int new_auto_refresh_interval;

	dramc_timing_parameter2 = (*((unsigned int volatile *)(SYSPA_DDRC_SDRC_BASE_ADDR + 0x18)));
	old_auto_refresh_interval = dramc_timing_parameter2 & 0xFFF;
	new_auto_refresh_interval = ((old_auto_refresh_interval + 1) * new_ahb_clock) / (old_ahb_clock) - 1;
	dramc_timing_parameter2 &= ~0xFFF;
	dramc_timing_parameter2 |= new_auto_refresh_interval & 0xFFF;
	(*((unsigned int volatile *)(SYSPA_DDRC_SDRC_BASE_ADDR + 0x18))) = dramc_timing_parameter2;
}

static unsigned int str8131_ahb_clock(void)
{
	unsigned int PLL_clock;
	unsigned int CPU_clock;
	unsigned int AHB_clock;

	switch (PWRMGT_SYSTEM_CLOCK_CONTROL_REG & 0x3) {
	case 0x0:
		PLL_clock = 175000000;
		break;
	case 0x1:
		PLL_clock = 200000000;
		break;
	case 0x2:
		PLL_clock = 225000000;
		break;
	case 0x3:
		PLL_clock = 250000000;
		break;
	}

	CPU_clock = PLL_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 2) & 0x3) + 1);
	AHB_clock = CPU_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 4) & 0x3) + 1);

	return AHB_clock / 1000000;
}

int str8131_cpu_clock_scale_end(void)
{
	unsigned int status;

	// disable timer2
	HAL_TIMER_DISABLE_TIMER2();
	// clear timer2 interrupt status
	status = TIMER1_TIMER2_INTERRUPT_STATUS_REG;
	TIMER1_TIMER2_INTERRUPT_STATUS_REG = status;
	TIMER1_TIMER2_INTERRUPT_MASK_REG = 0x3f;

	status = INTC_EDGE_INTERRUPT_SOURCE_CLEAR_REG;
	INTC_EDGE_INTERRUPT_SOURCE_CLEAR_REG = status;
	INTC_INTERRUPT_MASK_REG = 0xFFFFFFFF;
	
	return status;
}

void str8131_cpu_clock_scale_start(unsigned int cpu_pll, unsigned int ratio)
{
	unsigned int timer_control;
	unsigned int status;
	unsigned int last_time;
	unsigned int env_pll_clock;
	unsigned int env_pll_to_cpu_ratio = ratio;
	unsigned int env_cpu_to_ahb_ratio = 2;
	unsigned int new_pll_clock;
	unsigned int new_cpu_clock;
	unsigned int new_ahb_clock;
	unsigned int old_ahb_clock;
	char *s;

	old_ahb_clock = str8131_ahb_clock();

	env_pll_clock = cpu_pll;

	switch(env_pll_clock) {
	case 175:
		HAL_PWRMGT_SET_PLL_FREQUENCY_175MHZ();
		new_pll_clock = 175;
		break;
	case 200:
		HAL_PWRMGT_SET_PLL_FREQUENCY_200MHZ();
		new_pll_clock = 200;
		break;
	case 225:
		HAL_PWRMGT_SET_PLL_FREQUENCY_225MHZ();
		new_pll_clock = 225;
		break;
	case 250:
		HAL_PWRMGT_SET_PLL_FREQUENCY_250MHZ();
		new_pll_clock = 250;
		break;
	default:
		return;
		break;
	}

	switch(env_pll_to_cpu_ratio) {
	case 1:
	case 2:
	case 3:
	case 4:
		break;
	default:
		env_pll_to_cpu_ratio = 1;
		break;
	}
	new_cpu_clock = new_pll_clock / env_pll_to_cpu_ratio;

	switch (env_cpu_to_ahb_ratio) {
	case 1:
	case 2:
	case 3:
	case 4:
		break;
	default:
		env_cpu_to_ahb_ratio = 2;
	}
	new_ahb_clock = new_cpu_clock / env_cpu_to_ahb_ratio;
	if (new_ahb_clock > 125) {
		env_cpu_to_ahb_ratio++;
		new_ahb_clock = new_cpu_clock / env_cpu_to_ahb_ratio;
	}

	HAL_PWRMGT_CONFIG_PLLCLK_TO_CPUCLK_RATIO(env_pll_to_cpu_ratio);
	HAL_PWRMGT_CONFIG_CPUCLK_TO_HCLK_RATIO(env_cpu_to_ahb_ratio);

	INTC_INTERRUPT_MASK_REG = 0xFFFFFFFF;
	INTC_EDGE_INTERRUPT_SOURCE_CLEAR_REG = 0xFFFFFFFF;
	INTC_SOFTWARE_INTERRUPT_CLEAR_REG = 0xFFFFFFFF;
	INTC_SOFTWARE_PRIORITY_MASK_REG = 0x0;
	INTC_FIQ_SELECT_REG = 0x0;
    INTC_VECTOR_INTERRUPT_ENABLE_REG = 0;

	HAL_TIMER_DISABLE_TIMER2();

	TIMER2_COUNTER_REG = TIMER_COUNTER_VAL;
	TIMER2_AUTO_RELOAD_VALUE_REG = TIMER_COUNTER_VAL;
	TIMER2_MATCH_VALUE1_REG = 0;
	TIMER2_MATCH_VALUE2_REG = 0;

	// mask all the timer interrupt
	TIMER1_TIMER2_INTERRUPT_MASK_REG = 0x3f;

	timer_control = TIMER1_TIMER2_CONTROL_REG;

	// timer2 down counter
	timer_control |= (1 << TIMER2_UP_DOWN_COUNT_BIT_INDEX);

	// timer2 enable overflow interrupt
	timer_control |= (1 << TIMER2_OVERFLOW_INTERRUPT_BIT_INDEX);

	/* timer2 seleck 1KHz Clock */
	timer_control |= (1 << TIMER2_CLOCK_SOURCE_BIT_INDEX);

	/* timer2 enable */
	timer_control |= (1 << TIMER2_ENABLE_BIT_INDEX);

	TIMER1_TIMER2_CONTROL_REG = timer_control;

	// unmask timer2 overflow interrupt
	TIMER1_TIMER2_INTERRUPT_MASK_REG = 0x1f;

	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_TIMER2_BIT_INDEX);

	HAL_PWRMGT_DISABLE_DRAMC_CLOCK();
	if (new_ahb_clock < old_ahb_clock) {
		str8131_adjust_dram_auto_refresh_interval(old_ahb_clock, new_ahb_clock);
	}

	str8131_cpu_enter_idle_mode();
	
	if (new_ahb_clock > old_ahb_clock)
    {
        str8131_adjust_dram_auto_refresh_interval(old_ahb_clock, new_ahb_clock);
    }
    
}

int ZOT716U2CODE2Checksum( void )
{
	unsigned int CheckSum1, CheckSum2, Length, LengthOrg, tmp;

#ifdef SPI_FLASH
	read_flash(FL_ZOT716U2_CODE2_LEN, &LengthOrg , 4);
	Length = NGET32( &LengthOrg );
	if (Length > 0xB0000)	//704KB
		return 0;
	read_flash(FL_ZOT716U2_CODE2_START, ZOT716U2_SDRAM_COPY_START, Length + sizeof(HEADER_VER1) );	
#endif	

	memcpy( &CheckSum1, ZOT716U2_SDRAM_COPY_CRC, 4 );
	memcpy( &LengthOrg, ZOT716U2_SDRAM_COPY_LEN, 4 );	
	
	Length = NGET32( &LengthOrg );

	if (Length > 0xB0000)	//704KB
		return 0;
		
	CheckSum2 = crc32( ZOT716U2_SDRAM_COPY_START + 4, Length + (sizeof(HEADER_VER1) - 4), 0xFFFFFFFFL );		//ZOT716u2 SPI Flash :: 4Bytes read once.

	if( CheckSum2 != CheckSum1 )
		return 0;

	return Length;
}

int ZOT716U2CODE1Checksum( void )
{
	unsigned int CheckSum1, CheckSum2, Length, LengthOrg, tmp;

#ifdef SPI_FLASH
	read_flash(FL_ZOT716U2_CODE1_LEN, &LengthOrg , 4);
	Length = NGET32( &LengthOrg );
	
	if (Length > 0xB0000)	//704KB
		return 0;
	
	read_flash(FL_ZOT716U2_CODE1_START, ZOT716U2_SDRAM_COPY_START, Length + sizeof(HEADER_VER1) );	
#endif	

	memcpy( &CheckSum1, ZOT716U2_SDRAM_COPY_CRC, 4 );
	memcpy( &LengthOrg, ZOT716U2_SDRAM_COPY_LEN, 4 );	
	
	Length = NGET32( &LengthOrg );

	if (Length > 0xB0000)	//704KB
		return 0;
		
	CheckSum2 = crc32( ZOT716U2_SDRAM_COPY_START + 4, Length + (sizeof(HEADER_VER1) - 4), 0xFFFFFFFFL );		//ZOT716u2 SPI Flash :: 4Bytes read once.

	if( CheckSum2 != CheckSum1 )
		return 0;

	return Length;
}

void ARMMain()
{
	int checksum = 0, i;
	unsigned int button_value1 = 0;
	unsigned int button_value2 = 0;	
	
	//LED init
	*(volatile unsigned int *)(0x7C000008) = 0x0001C000;   	//LED (GPIO14.15.16) is output 
	*(volatile unsigned int *)(0x7C000000) = 0x0001C000;    //All LED (GPIO14.15.16) light off

	//SOC init finish. (Light 1)	
	i = 0x500000;
	*(volatile unsigned int *)(0x7C000000) = 0x0000C000; 	//Stauts LED (GPIO16) light on
	while(i--);
	*(volatile unsigned int *)(0x7C000000) = 0x0001C000;    //Stauts LED (GPIO16) light off

	enable_interrupts();
	str8131_cpu_clock_scale_start(250, 1);
		
	SysRemap();	

	//clock scale and remap finish. (Light 2)	
	i = 0x500000;
	*(volatile unsigned int *)(0x7C000000) = 0x0000C000; 	//Stauts LED (GPIO16) light on
	while(i--);
	*(volatile unsigned int *)(0x7C000000) = 0x0001C000;    //Stauts LED (GPIO16) light off
	
	AT91F_SpiInit();
	
	button_value1 = *(volatile unsigned int *)(0x7C000004);
	button_value1 &= 0x01;

	checksum = ZOT716U2CODE2Checksum();	

	//Load default
	button_value2 = *(volatile unsigned int *)(0x7C000004);
	button_value2 &= 0x01;

	if ((button_value1 == button_value1)&&(button_value1 == (0x0))){
		vEraseFlash(0x000FE000, 2);
		i = 0x5000000;
		*(volatile unsigned int *)(0x7C000000) = 0x00000000; 	//All LED light on
		while(i--);
		*(volatile unsigned int *)(0x7C000000) = 0x0001C000;    //All LED light off
		HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();
	}

	//check sum finish. (Light 3)	
	i = 0x500000;
	*(volatile unsigned int *)(0x7C000000) = 0x0000C000; 	//Stauts LED (GPIO16) light on
	while(i--);
	*(volatile unsigned int *)(0x7C000000) = 0x0001C000;    //Stauts LED (GPIO16) light off	
	
	if (checksum){	

		gz_decompress( ZOT716U2_SDRAM_COPY_BIN, ZOT716U2_SDRAM_START );
//debug		gz_decompress( ZOT716U2_SDRAM_COPY_BIN, ZOT716U2_SDRAM_START + 0x4000 );

	//decompress finish. (Light 4)	
	i = 0x500000;
	*(volatile unsigned int *)(0x7C000000) = 0x0000C000; 	//Stauts LED (GPIO16) light on
	while(i--);
	*(volatile unsigned int *)(0x7C000000) = 0x0001C000;  //Stauts LED (GPIO16) light off

	DoeCos();

	}else{	

		checksum = ZOT716U2CODE1Checksum();	
		
		if (checksum){	

			gz_decompress( ZOT716U2_SDRAM_COPY_BIN, ZOT716U2_SDRAM_START );
//debug		gz_decompress( ZOT716U2_SDRAM_COPY_BIN, ZOT716U2_SDRAM_START + 0x4000 );
	
			//decompress finish. (Light 4)	
			i = 0x500000;
			*(volatile unsigned int *)(0x7C000000) = 0x0000C000; 	//Stauts LED (GPIO16) light on
			while(i--);
			*(volatile unsigned int *)(0x7C000000) = 0x0001C000;  //Stauts LED (GPIO16) light off
	
			DoeCos();
		}
		else{
			//Error!! LED flash
		 	*(volatile unsigned int *)(0x7C000008) = 0x0001C000;   	//LED (GPIO7) is output 
		 	
		    while(1){
		    	i = 0x500000;
		    	while(i--);
		 		*(volatile unsigned int *)(0x7C000000) = 0x0000C000; 	//light on
		     	i = 0x100000;
		    	while(i--);		
		 		*(volatile unsigned int *)(0x7C000000) = 0x0001C000;  //light off
		    }
		}
	}
}
