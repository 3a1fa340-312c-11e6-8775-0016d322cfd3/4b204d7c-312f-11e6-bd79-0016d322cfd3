/*
 *	CLIENT routines for Simple Mail Transfer Protocol ala RFC821
 *	A.D. Barksdale Garbee II, aka Bdale, N3EUA
 *	Copyright 1986 Bdale Garbee, All Rights Reserved.
 *	Permission granted for non-commercial copying and use, provided
 *	this notice is retained.
 * 	Modified 14 June 1987 by P. Karn for symbolic target addresses,
 *	also rebuilt locking mechanism
 *	Copyright 1987 1988 David Trulli, All Rights Reserved.
 *	Permission granted for non-commercial copying and use, provided
 *	this notice is retained.
 */
 
 #include <cyg/kernel/kapi.h>
#include <network.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "prnport.h"
#include <setjmp.h>
#include <stdarg.h>
#include "alert.h"

#define		fflush(x)				zot_fflush(x)
#define		fputs(x,y) 				zot_fputs(x,y)
#define 	fgets(x,y,z)			zot_fgets(x,y,z)
#define 	fdopen(x,y)				zot_fopen(x)
#define		fclose(x)     			zot_fclose(x)
#define 	FILE 					ZOT_FILE
#define		Fputc(x,y) 				zot_fputc(x,y)

extern int Network_TCPIP_ON;

//for Email Alert
#ifdef Print_ALERT
uint8 	alertType[NUM_OF_PRN_PORT] = {0};		//1:recover,2:offline
extern uint8 	PortType[NUM_OF_PRN_PORT];		//1:recover,2:offline
extern uint8   adjPortType[NUM_OF_PRN_PORT];
extern uint32	Printing_MarkNum[NUM_OF_PRN_PORT];

extern char 			Hostname[LENGTH_OF_BOX_NAME+1];
extern uint32 	Printing_StartNum[NUM_OF_PRN_PORT];

long Send_Mail_Interval = 90 * 1000; //1.5 Mins

#ifdef Mail_ALERT
static void del_session(struct smtpcli *cb);
static int getresp(struct smtpcli *ftp,int mincode);
void sendcmd(struct smtpcli *cb,char *fmt,...);
static int smtpsendheader(struct smtpcli *cb);
static int smtpsendfile(struct smtpcli *cb);

//int alertSignal = 0;
cyg_sem_t alertSignal;
uint8 	MailPort = 0;
uint8 	Is_Constraint = 0;	//constraint sent error mail
#endif //Mail_ALERT



void PrnPortAlert( BYTE port, BYTE status)
{
	if ( (status == PORT_PAPER_OUT) || (status == PORT_OFF_LINE) ){
		adjPortType[port] = status;
		PortType[port] = ALERT_ERROR;
	} else {
		if ( (Printing_StartNum[port] > 0) && (Printing_MarkNum[port] == Printing_StartNum[port]) ){
			adjPortType[port] = PORT_PAPER_OUT;
			PortType[port] = ALERT_ERROR;
		} else	{
			adjPortType[port] = PORT_READY;
			PortType[port] = ALERT_RECOVERY;
		}
			
		if (Printing_StartNum[port] > 0)
			Printing_MarkNum[port] = Printing_StartNum[port];
		else
			Printing_MarkNum[port] = 0;
	}
}
/*
void
mail_alert(unused,v1,v2)
int unused;
void *v1;
void *v2;
*/
void mail_alert(cyg_addrword_t data)
{
	BYTE 	port, status;
	
//	while( Network_TCPIP_ON == 0 )
//		ppause(100);
	ppause(5000);
			
	while (1){
#ifdef Mail_ALERT
		if(alertType[0] == 0){
			sendmail(NUM_OF_PRN_PORT, 0);
			ppause(Send_Mail_Interval);		//90 sec
			continue;
		}
#endif //Mail_ALERT
		//after send boot ready mail
		
		for (port=0; port<NUM_OF_PRN_PORT; port++)
		{
			status = PrnReadPortStatus( port );
			PrnPortAlert( port, status );
#ifdef Mail_ALERT
			if ( alertType[port] != PortType[port] ){
				sendmail(port,0);
			}
#endif //Mail_ALERT
		}
		
		ppause(Send_Mail_Interval);		//90sec
	}
}

