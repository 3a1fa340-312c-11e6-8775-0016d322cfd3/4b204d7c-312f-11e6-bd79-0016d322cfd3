#include <stdio.h>
#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "eeprom.h"
#include "atalkd.h"
#include "atp.h"
#include "pap.h"
#include "file.h"
#include "paprint.h"
#include "prnqueue.h"
#include "joblog.h"

#define CtrlA 0x01
#define CtrlC 0x03
#define CtrlD 0x04
#define CtrlE 0x05
#define CtrlQ 0x11
#define CtrlS 0x13
#define CtrlT 0x14
#define ESC 0x1b
#define CtrlBksl 0x1c

#define IsNormalPacket (flags != PRN_Q_NORMAL && i == 0 && in->iocnt > 5)
void ascpaprint(int port, struct Mypapfile *in, int flags);
void binpaprint(int port, struct Mypapfile *in, int flags);
int16 xBCP_Encode(struct prnbuf *pbuf, BYTE *inbuf, int16 insize);

char TBCPEndString[]= "\x1b%-12345X";

#ifdef CONST_DATA
extern const BYTE far BCP_escaped[32];
extern const BYTE far TBCP_escaped[32];
#else
const BYTE TBCP_escaped[32] =
{
	0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0
};

const BYTE BCP_escaped[32] =
{
	0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
};
#endif

const BYTE *xBCP_escaped;

void paprint(int port, struct Mypapfile *in, int flags)
{
	switch(EEPROM_Data.ATDataFormat[port]) {
	case AT_COMM_NONE:
		ascpaprint(port,in,flags);
		break;
	case AT_COMM_TBCP:
		xBCP_escaped = TBCP_escaped;
		binpaprint(port,in,flags);
		break;
	case AT_COMM_BCP:
		xBCP_escaped = BCP_escaped;
		binpaprint(port,in,flags);
		break;
	default:
#ifdef PC_OUTPUT
		while(1) printf("papint.c, paprint() design error !\n");
#endif PC_OUTPUT
		break;
	}
}

void ascpaprint(int port, struct Mypapfile *in, int flags)
{
	struct prnbuf   *pbuf;
	int    nsize,i,j;
	char * iov;	//615wu

	if(flags != PRN_Q_NORMAL) {
		struct iovec *tmpiov;
		tmpiov = &((in->iobuf)[in->iocnt-1]);
		iov  = tmpiov->iov_base;
		iov[tmpiov->iov_len++] = '\4';
    }

//	do {
//		while((pbuf = PrnGetInQueueBuf(port)) == NULL) {
//			//queue buffer is full, waiting for printing
//			kwait(0);
//			//printf("Warning: Can not get In buffer !!!\n");
//		}
//		nsize = min(PF_BUFSIZ(papf),BLOCKSIZE);
//		memcpy(pbuf->data,papf->pf_cur,nsize);
//		pbuf->size = nsize;
//		papf->pf_cur += nsize;
//		PrnPutOutQueueBuf(port,pbuf,flags);
//	} while(PF_BUFSIZ(papf) != 0);
//	papf->pf_cur = papf->pf_end = papf->pf_buf;

#ifdef PC_OUTPUT
	if(in->iocnt < PAP_MAXQUANTUM) {
		printf("Print PAPD finish !\n");
	}
#endif PC_OUTPUT
	in->pf_state |= PF_PRINT;
	for(i = 0 ; i < 2 ; i++) {
		pbuf = in->pbuf[i];
		pbuf->size = 0;
		for(j = i*5; j < min((i+1)*5, in->iocnt); j++) {
			nsize = IOV_LEN(*in,j);
			memcpy(pbuf->data+pbuf->size,IOV_BASE(*in,j),nsize);
			pbuf->size += nsize;
		}

		if(i == 1 && pbuf->size == 0) {
			PrnPutInQueueBuf(port,pbuf);
		}
		else {
#ifdef SUPPORT_JOB_LOG
			JL_AddSize(port, pbuf->size);	// ascpaprint.
#endif SUPPORT_JOB_LOG
			PrnPutOutQueueBuf(port,pbuf,IsNormalPacket?PRN_Q_NORMAL:flags);
		}

		in->pbuf[i] = NULL;
	}
//	in->iobuf_row= in->iobuf_next_row= in->iobuf_col= in->iobuf_next_col= 0;
	in->iocnt = 0;
}
//////////////////////////////////////////////////////////////////////////////
/* Copyright (C) 1993, 1994, 1996, 1998 Aladdin Enterprises.  All rights reserved.

  This file is part of GNU Ghostscript.

  GNU Ghostscript is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
  to anyone for the consequences of using it or for whether it serves any
  particular purpose or works at all, unless he says so in writing.  Refer
  to the GNU General Public License for full details.

  Everyone is granted permission to copy, modify and redistribute GNU
  Ghostscript, but only under the conditions described in the GNU General
  Public License.  A copy of this license is supposed to have been given
  to you along with GNU Ghostscript so you can know your rights and
  responsibilities.  It should be in a file named COPYING.  Among other
  things, the copyright notice and this notice must be preserved on all
  copies.

  Aladdin Enterprises supports the work of the GNU Project, but is not
  affiliated with the Free Software Foundation or the GNU Project.  GNU
  Ghostscript, as distributed by Aladdin Enterprises, does not require any
  GNU software to build or run it.
*/
/////////////////////////////////////////////////////////////////////////////


