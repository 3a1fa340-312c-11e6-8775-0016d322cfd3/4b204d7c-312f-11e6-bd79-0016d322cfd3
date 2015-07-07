#ifndef _SETUP_UART_H_
#define _SETUP_UART_H_

//=============================================================================
//                                DEFINITIONS
//=============================================================================
#define FAIL	        0
#define OK	            1

#define	bit(x)			(0x0001<<x)

//=============================================================================
//                    PUBLIC PROCEDURES (ANSI Prototypes)
//=============================================================================
/******************************************************************************
 *
 * Name: get_char
 *
 * Description:
 *   Return the next char available on UART receive. If no char is
 *   ready, it will wait. Should no char still come after a timeout
 *   passes, then return back to the   uart_boot() code and NAK the
 *   packet. 
 *
 * Conditions For Use:
 *   GPIO Clock should be enabled before attempting to use. (In PAU).
 *
 * Arguments:
 *   Arg1( u16 ): 16 bits of data to appear at the pins.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
int get_char(void);

/******************************************************************************
 *
 * Name: put_char
 *
 * Description:
 *   Put a char into the UART transmit buffer. Wait until that buffer
 *   is ready. No timeout here, assume we can always be ready to send
 *   after a wait. 
 *
 * Conditions For Use:
 *   GPIO Clock should be enabled before attempting to use. (In PAU).
 *
 * Arguments:
 *   Arg1( u8 ): 8 bits of data to be placed in the 16550 xmit buffer.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
int put_char (int u8c);

/******************************************************************************
 *
 * Name: setup_uart
 *
 * Description:
 *   Place the 16550 into a useable state for us. This means the the
 *   baud rate needs to be 38400, no parity, one stop bit. Also the
 *   pad mux needs to get flipped from the default of USB to UART. 
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
extern void setup_uart( void );

//quick watch for the in and out byte functions:
void outbyte(int c);
int inbyte(void);

#endif /* _SETUP_UART_H_ */
