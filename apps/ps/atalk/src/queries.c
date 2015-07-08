/*
 * Copyright (c) 1990,1994 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */
#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "atalkd.h"
#include "atp.h"
#include "pap.h"
#include "file.h"
#include "comment.h"
#include "atprn.h"
#include "ppd.h"

int cq_default(int port, struct Mypapfile *in,struct papfile *out)
{
	char		*start, *stop, *p;
	struct comment	*comment = compeek(port);

	for (;;) {
		switch ( markline( &start, &stop, in )) {
			case 0 :
//				return( 0 );  //8/4/99 marked
				return(CH_MORE ); //In normal case cann't into this statement
			case -1 :
				return( CH_MORE );
		}

		if ( comgetflags(port) == 0 ) {	// started
			if ( comment->c_end ) {
				comsetflags(port, 1 );
			} else {
				compop(port);

				consumetomark( in );
				return( CH_DONE );
			}
		} else {
			// return default
			if (comcmp( start, stop, comment->c_end, 0 ) == 0 ) {
				for ( p = start; p < stop; p++ ) {
					if ( *p == ':' ) break;
				}
				p++;
				while ( *p == ' ' ) {
					p++;
				}
				*stop = '\n';
				APPEND( out, p, stop - p + 1 );
				compop(port);
				consumetomark( in );
				return( CH_DONE );
			}
		}
		consumetomark( in );
	}
}

void cq_font_answer(int port, char *start,char *stop,struct papfile *out)
{
	char   *p, *q, buf[ 256 ];

	p = start;
	while ( p < stop ) {
		while (( *p == ' ' || *p == '\t' ) && p < stop ) {
	    	p++;
		}

		q = buf;
		while ( *p != ' ' && *p != '\t' &&
			*p != '\n' && *p != '\r' && p < stop ) {
		    *q++ = *p++;
		}

		if ( q != buf ) {
		    *q = '\0';

			APPEND( out, "/", 1 );
			APPEND( out, buf, strlen( buf ));
			APPEND( out, ":", 1 );
#ifdef _PC
			if(ppd_font(port,buf) == NULL ) {
				APPEND( out, "No\n", 3 );
			} else {
				APPEND( out, "Yes\n", 4 );
			}
#else
			APPEND(out,"No\n",3);
//			if(strcmp("Taipei",buf) != 0) {
//				APPEND(out,"Yes\n",4);
//			} else {
//				APPEND(out,"No\n",3);
//			}
#endif _PC
		}
	}
	return;
}

int cq_font(int port, struct Mypapfile *in,struct papfile *out)
{
	char		*start, *stop, *p;
	struct comment	*comment = compeek(port);

	for (;;) {
		switch ( markline( &start, &stop, in )) {
			case 0 :
//				return( 0 );  //8/4/99 marked
				return(CH_ERROR );//In normal case cann't into this statement
			case -1 :
				return( CH_MORE );
		}

		if(comgetflags(port) == 0) {
			comsetflags(port, 1 );

			for ( p = start; p < stop; p++ ) {
				if (*p == ':' ) break;
			}
			p++;
			cq_font_answer(port, p, stop, out );
		} else {
			if ( comgetflags(port) == 1 &&
				comcmp( start, stop, COMM_CONT, 0 ) == 0 ) {
				// continuation
				for ( p = start; p < stop; p++ ) {
					if ( *p == ' ' ) break;
				}
				p++;
				cq_font_answer(port, p, stop, out );
			} else {
				comsetflags(port, 2 );
				if ( comcmp( start, stop, comment->c_end, 0 ) == 0 ) {
					APPEND( out, "*\n", 2 );
					compop(port);
					consumetomark( in );
					return( CH_DONE );
				}
			}
		}
		consumetomark( in );
	}
}

