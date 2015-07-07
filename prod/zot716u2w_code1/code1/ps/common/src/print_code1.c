/*      Copyright Motorola, Inc. 1993, 1994
        ALL RIGHTS RESERVED

        You are hereby granted a copyright license to use, modify, and
        distribute the SOFTWARE so long as this entire notice is retained
        without alteration in any modified and/or redistributed versions,
        and that such modified versions are clearly identified as such.
        No licenses are granted by implication, estoppel or otherwise under
        any patents or trademarks of Motorola, Inc.

        The SOFTWARE is provided on an "AS IS" basis and without warranty.
        To the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS
        ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED
        WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
        PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
        REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS
        THEREOF) AND ANY ACCOMPANYING WRITTEN MATERIALS.

        To the maximum extent permitted by applicable law, IN NO EVENT SHALL
        MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER
        (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF
        BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
        INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR
        INABILITY TO USE THE SOFTWARE.   Motorola assumes no responsibility
        for the maintenance and support of the SOFTWARE.  */

#include <ctype.h>
#include <stdarg.h>
#include <string.h>


#define va_get(vvval,vvvty)     vvval = va_arg(ap,vvvty);

#define LF 10			/* line feed */
#define CR 13			/* carriage return */
#define FALSE 0
#define TRUE 1
#define TAB '\t'		/* tab character */
#define TABCOUNT 8		/* number of spaces for tab conversion */
#define WIDE_COLS 132    
#ifdef ON_BOARD
#endif


/* ---------------------------------------------------- */
/* FUNC:   tab_convert()
 * DESC:   convert tabs into spaces for output
 * INPUT:  chp - possible tab character
 * RETURN: error = FALSE
 */

tab_convert(chp)
char chp;
{
static int tab_count;
int i;
int error = FALSE;
	if(chp == '\t') 
	{
		for(i=tab_count; i>0 && !error; i--)
			put_char(' ');
		tab_count = TABCOUNT;
	}
	else
	{
	  tab_count--;
	  if(!tab_count || chp == CR || chp == LF || chp == '\n') 
		tab_count = TABCOUNT;
	};
  return(error);
}


/* FUNCTION   write_char
 * DESC:  Writes out single characters to the console
 *
 * INPUTS:  ch - an ascci character
 * RETURNS: ch - character written
 */
char write_char(ch) 
char ch;

{
switch(ch)
	{
	
	case '\n': 				
		{
		put_char(CR);     /* cr */
	  	put_char(LF);     /* lf */
	  	tab_convert(ch);
	  	break;
	  	}
	case '\t':
		{
		tab_convert(ch);	/* tab */
		break;
		}
	default: 
		put_char(ch);
	};
return(ch);
}
		
#if 1
/* FUNC:   is_digit()
 * DESC:   determine if character is a digit
 * INPUT:  ch - possible digit character
 * RETURN: 1 if digit; 0 if not
 */

int is_digit(char ch) {
	if ( (ch>='0') && (ch <='9') ) return 1;
	return 0; 
}

/* FUNC:   is_space()
 * DESC:   determine if character is a space (or equivalent)
 * INPUT:  ch - possible space character
 * RETURN: 1 if space; 0 if not
 */

int is_space(ch)
char ch;
{
	if ( ch == ' ') return 1;
	if ( ch == '\n' ) return 1;
	if ( ch == '\r' ) return 1;
	return 0;
}
#endif
/* FUNC:   convert_to_int
 * DESC:   Skip spaces; set sign of result; convert ascii digits to integer
 * INPUT:  s - a string
 * RETURN: a signed integer
*/		
convert_to_int(s) unsigned char *s;
{
int sign, n;
        if ( s == 0 )
	{
		return 0;
	}
	while (is_space(*s)) ++s;
	sign = 1;
	switch(*s)
		{ 
		case '-': sign = -1;
		case '+': ++s;
		};
	n = 0;
	while(is_digit(*s)) n = 10 * n + *s++ - '0';
	return (sign * n);
}

/* FUNC:     reverse
 * DESC:     reverse string
 * INPUTS:   s - address of sting to be reversed
 * RETURNS:  nothing
 */
