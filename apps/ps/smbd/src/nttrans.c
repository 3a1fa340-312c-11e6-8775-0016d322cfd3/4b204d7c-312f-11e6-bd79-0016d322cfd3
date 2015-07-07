/*
   Unix SMB/Netbios implementation.
   Version 1.9.
   SMB NT transaction handling
   Copyright (C) Jeremy Allison 1994-1998

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

#include "nterr.h"

//from srvpipe.c
extern pipes_struct *open_rpc_pipe_p(char *pipe_name, uint16 vuid, int threadid);

//from pipes.c
extern int should_fail_next_srvsvc_open(const char *pipename, int threadid);

//from smbutil.c
extern int set_message(char *buf,int num_words,int num_bytes,int zero);

//from process.c
extern int chain_reply(char *inbuf,char *outbuf,int size,int bufsize, int ProcSockID ,int threadid);
extern int receive_next_smb(char *inbuf, int bufsize, int timeout, int ProcSockID ,int threadid);
                      
//from utilsock.c
extern int send_smb(int fd,char *buffer);


//0618extern int DEBUGLEVEL;
extern int Protocol[];
//extern int Client;  
extern int smb_read_error[];
extern int global_oplock_break;
//0620extern int case_sensitive;
//0620extern int case_preserve;
//0620extern int short_case_preserve;

//static void remove_pending_change_notify_requests_by_mid(int mid);

static char *known_nt_pipes[] = {
  "\\LANMAN",
  "\\srvsvc",
//  "\\samr",
  "\\wkssvc",
// "\\NETLOGON",
//  "\\ntlsa",
  "\\ntsvcs",
//  "\\lsass",
//  "\\lsarpc",
  "\\winreg",
  NULL
};

/****************************************************************************
 Reply to an NT create and X call on a pipe.
****************************************************************************/
int nt_open_pipe(char *fname, char *inbuf, char *outbuf, int *ppnum, int threadid)
{
	pipes_struct *p = NULL;

	uint16 vuid = SVAL(inbuf, smb_uid);
	int i;
	

//Jesse
	if ( !strcmp(fname, "\\spoolss"))
		return(ERROR(ERRDOS,ERRbadpipe));

//0502	DEBUG(4,("nt_open_pipe: Opening pipe %s.\n", fname));
    
	/* See if it is one we want to handle. */
	for( i = 0; known_nt_pipes[i]; i++ )
//0430		if( strequal(fname,known_nt_pipes[i]))
		if( !strcmp(fname,known_nt_pipes[i]))
			break;
    
	if ( known_nt_pipes[i] == NULL )
		return(ERROR(ERRSRV,ERRaccess));
    
	/* Strip \\ off the name. */
	fname++;
    
	if(should_fail_next_srvsvc_open(fname, threadid))
		return (ERROR(ERRSRV,ERRaccess));

//0502	DEBUG(3,("nt_open_pipe: Known pipe %s opening.\n", fname));

	p = open_rpc_pipe_p(fname, vuid, threadid);
	if (!p)
		return(ERROR(ERRSRV,ERRnofids));

	*ppnum = p->pnum;

	return 0;
}