#ifdef Mail_ALERT
//i_status----is inital status
//c_status----is Constraint ststus
void sendmail(int port, uint8 c_status )
{
//Jesse	if (AlertActive && !alertSignal) {
		if (AlertActive ) {
//		alertSignal = 1;
		MailPort = port;
		Is_Constraint = c_status;
//os		ksignal(&alertSignal, 0);
		cyg_semaphore_post( &alertSignal );
	}
}

/* This is the master state machine that handles a single SMTP transaction.
 * It is called with a queue of jobs for a particular host.
 * The logic is complicated by the "Smtpbatch" variable, which controls
 * the batching of SMTP commands. If Smtpbatch is true, then many of the
 * SMTP commands are sent in one swell foop before waiting for any of
 * the responses. Unfortunately, this breaks many brain-damaged SMTP servers
 * out there, so provisions have to be made to operate SMTP in lock-step mode.
 */
/*
void
smtp_alert(unused,v1,v2)
int unused;
void *v1;
void *v2;
*/
void smtp_alert(cyg_addrword_t data)
{
	struct smtpcli *cb;
	struct list *tp;
	struct sockaddr_in fsocket;
	int rcode;
	int rcpts;
	int goodrcpt;
	int i,s;
	uint32 smtpip=0;
	unsigned char smtpdest[32]={0};

	if ((cb = (struct smtpcli *)malloc(sizeof(struct smtpcli))) == NULL)
		return;

	memcpy(smtpdest, EEPROM_Data.AlertAddr, 32);
	cb->network = NULL;
	cb->to = NULL;
	addlist(&cb->to, smtpdest, 0);
	cb->from = strdup(cb->to->val);
	memcpy(smtpdest, EEPROM_Data.SMTPIP, 4);
	smtpip = *(uint32 *)smtpdest;
	//cb->ipdest = resolve(smtpdest);
	cb->ipdest = 0;
	
	while( Network_TCPIP_ON == 0 )
		ppause(100);

loop:
	//while(alertSignal == 0)
//os		kwait(&alertSignal);
		cyg_semaphore_wait(&alertSignal);

//	alertSignal = 1;
//Jesse	cb->lock = 1;
//	kalarm(60000L);     // 1mins
	if (cb->ipdest == 0 && (cb->ipdest = htonl(smtpip)) == 0)
		goto skip;

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = htonl(cb->ipdest);
	fsocket.sin_port = htons(IPPORT_SMTP);

	s = socket(AF_INET,SOCK_STREAM,0);
	if(connect(s,(struct sockaddr *)&fsocket,(sizeof(struct sockaddr))) == 0){
		cb->network = fdopen(s,"r+t");
	} else {
		goto quit;
	}
	rcode = getresp(cb,200);
	if(rcode == -1 || rcode >= 400)
		goto quit;
	/* Say HELO */
	sendcmd(cb,"HELO PSAlert%02X%02X%02X\r\n", EEPROM_Data.EthernetID[3], EEPROM_Data.EthernetID[4],
			        EEPROM_Data.EthernetID[5]);	//,Hostname);
	rcode = getresp(cb,200);
	if(rcode == -1 || rcode >= 400)
		goto quit;

	/* Send MAIL and RCPT commands */
	sendcmd(cb,"MAIL FROM:<%s>\r\n",cb->from);
	rcode = getresp(cb,200);
	if(rcode == -1 || rcode >= 400)
		goto quit;
	rcpts = 0;
	goodrcpt = 0;
	for (tp = cb->to; tp != NULL; tp = tp->next){
		sendcmd(cb,"RCPT TO:<%s>\r\n",tp->val);
		rcode = getresp(cb,200);
		if(rcode == -1)
			goto quit;
		if(rcode < 400)
			goodrcpt = 1; /* At least one good */
		rcpts++;
	}
	/* Send DATA command */
	sendcmd(cb,"DATA\r\n");
	rcode = getresp(cb,200);
	if(rcode == -1 || rcode >= 400)
		goto quit;

	smtpsendheader(cb);
	smtpsendfile(cb);

	/* Wait for the OK response */
	rcode = getresp(cb,200);
	if(rcode == -1)
		goto quit;

	if (alertType[0] == 0){
		for( i=0;i<NUM_OF_PRN_PORT;i++){
			alertType[i] = ALERT_RECOVERY;
			PortType[i] = ALERT_RECOVERY;
		}
	} else
		if (!Is_Constraint)
			alertType[MailPort] = (3-alertType[MailPort]);
			
quit:
	if(cb->network)
		sendcmd(cb,"QUIT\r\n");
	
	if(cb->network)
		(void) fclose(cb->network);
	else
		(void) close(s);		
skip:

	cb->network = 0;
//	kalarm(0L);
	cb->lock = 0;
//	alertSignal = 0;

	ppause(1000L);
	goto loop;

	del_session(cb);
}

