 /* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Inter-process communication and named pipe handling
   Copyright (C) Andrew Tridgell 1992-1998

   SMB Version handling
   Copyright (C) John H Terpstra 1995-1998
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   */
/*
   This file handles the named pipe and mailslot calls
   in the SMBtrans protocol
   */

#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include <stdio.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "btorder.h"

#include "prnqueue.h" //0620

#include "smbinc.h"
#include "smb.h"
#include "nterr.h"

//for pipes_struct must include
#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"

#define issafe(c) (isalnum((c&0xff)) || strchr("-._",c))
#define HAVE_STDARG_H 1

//form psmain.c
extern char * strupr ( char * string );

//from smbutil.c
extern int set_message(char *buf,int num_words,int num_bytes,int zero);

//from utilsock.c
extern int send_smb(int fd,char *buffer);

//from srvpipe.c
extern int write_to_pipe(pipes_struct *p, char *data, uint32 n);
extern int read_from_pipe(pipes_struct *p, char *data, uint32 n);
extern int wait_rpc_pipe_hnd_state(pipes_struct *p, uint16 priority);
extern int set_rpc_pipe_hnd_state(pipes_struct *p, uint16 device_state);
extern pipes_struct *get_rpc_pipe(int pnum, int threadid);;

//from conn.c
extern void close_cnum(connection_struct *conn, int threadid);

//from process.c
extern int receive_next_smb(char *inbuf, int bufsize, int timeout, int ProcSockID ,int threadid);

//form smbutil
extern int error_packet(char *inbuf,char *outbuf,int error_class,uint32 error_code); //line just for debug

//form unistr.c
extern char *skip_string(char *buf,uint32 n);

//from pippes.c
extern void fail_next_srvsvc_open(int threadid);

#ifdef CHECK_TYPES
#undef CHECK_TYPES
#endif
#define CHECK_TYPES 0

//0710extern int max_send[];
extern char my_netbios_names[MAX_NETBIOS_NAMES][NCBNAMSZ];
//extern fstring local_machine;
//extern fstring global_myworkgroup;
extern int chain_size[];
extern uint16 printing_conn_tid[]; //reply.c (reply_write() use ) 6/28/2001
#define NERR_Success 0
#define NERR_badpass 86
#define NERR_notsupported 50

#define NERR_BASE (2100)
#define NERR_BufTooSmall (NERR_BASE+23)
#define NERR_JobNotFound (NERR_BASE+51)
#define NERR_DestNotFound (NERR_BASE+52)
#define ERROR_INVALID_LEVEL 124

#define ACCESS_READ 0x01
#define ACCESS_WRITE 0x02
#define ACCESS_CREATE 0x04

#define SHPWLEN 8		/* share password length */
#define NNLEN 12		/* 8.3 net name length */
#define SNLEN 15		/* service name length */
#define QNLEN 12		/* queue name maximum length */

//extern int Client;
extern int smb_read_error[];

static int api_Unsupported(connection_struct *conn, uint16 vuid, 
				char *param,char *data, int mdrcnt,int mprcnt,
			    char **rdata,char **rparam,
			    int *rdata_len,int *rparam_len, int threadid);
static int api_TooSmall(uint16 vuid, char *param,char *data,
			 int mdrcnt,int mprcnt,
			 char **rdata,char **rparam,
			 int *rdata_len,int *rparam_len);

//******************************************************************

char Smbprinterserver[3][SERVICENAME_LENGTH] = {"lp1", "lp2", "lp3"};  //0404/2001 replace ServicePtrs

//********************************************************************
static int CopyExpanded(int snum, char** dst, char* src, int* n)
{
	pstring buf;
	int l;

	if (!src || !dst || !n || !(*dst)) return(0);
	strncpy(buf,src,sizeof(buf)/2);	
	strncpy(*dst,buf,*n);
	l = strlen(*dst) + 1;
	(*dst) += l;
	(*n) -= l;
	return l;
}

static int StrlenExpanded(int snum, char* s)
{
	pstring buf;
	if (!s) return(0);
	strncpy(buf,s,sizeof(buf)/2);
	return strlen(buf) + 1;
}

static char* Expand(int snum, char* s)
{
	static pstring buf;
	if (!s) return(NULL);
	strncpy(buf,s,sizeof(buf)/2);
	return &buf[0];
}

/*******************************************************************
  check a API string for validity when we only need to check the prefix
  ******************************************************************/
static int prefix_ok(char *str,char *prefix)
{
  return(strncmp(str,prefix,strlen(prefix)) == 0);
}

/*******************************************************************
 copies parameters and data, as needed, into the smb buffer

 *both* the data and params sections should be aligned.  this
 is fudged in the rpc pipes by 
 at present, only the data section is.  this may be a possible
 cause of some of the ipc problems being experienced.  lkcl26dec97

 ******************************************************************/

static void copy_trans_params_and_data(char *outbuf, int align,
				char *rparam, int param_offset, int param_len,
				char *rdata, int data_offset, int data_len)
{
	char *copy_into = smb_buf(outbuf)+1;

	if(param_len < 0)
		param_len = 0;

	if(data_len < 0)
		data_len = 0;

	if (param_len)
		memcpy(copy_into, &rparam[param_offset], param_len);

	copy_into += param_len + align;

	if (data_len )
		memcpy(copy_into, &rdata[data_offset], data_len);
}


/****************************************************************************
 Send a trans reply.
 ****************************************************************************/

static void send_trans_reply(char *outbuf,
				char *rparam, int rparam_len,
				char *rdata, int rdata_len,
				int buffer_too_large, int ProcSockID, int threadid)
{
	int this_ldata,this_lparam;
	int tot_data_sent = 0;
	int tot_param_sent = 0;
	int align;

	int ldata  = rdata  ? rdata_len : 0;
	int lparam = rparam ? rparam_len : 0;

	this_lparam = lparam; /* hack */
	this_ldata  = ldata;

	align = ((this_lparam)%4);

	set_message(outbuf,10,1+align+this_ldata+this_lparam,True);

	if (buffer_too_large)
	{
		/* issue a buffer size warning.  on a DCE/RPC pipe, expect an SMBreadX... */
		SIVAL(outbuf, smb_flg2, FLAGS2_32_BIT_ERROR_CODES);
//0603		SIVAL(outbuf, smb_rcls, 0x80000000 | NT_STATUS_ACCESS_VIOLATION);
		SIVAL(outbuf, smb_rcls, 0x80000000 | 5);  //nterr.h
	}

	copy_trans_params_and_data(outbuf, align,
								rparam, tot_param_sent, this_lparam,
								rdata, tot_data_sent, this_ldata);

	SSVAL(outbuf,smb_vwv0,lparam);
	SSVAL(outbuf,smb_vwv1,ldata);
	SSVAL(outbuf,smb_vwv3,this_lparam);
	SSVAL(outbuf,smb_vwv4,(PTR_DIFF(smb_buf(outbuf)+1,outbuf+4) + 
							chain_size[threadid])); //smb_offset(p,buf)
	SSVAL(outbuf,smb_vwv5,0);
	SSVAL(outbuf,smb_vwv6,this_ldata);
	SSVAL(outbuf,smb_vwv7,(PTR_DIFF(smb_buf(outbuf)+1+
					this_lparam+align,outbuf+4) + chain_size[threadid])); //smb_offset(p,buf)
	SSVAL(outbuf,smb_vwv8,0);
	SSVAL(outbuf,smb_vwv9,0);

	send_smb(ProcSockID,outbuf);

	tot_data_sent = this_ldata;
	tot_param_sent = this_lparam;

	while (tot_data_sent < ldata || tot_param_sent < lparam)
	{
		this_lparam = lparam-tot_param_sent; /* hack */
		this_ldata  = ldata -tot_data_sent;

		if(this_lparam < 0)
			this_lparam = 0;

		if(this_ldata < 0)
			this_ldata = 0;

		align = (this_lparam%4);

		set_message(outbuf,10,1+this_ldata+this_lparam+align,False);

		copy_trans_params_and_data(outbuf, align,
									rparam, tot_param_sent, this_lparam,
									rdata, tot_data_sent, this_ldata);

		SSVAL(outbuf,smb_vwv3,this_lparam);
		SSVAL(outbuf,smb_vwv4,(PTR_DIFF(smb_buf(outbuf)+1,outbuf+4) 
							  + chain_size[threadid])); //smb_offset(p,buf)
		SSVAL(outbuf,smb_vwv5,tot_param_sent);
		SSVAL(outbuf,smb_vwv6,this_ldata);
		SSVAL(outbuf,smb_vwv7,(PTR_DIFF(smb_buf(outbuf)+1+this_lparam+align,outbuf+4)
								 + chain_size[threadid])); //smb_offset(p,buf)
		
		SSVAL(outbuf,smb_vwv8,tot_data_sent);
		SSVAL(outbuf,smb_vwv9,0);

		send_smb(ProcSockID,outbuf);

		tot_data_sent  += this_ldata;
		tot_param_sent += this_lparam;
	}
}

