#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "prnqueue.h"
#include "atalkd.h"
#include "atp.h"
#include "atprn.h"
#include "pap.h"
#include "file.h"
#include "magics.h"
#include "session.h"
#include "comment.h"
#include "paprint.h"
#include "joblog.h"

int SendATickle(uint8 connid,ATP atp,struct sockaddr_at  *sat);

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
int16 AtalkRemoveEndCommand(BYTE *outbuf,int16 len);
int16 AtalkChangeTrue2False(BYTE *outbuf,int16 len);
int16 AtalkRemovePJLCommand(BYTE *outbuf,int16 len);
int16 AtalkRemoveTBCPCommand(BYTE *outbuf,int16 len);
#endif DEF_IEEE1284 && BI_ATALKD

extern int 	ApplePortNumber;
extern cyg_sem_t ATALK_SIGNAL_NO_1;

#ifdef SUPPORT_JOB_LOG
extern uint8 	HadLoggedA;
#endif SUPPORT_JOB_LOG

//*
//* Accept files until the client closes the connection.
//* Read lines of a file, until the client sends eof, after
//* which we'll send eof also.
//*
void pap_session(cyg_addrword_t data)
{
	int PrnPort = ApplePortNumber;	//615wu
	char   cbuf[20];
	struct atp_block	atpb;
	struct sockaddr_at	ssat;
	ATP    atp       = pr[PrnPort].cur_atp;
	struct papfile	 outfile;
	struct Mypapfile infile;
	uint16 seq = 0, netseq;
//	uint16 rseq;
	uint16 tv_sec;
	int16  i, cc, readpending = 0;
	uint8  readport;
	uint8  connid = pr[PrnPort].connid;
	struct sockaddr_at	sat = pr[PrnPort].sat;

	struct iovec niov[1], *tmpiov;
	int8   *tmpbuf;
	WORD   ReadBackCount;

//	char buf[ PAP_MAXQUANTUM ][ 4 + PAP_MAXDATA ];
//  struct iovec	niov[ PAP_MAXQUANTUM ];
//	struct iovec	iov[ PAP_MAXQUANTUM ];
	int16 HaveRequestData = 0;
	int32 TimeOutValue = EEPROM_Data.TimeOutValue * 1000L;
	int32 TickleTime = -TimeOutValue;
	int32 StartTime = msclock(), tv;

	int32 StartReadBackTime = 0;

	uint8 *outbuf;   //3/17/2000

	cyg_semaphore_post (&ATALK_SIGNAL_NO_1);

#ifndef _PC
//		Light_On(R_Lite);
#endif  _PC

//	for(i = 0 ; i < PAP_MAXQUANTUM; i++) {
//		niov[i].iov_base = buf[i];
//		niov[i].iov_len  = 0;
////		iov[i].iov_base  = buf[i] + 4;
////		iov[i].iov_len   = 0;
//	}

	infile.pf_state = PF_BOT;
	infile.pbuf[0] = infile.pbuf[1] = NULL;
	infile.iobuf[PAP_MAXQUANTUM].iov_len = 0; //for markline buffer
	infile.iocnt     = 0;  // 1 - 8	, 0: no data

	//3/17/2000 for replace infile.iobuf[MAX_QUANTUM].iov_base
	if( (outbuf = malloc(PAP_MAXDATA+4)) == NULL) return;

//	infile.pf_len = 0;
//	infile.pf_buf = 0;
//	infile.pf_cur = 0;
//	infile.pf_end = 0;
	memset(outbuf, 0x00, (PAP_MAXDATA+4));
	outfile.pf_state = PF_BOT;
	outfile.pf_len = 0;
	outfile.pf_buf = 0;
	outfile.pf_cur = 0;
	outfile.pf_end = 0;

	for (;;) {
		if(!HaveRequestData) {
			if( (msclock() - TickleTime) > 60*1000L)  {
				//*
				//* Send a tickle.
				//*
				cbuf[ 0 ] = connid;
				cbuf[ 1 ] = PAP_TICKLE;
				cbuf[ 2 ] = cbuf[ 3 ] = 0;
 				atpb.atp_saddr = &sat;
				atpb.atp_sreqdata = cbuf;
				atpb.atp_sreqdlen = 4;		/* bytes in Tickle request */
				atpb.atp_sreqto = 0;		/* best effort */
	  			atpb.atp_sreqtries = 1;		/* try once */
				if ( atp_sreq( atp, &atpb, 0, 0 )) {
#ifdef PC_OUTPUT
					printf("ERROR:atp_sreq() (session.c)\n");
#endif PC_OUTPUT
					goto end_session;
				}
				TickleTime = msclock();
			}
			if(
#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
			    (!(outfile.pf_state & PF_QUERY_END)) &&
#endif DEF_IEEE1284 && BI_ATALKD
			    (infile.pbuf[0] != NULL || PrnGetAvailQueueNO(PrnPort) >= PAP_QUEUE_SIZE)
			) {
				//*
				//* Ask for data.
				//*
				cbuf[ 0 ] = connid;
				cbuf[ 1 ] = PAP_READ;  //Please SendData ....
				if ( seq++ == 0xffff ) seq = 1;
				netseq = htons( seq );
				bcopy( &netseq, &cbuf[ 2 ], sizeof( netseq ));	//sequence number
				atpb.atp_saddr = &sat;
				atpb.atp_sreqdata = cbuf;
				atpb.atp_sreqdlen = 4;		// bytes in SendData request
				atpb.atp_sreqto = 5;		// retry timer
				atpb.atp_sreqtries = -1;	// infinite retries */
				//Send Request {Please (Send Data) }
				if ( atp_sreq( atp, &atpb, PAP_MAXQUANTUM, ATP_XO )) {
#ifdef PC_OUTPUT
					printf("ERROR: atp_sreq() in session.c\n");
#endif PC_OUTPUT
					goto end_session;
				}
//************ use queue buffer as receive buffer ******************
#ifdef _PC
				while(PAP_QUEUE_SIZE > PRNQUEUELEN || BLOCKSIZE != 2920) {
					printf("(ATALK) SESSION.C Design Error\n");
				}
#endif _PC
				for(i = 0 ; i < 2 ; i++) {
					if(infile.pbuf[i] == NULL) {
						while((infile.pbuf[i] = PrnGetInQueueBuf(PrnPort)) == NULL) {
#ifdef PC_OUTPUT
							printf("Get Print Queue Error !\n");
#endif PC_OUTPUT
#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
//							if(papqueryfull(PrnPort,2)) papqueryprint(PrnPort);
#endif DEF_IEEE1284 && BI_ATALKD
							cyg_thread_yield();
						}
					}
				}

				// iobuf[0] - iobuf[7] for receive packet
				// iobuf[8] for	return to markline

				tmpbuf = (infile.pbuf[0])->data;
				for(i = 0; i < (PAP_MAXQUANTUM+2) ; i++) {
					//reserved one buffer for temp
					if(i == 5) tmpbuf = (infile.pbuf[1])->data;
					infile.iobuf[i].iov_base = tmpbuf;
					tmpbuf += (BLOCKSIZE/5);
					if(i != PAP_MAXQUANTUM) infile.iobuf[i].iov_len = PAP_MAXDATA + 4;
				}

				infile.iobuf[PAP_MAXQUANTUM+1].iov_base = outbuf; //3/17/2000
//**********************************************************************
				HaveRequestData = ATP_TRESP;
			}
			else 
				cyg_thread_yield();

		} //if(!HaveRequestData) ......

		//check receive packet and timeout = Get from EEPROM !
		if(( cc = atp_select(atp->atph_socket)) < 0 ) {
#ifdef PC_OUTPUT
			printf("ERROR: atp_select() (session.c)\n");
#endif PC_OUTPUT
			goto end_session;
		}

		tv = msclock();
		if ( cc == 0 ) {
			if((tv - StartTime) > TimeOutValue)  {
#ifdef PC_OUTPUT
				AtSaySpace(0,22,79);
				printf("!!! ATALKD: TIME OUT (port%d)!! (session.c)\n",PrnPort);
#endif PC_OUTPUT
				//abort printing
				if(infile.pf_state & PF_PRINT && infile.pbuf[0]) {
#ifdef PC_OUTPUT
					while(infile.iocnt) {
						printf("SESSCION.C IOCNT(0) error\n");
					}
#endif PC_OUTPUT
					infile.iocnt = 1;
//5/19/2000					infile.iobuf[0].iov_base[0] = '\4';
//5/19/2000					infile.iobuf[0].iov_len = 1;

					infile.iobuf[0].iov_len = 0;
					paprint(PrnPort,&infile,PRN_Q_ABORT);
#ifdef SUPPORT_JOB_LOG
					JL_EndList(PrnPort, 3);	// Abort or Timeout. George Add February 12, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
					SendEOF(PrnPort);	    // Send the EOF page. George Add January 10, 2008
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
					HadLoggedA = 0;			// George Add April 10, 2007
#endif SUPPORT_JOB_LOG
				}
				goto end_session;
			}
			if(HaveRequestData && ((tv - atp->atph_reqtv) > (atp->atph_reqto*1000L)) ) //Ron modified for performance issue 12/22/04
//			if(HaveRequestData && ((tv - atp->atph_reqtv) > (atp->atph_reqto*200L)) )
				resend_request(atp);
			cyg_thread_yield();
			continue;
		} else {
			StartTime = tv;
		}

		memset( &ssat,'\0', sizeof( struct sockaddr_at ));

		switch( atp_rsel( atp, &ssat, HaveRequestData | ATP_TREQ)) {
		//Request (Please Send data or Close connect or Tickle)
		case ATP_TREQ :
			atpb.atp_saddr = &ssat;
			atpb.atp_rreqdata = cbuf;
			atpb.atp_rreqdlen = sizeof( cbuf );
			if ( atp_rreq( atp, &atpb ) < 0 ) {
#ifdef PC_OUTPUT
				printf("ERROR:atp_rreq() (session.c)\n");
#endif PC_OUTPUT
				goto end_session;
			}
		    // sanity
			if ( (unsigned char)cbuf[ 0 ] != connid ) {
#ifdef PC_OUTPUT
				printf("Bad ATP request!\n (session.c)" );
#endif PC_OUTPUT
				continue;
		    }

			switch( cbuf[ 1 ] ) { //command

			case PAP_READ :
				//*
				//* Other side is ready for some data.
				//*
//				bcopy( &cbuf[ 2 ], &netseq, sizeof( netseq ));
//				if ( netseq != 0 ) {
//				    if ( rseq != ntohs( netseq )) {
//						break;
//				    }
//			    	if ( rseq++ == 0xffff ) rseq = 1;
//				}
				readpending = 1;
				readport = ssat.sat_port;
				break;
			case PAP_CLOSE :
				//*
				//* Respond to the close request.
				//* If we're in the middle of a file, clean up.
				//*
				niov[0].iov_len = 4;
				niov[0].iov_base = cbuf;
				cbuf[0] = connid;
				cbuf[1] = PAP_CLOSEREPLY;
				cbuf[2] = cbuf[3] =  0;


//				((char *)niov.iov_base)[ 0 ] = connid;
//				((char *)niov.iov_base)[ 1 ] = PAP_CLOSEREPLY;
//				((char *)niov.iov_base)[ 2 ] =
//				((char *)niov.iov_base)[ 3 ] = 0;

				atpb.atp_sresiov = niov;
				atpb.atp_sresiovcnt = 1;
				if ( atp_sresp( atp, &atpb ) < 0 ) {
#ifdef PC_OUTPUT
					printf("session.c atp_sresp() error \n");
#endif PC_OUTPUT
					goto end_session;
				}
#ifdef PC_OUTPUT
				AtSaySpace(0,22,79);
				printf("!!!PAP CLOSE !!!!");
				sound(500);
				delay(50);
				nosound();
				delay(50);
				sound(500);
				delay(50);
				nosound();
#endif PC_OUTPUT
				goto end_session;
		    case PAP_TICKLE :
//				if(SendATickle(connid,atp,&ssat) < 0) {
//					goto end_session;
//				}
				break;
		    default :
#ifdef PC_OUTPUT
				printf("(ATALKD) SESSION.C Bad PAP request!\n" );
#endif PC_OUTPUT
				break;
			} //switch( cbuf[ 1 ] ) { //command
			break;
		//Recv Client (Data)
		case ATP_TRESP :
			atpb.atp_saddr = &ssat;
//			for ( i = 0; i < PAP_MAXQUANTUM; i++ ) {
//				niov[ i ].iov_len = PAP_MAXDATA + 4;
//			}
//			atpb.atp_rresiov = niov;
			atpb.atp_rresiov = infile.iobuf;

			atpb.atp_rresiovcnt = PAP_MAXQUANTUM;
			if ( atp_rresp( atp, &atpb ) < 0 ) {
#ifdef PC_OUTPUT
				printf("(ATALKD): error atp_rresp\n");
#endif PC_OUTPUT
				goto end_session;
			}

			// sanity
			if ( ((unsigned char *)IOV_BASE(infile,0))[ 0 ] != connid ||
			    ((char *)IOV_BASE(infile,0))[ 1 ] != PAP_DATA ) {
#ifdef PC_OUTPUT
				printf("Bad data response!\n(session.c)\n");
#endif PC_OUTPUT
				continue;
			}
#ifdef PC_OUTPUT
			while(infile.iocnt) {
				printf("SESSCION.C IOCNT(1) error\n");
			}
#endif PC_OUTPUT

			if (( infile.pf_state & PF_EOF ) == 0 &&
				(((char *)IOV_BASE(infile,0))[ 2 ] )
			){
				infile.pf_state |= PF_EOF;
			}
			for ( i = 0; i < atpb.atp_rresiovcnt; i++ ) {
				memcpy(IOV_BASE(infile,i),IOV_BASE(infile,i)+4,IOV_LEN(infile,i)-4);
				IOV_LEN(infile,i) -= 4;
			}
			infile.iobuf_row = infile.iobuf_next_row = 0;  // 0 - 7
			infile.iobuf_col = infile.iobuf_next_col = 0;
			infile.iocnt = atpb.atp_rresiovcnt;

			// move data
			if ( PostScript(PrnPort, &infile, &outfile ) < 0 ) {
#ifdef PC_OUTPUT
				printf("postscript parse: bad return" );
#endif PC_OUTPUT
				goto end_session;
			}
			HaveRequestData = 0;
			if(outfile.pf_state & PF_EOP) {
#ifndef _PC
//				Light_Off(G_Lite);
#endif  _PC
				outfile.pf_state |= PF_EOF;
				TimeOutValue = 10 * 1000L;
			}
			break;
		case 0:
	    	break;
		default :
#ifdef PC_OUTPUT
			printf("(SESSION.C) ERROR :atp_rsel(258)\n");
#endif PC_OUTPUT
			goto end_session;
		} //switch( atp_rsel( ...

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
		if(outfile.pf_state & PF_QUERY_END) {
			if((outfile.pf_state & PF_EOP)			         &&
				(G_PortReady & (0x01 << PrnPort))            &&
			    (PrnGetAvailQueueNO(PrnPort) == PRNQUEUELEN)
			    && PortReady(PrnPort)
			){

				if(!(outfile.pf_state & PF_READBACK)) {
					//Start ReadBack
					outfile.pf_state |=	PF_READBACK;
					ReadBackCount = 0;
					while( (StartReadBackTime = msclock()) == 0);
				}

				PrnEndNegotiate(PrnPort);
				cc = i = PrnReadBack(PrnPort,outbuf,PAP_MAXDATA);
				PrnStartNegotiate(PrnPort);

				if(i) {
					i = AtalkRemoveEndCommand(outbuf,i);

					if(i && EEPROM_Data.ATDataFormat[PrnPort] == AT_COMM_NONE) {					
						//binary to ascii
						i = AtalkChangeTrue2False(outbuf,i);
					}

					while(i && ReadBackCount == 0 && (outbuf[0] == ';' || outbuf[0] == '\0')) {
						memcpy(outbuf,outbuf+1,--i);   //IBM InfoPrint 20
					}

					if(i) i = AtalkRemoveTBCPCommand(outbuf,i);
					if(i) i = AtalkRemovePJLCommand(outbuf,i);
					if(i)  {
						APPEND(&outfile,outbuf,i);
						ReadBackCount += i;
					}
				}

				if( (cc < (PAP_MAXDATA) && ReadBackCount)) {
					outfile.pf_state &=  ~(int16)(PF_QUERY|PF_QUERY_END);
				}
				if((msclock() - StartReadBackTime) > EEPROM_Data.TimeOutValue * 1000L)  {					
					//Read Back TimeOut
                	goto end_session;
				}
			}
			cyg_thread_yield();
		}

#endif DEF_IEEE1284 && BI_ATALKD

		// send any data that we have
		if ( readpending &&
#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
		     ( (outfile.pf_state & PF_READBACK) || (PortIO[PrnPort].PrnReadBackMode == PRN_NO_PRINTER) ) &&
#endif DEF_IEEE1284 && BI_ATALKD
//			 ( PF_BUFSIZ( &outfile ) || ( outfile.pf_state & PF_EOF) )
			 ( PF_BUFSIZ( &outfile ))
		   )
		{
//			for ( i = 0; i < pr[PrnPort].quantum; i++ ) {

				// iobuf[9] for	outfile	(== outbuf 3/22/2000)
				tmpbuf = (char *)(infile.iobuf[PAP_MAXQUANTUM+1].iov_base);
				tmpbuf[ 0 ] = connid;
				tmpbuf[ 1 ] = PAP_DATA;
				tmpbuf[ 2 ] = tmpbuf[ 3 ] = 0;

				if ( PF_BUFSIZ( &outfile ) > PAP_MAXDATA  ) {
					cc = PAP_MAXDATA;
				} else {
					cc = PF_BUFSIZ( &outfile );

					if ( (outfile.pf_state & PF_EOF)
#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
						&& (!(outfile.pf_state & PF_QUERY_END))
#endif DEF_IEEE1284	&& BI_ATALKD
		     		) {
						tmpbuf[ 2 ] = 1;	// eof
						outfile.pf_state = PF_BOT;
						infile.pf_state = PF_BOT;
					}
				}

				infile.iobuf[PAP_MAXQUANTUM+1].iov_len = 4 + cc;
				bcopy( outfile.pf_cur, tmpbuf+4, cc );
				CONSUME( &outfile, cc );
//				if ( PF_BUFSIZ( &outfile ) == 0 ) {
//					i++;
//					break;
//				}
//			}
			ssat.sat_port = readport;
			atpb.atp_saddr = &ssat;
//			atpb.atp_sresiov = niov;
//			atpb.atp_sresiovcnt = i;	// reported by stevebn@pc1.eos.co.uk
			atpb.atp_sresiov = &infile.iobuf[PAP_MAXQUANTUM+1];
			atpb.atp_sresiovcnt = 1;	// reported by stevebn@pc1.eos.co.uk

			if ( atp_sresp( atp, &atpb ) < 0 ) {
#ifdef PC_OUTPUT
				printf("ATALKD:SESSION.C atp_sresp(298)\n");
#endif PC_OUTPUT
				goto end_session;
			}
			readpending = 0;
		}//if ( readpending && ( PF_BUFSIZ( &outfile ) ...
	} // for(;;)
end_session:
	for(i = 0 ; i < 2; i++)
		if(infile.pbuf[i]) {
			PrnPutInQueueBuf(PrnPort,infile.pbuf[i]);
		}

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
//	papqueryfree(PrnPort);
#endif DEF_IEEE1284	&& BI_ATALKD
#if defined(WEBADMIN) && !defined(_PC)
//	PrnStartStatusInfo(PrnPort);
#endif

	comfree(PrnPort);
//	free(infile.pf_buf);
	free(outfile.pf_buf);
	atp_close(atp);
	free(pr[PrnPort].jobname);
	pr[PrnPort].jobname = NULL;
	PrnSetNoUse(PrnPort);
	free(outbuf); //3/22/2000
	
	pap_socksum[data].flag = 0;
	pap_socksum[data].needrelase = 1;

	cyg_thread_exit();

#ifndef _PC
//	Light_Off(R_Lite);
//	Light_On(G_Lite);
#endif  _PC
}

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)

