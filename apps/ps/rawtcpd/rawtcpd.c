// This module works for raw-tcp printing on printer server
// add by Charles 2001/10/31

#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"
#include "prnqueue.h"
#include "prnport.h"
#include "joblog.h"

#ifndef USE_PS_LIBS
#undef NOVELL_PS
#endif

//#if defined(RAWTCPD)

#define  BLOCK_SIZE				1460	//same with TCPIP 'TCP_MSS'
#define  DEFAULT_PORT			0

extern int Network_TCPIP_ON;

//os int rawtcpd( int port, void *noused1, void *noused2 )
void rawtcpd(cyg_addrword_t data)
{
	struct sockaddr_in lsocket;
	struct sockaddr_in from_addr;
	struct prnbuf *pbuf;
	int ss, s, from_len, bytes;
	int port = DEFAULT_PORT;
	uint32 startime;
	BYTE	btTimeout;
#ifdef USB_ZERO_CPY	
	int retrycnt, dataremain;
	int blocksize = 8192;
#endif
	while( Network_TCPIP_ON == 0 )
		ppause(100);

	lsocket.sin_family = AF_INET;
	lsocket.sin_addr.s_addr = htonl (INADDR_ANY);
#if (NUM_OF_PRN_PORT == 1)
	lsocket.sin_port = htons (IPPORT_RAWTCP);
#else
	lsocket.sin_port = htons (IPPORT_RAWTCP + port + 1);
#endif
	lsocket.sin_len = sizeof(lsocket);
	
	ss = socket( AF_INET, SOCK_STREAM, 0 );
	bind( ss, (struct sockaddr *)&lsocket, sizeof(lsocket) );
	listen( ss, 1 );

	while( 1 )
	{
		from_len = sizeof(from_addr);
		if( ( s = accept( ss,(struct sockaddr *) &from_addr, &from_len ) ) == -1 )
		{
			continue;
		}

#ifdef USE_PS_LIBS
		if( ReadPortStatus(port) != PORT_OFF_LINE )
		{
			cyg_scheduler_lock();	//615wu::No PSMain
			startime = rdclock();			
			
			while( (PrnGetPrinterStatus(port) != PrnNoUsed ) ||//&&
					 (ReadPortStatus(port) != PORT_READY) )
			{
				cyg_scheduler_unlock();	//615wu::No PSMain
				
				if((( rdclock()-startime)  > ((uint32)TICKS_PER_SEC*10) )) {
			
					sendack(s);
					startime = rdclock();
				}
				cyg_thread_yield();
//				ppause(100);
			
				cyg_scheduler_lock();	//615wu::No PSMain
			
				continue;
			}
		
			PrnSetRawTcpInUse(port);
			
			cyg_scheduler_unlock();	//615wu::No PSMain
				
#ifdef SUPPORT_JOB_LOG
			JL_PutList(4, port, "User", 32);
#endif SUPPORT_JOB_LOG

			while( 1 )
			{
				startime = rdclock();
				while( ( pbuf = PrnGetInQueueBuf(port) ) == NULL) {
/*
//os					kwait(NULL);
						cyg_thread_yield();
*/
						//10/11/99 Send Ack When Buffer is empty *****************
						if((( rdclock()-startime)  > ((uint32)TICKS_PER_SEC*5) )) {
							//if(startime)	//2003Nov28
							sendack(s);
							startime = rdclock();
						}
						//********************************************************
//os					kwait(0);
						cyg_thread_yield();
				}
				pbuf->size = 0;
				btTimeout = 0;
				
#ifndef USB_ZERO_CPY				
				while( ( bytes = recv( s, &pbuf->data[pbuf->size], BLOCK_SIZE, 0 ) ) > 0 )
#else
				retrycnt =0;
				dataremain = blocksize;
				while( ( bytes = recv( s, &pbuf->data[pbuf->size], dataremain, 0 ) ) > 0 || retrycnt++ < 2)				
#endif				
				{
#ifdef USB_ZERO_CPY			
					if (bytes <= 0){
						cyg_thread_yield();
						continue;	
					}
					retrycnt =0;
#endif			
					
					pbuf->size += bytes;
#ifdef SUPPORT_JOB_LOG
					JL_AddSize(port, bytes);
#endif SUPPORT_JOB_LOG

#ifndef USB_ZERO_CPY
					if( pbuf->size + BLOCK_SIZE > BLOCKSIZE )
#else
					dataremain -= bytes;	
					if( !dataremain )
#endif					
					{
						PrnPutOutQueueBuf(port,pbuf,PRN_Q_NORMAL);
						//PrnPutInQueueBuf(port, pbuf);
						break;
					}
				}
				if( bytes <= 0 )
				{
					if( pbuf->size )
						PrnPutOutQueueBuf(port,pbuf,PRN_Q_NORMAL);
					else
						PrnPutInQueueBuf(port, pbuf);
					
					// George Add February 14, 2007
					if((rdclock()-startime) > ((uint32)(EEPROM_Data.TimeOutValue))*10)
						btTimeout = 1;
						
					break;
				}
//os		kwait(NULL);
				cyg_thread_yield();
			}

 			PrnSetNoUse(port);
#ifdef SUPPORT_JOB_LOG 			
 			if( btTimeout == 1 )
 			{
				JL_EndList(port, 3);	// Timeout. George Add February 1, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINA) && !defined(O_TPLINS) && !defined(O_LS)
#ifdef NOVELL_PS
				SendEOF(port);	        // Send the EOF page. George Add Junuary 10, 2008
#endif
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
			}
			else
				JL_EndList(port, 0);	// Completed. George Add January 26, 2007
#endif SUPPORT_JOB_LOG		
		}
		else
			cyg_scheduler_unlock();	//615wu::No PSMain
#endif /* USE_PS_LIBS */
 		close( s );
	}
}
//#endif
