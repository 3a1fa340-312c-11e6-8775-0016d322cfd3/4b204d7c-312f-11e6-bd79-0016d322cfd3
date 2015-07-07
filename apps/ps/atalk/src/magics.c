/*
 * Copyright (c) 1990,1994 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */
#include <stdio.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "atalkd.h"
#include "atp.h"
#include "pap.h"
#include "file.h"
#include "comment.h"
#include "paprint.h"
#include "magics.h"
#include "prnport.h"
#include "prnqueue.h"
#include "joblog.h"

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
//char QueryEndString[]="%!PS-Adobe-3.0 \r%%EOF\4";
char QueryEndString[]="%!PS-Adobe-3.0 \r%%EOF";

//struct {
//	struct prnbuf   *pBuf[PRNQUEUELEN];
//	BYTE Count;
//} QueryHold[NUM_OF_PRN_PORT];

#endif DEF_IEEE1284	&& BI_ATALKD

#ifdef SUPPORT_JOB_LOG
uint8 	HadLoggedA = 0;
#endif SUPPORT_JOB_LOG

static int cm_psadobe(int port, struct Mypapfile *in,struct papfile *out);

int PostScript(int port, struct Mypapfile *infile,struct papfile *outfile)
{
	char   *start, *stop;
	struct comment	*comment;
	char   matched = 0;
#ifdef SUPPORT_JOB_LOG
	char	CheckWord1[15] = {"%!PS-Adobe-3.0"};
	char	CheckWord2[6] = {"%%For"};
	BYTE	CheckWord3[1] = {0x0A};	// Line feed
	char UserWord1[5] = {"User"};		// Avoid those long names and non-ASCII names
	int	 iPosition1=0, iPosition2=0;
	char PersonName[32]={0};
#endif SUPPORT_JOB_LOG

	for (;;) {
		if ( (comment = compeek(port)) != NULL) {
			//get comment
			switch( (*comment->c_handler)(port, infile, outfile )) {
				case CH_DONE :
					matched = 1;
					continue;
				case CH_MORE :
					return( CH_MORE );
				default :
					return( CH_ERROR );
			}
		} else {
			//start a new command
			if ( infile->pf_state & PF_BOT ) {
				switch (markline( &start, &stop, infile )) {
					case 0 :
						// eof on infile
						outfile->pf_state |= PF_EOF;
						return( 0 );
					case -1 :  //to be continue ....
						return( 0 );
				}
				//begin
				if (( comment = commatch( start, stop, magics )) != NULL ) {
					compush(port, comment );
					continue;	// top of for (;;)
				}
				infile->pf_state &= ~PF_BOT;
			}
			if(infile->pf_state & PF_EOF) {
				outfile->pf_state |= PF_EOF;
				if(!matched) {
					outfile->pf_state |= PF_EOP;
					paprint(port,infile, PRN_Q_EOF);	// PostScript
#ifdef SUPPORT_JOB_LOG
					JL_EndList(port, 0);	// PostScript Completed. George Add January 26, 2007
					HadLoggedA = 0;
#endif SUPPORT_JOB_LOG
				}
				return (0);
			}
			else {
#ifdef SUPPORT_JOB_LOG
				// Microsoft Windows 2000 will come here!
				if( HadLoggedA == 1 )
				{
					// I want to know where the head of the PostScript is.
					for( iPosition1 = 0; iPosition1 < 512; iPosition1++)
					{
						if( memcmp(infile->pbuf[0]->data + iPosition1, CheckWord1, 14) == 0 )
						{
							// I want to know where the person name is.
							for(iPosition2 = iPosition1; iPosition2 < 512; iPosition2++)
							{
								if( memcmp(infile->pbuf[0]->data + iPosition2, CheckWord2, 5) == 0 )
									break;
							}
							break;
						}
					}
					
					// I want to know where the end character is.
					for( iPosition1 = 1; iPosition1 < 32; iPosition1++ )
					{
						// Find the end character.
						if( memcmp(infile->pbuf[0]->data + iPosition2 + 7 + iPosition1, CheckWord3, 1) == 0 )
						{
							memcpy(PersonName, infile->pbuf[0]->data + iPosition2 + 7, iPosition1);
							break;
						}
						
						// I don't find the end character.
						if(iPosition1 == 31)
							memcpy(PersonName, UserWord1, 5);
					}
					
					JL_PutList(3, port, PersonName, iPosition1);
				}
				
				if( HadLoggedA < 2 )
					HadLoggedA++;
#endif SUPPORT_JOB_LOG
				paprint(port, infile, PRN_Q_NORMAL);	// PostScript
				return( CH_MORE );
			}	// end of if(infile->pf_state & PF_EOF) else
		}	// end of if ( (comment = compeek(port)) != NULL) else
	} //for (;;) ...
//	return (1);
}

