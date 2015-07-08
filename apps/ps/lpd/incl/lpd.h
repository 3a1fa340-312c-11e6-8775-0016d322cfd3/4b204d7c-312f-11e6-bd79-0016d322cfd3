/* Internet LPD Server definitions
 *   written by David Johnson (dave@cs.olemiss.edu)
 *
 * This code is in the public domain.
 *
 * Revision History:
 *
 * Revision 1.3  91/10/93  dave
 * Added pointer to LPD structure in LPDdevice so status message can be
 *     placed there during filter use
 *
 * Revision 1.2  91/09/27  dave (from Hans-Juergen)
 * Changed default directory and file definitions to allow setting of
 *     NOS logical root directory
 *
 * Revision 1.1  91/09/17  dave
 * Added output processing flags to device structure
 *
 * Revision 1.0  91/09/04  dave
 * Initial Release
 *
 */
#ifndef	_LPD_H
#define	_LPD_H

//#include <stdio.h>
#include "httpd.h"
#define FILE 			ZOT_FILE

/*
 * Defaults for various LPD system files
 */
#define D_LOCK		"lock"		/* spool lock file */
#define D_STATUS	"status"	/* spool status file */


/*
 * Defaults for line printer capabilities database
 */
#define D_BAUDRATE	9600
#define D_DEVLP		"LPT1"
#define D_MAXCOPIES	0
#define D_MAXJOBSIZE	0
#define D_FF		"\f"
#define D_WIDTH		132
#define D_LENGTH	66
#define D_PWIDTH	0	/* page width in pixels */
#define D_PLENGTH	0	/* page length in pixels */
#define D_IF		"text"	/* IF filter */
#define D_OF		"text"	/* OF filter */
#define D_FILTERS	"flp"	/* allowed filter formats */

#define MAX_PARMS	64	/* maximum parameters from lprm */
#define MAX_JOBFILES	64	/* maximum number of files per job */

/* internal interface filter processing flags */
#define CRMOD		0x01
#define XTABS		0x02

/* these two symbol had been defined in global.h, Simon 12/3/97 */
#undef FF
#undef LF

/*
 * LPD printcap storage
 */
struct LPDpr {
	char buffer[512];
	char strings[256];
	int   AB;		/* always print banner? */
	char *AF;		/* accounting file name */
	int   BR;		/* baud rate */
	char *CF;		/* cifplit filter */
	char *CM;		/* comment */
	char *DF;		/* dvi filter */
	int   DP;		/* default to postscript mode */
	char *FF;		/* form-feed string */
	char *FL;		/* file leader */
	int   FO;		/* print form-feed on open? */
	int   FQ;		/* print form-feed on quit? */
	char *FT;		/* file trailer */
	char *FX;		/* filters allowed */
	char *GF;		/* plot filter */
	int   HL;		/* print header last? */
	char *IF;		/* text filter */
	char *JL;		/* job leader */
	char *JT;		/* job trailer */
	char *LD;		/* leader, print on open */
#ifdef LPD_DEBUG
	char *LF;		/* log file */
#endif
	char *LP;		/* printer device */
	int   MC;		/* maximum number of copies allowed */
	int   MX;		/* maximum job size in K */
	char *NF;		/* ditroff filter */
	char *OF;		/* banner filter */
	char *PE;		/* postscript end */
	int   PL;		/* page length */
	int   PW;		/* page width */
	char *PS;		/* postscript start */
	int   PX;		/* page width in pixels */
	int   PY;		/* page length in pixels */
	int   SB;		/* short banner */
	int   SC;		/* suppress multiple copies */
	char *SD;		/* spool directory */
	int   SF;		/* suppress form-feeds */
	int   SH;		/* suppress header */
	int   SS;		/* single-sheet? */
	char *TF;		/* troff filter */
	char *TR;		/* trailer, print on close */
	char *TY;		/* stty parameters */
	char *VF;		/* raster filter */
};

struct LPDxlat {
	struct {				/* translations */
		char *dfname;
		char *dos_dfname;
	} xlat[MAX_JOBFILES];
};

struct LPDtrans {
	int PortNumber;	//Add by Simon , Print Port number (0 - n)
	int remote;		// remote host socket
	FILE * network; //associate with remote host socket for FILE */

/*Jesse
#if 0
	struct LPDstatus *status;
#endif
*/
#ifdef LPD_DEBUG
	FILE *logfp;		/* log file fp */
#endif

	/*
	 * Control file is stored here until translations can be made.
	 */
/*Jesse	 
#if 0
	char *cf_data;				// actual control file 
	struct LPDxlat *xlat_table;		// translations 
	struct LPDjob *job;	// current job information 
	struct LPDpr *pc;	// printcap definitions 
	struct LPDdevice *device;
#endif
*/
};

/*
 * Created to hold information extracted from control file for each job.
 */
struct LPDjob {
	char *class;		/* job class */
	char *date;		/* date submitted */
	char *hostname;		/* originating host name */
	int  indent;		/* amount to indent output */
	char *jobname;		/* job name printed on banner page */
	char *username;		/* name to print on banner */
	char *mail;		/* mail to user when done */
	char *name;		/* name of file in job */
	char *person;		/* user's login name */
	char *noheader;		/* no header request */
	char *account;		/* accounting information */
	int  width;		/* page width for pr */
	char *title;		/* title for pr */

	unsigned long size;	/* job size */
};

/*
 * One per supported device.  Used to synchronize unspoolers using same
 * device.
 */
struct LPDdevice {
	char *name;		/* device name */
	unsigned int busy;	/* device busy? */
	unsigned int flags;	/* output processing flags */
	struct LPDtrans *lpd;	/* pointer to associated job information */
};

/*
 * Unspooler status structure
 */
struct LPDstatus {
	char *name;		/* printer (queue) name */
	unsigned int device_open;	/* device open? */

/*Jesse
#if 0
	unsigned int flags;
	unsigned int next_seq;	// next sequence number to receive 
#endif
*/
/*Jesse
#if 0
	char type;
#define INTERNAL	0
#define EXTERNAL	1
	union {
		FILE *fp;
		struct iface *ifp;
	} device;
	unsigned int jobno;
	unsigned int busy;
	struct proc *unspooler_proc;
	char *current_job;	// name of current job control file 
	char *message;		// current status message 
#endif
*/
};

/*
 * Job entry structure for in memory copy of print queue
 */
struct job_entry {
	char	*j_name;
	time_t	j_time;
	int	j_priority;
};


#endif	/* _LPD_H */