int16 AtalkChangeTrue2False(BYTE *outbuf,int16 len)
{
	BYTE *p;
	int16 i;

	if(len >= PAP_MAXDATA-2) return len;
	if(!memicmp(outbuf,"true",4)) {
		for(i = len; i > 4 ; i--) {
			outbuf[i] = outbuf[i-1];
		}
		memcpy(outbuf,"False",5);
		return len+1;
	}

	return len;
}

int16 AtalkRemoveEndCommand(BYTE *outbuf,int16 len)
{
	int16 i;

	for(i = 0 ; i < len;) {
		if(*outbuf == 0x04) {
			len--;
			memcpy(outbuf, outbuf+1,len-i);
			continue;
		}
		outbuf++;
		i++;
	}
	return len;
}


int16 AtalkRemoveTBCPCommand(BYTE *outbuf,int16 len)
{
	int16 i;

	for(i = 0 ; i <= len-2;) {
		if(!memcmp(outbuf,"\x01\x4D",2)){
			len -= 2;
			if(len > 0) memcpy(outbuf,outbuf+2,len-i);
			continue;
		}
		else {
			if(i <= len-9 && !memcmp(outbuf,"\x1B%-12345X",9)) {
				len -= 9;
				if(len > 0) memcpy(outbuf,outbuf+9,len-i);
				continue;
			}
		}
		outbuf++;
		i++;
	}
	return len;
}

int16 AtalkRemovePJLCommand(BYTE *outbuf,int16 len)
{
	int16 rc, i;

	if(len > 4 && !memcmp(outbuf,"@PJL",4)) {
		for(i = 4 ; i < len; i++) {
		    if(outbuf[i] == 0x0C) { //<FF>
		    	i++;
		        break;
			}
		}
		rc = len - i;
		if(rc > 0) memcpy(outbuf,outbuf+i, rc);
		return rc;
	}
	return len;
}
#endif DEF_IEEE1284 && BI_ATALKD