/****************************************************************************
  reply to an open and X on a named pipe

  This code is basically stolen from reply_open_and_X with some
  wrinkles to handle pipes.
****************************************************************************/
int reply_open_pipe_and_X(char *inbuf,char *outbuf,int length,int bufsize, int ProcSockID, int threadid)
{
	pstring fname;
	uint16 vuid = SVAL(inbuf, smb_uid);
	pipes_struct *p;
	int smb_ofun = SVAL(inbuf,smb_vwv8);
	int size=0,fmode=0,mtime=0,rmode=0;
	int i;
	
	/* XXXX we need to handle passed times, sattr and flags */
	strcpy(fname,smb_buf(inbuf));

	/* If the name doesn't start \PIPE\ then this is directed */
	/* at a mailslot or something we really, really don't understand, */
	/* not just something we really don't understand. */
//	if ( strcmp(fname,PIPE) != 0 )
//		return(ERROR(ERRSRV,ERRaccess));

	/* See if it is one we want to handle. */
	for( i = 0; known_nt_pipes[i]; i++ )
		if( !strcmp(&fname[5],known_nt_pipes[i]))
			break;
    
	if ( known_nt_pipes[i] == NULL )
		return(ERROR(ERRSRV,ERRaccess));

	/* Strip \PIPE\ off the name. */
//	strcpy(fname,smb_buf(inbuf) + PIPELEN);

	/*
	 * Hack for NT printers... JRA.
	 */
	if(should_fail_next_srvsvc_open(&fname[6], threadid))
		return (ERROR(ERRSRV,ERRaccess));


	/* Known pipes arrive with DIR attribs. Remove it so a regular file */
	/* can be opened and add it in after the open. */
	smb_ofun |= FILE_CREATE_IF_NOT_EXIST;

	p = open_rpc_pipe_p(&fname[6], vuid, threadid);
	if (!p) return(ERROR(ERRSRV,ERRnofids));

	/* Prepare the reply */
	set_message(outbuf,15,0,True);

	/* Mark the opened file as an existing named pipe in message mode. */
	SSVAL(outbuf,smb_vwv9,2);
	SSVAL(outbuf,smb_vwv10,0xc700);

	if (rmode == 2) {
//		DEBUG(4,("Resetting open result to open from create.\n"));
		rmode = 1;
	}

	SSVAL(outbuf,smb_vwv2, p->pnum);
	SSVAL(outbuf,smb_vwv3,fmode);
//	put_dos_date3(outbuf,smb_vwv4,mtime);
	SIVAL(outbuf,smb_vwv6,size);
	SSVAL(outbuf,smb_vwv8,rmode);
	SSVAL(outbuf,smb_vwv11,0x0001);

	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
}

/****************************************************************************
 Reply to a NT CANCEL request.
****************************************************************************/
int reply_ntcancel(connection_struct *conn, char *inbuf,char *outbuf,int length,int bufsize)
{
	/*
	 * Go through and cancel any pending change notifies.
	 */
	
//	int mid = SVAL(inbuf,smb_mid);
//	remove_pending_change_notify_requests_by_mid(mid);
//	remove_pending_lock_requests_by_mid(mid);
	
//0618	DEBUG(3,("reply_ntcancel: cancel called on mid = %d.\n", mid));

	return(-1);
}

/****************************************************************************
 Reply to IOCTL - not implemented - no plans.
****************************************************************************/
static int call_nt_transact_ioctl(char *inbuf, char *outbuf, int length,
				  int bufsize, 
				  char **ppsetup, char **ppparams, char **ppdata)
{
//0704  static int logged_message = False;

//0704  if(!logged_message) {
//0618    DEBUG(0,("call_nt_transact_ioctl: Currently not implemented.\n"));
//0704    logged_message = True; /* Only print this once... */
//0704  }

  return(ERROR(ERRSRV,ERRnosupport));
}
   
