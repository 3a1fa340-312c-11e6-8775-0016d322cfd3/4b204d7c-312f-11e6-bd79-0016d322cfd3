//------------------------------------------------------------------------
// Purpose : This Program will English & Symbol FONT change to Large Font.
//           (7 X 7).
//
// Change Font : 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&()_-
// Original Author : David Johnson. (from LPD Server Banner)
// Modifier        : James Pu. (Zero One Tech Co.)
//                 : Simon Hung 2/19/98
// Revision        : 95 July 20
//------------------------------------------------------------------------

//
//#define BANNER_TEST  //Turn on Banner test routine, Simon 2/19/98
//

#include <stdio.h>
#include <string.h>

#include "banner.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
extern char * strupr ( char * string );

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////




#ifndef CODE1 //--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//

//
// large banner character definitions
//
#define MaxChar        9
#define LineLen       80  //add by Simon 2/19/98

#ifdef CONST_DATA
//move this table to constant.c 3/24/98
extern const char far CharTable[];
extern const char far (far lrgchars[])[7];
extern const char far BannerData[];

#else

#define c_______    0x00
#define c______1    0x01
#define c_____1_    0x02
#define c____1__    0x04
#define c____11_    0x06
#define c___1___    0x08
#define c___1__1    0x09
#define c___1_1_    0x0A
#define c___11__    0x0C

#define c__1____    0x10
#define c__1__1_    0x12
#define c__1_1__    0x14
#define c__11___    0x18
#define c__111__    0x1C
#define c__111_1    0x1D
#define c__1111_    0x1E
#define c__11111    0x1F

#define c_1_____    0x20
#define c_1____1    0x21
#define c_1___1_    0x22
#define c_1__1__    0x24
#define c_1__1_1    0x25
#define c_1_1___    0x28
#define c_1_1__1    0x29
#define c_1_1_1_    0x2A

#define c_11____    0x30
#define c_11_11_    0x36
#define c_111___    0x38
#define c_111__1    0x39
#define c_111_1_    0x3A
#define c_1111__    0x3C
#define c_1111_1    0x3D
#define c_11111_    0x3E
#define c_111111    0x3F

#define c1______    0x40
#define c1_____1    0x41
#define c1____1_    0x42
#define c1____11    0x43
#define c1___1__    0x44
#define c1___1_1    0x45
#define c1___11_    0x46
#define c1__1___    0x48
#define c1__1__1    0x49
#define c1__11_1    0x4D
#define c1__111_    0x4E
#define c1__1111    0x4F

#define c1_1____    0x50
#define c1_1___1    0x51
#define c1_1__1_    0x52
#define c1_1_1__    0x54
#define c1_1_1_1    0x55
#define c1_1_11_    0x56
#define c1_111__    0x5C
#define c1_1111_    0x5E

#define c11____1    0x61
#define c11___1_    0x62
#define c11___11    0x63
#define c11_1___    0x68
#define c11_1__1    0x69

#define c111_11_    0x76
#define c1111___    0x78
#define c11111__    0x7C
#define c111111_    0x7E
#define c1111111    0x7F


