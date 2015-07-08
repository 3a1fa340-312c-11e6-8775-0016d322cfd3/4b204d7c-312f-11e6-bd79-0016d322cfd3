/*
 * "$Id: ipp.h,v 1.30 2000/06/08 19:56:38 mike Exp $"
 *
 *	 Internet Printing Protocol	definitions	for	the	Common UNIX	Printing
 *	 System	(CUPS).
 *
 *	 Copyright 1997-2000 by	Easy Software Products.
 *
 *	 These coded instructions, statements, and computer	programs are the
 *	 property of Easy Software Products	and	are	protected by Federal
 *	 copyright law.	 Distribution and use rights are outlined in the file
 *	 "LICENSE.txt" which should	have been included with	this file.	If this
 *	 file is missing or	damaged	please contact Easy	Software Products
 *	 at:
 *
 *		 Attn: CUPS	Licensing Information
 *		 Easy Software Products
 *		 44141 Airport View	Drive, Suite 204
 *		 Hollywood,	Maryland 20636-3111	USA
 *
 *		 Voice:	(301) 373-9603
 *		 EMail:	cups-info@cups.org
 *		   WWW:	http://www.cups.org
 */

#ifndef	_IPPD_H_
#define	_IPPD_H_

// Form FILE.H of ARM9PS
struct papfile {
	int			pf_state;
//    struct state	*pf_xstate;
	int			pf_len;
	char		*pf_buf;
	char		*pf_cur;
	char		*pf_end;
};

#define IPP_MAX_USER                3	  //8/16/2000 move to ZOT719.H

#define IPP_PRINT_JOB               0x002
#define IPP_VALIDATE_JOB            0x004
#define IPP_CANCEL_JOB	            0x008
#define IPP_GET_JOB_ATTRIBUTES      0x009
#define	IPP_GET_JOBS                0x00A
#define IPP_GET_PRINTER_ATTRIBUTES	0x00B
#define IPP_PRINT_JOB_ERROR         0x07E
#define IPP_PRINT_JOB_TIME_OUT      0x07F

#define IPP_PAUSE_PRINTER           0x010
#define IPP_RESUME_PRINTER          0x011
#define IPP_PURGE_JOBS              0x012

#define IPP_MAX_ATTRIB_NAME	        80
#define	IPP_TEMP_BUF_SIZE	        50

#define IPP_DEFAULT_USER_NAME       "anonymous"
#define IPP_DEFAULT_JOB_NAME        "Untitled"

#define	IPP_DEFAULT_DOC_FORMAT      "application/octet-stream"
#define	IPP_SECOND_DOC_FORMAT       "text/plain"
#define	IPP_DEFAULT_CHARSET	        "utf-8"
#define	IPP_SECOND_CHARSET	        "us-ascii"

#define	IPP_DEFAULT_LANGUAGE        "en-us"

#define	IPP_BIN_PRINTER_URI_FORMAT "lpx"
#define IPP_TXT_PRINTER_EXT_FORMAT "_txt"
#define	IPP_PRINTER_PORT_POS        2
#define	IPP_MAX_PRINTER_URI		   sizeof(IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT)

#define	IPP_JOB_URI_FORMAT	   "/job%03d"
#define	IPP_JOB_NUM_POS        4
#define	IPP_MAX_JOB_URI		   IPP_MAX_PRINTER_URI+sizeof(IPP_JOB_URI_FORMAT)+1

typedef	struct papfile ippBuf;

#define IPP_JOB_PENDING      0x03
#define	IPP_JOB_PROCESS		 0x05
#define	IPP_JOB_CANCEL		 0x07
#define	IPP_JOB_COMPLETE	 0x09
#define	IPP_JOB_ABORTING	 0x80
#define	IPP_JOB_NOT_COMPLETE (IPP_JOB_COMPLETE & 0x80)

#define IPP_BINARY_DOC       0x01
#define IPP_TEXT_DOC         0x02

#define IPP_FIND_ERR         (-1)
#define IPP_FIND_NOT_FOUND   (-2)
#define IPP_FIND_NO_VALUE    (-3)


//*
//*	Types and structures...
//*

typedef	enum
{
	IPP_TAG_ZERO = 0x00,
	IPP_TAG_OPERATION,
	IPP_TAG_JOB,
	IPP_TAG_END,
	IPP_TAG_PRINTER,
	IPP_TAG_UNSUPPORTED_GROUP,
	IPP_TAG_UNSUPPORTED_VALUE =	0x10,
	IPP_TAG_DEFAULT,
	IPP_TAG_UNKNOWN,
	IPP_TAG_NOVALUE,
	IPP_TAG_NOTSETTABLE	= 0x15,
	IPP_TAG_DELETEATTR,
	IPP_TAG_ANYVALUE,
	IPP_TAG_INTEGER	= 0x21,
	IPP_TAG_BOOLEAN,
	IPP_TAG_ENUM,
	IPP_TAG_STRING = 0x30,
	IPP_TAG_DATE,
	IPP_TAG_RESOLUTION,
	IPP_TAG_RANGE,
	IPP_TAG_COLLECTION,
	IPP_TAG_TEXTLANG,
	IPP_TAG_NAMELANG,
	IPP_TAG_TEXT = 0x41,
	IPP_TAG_NAME,
	IPP_TAG_KEYWORD	= 0x44,
	IPP_TAG_URI,
	IPP_TAG_URISCHEME,
	IPP_TAG_CHARSET,
	IPP_TAG_LANGUAGE,
	IPP_TAG_MIMETYPE
} ipp_tag_t;

