/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Pipe SMB reply routines
   Copyright (C) Andrew Tridgell 1992-1998
   Copyright (C) Luke Kenneth Casson Leighton 1996-1998
   Copyright (C) Paul Ashton  1997-1998.
   
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
   This file handles reply_ calls on named pipes that the server
   makes to handle specific protocols
*/


#include <cyg/kernel/kapi.h>
#include "network.h"
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"

//for pipes_struct must include
#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"


//from psmain.c
extern uint32 msclock();

//from srvpipe.c
extern int write_to_pipe(pipes_struct *p, char *data, uint32 n);
extern int read_from_pipe(pipes_struct *p, char *data, uint32 n);
extern pipes_struct *get_rpc_pipe_p(char *buf, int where, int threadid);

//from smbutil.c
extern int set_message(char *buf,int num_words,int num_bytes,int zero);

//from process.c
extern int chain_reply(char *inbuf,char *outbuf,int size,int bufsize, int ProcSockID ,int threadid);

#define PIPE            "\\PIPE\\"
#define PIPELEN         strlen(PIPE)

//0507 extern int DEBUGLEVEL;

//0704extern struct pipe_id_info pipe_names[];



/*
 * HACK alert.... see above - JRA.
 */
//copy from nttrans.c 0611/2001
//0706 static int fail_next_srvsvc[NUM_SMBTHREAD] = {False};

/* I do not declare fail_next_srvsvc[[NUM_SMBTHREAD], since the action that WinXP 
   get printing information will send spools pipe will be rejected by me. ""BUT"" he send
   next srvsvc in different connection. That thread fail_next_srvsvc[threadid] != 1 (since 
   I did not set in this thread). Maybe I set fail_next_srvsvc[i] at previous thread, 
   and it can not reject the next srvsvc pipe, and WinXP get printer information fail.   Ron 3/8/2002  */
   
int fail_next_srvsvc = False; 
static time_t fail_time;
extern int chain_size[];

#define HACK_FAIL_TIME 2 /* In seconds. */
void fail_next_srvsvc_open(int threadid)
{
  /* Check client is WinNT proper; Win2K doesn't like Jeremy's hack - matty */
//0611  if (get_remote_arch() != RA_WINNT)
//    return;

  fail_next_srvsvc = True;
  fail_time = (msclock()/1000);
//  DEBUG(10,("fail_next_srvsvc_open: setting up timeout close of \\srvsvc pipe for print fix.\n"));
}

int should_fail_next_srvsvc_open(const char *pipename, int threadid)
{

//  DEBUG(10,("should_fail_next_srvsvc_open: fail = %d, pipe = %s\n",
//    (int)fail_next_srvsvc, pipename));

  if(fail_next_srvsvc && ((msclock()/1000) > fail_time + HACK_FAIL_TIME)) {
  	  fail_next_srvsvc = False;
      fail_time = (msclock()/1000);
//    DEBUG(10,("should_fail_next_srvsvc_open: End of timeout close of \\srvsvc pipe for print fix.\n"));
  }

  if(fail_next_srvsvc && !strcmp(pipename, "srvsvc")) {
      fail_next_srvsvc = False;
//    DEBUG(10,("should_fail_next_srvsvc_open: Deliberately failing open of \\srvsvc pipe for print fix.\n"));
      return True;
  }
  return False;
}


