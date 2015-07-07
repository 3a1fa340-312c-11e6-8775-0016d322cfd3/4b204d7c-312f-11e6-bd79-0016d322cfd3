#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "ippd.h"
#include "httpd.h"
#include "prnport.h"
#include "prnqueue.h"
#include "joblog.h"

#define bcopy(x,y,z)  memcpy((y),(x),(z))	 

//******************************IP ADDRESS*********
typedef struct MIB_DHCP_s
{
    uint32 IPAddr;
    uint32 SubnetMask;
    uint32 GwyAddr;
}MIB_DHCP;

extern MIB_DHCP                   *mib_DHCP_p;

extern int morespace(struct papfile *pf,char *data,int len);

//extern int recv(int s, void *buf, size_t len, int flags);
extern int zot_fclose( ZOT_FILE* fp);
extern char *zot_fgets(char *buf,int len,ZOT_FILE *fp);
#define fclose(x)		 zot_fclose(x)
#define fgets(x,y,z)	 zot_fgets(x,y,z)
#define FILE	 		 ZOT_FILE

/* Read a line from a stream into a buffer*/
int zot_recv ( ZOT_FILE *fp , char *buf , int len, int flag )
{
	if( fp->ibuf_cnt ==0 )
	{
		fp->ibuf = fp->ibuf_temp;
//		memset(fp->ibuf ,0 , BLOCKSIZE);
		memset(fp->ibuf ,0 , 1460);
		
//		fp->bufsize = recv( fp->fd , fp->ibuf , BLOCKSIZE , 0  );
		fp->bufsize = recv( fp->fd , fp->ibuf , 1460 , 0  );
		
		fp->ibuf_cnt = fp->bufsize;
		if( fp->bufsize <= 0 )
			return -1;
	}

	len = min( len , fp->ibuf_cnt );	
	memcpy(buf,fp->ibuf,len);
	fp->ibuf +=len;
	fp->ibuf_cnt = fp->ibuf_cnt - len;

	return len;
}


#ifdef IPPD

#define  BLOCK_SIZE				( 1460 )

//IPPD_Start Thread initiation information
#define IPPD_TASK_PRI         	20	//ZOT716u2
#define IPPD_TASK_STACK_SIZE  	1024 //ZOT716u2 3072
static	uint8			IPPD_Stack[IPPD_TASK_STACK_SIZE];
static  cyg_thread		IPPD_Task;
static  cyg_handle_t	IPPD_TaskHdl;


//from psmain.c
extern uint16 NGET16( uint8 *pSrc );
extern uint32 NGET32( uint8 *pSrc );
extern void NSET16( uint8 *pDest, uint16 value );
extern void NSET32( uint8 *pDest, uint32 value );
extern uint32 rdclock();
extern uint32 msclock();
extern int strnicmp( char * dst,  char * src ,int n );
extern int stricmp( char * dst,  char * src );

//from httpd.c
extern void ippSendResp(ipp_t *ippObj);
extern int	sendack(int s);

//from prnport.c
extern BYTE ReadPortStatus(BYTE port);

//from prnqueue.c
extern short PrnGetPrinterStatus(int PrnPort);
extern char * strdup(const char *str);

//from ippd.h
void ProcessCRLF(BYTE *KeepCR, struct prnbuf *pbuf,BYTE *Tmpbuf, int16 size);

//from eeprom.c
extern char	Hostname[LENGTH_OF_BOX_NAME+1];

//from prnport.c
extern struct parport PortIO[NUM_OF_PRN_PORT];

#define PrnPortDisConnected(port) (RealPortStatus((port)) == 0x7F)

#define ippPrintJobClose(port, x)	 \
{							 \
	ippSetPrintJobResp((port),(x)); \
	ippSendResp((x));		 \
	fclose((x)->network);    \
}

#define CONST_DATA
#ifdef CONST_DATA
extern const BYTE *AttribTAG[];
extern const BYTE *JobTempAttribTAG[];
extern const BYTE *PrinterDescriptionTAG[];

#define	ATTR_PRINTER_URI     0 //for ALL
#define	ATTR_USER_NAME       1 //for ALL
#define	ATTR_JOB_NAME        2 //for Print, Validate
#define	ATTR_FIDELITY        3 //for Print
#define	ATTR_JOB_ID	         4 //for Cancel, JobAttr, Jobs
#define	ATTR_JOB_URI         5 //for Cancel, JobAttr, Jobs
#define	ATTR_LIMIT           6 //for JobAttr, Jobs
#define	ATTR_WHICH_JOB	     7 //for JobAttr, Jobs
#define	ATTR_MY_JOB		     8 //for JobAttr, Jobs
#define	ATTR_REQUEST_ATTRIB	 9 //for JobAttr, Jobs, PrnAttr
#define	ATTR_DOC_FORMAT     10 //for Print, Validate, PrnAttr
#define ATTR_DOC_NAME       11 //for Print, Vaildate

#define	JTAT_ALL			 0
#define	JTAT_JOB_URI		 1
#define	JTAT_JOB_ID			 2
#define	JTAT_PRINTER_URI	 3
#define	JTAT_JOB_NAME		 4
#define	JTAT_USER_NAME		 5
#define	JTAT_JOB_STATE		 6
#define	JTAT_CHARSET		 7
#define	JTAT_LANGUAGE		 8
#define	JTAT_JOB_DESCRIPTION 9

#define	PDT_ALL                       0
#define	PDT_PRINTER_URI_SUPPORTED     1
#define	PDT_SECURITY_SUPPORTED        2
#define	PDT_PRINTER_NAME              3
#define	PDT_PRINTER_LOCATION          4
#define	PDT_PRINTER_MORE_INFO         5
#define	PDT_MODEL                     6
#define	PDT_PRINTER_STATE             7
#define	PDT_STATE_REASON              8
#define	PDT_OP_SUPPORTED              9
#define	PDT_CHARSET_CONFIGED         10
#define	PDT_CHARSET_SUPPORTED        11
#define	PDT_LANGUAGE_CONFIGED        12
#define	PDT_LANGUAGE_SUPPORTED       13
#define	PDT_DOC_FORMAT_DEFAULT       14
#define	PDT_DOC_FORMAT_SUPPORTED     15
#define	PDT_ACCEPTING_JOBS           16
#define	PDT_PDL_OVERRIDE_SUPPORTED   17
#define	PDT_UP_TIME                  18
#define	PDT_PRINTER_DESCRIPTION      19

#else

