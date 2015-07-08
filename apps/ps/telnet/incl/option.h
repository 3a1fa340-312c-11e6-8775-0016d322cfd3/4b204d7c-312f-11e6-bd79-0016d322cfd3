
/* +++=>>> * %n  Edition: %v  Last changed: %f  By: %w */
/* TELNET.H  Edition: 2  Last changed: 26-Jan-96,11:55:08  By: HARRY */
/* +++=>>> '%l' */
/* Currently extracted for edit by: '***_NOBODY_***' */
/*
    TELNET.H -- Telnet Definitions for USNET

    Copyright (C) 1993 By
    United States Software Corporation
    14215 N.W. Science Park Drive
    Portland, Oregon 97229

    This software is furnished under a license and may be used
    and copied only in accordance with the terms of such license
    and with the inclusion of the above copyright notice.
    This software or any other copies thereof may not be provided
    or otherwise made available to any other person.  No title to
    and ownership of the software is hereby transferred.

    The information in this software is subject to change without
    notice and should not be construed as a commitment by United
    States Software Corporation.
*/

#ifndef _INC_OPTION_H_
#define _INC_OPTION_H_
/* Network Virtual Terminal(NVT) cotrol codes */

#define IAC     255     /* interpret as controls  (=escape) */

/* command code */

#define SE      240     /* end of subnegotiation */
#define SB      250     /* subnegotiation */
#define WILL    251     /* I will do */
#define WONT    252     /* I refuse */
#define DO      253     /* please do */
#define DONT    254     /* please stop doing */

/* option codes */

#define ECHO            1       /* echo */
#define SUPPRESSGOAHEAD 3       /* suppress go-ahead */
#define STAT            5
#define TERMTYPE        24      /* terminal type */
#define EOROPT          25      /* end of record */

/* suboption codes */

#define ASCII_TERM      1       /* ASCII terminal emulator */
#define SEND            1       /* send */

#endif