/****************************************************************************
 Reply to a SMBNTtrans.
****************************************************************************/
int reply_nttrans(connection_struct *conn, char *inbuf,char *outbuf,int length,
					int bufsize, int ProcSockID, int threadid)
{
  int  outsize = 0;
#if 0 /* Not used. */
  uint16 max_setup_count = CVAL(inbuf, smb_nt_MaxSetupCount);
  uint32 max_parameter_count = IVAL(inbuf, smb_nt_MaxParameterCount);
  uint32 max_data_count = IVAL(inbuf,smb_nt_MaxDataCount);
#endif //0/* Not used. */
  uint32 total_parameter_count = IVAL(inbuf, smb_nt_TotalParameterCount);
  uint32 total_data_count = IVAL(inbuf, smb_nt_TotalDataCount);
  uint32 parameter_count = IVAL(inbuf,smb_nt_ParameterCount);
  uint32 parameter_offset = IVAL(inbuf,smb_nt_ParameterOffset);
  uint32 data_count = IVAL(inbuf,smb_nt_DataCount);
  uint32 data_offset = IVAL(inbuf,smb_nt_DataOffset);
  uint16 setup_count = 2*CVAL(inbuf,smb_nt_SetupCount); /* setup count is in *words* */
  uint16 function_code = SVAL( inbuf, smb_nt_Function);
  char *params = NULL, *data = NULL, *setup = NULL;
  uint32 num_params_sofar, num_data_sofar;

//0620  if(global_oplock_break && (function_code == NT_TRANSACT_CREATE)) {
    /*
     * Queue this open message as we are the process of an oplock break.
     */

//0618    DEBUG(2,("reply_nttrans: queueing message NT_TRANSACT_CREATE \
//0618due to being in oplock break state.\n" ));

//0620    push_oplock_pending_smb_message( inbuf, length);
//    return -1;
//  }

  outsize = set_message(outbuf,0,0,True);

  /* 
   * All nttrans messages we handle have smb_wct == 19 + setup_count.
   * Ensure this is so as a sanity check.
   */

  if(CVAL(inbuf, smb_wct) != 19 + (setup_count/2)) {
//0618    DEBUG(2,("Invalid smb_wct %d in nttrans call (should be %d)\n",
//0618	  CVAL(inbuf, smb_wct), 19 + (setup_count/2)));
    return(ERROR(ERRSRV,ERRerror));
  }
    
  /* Allocate the space for the setup, the maximum needed parameters and data */

  if(setup_count > 0)
    setup = (char *)malloc(setup_count);
  if (total_parameter_count > 0)
    params = (char *)malloc(total_parameter_count);
  if (total_data_count > 0)
    data = (char *)malloc(total_data_count);
 
  if ((total_parameter_count && !params)  || (total_data_count && !data) ||
      (setup_count && !setup)) {
//0618    DEBUG(0,("reply_nttrans : Out of memory\n"));
    return(ERROR(ERRDOS,ERRnomem));
  }

  /* Copy the param and data bytes sent with this request into
     the params buffer */
  num_params_sofar = parameter_count;
  num_data_sofar = data_count;

  if (parameter_count > total_parameter_count || data_count > total_data_count){
//0618    exit_server("reply_nttrans: invalid sizes in packet.\n");
        if(params)
	  		free(params);
		if(data)
	  		free(data);
		if(setup)
	  		free(setup); //addd by ron 7/19/2001
		return -1;
  }																				
  if(setup) {
    memcpy( setup, &inbuf[smb_nt_SetupStart], setup_count);
//0618    DEBUG(10,("reply_nttrans: setup_count = %d\n", setup_count));
//0618    dump_data(10, setup, setup_count);
  }
  if(params) {
    memcpy( params, smb_base(inbuf) + parameter_offset, parameter_count);
//0618    DEBUG(10,("reply_nttrans: parameter_count = %d\n", parameter_count));
//0618    dump_data(10, params, parameter_count);
  }
  if(data) {
    memcpy( data, smb_base(inbuf) + data_offset, data_count);
//0618    DEBUG(10,("reply_nttrans: data_count = %d\n",data_count));
//0618    dump_data(10, data, data_count);
  }

  if(num_data_sofar < total_data_count || num_params_sofar < total_parameter_count) {
    /* We need to send an interim response then receive the rest
       of the parameter/data bytes */
    outsize = set_message(outbuf,0,0,True);
    send_smb(ProcSockID,outbuf);

    while( num_data_sofar < total_data_count || num_params_sofar < total_parameter_count) {
      int ret;

      ret = receive_next_smb(inbuf,bufsize,SMB_SECONDARY_WAIT, ProcSockID, threadid);

      if((ret && (CVAL(inbuf, smb_com) != SMBnttranss)) || !ret) {
	outsize = set_message(outbuf,0,0,True);
	if(ret) {
//0618		DEBUG(0,("reply_nttrans: Invalid secondary nttrans packet\n"));
	} else {
//0618		DEBUG(0,("reply_nttrans: %s in getting secondary nttrans response.\n",
//0618			 (smb_read_error == READ_ERROR) ? "error" : "timeout" ));
	}
	if(params)
	  free(params);
	if(data)
	  free(data);
	if(setup)
	  free(setup);
	return(ERROR(ERRSRV,ERRerror));
      }
      
      /* Revise total_params and total_data in case they have changed downwards */
      total_parameter_count = IVAL(inbuf, smb_nts_TotalParameterCount);
      total_data_count = IVAL(inbuf, smb_nts_TotalDataCount);
      num_params_sofar += (parameter_count = IVAL(inbuf,smb_nts_ParameterCount));
      num_data_sofar += ( data_count = IVAL(inbuf, smb_nts_DataCount));
      if (num_params_sofar > total_parameter_count || num_data_sofar > total_data_count){
//0618	exit_server("reply_nttrans2: data overflow in secondary nttrans packet\n");
        if(params)
	  		free(params);
		if(data)
	  		free(data);
		if(setup)
	  		free(setup); //addd by ron 7/19/2001
		return -1;
	  }

      memcpy( &params[ IVAL(inbuf, smb_nts_ParameterDisplacement)], 
	      smb_base(inbuf) + IVAL(inbuf, smb_nts_ParameterOffset), parameter_count);
      memcpy( &data[IVAL(inbuf, smb_nts_DataDisplacement)],
	      smb_base(inbuf)+ IVAL(inbuf, smb_nts_DataOffset), data_count);
    }
  }

  if (Protocol[threadid] >= PROTOCOL_NT1) {
    uint16 flg2 = SVAL(outbuf,smb_flg2);
    SSVAL(outbuf,smb_flg2,flg2 | 0x40); /* IS_LONG_NAME */
  }

  /* Now we must call the relevant NT_TRANS function */
  switch(function_code) {
    case NT_TRANSACT_CREATE:
//0620      outsize = call_nt_transact_create(inbuf, outbuf, length, bufsize, 
//					&setup, &params, &data);
      break;
    case NT_TRANSACT_IOCTL:
      outsize = call_nt_transact_ioctl(inbuf, outbuf, length, bufsize, 
				       &setup, &params, &data);
      break;
    case NT_TRANSACT_SET_SECURITY_DESC:
//0602      outsize = call_nt_transact_set_security_desc(inbuf, outbuf, 
//						   length, bufsize, 
//						   &setup, &params, &data);
      break;
    case NT_TRANSACT_NOTIFY_CHANGE:
//      outsize = call_nt_transact_notify_change(inbuf, outbuf, 
//					       length, bufsize, 
//					       &setup, &params, &data);
      break;
    case NT_TRANSACT_RENAME:
//0602      outsize = call_nt_transact_rename(conn, inbuf, outbuf, length, 
//0602					bufsize, 
//0602					&setup, &params, &data);
      break;

    case NT_TRANSACT_QUERY_SECURITY_DESC:
//0602      outsize = call_nt_transact_query_security_desc(conn, inbuf, outbuf, 
//0602						     length, bufsize, 
//0602						     &setup, &params, &data);
      break;
  default:
	  /* Error in request */
//0618	  DEBUG(0,("reply_nttrans: Unknown request %d in nttrans call\n", function_code));
	  if(setup)
		  free(setup);
	  if(params)
		  free(params);
	  if(data)
		  free(data);
	  return (ERROR(ERRSRV,ERRerror));
  }

  /* As we do not know how many data packets will need to be
     returned here the various call_nt_transact_xxxx calls
     must send their own. Thus a call_nt_transact_xxxx routine only
     returns a value other than -1 when it wants to send
     an error packet. 
  */

  if(setup)
    free(setup);
  if(params)
    free(params);
  if(data)
    free(data);
  return outsize; /* If a correct response was needed the call_nt_transact_xxxx 
		     calls have already sent it. If outsize != -1 then it is
		     returning an error packet. */
}