BYTE *AttribTAG[] =	{
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

BYTE *JobTempAttribTAG[] = {
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

BYTE *PrinterDescriptionTAG[] =	{
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

#endif CONST_DATA

#define ippPrintJob(x, y)    ippPrintValidateJob((x),(y))
#define ippValidateJob(x, y) ippPrintValidateJob((x),(y))

static ipp_tag_t ippGetAttribute(ipp_t *ippObj, ipp_attrib_t *t);
static void ippFreeAttribute(ipp_tag_t tag, ipp_attrib_t *t);
static int httpRead(ipp_t *ippObj, BYTE *buffer, int length);
static int ippRead(ipp_t *ippObj, BYTE *buffer, int length);

static void ippPrintValidateJob(BYTE port, ipp_t *ippObj);
static void ippCancelJob(BYTE port, ipp_t *ippObj);
static void ippGetJobAttribute(BYTE port, ipp_t *ippObj);
static void ippGetJobs(BYTE port, ipp_t *ippObj);
static void ippGetPrinterAttr(BYTE port, ipp_t *ippObj);

static void ippProcessJob(cyg_addrword_t data);
static void ippProcessBINJob (int port, ipp_t *ippObj);
static void ippProcessTXTJob (int port, ipp_t *ippObj);

static int8 ippFindAttribute(const BYTE **attributeTAG,BYTE *attrib);
static BYTE ippValidDestPort(BYTE *URI,BYTE DocType);
static ipp_t *ippValidDestJob(BYTE port, BYTE *URI, BYTE DocType);
static void ippAddAttribute(ippBuf *buf, ipp_tag_t tag, ipp_attrib_t *attr);
static void ippSetPrintJobResp(BYTE port, ipp_t *ippObj);
static void ippSetJobAttribute(BYTE port, int8 val,ipp_t *ippPrnJob, ipp_t *ippObj);
static void ippSetPrinterAttribute(BYTE port, int8 val, ipp_t *ippObj);
static void ippAppend(ippBuf *pf, BYTE *data, int16 len);

BYTE       ippProcessOn[NUM_OF_PRN_PORT];
BYTE       ippJobsNo[NUM_OF_PRN_PORT];	 //How many Obj connected for print !
WORD       ippJobID[NUM_OF_PRN_PORT];	 // 1 - 1000
ipp_t      *ippObjList[NUM_OF_PRN_PORT];

//if into this function must free ippObj from this function !
void ippd(BYTE port, ipp_t *ippObj)
{
	BYTE buf[256];
	ipp_attrib_t ippAttrib;
	ipp_tag_t tag;

	////// initialize ippObject	///////////////////////
	ippObj->Version = 0x001;	 //01.00 version 1.0
	ippObj->RetCode = IPP_OK;
	ippObj->ID      = 0;
	//////////////////////////////////////////////////

	//read version,	request	op,	request	id
	if(ippRead(ippObj,buf,8) < 8) {
		ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
		goto ippResp;
	}

	//Version
	if(buf[0] != 1)	{
		//Return an	error, since we	only support IPP 1.x.
		ippObj->RetCode = IPP_SERVER_VERSION_NOT_SUPPORTED;
		goto ippResp;
	}

	//Resuest ID , copy	to Response	ID filed
	ippObj->ID = NGET32(buf+4); //response id = request id

	//Get TAG
	if((tag = ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_OPERATION) {
		ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
		ippFreeAttribute(tag, &ippAttrib);
		goto ippResp;
	}

	//Get Charset
	if((tag	= ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_CHARSET) {
		ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
		ippFreeAttribute(tag, &ippAttrib);
		goto ippResp;
	}
	if(strcmp(ippAttrib.value.string.text,IPP_DEFAULT_CHARSET) &&
	   strcmp(ippAttrib.value.string.text,IPP_SECOND_CHARSET))
	{
		ippObj->RetCode = IPP_CLIENT_CHARSET;
		ippFreeAttribute(tag, &ippAttrib);
		goto ippResp;
	}
	ippObj->Charset = ippAttrib.value.string.text;

	//Get Language
	if((tag	= ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_LANGUAGE) {
		ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
		ippFreeAttribute(tag, &ippAttrib);
		goto ippResp;
	}
	ippObj->Language	= ippAttrib.value.string.text; //assign to Language

	ippObj->ippreq = buf[3]; //for http check need close sock or not !

	//OP
	switch(ntohs(NGET16(buf+2))) {
	case IPP_PRINT_JOB :
		ippPrintJob(port, ippObj);
		break;
	case IPP_VALIDATE_JOB :
		ippValidateJob(port, ippObj);
		break;
	case IPP_CANCEL_JOB	:
		ippCancelJob(port, ippObj);
		break;
	case IPP_GET_JOB_ATTRIBUTES	:
	    ippGetJobAttribute(port, ippObj);
		break;
	case IPP_GET_JOBS :
	    ippGetJobs(port, ippObj);
		break;
	case IPP_GET_PRINTER_ATTRIBUTES	:
		ippGetPrinterAttr(port, ippObj);
		break;
	case IPP_PAUSE_PRINTER :
	case IPP_RESUME_PRINTER	:
	case IPP_PURGE_JOBS	:
	default	:
		ippObj->RetCode = IPP_SERVER_OPERATION_NOT_SUPPORTED;
		break;
	}
ippResp:

	if(ippObj->ippreq == IPP_PRINT_JOB) {
		if(ippObj->RetCode < IPP_CLIENT_BAD_REQUEST) {

			//create a new process for print job !
			cyg_scheduler_lock();
				
			if(!ippProcessOn[port]) {
				ippProcessOn[port] = 1;
				cyg_scheduler_unlock();
				
				if( IPPD_TaskHdl != 0 )
					cyg_thread_delete(IPPD_TaskHdl);
				
//os			newproc("ippChild", 384, ippProcessJob, port, NULL, NULL, 0 );
				//Create IPPD_Start Thread
				cyg_thread_create(IPPD_TASK_PRI,
									ippProcessJob,
									port,
									"ippChild",
									(void *) (IPPD_Stack),
									IPPD_TASK_STACK_SIZE,
									&IPPD_TaskHdl,
									&IPPD_Task);
			
				//Start IPPD_start Thread
				cyg_thread_resume(IPPD_TaskHdl);

			}
			else{
				sendack(ippObj->s);
				cyg_scheduler_unlock();
			}
			return;
		} else {
			ippObj->ippreq = IPP_PRINT_JOB_ERROR;
		}
	}

//Jesse	wndsize(ippObj->s,0); //use system default size
	ippSendResp(ippObj);
}

void ippProcessJob(cyg_addrword_t data)
{
	int port = data;
	ipp_t *ippObj;
	uint32 startime;
	ipp_t *TmpObjList;
	int i;
	
	cyg_scheduler_lock();	//615wu::No PSMain
	
	startime = rdclock();
	
	while(ippJobsNo[port]) {
	
		if(ReadPortStatus(port) != PORT_READY ||
		   PrnGetPrinterStatus(port) != PrnNoUsed)
		{
//os			kwait(NULL);
			cyg_scheduler_unlock();	//615wu::No PSMain
			
			if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
//				if(startime)	//2003Nov28
				
				//Because lwip don't send ack, send here!
				TmpObjList = ippObjList[port];
				sendack(TmpObjList->s);
				
				for(i=1; i < ippJobsNo[port]; i++)
				{
					TmpObjList = TmpObjList->next;
					sendack(TmpObjList->s);
				}
								
				startime = rdclock();
			}
			
			cyg_thread_yield();
			
			cyg_scheduler_lock();	//615wu::No PSMain
			
			continue;	//after kwait() the job may be cancel !
		}

		PrnSetIppInUse(port);
		
		cyg_scheduler_unlock();	//615wu::No PSMain

		ippObj = ippObjList[port];

#ifdef SUPPORT_JOB_LOG
			JL_PutList(5, port, ippObj->user_name, 32);
#endif SUPPORT_JOB_LOG


#ifdef PC_OUTPUT
		while(ippObj == NULL || ippObj->job_state != IPP_JOB_PENDING)
		    printf("(ippd.c) ippProcessJob Design Error (1) !\n\a");
#endif PC_OUTPUT

		ippObj->job_state = IPP_JOB_PROCESS;
//Jesse		wndsize(ippObj->s,0); //use system default size

		switch(ippObj->DocType) {
		case IPP_BINARY_DOC:
			ippProcessBINJob(port, ippObj);
			break;
		case IPP_TEXT_DOC:
			ippProcessTXTJob(port, ippObj);
			break;
#ifdef PC_OUTPUT
		default:
			while(1) printf("(ippd.c) ippProcessJob Design Error (2) !\n\a");
#endif PC_OUTPUT
		}

		PrnSetNoUse(port);

		if(ippObj->job_state == IPP_JOB_PROCESS) {
			ippObj->job_state = IPP_JOB_COMPLETE;
		}

		ippPrintJobClose(port, ippObj);

#ifdef SUPPORT_JOB_LOG
		if( ippObj->job_state == IPP_JOB_CANCEL )
		{
			JL_EndList(port, 3);	// Timeout. George Add February 1, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
			SendEOF(port);	        // Send the EOF page. George Add January 10, 2008
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
		}
		else
			JL_EndList(port, 0);	// Completed. George Add January 26, 2007
#endif SUPPORT_JOB_LOG

		
		
		cyg_scheduler_lock();
                  ippObjList[port] = ippObjList[port]->next;
		ippJobsNo[port]--;
		cyg_scheduler_unlock();
                  free(ippObj);
//os		kwait(NULL);
		cyg_thread_yield();
	}

#ifdef PC_OUTPUT
	while(ippObjList[port] != NULL)
	    printf("(ippd.c) ippProcessJob Design Error (3) !\n\a");
#endif PC_OUTPUT
	

	cyg_scheduler_lock();
	ippProcessOn[port] = 0;
	cyg_scheduler_unlock();
	cyg_thread_exit();
}

void ippProcessBINJob (int port, ipp_t *ippObj)
{
	uint32 startime;
	struct prnbuf *pbuf;
//	int bytes,recv_bytes;
	ipp_t *TmpObjList;
	int i;
	
	
	do {
		startime = 0;
		while((pbuf = PrnGetInQueueBuf(port)) == NULL) {

			//********************************************************
			if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
//				if(startime)	//2003Nov28

				sendack(ippObj->s);
				//Because lwip don't send ack, send here! 
				TmpObjList = ippObj;
				for(i=1; i < ippJobsNo[port ]; i++)
				{
					TmpObjList = TmpObjList->next;
					sendack(TmpObjList->s);					
				}

				startime = rdclock();
			}
			//********************************************************

//os			kwait(0);
			cyg_thread_yield();
		}

		if((pbuf->size = httpRead(ippObj,pbuf->data,BLOCKSIZE)) <= 0) {
			if(ippObj->ippreq == IPP_PRINT_JOB_TIME_OUT) {
				ippObj->job_state = IPP_JOB_CANCEL;
			} else {
//				ippObj->job_state = IPP_JOB_COMPLETE;
			}
			break;
		}
		
/*		bytes = 0;
		while((recv_bytes = httpRead(ippObj,&pbuf->data[bytes],BLOCKSIZE)) > 0)
		{
		if(ippObj->job_state == IPP_JOB_ABORTING) {
				pbuf->size = 0;
				ippObj->job_state = IPP_JOB_CANCEL;
				break;
			}
			bytes += recv_bytes;
			
			if( bytes + BLOCK_SIZE > BLOCKSIZE )
				break;
		}
		if( recv_bytes <= 0 )
		{	
			if( (ippObj->job_state == IPP_JOB_ABORTING)||(ippObj->ippreq == IPP_PRINT_JOB_TIME_OUT)) 
			{
				pbuf->size = 0;
				ippObj->job_state = IPP_JOB_CANCEL;
				break;
			}
			pbuf->size = bytes;
			break;
		}
*/		
		if(ippObj->job_state == IPP_JOB_ABORTING) {
			pbuf->size = 0;
			ippObj->job_state = IPP_JOB_CANCEL;
			break;
		}
		
#ifdef SUPPORT_JOB_LOG
		JL_AddSize(port, pbuf->size);
#endif SUPPORT_JOB_LOG

//		pbuf->size = bytes;
		
		PrnPutOutQueueBuf(port,pbuf,PRN_Q_NORMAL);
	} while( 1 );

	if( ippObj->job_state == IPP_JOB_CANCEL ) {
		PrnAbortSpooler(port,pbuf);
		ippObj->RetCode	= IPP_SERVER_ERROR_JOB_CANCELLED;
	} else {
		PrnPutOutQueueBuf(port,pbuf,PRN_Q_EOF);
	}
}

void ippProcessTXTJob(int port, ipp_t *ippObj)
{
	uint32 startime;
	struct prnbuf *pbuf[3];
	int16 bytes;
	BYTE  KeepCR = 0, i;

	startime = 0;
	while((pbuf[2] = PrnGetInQueueBuf(port)) == NULL) {

		//********************************************************
		if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
			if(startime) sendack(ippObj->s);
			startime = rdclock();
		}
		//********************************************************

//os		kwait(0);
		cyg_thread_yield();
	}
	pbuf[2]->size = 0;

	do {
		startime = 0;
		for(i = 0 ; i < 2; i++) {
			while((pbuf[i] = PrnGetInQueueBuf(port)) == NULL) {

				//********************************************************
				if(((rdclock()-startime) > ((uint32)TICKS_PER_SEC*5))) {
//					if(startime)	//2003Nov28
					sendack(ippObj->s);
					startime = rdclock();
				}
				//********************************************************

//os				kwait(0);
				cyg_thread_yield();
			}
			pbuf[i]->size = 0;
		}

#ifdef PC_OUTPUT
		AtSaySpace(40,port*4+3,40);
		printf("(IPP)   Port%d is receiving (In)", port);
#endif

		if((pbuf[2]->size = httpRead(ippObj,pbuf[2]->data,BLOCKSIZE)) <= 0) {
			if(ippObj->ippreq == IPP_PRINT_JOB_TIME_OUT) {
				ippObj->job_state = IPP_JOB_CANCEL;
#ifdef PC_OUTPUT
				AtSaySpace(0,port*4+4,40);
				printf("(IPP) Time Out !");
#endif
			} else {
//				ippObj->job_state = IPP_JOB_COMPLETE;
#ifdef PC_OUTPUT
				AtSaySpace(0,port*4+4,40);
				printf("(IPP) Finish !");
#endif
			}
			break;
		}
#ifdef PC_OUTPUT
		AtSaySpace(40,port*4+3,40);
		printf("(TCP/IP) Port%d is receiving (Out) ",port);
#endif

		if(ippObj->job_state == IPP_JOB_ABORTING) {
#ifdef PC_OUTPUT
			AtSaySpace(0,port*4+4,40);
			printf("(IPP) Cancel Job by user !");
#endif
			ippObj->job_state = IPP_JOB_CANCEL;
			break;
		}

#ifdef SUPPORT_JOB_LOG
		JL_AddSize(port, pbuf[2]->size);
#endif SUPPORT_JOB_LOG

		bytes = (pbuf[2]->size > (BLOCKSIZE/2))?(BLOCKSIZE/2):pbuf[2]->size;
		ProcessCRLF(&KeepCR, pbuf[0], pbuf[2]->data,bytes);
		PrnPutOutQueueBuf(port,pbuf[0],PRN_Q_NORMAL);

		if( (bytes = (pbuf[2]->size - (BLOCKSIZE/2))) > 0) {
			ProcessCRLF(&KeepCR, pbuf[1], pbuf[2]->data+(BLOCKSIZE/2),bytes);
			PrnPutOutQueueBuf(port,pbuf[1],PRN_Q_NORMAL);
		} else {
			PrnPutInQueueBuf(port, pbuf[1]);
		}
	} while( 1 );

    pbuf[2]->size = 0;
	if( ippObj->job_state == IPP_JOB_CANCEL ) {
		PrnAbortSpooler(port,pbuf[2]);
		ippObj->RetCode	= IPP_SERVER_ERROR_JOB_CANCELLED;
	} else {
		PrnPutOutQueueBuf(port,pbuf[2],PRN_Q_EOF);
	}
	PrnPutInQueueBuf(port, pbuf[0]);
	PrnPutInQueueBuf(port, pbuf[1]);
}

// (1) CR+LF  ==> CR+LF
// (2) CR+nLF ==> CR+LF+(nLF....)
// (3) LF     ==> CR+LF
// note : CR: 0x0D, LF: 0x0A , nLF: != (0x0A)
// if nLF == CR then   ex: CR+CR+LF       ==> CR+LF+CR+LF
//                     ex: CR+CR+CR+LF    ==> CR+LF+CR+LF+CR+LF
//                     ex: LF+LF+CR+LF+LF ==> CR+LF+CR+LF+CR+LF+CR+LF
void ProcessCRLF(BYTE *KeepCR, struct prnbuf *pbuf,BYTE *tmpbuf, int16 size)
{
	BYTE *endbuf = tmpbuf+ size;
	BYTE *sbuf = tmpbuf;
	BYTE *dbuf = pbuf->data;

	if(*KeepCR && sbuf != endbuf) {
		if(*sbuf == 0x0A) {
			sbuf++;
		}
	}

	*KeepCR = 0;
	while(sbuf != endbuf) {
		switch(*sbuf) {
		case 0x0D:
			*(dbuf++) = 0x0D;
			*(dbuf++) = 0x0A;
			if((++sbuf) == endbuf) {
				*KeepCR = 1;
			} else {
				if(*sbuf == 0x0A) {
					sbuf++;
				}
			}
			continue;
		case 0x0A:
			*(dbuf++) = 0x0D;
			break;
		}
		*(dbuf++) = *(sbuf++);
	}
	pbuf->size = dbuf - pbuf->data;
}

ipp_tag_t ippGetAttribute(ipp_t *ippObj, ipp_attrib_t *t)
{
	ipp_tag_t tag;
	int32 n32;
	int16 n16;
	int8  n8;

	if(ippRead(ippObj, &n8, 1) <= 0) return (IPP_TAG_ZERO); //error

	//*
	//* Read this attribute...
	//*

	tag	= (ipp_tag_t)n8;

	// Group tag
	if(tag < IPP_TAG_UNSUPPORTED_VALUE)	return (tag);

	//Get length of	attribute name
	if(ippRead(ippObj,(BYTE*) &n16, 2) < 2) return (IPP_TAG_ZERO); //error

	//attribute	name too long
	if((n16	= ntohs(n16)) >	IPP_MAX_ATTRIB_NAME) {
		//internal error
		return (IPP_TAG_ZERO);	//error
	}

	if(n16 == 0) {
		t->attrib_name[0] = '\0';
	} else {
		//Get attribute	name

		if (ippRead(ippObj, t->attrib_name, n16) < n16) {
			return (IPP_TAG_ZERO);	//error
		}
		t->attrib_name[n16]	= '\0';
	}

	//Get length of	attribute value
	if(ippRead(ippObj,(BYTE*) &n16, 2) < 2) return (IPP_TAG_ZERO); //error

	n16	= ntohs(n16);

	memset(&t->value,'\0',sizeof(ipp_value_t));

	//RFC2565  Encoding	& Transport	, 3.11 Value (Page 15)
	switch (tag) {
	case IPP_TAG_INTEGER :
	case IPP_TAG_ENUM :
		if(n16 != 4	|| ippRead(ippObj,(BYTE*) &n32, 4) < 4) return (IPP_TAG_ZERO);

		t->value.integer = ntohl(n32);
		break;
	case IPP_TAG_BOOLEAN :
		if(n16 != 1	|| ippRead(ippObj, &n8, 1) < 1) return (IPP_TAG_ZERO);
		t->value.boolean = n8;
		break;
	case IPP_TAG_TEXT :
	case IPP_TAG_NAME :
	case IPP_TAG_KEYWORD :
	case IPP_TAG_STRING	:
	case IPP_TAG_URI :
	case IPP_TAG_URISCHEME :
	case IPP_TAG_CHARSET :
	case IPP_TAG_LANGUAGE :
	case IPP_TAG_MIMETYPE :
		if((t->value.string.text = calloc(n16+1,1)) == NULL) {
			//not enough memory	(internal error)
			return (IPP_TAG_ZERO);
		}
		if(ippRead(ippObj, t->value.string.text, n16) < n16) {
			free(t->value.string.text);
			return (IPP_TAG_ZERO);
		}
		break;
	case IPP_TAG_DATE :
		if(n16 != 11 ||	ippRead(ippObj,t->value.date, 11) < 11)
			return (IPP_TAG_ZERO);
		break;
	case IPP_TAG_RESOLUTION	:
		if(n16 != 9	|| ippRead(ippObj, (BYTE*)&n32, 4) < 4) return (IPP_TAG_ZERO);
		t->value.resolution.xres = ntohl(n32);

		if(ippRead(ippObj, (BYTE*)&n32, 4) < 4) return (IPP_TAG_ZERO);
		t->value.resolution.yres = ntohl(n32);

		if(ippRead(ippObj, &n8, 1) < 1) return (IPP_TAG_ZERO);
		t->value.resolution.units =	(ipp_res_t)n8;
		break;
	case IPP_TAG_RANGE :
		if(n16 != 8	|| ippRead(ippObj,(BYTE*) &n32, 4) < 4) return (IPP_TAG_ZERO);
		t->value.range.lower = ntohl(n32);

		if(ippRead(ippObj,(BYTE*)&n32, 4) < 4) return (IPP_TAG_ZERO);
		t->value.range.upper = ntohl(n32);
		break;
	case IPP_TAG_TEXTLANG :
	case IPP_TAG_NAMELANG :
		//*
		//*	text-with-language and name-with-language are composite
		//*	values:
		//*
		//*	   charset-length (2)
		//*	   charset
		//*	   text-length	  (2)
		//*	   text
		//*

		n32	= n16;

		//charset-length
		if(ippRead(ippObj,(BYTE*) &n16, 2) < 2) return (IPP_TAG_ZERO);
		n16	= ntohs(n16);
		n32	-= n16;

		//charset
		if((t->value.string.charset	= calloc(n16 + 1,1)) == NULL)	{
			//not enough memory
			return (IPP_TAG_ZERO);
		}
		if(ippRead(ippObj,t->value.string.charset , n16) < n16) {
			free(t->value.string.charset);
			return (IPP_TAG_ZERO);
		}

		//text-length
		if(ippRead(ippObj,(BYTE*) &n16, 2) < 2) {
			free(t->value.string.charset);
		    return (IPP_TAG_ZERO);
		}
		n16	= ntohs(n16);

		if(n32 != n16) {
			//length no	same !
			free(t->value.string.charset);
			return (IPP_TAG_ZERO);
		}

		//text
		if((t->value.string.text = calloc(n16 +	1,1)) == NULL) {
			//not enough memory
			free(t->value.string.charset);
			return (IPP_TAG_ZERO);
		}

		if(ippRead(ippObj,t->value.string.text, n16) < n16) {
			free(t->value.string.charset);
			free(t->value.string.text);
			return (IPP_TAG_ZERO);
		}
		break;

	default	:
		// Other unsupported values
		t->value.unknown.length	= n16;
		if (n16	> 0)	{
			if(	(t->value.unknown.data = malloc(n16)) == NULL) {
				//not enough memory
				return (IPP_TAG_ZERO);
			}
			if(ippRead(ippObj , t->value.unknown.data, n16) < n16) {
				free(t->value.unknown.data);
				return (IPP_TAG_ZERO);
			}
		} else {
			t->value.unknown.data =	NULL;
		}
		break;
	}//switch (tag)

	return (tag);
}

void ippFreeAttribute(ipp_tag_t	tag, ipp_attrib_t *t)
{
	switch(tag)	{
	case IPP_TAG_ZERO :
	case IPP_TAG_INTEGER :
	case IPP_TAG_ENUM :
	case IPP_TAG_BOOLEAN :
		break;
	case IPP_TAG_TEXT :
	case IPP_TAG_NAME :
	case IPP_TAG_KEYWORD :
	case IPP_TAG_STRING	:
	case IPP_TAG_URI :
	case IPP_TAG_URISCHEME :
	case IPP_TAG_CHARSET :
	case IPP_TAG_LANGUAGE :
	case IPP_TAG_MIMETYPE :
		free(t->value.string.text);
		break;
	case IPP_TAG_TEXTLANG :
	case IPP_TAG_NAMELANG :
		free(t->value.string.charset);
		free(t->value.string.text);
		break;
	case IPP_TAG_DATE :
	case IPP_TAG_RESOLUTION	:
	case IPP_TAG_RANGE :
		break;
	default	:
		free(t->value.unknown.data);
		break;
	}
}

void ippPrintValidateJob(BYTE port, ipp_t *ippObj)
{
	ipp_attrib_t ippAttrib;
	ipp_tag_t tag;
	int8 fidelity =	0, unsupport = 0;
//JEsse	BYTE PortStatus;
	int8 HaveJobName =0 , HaveUserName = 0;
	int16 Tmplen;
	ipp_t **TmpObjList;

	if(ippObj->ippreq == IPP_PRINT_JOB) {
		if(ippJobsNo[port] >= IPP_MAX_USER) {
			ippObj->RetCode	= IPP_SERVER_SERVICE_UNAVAILABLE;
			return;
		}
#ifdef DEF_IEEE1284
//		if(PrnPortDisConnected(port)) {
//			//disconnected
//			ippObj->RetCode	= IPP_SERVER_SERVICE_UNAVAILABLE;
//			return;
//		}
#endif DEF_IEEE1284
	}

	ippObj->RetCode	= IPP_OK;

	while((tag = ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_ZERO) {
		if(tag < IPP_TAG_UNSUPPORTED_VALUE) {
			if(tag == IPP_TAG_END) break;
			continue;
		}

		switch(ippFindAttribute(AttribTAG,ippAttrib.attrib_name)) {
		case ATTR_PRINTER_URI: //"printer-uri" (R)
			if(tag != IPP_TAG_URI
			   || ippValidDestPort(ippAttrib.value.string.text, ippObj->DocType) != port
			) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			break;
		case ATTR_USER_NAME:   //"requesting-user-name"	(R*) "anonymous"
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(ippObj->ippreq == IPP_PRINT_JOB) {
				Tmplen = strlen(ippAttrib.value.string.text);
				if((ippObj->user_name = malloc(Tmplen+1)) == NULL) {
					ippObj->RetCode	= IPP_SERVER_INTERNAL_ERROR;
					break;
				}
				memcpy(ippObj->user_name,ippAttrib.value.string.text,Tmplen);
				ippObj->user_name[Tmplen] = '\0';
			}
			HaveUserName = 1;
			break;
		case ATTR_JOB_NAME:	   //"job-name", //(R*), "Untitled"
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(ippObj->ippreq == IPP_PRINT_JOB) {
				Tmplen = strlen(ippAttrib.value.string.text);
				if((ippObj->job_name = malloc(Tmplen+1)) == NULL) {
					ippObj->RetCode	= IPP_SERVER_INTERNAL_ERROR;
					break;
				}
				memcpy(ippObj->job_name,ippAttrib.value.string.text,Tmplen);
				ippObj->job_name[Tmplen] = '\0';
			}
			HaveJobName = 1;
			break;
		case ATTR_FIDELITY:	   //"ipp-attribute-fidelity"
			if(tag != IPP_TAG_BOOLEAN) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			fidelity = ippAttrib.value.boolean;
			break;
		case ATTR_DOC_FORMAT:  //"document-format",	//(R*) reject "client-error-document-format-not-supported"
			if(tag != IPP_TAG_MIMETYPE)	{
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(strcmp(ippAttrib.value.string.text,IPP_DEFAULT_DOC_FORMAT) &&
			   strcmp(ippAttrib.value.string.text,IPP_SECOND_DOC_FORMAT))
			{
				ippObj->RetCode	= IPP_CLIENT_DOCUMENT_FORMAT;
			}
			break;
		case ATTR_DOC_NAME:  //"document-name",	//(R*)
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
		    break;
		default:
			//add to unsupport group !
			if(!unsupport) {
				unsupport =	(int8) IPP_TAG_UNSUPPORTED_GROUP;
				ippAppend(&ippObj->UnSupportGroup,(BYTE*)&unsupport,1);
			}
			ippAddAttribute(&ippObj->UnSupportGroup,tag,&ippAttrib);
		}
		ippFreeAttribute(tag, &ippAttrib);
		if(ippObj->RetCode != IPP_OK) goto ippPrintValidJobExit;
	}

	if(tag != IPP_TAG_END) {
		ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
		goto ippPrintValidJobExit;
	}
	else {
		if(unsupport) {
			if(fidelity) {
				ippObj->RetCode = IPP_CLIENT_ATTRIBUTES;
				goto ippPrintValidJobExit;
			}
			else ippObj->RetCode = IPP_OK_SUBST;
		}
	}

	if(ippObj->ippreq == IPP_PRINT_JOB) {
		//ippPrintJob only !

		if(!HaveJobName) {
			if((ippObj->job_name = malloc(sizeof(IPP_DEFAULT_JOB_NAME))) == NULL) {
				ippObj->RetCode	= IPP_SERVER_INTERNAL_ERROR;
				goto ippPrintValidJobExit;
			}
			memcpy(ippObj->job_name,IPP_DEFAULT_JOB_NAME,sizeof(IPP_DEFAULT_JOB_NAME)-1);
			ippObj->job_name[sizeof(IPP_DEFAULT_JOB_NAME)-1] = '\0';
		}

		if(!HaveUserName) {
			if((ippObj->user_name = malloc(sizeof(IPP_DEFAULT_USER_NAME))) == NULL) {
				ippObj->RetCode	= IPP_SERVER_INTERNAL_ERROR;
				goto ippPrintValidJobExit;
			}
			memcpy(ippObj->user_name,IPP_DEFAULT_USER_NAME, sizeof(IPP_DEFAULT_USER_NAME)-1);
			ippObj->user_name[sizeof(IPP_DEFAULT_USER_NAME)-1] = '\0';
		}

		ippObj->job_state = IPP_JOB_PENDING;
		if(++ippJobID[port] >= 1000) ippJobID[port] = 1;
		ippObj->job_id = ippJobID[port];

		cyg_scheduler_lock();
		
		//insert ippObj to ipp
		TmpObjList = &(ippObjList[port]);
		while(*TmpObjList != NULL) TmpObjList = &((*TmpObjList)->next);
		*TmpObjList = ippObj;

		ippJobsNo[port]++;
		
		cyg_scheduler_unlock();
	}

ippPrintValidJobExit:
	return;
}

void ippSetPrintJobResp(BYTE port, ipp_t *ippObj)
{
	ipp_tag_t tag;
	ipp_attrib_t ippAttrib;
	BYTE TmpBuf[IPP_MAX_JOB_URI];

	ippAttrib.value.string.text = TmpBuf;

	tag = IPP_TAG_JOB;
	ippAppend(&ippObj->RespGroup,(BYTE*)&tag,1);

	if(ippObj->DocType == IPP_BINARY_DOC) {
		sprintf(ippAttrib.value.string.text,"/" IPP_BIN_PRINTER_URI_FORMAT IPP_JOB_URI_FORMAT,ippObj->job_id);
	} else {
		sprintf(ippAttrib.value.string.text,"/" IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT IPP_JOB_URI_FORMAT,ippObj->job_id);
	}
	ippAttrib.value.string.text[IPP_PRINTER_PORT_POS+1] = port + '1';

	strcpy(ippAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_URI]);
	ippAddAttribute(&ippObj->RespGroup, IPP_TAG_URI, &ippAttrib);

	ippAttrib.value.integer = ippObj->job_id;
	strcpy(ippAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_ID]);
	ippAddAttribute(&ippObj->RespGroup, IPP_TAG_INTEGER, &ippAttrib);

	ippAttrib.value.integer = ippObj->job_state;
	strcpy(ippAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_STATE]);
	ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippAttrib);
}

void ippCancelJob(BYTE port, ipp_t *ippObj)
{
	ipp_attrib_t ippAttrib;
	ipp_tag_t tag;
	int8 unsupport = 0;
	BYTE PrnURI	= 0;
	BYTE CancelJOB = 0;
	ipp_t *ippPrnJob, *ippPreJob;

	ippObj->RetCode	= IPP_OK;

	while((tag = ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_ZERO) {

		if(tag < IPP_TAG_UNSUPPORTED_VALUE)	{
			if(tag == IPP_TAG_END) break;
			continue;
		}

		switch(ippFindAttribute(AttribTAG,ippAttrib.attrib_name)) {
		case ATTR_PRINTER_URI: //"printer-uri" (R)
			if(tag != IPP_TAG_URI
			   || ippValidDestPort(ippAttrib.value.string.text, ippObj->DocType) != port
			) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			PrnURI = 0x01;
			break;
		case ATTR_JOB_ID:  //"job-id" (R)
			if(tag != IPP_TAG_INTEGER || !PrnURI) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}

			ippPrnJob = ippObjList[port];
			while(ippPrnJob != NULL) {
				if((ippPrnJob->job_id == ippAttrib.value.integer)) {
					CancelJOB = 1;
					break;
				}
				ippPrnJob = ippPrnJob->next;
			}
			if(!CancelJOB) ippObj->RetCode = IPP_CLIENT_NOT_FOUND;
			break;
		case ATTR_JOB_URI: //"job-uri" (R)
			if(tag != IPP_TAG_URI) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if((ippPrnJob = ippValidDestJob(port, ippAttrib.value.string.text, ippObj->DocType)) == NULL) {
				ippObj->RetCode	= IPP_CLIENT_NOT_FOUND;
				break;
			}
			CancelJOB = 1;
			break;
		case ATTR_USER_NAME:   //"requesting-user-name"	(R*)
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			break;
		default:
			//unsupport	!
			if(!unsupport) {
				unsupport =	(int8) IPP_TAG_UNSUPPORTED_GROUP;
				ippAppend(&ippObj->UnSupportGroup,(BYTE*)&unsupport,1);
			}
			ippAddAttribute(&ippObj->UnSupportGroup, tag, &ippAttrib);
			break;
		}
		ippFreeAttribute(tag, &ippAttrib);
		if(ippObj->RetCode != IPP_OK) return;
	}

	if(CancelJOB && ippPrnJob->job_state == IPP_JOB_ABORTING) {
		//client has set the cancel flag but BOX is not cancel yet !
		ippObj->RetCode = IPP_CLIENT_NOT_POSSIBLE;
		return;
	}

	if(tag != IPP_TAG_END || !CancelJOB) {
		ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
	}
	else {
		if(ippPrnJob->job_state == IPP_JOB_PROCESS) {
			ippPrnJob->job_state = IPP_JOB_ABORTING; //aborting
		} else {
#ifdef PC_OUTPUT
			while(ippPrnJob->job_state != IPP_JOB_PENDING)
				printf("(ippd.c) CancelJob Error !\n");
#endif PC_OUTPUT

			if((ippPreJob = ippObjList[port]) == ippPrnJob) {
				ippObjList[port] = NULL;
			} else {
				while(ippPreJob->next != ippPrnJob) ippPreJob = ippPreJob->next;
				ippPreJob->next = ippPrnJob->next;
			}
			cyg_scheduler_lock();
			ippJobsNo[port]--;
			cyg_scheduler_unlock();

			ippPrnJob->job_state = IPP_JOB_CANCEL;
//Jesse			wndsize(ippPrnJob->s,0); //use system default size
			ippPrnJob->RetCode = IPP_SERVER_ERROR_JOB_CANCELLED;
			ippPrintJobClose(port, ippPrnJob);

			free(ippPrnJob);
		}
		if(unsupport) ippObj->RetCode =	IPP_OK_SUBST;
	}
}

void ippGetJobAttribute(BYTE port, ipp_t *ippObj)
{
	ipp_attrib_t ippAttrib;
	ipp_tag_t tag;
	int8 unsupport = 0, ippReqUnsupport = 0, val = IPP_FIND_NO_VALUE;
	BYTE PrnURI	= 0;
	WORD RequestAttr = 0;
	int8 AttribID = 0xFF;
	ipp_t *ippPrnJob = NULL;

	while((tag = ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_ZERO) {

		if(tag < IPP_TAG_UNSUPPORTED_VALUE)	{
			if(tag == IPP_TAG_END) break;
			continue;
		}

		if(ippAttrib.attrib_name[0] != '\0') {
			AttribID = ippFindAttribute(AttribTAG,ippAttrib.attrib_name);
		} else {
			//keep last attribID !
		}

		switch(AttribID) {
		case ATTR_PRINTER_URI: //"printer-uri" (R)
			if(tag != IPP_TAG_URI
			   || ippValidDestPort(ippAttrib.value.string.text, ippObj->DocType) != port
			) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			PrnURI = 0x01;
			break;
		case ATTR_JOB_ID:  //"job-id" (R)
			if(tag != IPP_TAG_INTEGER || !PrnURI) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}

			ippPrnJob = ippObjList[port];
			while(ippPrnJob != NULL) {
				if((ippPrnJob->job_id == ippAttrib.value.integer)) {
					break;
				}
				ippPrnJob = ippPrnJob->next;
			}
			if(ippPrnJob == NULL) ippObj->RetCode = IPP_CLIENT_NOT_FOUND;
			break;
		case ATTR_JOB_URI: //"job-uri" (R)
			if(tag != IPP_TAG_URI) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}

			if((ippPrnJob= ippValidDestJob(port, ippAttrib.value.string.text, ippObj->DocType)) == NULL)
			{
				ippObj->RetCode	= IPP_CLIENT_NOT_FOUND;
				break;
			}
			break;
		case ATTR_USER_NAME:   //"requesting-user-name"	(R*)
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			break;
		case ATTR_REQUEST_ATTRIB: //"requested-attributes" (R*)
			if(tag != IPP_TAG_KEYWORD || ippPrnJob == NULL) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if((val = ippFindAttribute(JobTempAttribTAG,ippAttrib.value.string.text)) >= 0) {
				RequestAttr	|= ((WORD)1 << val);
			} else ippReqUnsupport = 1;
			break;
		default:
			//unsupport	!
			if(!unsupport) {
				unsupport =	(int8) IPP_TAG_UNSUPPORTED_GROUP;
				ippAppend(&ippObj->UnSupportGroup,(BYTE*)&unsupport,1);
			}
			ippAddAttribute(&ippObj->UnSupportGroup, tag, &ippAttrib);
			break;
		}
		ippFreeAttribute(tag, &ippAttrib);
		if((ippObj->RetCode) !=	IPP_OK)	return;
	}

	if(tag != IPP_TAG_END || ippPrnJob == NULL) {
		ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
	} else {
		if(unsupport || ippReqUnsupport) ippObj->RetCode = IPP_OK_SUBST;

		//JOB TAG
		tag = IPP_TAG_JOB;
		ippAppend(&ippObj->RespGroup,(BYTE*)&tag,1);

		if((val == IPP_FIND_NO_VALUE)      ||
		   (RequestAttr & ((WORD)1 << JTAT_ALL)) ||
		   (RequestAttr & ((WORD)1 << JTAT_JOB_DESCRIPTION)) )
		{
			//request all :
			for(val	= 1	; JobTempAttribTAG[val]	!= NULL; val++)	{
				ippSetJobAttribute(port, val, ippPrnJob, ippObj);
			}
		} else {
			val = 0;
			while(RequestAttr != 0) {
				if(RequestAttr & 0x01) {
					ippSetJobAttribute(port, val, ippPrnJob, ippObj);
				}
				RequestAttr >>= 1;
				val++;
			}
		}
	}
}