reverse(s)
char *s;
{
char *j;
int c;
        if ( s == 0 )
	{
		return 0;
	}
	j = s + strlen(s) - 1;
	while(s<j)
	{
		c = *s;
		*s++ = *j;
		*j-- = c;
	};
}

/* FUNC:   convert_char_to_string
 * DESC:   convert n to (at least precision) characters in s
 * INPUT:  n - a signed integer
 *         s - address of a string to be returned
 *         precision - minimum number of digits (pad with 0 as necessary)
 * RETURN: s - address of resulting string
 */
convert_char_to_string_prec(n,s,precision)
unsigned char *s;
int n,precision;
{
int sign;
unsigned char *ptr;
	if ( s == 0 )
	{
		return 0;
	}
	ptr = s;
	if ((sign = n) < 0)
	     n = -n;   /* save sign and make n positive */
	do
		{
		*ptr++ = n % 10 + '0';  /* Save n modulo 10 as ascii char */
		--precision;		
		}
		while ((n = n /10) > 0);  /* Continue 'til n <= 0 */
	while(precision>0)
	    {
	       *ptr = '0';
	       ptr ++;
		--precision;		
	     }
	if (sign <0) *ptr++ = '-';  /* If negative, prepend a minus sign */
	*ptr = '\0';                /* Last character is the NULL char */
	reverse(s);		/* Reverse the resulting string  */
}

/* FUNC:   convert_char_to_string
 * DESC:   convert n to characters in s  (itoa)
 *           (This is the original function before adding precision.
 *           Left here for compatibility with any previous callers.
 *           CJC 991031) 
 * INPUT:  n - a signed integer
 *         s - address of a string to be returned
 * RETURN: s - address of resulting string
 */
convert_char_to_string(n,s)
unsigned char *s;
int n;
{
   convert_char_to_string_prec(n,s,0);
}



/* FUNC:     convert_unsigned_char_prec
 * DESC:     convert unsigned n to at least precision characters
 *           in s using base b (itoab) using abcdef or ABCDEF for hex
 * INPUTS:   n - an unsigned integer number
 *           s - a string pointer
 *           b - the number base (hex, decimal, octal, binary)
 *           caps - 0 for hex abcdef or 1 for hex ABCDEF
 *           precision - minimum number of digits (pad with 0 as necessary)
 * RETURNS:  s - address of result string
 */
convert_unsigned_char_prec(n,s,b,caps,precision) 
unsigned int n;
unsigned char * s;
int b,caps,precision;
{

unsigned int tmp;
unsigned char *ptr;
        if ( s == 0 )
	{
		return 0;
	}
	ptr = s;
	do
	{
		tmp = n % b;     /* n modulo b will get next least significant digit  */
		n /= b;          /* reduce n by base */

		if ( tmp <=9 )   /* For b != 16  */ 
			*ptr = '0' + tmp;  /* convert to ascii 0-9  */
		else if (tmp>9 && caps == 0)           
			*ptr = 'a' + tmp - 10;  /* if hex and > 9, convert to a-f */
		     else if (tmp>9 && caps == 1)
			*ptr = 'A' + tmp - 10;  /* if hex and > 9, convert to A-F */		     
		ptr ++;
		--precision;		
	}
	while(n >0 );  /* End of do loop  */
	while(precision>0)
	    {
	       *ptr = '0';
	       ptr ++;
		--precision;		
	     }
	*ptr = 0;      /* End string with NULL  */
	reverse(s);    /* Reverse the string to put Most Significant first. */
}

/* FUNC:     convert_unsigned_char
 * DESC:     convert unsigned n to characters in s using base b (itoab)
 *           (This is the original function before adding precision and
 *           choice of capitalization.  Left here for compatibility with
 *           any previous callers.  CJC 991031  
 * INPUTS:   n - an unsigned integer number
 *           s - a string pointer
 *           b - the number base (hex, decimal, octal, binary)
 * RETURNS:  s - address of result string
 */