/* free the message struct and data */
static void
del_session(cb)
register struct smtpcli *cb;
{

	if (cb == NULL)
		return;
	free(cb->from);
	del_list(cb->to);
	free(cb);
}

static int
smtpsendheader(cb)
register struct smtpcli *cb;
{
	int error = 0;
    long t;
	char *cp;

	sendcmd(cb, "From: %s <alert@PSAlert>\r\n", Hostname );
	sendcmd(cb, "To: %s\r\n", cb->to->val);
	if( (!Is_Constraint) && ((alertType[0] == 0)|| (alertType[MailPort] == 2)) )
		sendcmd(cb, "Subject: Recovery Message!!!\r\n");
	else
		sendcmd(cb, "Subject: ERROR Message!!!\r\n");

    time(&t);
    cp = ctime(&t);
    rip(cp);
	sendcmd(cb, "Sent: %s\r\n",cp);
	sendcmd(cb, "\r\n");
	sendcmd(cb, "\r\n");
	
	return error;
}

static int
smtpsendfile(cb)
register struct smtpcli *cb;
{
	int error = 0;
	uint8 mail_status = 0;
	
	strcpy(cb->buf,"\r\n");

	if(strcmp(cb->buf,".\n") == 0)
		Fputc('.',cb->network);
		
	if (alertType[0] == 0)
		sendcmd(cb, "$( %s ) %d port PrintServer is ready\r\n", EEPROM_Data.BoxName, MailPort);
	else
	{
		if (!Is_Constraint)
			mail_status = alertType[MailPort];
			
		switch(mail_status)
		{
		case 1:		//current ststus : ALERT_RECOVERY
			sendcmd(cb, "$( %s ) port %d is out of paper or (offline)\r\n", EEPROM_Data.BoxName, MailPort+1);
			break;
		case 2:		//current ststus : ALERT_ERROR
			sendcmd(cb, "$( %s ) port %d is ready\r\n", EEPROM_Data.BoxName, MailPort+1);
			break;
		default:
			sendcmd(cb, "$( %s ) MAC is reset\r\n", EEPROM_Data.BoxName);
			break;
		}
	}
	fflush(cb->network);
	/* Send the end-of-message command */
	if(cb->buf[strlen(cb->buf)-1] == '\n')
		sendcmd(cb,".\r\n");
	else
		sendcmd(cb,"\r\n.\r\n");
	fflush(cb->network);
	
	return error;
}