int cq_query(int port, struct Mypapfile	*in,struct papfile *out)
{
	char		*start, *stop, *p;
	struct comment	*c, *comment = compeek(port);
	struct ppd_feature	*pfe;

	for (;;) {
		switch ( markline( &start, &stop, in )) {
			case 0 :
//				return( 0 );  //8/4/99
				return(CH_MORE ); //In normal case cann't into this statement
			case -1 :
				return( CH_MORE );
		}

		if ( comgetflags(port) == 0 ) {
			comsetflags(port, 1 );

			// parse for query
			for ( p = start; p < stop; p++ ) {
				if ( *p == ':' ) {
					break;
				}
			}
			p++;
			while ( *p == ' ' ) {
				p++;
			}
			if(memcmp(p,"ADOSpooler",10) != NULL || (p[10] != '\r' && p[10] != '\n'))
			{
				if ( comswitch(port, queries, cq_default ) < 0 ) {
#ifdef PC_OUTPUT
				    printf("cq_feature: can't find default!\n" );
#endif PC_OUTPUT
				}
				return( CH_DONE );
			}

			APPEND( out, "0\n", 2 );
		} else {
	    	if ( comcmp( start, stop, comment->c_end, 0 ) == 0 ) {
				compop(port);
				consumetomark( in );
				return( CH_DONE );
			}
		}
		consumetomark( in );
	}//for (;;).....
}

#ifdef _PC
int cq_font_list(int port, struct Mypapfile *in,struct papfile *out)
{
	char		*start, *stop, *p;
	struct comment	*comment = compeek(port);
	struct ppd_font	*pfo;

	ppd_init(port);

	for (;;) {
		switch ( markline( &start, &stop, in )) {
			case 0 :
//				return( 0 );  //8/4/99
			case -1 :
				return( CH_MORE );
		}

		if(comgetflags(port) == 0) {
			comsetflags(port, 1 );
			for ( pfo = ppd_fonts; pfo; pfo = pfo->pd_next ) {
				APPEND( out, "/", 1 );
				APPEND( out, pfo->pd_font, strlen(pfo->pd_font));
				APPEND( out, "\n", 1 );
			}
		} else {
			if ( comcmp( start, stop, comment->c_end, 0 ) == 0 ) {
				APPEND( out, "*\n", 2 );
				compop(port);
				consumetomark( in );
				return( CH_DONE );
			}
		}
		consumetomark( in );
	}
}
#endif _PC

/********************

char	*rmjobfailed = "Failed\n";
char	*rmjobok = "Ok\n";

cq_rmjob( in, out )
    struct papfile	*in, *out;
{
    char		*start, *stop, *p;
    int			job;

	switch ( markline( &start, &stop, in )) {
		case 0 :
//			return( 0 );
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

	*stop = '\0';
	if ( p < stop && ( job = atoi( p )) > 0 ) {
		lp_rmjob( job );
		APPEND( out, rmjobok, strlen( rmjobok ));
	} else {
		APPEND( out, rmjobfailed, strlen( rmjobfailed ));
	}

	compop(port);
	consumetomark( in );
	return( CH_DONE );
}

cq_listq( in, out )
    struct papfile	*in, *out;
{
	char *start, *stop;

	switch ( markline( &start, &stop, in )) {
		case 0 :
//			return( 0 );
		case -1 :
			return( CH_MORE );
	}

	if ( lp_queue( out )) {
		syslog( LOG_ERR, "cq_listq: lp_queue failed" );
	}

	compop(port);
	consumetomark( in );
	return( CH_DONE );
}
**********************************************************************/


//*
//* All queries start with %%?Begin and end with %%?End.  Note that the
//* "Begin"/"End" general queries have to be last.
//*
struct comment	queries[] = {
#ifdef _PC
	{ "%%?BeginFontListQuery","%%?EndFontListQuery",	cq_font_list,	0 },
#endif _PC
	{ "%%?BeginFontQuery",	"%%?EndFontQuery",	cq_font,	0 },
	{ "%%?Begin",		"%%?End",		cq_default,	0 },
	{ 0 }
};