/****************************************************************************
  reply to an open and X on a named pipe

  This code is basically stolen from reply_open_and_X with some
  wrinkles to handle pipes.
****************************************************************************/
#if 0 //0629
int reply_open_pipe_and_X(char *inbuf,char *outbuf,int length,int bufsize)
{
	pstring fname;
	uint16 vuid = SVAL(inbuf, smb_uid);
	pipes_struct *p;
	int smb_ofun = SVAL(inbuf,smb_vwv8);
	int size=0,fmode=0,mtime=0,rmode=0;
	int i;

	/* XXXX we need to handle passed times, sattr and flags */
	pstrcpy(fname,smb_buf(inbuf));

	/* If the name doesn't start \PIPE\ then this is directed */
	/* at a mailslot or something we really, really don't understand, */
	/* not just something we really don't understand. */
	if ( strncmp(fname,PIPE,PIPELEN) != 0 )
		return(ERROR(ERRSRV,ERRaccess));

//0507	DEBUG(4,("Opening pipe %s.\n", fname));

	/* See if it is one we want to handle. */
	for( i = 0; pipe_names[i].client_pipe ; i++ )
//0507		if( strequal(fname,pipe_names[i].client_pipe) )
		if( !strcmp(fname,pipe_names[i].client_pipe) )
			break;

	if (pipe_names[i].client_pipe == NULL)
		return(ERROR(ERRSRV,ERRaccess));
	/* Strip \PIPE\ off the name. */
	pstrcpy(fname,smb_buf(inbuf) + PIPELEN);

	/*
	 * Hack for NT printers... JRA.
	 */
//0611    if(should_fail_next_srvsvc_open(fname))
//      return(ERROR(ERRSRV,ERRaccess));
//0507	  	return 0;
	/* Known pipes arrive with DIR attribs. Remove it so a regular file */
	/* can be opened and add it in after the open. */
//0507	DEBUG(3,("Known pipe %s opening.\n",fname));
	smb_ofun |= FILE_CREATE_IF_NOT_EXIST;

	p = open_rpc_pipe_p(fname, vuid, threadid);

	if (!p) return(ERROR(ERRSRV,ERRnofids));

	/* Prepare the reply */
	set_message(outbuf,15,0,True);

	/* Mark the opened file as an existing named pipe in message mode. */
	SSVAL(outbuf,smb_vwv9,2);
	SSVAL(outbuf,smb_vwv10,0xc700);

	if (rmode == 2) {
//0507		DEBUG(4,("Resetting open result to open from create.\n"));
		rmode = 1;
	}

	SSVAL(outbuf,smb_vwv2, p->pnum);
	SSVAL(outbuf,smb_vwv3,fmode);
//0507	put_dos_date3(outbuf,smb_vwv4,mtime);
	SIVAL(outbuf,smb_vwv6,size);
	SSVAL(outbuf,smb_vwv8,rmode);
	SSVAL(outbuf,smb_vwv11,0x0001);

	return chain_reply(inbuf,outbuf,length,bufsize);
}
#endif //0
/****************************************************************************
  reply to a write on a pipe
****************************************************************************/
int reply_pipe_write(char *inbuf,char *outbuf,int length,int dum_bufsize, int threadid)
{
	pipes_struct *p = get_rpc_pipe_p(inbuf,smb_vwv0, threadid);
	uint32 numtowrite = SVAL(inbuf,smb_vwv1);
	int nwritten;
	int outsize;
	char *data;

	if (!p)
		return(ERROR(ERRDOS,ERRbadfid));

	data = smb_buf(inbuf) + 3;

	if (numtowrite == 0)
		nwritten = 0;
	else
		nwritten = write_to_pipe(p, data, numtowrite);

	if ((nwritten == 0 && numtowrite != 0) || (nwritten < 0))
//0507		return (UNIXERROR(ERRDOS,ERRnoaccess));
      	return 0;
	outsize = set_message(outbuf,1,0,True);

	SSVAL(outbuf,smb_vwv0,nwritten);
  
//0507	DEBUG(3,("write-IPC pnum=%04x nwritten=%d\n",
//0507		 p->pnum, nwritten));

	return(outsize);
}

