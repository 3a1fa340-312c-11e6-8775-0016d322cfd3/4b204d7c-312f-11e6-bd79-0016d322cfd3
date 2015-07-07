/*******************************************************************************
 *
 *  Copyright(c) 2006 Star Semiconductor Corporation, All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 59
 *  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  The full GNU General Public License is included in this distribution in the
 *  file called LICENSE.
 *
 *  Contact Information:
 *  Technology Support <tech@starsemi.com>
 *  Star Semiconductor 4F, No.1, Chin-Shan 8th St, Hsin-Chu,300 Taiwan, R.O.C
 *
 ******************************************************************************/

#ifndef _STAR_SPI_H_
#define _STAR_SPI_H_




#define	SYSPA_SPI_BASE_ADDR			0x71000000

#define SPI_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_SPI_BASE_ADDR + reg_offset)))


/*
 * define access macros
 */
#define SPI_CONFIGURATION_REG			SPI_MEM_MAP_VALUE(0x40)
#define SPI_SERVICE_STATUS_REG			SPI_MEM_MAP_VALUE(0x44)
#define SPI_BIT_RATE_CONTROL_REG		SPI_MEM_MAP_VALUE(0x48)
#define SPI_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x4C)
#define SPI_TRANSMIT_BUFFER_REG			SPI_MEM_MAP_VALUE(0x50)
#define SPI_RECEIVE_CONTROL_REG			SPI_MEM_MAP_VALUE(0x54)
#define SPI_RECEIVE_BUFFER_REG			SPI_MEM_MAP_VALUE(0x58)
#define SPI_FIFO_TRANSMIT_CONFIG_REG		SPI_MEM_MAP_VALUE(0x5C)
#define SPI_FIFO_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x60)
#define SPI_FIFO_RECEIVE_CONFIG_REG		SPI_MEM_MAP_VALUE(0x64)
#define SPI_INTERRUPT_STATUS_REG		SPI_MEM_MAP_VALUE(0x68)
#define SPI_INTERRUPT_ENABLE_REG		SPI_MEM_MAP_VALUE(0x6C)


/*
 * define constants macros
 */
#define SPI_TX_RX_FIFO_DEPTH			(8)

#define SPI_CH0					(0)
#define SPI_CH1					(1)
#define SPI_CH2					(2)
#define SPI_CH3					(3)


#define SPI_RXFIFO_OT_FG			(0x01)
#define SPI_TXFIFO_UT_FG			(0x02)
#define SPI_RXBUF_FULL_FG			(0x04)
#define SPI_TXBUF_EMPTY_FG			(0x08)

#define SPI_RXFIFO_OR_FG			(0x10)
#define SPI_TXFIFO_UR_FG			(0x20)
#define SPI_RXBUF_OR_FG				(0x40)
#define SPI_TXBUF_UR_FG				(0x80)

/*
 * define Character Length Control
 */
#define SPI_LEN_BIT_8				(0)
#define SPI_LEN_BIT_16				(1)
#define SPI_LEN_BIT_24				(2)
#define SPI_LEN_BIT_32				(3)




#endif // end of #ifndef _STAR_SPI_H_