//**** IPP status codes... ****
typedef	enum
{
	IPP_OK = 0x0000,
	IPP_OK_SUBST,
	IPP_OK_CONFLICT,
	IPP_CLIENT_BAD_REQUEST = 0x0400,
	IPP_CLIENT_FORBIDDEN,
	IPP_CLIENT_NOT_AUTHENTICATED,
	IPP_CLIENT_NOT_AUTHORIZED,
	IPP_CLIENT_NOT_POSSIBLE,
	IPP_CLIENT_TIMEOUT,
	IPP_CLIENT_NOT_FOUND,
	IPP_CLIENT_GONE,
	IPP_CLIENT_REQUEST_ENTITY,
	IPP_CLIENT_REQUEST_VALUE,
	IPP_CLIENT_DOCUMENT_FORMAT,
	IPP_CLIENT_ATTRIBUTES,
	IPP_CLIENT_URI_SCHEME,
	IPP_CLIENT_CHARSET,
	IPP_CLIENT_CONFLICT,
	IPP_CLIENT_COMPRESSION_NOT_SUPPORTED,
	IPP_CLIENT_COMPRESSION_ERROR,
	IPP_CLIENT_DOCUMENT_FORMAT_ERROR,
	IPP_CLIENT_DOCUMENT_ACCESS_ERROR,
	IPP_SERVER_INTERNAL_ERROR =	0x0500,
	IPP_SERVER_OPERATION_NOT_SUPPORTED,
	IPP_SERVER_SERVICE_UNAVAILABLE,
	IPP_SERVER_VERSION_NOT_SUPPORTED,
	IPP_SERVER_DEVICE_ERROR,
	IPP_SERVER_TEMPORARY_ERROR,
	IPP_SERVER_NOT_ACCEPTING,
	IPP_SERVER_PRINTER_BUSY,
	IPP_SERVER_ERROR_JOB_CANCELLED,
	IPP_SERVER_MULTIPLE_JOBS_NOT_SUPPORTED
} ipp_status_t;

//**** Resolution units... ****
typedef	enum
{
	IPP_RES_PER_INCH = 3,
	IPP_RES_PER_CM
} ipp_res_t;

//**** Attribute Value ****
typedef	union
{
	int32 integer;		 //	Integer/enumerated value

	int8  boolean;		 //	Boolean	value

	BYTE  date[11];		 //	Date/time value

	struct {
		int32 xres;		 //	Horizontal resolution
		int32 yres;		 //	Vertical resolution
		ipp_res_t units; //	Resolution units
	} resolution;		 //	Resolution value

	struct	{
		int32 lower;	 //	Lower value
		int32 upper;	 //	Upper value
	} range;			 //	Range of integers value

	struct {
		BYTE *charset;	 //	Character set
		BYTE *text;		 //	String
	} string;			 //	String with	language value

	struct {
		int16 length;	 //	Length of attribute
		void  *data;	 //	Data in	attribute
	} unknown;			 //	Unknown	attribute type

} ipp_value_t;

typedef	struct {
	BYTE attrib_name[IPP_MAX_ATTRIB_NAME+1];
	ipp_value_t	value;
} ipp_attrib_t;

typedef	struct IPP_T {
	FILE		 *network;
	int		  	  s;
	BYTE		  ippreq;
	BYTE		  chunked;
	int32		  qsize;
	BYTE          DocType;   //Document Type, 1: BINARY,  2:TEXT

	WORD		  Version;
	ipp_status_t  RetCode;
	DWORD		  ID;
	BYTE		 *Language;
	BYTE         *Charset;
	ippBuf		  UnSupportGroup;
	ippBuf		  RespGroup;

	BYTE 		 *job_name;
	BYTE 		 *user_name;
	BYTE          job_state;    //0: nojob,	 5:	processing,	7: canceled, 8:	aborted, 9:completed, 0x80:	is aborting
	WORD          job_id;       //001 - 999
	struct IPP_T *next;
} ipp_t;

#define	IPP_PORT_MAX_NAME  21
/*
void ippd(BYTE port, ipp_t *ippObj);
BYTE ippCheckPort(BYTE *url, ipp_t *ippObj);
int16 ippSetRespBuf(BYTE *buf, ipp_t *ippObj);
void ippSendResp(ipp_t *ippObj); //for HTTPD.c
void ProcessCRLF(BYTE *KeepCR, struct prnbuf *pbuf,BYTE *Tmpbuf, int16 size);
*/
//Jesse extern ipp_t *ippObjList[NUM_OF_PRN_PORT];
//Jesse extern BYTE  ippJobsNo[NUM_OF_PRN_PORT]; //How many Obj connected for print !
ipp_t *ippObjList[NUM_OF_PRN_PORT];
BYTE  ippJobsNo[NUM_OF_PRN_PORT]; //How many Obj connected for print !

#endif	_IPPD_H_
