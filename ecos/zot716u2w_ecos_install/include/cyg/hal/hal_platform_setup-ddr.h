#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    ZOT
// Contributors: ZOT
// Date:        2007-10-25
// Purpose:     ARM/ZOT716U2 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#define CONFIG_SPI_FLASH_SUPPORT
//#define CONFIG_USE_SDRAM			/* use sdram */
#define CONFIG_USE_DDR				/* use ddr ram */

#define RAM_SIZE_16MBIT		(0)
#define RAM_SIZE_64MBIT		(1)
#define RAM_SIZE_128MBIT	(2)
#define RAM_SIZE_256MBIT	(3)
#define RAM_SIZE_512MBIT	(4)

#define RAM_SIZE		RAM_SIZE_256MBIT //32MB

#define PLATFORM_SETUP1 platform_setup1

    .macro  platform_setup1

	//mkl070509: shut down the watchdog, in case the HW reset is enabled.
	ldr	r0, =0x7A00000C
	mov	r1, #0x0
	str	r1, [r0]

#ifdef CONFIG_SPI_FLASH_SUPPORT
	// enable SPI high speed read for booting
	ldr	r0, =0x71000040
	ldr	r1, [r0]
	mov	r2, #0x40000000
	orr	r1, r1, r2
	str	r1, [r0]

	// change SPI clock rate
	ldr	r0, =0x71000048
	mov	r1, #0x1
	str	r1, [r0]
#endif

	// set SMC bank0 setting
	ldr	r0, =0x73000000
	// write protect off, bank enable, bus width 8
	ldr	r1, =0x00000002
	str	r1, [r0]

	// set SMC bank0 timing
	ldr	r0, =0x73000004
	ldr	r1, =0x330F0F0F
	str	r1, [r0]

	// disable PLL power-down, Disable I2S,USB Device
	ldr	r0, =0x77000010
	ldr	r1, =0x00000050
	str	r1, [r0]

	//Disable PCI,DMA,IDE;Enable USB, MAC, VIC, DRAM, Static Memory
	ldr	r0, =0x77000000
	ldr	r1, =0x01F01033
	str	r1, [r0]
	
	ldr	r0, =0x77000004
	ldr	r1, =0x02021003
	str	r1, [r0]

	//ZOT716u2 reset all devices but Global and Static Memory controller.
	ldr	r0, =0x77000008
	ldr	r1, =0x00000003
	str	r1, [r0]

	// De-assert all of RESETn, i.e., exit reset state
	ldr	r0, =0x77000008
	ldr	r1, =0xFFFFFFFF
	str	r1, [r0]

#ifdef CONFIG_USE_DDR
	// for Str8181 not using both SDR and MII interface simultaneously
	// Set pd_25 = 0, and sel_sdr = 0
	ldr	r0, =0x77000024
	ldr	r1, =0x00130723
	str	r1, [r0]

	// Configuration of Memory Interface Configure Register 
	ldr	r0, =0x72000000
#if (RAM_SIZE == RAM_SIZE_256MBIT)
	// DDRC: 16-bit mode, SDRAM Module: 512Mb * 16, Little-Endian
	ldr	r1, =0x02000021
#elif (RAM_SIZE == RAM_SIZE_512MBIT)
	// DDRC: 16-bit mode, SDRAM Module: 256Mb * 16, Little-Endian
	ldr	r1, =0x02000022
#elif (RAM_SIZE == RAM_SIZE_128MBIT)
	// DDRC: 16-bit mode, SDRAM Module: 128Mb * 16, Little-Endian
	ldr	r1, =0x02000020

#endif
	str	r1, [r0]

	// Configuration of Parameter Configure Register
	ldr	r0, =0x72000004
#ifdef CONFIG_USE_DDR_ON_FPGA
	ldr	r1, =0x00000020
#else
	ldr	r1, =0x00000020
#endif
	str	r1, [r0]

	// Configuration of Timing Parameter 0 Register
	ldr	r0, =0x72000010
	ldr	r1, =0x32292A62
	str	r1, [r0]

	// Configuration of Timing Parameter 1 Register
	ldr	r0, =0x72000014
	ldr	r1, =0x140F10C8
	str	r1, [r0]

	// Configuration of Timing Parameter 2 Register
	ldr	r0, =0x72000018
