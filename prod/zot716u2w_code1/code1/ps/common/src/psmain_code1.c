#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "star_gpio.h"	//ZOT716u2

extern void PhyMonitor(void);

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

void udelay( int x )
{

	unsigned int delay, us = x*5;
	
	for( delay = 0; delay < us ; delay++ );

}

/////////////////////////PS Main/////////////////////////////////////////

extern void NTUtilityUDP(cyg_addrword_t data);
#ifdef UNIXUTIL_TFTP
extern void tftpd(cyg_addrword_t data);
#endif //UNIXUTIL_TFTP

static void WaitingUpgradeLightFlash(cyg_addrword_t data);

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

//NTUDP Task create information definition
#define NTUDP_TASK_PRI         20	//ZOT716u2
#define NTUDP_TASK_STACK_SIZE  2048 //ZOT716u2 4096
static uint8 			NTUDP_Stack[NTUDP_TASK_STACK_SIZE];
static cyg_thread       NTUDP_Task;
static cyg_handle_t     NTUDP_TaskHdl;

//TFTPD Thread initiation information
#define TFTPD_TASK_PRI         	20	//ZOT716u2
#define TFTPD_TASK_STACK_SIZE  	2048
static	uint8			TFTPD_Stack[TFTPD_TASK_STACK_SIZE];
static  cyg_thread		TFTPD_Task;
static  cyg_handle_t	TFTPD_TaskHdl;

//WAITFLASH Thread initiation information
#define WAITFLASH_TASK_PRI         	20	//ZOT716u2
#define WAITFLASH_TASK_STACK_SIZE  	512
static	uint8			WAITFLASH_Stack[WAITFLASH_TASK_STACK_SIZE];
static  cyg_thread		WAITFLASH_Task;
static  cyg_handle_t	WAITFLASH_TaskHdl;

//Print Server initialization 
void ps_init(void)
{
//////// Supported Windows Print Server <UDP>
	//Create NTUDP Thread
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
	UpgradeSocketInit();
	
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

	//Create WAITFLASH Thread
	    cyg_thread_create(WAITFLASH_TASK_PRI,
	                  WaitingUpgradeLightFlash,
	                  0,
	                  "WAITUPFLASH",
	                  (void *) (WAITFLASH_Stack),
	                  WAITFLASH_TASK_STACK_SIZE,
	                  &WAITFLASH_TaskHdl,
	                  &WAITFLASH_Task);
		
		//Start WAITFLASH Thread
		cyg_thread_resume(WAITFLASH_TaskHdl);	



}


void sysreboot(cyg_handle_t handle, cyg_addrword_t ptr){
	Reset();
}

void REBOOT(void){
	
    cyg_handle_t hSysClk;
    cyg_handle_t hCounter;

    static cyg_handle_t hAlarm;    
    static unsigned int alarmData;    
    static cyg_alarm timerAlarm;
    int interval = 200;   /* 2 sec*/
    
    /* Attach the timer to the real-time clock */
    hSysClk = cyg_real_time_clock();

    cyg_clock_to_counter(hSysClk, &hCounter);

    cyg_alarm_create(hCounter, (cyg_alarm_t *)sysreboot,
                     (cyg_addrword_t) &alarmData,
                     &hAlarm, &timerAlarm);

    /* This creates a periodic timer */
    cyg_alarm_initialize(hAlarm, cyg_current_time() + interval, interval);
}

extern uint8   PSUpgradeMode;

void WaitingUpgradeLightFlash(cyg_addrword_t data)
{
	for(;;)
	{
		if( PSUpgradeMode == WAIT_UPGRADE_MODE )
		{
			Light_On(GPIO_16_MASK);
			ppause(100);
			Light_Off(GPIO_16_MASK);
			ppause(100);
		}
		else
		{
			ppause(1000);
		}	
	}
}