const char lrgchars[][7] = {

    { c_11111_,
      c1____11,
      c1___1_1,
      c1__1__1,
      c1_1___1,
      c11____1,
      c_11111_ },

    { c___1___,
      c__11___,
      c___1___,
      c___1___,
      c___1___,
      c___1___,
      c__111__ },

    { c_11111_,
      c1_____1,
      c______1,
      c_11111_,
      c1______,
      c1______,
      c1111111 },

    { c_11111_,
      c1_____1,
      c______1,
      c_11111_,
      c______1,
      c1_____1,
      c_11111_ },

    { c____11_,
      c___1_1_,
      c__1__1_,
      c_1___1_,
      c1____1_,
      c1111111,
      c_____1_ },

    { c1111111,
      c1______,
      c1______,
      c111111_,
      c______1,
      c1_____1,
      c_11111_ },

    { c_11111_,
      c1______,
      c1______,
      c111111_,
      c1_____1,
      c1_____1,
      c_11111_ },

    { c1111111,
      c1_____1,
      c_____1_,
      c____1__,
      c___1___,
      c___1___,
      c___1___ },

    { c_11111_,
      c1_____1,
      c1_____1,
      c_11111_,
      c1_____1,
      c1_____1,
      c_11111_ },

    { c_11111_,
      c1_____1,
      c1_____1,
      c_111111,
      c______1,
      c1_____1,
      c_1111__ },

    { c__111__,
      c_1___1_,
      c1_____1,
      c1_____1,
      c1111111,
      c1_____1,
      c1_____1 },

    { c111111_,
      c_1____1,
      c_1____1,
      c_11111_,
      c_1____1,
      c_1____1,
      c111111_ },

    { c__1111_,
      c_1____1,
      c1______,
      c1______,
      c1______,
      c_1____1,
      c__1111_ },

    { c11111__,
      c_1___1_,
      c_1____1,
      c_1____1,
      c_1____1,
      c_1___1_,
      c11111__ },

    { c1111111,
      c1______,
      c1______,
      c111111_,
      c1______,
      c1______,
      c1111111 },

    { c1111111,
      c1______,
      c1______,
      c111111_,
      c1______,
      c1______,
      c1______ },

    { c__1111_,
      c_1____1,
      c1______,
      c1__1111,
      c1_____1,
      c_1____1,
      c__1111_ },

    { c1_____1,
      c1_____1,
      c1_____1,
      c1111111,
      c1_____1,
      c1_____1,
      c1_____1 },

    { c_11111_,
      c___1___,
      c___1___,
      c___1___,
      c___1___,
      c___1___,
      c_11111_ },

    { c__11111,
      c____1__,
      c____1__,
      c____1__,
      c____1__,
      c1___1__,
      c_111___ },

    { c1_____1,
      c1____1_,
      c1___1__,
      c1__1___,
      c1_1_1__,
      c11___1_,
      c1_____1 },

    { c1______,
      c1______,
      c1______,
      c1______,
      c1______,
      c1______,
      c1111111 },

    { c1_____1,
      c11___11,
      c1_1_1_1,
      c1__1__1,
      c1_____1,
      c1_____1,
      c1_____1 },

    { c1_____1,
      c11____1,
      c1_1___1,
      c1__1__1,
      c1___1_1,
      c1____11,
      c1_____1 },

    { c__111__,
      c_1___1_,
      c1_____1,
      c1_____1,
      c1_____1,
      c_1___1_,
      c__111__ },

    { c111111_,
      c1_____1,
      c1_____1,
      c111111_,
      c1______,
      c1______,
      c1______ },

    { c__111__,
      c_1___1_,
      c1_____1,
      c1__1__1,
      c1___1_1,
      c_1___1_,
      c__111_1 },

    { c111111_,
      c1_____1,
      c1_____1,
      c111111_,
      c1___1__,
      c1____1_,
      c1_____1 },

    { c_11111_,
      c1_____1,
      c1______,
      c_11111_,
      c______1,
      c1_____1,
      c_11111_ },

    { c1111111,
      c___1___,
      c___1___,
      c___1___,
      c___1___,
      c___1___,
      c___1___ },

    { c1_____1,
      c1_____1,
      c1_____1,
      c1_____1,
      c1_____1,
      c1_____1,
      c_11111_ },

    { c1_____1,
      c1_____1,
      c1_____1,
      c1_____1,
      c_1___1_,
      c__1_1__,
      c___1___ },

    { c1_____1,
      c1_____1,
      c1_____1,
      c1__1__1,
      c1_1_1_1,
      c11___11,
      c1_____1 },

    { c1_____1,
      c_1___1_,
      c__1_1__,
      c___1___,
      c__1_1__,
      c_1___1_,
      c1_____1 },

    { c1_____1,
      c1_____1,
      c_1___1_,
      c__1_1__,
      c___1___,
      c___1___,
      c___1___ },

    { c1111111,
      c_____1_,
      c____1__,
      c___1___,
      c__1____,
      c_1_____,
      c1111111 },

    { c__11___,
      c__11___,
      c__11___,
      c__11___,
      c_______,
      c__11___,
      c__11___ },

    { c__1111_,
      c_1____1,
      c1__11_1,
      c1_1_1_1,
      c1__111_,
      c_1____1,
      c__1111_ },

    { c__1_1__,
      c__1_1__,
      c1111111,
      c__1_1__,
      c1111111,
      c__1_1__,
      c__1_1__ },

    { c___1___,
      c_111111,
      c1__1___,
      c_11111_,
      c___1__1,
      c111111_,
      c___1___ },

    { c_1____1,
      c1_1__1_,
      c_1__1__,
      c___1___,
      c__1__1_,
      c_1__1_1,
      c1____1_ },

    { c___1___,
      c__1_1__,
      c_1___1_,
      c1_____1,
      c_______,
      c_______,
      c_______ },

    { c_111___,
      c1___1__,
      c_1_1___,
      c__1____,
      c_1_1__1,
      c1___11_,
      c_111__1 },

    { c____1__,
      c___1___,
      c__1____,
      c__1____,
      c__1____,
      c___1___,
      c____1__ },

    { c__1____,
      c___1___,
      c____1__,
      c____1__,
      c____1__,
      c___1___,
      c__1____ },

    { c_______,
      c_______,
      c_______,
      c_______,
      c_______,
      c_______,
      c1111111 },

    { c_______,
      c_______,
      c_______,
      c1111111,
      c_______,
      c_______,
      c_______ },

};