void ippGetJobs(BYTE port, ipp_t *ippObj)
{
	ipp_attrib_t ippAttrib;
	ipp_tag_t tag;
	int8 unsupport = 0, ippReqUnsupport = 0, val = IPP_FIND_NO_VALUE;
	BYTE *ReqUserName =	NULL;
	BYTE JobState =	IPP_JOB_NOT_COMPLETE;
	BYTE MyJobOnly = 0;
	int8 AttribID = 0xFF;
	ipp_t *ippPrnJob;
	int32 limit = IPP_MAX_USER;
	WORD TmpReqAttr, RequestAttr = 0;

	while((tag = ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_ZERO) {

		if(tag < IPP_TAG_UNSUPPORTED_VALUE)	{
			if(tag == IPP_TAG_END) break;
			continue;
		}

		if(ippAttrib.attrib_name[0] !=	'\0') {
			AttribID = ippFindAttribute(AttribTAG,ippAttrib.attrib_name);
		}

		switch(AttribID) {
		case ATTR_PRINTER_URI: //"printer-uri" (R)
			if(tag != IPP_TAG_URI
			   || ippValidDestPort(ippAttrib.value.string.text, ippObj->DocType) != port
			) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			break;
		case ATTR_USER_NAME:   //"requesting-user-name"	(R*)
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(ReqUserName != NULL)	{
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			ReqUserName	= strdup(ippAttrib.value.string.text);
			break;
		case ATTR_REQUEST_ATTRIB: //"requested-attributes" (R*)
			if(tag != IPP_TAG_KEYWORD) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if((val = ippFindAttribute(JobTempAttribTAG,ippAttrib.value.string.text)) >= 0) {
				RequestAttr	|= ((WORD)1 << val);
			} else ippReqUnsupport = 1;
			break;
		case ATTR_LIMIT:
			if(tag != IPP_TAG_INTEGER) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(ippAttrib.value.integer < limit) limit = ippAttrib.value.integer;
			break;
		case ATTR_WHICH_JOB:
			if(tag != IPP_TAG_KEYWORD) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(!strcmp(ippAttrib.value.string.text,	"completed")) {
				JobState = IPP_JOB_COMPLETE;
			} else if (!strcmp(ippAttrib.value.string.text,	"not-completed")) {
				JobState = IPP_JOB_NOT_COMPLETE;
			} else {
				if(!unsupport) {
					unsupport =	(int8) IPP_TAG_UNSUPPORTED_GROUP;
					ippAppend(&ippObj->UnSupportGroup,(BYTE*)&unsupport,1);
				}

				strcpy(ippAttrib.attrib_name,AttribTAG[ATTR_REQUEST_ATTRIB]);
				ippAddAttribute(&ippObj->UnSupportGroup, tag, &ippAttrib);
				ippObj->RetCode	= IPP_CLIENT_ATTRIBUTES;
			}
			break;
		case ATTR_MY_JOB:
			if(tag != IPP_TAG_BOOLEAN) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(ippAttrib.value.boolean)	{
				MyJobOnly =	1;
			}
			break;
		default:
			//unsupport	!
			if(!unsupport) {
				unsupport =	(int8) IPP_TAG_UNSUPPORTED_GROUP;
				ippAppend(&ippObj->UnSupportGroup,(BYTE*)&unsupport,1);
			}
			ippAddAttribute(&ippObj->UnSupportGroup, tag, &ippAttrib);
			break;
		}
		ippFreeAttribute(tag, &ippAttrib);
		if((ippObj->RetCode) != IPP_OK ) break;
	}

	if(tag != IPP_TAG_END) {
		if(ippObj->RetCode == IPP_OK) ippObj->RetCode = IPP_CLIENT_BAD_REQUEST;
	}
	else {
		if(unsupport || ippReqUnsupport) ippObj->RetCode = IPP_OK_SUBST;

		if(JobState == IPP_JOB_NOT_COMPLETE) {
			ippPrnJob = ippObjList[port];
			while(ippPrnJob) {
				if(limit && (!MyJobOnly || !strcmp(ippPrnJob->user_name,ReqUserName)) ){
					limit--;

					//JOB TAG
					tag = IPP_TAG_JOB;
					ippAppend(&ippObj->RespGroup,(BYTE*)&tag,1);

					if(val == IPP_FIND_NO_VALUE) {
						//request job_uri &	job_id only
						ippSetJobAttribute(port,JTAT_JOB_URI,ippPrnJob, ippObj);
						ippSetJobAttribute(port,JTAT_JOB_ID, ippPrnJob, ippObj);
					} else {
						if((RequestAttr & ((WORD)1 << JTAT_ALL)) ||
						   (RequestAttr & ((WORD)1 << JTAT_JOB_DESCRIPTION)))
						{
							//request all
							for(val	= 1	; JobTempAttribTAG[val]	!= NULL; val++)	{
								ippSetJobAttribute(port, val, ippPrnJob, ippObj);
							}
						} else {
							val = 0;
							TmpReqAttr = RequestAttr;
							while(TmpReqAttr != 0) {
								if(TmpReqAttr & 0x01) {
									ippSetJobAttribute(port, val, ippPrnJob, ippObj);
								}
								TmpReqAttr >>= 1;
								val++;
							}
						}
					}
				}  // if(limit && (!MyJobOnly ....))
				ippPrnJob = ippPrnJob->next;
			} // while(ippPrnJob) {...
		} // if(JobState ==....)
	}
	if(ReqUserName)	free(ReqUserName);
}

void ippGetPrinterAttr(BYTE	port, ipp_t *ippObj)
{
	ipp_attrib_t ippAttrib;
	ipp_tag_t tag;
	int8 unsupport = 0, ippReqUnsupport = 0, val = IPP_FIND_NO_VALUE;
	int8 AttribID = 0xFF;
	DWORD RequestAttr = 0;

	while((tag = ippGetAttribute(ippObj, &ippAttrib)) != IPP_TAG_ZERO) {

		if(tag < IPP_TAG_UNSUPPORTED_VALUE)	{
			if(tag == IPP_TAG_END) break;
			continue;
		}

		if(ippAttrib.attrib_name[0] != '\0') {
			AttribID = ippFindAttribute(AttribTAG,ippAttrib.attrib_name);
		}

		switch(AttribID) {
		case ATTR_PRINTER_URI: //"printer-uri" (R)
			if(tag != IPP_TAG_URI
			   || ippValidDestPort(ippAttrib.value.string.text, ippObj->DocType) != port
			) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			break;
		case ATTR_USER_NAME:   //"requesting-user-name"	(R*)
			if(tag != IPP_TAG_NAME && tag != IPP_TAG_NAMELANG) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			break;
		case ATTR_REQUEST_ATTRIB: //"requested-attributes" (R*)
			if(tag != IPP_TAG_KEYWORD) {
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if((val = ippFindAttribute(PrinterDescriptionTAG,ippAttrib.value.string.text)) >= 0) {
				RequestAttr	|= ((DWORD)1 << val);
			} else ippReqUnsupport = 1;
			break;
		case ATTR_DOC_FORMAT: //"document-format"
			if(tag != IPP_TAG_MIMETYPE)	{
				ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
				break;
			}
			if(strcmp(ippAttrib.value.string.text,IPP_DEFAULT_DOC_FORMAT)	&&
			   strcmp(ippAttrib.value.string.text,IPP_SECOND_DOC_FORMAT) )
			{
				ippObj->RetCode	= IPP_CLIENT_DOCUMENT_FORMAT;
				break;
			}
			break;
		default:
			//unsupport	!
			if(!unsupport) {
				unsupport =	(int8) IPP_TAG_UNSUPPORTED_GROUP;
				ippAppend(&ippObj->UnSupportGroup,(BYTE*)&unsupport,1);
			}
			ippAddAttribute(&ippObj->UnSupportGroup, tag, &ippAttrib);
			break;
		}
		ippFreeAttribute(tag, &ippAttrib);
		if(ippObj->RetCode != IPP_OK) return;
	}

	if(tag != IPP_TAG_END) {
		ippObj->RetCode	= IPP_CLIENT_BAD_REQUEST;
	} else {
		if(unsupport || ippReqUnsupport) ippObj->RetCode = IPP_OK_SUBST;

		tag = IPP_TAG_PRINTER;
		ippAppend(&ippObj->RespGroup,(BYTE*)&tag,1);

		if((val == IPP_FIND_NO_VALUE)     ||
		   (RequestAttr & ((DWORD)1 << PDT_ALL)) ||
		   (RequestAttr & ((DWORD)1 << PDT_PRINTER_DESCRIPTION)) )
		{
			//request all
			for(val	= 1	; PrinterDescriptionTAG[val] != NULL; val++) {
				ippSetPrinterAttribute(port, val, ippObj);
			}
		} else {
			val = 0;
			while(RequestAttr != 0) {
				if(RequestAttr & 0x01) {
					ippSetPrinterAttribute(port, val, ippObj);
				}
				RequestAttr >>= 1;
				val++;
			}
		}
	}
}

int8 ippFindAttribute(const BYTE **attributeTAG,BYTE *attrib)
{
	int8 count = 0;

	if(attrib == NULL) return (IPP_FIND_ERR);

	while(attributeTAG[count] != NULL) {
		if(!strcmp(attributeTAG[count],attrib))	return (count);
		count++;
	}

	return (IPP_FIND_NOT_FOUND);
}

//return "filename"
BYTE *ippGetURIFileName(BYTE *URI)
{
	BYTE *p;
	//(1)  "method://hostname[:port]/filename"
	//(2)  "//hostname[:port]/filename"
	//(3)  "hostname[:port]/filename"
	//(4)  "/filename"

	//get "/filename"
	if((p =	strchr(URI,'/')) !=	NULL) {
		if(*(p+1) == '/') {
			// method (1) or (2)
			p =	strchr(p+2,'/');
		}
		//get "filename"
		if(p != NULL) p++;
	}
	return p;
}

//return 0 (port1) , 1 (port2)	, 2	(port3)
BYTE ippValidDestPort(BYTE *URI, BYTE DocType)
{
	BYTE q[IPP_MAX_PRINTER_URI];
	BYTE *p, i;

	strcpy(q, IPP_BIN_PRINTER_URI_FORMAT);
	if(DocType == IPP_TEXT_DOC) strcat(q, IPP_TXT_PRINTER_EXT_FORMAT);

	p =	ippGetURIFileName(URI);

	for(i =	0; i < NUM_OF_PRN_PORT;	i++) {
		q[IPP_PRINTER_PORT_POS]	= i	+ '1';
		if(!stricmp(p,q)) break;
	}

	return i;
}

ipp_t *ippValidDestJob(BYTE port, BYTE *URI, BYTE DocType)
{
	BYTE q[20];
	BYTE *p;
	ipp_t *ippPrnJob = ippObjList[port];

	p =	ippGetURIFileName(URI);

	while(ippPrnJob != NULL) {
		if(DocType == IPP_BINARY_DOC) {
			sprintf(q,IPP_BIN_PRINTER_URI_FORMAT IPP_JOB_URI_FORMAT,ippPrnJob->job_id);
		} else {
			sprintf(q,IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT IPP_JOB_URI_FORMAT,ippPrnJob->job_id);
		}
		q[IPP_PRINTER_PORT_POS] = port + '1';
		if(!stricmp(q,p)) return ippPrnJob;

		ippPrnJob = ippPrnJob->next;
	}
	return NULL;
}

void ippSetJobAttribute(BYTE port, int8 val,ipp_t *ippPrnJob, ipp_t *ippObj)
{
	BYTE TmpBuf[IPP_TEMP_BUF_SIZE+1];
	ipp_attrib_t ippRspAttrib;

	memset(&ippRspAttrib.value,'\0',sizeof(ipp_value_t));
	ippRspAttrib.value.string.text = TmpBuf;

	switch(val)	{
	case JTAT_ALL: //"all"
		break;
	case JTAT_JOB_URI:
		if(ippPrnJob->DocType == IPP_BINARY_DOC) {
			sprintf(ippRspAttrib.value.string.text,"/" IPP_BIN_PRINTER_URI_FORMAT IPP_JOB_URI_FORMAT,ippPrnJob->job_id);
		} else {
			sprintf(ippRspAttrib.value.string.text,"/" IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT IPP_JOB_URI_FORMAT,ippPrnJob->job_id);
		}
		ippRspAttrib.value.string.text[IPP_PRINTER_PORT_POS+1] = port + '1';
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_URI]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_URI, &ippRspAttrib);
		break;
	case JTAT_JOB_ID:
		ippRspAttrib.value.integer = ippPrnJob->job_id;
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_ID]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_INTEGER, &ippRspAttrib);
		break;
	case JTAT_PRINTER_URI:
		strcpy(ippRspAttrib.value.string.text,"/" IPP_BIN_PRINTER_URI_FORMAT);
		if(ippPrnJob->DocType == IPP_TEXT_DOC) {
			strcat(ippRspAttrib.value.string.text,IPP_TXT_PRINTER_EXT_FORMAT);
		}
		ippRspAttrib.value.string.text[IPP_PRINTER_PORT_POS+1] = port + '1';
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_PRINTER_URI]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_URI, &ippRspAttrib);
		break;
	case JTAT_JOB_NAME:
		ippRspAttrib.value.string.text = ippPrnJob->job_name;
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_NAME]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_NAME, &ippRspAttrib);
		break;
	case JTAT_USER_NAME:
		ippRspAttrib.value.string.text = ippPrnJob->user_name;
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_USER_NAME]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_NAME, &ippRspAttrib);
		break;
	case JTAT_JOB_STATE:
		ippRspAttrib.value.integer = ippPrnJob->job_state;
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_JOB_STATE]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);
		break;
	case JTAT_CHARSET:
		strcpy(ippRspAttrib.value.string.text,ippPrnJob->Charset);
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_CHARSET]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_CHARSET, &ippRspAttrib);
		break;
	case JTAT_LANGUAGE:
		strcpy(ippRspAttrib.value.string.text,ippPrnJob->Language);
		strcpy(ippRspAttrib.attrib_name,JobTempAttribTAG[JTAT_LANGUAGE]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_LANGUAGE, &ippRspAttrib);
		break;
	case JTAT_JOB_DESCRIPTION:
		break;
	default:
		break;
	}
}