#ifdef CONFIG_USE_DDR_ON_FPGA
	ldr	r1, =0x00080078
#else
	ldr	r1, =0x00070029
#endif
	str	r1, [r0]

	// Configuration of Power-Up Control Register
	ldr	r0, =0x72000008
	ldr	r1, =0x00000001
	str	r1, [r0]

	// wait for InitCmp bit to become 1
wait:
	ldr	r1, [r0]
	cmp	r1, #0x2
	bne	wait

	// Configuration of Preread Timeout Disable Register	
	ldr	r0, =0x7200001C
	ldr	r1, =0x00000080
	str	r1, [r0]

	// Configuration of Preread Enable Register
	ldr	r0, =0x72000020
	ldr	r1, =0x000000FF
	str	r1, [r0]

	// Configuration of DQS Input Delay Control Register
	ldr	r0, =0x72000034
	ldr	r1, =0x00330033
	str	r1, [r0]

#if 0
	// Configuration of DDQ Output Delay Control Register
	ldr	r0, =0x72000030
	ldr	r1, =0x00000044
	str	r1, [r0]

	// Configuration of DQS Input Delay Control Register
	ldr	r0, =0x72000034
	ldr	r1, =0x04040404
	str	r1, [r0]

	// Configuration of HCLK Delay Control Register
	ldr	r0, =0x72000038
	ldr	r1, =0x07
	str	r1, [r0]
#endif
#else
	// for Str8133 using SDR and MII interface simultaneously
	// Set pd_25 = 1, and sel_sdr = 1
	ldr	r0, =0x77000024
	ldr	r1, =0x00133723
	str	r1, [r0]

	// Configuration of Memory Interface Configure Register 
	// SDRC: 16-bit mode, SDRAM Module: 256Mb * 16, Little-Endian
	ldr	r0, =0x72000000

#if (RAM_SIZE == RAM_SIZE_512MBIT)
	ldr	r1, =0x00000022
#elif (RAM_SIZE == RAM_SIZE_256MBIT)
	ldr	r1, =0x00000021
#elif (RAM_SIZE == RAM_SIZE_128MBIT)
	ldr	r1, =0x00000020
#elif (RAM_SIZE == RAM_SIZE_64MBIT)
	ldr	r1, =0x00000026
#elif (RAM_SIZE == RAM_SIZE_16MBIT)
	ldr	r1, =0x00000024
#endif

	str	r1, [r0]

	// Configuration of Parameter Configure Register
	ldr	r0, =0x72000004
	ldr	r1, =0x00000020
	str	r1, [r0]

	// Configuration of Timing Parameter 0 Register
	ldr	r0, =0x72000010
	ldr	r1, =0x32292A62
	str	r1, [r0]

	// Configuration of Timing Parameter 1 Register
	ldr	r0, =0x72000014
	ldr	r1, =0x140F09C8
	str	r1, [r0]

	// Configuration of Timing Parameter 2 Register
#ifdef CONFIG_USE_SDR_ON_FPGA
	ldr	r0, =0x72000018
	ldr	r1, =0x00080078
	str	r1, [r0]
#else
	ldr	r0, =0x72000018
	ldr	r1, =0x00070029
	str	r1, [r0]
#endif

#if 0	//BOOT loader only (Run on SPI Flash)//ZOT716u2
	// Configuration of Power-Up Control Register
	ldr	r0, =0x72000008
	ldr	r1, =0x00000001
	str	r1, [r0]

	// wait for InitCmp bit to become 1
wait:
	ldr	r1, [r0]
	cmp	r1, #0x2
	bne	wait
#endif //ZOT716u2

	// Configuration of Preread Timeout Disable Register	
	ldr	r0, =0x7200001C
	ldr	r1, =0x00000080
	str	r1, [r0]

	// Configuration of Preread Enable Register
	ldr	r0, =0x72000020
	ldr	r1, =0x000000FF
	str	r1, [r0]
#endif
	
	.endm

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
