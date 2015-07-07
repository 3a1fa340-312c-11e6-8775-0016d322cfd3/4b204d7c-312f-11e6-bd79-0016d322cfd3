#ifndef _HTTPD_H
#define _HTTPD_H

#ifdef IPPD
//ZOT716u2 #define HTTP_MAX_USER  24
#define HTTP_MAX_USER  24
#else
#define HTTP_MAX_USER  12
#endif

#define DEFAULT_PS_NAME  "DEFAULT_PS_NAME\x0"
#define FS_NOT_FOUND     "File Server not found !"
#define FS_BOX_BUSY      "Box Busy !"

//Last modified time : "Sat, 06 Feb 2010 01:50:00 GMT"
#define HTTP_LAST_MODIFIED_TIME 1265421000L

//System time : "Fri, 06 Feb 1998 01:50:00 GMT"
#define	HTTP_SYSTEM_TIME    886729800L

//Expire time : "Sat, 07 Feb 1998 01:50:00 GMT"
#define	HTTP_EXPIRE_TIME    (HTTP_SYSTEM_TIME+60L*60L*24L)  //System Day + 1

// HTTP Method defines
#define METHOD_HEAD     0   // this one must be defined 0
#define METHOD_GET      1
#define METHOD_POST     2
#define METHOD_HTML     3   // used when a cgi func is called from inside a html doc
#define METHOD_UNKNOWN  -1

#define HLINELEN        768+128

struct reqInfo {
//    int index;
    int method;
    int version;
#ifdef IPPD
    long qsize;
#else
    int qsize;
#endif IPPD
    int response;
    char *myname;
    char *url;
    char *arg;
    char *query;
    char *newcheck;
    char *from;
    char *referer;
    char *agent;
    char *passwd;
	char *boundary;
	char *binbuf;
	int  type;
};

// Struct for looking up MIME types from file extensions
struct FileTypes {
	const char *ext;
	const char *type;
};

//struct cgi {
//    char *name; /* name of the cgi program, ie. path portion of the URL */
//    int  (*func)(FILE *network,char *inbuf, char *outbuf, struct reqInfo *rq);
//};

void httpstart (cyg_addrword_t data);
// Created on 4/26/2000	, 5/18/2000.
// Jesse modified this to support 2-byte WebLangVersion at build0007 of 716U2W on August 23, 2011.
extern BYTE WebLangVersion[];

#define EOL_LEN 3

struct _file {
	unsigned cookie;		// Detect bogus file pointers 
#define	_COOKIE	0xdead
	int refcnt;
	struct _file *prev;
	struct _file *next;

	int fd;			// File, socket or asy descriptor 
	long offset;		

	enum {
		_FL_FILE,	// Associated with file 
		_FL_SOCK,	// Associated with network socket 
		_FL_ASY,	// Asynch port 
		_FL_DISPLAY,	// Associated with display driver 
		_FL_PIPE,   // Pipe mode 
		_FL_BUF     // free buffer only 
	} type;

	enum {
		__IOFBF=1,	// Full buffering 
		__IOLBF,		// Line buffering 
		__IONBF		// No buffering 
	} bufmode;		// Output buffering mode 
	
	struct {
		unsigned int err:1;	// Error on stream 
		unsigned int eof:1;	// EOF seen 
		unsigned int ascii:1;	// Ascii (newline translate) mode 
		unsigned int append:1;	// Always seek to end before writing 
		unsigned int tmp:1;	// Delete on close 
		unsigned int partread:1;// Allow partial reads from fread() 
	} flags;
//	struct mbuf *obuf;	// Output buffer 
//	struct mbuf *ibuf;	// Input buffer 
	char *obuf;	// Output buffer 
	char *ibuf;	// Input buffer 
	char *ibuf_temp; // Input buffer temp for free buffer
	int32 ibuf_cnt; // Input buffer read cnt
	char eol[EOL_LEN];	// Text mode end-of-line sequence, if any 
	int32 bufsize;		// Size of buffer to use 
	void *ptr;		// File name or display pointer //
};

typedef struct _file ZOT_FILE;

#endif  _HTTPD_H
