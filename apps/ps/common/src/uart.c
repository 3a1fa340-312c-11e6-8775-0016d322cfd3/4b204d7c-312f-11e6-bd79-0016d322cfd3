#include <stdio.h>
#include <stdlib.h>
//=============================================================================
//                               INCLUDE FILES
//=============================================================================


//=============================================================================
//                         CODED PUBLIC PROCEDURES
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
 *   Must be run with cache off or NAK_TIMEOUT loop will not be 10sec.
 *   Requires ARM ADS1.2 Thumb compiler. Again, timeout loop sensitivity.
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


int get_char( void ) 
{}


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

int put_char(int u8c)
{}

void outbyte(int c)
{}


int inbyte(void)
{}



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
extern void 
setup_uart( void )
{}

#ifdef UART_OUTPUT
char printk_buf[1024];
#endif

int printk(const char *fmt, ...)
{
#ifdef UART_OUTPUT 
    int n;

    va_list args;
    va_start(args, fmt);
    n = vsprintf(printk_buf, fmt, args);    
    va_end(args);

    serial_puts(printk_buf); 
#endif
    return 0;
}