struct pack_desc {
  char* format;	    /* formatstring for structure */
  char* subformat;  /* subformat for structure */
  char* base;	    /* baseaddress of buffer */
  int buflen;	   /* remaining size for fixed part; on init: length of base */
  int subcount;	    /* count of substructures */
  char* structbuf;  /* pointer into buffer for remaining fixed part */
  int stringlen;    /* remaining size for variable part */		
  char* stringbuf;  /* pointer into buffer for remaining variable part */
  int neededlen;    /* total needed size */
  int usedlen;	    /* total used size (usedlen <= neededlen and usedlen <= buflen) */
  char* curpos;	    /* current position; pointer into format or subformat */
  int errcode;
};

static int get_counter(char** p)
{
  int i, n;
  if (!p || !(*p)) return(1);
  if (!isdigit((int)**p)) return 1;
  for (n = 0;;) {
    i = **p;
    if (isdigit(i))
      n = 10 * n + (i - '0');
    else
      return n;
    (*p)++;
  }
}

static int getlen(char* p)
{
  int n = 0;
  if (!p) return(0);
  while (*p) {
    switch( *p++ ) {
    case 'W':			/* word (2 byte) */
      n += 2;
      break;
    case 'N':			/* count of substructures (word) at end */
      n += 2;
      break;
    case 'D':			/* double word (4 byte) */
    case 'z':			/* offset to zero terminated string (4 byte) */
    case 'l':			/* offset to user data (4 byte) */
      n += 4;
      break;
    case 'b':			/* offset to data (with counter) (4 byte) */
      n += 4;
      get_counter(&p);
      break;
    case 'B':			/* byte (with optional counter) */
      n += get_counter(&p);
      break;
    }
  }
  return n;
}

static int init_package(struct pack_desc* p, int count, int subcount)
{
  int n = p->buflen;
  int i;

  if (!p->format || !p->base) return(False);

  i = count * getlen(p->format);
  if (p->subformat) i += subcount * getlen(p->subformat);
//0621    i =44; //for test
  p->structbuf = p->base;
  p->neededlen = 0;
  p->usedlen = 0;
  p->subcount = 0;
  p->curpos = p->format;
  if (i > n) {
    p->neededlen = i;
    i = n = 0;
//Jesse    p->errcode = ERRmoredata;
#if 0
    /*
     * This is the old error code we used. Aparently
     * WinNT/2k systems return ERRbuftoosmall (2123) and
     * OS/2 needs this. I'm leaving this here so we can revert
     * if needed. JRA.
     */
    p->errcode = ERRmoredata;
#else
	p->errcode = ERRbuftoosmall;
#endif
  }
  else
    p->errcode = NERR_Success;
  p->buflen = i;
  n -= i;
  p->stringbuf = p->base + i;
  p->stringlen = n;
  return(p->errcode == NERR_Success);
}

