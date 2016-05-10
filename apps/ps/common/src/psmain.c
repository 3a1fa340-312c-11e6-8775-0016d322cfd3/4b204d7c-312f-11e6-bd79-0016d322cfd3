#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "psdefine.h"
#include "prnqueue.h"
#include "prnport.h"
#include "nps.h"
#include "star_timer.h"

#ifndef USE_NETAPP_LIBS
#undef SMBD
#undef TELNETD
#endif /* !NETAPP_LIBS */

#ifndef USE_PS_LIBS
#undef SNMPD
#undef LPD_TXT
#undef NOVELL_PS
#undef NDS_PS
#undef ATALKD
#undef Mail_ALERT
#undef Print_ALERT
#undef RENDEZVOUS
#undef RAWTCPD
#undef UNIXUTIL_TFTP
#undef TELNETD
#endif /* !USE_PS_LIBS */

cyg_sem_t ATD_INIT_OK;

/////////////////////////Utility/////////////////////////////////////////
inline void sti(void)
{
	cyg_interrupt_enable();
}

inline void cli(void)
{
	cyg_interrupt_disable();
}

inline void restore(cyg_uint32  old_intr)
{
	HAL_RESTORE_INTERRUPTS(old_intr)		
}

inline int dirps()
{
	cyg_uint32  old_intr;
	HAL_DISABLE_INTERRUPTS(old_intr);
	return old_intr;
}

uint32 rdclock()
{
	uint32 pvalue;

	pvalue = cyg_current_time();
//	pvalue = get_current_time();
	return ((uint32)pvalue);
	
}

uint32 msclock( void )
{	
	return jiffies;
}


/* Generate a uniformly-distributed random number between 0 and n-1
 * Uses rejection method
 */
int urandom(unsigned int n)
{
    if(n == 0) n = 255;
	return (rand() % n);
}

uint16 NGET16( uint8 *pSrc )
{
	return (uint16)( pSrc[0] | pSrc[1]<<8 );
}

void NSET16( uint8 *pDest, uint16 value )
{
	pDest[0] = value;
	pDest[1] = (value>>8);
}

uint32 NGET32( uint8 *pSrc )
{
	return (uint32)( pSrc[0] | pSrc[1]<<8 | pSrc[2]<<16 | pSrc[3]<<24 );
}

uint32 get16(uint8 *cp)
{
	uint32 x;

	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
}

/* Machine-independent, alignment insensitive network-to-host long conversion */
uint32 get32(uint8 *cp)
{
	uint32 rval;

	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;

	return rval;
}

void NSET32( uint8 *pDest, uint32 value )
{
	pDest[0] = value;
	pDest[1] = (value>>8);
	pDest[2] = (value>>16);
	pDest[3] = (value>>24);
}

void NCOPY32( uint8 *pDest, uint8 *pSrc )
{
	pDest[0] = pSrc[0];
	pDest[1] = pSrc[1];
	pDest[2] = pSrc[2];
	pDest[3] = pSrc[3];
}

#if 0 //defined(DWP2020)
/* Convert Internet address in ascii dotted-decimal format (44.0.0.1) to
 * binary IP address
 */
int32
aton(char *s)
{
    int32 n;

    register int i;

    n = 0;
    if(s == NULL)
        return 0;
    for(i=24;i>=0;i -= 8){
    /* Skip any leading stuff (e.g., spaces, '[') */
        while(*s != '\0' && !isdigit(*s))
            s++;
        if(*s == '\0')
            break;
        n |= (int32)atoi(s) << i;
        if((s = strchr(s,'.')) == NULL)
            break;
        s++;
    }
    return n;
}
#endif // defined(DWP2020)

//ZOT==>
void __spin_lock_init(void *lock){
	cyg_mutex_t * mut_t = lock;
	cyg_mutex_init(mut_t);	
}

void spin_unlock_irqrestore(void *lock, unsigned long *flagg)
{
	cyg_mutex_t * mut_t = lock;
	cyg_mutex_unlock(mut_t);
	restore(*flagg);
}

void spin_lock_irqsave(void *lock, unsigned long *flagg)
{
	cyg_mutex_t * mut_t = lock;
	*flagg = dirps();
	cyg_mutex_lock(mut_t);
}

void spin_unlock_irq(void *lock)
{
	cyg_mutex_t * mut_t = lock;
	cyg_mutex_unlock(mut_t);
	cyg_interrupt_enable();
}