void ippSetPrinterAttribute(BYTE port, int8 val, ipp_t *ippObj)
{
	ipp_attrib_t ippRspAttrib;
	BYTE TmpBuf[IPP_TEMP_BUF_SIZE+1];
	BYTE *IPAddr;

	memset(&ippRspAttrib.value,'\0',sizeof(ipp_value_t));

	ippRspAttrib.value.string.text = TmpBuf;

	switch(val)	{
	case PDT_ALL: //"all"
		break;
	case PDT_PRINTER_URI_SUPPORTED:	//"printer-uri-supported"	//(R)	uri
		// "/lp?"
		strcpy(ippRspAttrib.value.string.text,"/" IPP_BIN_PRINTER_URI_FORMAT);
		ippRspAttrib.value.string.text[IPP_PRINTER_PORT_POS+1] = port + '1';
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_PRINTER_URI_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_URI, &ippRspAttrib);

		// "/lp?_txt
		strcat(ippRspAttrib.value.string.text,IPP_TXT_PRINTER_EXT_FORMAT);
		ippRspAttrib.attrib_name[0]	= '\0';
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_URI, &ippRspAttrib);

		break;
	case PDT_SECURITY_SUPPORTED: //"uri-security-supported"	//(R) type2	keyword
		strcpy(ippRspAttrib.value.string.text,"none");
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_SECURITY_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_KEYWORD, &ippRspAttrib);
		break;
	case PDT_PRINTER_NAME: //"printer-name"	 //(R) name	(127)
		strcpy(ippRspAttrib.value.string.text,Hostname);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_PRINTER_NAME]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_NAME, &ippRspAttrib);
		break;
