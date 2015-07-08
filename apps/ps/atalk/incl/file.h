/*
 * Copyright (c) 1990,1991 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */
#ifndef _FILE_H
#define _FILE_H

struct papfile {
	int			pf_state;
//    struct state	*pf_xstate;
	int			pf_len;
	char		*pf_buf;
	char		*pf_cur;
	char		*pf_end;
};

struct Mypapfile {
	int  pf_state;
	struct prnbuf   *pbuf[2];
	struct iovec iobuf[PAP_MAXQUANTUM+2]; //0-7 : recv packet, 8: for markline 9: for outfile
	uint8  iobuf_row;  //current row of	iobuf
	uint8  iobuf_next_row;  //next row of	iobuf
	uint16 iobuf_col;  //current column of iobuf
	uint16 iobuf_next_col;  //next column of iobuf
	uint8  iocnt;     //buffer used count
};


#define PF_BOT		     (1<<0)
#define PF_EOF		     (1<<1)
#define PF_QUERY	     (1<<2)	//begin ieee1284 qurry
#define PF_EOP           (1<<3)  //end of printing
#define PF_PRINT         (1<<4)  //have printed
#define PF_READBACK      (1<<5)  //begin ieee1284 readback 3/22/2000
#define PF_BPS           (1<<6)  //begin PS-ADOBE
#define PF_QUERY_END     (1<<7)  //end ieee1284 query  5/5/2000

#define APPEND( pf, data, len )	\
	if ( (pf)->pf_end + (len) > (pf)->pf_buf + (pf)->pf_len ) { \
		morespace( (pf), (data), (len)); \
	} else { \
		bcopy( (data), (pf)->pf_end, (len)); \
		(pf)->pf_end += (len); \
	}
#define PF_BUFSIZ( pf )		((pf)->pf_end - (pf)->pf_cur)
#define CONSUME( pf, len )	{(pf)->pf_cur += (len); \
	  if((pf)->pf_cur >= (pf)->pf_end) \
	     (pf)->pf_cur = (pf)->pf_end = (pf)->pf_buf;\
	  }

#define PF_MORESPACE 1024

int markline(char **start,char **stop,struct Mypapfile *pf);
int morespace(struct papfile *pf,char *data,int len);
int consumetomark(struct Mypapfile *pf);

#endif _FILE_H
