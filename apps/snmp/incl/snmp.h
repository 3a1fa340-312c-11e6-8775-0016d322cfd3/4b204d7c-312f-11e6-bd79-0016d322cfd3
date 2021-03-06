/*
 * Definitions for the Simple Network Management Protocol (RFC 1067).
 *
 *
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#ifndef SNMP_H
#define SNMP_H

#define SNMP_PORT	    161
#define SNMP_TRAP_PORT	162

#define SNMP_MESSAGE_LIFETIME 150

#define SNMP_VERSION_1	    0
#define SNMP_VERSION_2C	    1
#define SNMP_VERSION_2	    2

#define SNMP_USEC_MODEL	    1

#define GET_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x0)
#define GETNEXT_REQ_MSG	(ASN_CONTEXT | ASN_CONSTRUCTOR | 0x1)
#define GET_RSP_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x2)
#define SET_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x3)
#define TRP_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4)

#define GETBULK_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x5)
#define INFORM_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x6)
#define TRP2_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x7)
#define REPORT_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x8)

#define SNMP_NOSUCHOBJECT    (ASN_CONTEXT | ASN_PRIMITIVE | 0x0)
#define SNMP_NOSUCHINSTANCE  (ASN_CONTEXT | ASN_PRIMITIVE | 0x1)
#define SNMP_ENDOFMIBVIEW    (ASN_CONTEXT | ASN_PRIMITIVE | 0x2)

#define SNMP_ERR_NOERROR    		(0)
#define SNMP_ERR_TOOBIG	    		(1)
#define SNMP_ERR_NOSUCHNAME 		(2)
#define SNMP_ERR_BADVALUE   		(3)
#define SNMP_ERR_READONLY   		(4)
#define SNMP_ERR_GENERR	    		(5)

#define SNMP_ERR_NOACCESS               (6)
#define SNMP_ERR_WRONGTYPE              (7)
#define SNMP_ERR_WRONGLENGTH            (8)
#define SNMP_ERR_WRONGENCODING          (9)
#define SNMP_ERR_WRONGVALUE             (10)
#define SNMP_ERR_NOCREATION             (11)
#define SNMP_ERR_INCONSISTENTVALUE      (12)
#define SNMP_ERR_RESOURCEUNAVAILABLE    (13)
#define SNMP_ERR_COMMITFAILED           (14)
#define SNMP_ERR_UNDOFAILED             (15)
#define SNMP_ERR_AUTHORIZATIONERROR     (16)
#define SNMP_ERR_NOTWRITABLE            (17)
#define SNMP_ERR_INCONSISTENTNAME	(18)

#define SNMP_TRAP_COLDSTART		(0x0)
#define SNMP_TRAP_WARMSTART		(0x1)
#define SNMP_TRAP_LINKDOWN		(0x2)
#define SNMP_TRAP_LINKUP		(0x3)
#define SNMP_TRAP_AUTHFAIL		(0x4)
#define SNMP_TRAP_EGPNEIGHBORLOSS	(0x5)
#define SNMP_TRAP_ENTERPRISESPECIFIC	(0x6)

#define USEC_QOS_AUTH			(0x01)
#define USEC_QOS_PRIV			(0x02)
#define USEC_QOS_AUTHPRIV		(0x03)
#define USEC_QOS_GENREPORT		(0x04)
#define USEC_QOS_NOAUTH_NOPRIV		(0)

#define SNMP_STAT_PACKETS			0
#define SNMP_STAT_ENCODING_ERRORS		1
#define USEC_STAT_UNSUPPORTED_QOS		2
#define USEC_STAT_NOT_IN_WINDOWS		3
#define USEC_STAT_UNKNOWN_USERNAMES		4
#define USEC_STAT_WRONG_DIGEST_VALUES		5
#define USEC_STAT_UNKNOWN_CONTEXT_SELECTORS	6
#define USEC_STAT_BAD_PARAMETERS		7
#define USEC_STAT_UNAUTHORIZED_OPERATIONS	8
#define SNMP_STAT_BAD_OPERATIONS		9
#define SNMP_STAT_PROXY_DROPS			10
#define SNMP_STAT_SILENT_DROPS			11
#define SNMP_STAT_V1_BAD_COMMUNITY_NAMES	12
#define SNMP_STAT_V1_BAD_COMMUNITY_USES		13

#define SNMP_LAST_STAT				SNMP_STAT_V1_BAD_COMMUNITY_USES

#ifdef PS_OUTPUT
#define ERROR(string)	printf("%s(%d): %s\n",__FILE__, __LINE__, string);
#else
#define ERROR(string)
#endif

#define bcopy(x,y,z)  memcpy((y),(x),(z))	 //move to swap.h 2/9/99
//#define ntohl(x)      DWordSwap((x))	//move to swap.h 2/9/99


uint8 *snmp_parse_var_op (uint8 *data,
					      oid   *var_name,
					      int   *var_name_len,
					      uint8 *var_val_type,
					      int   *var_val_len,
					      uint8 **var_val,
					      int   *listlength);

uint8 *snmp_build_var_op (uint8 *data,
					      oid   *var_name,
					      int   *var_name_len,
					      uint8 var_val_type,
					      int   var_val_len,
					      uint8 *var_val,
					      int   *listlength);

#endif