#ifdef SNMPD
	case PDT_PRINTER_LOCATION: //"printer-location"	(O)	text
		strcpy(ippRspAttrib.value.string.text,EEPROM_Data.SnmpSysLocation);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_PRINTER_LOCATION]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_TEXT, &ippRspAttrib);
		break;
#endif SNMPD
	case PDT_PRINTER_MORE_INFO:	 //uri
//		IPAddr = (BYTE *)&Lanface->addr;
		IPAddr = (BYTE *)&mib_DHCP_p->IPAddr;
		sprintf(ippRspAttrib.value.string.text,"http://%d.%d.%d.%d",
				IPAddr[3],IPAddr[2],IPAddr[1],IPAddr[0]);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_PRINTER_MORE_INFO]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_URI, &ippRspAttrib);
		break;
#ifdef DEF_IEEE1284
	case PDT_MODEL:	//"printer-make-and-model" (O) text	(127)
/*
		if(PortIO[port].Model != NULL) {
			memcpy(ippRspAttrib.value.string.text, PortIO[port].Model,IPP_TEMP_BUF_SIZE);
			ippRspAttrib.value.string.text[IPP_TEMP_BUF_SIZE] =	'\0';
		} else  {
			ippRspAttrib.value.string.text[0] =	'\0';
		}
*/	
		ippRspAttrib.value.string.text[0] =	'\0';  //Modify 4/2/2005
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_MODEL]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_TEXT, &ippRspAttrib);
		break;
