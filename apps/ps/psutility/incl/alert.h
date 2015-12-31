#ifndef	_ALERT_H
#define	_ALERT_H

#ifdef Print_ALERT
extern uint8 	alertType[NUM_OF_PRN_PORT];
extern uint8  	PortType[NUM_OF_PRN_PORT];
extern uint8   	adjPortType[NUM_OF_PRN_PORT];
#define ALERT_RECOVERY		1
#define ALERT_ERROR			2

#ifdef Mail_ALERT

#define MAXSESSIONS	10		/* most connections allowed */
#define JOBNAME		13		/* max size of a job name with null */
#define	LINELEN		256
#define SLINELEN	64
#define MBOXLEN		8		/* max size of a mail box name */

/* types of address used by smtp in an address list */
#define SMTP_BADADDR	0
#define SMTP_LOCAL		1
#define SMTP_DOMAIN		2

/* a list entry */
struct list {
	struct list *next;
	char *val;
	char type;
};

/* control structure used by an smtp client session */
struct smtpcli {
	FILE    *network;  	/* The network stream for this connection */
	uint32	ipdest;		/* address of forwarding system */
	char	*tname;		/* name of data file */
	char	buf[LINELEN];	/* Output buffer */
	char	cnt;		/* Length of input buffer */
	FILE	*tfile;
	char	*from;		/* address of sender */
	struct list *to;	/* Linked list of recipients */
	int     lock;
};

/* In ALERT.c: */
struct list *addlist(struct list **head,char *val,int type);
void del_list(struct list *lp);
//void smtp_alert(int unused,void *v1,void *v2);
void smtp_alert(cyg_addrword_t data);
void sendmail(int port,uint8 c_status);
#endif //Mail_ALERT

//void mail_alert(int unused,void *v1,void *v2);
void mail_alert(cyg_addrword_t data);
#endif //Print_ALERT

#endif	/* _ALERT_H */

