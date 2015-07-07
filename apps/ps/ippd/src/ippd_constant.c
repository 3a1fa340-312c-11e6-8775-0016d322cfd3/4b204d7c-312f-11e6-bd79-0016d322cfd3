#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"


const BYTE *AttribTAG[] = {
#define	ATTR_PRINTER_URI     0 //for ALL
"printer-uri"
#define	ATTR_USER_NAME       1 //for ALL
,"requesting-user-name"
#define	ATTR_JOB_NAME        2 //for Print, Validate
,"job-name"
#define	ATTR_FIDELITY        3 //for Print
,"ipp-attribute-fidelity"
#define	ATTR_JOB_ID	         4 //for Cancel, JobAttr, Jobs
,"job-id"
#define	ATTR_JOB_URI         5 //for Cancel, JobAttr, Jobs
,"job-uri"
#define	ATTR_LIMIT           6 //for JobAttr, Jobs
,"limit"
#define	ATTR_WHICH_JOB	     7 //for JobAttr, Jobs
,"which-jobs"
#define	ATTR_MY_JOB		     8 //for JobAttr, Jobs
,"my-jobs"
#define	ATTR_REQUEST_ATTRIB	 9 //for JobAttr, Jobs, PrnAttr
,"requested-attributes"
#define	ATTR_DOC_FORMAT     10 //for Print, Validate, PrnAttr
, "document-format"
#define ATTR_DOC_NAME       11 //for Print, Vaildate
, "document-name"
, NULL
};

const BYTE *JobTempAttribTAG[] = {
#define	JTAT_ALL			 0
 "all"
#define	JTAT_JOB_URI		 1
,"job-uri"					   //(R) uri
#define	JTAT_JOB_ID			 2
,"job-id"					   //(R) integer(1:MAX)
#define	JTAT_PRINTER_URI	 3
,"job-printer-uri"			   //(R) uri
#define	JTAT_JOB_NAME		 4
,"job-name"					   //(R) name (MAX)
#define	JTAT_USER_NAME		 5
,"job-originating-user-name"   //(R) name (MAX)
#define	JTAT_JOB_STATE		 6
,"job-state"				   //(R) type1 enum
#define	JTAT_CHARSET		 7
,"attributes-charset"		   //(R) charset
#define	JTAT_LANGUAGE		 8
,"attributes-natural-language" //(R) naturalLanguage
#define	JTAT_JOB_DESCRIPTION 9
,"job-description"			   //group
, NULL
};

const BYTE *PrinterDescriptionTAG[] = {
#define	PDT_ALL                       0
"all"
#define	PDT_PRINTER_URI_SUPPORTED     1
,"printer-uri-supported"	                //(R) uri
#define	PDT_SECURITY_SUPPORTED        2
,"uri-security-supported"                   //(R) type2 keyword
#define	PDT_PRINTER_NAME              3
,"printer-name"                             //(R) name (127)
#define	PDT_PRINTER_LOCATION          4
,"printer-location"                         //(O) name
#define	PDT_PRINTER_MORE_INFO         5
,"printer-more-info"                        //(O) URI
#define	PDT_MODEL                     6
,"printer-make-and-model"                   //(O) text (127)
#define	PDT_PRINTER_STATE             7
,"printer-state"                            //(R) type1 enum
#define	PDT_STATE_REASON              8
,"printer-state-reasons"                    //(O) type2 keyword
#define	PDT_OP_SUPPORTED              9
,"operations-supported"                     //(R) 1setOf type2 enum
#define	PDT_CHARSET_CONFIGED         10
,"charset-configured"                       //(R) charset
#define	PDT_CHARSET_SUPPORTED        11
,"charset-supported"                        //(R) 1setOf charset
#define	PDT_LANGUAGE_CONFIGED        12
,"natural-language-configured"              //(R) naturalLanguage
#define	PDT_LANGUAGE_SUPPORTED       13
,"generated-natural-language-supported"     //(R) naturalLanguage
#define	PDT_DOC_FORMAT_DEFAULT       14
,"document-format-default"                  //(R) mimeMediaType
#define	PDT_DOC_FORMAT_SUPPORTED     15
,"document-format-supported"                //(R) mimeMediaType
#define	PDT_ACCEPTING_JOBS           16
,"printer-is-accepting-jobs"                //(R) boolean
#define	PDT_PDL_OVERRIDE_SUPPORTED   17
,"pdl-override-supported"                   //(R) type2 keyword
#define	PDT_UP_TIME                  18
,"printer-up-time"                          //(R) integer (1:MAX)
#define	PDT_PRINTER_DESCRIPTION      19
,"printer-description"
,NULL
};