convert_unsigned_char(n,s,b) 
unsigned int n;
unsigned char *s;
int b;
{
   convert_unsigned_char_prec(n,s,b,0,0);
}

/* FUNC:     convert_float_char
 * DESC:     convert double float to characters in s
 * INPUTS:   f - floating point number
 *           s - address of return sting
 *           sig_digits - desired number of digits right of decimal
 * RETURNS:  s - address of return string
 */
 
convert_float_char(f,s,sig_digits) 
double f;
char *s;
int sig_digits;
{

unsigned int 	n;
	double 	next_digit = 1;
	char 	*ptr;
	ptr = s;
	if (f < 0)          /* If float is negative, */
	{	f = - f;    /* convert to positive and  */
		*ptr = '-';  /* save the sign  */
		ptr ++;      /* Advance the temporary string pointer */
		s ++;}       /* Advance the real string pointer */
	n = (int) f;         /* Convert the integer part to an integer  */
	f -= (double) n;	/* Save the fraction as a float */
	convert_unsigned_char(n,s,10);  /* Convert the integer to a string */
	ptr = ptr + strlen(s);   /* Advance the temporary ptr by the length of integer*/
	if (sig_digits) *ptr = '.';   /* Add a decimal point if prec > 0 */
	ptr ++;       /* Move to next character */
	while (sig_digits)  /* Add significant digits right of decimal */
	{
		next_digit *= 10.0;     
		n = (int) (f * next_digit);
		*ptr = '0' + n;
		ptr ++;
		f -= ((double) n )/ next_digit;
		sig_digits--;
	}
	*ptr = 0;
}

/* FUNC:     *strcpy2
 * DESC:     string copy
 * INPUTS:   s1 - address of destination string
 *           s2 - address of sting to be copied
 * RETURNS:  address of destination string
 */
char *strcpy2(s1,s2)
char *s1, *s2;
{
                                /* this is so that we can return the proper
                                value to the caller when we exit */
char *retvalue;


                                /* it just so happens that s1 is the proper
                                return value */
        retvalue = s1;


                                /* now lets loop on s2 until *s2 is equal to
                                '\0' and in this loop we need to copy the
                                value from s2 into s1 and increment both
                                pointers. */
        while( *s2 != '\0' )
        {
                *s1=*s2;
                s1 ++; s2 ++;
        }


                                /* since we terminated the loop on detecting
                                the NULL char without copying the character


                               we need to place '\0' on s1 so that we have
                                a proper C-string in s1 */
        *s1 = '\0';


                                /* now lets return to the caller what he/she
                                needs */
        return retvalue;
}

/*--------------------------------------------------------------
		PRINTF ROUTINE
----------------------------------------------------------------*/
/* FUNC:     dink_printf
 * DESC:     dink's printf routine
 *           interprets format conversions: 
 *           %c, %d, %e, %E, %f, %g, %G, %i, %o, %p, %s, %u, %x, %X, %%
 *           and flags (zero or more may be present):
 *           -, +, 0, #, or space where
 *              -       left justify
 *              +       always display a sign, negative or positive
 *              0       pad with leading zeroes. Ignore if a - flag is
 *                      present or, if for an integer form,
 *                      precision is specified.
 *              #       precede octal with 0 and hex with 0x or 0X
 *                     (depending on %x or %X).  Guarantees a decimal point
 *                      for floating point forms.
 *              space   signed values are displayed with a leading space
 *                      (but no sign) if positive and with a minus sign
 *                      if negative.  A space overrides a + flag.
 *           for fp format specifications of form digit(s).digit(s) or x.y
 *           (e.g. %6.2f) internal variables are:
 *             width = x (e.g. 6) or length needed to represent a number
 *             fprec = y (e.g. 2) or number of digits to right of decimal point
 *           for integer conversions of form digit(s).digit(s) or x.y
 *           (e.g. %6.2i or %6.2d) internal variables are:
 *                width = x (e.g. 6) or number of character positions to be printed
 *                prec = y (e.g. 2) or minimum number of digits to be printed
 *                  (pad with leading zero(es) if necessary)
 *           for string conversions of form digit(s).digit(s) or x.y
 *           (e.g. %6.2s) internal variables are:
 *                width = x (e.g. 6) or number of character positions to be printed
 *                prec = y (e.g. 2) or maximum number of characters to be printed
 * INPUTS:   fmt - format string
 *           ... - variable list of arguments
 * RETURNS:  number of characters printed (cc)
 */