//"%!PS-Adobe-3.0 Query"
int cm_psquery(int port, struct Mypapfile *in,struct papfile *out)
{
	struct comment	*comment;
	char		*start, *stop;

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
//5/22/2000 marked	if(PrnLangSupport(port,P1284_POSTSCRIPT)) {
	if(PortIO[port].PrnReadBackMode != PRN_NO_PRINTER) { //5/29/2000 added
#if defined(WEBADMIN) && !defined(_PC)
//		PrnStopStatusInfo(port);
#endif
		out->pf_state |= PF_QUERY;
		comswitch(port, magics, cm_psadobe);
		return (CH_DONE);
	}
#endif DEF_IEEE1284 && BI_ATALKD

	for (;;) {
		switch ( markline( &start, &stop, in )) {
			case 0 :
				// eof on infile
				out->pf_state |= PF_EOF;
				compop(port);
				return( CH_DONE );
			case -1 :
				return( CH_MORE );
		}
		if ( in->pf_state & PF_BOT ) {
			in->pf_state &= ~PF_BOT;
		} else {
			if( (comment = commatch( start, stop, queries)) != NULL) {
				compush(port, comment );
				return( CH_DONE );
			}
		}
		consumetomark( in );
	}
}

//"%!PS-Adobe-3.0"
int cm_psadobe(int port, struct Mypapfile *in,struct papfile *out)
{
//	char		*start, *stop;
//	struct comment	*comment = compeek();
	int rc = CH_MORE;
	int flag = PRN_Q_NORMAL;
	struct iovec *tmpiov;
#ifdef SUPPORT_JOB_LOG
	char	CheckWord1[15] = {"%!PS-Adobe-3.0"};
	char	CheckWord2[6] = {"%%For"};
	BYTE	CheckWord3[1] = {0x29};	// ")"
	char UserWord1[5] = {"User"};		// Avoid those long names and non-ASCII names
	int  iPosition1=0, iPosition2=0;
	char PersonName[32]={0};
#endif SUPPORT_JOB_LOG

/*****************
	for (;;) {
		switch ( markline( &start, &stop, in )) {
			case 0 :
	    		// eof on infile
				out->pf_state |= PF_EOF;
				compop(port);
				return( CH_DONE );
			case -1 :
				return( CH_MORE );
		}

		if(in->pf_state & PF_BOT) {
			in->pf_state &= ~PF_BOT;
		} else {
			if (( comment = commatch( start, stop, headers )) != NULL ) {
				compush(port, comment );
				return( CH_DONE );
			}
		}
		*stop = '\n';
		paprint(port, start, stop-start+1, PRN_Q_NORMAL);
		consumetomark( start, stop, in );
	}
**********************************/
	if(in->pf_state & PF_EOF) {
		out->pf_state |= (PF_EOF | PF_EOP);
		compop(port);

		//Clear Printer data !
//		if(PF_BUFSIZ(in) && *(in->pf_end -1) != '\4')
//			APPEND( in,"\4", 1 );

		if(!in->iocnt) {
			in->iocnt = 1;
			in->iobuf[0].iov_len = 0;
		}
		tmpiov = &((in->iobuf)[in->iocnt-1]);

#ifdef _PC
		while(tmpiov->iov_len > (PAP_MAXDATA+4) ) {
			printf("Magic.c iobuf_col design error !\n");
		}
#endif _PC

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
		if((out->pf_state & PF_QUERY)) {
			memcpy(&(tmpiov->iov_base)[tmpiov->iov_len],QueryEndString,sizeof(QueryEndString)-1);
			tmpiov->iov_len += (sizeof(QueryEndString)-1);
			out->pf_state |= PF_QUERY_END;
		}
#endif DEF_IEEE1284	&& BI_ATALKD
//5/19/2000		else
//5/19/2000		if(!(tmpiov->iov_len && (tmpiov->iov_base)[tmpiov->iov_len-1] == '\4'))
//5/19/2000			(tmpiov->iov_base)[tmpiov->iov_len++] = '\4';

		rc =  CH_DONE;
		flag = PRN_Q_EOF;
	}

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
/*
	if((out->pf_state & PF_QUERY)) {
		if(!(out->pf_state & PF_QUERY_END)) {
    		papqueryhold(port, in);
    		return rc;
		} else {
			papqueryprint(port);
		}
	}
*/
#endif DEF_IEEE1284 && BI_ATALKD

	if(out->pf_state & PF_QUERY)
		ascpaprint(port,in,flag);
	else
	{
#ifdef SUPPORT_JOB_LOG
		// Apple Mac OS X will come here!
		if( HadLoggedA == 0 )
		{
			// I want to know where the head of the PostScript is.
			for( iPosition1 = 0; iPosition1 < 512; iPosition1++)
			{
				if( memcmp(in->pbuf[0]->data + iPosition1, CheckWord1, 14) == 0 )
				{
					// I want to know where the person name is.
					for(iPosition2 = iPosition1; iPosition2 < 512; iPosition2++)
					{
						if( memcmp(in->pbuf[0]->data + iPosition2, CheckWord2, 5) == 0 )
							break;
					}
					break;
				}
			}
			
				// I want to know where the end character is.
				for( iPosition1 = 1; iPosition1 < 32; iPosition1++ )
				{
					// Find the end character.
					if( memcmp(in->pbuf[0]->data + iPosition2 + 8 + iPosition1, CheckWord3, 1) == 0 )
					{
						memcpy(PersonName, in->pbuf[0]->data + iPosition2 + 8, iPosition1);
						break;
					}
				
				// I don't find the end character.
					if( iPosition1 == 31 )
					memcpy(PersonName, UserWord1, 5);
			}
			
			JL_PutList(3, port, PersonName, iPosition1);
		}
		
		if( HadLoggedA < 1 )
			HadLoggedA++;
#endif SUPPORT_JOB_LOG

		paprint(port, in, flag);	// cm_psadobe. PRN_Q_NORMAL

#ifdef SUPPORT_JOB_LOG
		if( flag == PRN_Q_EOF )
		{
			JL_EndList(port, 0);	// cm_psadobe Completed. George Add January 26, 2007
			HadLoggedA = 0;
		}
#endif SUPPORT_JOB_LOG
	}	// end of if(out->pf_state & PF_QUERY) else

//	consumetomark(in->pf_cur,in->pf_end, in);
	return rc;
}