#endif DEF_IEEE1284
	case PDT_PRINTER_STATE:	//"printer-state" (R) type1	enum
		switch(ReadPortStatus(port)) {
		case PORT_READY: //idle
			ippRspAttrib.value.integer = 3;	//idle
			break;
		case PORT_PRINTING: //processing
			ippRspAttrib.value.integer = 4;	//processing
			break;
		default:
			ippRspAttrib.value.integer = 5;	 //stopped
			break;
		}
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_PRINTER_STATE]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);
		break;
	case PDT_OP_SUPPORTED: //"operations-supported"	  (R) 1setOf type2 enum
		//2	, 4	, 8	, 9	, 10, 11
		ippRspAttrib.value.integer = 2;
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_OP_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);

		ippRspAttrib.attrib_name[0]	= '\0';

		ippRspAttrib.value.integer = 4;
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);

		ippRspAttrib.value.integer = 8;
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);

		ippRspAttrib.value.integer = 9;
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);

		ippRspAttrib.value.integer = 10;
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);

		ippRspAttrib.value.integer = 11;
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_ENUM, &ippRspAttrib);
		break;
	case PDT_CHARSET_CONFIGED: //"charset-configured"  (R) charset
		strcpy(ippRspAttrib.value.string.text, IPP_DEFAULT_CHARSET);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_CHARSET_CONFIGED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_CHARSET, &ippRspAttrib);
		break;
	case PDT_CHARSET_SUPPORTED:	//"charset-supported" (R) 1setOf charset
		strcpy(ippRspAttrib.value.string.text, IPP_DEFAULT_CHARSET);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_CHARSET_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_CHARSET, &ippRspAttrib);

		strcpy(ippRspAttrib.value.string.text, IPP_SECOND_CHARSET);
		ippRspAttrib.attrib_name[0]	= '\0';
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_CHARSET, &ippRspAttrib);
		break;
	case PDT_LANGUAGE_CONFIGED:	//"natural-language-configured"	(R)	naturalLanguage
		strcpy(ippRspAttrib.value.string.text, IPP_DEFAULT_LANGUAGE);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_LANGUAGE_CONFIGED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_LANGUAGE, &ippRspAttrib);
		break;
	case PDT_LANGUAGE_SUPPORTED: //"generated-natural-language-supported" (R) naturalLanguage
		strcpy(ippRspAttrib.value.string.text, IPP_DEFAULT_LANGUAGE);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_LANGUAGE_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_LANGUAGE, &ippRspAttrib);
		break;
	case PDT_DOC_FORMAT_DEFAULT: //"document-format-default" (R) mimeMediaType
		strcpy(ippRspAttrib.value.string.text, IPP_DEFAULT_DOC_FORMAT);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_DOC_FORMAT_DEFAULT]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_MIMETYPE, &ippRspAttrib);
		break;
	case PDT_DOC_FORMAT_SUPPORTED: //"document-format-supported" (R) mimeMediaType
		strcpy(ippRspAttrib.value.string.text, IPP_DEFAULT_DOC_FORMAT);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_DOC_FORMAT_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_MIMETYPE, &ippRspAttrib);

		strcpy(ippRspAttrib.value.string.text, IPP_SECOND_DOC_FORMAT);
		ippRspAttrib.attrib_name[0]	= '\0';
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_MIMETYPE, &ippRspAttrib);
		break;
	case PDT_ACCEPTING_JOBS: //"printer-is-accepting-jobs" (R) boolean
		if(PrnGetPrinterStatus(port) !=	PrnNoUsed || PrnPortDisConnected(port)) {
			ippRspAttrib.value.boolean = 0;
		} else {
			ippRspAttrib.value.boolean = 1;
		}
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_ACCEPTING_JOBS]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_BOOLEAN, &ippRspAttrib);
		break;
	case PDT_PDL_OVERRIDE_SUPPORTED: //"pdl-override-supported"	(R)	type2 keyword
		strcpy(ippRspAttrib.value.string.text, "not-attempted");
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_PDL_OVERRIDE_SUPPORTED]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_KEYWORD, &ippRspAttrib);
		break;
	case PDT_UP_TIME: //"printer-up-time" (R) integer (1:MAX)
		ippRspAttrib.value.integer = (int32) ((msclock() / 1000) + 1);
		strcpy(ippRspAttrib.attrib_name,PrinterDescriptionTAG[PDT_UP_TIME]);
		ippAddAttribute(&ippObj->RespGroup, IPP_TAG_INTEGER, &ippRspAttrib);
		break;
	case PDT_PRINTER_DESCRIPTION: //"printer-description" ,	group
		break;
	default:
		break;
	}
}