void spin_lock_irq(void *lock)
{
	cyg_mutex_t * mut_t = lock;
	cyg_interrupt_disable();
	cyg_mutex_lock(mut_t);
}

void spin_unlock(void *lock)
{
	cyg_mutex_t * mut_t = lock;
	cyg_mutex_unlock(mut_t);
}

void spin_lock(void *lock)
{
	cyg_mutex_t * mut_t = lock;
	cyg_mutex_lock(mut_t);
}

int memicmp(
        const void * first,
        const void * last,
        unsigned int count
        )
{
        int f = 0;
        int l = 0;
#ifdef _MT
        int local_lock_flag;
#endif  /* _MT */

#if defined (_WIN32)
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif  /* defined (_WIN32) */
            while ( count-- )
            {
                if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                     ((f = _TOLOWER( *(unsigned char *)first )) ==
                      (l = _TOLOWER( *(unsigned char *)last ))) )
                {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
                }
                else
                    break;
            }
#if defined (_WIN32)
        }
        else {
            _lock_locale( local_lock_flag )
            while ( count-- )
                if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                     ((f = _tolower_lk( *(unsigned char *)first )) ==
                      (l = _tolower_lk( *(unsigned char *)last ))) )
                {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
                }
                else
                    break;
            _unlock_locale( local_lock_flag )
        }
#endif  /* defined (_WIN32) */

        return ( f - l );
}

/* Case-insensitive string comparison */
int strnicmp(char *a,char *b,int n)
{
	char a1,b1;

	while(n-- != 0 && (a1 = *a++) != '\0' && (b1 = *b++) != '\0'){
		if(a1 == b1)
			continue;	/* No need to convert */
		a1 = tolower(a1);
		b1 = tolower(b1);
		if(a1 == b1)
			continue;	/* NOW they match! */
		if(a1 > b1)
			return 1;
		if(a1 < b1)
			return -1;
	}
	return 0;
}

int stricmp(char *a ,char *b)
{
	int f,l;
	do{
		if ( ((f = (unsigned char)(*(a++))) >= 'A') && (f <= 'Z') )
	    	f -= ('A' - 'a');
	
	    if ( ((l = (unsigned char)(*(b++))) >= 'A') && (l <= 'Z') )
	        l -= ('A' - 'a');
	    
	    if(f > l)
			return 1;
		
		if(f < l)
			return -1;    
	        
	} while ( f && (f == l) );

	return 0;	
}

char * strupr ( char * string )
{
	char * cp;

    for (cp=string; *cp; ++cp)
    {
        if ('a' <= *cp && *cp <= 'z')
            *cp += 'A' - 'a';
    }

    return(string);
}

char* strdup(const char *str)			//615wu-spooler-temp
{
  char *x = malloc (strlen (str) + 1);
  if (x != NULL)
    strcpy (x, str);
  return x;
}

/* Version of malloc() that waits if necessary for memory to become available */
void *mallocw(size_t nb)
{
	void *p;

	if( nb == 0 )
		return NULL;
		
	while((p = malloc(nb)) == NULL){
		ppause(10);
	}
	return p;
}

/* Version of calloc that waits if necessary for memory to become available */
void *
callocw(nelem,size)
unsigned nelem;	/* Number of elements */
unsigned size;	/* Size of each element */
{
	register unsigned i;
	register char *cp;

	i = nelem * size;
	cp = mallocw(i);
	memset(cp,0,i);
	return cp;
}

extern unsigned int APB_clock;
#define MHZ	1000000
void udelay( int x )
{
#if 0	
	unsigned int delay, us = x*2;
	
	for( delay = 0; delay < us ; delay++ );
#endif

	unsigned int control_value, value; 

	TIMER2_COUNTER_REG = ((APB_clock / MHZ ) * x);
	
	control_value = TIMER1_TIMER2_CONTROL_REG;
	control_value |= (1 << TIMER2_ENABLE_BIT_INDEX);
	TIMER1_TIMER2_CONTROL_REG = control_value;
	
    do {
		value = TIMER2_COUNTER_REG;

		// Jesse unmarked this at build0006 in 716U2W on April 27, 2011.
		//  If unmarked, the system will hang when it is printing and we unplug the USB cable.
		//  If marked, the system will not hang and eat data elapsed.
		//	This is for 716U2W only.
		//cyg_thread_yield();
		
    } while (value != 0);

	control_value = TIMER1_TIMER2_CONTROL_REG;
	control_value &= ~(1 << TIMER2_ENABLE_BIT_INDEX);
	TIMER1_TIMER2_CONTROL_REG = control_value;	

}