/****************************************************************************
 Reply to a write and X.

 This code is basically stolen from reply_write_and_X with some
 wrinkles to handle pipes.
****************************************************************************/
//#if 0
int reply_pipe_write_and_X(char *inbuf,char *outbuf,int length,int bufsize, int ProcSockID, int threadid)
{
	pipes_struct *p = get_rpc_pipe_p(inbuf,smb_vwv2, threadid);
	uint32 numtowrite = SVAL(inbuf,smb_vwv10);
	int nwritten = -1;
	int smb_doff = SVAL(inbuf, smb_vwv11);
	int pipe_start_message_raw = ((SVAL(inbuf, smb_vwv7) & (PIPE_START_MESSAGE|PIPE_RAW_MODE)) ==
									(PIPE_START_MESSAGE|PIPE_RAW_MODE));
	char *data;

	if (!p)
		return(ERROR(ERRDOS,ERRbadfid));

	data = smb_base(inbuf) + smb_doff;

	if (numtowrite == 0)
		nwritten = 0;
	else {
		if(pipe_start_message_raw) {
			/*
			 * For the start of a message in named pipe byte mode,
			 * the first two bytes are a length-of-pdu field. Ignore
			 * them (we don't trust the client. JRA.
			 */
		if(numtowrite < 2) {
//0507				DEBUG(0,("reply_pipe_write_and_X: start of message set and not enough data sent.(%u)\n",
//0507					(unsigned int)numtowrite ));
//0507				return (UNIXERROR(ERRDOS,ERRnoaccess));
				return 0;
			}

			data += 2;
			numtowrite -= 2;
	}                        
		nwritten = write_to_pipe(p, data, numtowrite);
	}

	if ((nwritten == 0 && numtowrite != 0) || (nwritten < 0))
//0507		return (UNIXERROR(ERRDOS,ERRnoaccess));
			return 0;	
	set_message(outbuf,6,0,True);

	nwritten = (pipe_start_message_raw ? nwritten + 2 : nwritten);
	SSVAL(outbuf,smb_vwv2,nwritten);
  
//0507	DEBUG(3,("writeX-IPC pnum=%04x nwritten=%d\n",
//0507		 p->pnum, nwritten));

	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
}
//#endif //0
/****************************************************************************
  reply to a read and X

  This code is basically stolen from reply_read_and_X with some
  wrinkles to handle pipes.
****************************************************************************/
int reply_pipe_read_and_X(char *inbuf,char *outbuf,int length,int bufsize, 
							int ProcSockID, int threadid)
{
	pipes_struct *p = get_rpc_pipe_p(inbuf,smb_vwv2, threadid);
	int smb_maxcnt = SVAL(inbuf,smb_vwv5);
	int smb_mincnt = SVAL(inbuf,smb_vwv6);
	int nread = -1;
	char *data;
	/* we don't use the offset given to use for pipe reads. This
	   is deliberate, instead we always return the next lump of
	   data on the pipe */
#if 0
	uint32 smb_offs = IVAL(inbuf,smb_vwv3);
#endif //0

	if (!p)
		return(ERROR(ERRDOS,ERRbadfid));

	set_message(outbuf,12,0,True);
	data = smb_buf(outbuf);

	nread = (int)read_from_pipe(p, data, (uint32)smb_maxcnt);

	if (nread < 0)
//0507		return(UNIXERROR(ERRDOS,ERRnoaccess));
		return 0;
	SSVAL(outbuf,smb_vwv5,nread);
//0706	SSVAL(outbuf,smb_vwv6,smb_offset(data,outbuf));
	SSVAL(outbuf,smb_vwv6,(PTR_DIFF(data,outbuf+4) 
					+ chain_size[threadid]));//smb_offset(p,buf)
	SSVAL(smb_buf(outbuf),-2,nread);
  
//0507	DEBUG(3,("readX-IPC pnum=%04x min=%d max=%d nread=%d\n",
//0507		 p->pnum, smb_mincnt, smb_maxcnt, nread));

	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
}

/****************************************************************************
  reply to a close
****************************************************************************/
int reply_pipe_close(char *inbuf,char *outbuf, int threadid)
{
	pipes_struct *p = get_rpc_pipe_p(inbuf,smb_vwv0, threadid);
	int outsize = set_message(outbuf,0,0,True);

	if (!p)
		return(ERROR(ERRDOS,ERRbadfid));
//0507	DEBUG(5,("reply_pipe_close: pnum:%x\n", p->pnum));

	if (!close_rpc_pipe_hnd(p, threadid))
		return(ERROR(ERRDOS,ERRbadfid));

	return(outsize);
}
