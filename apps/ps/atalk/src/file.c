/*
 * Copyright (c) 1990,1994 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */ 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "atalkd.h"
#include "atp.h"
#include "pap.h"
#include "file.h"

int markline(char **start,char **stop,struct Mypapfile *pf)
{
	char ch, *out_buf;
	int16    *out_len;
	int      rc_len,i,j;
	char * iov;	//615wu

	out_buf = *start = (pf->iobuf[PAP_MAXQUANTUM]).iov_base;
	out_len = &(pf->iobuf[PAP_MAXQUANTUM]).iov_len;

	if(pf->iocnt == 0 && *out_len == 0 && ( pf->pf_state & PF_EOF )) {
		return( 0 );  //EOF
	}

#ifdef _PC
	if(pf->iobuf_row >= pf->iocnt) {
		printf("FILE.C iobuf design error \n");
	}
#endif _PC

	if(pf->iocnt) {
		//still have data in recv packet
		i = pf->iobuf_row;
		j = pf->iobuf_col;

		//Search Until '\n' or	'\r' or LEN > (BLOCKSIZE/5)
		for(; i < pf->iocnt; i++) {
			for(; j < pf->iobuf[i].iov_len; j++) {
				iov  = pf->iobuf[i].iov_base;
				ch = out_buf[(*out_len)++] = iov[j];
				if(*out_len >= (BLOCKSIZE/5))
				 	break;
				if( ch == '\n' || ch == '\r')
					break;
			}
			if(j < pf->iobuf[i].iov_len) break;
			j = 0;
		}
		pf->iobuf_next_row = i;
		pf->iobuf_next_col = j+1;
		if(i >= pf->iocnt) {
			//no more data in packet
			pf->iocnt = 0;
			if(!(pf->pf_state & PF_EOF)) {
				return( -1 ); //keep some data in iobuf[8]
			}
		}
	}

	//no data in recv packet buf, only data in iobuf[8]
	*start = out_buf;
	if(*out_len) {
		*stop  = out_buf + *out_len - 1;
	}
	else {
		out_buf[0] = '\n';
		*stop = out_buf;
		*out_len = 1;
	}
	rc_len = *out_len;
	*out_len = 0;

	return (rc_len);
}

int consumetomark(struct Mypapfile *in)
{
	in->iobuf_row = in->iobuf_next_row;
	in->iobuf_col = in->iobuf_next_col;
	return (1);
}

/*
int consumetomark(char *start,char *stop, struct papfile *pf)
{
	if ( start != pf->pf_cur || pf->pf_cur > stop || stop > pf->pf_end ) {
#ifdef PC_OUTPUT
		printf("\aATALKD: consumetomark error (1)  (file.c)\n");
#endif PC_OUTPUT
		return (-1);
	}

	pf->pf_cur = stop + 1;		// past the stop char
	if ( pf->pf_cur > pf->pf_end ) {
#ifdef PC_OUTPUT
		printf("\aATALKD: consumetomark error (2)  (file.c)\n");
#endif PC_OUTPUT
		return (-1);
	}
	if ( pf->pf_cur == pf->pf_end ) {
		pf->pf_cur = pf->pf_end = pf->pf_buf;
	}
	return (1);
}
*/

int morespace(struct papfile *pf,char *data,int len)
{
	char		*nbuf;
	int			nsize;

	if ( pf->pf_cur != pf->pf_buf ) {
		// pull up
		bcopy( pf->pf_cur, pf->pf_buf, PF_BUFSIZ( pf ));
		pf->pf_end = pf->pf_buf + PF_BUFSIZ( pf );
		pf->pf_cur = pf->pf_buf;
	}

	if( (char huge *)pf->pf_end + len > (char huge *)pf->pf_buf + pf->pf_len ) {
		// make more space (PF_MORESPACE X N)
		nsize = (( pf->pf_len + len ) / PF_MORESPACE +
		        (( pf->pf_len + len ) % PF_MORESPACE != 0 )) * PF_MORESPACE;
		if ( pf->pf_buf ) {
			//buf already exist
			if (( nbuf = (char *)realloc( pf->pf_buf, nsize )) == 0 ) {
#ifdef PC_OUTPUT
				printf("ATALKD:MoreSpace error(0): Not enough memory !\n");
#endif PC_OUTPUT
				return (-1);
			}
		} else {
			if (( nbuf = (char *)malloc( nsize )) == 0 ) {
#ifdef PC_OUTPUT
				printf("ATALKD:MoreSpace error(1): Not enough memory !\n");
#endif PC_OUTPUT
				return (-1);
			}
		}
		pf->pf_len = nsize;
		pf->pf_end = nbuf + ( pf->pf_end - pf->pf_buf );
		pf->pf_cur = nbuf + ( pf->pf_cur - pf->pf_buf );
		pf->pf_buf = nbuf;
	}

	bcopy( data, pf->pf_end, len );
	pf->pf_end += len;
	return (1);
}