/////////////////////////PS Main/////////////////////////////////////////

extern void NT3main(cyg_addrword_t data);
#ifdef USE_ADMIN_LIBS
extern void NTUtilityUDP(cyg_addrword_t data);
#endif
extern void IPXCounter_thread(cyg_addrword_t data);
#ifdef SMBD	
extern void SMBInit(cyg_addrword_t data);
#endif SMBD
extern void lpdstart(cyg_addrword_t data);
extern void rawtcpd(cyg_addrword_t data);
#ifdef NOVELL_PS
extern void NovellPSmain(cyg_addrword_t data);	//615wu::No PSMain
extern void NovellPrintSocketInit(void);
extern void SendNovellSAP(cyg_addrword_t data);
extern void NovellPrintServerInit (void);
extern void BinderyPrintServerInit (void);
extern void ConnectNetware(cyg_addrword_t data);
#endif NOVELL_PS
#ifdef NDS_PS
extern void NDSmain(cyg_addrword_t data);
#endif NDS_PS
#if defined(HTTPD) && !defined(CODE1)
extern BYTE HttpdUsed;
#endif HTTPD
extern BYTE IntoNPS3main;	  //8/26/98
#ifdef ATALKD
void atalkd_init(cyg_addrword_t data);
void papd(cyg_addrword_t data);
void aep_input(cyg_addrword_t data);
void nbp_input(cyg_addrword_t data);
void zip_info_query(cyg_addrword_t data);
#endif ATALKD
#ifdef SNMPD
void snmpd(cyg_addrword_t data);
#endif SNMPD
#ifdef RENDEZVOUS
//int  Rendezous_init(void); //Ron
extern void Rendezous_init(cyg_addrword_t data);
#endif
extern void PhyMonitor(void);

#ifdef UNIXUTIL_TFTP
extern void tftpd(cyg_addrword_t data);
#endif //UNIXUTIL_TFTP

#ifdef Mail_ALERT
extern void smtp_alert(cyg_addrword_t data);
#endif //Mail_ALERT

#ifdef Print_ALERT
extern void mail_alert(cyg_addrword_t data);
#endif //Print_ALERT

#ifdef TELNETD
extern void telnetstart(cyg_addrword_t data);
#endif //TELNETD

//Print Server working platform mode Configuration
//PSMode Bitmap:
//Bit 0 : Netware mode supported
//Bit 1 : UNIX mode supported
//Bit 2 : Windows mode supported
//Bit 3 : Apple Talk mode supported
//Bit 4 : NDS mode supported
//Bit 5 : IPP mode supported
//Bit 6 : SMB mode supported
//Bit 7 : DHCP On
extern  uint8 PSMode;
//PSMode2 Bitmap:
//Bit 0 : RAWTCP mode supported 
//Bit 1 : FTP mode supported
//Bit 2 : WEBJETADMIN On
//Bit 3 : N/A
//Bit 4 : N/A
//Bit 5 : N/A
//Bit 6 : N/A
//Bit 7 : CENTRAL PRINT
extern	uint8 PSMode2;

//NT3MAIN Task create information definition
#define NT3MAIN_TASK_PRI         20	//ZOT716u2
#define NT3MAIN_TASK_STACK_SIZE	 2048 //ZOT716u2 3072
static uint8 			NT3MAIN_Stack[NT3MAIN_TASK_STACK_SIZE];
static cyg_thread       NT3MAIN_Task;
static cyg_handle_t     NT3MAIN_TaskHdl;

//NTUDP Task create information definition
#define NTUDP_TASK_PRI         20	//ZOT716u2
#define NTUDP_TASK_STACK_SIZE  2048 //ZOT716u2 4096
static uint8 			NTUDP_Stack[NTUDP_TASK_STACK_SIZE];
static cyg_thread       NTUDP_Task;
static cyg_handle_t     NTUDP_TaskHdl;

//615wu::No PSMain
//NovellPSMAIN Task create information definition
#define NovellPSMAIN_TASK_PRI         20	//ZOT716u2
#define NovellPSMAIN_TASK_STACK_SIZE	1024
static uint8 			NovellPSMAIN_Stack[NovellPSMAIN_TASK_STACK_SIZE];
static cyg_thread       NovellPSMAIN_Task;
static cyg_handle_t     NovellPSMAIN_TaskHdl;