#if defined(DEF_IEEE1284) && defined(BI_ATALKD)
/*
void papqueryhold(int port, struct Mypapfile *in)
{
	struct prnbuf   *pbuf;
	int    nsize,i, j;

	in->pf_state |= PF_PRINT;
	for(i = 0 ; i < 2 ; i++) {
		pbuf = in->pbuf[i];
		pbuf->size = 0;
		for(j = i*5; j < min((i+1)*5, in->iocnt); j++) {
			nsize = IOV_LEN(*in,j);
			memcpy(pbuf->data+pbuf->size,IOV_BASE(*in,j),nsize);
			pbuf->size += nsize;
		}

		if(pbuf->size == 0) {
			PrnPutInQueueBuf(port,pbuf);
		}
		else {
			if(QueryHold[port].Count >= PRNQUEUELEN) {
				papqueryprint(port);
			}
			QueryHold[port].pBuf[QueryHold[port].Count++] = pbuf;
		}

		in->pbuf[i] = NULL;
	}
	in->iocnt = 0;
}

void papqueryprint(int port)
{
	int i;

	for(i = 0 ; i <	QueryHold[port].Count; i++) {
		PrnPutOutQueueBuf(port,QueryHold[port].pBuf[i],PRN_Q_NORMAL);
	}
	QueryHold[port].Count = 0;
}

void papqueryfree(int port)
{
	int i;

	for(i = 0 ; i <	QueryHold[port].Count; i++) {
		PrnPutInQueueBuf(port,QueryHold[port].pBuf[i]);
	}
	QueryHold[port].Count = 0;
}

char papqueryfull(int port, int needbuf)
{
	return ((QueryHold[port].Count + needbuf) > PRNQUEUELEN? 1: 0);
}
*/
#endif DEF_IEEE1284 && BI_ATALKD



char	*Query = "Query";

//"%!PS-Adobe-"
int cm_psswitch(int port, struct Mypapfile *in,struct papfile *out)
{
	char		*start, *stop, *p;

	switch ( markline( &start, &stop, in )) {
		case 0 :
			// eof on infile
			out->pf_state |= PF_EOF;
			compop(port);
			return( 0 );
		case -1 :
			return( CH_MORE );
	}

	for ( p = start; p < stop; p++ ) {
		if ( *p == ' ' || *p == '\t' ) {
			break;
		}
	}
	for ( ; p < stop; p++ ) {
		if ( *p != ' ' && *p != '\t' ) {
			break;
		}
	 }

	if ( stop - p >= strlen( Query ) &&
		strncmp( p, Query, strlen( Query )) == 0 )
	{
		if ( comswitch(port, magics, cm_psquery ) < 0 ) {
			return (CH_DONE); //design error !!!
		}
	} else {
		if ( comswitch( port, magics, cm_psadobe ) < 0 ) {
			return (CH_DONE); //design error !!!
		}
	}
	return( CH_DONE );
}

struct comment	magics[] = {
    { "%!PS-Adobe-3.0 Query",	0,	cm_psquery, C_FULL },
    { "%!PS-Adobe-3.0",         0,	cm_psadobe, C_FULL },
    { "%!PS-Adobe-",            0,  cm_psswitch,	 0 },
    { 0 },
};