void binpaprint(int port, struct Mypapfile *in, int flags)
{
	struct prnbuf *pbuf[4];
	int    nsize,rsize, i,j;


#ifdef PC_OUTPUT
	if(in->iocnt < PAP_MAXQUANTUM) {
		printf("Print PAPD finish !\n");
	}
#endif PC_OUTPUT
	in->pf_state |= PF_PRINT;

	for(i = 0 ; i < 2 ; i ++) {
		while((pbuf[i] = PrnGetInQueueBuf(port)) == NULL) 
			cyg_thread_yield();
			pbuf[i]->size = 0;
#ifdef PC_OUTPUT
			printf("(PAPRINT.C) Get Print Queue Error !\n");
#endif PC_OUTPUT
	}
	pbuf[2] = in->pbuf[0];
	pbuf[2]->size = 0;
	in->pbuf[0] = NULL;

	pbuf[3] = in->pbuf[1];
	pbuf[3]->size = 0;
	in->pbuf[1] = NULL;

	if((in->pf_state & PF_BOT) && EEPROM_Data.ATDataFormat[port] == AT_COMM_TBCP) {
		pbuf[0]->data[0] = 0x01;
		pbuf[0]->data[1] = 0x4D;
		pbuf[0]->size = 2;

		in->pf_state &= ~PF_BOT;
	}

	j = 0;
	for(i = 0 ; i < in->iocnt; i++) {
		nsize = IOV_LEN(*in,i);
		rsize = 0;
		do {
			nsize -= rsize;
			rsize = xBCP_Encode(pbuf[j],IOV_BASE(*in,i)+rsize, nsize);
			if(pbuf[j]->size >= (BLOCKSIZE-1)) {
#ifdef SUPPORT_JOB_LOG
				JL_AddSize(port, pbuf[j]->size);	// binpaprint.
#endif SUPPORT_JOB_LOG
				PrnPutOutQueueBuf(port,pbuf[j++],PRN_Q_NORMAL);
			}
//			while(j >= 4)
//			    kwait(NULL);
		} while(rsize < nsize);
	}

	if(flags != PRN_Q_NORMAL) {
		if(j < 3) {
#ifdef SUPPORT_JOB_LOG
			JL_AddSize(port, pbuf[j]->size);	// binpaprint.
#endif SUPPORT_JOB_LOG
		    //still have at least one buffer
			PrnPutOutQueueBuf(port,pbuf[j++],PRN_Q_NORMAL);
		}
		(pbuf[j]->data)[pbuf[j]->size++] = '\4';
		if(EEPROM_Data.ATDataFormat[port] == AT_COMM_TBCP) {
			memcpy((pbuf[j]->data)+pbuf[j]->size,TBCPEndString,sizeof(TBCPEndString)-1);
			pbuf[j]->size += (sizeof(TBCPEndString)-1);
		}
	}
#ifdef SUPPORT_JOB_LOG
	JL_AddSize(port, pbuf[j]->size);	// binpaprint.
#endif SUPPORT_JOB_LOG
	PrnPutOutQueueBuf(port,pbuf[j++],flags);


	for(; j < 4 ; j++) {
		PrnPutInQueueBuf(port,pbuf[j]);
	}

	in->iocnt = 0;
}

int16 xBCP_Encode(struct prnbuf *pbuf, BYTE *inbuf, int16 insize)
{
	BYTE *wp = pbuf->data+pbuf->size;
	BYTE *wend = pbuf->data+(BLOCKSIZE-1);
	BYTE *rp = inbuf;
	BYTE *rend = inbuf + insize;
	BYTE ch;

	while (wp < wend && rp < rend) {
		ch = *(rp++);
		if (ch <= 31 && xBCP_escaped[ch]) {
			*(wp++) = CtrlA;
			ch ^= 0x40;
		}
		*(wp++) = ch;
	}
	pbuf->size = (wp - pbuf->data);
	return (insize - (rend - rp));
}

#if 0

FILE *papopen(int port);
void papclose(FILE **fp);
void papwrite(FILE *fp,char *data, int len);

void paprint(int port, struct papfile *papf, int flags)
{
	static FILE *fp[NUM_OF_PRN_PORT];
	int nsize;

	if(fp[port] == NULL)
		if((fp[port] = papopen(port)) == NULL) printf("(paprint.c) Open PAP File Error !\n");

	while(PF_BUFSIZ(papf) != 0){
		nsize = min(PF_BUFSIZ(papf),BLOCKSIZE);
		papwrite(fp[port],papf->pf_cur,nsize);
		papf->pf_cur += nsize;
	}

	papf->pf_cur = papf->pf_end = papf->pf_buf;

	if(flags == PRN_Q_EOF) papclose(&fp[port]);
}

FILE *papopen(int port)
{
	char *FileName = "ATALK-1.PS";
	FileName[6] = port + '1';
	return (fopen(FileName,"w+b"));
}

void papclose(FILE **fp)
{
	fclose(*fp);
	*fp = NULL;
}

void papwrite(FILE *fp,char *data, int len)
{
	fwrite(data,len,1,fp);
}

#endif 0