#if 0	// Backup these 2 functions for testing, change from fprintf to sendcmd - Kevin [03/01/2005]
static int
smtpsendheader(cb)
register struct smtpcli *cb;
{
	int error = 0;
    long t;
	char *cp;

	fprintf(cb->network, "From: %s <alert@PSAlert>\n", Hostname );
	fprintf(cb->network, "To: %s\n", cb->to->val);
	if( (!Is_Constraint) && ((alertType[0] == 0)|| (alertType[MailPort] == 2)) )
		fprintf(cb->network, "Subject: Recovery Message!!!\n");
	else
		fprintf(cb->network, "Subject: ERROR Message!!!\n");

    time(&t);
    cp = ctime(&t);
    rip(cp);
	fprintf(cb->network, "Sent: %s\n",cp);
	fprintf(cb->network, "\n");
	fprintf(cb->network, "\n");
	return error;
}

static int
smtpsendfile(cb)
register struct smtpcli *cb;
{
	int error = 0;
	uint8 mail_status = 0;
	
	strcpy(cb->buf,"\n");

	if(strcmp(cb->buf,".\n") == 0)
		Fputc('.',cb->network);
		
	if (alertType[0] == 0)
		fprintf(cb->network, "$( %s ) %d port PrintServer is ready\n", EEPROM_Data.BoxName, MailPort);
	else
	{
		if (!Is_Constraint)
			mail_status = alertType[MailPort];
			
		switch(mail_status)
		{
		case 1:		//current ststus : ALERT_RECOVERY
			fprintf(cb->network, "$( %s ) port %d is out of paper or (offline)\n", EEPROM_Data.BoxName, MailPort+1);
			break;
		case 2:		//current ststus : ALERT_ERROR
			fprintf(cb->network, "$( %s ) port %d is ready\n", EEPROM_Data.BoxName, MailPort+1);
			break;
		default:
			fprintf(cb->network, "$( %s ) MAC is reset\n", EEPROM_Data.BoxName);
			break;
		}
	}
	fflush(cb->network);
	/* Send the end-of-message command */
	if(cb->buf[strlen(cb->buf)-1] == '\n')
		sendcmd(cb,".\r\n");
	else
		sendcmd(cb,"\r\n.\r\n");
	fflush(cb->network);
	return error;
}

#endif


/* do a printf() on the network stream with optional local tracing */
void
sendcmd(struct smtpcli *cb,char *fmt, ...)
{
	va_list args;

	va_start(args,fmt);
	vsprintf(cb->buf,fmt,args);
	if( cb->network != 0)
		fputs(cb->buf,cb->network);
	
	va_end(args);
}

/* Wait for, read and display response from server. Return the result code. */
/* Keep reading until at least this code comes back -- for mincode*/
static int
getresp(struct smtpcli *cb ,int mincode) {

	int rval;
	char line[LINELEN];

	fflush(cb->network);
	for(;;){
		/* Get line */
		if(fgets(line,LINELEN,cb->network) == NULL){
			rval = -1;
			break;
		}
		rip(line);		/* Remove cr/lf */
		rval = atoi(line);
		/* Messages with dashes are continued */
		if(line[3] != '-' && rval >= mincode)
			break;
	}
	return rval;
}

/* add an element to the front of the list pointed to by head 
** return NULL if out of memory.
*/
struct list *
addlist(head,val,type)
struct list **head;
char *val;
int type;
{
	register struct list *tp;

	tp = (struct list *)callocw(1,sizeof(struct list));
	tp->next = NULL;
	/* allocate storage for the char string */
	tp->val = strdup(val);
	tp->type = type;

	/* add entry to front of existing list */
	if (*head == NULL)
		*head = tp;
	else {
		tp->next = *head;
		*head = tp;
	}
	return tp;
}

/* delete a list of list structs */
void
del_list(lp)
struct list *lp;
{
	register struct list *tp, *tp1;
	for (tp = lp; tp != NULL; tp = tp1) {
		tp1 = tp->next;
		free(tp->val);
		free(tp);
	}
}
#endif //Mail_ALERT
#endif //Print_ALERT