const char BannerData[] = "\
*******************************************************************************\r\n\
*          > >>> >>>>>>>>>>    Print Banner    <<<<<<<<<<< <<< <              *\r\n\
*                                                                             *\r\n\
*     User Name   : %-13s                                             *\r\n\
*     Form Name   : %-16s                                          *\r\n\
*     Description : %-50s        *\r\n\
*                                                                             *\r\n\
*     Date :  %02d/%02d, %d       Time :  %2d:%02d:%02d %s                           *\r\n\
*******************************************************************************\r\n\r\n";

//
// Print a line of LARGE characters for the specified string
//
const char CharTable[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&()_-";

#endif CONST_DATA

char *print_large(char *BannerBuffer, char *string, char IsEndText)
{
	int scanline, scancol, not_empty ,nc,charlen ,space, i;
	char bits, *sp ;

	strupr (string);
	charlen = strlen(string);          // let Font set to center
	space = 0;

	if(charlen > MaxChar) charlen = MaxChar;
	space = (LineLen - (charlen * 8) )/2;

	for( scanline = 0; scanline < 7; scanline++ ) {
		not_empty = 0;
		sp = string;
		nc = 0;

		for (i=0;i<space;i++) *BannerBuffer++ = ' ';  // Add Space

		for(i=0 ; i < charlen; i++) {
			nc = 0;
			// don't overflow buffer
			while(CharTable[nc]!=*sp) nc++;

			bits = lrgchars[nc][scanline];
			if( bits ) not_empty++;

			for(scancol= 0;scancol < 7; scancol++) {
				*BannerBuffer++ = bits & 0x40 ? CharTable[nc] : ' ';
				bits = bits << 1;
			}

			*BannerBuffer++ = ' ';    // add one space
			sp++;
		}

		*BannerBuffer++ = '\r';
		*BannerBuffer++ = '\n';

	}
	*BannerBuffer++ = '\r';
	*BannerBuffer++ = '\n';
	if(IsEndText) {
		memset(BannerBuffer,'*',37);
		BannerBuffer+= 37;
		strcpy(BannerBuffer," O K ");
		BannerBuffer+= 5;
		memset(BannerBuffer,'*',37);
		BannerBuffer+= 37;
//        sprintf(BannerBuffer[strlen(BannerBuffer)],"\r\n%c", 0x0C);
		*BannerBuffer++ = '\r';
		*BannerBuffer++ = '\n';
		*BannerBuffer++ = 0x0C;
	}
	return BannerBuffer;
}

#ifdef BANNER_TEST
char BannerBuffer[2048];
void main (void)
{
	char STRING[20];

	sprintf(BannerBuffer, BannerData,
		    "1234567890123","1234567890123456",
		 "12345678901234567890123456789012345678901234567890",
		     2,19,1998,13,1,1,"PM");
	printf("%s%d\n",BannerBuffer,strlen(BannerBuffer));
	print_large(BannerBuffer+strlen(BannerBuffer),"#2-19-98#");
	printf("%s%d\n",BannerBuffer, strlen(BannerBuffer));
	getch ();
}
#endif

#endif !CODE1 //--//--//--//--//--//--//--//--//--//--//--//--//--//--//--//