//#define debug
#undef debug

#ifndef debug
//ZOTIPS int armond_printf(const char *fmt, ...)		/* begin routine */
//ZOTIPS{

//ZOTIPS}
#else
//ZOTIPS int armond_printf(const char *fmt, ...)		/* begin routine */

{
    va_list ap;

int temp, left, pad, cc, len, maxchr, width, always_sign, format_conv, plus_sign_blank, i,exp;
char *fmt_string_pointer;
char *string_arg, *cptr, *ptr_estr, str[30],estr[30],exp_str[5];
unsigned int fprec, base, caps, efg_flag, saved_sign, prec, too_big;
double temp_float;
    if ( fmt == 0 )
    {
	    return 0;
    }

va_start( ap, fmt );
    
fmt_string_pointer = (char *)fmt;		/* set "format" to the format string */
cc = 0;
temp = 0;
string_arg = 0;
while (*fmt_string_pointer != '\0' )
{
        string_arg = 0;
        left = 0;
	width = 0;
	maxchr = 0;
        str[0] = '\0';
        estr[0] = '\0';
        exp_str[0] = '\0';
	always_sign = 0;
	format_conv = 0;
	plus_sign_blank = 0;
	pad = ' ';
	cptr = 0;
	fprec = 6;
	base = 10;
	caps = 0;
	prec = 0;
	fprec = 6;
	efg_flag = 0;
	saved_sign = 0;


	if (*fmt_string_pointer != '%')
	{
          /* if not %, just print char */
	  write_char(*(fmt_string_pointer++)); /* write_char same as fputc*/
	  ++cc;
	  continue;
	}
	else
	{
           ++fmt_string_pointer;
        }
				/* this checks for two % next to each
				   other, it if finds one it loops */
	if ( *fmt_string_pointer != '\0' && *fmt_string_pointer == '%')
	{
	   write_char(*(fmt_string_pointer++)); /*fmt_string_pointer = ctl */
       	   ++cc;
	   continue;
	}
        while (*fmt_string_pointer != '\0' &&
	      (*fmt_string_pointer == '-' | \
	       *fmt_string_pointer == '+' | \
	       *fmt_string_pointer == '#' | \
	       *fmt_string_pointer == '0' | \
	       *fmt_string_pointer == ' '  ))
	    {
	switch(*fmt_string_pointer)
	{
	   case  '-':  /* checking left */
		left = 1;
		break;
	   case '+':  /* checking always show sign */
		always_sign = 1;
		break;
	   case '#':  /* checking format conversion */
		format_conv = 1;
		break;
	   case '0': /* checking for zero padding */
		pad = '0';
		break;
	   case ' ':  /* checking for space where plus sign would go */
		plus_sign_blank = 1;	       
	}   /* End of switch(*fmt_string_pointer) */
	++fmt_string_pointer;
   }   /* End of while (*fmt_string_pointer == ... */
   
/*  Determine if width is specified as digit(s) or x  */
	if (*fmt_string_pointer != '\0' && is_digit(*fmt_string_pointer))
	{
		width = convert_to_int(fmt_string_pointer++);
		while(*fmt_string_pointer != '\0'&& is_digit(*fmt_string_pointer))
		  {
			++fmt_string_pointer;
	          }
	}
		
/*  Determine if precision is specified as .digit(s) or y  */	
	if (*fmt_string_pointer != '\0' && *fmt_string_pointer == '.')
		{
			fprec = prec = maxchr = convert_to_int(++fmt_string_pointer);
			while(*fmt_string_pointer != '\0' && is_digit(*fmt_string_pointer))
			{
				++fmt_string_pointer;
	          	}
		}

/*  Ignore %***lu conversion on PowerPC   */
	if ( *fmt_string_pointer != '\0' && *(fmt_string_pointer) == 'l' )
	{
	    fmt_string_pointer++;
	}

	if ( *fmt_string_pointer == '\0' )
	{
		continue;
	}

	switch( *(fmt_string_pointer) )
	{
		case 'c' : 	va_get(str[0] ,char);
		                str[1] = '\0';
		                string_arg = str;
			   	break;

		case 's' : 	va_get(string_arg, char *);


/* If this is a string and a maximum number of characters is specified,
 * and the string is longer than the maximum number, make the length
 * equal to the maximum number of characters. (Only way I could figure
 * out how to do that without destroying the original string was to make a
 * copy and then terminate it with a null at the appropriate place.  CJC 991031
 */	
				    if (maxchr && (maxchr < strlen(string_arg)))
			        {
			           ptr_estr = strcpy2(estr, string_arg);
			           for (i=1; i<=maxchr; i++)
			           {
			              ptr_estr ++;
			            }
			            *ptr_estr = '\0';
			            string_arg = estr;
			        }
			        
			   	    break;
		case 'u' :  	
		            va_get(temp,unsigned long);
				    convert_unsigned_char_prec(temp,str,10,0,prec);
				    string_arg=str; 
			   	    break;
		case 'i' : 	
		case 'd' :	
		            va_get(temp,long);
		            if (temp < 0)
			        {
			           saved_sign = 1;
			           temp = - temp;
			         }
				     convert_unsigned_char_prec(temp,str,10,0,prec);
			         string_arg=str;
			         break;
		case 'x' : 	 
		             va_get(temp,unsigned long);
				convert_unsigned_char_prec(temp,str,16,0,prec);
				string_arg=str; 
			   	break;
		case 'X' : 	va_get(temp,unsigned long);
				convert_unsigned_char_prec(temp,str,16,1,prec);
				string_arg=str; 
			   	break;
		case 'o' : 	va_get(temp,unsigned long);
				convert_unsigned_char_prec(temp,str,8,0,prec);
				string_arg=str; 
			   	break;
		case 'p' : 	va_get(temp,unsigned long);
				convert_unsigned_char_prec(temp,str,16,0,prec);
			        pad = '0';
			        width = 8;
				string_arg=str; 
			   	break;

/* This is crude but works like this:
 *             |  efg_flag   |
 *   G flag    |    0x04     |   Use 'E' use smaller of %E or %f (if not too_big)
 *   g flag    |    0x03     |   Use 'e' use smaller of %E or %f (if not too_big)
 *   E flag    |    0x02     |   Use 'E'
 *   e flag    |    0x01     |   Use 'e'
 *   f flag    |    0x01     |   if fpno > max int, use 'e' & set too_big
 *   f flag    |    0x00     |   N/A
 */
 
		case 'G' :      efg_flag ++;   /* Use %E below instead of %e */
		case 'g' :      efg_flag ++;   /* Choose shortest of %e or %f */
        	case 'E' :      efg_flag ++;   /* Use %E below instead of %e */
        	case 'e' :      efg_flag ++;   /* Do %e conversion */
		case 'f' :	va_get(temp_float,double);
		                if (temp_float < 0)
			        {
			           saved_sign = 1;
			           temp_float = - temp_float;
			        }      
			        if (temp_float <= 2147483647)   /* Is fpno <= largest integer? */
				{  /* do a conversion to floating point  */
			           convert_float_char(temp_float,str,fprec);
			           string_arg = str;
			           too_big = 0;
			        }
			        else
			        {
			           too_big = 1;
			           if (efg_flag == 0)
			              efg_flag = 1; /* if this was %f, promote to %e format */
			           /* This would be a string longer than most scientific notations */
			           /* and would have the wrong value anyway! */
			        } 
			        if (efg_flag)   /* do a conversion to scientific notation */
			        {  /* for too big fp or %e, %E, %g, and %E  */
			           exp = 0;
			           while (temp_float > 10.0 )
			           {
				      exp ++;         /* move decimal left. */
			              temp_float /= 10.0;
			           }
			           while (temp_float < 0.0 )
			           {
				      exp --;         /* move decimal right. */
			              temp_float *= 10.0;
			           }
			           convert_float_char(temp_float,estr,fprec);
			           len = strlen(estr);
			           ptr_estr = estr + len;
			           if ((efg_flag == 2) || (efg_flag == 4))   /* if G or E,*/
				        *ptr_estr ++ = 'E';  /* use 'E' */
			           else
			                *ptr_estr ++ = 'e'; /* otherwise use 'e' */
			           if (exp >= 0)
			              *ptr_estr ++ = '+';	           
			           convert_char_to_string_prec(exp,exp_str,2);
			           cptr = exp_str;
				   while (*cptr != '\0')
			           {
			              *ptr_estr = *cptr;
			              ptr_estr ++;
			              cptr ++;
			            }
			            *ptr_estr = '\0';
			        }
			        if (efg_flag > 2) /* if this is %g or %G  */
			      	   if (!too_big && strlen(estr) > strlen(str)) /* and string with scientic notation > %f */
			              string_arg = str;   /*  print %f format  */
			           else
			              string_arg = estr;  /* else print scientific notation format */   
			        else if (efg_flag >0)     /* else for the %e and %E format */
			                string_arg = estr;   /* print the scientific notation  */
				break;
		default : 	temp = *(fmt_string_pointer);
		                cptr = string_arg;
			        *cptr++ = temp;
			        *cptr = '\0';			        
	};  /* End of switch( *(fmt_string_pointer) )  */


	len = strlen(string_arg);  /* How long is the ascii string?  */
	

/* If the ascii won't fit in width or no width is specified,
 * make the number of pad characters to be used zero.  Otherwise
 * pad to width - len with pad character (possibly less plus sign space).
 */
	if(width>len) width = width - len; else width = 0;

/* If this was a negative number, we have saved the sign as saved_sign.
 * If a + flag was specified and the number is not negative,
 * then output a plus sign.  If a space flag was specified and the number is
 * not negative put a leading space out.  Deduct a plus sign or space from
 * required padding.
 */
	if(saved_sign || plus_sign_blank || always_sign)
			--width;   /* subtract one from padding */
			
/* If this is a hex or octal number and the format conversion flag (#) was
 * specified, we need to output the "0", "0x", or "0X" appropriate to the
 * conversion specifier.
 */

	if (*(fmt_string_pointer) == 'o' && format_conv)
			--width;   /* subtract one from padding */
	if (((*(fmt_string_pointer) == 'x') || \
	     (*(fmt_string_pointer) == 'X')) && format_conv )
			width -= 2;   /* subtract two from padding */			
	if (*(fmt_string_pointer) == 'f' && format_conv && fprec == 0)
			--width;   /* subtract one from padding for decimal */

	if(!left && pad == ' ') while(width-- > 0)
		{
			write_char(pad);
			++cc;
		}
		
	if(saved_sign)
			write_char('-');
	else if (plus_sign_blank)
			write_char(' ');
	     else if (always_sign)
			write_char('+');
			
			
	if (*(fmt_string_pointer) == 'o' && format_conv )
			write_char('0');
	if (*(fmt_string_pointer) == 'x' && format_conv )
		  {
			write_char('0');
			write_char('x');
		  }
	if (*(fmt_string_pointer) == 'X' && format_conv )
		  {
			write_char('0');
			write_char('X');
		  }

	if ((*(fmt_string_pointer) == 'd' || \
	    *(fmt_string_pointer) == 'i') && \
	    prec > len && pad == '0')
			width = prec;   /* pad to precision */

	if(!left && pad == '0') while (width-- > 0)
		{
			write_char(pad);
			++cc;
		}

	while (len-- > 0) {
			write_char(*string_arg);
		string_arg++;
		++cc;
	}
		
	if (*(fmt_string_pointer) == 'f' && format_conv && fprec == 0)
			write_char('.');

	if(left) 
        {
            while(width-- > 0) 
	    {
	     write_char(pad);
	     ++cc;
	    }
        }
	fmt_string_pointer++;  /* point to next character of format string */	
   }		/* return while */

   va_end( ap );

   return(cc);
}
#endif debug		