int httpRead(ipp_t *ippObj, BYTE *buffer, int length)
{
	int bytes;
	BYTE  len[64];

	if (ippObj->chunked	&& ippObj->qsize <= 0) {
		//read chunk-size, chunk-extension (if any)	and	CRLF
		if(fgets(len, sizeof(len), ippObj->network) == NULL) {
				return (0);
		}
		ippObj->qsize =	strtol(len,NULL,16);
	}

	if(ippObj->qsize <=	0) {

		//*
		//*	A zero-length chunk	ends a transfer; unless	we are reading POST
		//*	data, go idle...
		//*

		if(ippObj->chunked) {
			ippObj->chunked = 0; //no more data need read !
			fgets(len, sizeof(len), ippObj->network);
		}
		return (0);
	}

/*Jesse	if((bytes = recv(ippObj->s, buffer,
	    ((length > ippObj->qsize) ? ippObj->qsize : length) ,0)) <= 0)
*/	    
	if( (bytes = zot_recv( ippObj->network , buffer , 
		((length > ippObj->qsize) ? ippObj->qsize : length), 0)) <= 0) //Jesse
	{
		bytes = 0;
		ippObj->chunked = 0;
		ippObj->qsize = 0;	  //no more data need read !

		ippObj->ippreq = IPP_PRINT_JOB_TIME_OUT;
	}

	ippObj->qsize -= bytes;
	
	if (ippObj->chunked	&& ippObj->qsize <= 0) {
		fgets(len, sizeof(len), ippObj->network);
	}

	return (bytes);
}

