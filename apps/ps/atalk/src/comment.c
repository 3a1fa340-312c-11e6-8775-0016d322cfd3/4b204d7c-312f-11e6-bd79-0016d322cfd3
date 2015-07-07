/*
 * Copyright (c) 1990,1994 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */
 
#include <stdio.h>
#include <stdlib.h>
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

struct comstate	*comstate[NUM_OF_PRN_PORT];
void comfree(int port)
{
	struct comstate	*cs;

	while(comstate[port])
	{
		compop(port);
	}
}

void compop(int port)
{
	struct comstate	*cs;

	cs = comstate[port];
	comstate[port] = cs->cs_prev;
	free( cs );
}

void compush(int port, struct comment *comment)
{
	struct comstate	*cs;

	if (( cs = (struct comstate *)malloc( sizeof( struct comstate ))) ==
	    NULL ) {
#ifdef PC_OUTPUT
		printf("comment.c (compush) alloc memory error \n");
#endif PC_OUTPUT
		return;
    }
	memset(cs, 0x00, sizeof( struct comstate ));
	cs->cs_comment = comment;
	cs->cs_prev = comstate[port];
	cs->cs_flags = 0;
	comstate[port] = cs;
}

int comswitch(int port, struct comment *comments,int (*handler)())
{
	struct comment	*c, *comment = NULL;

	for ( c = comments; c->c_begin; c++ ) {
		if ( c->c_handler == handler ) {
			comment = c;
		}
	}
	if ( comment == NULL || comment->c_handler != handler ) {
#ifdef PC_OUTPUT
		printf("(ATALKD) : comswitch can't find handler (comment.c) !" );
#endif PC_OUTPUT
		return( -1 );
	}
	compop(port);
	compush(port, comment );
	return( 0 );
}

int comcmp(char *start, char *stop, char *str, int how)
{
	int		cc, len;

	len = stop - start;
	cc = strlen( str );
	if ( how & C_FULL ) {
		if ( cc == len & strncmp( str, start, cc ) == 0 ) {
			return( 0 );
		}
	} else {
		if ( cc <= len && strncmp( str, start, cc ) == 0 ) {
			return( 0 );
		}
	}
	return( 1 );
}

struct comment *
commatch(char *start,char *stop,struct comment comments[])
{
	struct comment	*comment;

	for ( comment = comments; comment->c_begin; comment++ ) {
		if( comcmp( start, stop, comment->c_begin, comment->c_flags ) == 0 ) {
			break;
		}
	}
	if( comment->c_begin) {
		return( comment );
	} else {
		return( NULL );
	}
}