#ifdef HAVE_STDARG_H
static int package(struct pack_desc* p, ...)
{
#else
static int package(va_alist)
va_dcl
{
  struct pack_desc* p;
#endif
  va_list args;
  int needed=0, stringneeded;
  char* str=NULL;
  int is_string=0, stringused;
  int32 temp;

#ifdef HAVE_STDARG_H
  va_start(args,p);
#else
  va_start(args);
  p = va_arg(args,struct pack_desc *);
#endif

  if (!*p->curpos) {
    if (!p->subcount)
      p->curpos = p->format;
    else {
      p->curpos = p->subformat;
      p->subcount--;
    }
  }
#if CHECK_TYPES
  str = va_arg(args,char*);
  SMB_ASSERT(strncmp(str,p->curpos,strlen(str)) == 0);
#endif
  stringneeded = -1;

  if (!p->curpos) {
    va_end(args);
    return(0);
  }

  switch( *p->curpos++ ) {
  case 'W':			/* word (2 byte) */
    needed = 2;
    temp = va_arg(args,int);
    if (p->buflen >= needed) SSVAL(p->structbuf,0,temp);
    break;
  case 'N':			/* count of substructures (word) at end */
    needed = 2;
    p->subcount = va_arg(args,int);
    if (p->buflen >= needed) SSVAL(p->structbuf,0,p->subcount);
    break;
  case 'D':			/* double word (4 byte) */
    needed = 4;
    temp = va_arg(args,int);
    if (p->buflen >= needed) SIVAL(p->structbuf,0,temp);
    break;
  case 'B':			/* byte (with optional counter) */
    needed = get_counter(&p->curpos);
    {
      char *s = va_arg(args,char*);
      if (p->buflen >= needed) strncpy(p->structbuf,s?s:"",needed-1);
    }
    break;
  case 'z':			/* offset to zero terminated string (4 byte) */
    str = va_arg(args,char*);
    stringneeded = (str ? strlen(str)+1 : 0);
    is_string = 1;
    break;
  case 'l':			/* offset to user data (4 byte) */
    str = va_arg(args,char*);
    stringneeded = va_arg(args,int);
    is_string = 0;
    break;
  case 'b':			/* offset to data (with counter) (4 byte) */
    str = va_arg(args,char*);
    stringneeded = get_counter(&p->curpos);
    is_string = 0;
    break;
  }
  va_end(args);
  if (stringneeded >= 0) {
    needed = 4;
    if (p->buflen >= needed) {
      stringused = stringneeded;
      if (stringused > p->stringlen) {
	stringused = (is_string ? p->stringlen : 0);
	if (p->errcode == NERR_Success) p->errcode = ERRmoredata;
      }
      if (!stringused)
	SIVAL(p->structbuf,0,0);
      else {
	SIVAL(p->structbuf,0,PTR_DIFF(p->stringbuf,p->base));
	memcpy(p->stringbuf,str?str:"",stringused);
	if (is_string) p->stringbuf[stringused-1] = '\0';
	p->stringbuf += stringused;
	p->stringlen -= stringused;
	p->usedlen += stringused;
      }
    }
    p->neededlen += stringneeded;
  }
  p->neededlen += needed;
  if (p->buflen >= needed) {
    p->structbuf += needed;
    p->buflen -= needed;
    p->usedlen += needed;
  }
  else {
    if (p->errcode == NERR_Success) p->errcode = ERRmoredata;
  }
  return 1;
}


static int check_printq_info(struct pack_desc* desc,
 			     int uLevel, char *id1, char *id2)
{

  desc->subformat = NULL;
  switch( uLevel ) {
  case 0:
    desc->format = "B13";
    break;
  case 1:
    desc->format = "B13BWWWzzzzzWW";
    break;
  case 2:
    desc->format = "B13BWWWzzzzzWN";
    desc->subformat = "WB21BB16B10zWWzDDz";
    break;
  case 3:
    desc->format = "zWWWWzzzzWWzzl";
    break;
  case 4:
    desc->format = "zWWWWzzzzWNzzl";
    desc->subformat = "WWzWWDDzz";
    break;
  case 5:
    desc->format = "z";
    break;
  case 52:
    desc->format = "WzzzzzzzzN";
    desc->subformat = "z";
    break;
  default: return False;
  }
  if (strcmp(desc->format,id1) != 0) return False;
  if (desc->subformat && strcmp(desc->subformat,id2) != 0) return False;

  return True;
}



#if CHECK_TYPES
#define PACK(desc,t,v) package(desc,t,v,0,0,0,0)
#define PACKl(desc,t,v,l) package(desc,t,v,l,0,0,0,0)
#else
#define PACK(desc,t,v) package(desc,v)
#define PACKl(desc,t,v,l) package(desc,v,l)
#endif

static void PACKI(struct pack_desc* desc,char *t,int v)
{
  PACK(desc,t,v);
}

static void PACKS(struct pack_desc* desc,char *t,char *v)
{
  PACK(desc,t,v);
}

/****************************************************************************
  get a print queue
  ****************************************************************************/

static void PackDriverData(struct pack_desc* desc)
{
  char drivdata[4+4+32];
  SIVAL(drivdata,0,sizeof drivdata); /* cb */
  SIVAL(drivdata,4,1000);	/* lVersion */
  memset(drivdata+8,0,32);	/* szDeviceName */
  pstrcpy(drivdata+8,"NULL");
  PACKl(desc,"l",drivdata,sizeof drivdata); /* pDriverData */
}


//copy on 5/8/2001 ----ron
/****************************************************************************
 Start the first part of an RPC reply which began with an SMBtrans request.
****************************************************************************/

static int api_rpc_trans_reply(char *outbuf, pipes_struct *p, 
					int ProcSockID, int threadid)
{
	char *rdata = malloc(p->max_trans_reply);
	int data_len;

	if(rdata == NULL) {
//0508		DEBUG(0,("api_rpc_trans_reply: malloc fail.\n"));
		return False;
	}

	if((data_len = (int)read_from_pipe( p, rdata, (uint32)p->max_trans_reply)) < 0) {
		free(rdata);
		return False;
	}

	send_trans_reply(outbuf, NULL, 0, rdata, data_len, 
	   			p->out_data.current_pdu_len > data_len, ProcSockID, threadid);

	free(rdata);
	return True;
}


/****************************************************************************
 When no reply is generated, indicate unsupported.
 ****************************************************************************/

static int api_no_reply(char *outbuf, int max_rdata_len, 
			int ProcSockID, int threadid)
{
	char rparam[4];

	/* unsupported */
	SSVAL(rparam,0,NERR_notsupported);
	SSVAL(rparam,2,0); /* converter word */

//0607	DEBUG(3,("Unsupported API fd command\n"));

	/* now send the reply */
	send_trans_reply(outbuf, rparam, 4, NULL, 0, False, ProcSockID, threadid);

	return -1;
}

/****************************************************************************
 WaitNamedPipeHandleState 
****************************************************************************/

static int api_WNPHS(char *outbuf, pipes_struct *p, char *param, int param_len, int ProcSockID, int threadid)
{
	uint16 priority;

	if (!param || param_len < 2)
		return False;

	priority = SVAL(param,0);
//	DEBUG(4,("WaitNamedPipeHandleState priority %x\n", priority));

	if (wait_rpc_pipe_hnd_state(p, priority)) {
		/* now send the reply */
		send_trans_reply(outbuf, NULL, 0, NULL, 0, False, ProcSockID, threadid);
		return True;
	}
	return False;
}


/****************************************************************************
 SetNamedPipeHandleState 
****************************************************************************/

static int api_SNPHS(char *outbuf, pipes_struct *p, char *param, int param_len, int ProcSockID, int threadid)
{
	uint16 id;

	if (!param || param_len < 2)
		return False;

	id = SVAL(param,0);
//	DEBUG(4,("SetNamedPipeHandleState to code %x\n", id));

	if (set_rpc_pipe_hnd_state(p, id)) {
		/* now send the reply */
		send_trans_reply(outbuf, NULL, 0, NULL, 0, False, ProcSockID, threadid);
		return True;
	}
	return False;
}
/****************************************************************************
 Handle remote api calls delivered to a named pipe already opened.
 ****************************************************************************/

static int api_fd_reply(uint16 vuid,char *outbuf,
			uint16 *setup,char *data,char *params,
			int suwcnt,int tdscnt,int tpscnt,int mdrcnt,
			int mprcnt, int ProcSockID, int threadid)
{
	int reply = False;
	pipes_struct *p = NULL;
	int pnum;
	int subcommand;

//0508	DEBUG(5,("api_fd_reply\n"));

	/* First find out the name of this file. */
	if (suwcnt != 2) {
//0508		DEBUG(0,("Unexpected named pipe transaction.\n"));
		return(-1);
	}

	/* Get the file handle and hence the file name. */
	/* 
	 * NB. The setup array has already been transformed
	 * via SVAL and so is in gost byte order.
	 */
	pnum = ((int)setup[1]) & 0xFFFF;
	subcommand = ((int)setup[0]) & 0xFFFF;

	if(!(p = get_rpc_pipe(pnum, threadid))) {
 		return api_no_reply(outbuf, mdrcnt, ProcSockID, threadid);
	}

//0508	DEBUG(3,("Got API command 0x%x on pipe \"%s\" (pnum %x)", subcommand, p->name, pnum));

	/* record maximum data length that can be transmitted in an SMBtrans */
	p->max_trans_reply = mdrcnt;

//0508	DEBUG(10,("api_fd_reply: p:%p max_trans_reply: %d\n", p, p->max_trans_reply));

	switch (subcommand) {
	case 0x26:
		/* dce/rpc command */
		reply = write_to_pipe(p, data, tdscnt);
		if (reply)
			reply = api_rpc_trans_reply(outbuf, p, ProcSockID, threadid);
		break;
	case 0x53:
		/* Wait Named Pipe Handle state */
		reply = api_WNPHS(outbuf, p, params, tpscnt, ProcSockID, threadid);
		break;
	case 0x01:
		/* Set Named Pipe Handle state */
		reply = api_SNPHS(outbuf, p, params, tpscnt, ProcSockID, threadid);
		break;
	}

	if (!reply)
		return api_no_reply(outbuf, mdrcnt, ProcSockID, threadid);

	return -1;
}


static void fill_printjob_info(connection_struct *conn, 
				   int snum, int uLevel, struct pack_desc* desc,
			       print_queue_struct* queue, int n)
{
//0719  time_t t = queue->time;
  time_t t;
  BYTE SMBStatus;

  SMBStatus = PrnGetPrinterStatus(conn->smbprnportid);	   
  switch (SMBStatus){
    case PrnNoUsed:	
	break;

	case SMBUsed:
		if (printing_conn_tid[conn->smbprnportid] == conn->cnum){
//0719			queue->status = LPQ_PRINTING;
		}	                                                     
		else{ 
//0719			queue->status = LPQ_QUEUED;
		}	
    break;

	default:
//0719			queue->status = LPQ_QUEUED; 
	break;

  }							

  /* the client expects localtime */
//  t -= TimeDiff(t);

//0618  PACKI(desc,"W",printjob_encode(snum, queue->job)); /* uJobId */
  if (uLevel == 1) {
//0719    PACKS(desc,"B21",queue->user); /* szUserName */
    PACKS(desc,"B21",""); /* szUserName */
    PACKS(desc,"B","");		/* pad */
    PACKS(desc,"B16","");	/* szNotifyName */
    PACKS(desc,"B10","PM_Q_RAW"); /* szDataType */
    PACKS(desc,"z","");		/* pszParms */
    PACKI(desc,"W",n+1);		/* uPosition */
//0719    PACKI(desc,"W",queue->status); /* fsStatus */
//20071224    PACKI(desc,"W",""); /* fsStatus */
    PACKS(desc,"W",""); /* fsStatus */
    PACKS(desc,"z","");		/* pszStatus */
    PACKI(desc,"D",t); /* ulSubmitted */
//0719    PACKI(desc,"D",queue->size); /* ulSize */
//20071224    PACKI(desc,"D",""); /* ulSize */
    PACKS(desc,"D",""); /* ulSize */
//0719    PACKS(desc,"z",queue->file); /* pszComment */
    PACKS(desc,"z",""); /* pszComment */
  }
  if (uLevel == 2 || uLevel == 3) {
//0719    PACKI(desc,"W",queue->priority);		/* uPriority */
//20071224    PACKI(desc,"W","");		/* uPriority */
    PACKS(desc,"W","");		/* uPriority */
//0719    PACKS(desc,"z",queue->user); /* pszUserName */
    PACKS(desc,"z",""); /* pszUserName */
    PACKI(desc,"W",n+1);		/* uPosition */
//0719    PACKI(desc,"W",queue->status); /* fsStatus */
//20071224    PACKI(desc,"W",""); /* fsStatus */
    PACKS(desc,"W",""); /* fsStatus */
    PACKI(desc,"D",t); /* ulSubmitted */
//0719    PACKI(desc,"D",queue->size); /* ulSize */
//20071224    PACKI(desc,"D",""); /* ulSize */
    PACKS(desc,"D",""); /* ulSize */
    PACKS(desc,"z","Samba");	/* pszComment */
//0719    PACKS(desc,"z",queue->file); /* pszDocument */
    PACKS(desc,"z",""); /* pszDocument */
    if (uLevel == 3) {
      PACKS(desc,"z","");	/* pszNotifyName */
      PACKS(desc,"z","PM_Q_RAW"); /* pszDataType */
      PACKS(desc,"z","");	/* pszParms */
      PACKS(desc,"z","");	/* pszStatus */
//0402      PACKS(desc,"z",ServicePtrs[snum]->szService); /* pszQueue */
	  PACKS(desc,"z",Smbprinterserver[snum]); /* pszQueue */	
      PACKS(desc,"z","lpd");	/* pszQProcName */
      PACKS(desc,"z","");	/* pszQProcParms */
      PACKS(desc,"z","NULL"); /* pszDriverName */
      PackDriverData(desc);	/* pDriverData */
      PACKS(desc,"z","");	/* pszPrinterName */
    }
  }
}


static void fill_printq_info(connection_struct *conn, int snum, int uLevel,
 			     struct pack_desc* desc,
 			     int count, print_queue_struct* queue,
 			     print_status_struct* status)
{
  switch (uLevel) {
    case 1:
    case 2:    
//0402      PACKS(desc,"B13",ServicePtrs[snum]->szService);
			PACKS(desc,"B13",Smbprinterserver[snum]);
      break;
    case 3:
    case 4:
    case 5:
//0402      PACKS(desc,"z",Expand(conn,snum,ServicePtrs[snum]->szService));
			PACKS(desc,"z",Expand(snum,Smbprinterserver[snum]));
      break;
  }

  if (uLevel == 1 || uLevel == 2) {
    PACKS(desc,"B","");		/* alignment */
    PACKI(desc,"W",5);		/* priority */
    PACKI(desc,"W",0);		/* start time */
    PACKI(desc,"W",0);		/* until time */
    PACKS(desc,"z","");		/* pSepFile */
    PACKS(desc,"z","lpd");	/* pPrProc */
//0402    PACKS(desc,"z",ServicePtrs[snum]->szService); /* pDestinations */
	PACKS(desc,"z",Smbprinterserver[snum]); /* pDestinations */
    PACKS(desc,"z","");		/* pParms */
    if (snum < 0) {
      PACKS(desc,"z","UNKNOWN PRINTER");
      PACKI(desc,"W",LPSTAT_ERROR);
    }
    else if (!status || !status->message[0]) {
//0402      PACKS(desc,"z",Expand(conn,snum,ServicePtrs[snum]->comment));
	  PACKS(desc,"z",Expand(snum,""));
      PACKI(desc,"W",LPSTAT_OK); /* status */
    } else {
      PACKS(desc,"z",status->message);
      PACKI(desc,"W",status->status); /* status */
    }
    PACKI(desc,(uLevel == 1 ? "W" : "N"),count);
  }
  if (uLevel == 3 || uLevel == 4) {
    PACKI(desc,"W",5);		/* uPriority */
    PACKI(desc,"W",0);		/* uStarttime */
    PACKI(desc,"W",0);		/* uUntiltime */
    PACKI(desc,"W",5);		/* pad1 */
    PACKS(desc,"z","");		/* pszSepFile */
    PACKS(desc,"z","WinPrint");	/* pszPrProc */
//Jesse    PACKS(desc,"z","");		/* pszParms */
		PACKS(desc,"z",NULL);		/* pszParms */
//Jesse
		PACKS(desc,"z",NULL);		/* pszComment - don't ask.... JRA */
		/* "don't ask" that it's done this way to fix corrupted 
		   Win9X/ME printer comments. */
		   
//Jesse    if (!status || !status->message[0]) {
		if (!status ) {
//0621	  PACKS(desc,"z",Expand(conn,snum,ServicePtrs[snum]->comment)); /* pszComment */
//Jesse	  PACKS(desc,"z",Expand(snum,"")); /* pszComment */
      PACKI(desc,"W",LPSTAT_OK); /* fsStatus */
    } else {
//Jesse      PACKS(desc,"z",status->message); /* pszComment */
      PACKI(desc,"W",status->status); /* fsStatus */
    }
    PACKI(desc,(uLevel == 3 ? "W" : "N"),count);	/* cJobs */
//0402    PACKS(desc,"z",ServicePtrs[snum]->szService); /* pszPrinters */
	PACKS(desc,"z",Smbprinterserver[snum]); /* pszPrinters */
//0402    PACKS(desc,"z",ServicePtrs[snum]->szPrinterDriver);		/* pszDriverName */

//Jesse  	PACKS(desc,"z","NULL");		/* pszDriverName */
		PACKS(desc,"z","");		/* pszDriverName */
    PackDriverData(desc);	/* pDriverData */
  }
  if (uLevel == 2 || uLevel == 4) {
    int i;
    for (i=0;i<count;i++) {
      fill_printjob_info(conn, snum,uLevel == 2 ? 1 : 2,desc,&queue[i],i);
	}
  }

  if (uLevel==52) {
#if 0 //0720
    int i,ok=0;
    pstring tok,driver,datafile,langmon,helpfile,datatype;
    char *p,*q;
    FILE *f;
    pstring fname;

//0402    pstrcpy(fname,(char **)&Globals.szDriverFile);
//    f=sys_fopen(fname,"r");
//    if (!f) {
//      DEBUG(3,("fill_printq_info: Can't open %s - %s\n",fname,strerror(errno)));
//      desc->errcode=NERR_notsupported;
//      return;
//    }

    if((p=(char *)malloc(8192*sizeof(char))) == NULL) {
//      DEBUG(0,("fill_printq_info: malloc fail !\n"));
      desc->errcode=NERR_notsupported;
//0618      fclose(f);
      return;
    }

    memset(p, '\0',8192*sizeof(char));
    q=p;

    /* lookup the long printer driver name in the file description */
    while (f && !feof(f) && !ok)
    {
      p = q;			/* reset string pointer */
      fgets(p,8191,f);
      p[strlen(p)-1]='\0';
//0301      if (/*0301next_token(&p,tok,":",sizeof(tok)) &&*/
//0402       if((strlen(ServicePtrs[snum]->szPrinterDriver) == strlen(tok)) &&
//        (!strncmp(tok,ServicePtrs[snum]->szPrinterDriver,strlen(ServicePtrs[snum]->szPrinterDriver))))
//	ok=1;
    }
//0618    fclose(f);

    /* driver file name */
//0301    if (ok && !next_token(&p,driver,":",sizeof(driver))) ok = 0;
//0301    if (ok && !next_token(&p,driver,":",sizeof(driver))) ok = 0;
    /* data file name */
//0301    if (ok && !next_token(&p,datafile,":",sizeof(datafile))) ok = 0;
      /*
       * for the next tokens - which may be empty - I have to check for empty
       * tokens first because the next_token function will skip all empty
       * token fields 
       */
    if (ok) {
      /* help file */
      if (*p == ':') {
	  *helpfile = '\0';
	  p++;
    } 
//0301	else if (!next_token(&p,helpfile,":",sizeof(helpfile))) ok = 0;    
    }

    if (ok) {
      /* language monitor */
      if (*p == ':') {
	  *langmon = '\0';
	  p++;
    } 
//0301  else if (!next_token(&p,langmon,":",sizeof(langmon))) ok = 0;    
    }

    /* default data type */
//    if (ok && !next_token(&p,datatype,":",sizeof(datatype))) ok = 0;

    if (ok) {
      PACKI(desc,"W",0x0400);                    /* don't know */
//0402      PACKS(desc,"z",ServicePtrs[snum]->szPrinterDriver);    /* long printer name */
      PACKS(desc,"z",driver);                    /* Driverfile Name */
      PACKS(desc,"z",datafile);                  /* Datafile name */
      PACKS(desc,"z",langmon);			 /* language monitor */
//0402      PACKS(desc,"z",ServicePtrs[snum]->szPrinterDriverLocation);   /* share to retrieve files */
      PACKS(desc,"z",datatype);			 /* default data type */
      PACKS(desc,"z",helpfile);                  /* helpfile name */
      PACKS(desc,"z",driver);                    /* driver name */
//      DEBUG(3,("Driver:%s:\n",driver));
//      DEBUG(3,("Data File:%s:\n",datafile));
//      DEBUG(3,("Language Monitor:%s:\n",langmon));
//      DEBUG(3,("Data Type:%s:\n",datatype));
//      DEBUG(3,("Help File:%s:\n",helpfile));
      PACKI(desc,"N",count);                     /* number of files to copy */
      for (i=0;i<count;i++)
      {
	/* no need to check return value here - it was already tested in
	 * get_printerdrivernumber
	 */
//0301        next_token(&p,tok,",",sizeof(tok));
        PACKS(desc,"z",tok);                        /* driver files to copy */
//        DEBUG(3,("file:%s:\n",tok));
      }

//      DEBUG(3,("fill_printq_info on <%s> gave %d entries\n",
//	    SERVICE(snum),count));
    } else {
//      DEBUG(3,("fill_printq_info: Can't supply driver files\n"));
      desc->errcode=NERR_notsupported;
    }
    free(q);
#endif
  }

}




static int api_DosPrintQGetInfo(connection_struct *conn, uint16 vuid, 
				 char *param,char *data, int mdrcnt,int mprcnt,
				 char **rdata,char **rparam,
				 int *rdata_len,int *rparam_len, int threadid)
{
  char *str1 = param+2;
  char *str2 = str1+strlen(str1)+1;
  char *p = str2+strlen(str2)+1;
  char *QueueName = p;
  int uLevel;
  int count=0;
  int snum= -1;
  int i;
  char* str3;
  struct pack_desc desc;
  print_queue_struct *queue=NULL;
  print_status_struct status;
  
  memset((char *)&status,'\0',sizeof(status));
  memset((char *)&desc,'\0',sizeof(desc));
 
  p += strlen(p)+1;
  str3 = p + 4;
  uLevel = SVAL(p,0);
 
  /* remove any trailing username */
  if ((p =   (QueueName,'%'))) *p = 0;
 
  /* check it's a supported varient */
  if (!prefix_ok(str1,"zWrLh")) return False;
  if (!check_printq_info(&desc,uLevel,str2,str3)) {
    /*
     * Patch from Scott Moomaw <scott@bridgewater.edu>
     * to return the 'invalid info level' error if an
     * unknown level was requested.
     */
    *rdata_len = 0;
    *rparam_len = 6;
    *rparam = REALLOC(*rparam,*rparam_len);
    SSVALS(*rparam,0,ERROR_INVALID_LEVEL);
    SSVAL(*rparam,2,0);
    SSVAL(*rparam,4,0);
    return(True);
  }
 
//0605  snum = lp_servicenumber(QueueName);

/* replace lp_servicenumber(QueueName) to get snum 6/29/2001 by ron */ 	  

	for (i=0; i< NUM_OF_PRN_PORT; i++){
		if(!strcmp(strupr(QueueName),strupr(Smbprinterserver[i]))){			
		  snum = i;
		  break;
		}  
	}
  
#if 0 //0605
  if (snum < 0 && pcap_printername_ok(QueueName,NULL)) {
    int pnum = lp_servicenumber(PRINTERS_NAME);
    if (pnum >= 0) {
      lp_add_printer(QueueName,pnum);
      snum = lp_servicenumber(QueueName);
    }
  }
#endif  

  if (IS_PRINT(conn) && snum < 0) 
  	  return(False);

  if (uLevel==52) {
//0605	  count = get_printerdrivernumber(snum);
//0605	  DEBUG(3,("api_DosPrintQGetInfo: Driver files count: %d\n",count));
  } else {
//0618	  count = get_printqueue(snum,&queue,&status);  //not necessary
  }
//  if (mdrcnt > 0) 
//  	*rdata = REALLOC(*rdata,mdrcnt);
// REALLOC MACRO define size as max(size , 4*1024), so I can not support such large memory.
  if (mdrcnt > 0 ){
  if (!*rdata)
    *rdata = (void *)mallocw(mdrcnt);
  else
    *rdata = (void *)realloc(*rdata,mdrcnt); 	  	
  } 

  desc.base = *rdata;
  desc.buflen = mdrcnt;

  if (init_package(&desc,1,count)) {
	  desc.subcount = count;
	  fill_printq_info(conn, snum,uLevel,&desc,count,queue,&status);
  } else if(uLevel == 0) {
//Jesse
#if 0 //samba-2.2.0
	/*
	 * This is a *disgusting* hack.
	 * This is *so* bad that even I'm embarrassed (and I
	 * have no shame). Here's the deal :
 	 * Until we get the correct SPOOLSS code into smbd
 	 * then when we're running with NT SMB support then
 	 * NT makes this call with a level of zero, and then
 	 * immediately follows it with an open request to
 	 * the \\SRVSVC pipe. If we allow that open to
 	 * succeed then NT barfs when it cannot open the
 	 * \\SPOOLSS pipe immediately after and continually
 	 * whines saying "Printer name is invalid" forever
 	 * after. If we cause *JUST THIS NEXT OPEN* of \\SRVSVC
 	 * to fail, then NT downgrades to using the downlevel code
 	 * and everything works as well as before. I hate
 	 * myself for adding this code.... JRA.
 	 */

	fail_next_srvsvc_open(threadid);
#endif
  }

  *rdata_len = desc.usedlen;
  
  *rparam_len = 6;
  *rparam = REALLOC(*rparam,*rparam_len);
  SSVALS(*rparam,0,desc.errcode);
  SSVAL(*rparam,2,0);
  SSVAL(*rparam,4,desc.neededlen);
  
  if (queue) free(queue);
  return(True);
}

static int fill_share_info(int snum, int uLevel,
 			   char** buf, int* buflen,
 			   char** stringbuf, int* stringspace, char* baseaddr)
{
  int struct_len;
  char* p;
  char* p2;
  int l2;
  int len;
 
  switch( uLevel ) {
  case 0: struct_len = 13; break;
  case 1: struct_len = 20; break;
  case 2: struct_len = 40; break;
  case 91: struct_len = 68; break;
  default: return -1;
  }
  
 
  if (!buf)
    {
      len = 0;
      if (uLevel > 0) len += StrlenExpanded(snum,"Public Stuff");
      if (buflen) *buflen = struct_len;
      if (stringspace) *stringspace = len;
      return struct_len + len;
    }
  
  len = struct_len;
  p = *buf;
  if ((*buflen) < struct_len) return -1;
  if (stringbuf)
    {
      p2 = *stringbuf;
      l2 = *stringspace;
    }
  else
    {
      p2 = p + struct_len;
      l2 = (*buflen) - struct_len;
    }
  if (!baseaddr) baseaddr = p;
  
  strncpy(p,Smbprinterserver[snum],13);  
  if (uLevel > 0)
    {
      int type;
      CVAL(p,13) = 0;
	  type = STYPE_PRINTQ;	//0403      
      SSVAL(p,14,type);		/* device type */ 
      SIVAL(p,16,PTR_DIFF(p2,baseaddr));
	  len += CopyExpanded(snum,&p2,"Printer",&l2); //0404

    }
  
  if (uLevel > 1)
    {
      SSVAL(p,20,ACCESS_READ|ACCESS_WRITE|ACCESS_CREATE); /* permissions */
      SSVALS(p,22,-1);		/* max uses */
      SSVAL(p,24,1); /* current uses */
      SIVAL(p,26,PTR_DIFF(p2,baseaddr)); /* local pathname */
      memset(p+30,0,SHPWLEN+2); /* passwd (reserved), pad field */
    }
  
  if (uLevel > 2)
    {
      memset(p+40,0,SHPWLEN+2);
      SSVAL(p,50,0);
      SIVAL(p,52,0);
      SSVAL(p,56,0);
      SSVAL(p,58,0);
      SIVAL(p,60,0);
      SSVAL(p,64,0);
      SSVAL(p,66,0);
    }
       
  if (stringbuf)
    {
      (*buf) = p + struct_len;
      (*buflen) -= struct_len;
      (*stringbuf) = p2;
      (*stringspace) = l2;
    }
  else
    {
      (*buf) = p2;
      (*buflen) -= len;
    }
  return len;
}

/****************************************************************************
  view list of shares available
  ****************************************************************************/

static int api_RNetShareEnum(connection_struct *conn, uint16 vuid, 
				  char *param,char *data, int mdrcnt,int mprcnt,
  			      char **rdata,char **rparam,
  			      int *rdata_len,int *rparam_len, int threadid)
{
  char *str1 = param+2;
  char *str2 = str1+strlen(str1)+1;
  char *p = str2+strlen(str2)+1;
  int uLevel = SVAL(p,0);
  int buf_len = SVAL(p,2);
  char *p2;
  int total=0,counted=0;
  int missed = False;
  int i;
  int data_len, fixed_len, string_len;
  int f_len = 0, s_len = 0;
 
  if (!prefix_ok(str1,"WrLeh")) return False;
  
  data_len = fixed_len = string_len = 0;
  for (i=0; i< NUM_OF_PRN_PORT; i++)
    {
      total++;
      data_len += fill_share_info(i,uLevel,0,&f_len,0,&s_len,0);
      if (data_len <= buf_len)
      {
        counted++;
        fixed_len += f_len;
        string_len += s_len;
      }
      else
        missed = True;
    }
  *rdata_len = fixed_len + string_len;
  *rdata = REALLOC(*rdata,*rdata_len);
  memset(*rdata,0,*rdata_len);
  
  p2 = (*rdata) + fixed_len;	/* auxillery data (strings) will go here */ 
  p = *rdata;
  f_len = fixed_len;
  s_len = string_len;
  for (i = 0; i < NUM_OF_PRN_PORT;i++)
      if (fill_share_info(i,uLevel,&p,&f_len,&p2,&s_len,*rdata) < 0)
 	break;
  
  *rparam_len = 8;
  *rparam = REALLOC(*rparam,*rparam_len);
  SSVAL(*rparam,0,missed ? ERRmoredata : NERR_Success);
  SSVAL(*rparam,2,0);
  SSVAL(*rparam,4,counted);
  SSVAL(*rparam,6,total);
  
  return(True);
}

/****************************************************************************
  delete a print job
  Form: <W> <> 
  ****************************************************************************/

static int api_RDosPrintJobDel(connection_struct *conn, uint16 vuid, 
				char *param,char *data, int mdrcnt,int mprcnt,
				char **rdata,char **rparam,
				int *rdata_len,int *rparam_len, int threadid)
{
  *rparam_len = 4;
  *rparam = REALLOC(*rparam,*rparam_len);

  *rdata_len = 0;

  SSVAL(*rparam,0,NERR_Success);

  SSVAL(*rparam,2,0);		/* converter word */

  return(True);
}

/****************************************************************************
  set the property of a print job (undocumented?)
  ? function = 0xb -> set name of print job
  ? function = 0x6 -> move print job up/down
  Form: <WWsTP> <WWzWWDDzzzzzzzzzzlz> 
  or   <WWsTP> <WB21BB16B10zWWzDDz> 
****************************************************************************/
static int check_printjob_info(struct pack_desc* desc,
			       int uLevel, char* id)
{
  desc->subformat = NULL;
  switch( uLevel ) {
  case 0: desc->format = "W"; break;
  case 1: desc->format = "WB21BB16B10zWWzDDz"; break;
  case 2: desc->format = "WWzWWDDzz"; break;
  case 3: desc->format = "WWzWWDDzzzzzzzzzzlz"; break;
  default: return False;
  }
  if (strcmp(desc->format,id) != 0) return False;
  return True;
}



static int api_PrintJobInfo(connection_struct *conn,
				 uint16 vuid,char *param,char *data,
  			     int mdrcnt,int mprcnt,
  			     char **rdata,char **rparam,
  			     int *rdata_len,int *rparam_len, int threadid)
{
	struct pack_desc desc;
	char *str1 = param+2;
	char *str2 = str1+strlen(str1)+1;
	char *p = str2+strlen(str2)+1;
	int jobid, snum;
	int uLevel = SVAL(p,2);
	int function = SVAL(p,4);	/* what is this ?? */
	int i;
	char *s = data;
//0602	files_struct *fsp;
   
	*rparam_len = 4;
	*rparam = REALLOC(*rparam,*rparam_len);
  
	*rdata_len = 0;
	
	/* check it's a supported varient */
	if ((strcmp(str1,"WWsTP")) || 
	    (!check_printjob_info(&desc,uLevel,str2)))
		return(False);
  
	switch (function) {
	case 0xb:   /* change print job name, data gives the name */
		/* jobid, snum should be zero */
		if (isalpha((uint8)*s)) {
			pstring name;
			int l = 0;
			while (l<64 && *s) {
				if (issafe(*s)) name[l++] = *s;
				name[l++] = *s;				
				s++;
			}      
			name[l] = 0;
	
		}
		desc.errcode=NERR_Success;
		break;

	default:			/* not implemented */
		return False;
	}
 
	SSVALS(*rparam,0,desc.errcode);
	SSVAL(*rparam,2,0);		/* converter word */
	
	return(True);
}

/****************************************************************************
  get info about the server
  ****************************************************************************/
static int api_RNetServerGetInfo(connection_struct *conn, uint16 vuid, 
				  char *param,char *data, int mdrcnt,int mprcnt,
				  char **rdata,char **rparam,
				  int *rdata_len,int *rparam_len, int threadid)
{
  char *str1 = param+2;
  char *str2 = skip_string(str1,1);
  char *p = skip_string(str2,1);
  int uLevel = SVAL(p,0);
  char *p2;
  int struct_len;

//  DEBUG(4,("NetServerGetInfo level %d\n",uLevel));

  /* check it's a supported varient */
  if (!prefix_ok(str1,"WrLh")) return False;
  switch( uLevel ) {
  case 0:
    if (strcmp(str2,"B16") != 0) return False;
    struct_len = 16;
    break;
  case 1:
    if (strcmp(str2,"B16BBDz") != 0) return False;
    struct_len = 26;
    break;
  case 2:
    if (strcmp(str2,"B16BBDzDDDWWzWWWWWWWBB21zWWWWWWWWWWWWWWWWWWWWWWz")
	!= 0) return False;
    struct_len = 134;
    break;
  case 3:
    if (strcmp(str2,"B16BBDzDDDWWzWWWWWWWBB21zWWWWWWWWWWWWWWWWWWWWWWzDWz")
	!= 0) return False;
    struct_len = 144;
    break;
  case 20:
    if (strcmp(str2,"DN") != 0) return False;
    struct_len = 6;
    break;
  case 50:
    if (strcmp(str2,"B16BBDzWWzzz") != 0) return False;
    struct_len = 42;
    break;
  default: return False;
  }

  *rdata_len = mdrcnt;
  *rdata = REALLOC(*rdata,*rdata_len);

  p = *rdata;
  p2 = p + struct_len;
  if (uLevel != 20) {
    strncpy(p,my_netbios_names[0],16);
    strupr(p);
  }
  p += 16;
  if (uLevel > 0)
    {
//      struct srv_info_struct *servers=NULL;
//      int i,count;
//      pstring comment;
//      uint32 servertype= lp_default_server_announce();

 //     pstrcpy(comment,string_truncate(lp_serverstring(), MAX_SERVER_STRING_LENGTH));

 //     if ((count=get_server_info(SV_TYPE_ALL,&servers,global_myworkgroup))>0) {
 //	for (i=0;i<count;i++)
 //	  if (strequal(servers[i].name,local_machine))
 //     {
 //	    servertype = servers[i].type;
 //	    pstrcpy(comment,servers[i].comment);            
 //	  }
 //     }
 //     if (servers) free(servers);

      SCVAL(p,0,0x04);
      SCVAL(p,1,0x02);
      SIVAL(p,2,my_netbios_names[0]);

      if (mdrcnt == struct_len) {
	SIVAL(p,6,0);
      } else {
	SIVAL(p,6,PTR_DIFF(p2,*rdata));
//	standard_sub(conn,comment);
//0628	StrnCpy(p2,comment,MAX(mdrcnt - struct_len,0));
	strncpy(p2,"PrinterServer",MAX(mdrcnt - struct_len,0)); //Server Comment
	p2 = skip_string(p2,1);
      }
    }
  if (uLevel > 1)
    {
      return False;             /* not yet implemented */
    }

  *rdata_len = PTR_DIFF(p2,*rdata);

  *rparam_len = 6;
  *rparam = REALLOC(*rparam,*rparam_len);
  SSVAL(*rparam,0,NERR_Success);
  SSVAL(*rparam,2,0);           /* converter word */
  SSVAL(*rparam,4,*rdata_len);

  return(True);
}


/****************************************************************************
  get info about the server
  ****************************************************************************/

static int api_NetWkstaGetInfo(connection_struct *conn, uint16 vuid, 
				char *param,char *data, int mdrcnt,int mprcnt,
				char **rdata,char **rparam,
				int *rdata_len,int *rparam_len, int threadid)
{
  char *str1 = param+2;
  char *str2 = str1+strlen(str1)+1;
  char *p = str2+strlen(str2)+1;
  char *p2;
  extern pstring sesssetup_user;
  int level = SVAL(p,0);

  *rparam_len = 6;
  *rparam = REALLOC(*rparam,*rparam_len);

  if (!(level==10 && !(strcmp(str1,"WrLh")) && !(strcmp(str2,"zzzBBzz"))))
    return(False);

  *rdata_len = mdrcnt + 1024;
  *rdata = REALLOC(*rdata,*rdata_len);

  SSVAL(*rparam,0,NERR_Success);
  SSVAL(*rparam,2,0);		/* converter word */

  p = *rdata;
  p2 = p + 22;


  SIVAL(p,0,PTR_DIFF(p2,*rdata)); /* host name */
  pstrcpy(p2,my_netbios_names[0]);
  strupr(p2);
  p2 += strlen(p2)+1;
  p += 4;

  SIVAL(p,0,PTR_DIFF(p2,*rdata));
  pstrcpy(p2,sesssetup_user);
  p2 += strlen(p2)+1;
  p += 4;

  SIVAL(p,0,PTR_DIFF(p2,*rdata)); /* login domain */
//  pstrcpy(p2,global_myworkgroup);
  pstrcpy(p2,_MyWorkgroup);
  strupr(p2);
  p2 += strlen(p2);
  p += 4;

  SCVAL(p,0,0x04); /* system version - e.g 4 in 4.1 */
  SCVAL(p,1,0x02); /* system version - e.g .1 in 4.1 */

  p += 2;

  p += 4;

  SIVAL(p,0,PTR_DIFF(p2,*rdata)); /* don't know */
  pstrcpy(p2,"");
  p2 += strlen(p2)+1;
  p += 4;

  *rdata_len = PTR_DIFF(p2,*rdata);

  SSVAL(*rparam,4,*rdata_len);

  return(True);
}
//WinCE use ...6/4/2002 Add by Ron
static int api_WPrintJobEnumerate(connection_struct *conn,uint16 vuid, char *param,char *data,
				   int mdrcnt,int mprcnt,
				   char **rdata,char **rparam,
				   int *rdata_len,int *rparam_len, int threadid)
{
  char *str1 = param+2;
  char *str2 = skip_string(str1,1);
  char *p = skip_string(str2,1);
  char* name = p;
  int uLevel;
  int count=0;
  int i, succnt=0;
  int snum;
  struct pack_desc desc;
  print_queue_struct *queue=NULL;
  print_status_struct status;
  char *QueueName = p;

  memset((char *)&desc,'\0',sizeof(desc));
  memset((char *)&status,'\0',sizeof(status));

  p = skip_string(p,1);
  uLevel = SVAL(p,0);

//  DEBUG(3,("WPrintJobEnumerate uLevel=%d name=%s\n",uLevel,name));

  /* check it's a supported varient */
  if (strcmp(str1,"zWrLeh") != 0) return False;
  if (uLevel > 2) return False;	/* defined only for uLevel 0,1,2 */
  if (!check_printjob_info(&desc,uLevel,str2)) return False;

/* replace lp_servicenumber(QueueName) to get snum 6/29/2001 by ron */ 	  

	for (i=0; i< NUM_OF_PRN_PORT; i++){
		if(!strcmp(strupr(name),strupr(Smbprinterserver[i]))){			
		  snum = i;
		  break;
		}  
	}
  
#if 0 //0605
  if (snum < 0 && pcap_printername_ok(name,NULL)) {
    int pnum = lp_servicenumber(PRINTERS_NAME);
    if (pnum >= 0) {
      lp_add_printer(name,pnum);
      snum = lp_servicenumber(name);
    }
  }
#endif  
  if (IS_PRINT(conn) && snum < 0) 
  	  return(False);
//  if (snum < 0 || !VALID_SNUM(snum)) return(False);

//  count = print_queue_status(snum,&queue,&status);
  if (mdrcnt > 0) *rdata = REALLOC(*rdata,mdrcnt);
  desc.base = *rdata;
  desc.buflen = mdrcnt;

  if (init_package(&desc,count,0)) {
    succnt = 0;
    for (i = 0; i < count; i++) {
      fill_printjob_info(conn,snum,uLevel,&desc,&queue[i],i);
      if (desc.errcode == NERR_Success) succnt = i+1;
    }
  }

  *rdata_len = desc.usedlen;

  *rparam_len = 8;
  *rparam = REALLOC(*rparam,*rparam_len);
  SSVALS(*rparam,0,desc.errcode);
  SSVAL(*rparam,2,0);
  SSVAL(*rparam,4,succnt);
  SSVAL(*rparam,6,count);

  if (queue);
  	free(queue);

//  DEBUG(4,("WPrintJobEnumerate: errorcode %d\n",desc.errcode));
  return(True);
}

/****************************************************************************
 The buffer was too small
 ****************************************************************************/

static int api_TooSmall(uint16 vuid, char *param,char *data,
			 int mdrcnt,int mprcnt,
			 char **rdata,char **rparam,
			 int *rdata_len,int *rparam_len)
{
  *rparam_len = MIN(*rparam_len,mprcnt);
  *rparam = REALLOC(*rparam,*rparam_len);

  *rdata_len = 0;

  SSVAL(*rparam,0,NERR_BufTooSmall);

  return(True);
}


/****************************************************************************
 The request is not supported
 ****************************************************************************/

static int api_Unsupported(connection_struct *conn, uint16 vuid, 
				char *param,char *data, int mdrcnt,int mprcnt,
			    char **rdata,char **rparam,
			    int *rdata_len,int *rparam_len, int threadid)
{
  *rparam_len = 4;
  *rparam = REALLOC(*rparam,*rparam_len);

  *rdata_len = 0;

  SSVAL(*rparam,0,NERR_notsupported);
  SSVAL(*rparam,2,0);		/* converter word */

  return(True);
}




struct
{
  char *name;
  int id;
  int (*fn)(connection_struct *, uint16,char *,char *,
	     int,int,char **,char **,int *,int *, int);
  int flags;
} api_commands[] = {
  {"RNetShareEnum",	0,	api_RNetShareEnum,0},
  {"RNetServerGetInfo", 13, api_RNetServerGetInfo,0},
  {"NetWkstaGetInfo",	63,	api_NetWkstaGetInfo,0},
  {"DosPrintQGetInfo",	70,	api_DosPrintQGetInfo,0},
  {"WPrintJobEnumerate",76,	api_WPrintJobEnumerate,0},  //WinCE use ...6/4/2002 by Ron
  {"RDosPrintJobDel",	81,	api_RDosPrintJobDel,0},
  {"RDosPrintJobPause",	82,	api_RDosPrintJobDel,0},
  {"RDosPrintJobResume",83,	api_RDosPrintJobDel,0},
  {"PrintJobInfo",	147,	api_PrintJobInfo,0},
  {NULL,		-1,	api_Unsupported,0}};


/****************************************************************************
 Handle remote api calls
 ****************************************************************************/

static int api_reply(connection_struct *conn, uint16 vuid,char *outbuf,char *data,char *params,
		     int tdscnt,int tpscnt,int mdrcnt,int mprcnt, int ProcSockID, int threadid)
{
  int api_command;
  char *rdata = NULL;
  char *rparam = NULL;
  int rdata_len = 0;
  int rparam_len = 0;
  int reply=False;
  int i;
  int find_trans = 0;

  if (!params) {
	  return 0;
  }

  api_command = SVAL(params,0);

  for (i=0;api_commands[i].name;i++) {
    if (api_commands[i].id == api_command && api_commands[i].fn) {
		find_trans = 1;
        break;
    }
  }
/* //2/20/2002
  rdata = (char *)malloc(1024);
  if (rdata)
    memset(rdata,'\0',1024);

  rparam = (char *)malloc(1024);
  if (rparam)
    memset(rparam,'\0',1024);
*/
  rdata = (char *)malloc(256);
  if (rdata)
    memset(rdata,'\0',256);

  rparam = (char *)malloc(256);
  if (rparam)
    memset(rparam,'\0',256);

  if(!rdata || !rparam) {
    return -1;
  }

//  if (!find_trans){
//    api_Unsupported(conn, vuid,params,data,mdrcnt,mprcnt,
//		    &rdata,&rparam,&rdata_len,&rparam_len, threadid); //for test 7/13/2001
//  }

  if ((uint16)mdrcnt >256)	// Workaround for 16bit SoC - Kevin [03/30/2005]
     mdrcnt = 256;  //add by Ron 2/20/2002. Since I don't need buffer more than 256 bytes.		
  reply = api_commands[i].fn(conn, vuid,params,data,mdrcnt,mprcnt,
			     &rdata,&rparam,&rdata_len,&rparam_len, threadid);


  if (rdata_len > mdrcnt ||
      rparam_len > mprcnt) {
      reply = api_TooSmall(vuid,params,data,mdrcnt,mprcnt,
			   &rdata,&rparam,&rdata_len,&rparam_len);
  }

  /* if we get False back then it's actually unsupported */
  if (!reply)
    api_Unsupported(conn, vuid,params,data,mdrcnt,mprcnt,
		    &rdata,&rparam,&rdata_len,&rparam_len, threadid);

  send_trans_reply(outbuf, rparam, rparam_len, rdata, rdata_len, 
  						False, ProcSockID, threadid);

  if (rdata)
	free(rdata);
  if (rparam)
	free(rparam);
  
  return -1;
}

/****************************************************************************
  handle named pipe commands
  ****************************************************************************/
static int named_pipe(connection_struct *conn, uint16 vuid, char *outbuf,char *name,
		      uint16 *setup,char *data,char *params,
		      int suwcnt,int tdscnt,int tpscnt,
		      int msrcnt,int mdrcnt,int mprcnt, int ProcSockID, int threadid)
{
	if (!strcmp(name,"LANMAN"))
		return api_reply(conn, vuid,outbuf,data,params,tdscnt,tpscnt,mdrcnt
						,mprcnt, ProcSockID, threadid);

	if (!strcmp(name,"WKSSVC") ||
	    !strcmp(name,"SRVSVC") ||
	    !strcmp(name,"WINREG") ||
	    !strcmp(name,"SAMR") ||
	    !strcmp(name,"LSARPC"))
	{
		return api_fd_reply(vuid,outbuf,setup,data,params,suwcnt,tdscnt,tpscnt,
								mdrcnt,mprcnt, ProcSockID, threadid);
	}

	if (strlen(name) < 1)
		return api_fd_reply(vuid,outbuf,setup,data,params,suwcnt,tdscnt,tpscnt,
								mdrcnt,mprcnt, ProcSockID, threadid);

	return 0;
}

 
/****************************************************************************
 Reply to a SMBtrans.
 ****************************************************************************/

int reply_trans(connection_struct *conn, char *inbuf,char *outbuf, int size, 
				int bufsize, int ProcSockID, int threadid)
{
	fstring name;
	int name_offset = 0;
	char *data=NULL,*params=NULL;
	uint16 *setup=NULL;
	int outsize = 0;
	uint16 vuid = SVAL(inbuf,smb_uid);
	int tpscnt = SVAL(inbuf,smb_vwv0);
	int tdscnt = SVAL(inbuf,smb_vwv1);
	int mprcnt = SVAL(inbuf,smb_vwv2);
	int mdrcnt = SVAL(inbuf,smb_vwv3);
	int msrcnt = CVAL(inbuf,smb_vwv4);
	int close_on_completion = BITSETW(inbuf+smb_vwv5,0);
	int one_way = BITSETW(inbuf+smb_vwv5,1);
	int pscnt = SVAL(inbuf,smb_vwv9);
	int psoff = SVAL(inbuf,smb_vwv10);
	int dscnt = SVAL(inbuf,smb_vwv11);
	int dsoff = SVAL(inbuf,smb_vwv12);
	int suwcnt = CVAL(inbuf,smb_vwv13);
	int i;
	memset(name, '\0',sizeof(name));
	fstrcpy(name,smb_buf(inbuf));
  
	if (tdscnt)  {
		if((data = (char *)malloc(tdscnt)) == NULL) {
			return(ERROR(ERRDOS,ERRnomem));
		} 
		memcpy(data,smb_base(inbuf)+dsoff,dscnt);		// data = DCE_RPC 
	}
	if (tpscnt) {
		if((params = (char *)malloc(tpscnt)) == NULL) {
			if (data)
				free(data); //add by ron 7/18/2001
			return(ERROR(ERRDOS,ERRnomem));
		} 
		memcpy(params,smb_base(inbuf)+psoff,pscnt);
	}

	if (suwcnt) {
		int i;
		if((setup = (uint16 *)malloc(suwcnt*sizeof(uint16))) == NULL) {
			if (data)
				free(data);   //add by ron 7/18/2001
			if (params)
				free(params); //add by ron 7/18/2001
		    return(ERROR(ERRDOS,ERRnomem));
	} 
		for (i=0;i<suwcnt;i++)
			setup[i] = SVAL(inbuf,smb_vwv14+i*SIZEOFWORD);
	}

	if (pscnt < tpscnt || dscnt < tdscnt) {
		/* We need to send an interim response then receive the rest
		   of the parameter/data bytes */
		outsize = set_message(outbuf,0,0,True);
		send_smb(ProcSockID,outbuf);
	}

	/* receive the rest of the trans packet */
	while (pscnt < tpscnt || dscnt < tdscnt) {
		int ret;
		int pcnt,poff,dcnt,doff,pdisp,ddisp;
      
		ret = receive_next_smb(inbuf,bufsize,SMB_SECONDARY_WAIT, ProcSockID, threadid);

		if ((ret && (CVAL(inbuf, smb_com) != SMBtranss)) || !ret) {
			if (params)
				free(params);
			if (data)
				free(data);
			if (setup)
				free(setup);

			return(ERROR(ERRSRV,ERRerror));
		}
      
		tpscnt = SVAL(inbuf,smb_vwv0);
		tdscnt = SVAL(inbuf,smb_vwv1);

		pcnt = SVAL(inbuf,smb_vwv2);
		poff = SVAL(inbuf,smb_vwv3);
		pdisp = SVAL(inbuf,smb_vwv4);
		
		dcnt = SVAL(inbuf,smb_vwv5);
		doff = SVAL(inbuf,smb_vwv6);
		ddisp = SVAL(inbuf,smb_vwv7);
		
		pscnt += pcnt;
		dscnt += dcnt;
		
		if (pcnt)
			memcpy(params+pdisp,smb_base(inbuf)+poff,pcnt);
		if (dcnt)
			memcpy(data+ddisp,smb_base(inbuf)+doff,dcnt);      
	}
	/*
	 * WinCE wierdness....
	 */
#if 0 //Ron 9/2/2003 ... We can not support the machine_name included '\'.
	if (name[0] == '\\'  &&
			(name[strlen(my_netbios_names[0])+1] == '\\'))
		name_offset = strlen(my_netbios_names[0])+1;
#endif
	if (strncmp(&name[name_offset],"\\PIPE\\",strlen("\\PIPE\\")) == 0) {
		outsize = named_pipe(conn, vuid,outbuf,
				     name+name_offset+strlen("\\PIPE\\"),setup,data,params,
				     suwcnt,tdscnt,tpscnt,msrcnt,mdrcnt,mprcnt, ProcSockID, threadid);
	} else {
		outsize = 0;
	}

	
	if (data)
		free(data);
	if (params)
		free(params);
	if (setup)
		free(setup);

	if (close_on_completion){
 		for ( i =0; i< 3 ; i++){
 			if (printing_conn_tid[i] == conn->cnum){
        		if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed )
 					PrnSetNoUse(conn->smbprnportid);
 			}	
 		}
		close_cnum(conn, threadid);
	}	

	if (one_way)
		return(-1);

	if (outsize == 0)
		return(ERROR(ERRSRV,ERRnosupport));
	
	return(outsize);
}