//Novell-SAP Task create information definition
#define NSAP_TASK_PRI         20	//ZOT716u2
#define NSAP_TASK_STACK_SIZE  2048
static uint8 			NSAP_Stack[NSAP_TASK_STACK_SIZE];
static cyg_thread       NSAP_Task;
static cyg_handle_t     NSAP_TaskHdl;

//Novell-Connect Task create information definition
#define NCON_TASK_PRI         20	//ZOT716u2
#define NCON_TASK_STACK_SIZE  2048
static uint8 			NCON_Stack[NCON_TASK_STACK_SIZE];
static cyg_thread       NCON_Task;
static cyg_handle_t     NCON_TaskHdl;


//Novell-NDS Task create information definition
#define NDS_TASK_PRI         20	//ZOT716u2
#define NDS_TASK_STACK_SIZE  2048
static uint8 			NDS_Stack[NDS_TASK_STACK_SIZE];
static cyg_thread       NDS_Task;
static cyg_handle_t     NDS_TaskHdl;

//lpdStart Thread initiation information
#define LpdStart_TASK_PRI         	20	//ZOT716u2
#define LpdStart_TASK_STACK_SIZE  	2048 //ZOT716u2 4096
static	uint8			LpdStart_Stack[LpdStart_TASK_STACK_SIZE];
static  cyg_thread		LpdStart_Task;
static  cyg_handle_t	LpdStart_TaskHdl;

//rawtcpd Thread initiation information
#define Rawtcpd_TASK_PRI         	20	//ZOT716u2
#define Rawtcpd_TASK_STACK_SIZE  	2048 //ZOT716u2 4096
static	uint8			Rawtcpd_Stack[Rawtcpd_TASK_STACK_SIZE];
static  cyg_thread		Rawtcpd_Task;
static  cyg_handle_t	Rawtcpd_TaskHdl;

//SMBInit Thread initiation information
#define SMBInit_TASK_PRI         	20	//ZOT716u2
#define SMBInit_TASK_STACK_SIZE  	1024 //ZOT716u2 3072
static	uint8			SMBInit_Stack[SMBInit_TASK_STACK_SIZE];
static  cyg_thread		SMBInit_Task;
static  cyg_handle_t	SMBInit_TaskHdl;

//ATD Thread initiation information
#define ATD_TASK_PRI         	20	//ZOT716u2
#define ATD_TASK_STACK_SIZE  	2048 //ZOT716u2 3852
static	uint8			ATD_Stack[ATD_TASK_STACK_SIZE];
static  cyg_thread		ATD_Task;
static  cyg_handle_t	ATD_TaskHdl;

//PAPD1 Thread initiation information
#define PAPD1_TASK_PRI         	20	//ZOT716u2
#define PAPD1_TASK_STACK_SIZE  	1024 //ZOT716u2 8192
static	uint8			PAPD1_Stack[PAPD1_TASK_STACK_SIZE];
static  cyg_thread		PAPD1_Task;
static  cyg_handle_t	PAPD1_TaskHdl;

//AEP Thread initiation information
#define AEP_TASK_PRI         	20	//ZOT716u2
#define AEP_TASK_STACK_SIZE  	2048 //ZOT716u2 8192
static	uint8			AEP_Stack[AEP_TASK_STACK_SIZE];
static  cyg_thread		AEP_Task;
static  cyg_handle_t	AEP_TaskHdl;

//NBP Thread initiation information
#define NBP_TASK_PRI         	20	//ZOT716u2
#define NBP_TASK_STACK_SIZE  	2048 //ZOT716u2 8192
static	uint8			NBP_Stack[NBP_TASK_STACK_SIZE];
static  cyg_thread		NBP_Task;
static  cyg_handle_t	NBP_TaskHdl;

//ZIP Thread initiation information
#define ZIP_TASK_PRI         	20	//ZOT716u2
#define ZIP_TASK_STACK_SIZE  	1024 //ZOT716u2 10240
static	uint8			ZIP_Stack[ZIP_TASK_STACK_SIZE];
static  cyg_thread		ZIP_Task;
static  cyg_handle_t	ZIP_TaskHdl;
uint8 ZIP_WAIT;