//Number of	bytes read
int ippRead(ipp_t *ippObj, BYTE *buffer, int length)
{
	int Tmplen;
	int bytes	= 0; //Bytes read

	while(length && (Tmplen = httpRead(ippObj,buffer+bytes,length)) != 0) {

		bytes += Tmplen;
		length -= Tmplen;
	}

	return (bytes);
}

void ippAddAttribute(ippBuf *buf, ipp_tag_t tag, ipp_attrib_t *attr)
{
	int16 len;
	int16 n16;
	int32 n32;

	//Add TAG
	ippAppend(buf,(BYTE*)&tag,1);

	//Add Attribute Name Length
	len = strlen(attr->attrib_name);
	n16 = htons(len);
	ippAppend(buf,(BYTE*)&n16,2);

	//Add Attribute Name
	ippAppend(buf,(BYTE*)attr->attrib_name,len);

	switch (tag) {
	case IPP_TAG_INTEGER :
	case IPP_TAG_ENUM :
		//Add Value Length
		n16 = htons(4);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		n32 = htonl(attr->value.integer);
		ippAppend(buf,(BYTE*)&n32,4);
		break;
	case IPP_TAG_BOOLEAN :
		//Add Value Length
		n16 = htons(1);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		ippAppend(buf,(BYTE*)&attr->value.boolean,1);
		break;
	case IPP_TAG_TEXT :
	case IPP_TAG_NAME :
	case IPP_TAG_KEYWORD :
	case IPP_TAG_STRING	:
	case IPP_TAG_URI :
	case IPP_TAG_URISCHEME :
	case IPP_TAG_CHARSET :
	case IPP_TAG_LANGUAGE :
	case IPP_TAG_MIMETYPE :
		//Add Value Length
		len = strlen(attr->value.string.text);
		n16 = htons(len);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		ippAppend(buf,attr->value.string.text,len);
		break;
	case IPP_TAG_DATE :
		//Add Value Length
		n16 = htons(11);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		ippAppend(buf,attr->value.date,11);
		break;
	case IPP_TAG_RESOLUTION	:
		//Add Value Length
		n16 = htons(9);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		n32 = htonl(attr->value.resolution.xres);
		ippAppend(buf,(BYTE*)&n32,4);
		n32 = htonl(attr->value.resolution.yres);
		ippAppend(buf,(BYTE*)&n32,4);
		ippAppend(buf,(BYTE*)&attr->value.resolution.units,1);
		break;
	case IPP_TAG_RANGE :
		//Add Value Length
		n16 = htons(8);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		n32 = htonl(attr->value.range.lower);
		ippAppend(buf,(BYTE*)&n32,4);
		n32 = htonl(attr->value.range.upper);
		ippAppend(buf,(BYTE*)&n32,4);
		break;
	case IPP_TAG_TEXTLANG :
	case IPP_TAG_NAMELANG :
		//len1 XXX len2 YYY len3 ZZZ
		//*
		//*	text-with-language and name-with-language are composite
		//*	values:
		//*
		//*	   charset-length (2)
		//*	   charset
		//*	   text-length	  (2)
		//*	   text
		//*

		//Add Value Length
		len = strlen(attr->value.string.charset) + strlen(attr->value.string.text);
		n16 = htons(len);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		len = strlen(attr->value.string.charset);
		n16 = htons(len);
		ippAppend(buf,(BYTE*)&n16,2);
		ippAppend(buf,attr->value.string.charset,len);

		len = strlen(attr->value.string.text);
		n16 = htons(len);
		ippAppend(buf,(BYTE*)&n16,2);
		ippAppend(buf,attr->value.string.text,len);

		break;
	default	:
		//Add Value Length
    	len = attr->value.unknown.length;
    	n16 = htons(len);
		ippAppend(buf,(BYTE*)&n16,2);

		//Add Value
		ippAppend(buf,(BYTE*)attr->value.unknown.data, len);
		break;
	}//switch (tag)
}

int16 ippSetRespBuf(BYTE *buf, ipp_t *ippObj)
{
	int16 FileSize, len;

	//version
	NSET16(&buf[0],ippObj->Version);
	//status code
	NSET16(&buf[2],htons((WORD)ippObj->RetCode));
	//request id
	NSET32(&buf[4],ippObj->ID);
	//Group = operation
	buf[8] = (BYTE) IPP_TAG_OPERATION;

	//TAG = charset
	buf[9] = (BYTE) IPP_TAG_CHARSET;
	//Attrib Len
	NSET16(&buf[10],htons(18));
	//"attributes-charset"
	memcpy(&buf[12],"attributes-charset", 18);

	if(ippObj->Charset == NULL) {
		//value Len
		NSET16(&buf[30],htons((sizeof(IPP_DEFAULT_CHARSET)-1)));

		//value
		memcpy(&buf[32],IPP_DEFAULT_CHARSET,sizeof(IPP_DEFAULT_CHARSET)-1);
		FileSize = 32 + (sizeof(IPP_DEFAULT_CHARSET)-1);
	} else {
		//value Len
		len = strlen(ippObj->Charset);
		NSET16(&buf[30],htons(len));

		memcpy(&buf[32],ippObj->Charset, len);
		FileSize = 32 +	len;
	}

	//TAG = charset
	buf[FileSize] = (BYTE) IPP_TAG_LANGUAGE;
	FileSize++;

	//Attrib Len
	NSET16(&buf[FileSize],htons(27));
	FileSize += 2;

	//"attributes-natural-language"
	memcpy(&buf[FileSize],"attributes-natural-language", 27);
	FileSize += 27;

	if(ippObj->Language == NULL) {
		//value Len
		NSET16(&buf[FileSize],htons((sizeof(IPP_DEFAULT_LANGUAGE)-1)));
		FileSize += 2;

		//value
		memcpy(&buf[FileSize],IPP_DEFAULT_LANGUAGE,sizeof(IPP_DEFAULT_LANGUAGE)-1);
		FileSize += (sizeof(IPP_DEFAULT_LANGUAGE)-1);
	} else {
		//value Len
		len = strlen(ippObj->Language);
		NSET16(&buf[FileSize],htons(len));
		FileSize += 2;

		memcpy(&buf[FileSize],ippObj->Language,len);
		FileSize += len;
	}

	return (FileSize);
}

void ippAppend(ippBuf *pf, BYTE *data, int16 len)
{
	if((pf)->pf_end + (len) > (pf)->pf_buf + (pf)->pf_len ) {
		morespace( (pf), (data), (len));
	} else {
		bcopy( (data), (pf)->pf_end, (len));
		(pf)->pf_end += (len);
	}
}

BYTE ippCheckPort(BYTE *url, ipp_t *ippObj)
{
	BYTE p[IPP_MAX_PRINTER_URI+IPP_MAX_JOB_URI];
	BYTE q[IPP_MAX_PRINTER_URI+IPP_MAX_JOB_URI];
	BYTE port;

	strcpy(p,IPP_BIN_PRINTER_URI_FORMAT IPP_JOB_URI_FORMAT);
	strcpy(q,IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT IPP_JOB_URI_FORMAT);

	for(port = 0 ; port < NUM_OF_PRN_PORT ; port++) {
		//BINARY DOCUMENT

		p[sizeof(IPP_BIN_PRINTER_URI_FORMAT)-1] = '\0';
		//lp1, lp2, lp3	   (for PRINT-URI)
		p[IPP_PRINTER_PORT_POS] = port + '1';
		if(!stricmp(url,p)) {
			ippObj->DocType = IPP_BINARY_DOC;
		    break;
		}

		// lp1/job..., lp2/job..., lp3/job... (for JOB-URI)
		p[sizeof(IPP_BIN_PRINTER_URI_FORMAT)-1] = '/';
		if(!strnicmp(url,p, (sizeof(IPP_BIN_PRINTER_URI_FORMAT)-1+IPP_JOB_NUM_POS)) ) {
			ippObj->DocType = IPP_BINARY_DOC;
			break;
		}

		//TEXT DOCUMENT

		q[sizeof(IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT)-1] = '\0';
		//lp1_txt, lp2_txt, lp3_txt (for PRINT-URI)
		q[IPP_PRINTER_PORT_POS] = port + '1';
		if(!stricmp(url,q)) {
			ippObj->DocType = IPP_TEXT_DOC;
		    break;
		}

		// lp1_txt/job..., lp2_txt/job..., lp3_txt/job... (for JOB-URI)
		q[sizeof(IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT)-1] = '/';
		if(!strnicmp(url,q, sizeof(IPP_BIN_PRINTER_URI_FORMAT IPP_TXT_PRINTER_EXT_FORMAT)-1+IPP_JOB_NUM_POS)) {
			ippObj->DocType = IPP_TEXT_DOC;
			break;
		}
	}

	return port;
}

#endif IPPD