//SNMPD Thread initiation information
#define SNMPD_TASK_PRI         	20	//ZOT716u2
#define SNMPD_TASK_STACK_SIZE  	2048//ZOT716u2 4096
static	uint8			SNMPD_Stack[SNMPD_TASK_STACK_SIZE];
static  cyg_thread		SNMPD_Task;
static  cyg_handle_t	SNMPD_TaskHdl;

#ifdef RENDEZVOUS
//Rendezous Thread initiation information
#define Rendezous_TASK_PRI         	20	//ZOT716u2
#define Rendezous_TASK_STACK_SIZE  	2048
static	uint8			Rendezous_Stack[Rendezous_TASK_STACK_SIZE];
static  cyg_thread		Rendezous_Task;
static  cyg_handle_t	Rendezous_TaskHdl;
#endif //RENDEZVOUS

//TFTPD Thread initiation information
#define TFTPD_TASK_PRI         	20	//ZOT716u2
#define TFTPD_TASK_STACK_SIZE  	2048
static	uint8			TFTPD_Stack[TFTPD_TASK_STACK_SIZE];
static  cyg_thread		TFTPD_Task;
static  cyg_handle_t	TFTPD_TaskHdl;

//SMTP ALERT Thread initiation information
#define SMTP_ALERT_TASK_PRI         	20	//ZOT716u2
#define SMTP_ALERT_TASK_STACK_SIZE  	1024
static	uint8			SMTP_ALERT_Stack[SMTP_ALERT_TASK_STACK_SIZE];
static  cyg_thread		SMTP_ALERT_Task;
static  cyg_handle_t	SMTP_ALERT_TaskHdl;

//MAIL ALERT Thread initiation information
#define MAIL_ALERT_TASK_PRI         	20	//ZOT716u2
#define MAIL_ALERT_TASK_STACK_SIZE  	1024
static	uint8			MAIL_ALERT_Stack[MAIL_ALERT_TASK_STACK_SIZE];
static  cyg_thread		MAIL_ALERT_Task;
static  cyg_handle_t	MAIL_ALERT_TaskHdl;

//TELNET Thread initiation information
#define TELNET_TASK_PRI         	20	//ZOT716u2
#define TELNET_TASK_STACK_SIZE  	2048
static	uint8			TELNET_Stack[TELNET_TASK_STACK_SIZE];
static  cyg_thread		TELNET_Task;
static  cyg_handle_t	TELNET_TaskHdl;

extern uint32 get_current_time();
extern cyg_sem_t rendezvous_sem;

//Print Server initialization 
void ps_init(void)
{
//...........Add Supported Function Here...........//

//////// Supported Windows Print Server
#ifdef USE_PS_LIBS
	NTPrinterServerInit();
	
	//Create NT3main Thread :: for port 1
    cyg_thread_create(NT3MAIN_TASK_PRI,
                  NT3main,
                  0,
                  "NT-PS1",
                  (void *) (NT3MAIN_Stack),
                  NT3MAIN_TASK_STACK_SIZE,
                  &NT3MAIN_TaskHdl,
                  &NT3MAIN_Task);
	
	//Start NT3main Thread :: for port 1
	cyg_thread_resume(NT3MAIN_TaskHdl);
#endif
//////// Supported Windows Print Server <UDP>
	//Create NTUDP Thread
#ifdef USE_ADMIN_LIBS
    cyg_thread_create(NTUDP_TASK_PRI,
                  NTUtilityUDP,
                  0,
                  "NT-PS-UDP",
                  (void *) (NTUDP_Stack),
                  NTUDP_TASK_STACK_SIZE,
                  &NTUDP_TaskHdl,
                  &NTUDP_Task);
	
	//Start NTUDP Thread
	cyg_thread_resume(NTUDP_TaskHdl);
	
//////// Supported Windows Print Server <IPX>
	IPXUtilityInit();
	IPXReceiveInit();
	UpgradeSocketInit();
#endif
//////// Supported Windows Print Server <NetBEUI>
	//Supported NetBEUI protocol depend on IPX protocol

//////// Supported Windows Print Server <SMB>

#ifdef SMBD
#if !defined(N716U2S) && !defined(O_ELEC)
	//Create SMBInit Thread
	if(PSMode & PS_SMB_MODE)
	{
		cyg_thread_create(SMBInit_TASK_PRI,
							SMBInit,
							0,
							"SMBInit",
							(void *) (SMBInit_Stack),
							SMBInit_TASK_STACK_SIZE,
							&SMBInit_TaskHdl,
							&SMBInit_Task);
		
		//Start SMBInit Thread
		cyg_thread_resume(SMBInit_TaskHdl);
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif SMBD

//////// Supported UNIX Print Server <LPR>
#ifdef LPD_TXT
//615wu::no psmain	 cyg_semaphore_init( &UNIX_SIGNAL_NO, 0 );
	 
	//Create Lpdstart Thread
	if(PSMode & PS_UNIX_MODE)
	{
		cyg_thread_create(LpdStart_TASK_PRI,
							lpdstart,
							0,
							"LPD-PS",
							(void *) (LpdStart_Stack),
							LpdStart_TASK_STACK_SIZE,
							&LpdStart_TaskHdl,
							&LpdStart_Task);
		
		//Start Lpdstart Thread
		cyg_thread_resume(LpdStart_TaskHdl);
	}
#endif LPD_TXT

//////// Supported UNIX Print Server <LAW TCP>
#ifdef RAWTCPD

	if(PSMode2 & PS_RAWTCP_MODE){
			//Create Rawtcpd Thread
			cyg_thread_create(Rawtcpd_TASK_PRI,
								rawtcpd,
								0,
								"RAWD",
								(void *) (Rawtcpd_Stack),
								Rawtcpd_TASK_STACK_SIZE,
								&Rawtcpd_TaskHdl,
								&Rawtcpd_Task);
			
			//Start Rawtcpd Thread
			cyg_thread_resume(Rawtcpd_TaskHdl);
	}

#endif RAWTCPD


//////// Supported Apple Mac Print Server <Apple Talk>
#ifdef ATALKD
#if !defined(N716U2S) && !defined(O_ELEC)
#if defined(O_ZOTCH) || defined(O_AXIS)
	if(EEPROM_Data.APPTLKEn)
#else
	if(PSMode & PS_ATALK_MODE)
#endif	// defined(O_ZOTCH)
	{
        cyg_semaphore_init(&ATD_INIT_OK, 0);

		//Create ATD Thread
	    cyg_thread_create(ATD_TASK_PRI,
	                  atalkd_init,
	                  0,
	                  "ATD",
	                  (void *) (ATD_Stack),
	                  ATD_TASK_STACK_SIZE,
	                  &ATD_TaskHdl,
	                  &ATD_Task);
		
		//Start ATD Thread
		cyg_thread_resume(ATD_TaskHdl);
		
		//Create PAPD1 Thread
	    cyg_thread_create(PAPD1_TASK_PRI,
	                  papd,
	                  0,
	                  "PAPD-1",
	                  (void *) (PAPD1_Stack),
	                  PAPD1_TASK_STACK_SIZE,
	                  &PAPD1_TaskHdl,
	                  &PAPD1_Task);
		
		//Start PAPD1 Thread
		cyg_thread_resume(PAPD1_TaskHdl);
	

		//Create AEP Thread
	    cyg_thread_create(AEP_TASK_PRI,
	                  aep_input,
	                  0,
	                  "AEP",
	                  (void *) (AEP_Stack),
	                  AEP_TASK_STACK_SIZE,
	                  &AEP_TaskHdl,
	                  &AEP_Task);
		
		//Start AEP Thread
		cyg_thread_resume(AEP_TaskHdl);
		
		//Create NBP Thread
	    cyg_thread_create(NBP_TASK_PRI,
	                  nbp_input,
	                  0,
	                  "NBP",
	                  (void *) (NBP_Stack),
	                  NBP_TASK_STACK_SIZE,
	                  &NBP_TaskHdl,
	                  &NBP_Task);
		
		//Start NBP Thread
		cyg_thread_resume(NBP_TaskHdl);
		
		ZIP_WAIT = 1;
		
		//Create ZIP Thread
	    cyg_thread_create(ZIP_TASK_PRI,
	                  zip_info_query,
	                  &ZIP_WAIT,
	                  "ZIP",
	                  (void *) (ZIP_Stack),
	                  ZIP_TASK_STACK_SIZE,
	                  &ZIP_TaskHdl,
	                  &ZIP_Task);
		
		//Start ZIP Thread
		cyg_thread_resume(ZIP_TaskHdl);
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif ATALKD


//////// Supported Novell Print Server <NatWare>
#ifdef NOVELL_PS
#if !defined(N716U2S) && !defined(O_ELEC)
//615wu::No PSMain
	//Create NovellPSMAIN Thread
    cyg_thread_create(NovellPSMAIN_TASK_PRI,
                  NovellPSmain,
                  0,
                  "PS-Main",
                  (void *) (NovellPSMAIN_Stack),
                  NovellPSMAIN_TASK_STACK_SIZE,
                  &NovellPSMAIN_TaskHdl,
                  &NovellPSMAIN_Task);
	
	//Start NovellPSMAIN Thread
	cyg_thread_resume(NovellPSMAIN_TaskHdl);

	NovellPrintSocketInit();
	
	//Create Novell-SAP Thread
    cyg_thread_create(NSAP_TASK_PRI,
                  SendNovellSAP,
                  0,
                  "Novell-SAP",
                  (void *) (NSAP_Stack),
                  NSAP_TASK_STACK_SIZE,
                  &NSAP_TaskHdl,
                  &NSAP_Task);
	
	NovellPrintServerInit();
	
	if(PSMode & PS_NETWARE_MODE) {
		BinderyPrintServerInit();
		
		//Create Novell-Connect Thread
	    cyg_thread_create(NCON_TASK_PRI,
	                  ConnectNetware,
	                  0,
	                  "Novell-Connect",
	                  (void *) (NCON_Stack),
	                  NCON_TASK_STACK_SIZE,
	                  &NCON_TaskHdl,
	                  &NCON_Task);
		
		//Start Novell-Connect Thread
		cyg_thread_resume(NCON_TaskHdl);
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif NOVELL_PS	
//////// Supported Novell Print Server <NDS>
#ifdef NDS_PS
#if !defined(N716U2S) && !defined(O_ELEC)
	if(PSMode & PS_NDS_MODE) {
		//Create Novell-NDS Thread
	    cyg_thread_create(NDS_TASK_PRI,
	                  NDSmain,
	                  0,
	                  "Novell-NDS",
	                  (void *) (NDS_Stack),
	                  NDS_TASK_STACK_SIZE,
	                  &NDS_TaskHdl,
	                  &NDS_Task);
		
		//Start Novell-NDS Thread
		cyg_thread_resume(NDS_TaskHdl);
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif NDS_PS

//////// Supported SNMP
#ifdef SNMPD
#if !defined(N716U2S) && !defined(O_ELEC)
#if defined(O_ZOTCH) || defined(O_ZOTCHW)
	// George Add June 1, 2009
	if(PSMode2 & PS_SNMP_MODE)
#endif	// defined(O_ZOTCH) || defined(O_ZOTCHW)
	{
		//Create SNMPD Thread
	    cyg_thread_create(SNMPD_TASK_PRI,
	                  snmpd,
	                  0,
	                  "SNMPD",
	                  (void *) (SNMPD_Stack),
	                  SNMPD_TASK_STACK_SIZE,
	                  &SNMPD_TaskHdl,
	                  &SNMPD_Task);
		
		//Start SNMPD Thread
		cyg_thread_resume(SNMPD_TaskHdl);	//615wu socket not enough
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif SNMPD

#ifdef RENDEZVOUS
#if !defined(N716U2S) && !defined(O_ELEC)
//	Rendezous_init();
	if(EEPROM_Data.RENVEnable == 1)
	{
		//Create Rendezous Thread
	    cyg_thread_create(Rendezous_TASK_PRI,
	                  Rendezous_init,
	                  0,
	                  "Rendezous",
	                  (void *) (Rendezous_Stack),
	                  Rendezous_TASK_STACK_SIZE,
	                  &Rendezous_TaskHdl,
	                  &Rendezous_Task);
		
		//Start Rendezous_init Thread
		cyg_thread_resume(Rendezous_TaskHdl);	//615wu socket not enough
		cyg_semaphore_init(&rendezvous_sem, 0);
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif //RENDEZVOUS

//Supported HTTP
//////// Supported Windows Print Server <IPP>
#ifdef HTTPD
	#if defined(O_ZOTCH)
	if(PSMode2 & PS_HTTP_MODE)
	#endif	// defined(O_ZOTCH)
	httpd_init();
#endif HTTPD	

#ifdef UNIXUTIL_TFTP
//Create TFTPD Thread
	    cyg_thread_create(TFTPD_TASK_PRI,
	                  tftpd,
	                  0,
	                  "TFTPD",
	                  (void *) (TFTPD_Stack),
	                  TFTPD_TASK_STACK_SIZE,
	                  &TFTPD_TaskHdl,
	                  &TFTPD_Task);
		
		//Start TFTPD Thread
		cyg_thread_resume(TFTPD_TaskHdl);	//615wu socket not enough

#endif //UNIXUTIL_TFTP

#ifdef Mail_ALERT
#if !defined(N716U2S) && !defined(O_ELEC)
	if (AlertActive) {
//		newproc("alert", 768, smtp_alert, 0, NULL,NULL,0);

		//Create SMTP ALERT Thread
	    cyg_thread_create(SMTP_ALERT_TASK_PRI,
	                  smtp_alert,
	                  0,
	                  "smtp alert",
	                  (void *) (SMTP_ALERT_Stack),
	                  SMTP_ALERT_TASK_STACK_SIZE,
	                  &SMTP_ALERT_TaskHdl,
	                  &SMTP_ALERT_Task);
		
		//Start SMTP ALERT Thread
		cyg_thread_resume(SMTP_ALERT_TaskHdl);

	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif	//Mail_ALERT

#ifdef Print_ALERT
#if !defined(N716U2S) && !defined(O_ELEC)
//#ifdef Mail_ALERT
//	if (AlertActive){
//#endif //Mail_ALERT
	//newproc("mail", 256, mail_alert, 0, NULL,NULL,0);
#if defined(O_ZOTCH)
	if (AlertActive)
#endif // defined(O_ZOTCH)
	{
		//Create MAIL ALERT Thread
	    cyg_thread_create(MAIL_ALERT_TASK_PRI,
	                  mail_alert,
	                  0,
	                  "mail",
	                  (void *) (MAIL_ALERT_Stack),
	                  MAIL_ALERT_TASK_STACK_SIZE,
	                  &MAIL_ALERT_TaskHdl,
	                  &MAIL_ALERT_Task);
		
		//Start MAIL ALERT Thread
		cyg_thread_resume(MAIL_ALERT_TaskHdl);	
	}
#endif	// !defined(N716U2S) && !defined(O_ELEC)
#endif //Print_ALERT

#ifdef TELNETD
#if (!defined(O_ELEC) && !defined(N716U2S))
#if defined(O_ZOTCH) || defined(O_ZOTCHW)
	// George Add June 1, 2009
	if(PSMode2 & PS_TELNET_MODE)
#endif	// defined(O_ZOTCH) || defined(O_ZOTCHW)
	{	
		//Create TELNET Thread
		cyg_thread_create(TELNET_TASK_PRI,
		              telnetstart,
		              0,
		              "TELNETD",
		              (void *) (TELNET_Stack),
		              TELNET_TASK_STACK_SIZE,
		              &TELNET_TaskHdl,
		              &TELNET_Task);
		
		//Start TELNET Thread
		cyg_thread_resume(TELNET_TaskHdl);	
	}
#endif	// (!defined(O_ELEC) && !defined(N716U2S))
#endif TELNETD

}
void starSAPThread(void)
{
	//Start Novell-SAP Thread									//eCos
	cyg_thread_resume(NSAP_TaskHdl);
}

#if defined(NDWP2020)
int disconnect_wireless = 1;
extern void disassoc_cmd_sw(void);	//eason 20100608
#endif

void sysreboot(cyg_handle_t handle, cyg_addrword_t ptr){
    #if defined(NDWP2020)
	if(disconnect_wireless == 1)
	{
		disassoc_cmd_sw();	//eason 20100608
		disconnect_wireless = 0;
	}
	else
    #endif
	    Reset();
}

int ROOT_flag = 0;

void REBOOT(void){
	
    cyg_handle_t hSysClk;
    cyg_handle_t hCounter;

    static cyg_handle_t hAlarm;    
    static unsigned int alarmData;    
    static cyg_alarm timerAlarm;
    int interval = 200;   /* 2 sec*/

	//Init Rest time function one time    
    if( ROOT_flag == 1)
    	return;

    ROOT_flag = 1;
    
    /* Attach the timer to the real-time clock */
    hSysClk = cyg_real_time_clock();

    cyg_clock_to_counter(hSysClk, &hCounter);

    cyg_alarm_create(hCounter, (cyg_alarm_t *)sysreboot,
                     (cyg_addrword_t) &alarmData,
                     &hAlarm, &timerAlarm);

    /* This creates a periodic timer */
    cyg_alarm_initialize(hAlarm, cyg_current_time() + interval, interval);
}
