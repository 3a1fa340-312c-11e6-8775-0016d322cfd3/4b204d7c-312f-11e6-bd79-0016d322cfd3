/*                                                   
   Unix SMB/Netbios implementation.
   Version 1.9.
   process incoming packets - main loop
   Copyright (C) Andrew Tridgell 1992-1998
   
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"

#include "local.h"

//from psmain.c
extern uint32 msclock();

//from utilsock.c
extern int send_smb(int fd,char *buffer);
extern int receive_smb(int fd,char *buffer, unsigned int timeout, int threadid);

//from replay.c
extern int reply_special(char *inbuf,char *outbuf);
extern int reply_unknown(char *inbuf,char *outbuf);
extern int reply_tdis(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize, int ProcSockID, int threadid);		  
extern int reply_ioctl(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize, int ProcSockID, int threadid);
extern int reply_echo(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize, int ProcSockID, int threadid);
extern int reply_sesssetup_and_X(connection_struct *conn, char *inbuf,char *outbuf,	int length,int bufsize, int ProcSockID, int threadid);
extern int reply_tcon_and_X(connection_struct *conn, char *inbuf,char *outbuf,int length, int bufsize, int ProcSockID, int threadid);
extern int reply_write(connection_struct *conn, char *inbuf,char *outbuf,int size,	int dum_buffsize, int ProcSockID, int threadid);
extern int reply_close(connection_struct *conn, char *inbuf,char *outbuf, int size, int dum_buffsize, int ProcSockID, int threadid);
extern int reply_open_and_X(connection_struct *conn, char *inbuf,char *outbuf,	int length,int bufsize, int ProcSockID, int threadid);
extern int reply_printopen(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize, int ProcSockID, int threadid);
extern int reply_printwrite(connection_struct *conn, char *inbuf,char *outbuf , int dum_size, int dum_buffsize, int ProcSockID, int threadid);
extern int reply_printclose(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize, int ProcSockID, int threadid);
extern int reply_write_and_X(connection_struct *conn, char *inbuf,char *outbuf,int dum_size ,int bufsize, int ProcSockID, int threadid);
extern int reply_read_and_X(connection_struct *conn, char *inbuf,char *outbuf, int length,int bufsize, int ProcSockID, int threadid);
extern int reply_ntcreate_and_X(connection_struct *conn, char *inbuf,char *outbuf, int length,int bufsize, int ProcSockID, int threadid);

extern int reply_trans2(connection_struct *conn, char *inbuf,char *outbuf,int length,int bufsize, int ProcSockID, int threadid);

//form ipc.c
extern int reply_trans(connection_struct *conn, char *inbuf,char *outbuf, int size, int bufsize, int ProcSockID, int threadid);

//from conn.c
extern connection_struct *conn_find(int cnum, int threadid);
extern int conn_num_open(int threadid);
extern int conn_idle_all(time_t t, int deadtime, int threadid);
extern void conn_close_all(int threadid);
extern void check_idle_conn(int deadtime, int threadid);

//from srvpipe.c
extern void pipe_close_all(int threadid);

//form negport.c
extern int reply_negprot(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize, int ProcSockID, int threadid);

//form nttrans.c
extern int reply_nttrans(connection_struct *conn, char *inbuf,char *outbuf,int length,	int bufsize, int ProcSockID, int threadid);

//from smbutil.c
extern void smb_setlen(char *buf,int len);
extern int set_message(char *buf,int num_words,int num_bytes,int zero);

void construct_reply_common(char *inbuf,char *outbuf);

//0703struct timeval smb_last_time;
//0703char *last_inbuf = NULL;
/* 
 * Size of data we can send to client. Set
 *  by the client for all protocols above CORE.
 *  Set by us for CORE protocol.
 */
//0710int max_send[NUM_SMBTHREAD] = SMB_BUFFER_SIZE;
int max_send[NUM_SMBTHREAD]; //new define by ron 7/10/2001
/*
 * Size of the data we can receive. Set by us.
 * Can be modified by the max xmit parameter.
 */
int max_recv = SMB_BUFFER_SIZE;
int thread_isused[NUM_SMBTHREAD]={0}; //for debug, can know the thread has used 7/16/2001
extern int preEndoffset[NUM_SMBTHREAD];
//0703int last_message=-1;
extern pstring sesssetup_user;
//0703extern char *last_inbuf;
//0706extern char *InBuffer;
//0706extern char *OutBuffer;
//0705extern int global_machine_password_needs_changing;
extern int smbthread[];   //server.c 7/5/2001
extern int fail_next_srvsvc;
extern int Protocol[]; 
extern int chain_size[];
extern int num_open[];
extern int done_sesssetup[];
extern int smb_read_error[];
uint8 _forceclosethread[NUM_SMBTHREAD]; 
extern int timeout_paperout[NUM_SMBTHREAD];
//0703extern int max_send;
/****************************************************************************
  Do a select on an two fd's - with timeout. 

  If a local udp message has been pushed onto the
  queue (this can only happen during oplock break
  processing) return this first.

  If a pending smb message has been pushed onto the
  queue (this can only happen during oplock break
  processing) return this next.

  If the first smbfd is ready then read an smb from it.
  if the second (loopback UDP) fd is ready then read a message
  from it and setup the buffer header to identify the length
  and from address.
  Returns False on timeout or error.
  Else returns True.

The timeout is in milli seconds
****************************************************************************/

static int receive_message_or_smb(char *buffer, int buffer_len, 
//Jesse                                   int *got_smb, int ProcSockID, int threadid)
 									int *got_smb, int fd, int threadid)

{
  int ProcSockID = fd;
  fd_set fds;
  *got_smb = False;

  /*
   * Setup the select read fd set.
   */
  FD_SET(ProcSockID,&fds);

  if (FD_ISSET(ProcSockID,&fds))
 {
    *got_smb = True;
    return receive_smb(ProcSockID, buffer, 0, threadid);
  }
}

/****************************************************************************
Get the next SMB packet, doing the local message processing automatically.
****************************************************************************/

int receive_next_smb(char *inbuf, int bufsize, int timeout, int ProcSockID
                      ,int threadid)
{
  int got_smb = False;
  int ret;

  do
  {
    ret = receive_message_or_smb(inbuf,bufsize,&got_smb, ProcSockID, threadid);

    if(ret && !got_smb)
    {
      continue;
    }

    if(ret && (CVAL(inbuf,0) == 0x85))
    {
      /* Keepalive packet. */
      got_smb = False;
    }

  }
  while(ret && !got_smb);

  return ret;
}



/*
These flags determine some of the permissions required to do an operation 

Note that I don't set NEED_WRITE on some write operations because they
are used by some brain-dead clients when printing, and I don't want to
force write permissions on print services.
*/
#define AS_USER (1<<0)
#define NEED_WRITE (1<<1)
#define TIME_INIT (1<<2)
#define CAN_IPC (1<<3)
#define AS_GUEST (1<<5)
#define QUEUE_IN_OPLOCK (1<<6)
/* 
   define a list of possible SMB messages and their corresponding
   functions. Any message that has a NULL function is unimplemented -
   please feel free to contribute implementations!
*/
struct smb_message_struct
{
  int code;
  char *name;
  int (*fn)(connection_struct *conn,char *, char *, int, int, int, int);
  int flags;
} smb_messages[] = {

    /* CORE PROTOCOL */

   {SMBnegprot,"SMBnegprot",reply_negprot,0},
   {SMBtdis,"SMBtdis",reply_tdis,0},
   {SMBioctl,"SMBioctl",reply_ioctl,0},
   {SMBecho,"SMBecho",reply_echo,0},
   {SMBsesssetupX,"SMBsesssetupX",reply_sesssetup_and_X,0},
   {SMBtconX,"SMBtconX",reply_tcon_and_X,0},

   /* note that SMBmknew and SMBcreate are deliberately overloaded */   
   {SMBwrite,"SMBwrite",reply_write,AS_USER | CAN_IPC },
   {SMBclose,"SMBclose",reply_close,AS_USER | CAN_IPC },

   /* CORE+ PROTOCOL FOLLOWS */

   /* LANMAN1.0 PROTOCOL FOLLOWS */
   {SMBtrans,"SMBtrans",reply_trans,AS_USER | CAN_IPC},
   {SMBnttrans, "SMBnttrans", reply_nttrans, AS_USER | CAN_IPC },
   {SMBopenX,"SMBopenX",reply_open_and_X,AS_USER | CAN_IPC},
//   {SMBflush,"SMBflush",reply_flush,AS_USER},
   {SMBsplopen,"SMBsplopen",reply_printopen,AS_USER},
	{SMBsplwr,"SMBsplwr",reply_printwrite,AS_USER},
   {SMBsplclose,"SMBsplclose",reply_printclose,AS_USER},
   {SMBreadX,"SMBreadX",reply_read_and_X,AS_USER | CAN_IPC },
   {SMBwriteX,"SMBwriteX",reply_write_and_X,AS_USER | CAN_IPC },   
   {SMBntcreateX, "SMBntcreateX", reply_ntcreate_and_X, AS_USER | CAN_IPC },
   
   /* LANMAN2.0 PROTOCOL FOLLOWS */
//   {SMBfindnclose, "SMBfindnclose", reply_findnclose, AS_USER},
//   {SMBfindclose, "SMBfindclose", reply_findclose,AS_USER},
//   {SMBtrans2, "SMBtrans2", reply_trans2, AS_USER | QUEUE_IN_OPLOCK }
//   {SMBtranss2, "SMBtranss2", reply_transs2, AS_USER},
 };


/****************************************************************************
do a switch on the message type, and return the response size
****************************************************************************/
static int switch_message(int type,char *inbuf,char *outbuf,
							int size,int bufsize, int ProcSockID, int threadid)
{
  int outsize = 0;
  static int num_smb_messages = 
    sizeof(smb_messages) / sizeof(struct smb_message_struct);
  int match;
//  extern int Client;
  connection_struct *conn;
//0703  last_message = type;

  /* make sure this is an SMB packet */
  if (strncmp(smb_base(inbuf),"\377SMB",4) != 0)
  {
//os  	kwait(NULL);
	cyg_thread_yield();
    return(-1);
  }

  for (match=0;match<num_smb_messages;match++)
    if (smb_messages[match].code == type)
      break;

  if (match == num_smb_messages)
  {
//Jesse      outsize = reply_unknown(inbuf,outbuf);
outsize = ERROR(ERRSRV,ERRaccess);
  }
  else
  {
    if (smb_messages[match].fn)
    {
//0609      int flags = smb_messages[match].flags;
//0609      static uint16 last_session_tag = UID_FIELD_INVALID;
	  uint16 session_tag = SVAL(inbuf,smb_uid);	

      /* Ensure this value is replaced in the incoming packet. */
      SSVAL(inbuf,smb_uid,session_tag);

//0703      last_inbuf = inbuf;
	  conn = conn_find(SVAL(inbuf, smb_tid), threadid);

	  if (conn)
	  	  conn->lastused = (msclock() /1000);  //add by ron 7/5/2001

      outsize = smb_messages[match].fn(conn,inbuf,outbuf,size,bufsize, ProcSockID, 
	   								   threadid);
    }
    else
    {
      outsize = reply_unknown(inbuf,outbuf);
    }
  }

  return(outsize);
}


/****************************************************************************
  construct a reply to the incoming packet
****************************************************************************/
static int construct_reply(char *inbuf,char *outbuf,int size,int bufsize,
							 int ProcSockID, int threadid)
{
  int type = CVAL(inbuf,smb_com);
  int outsize = 0;
  int msg_type = CVAL(inbuf,0);

  chain_size[threadid] = 0;

  if (msg_type != 0)
    return(reply_special(inbuf,outbuf));  

  construct_reply_common(inbuf, outbuf);

  outsize = switch_message(type,inbuf,outbuf,size,bufsize, ProcSockID, threadid);

  outsize += chain_size[threadid];

  if(outsize > 4)
    smb_setlen(outbuf,outsize - 4);
  return(outsize);
}


/****************************************************************************
  process an smb from the client - split out from the process() code so
  it can be used by the oplock break code.
****************************************************************************/
void process_smb(char *inbuf, char *outbuf, int ProcSockID, int threadid)
{
//  extern int Client;
#ifdef WITH_SSL
#endif /* WITH_SSL */
  int msg_type = CVAL(inbuf,0);
  int32 len = smb_len(inbuf);
  int nread = len + 4;

#ifdef WITH_PROFILE
  profile_p->smb_count++;
#endif

#ifdef WITH_VTP
  if(trans_num == 1 && VT_Check(inbuf)) 
  {
    VT_Process();
    return;
  }
#endif
  if(msg_type == 0x85)
    return; /* Keepalive packet. */

  nread = construct_reply(inbuf,outbuf,nread,max_send[threadid], ProcSockID
  					,threadid );
      
  if(nread > 0) 
  {
	
    if (nread != smb_len(outbuf) + 4) 
    {
    }
    else 
      send_smb(ProcSockID,outbuf);
  }
}


/****************************************************************************
 Helper function for contruct_reply.
****************************************************************************/

void construct_reply_common(char *inbuf,char *outbuf)
{
  memset(outbuf,'\0',smb_size);

  set_message(outbuf,0,0,True);
  CVAL(outbuf,smb_com) = CVAL(inbuf,smb_com);

  memcpy(outbuf+4,inbuf+4,4);
  CVAL(outbuf,smb_rcls) = SMB_SUCCESS;
  CVAL(outbuf,smb_reh) = 0;
  SCVAL(outbuf,smb_flg, FLAG_REPLY | (CVAL(inbuf,smb_flg) & FLAG_CASELESS_PATHNAMES)); /* bit 7 set
                                 means a reply */
  SSVAL(outbuf,smb_flg2,FLAGS2_LONG_PATH_COMPONENTS); /* say we support long filenames */
  SSVAL(outbuf,smb_err,SMB_SUCCESS);
  SSVAL(outbuf,smb_tid,SVAL(inbuf,smb_tid));
  SSVAL(outbuf,smb_pid,SVAL(inbuf,smb_pid));
  SSVAL(outbuf,smb_uid,SVAL(inbuf,smb_uid));
  SSVAL(outbuf,smb_mid,SVAL(inbuf,smb_mid));
}

/****************************************************************************
  construct a chained reply and add it to the already made reply
  **************************************************************************/
int chain_reply(char *inbuf,char *outbuf,int size,int bufsize, int ProcSockID
					,int threadid)
{
  static char *orig_inbuf[NUM_SMBTHREAD];
  static char *orig_outbuf[NUM_SMBTHREAD];

  int smb_com1, smb_com2 = CVAL(inbuf,smb_vwv0);
  unsigned smb_off2 = SVAL(inbuf,smb_vwv1);
  char *inbuf2, *outbuf2;
  int outsize2;
  char inbuf_saved[smb_wct];
  char outbuf_saved[smb_wct];
  int wct = CVAL(outbuf,smb_wct);
  int outsize = smb_size + 2*wct + SVAL(outbuf,smb_vwv0+2*wct);

  /* maybe its not chained */
  if (smb_com2 == 0xFF) {
    CVAL(outbuf,smb_vwv0) = 0xFF;
    return outsize;
  }

  if (chain_size[threadid] == 0) {
    /* this is the first part of the chain */
    orig_inbuf[threadid] = inbuf;
    orig_outbuf[threadid] = outbuf;
  }

  /*
   * The original Win95 redirector dies on a reply to
   * a lockingX and read chain unless the chain reply is
   * 4 byte aligned. JRA.
   */

  outsize = (outsize + 3) & ~3;

  /* we need to tell the client where the next part of the reply will be */
//0706  SSVAL(outbuf,smb_vwv1,smb_offset(outbuf+outsize,outbuf));
  SSVAL(outbuf,smb_vwv1,(PTR_DIFF(outbuf+outsize,outbuf+4) 
  					+ chain_size[threadid])); //smb_offset(p,buf)
  CVAL(outbuf,smb_vwv0) = smb_com2;

  /* remember how much the caller added to the chain, only counting stuff
     after the parameter words */
  chain_size[threadid] += outsize - smb_wct;

  /* work out pointers into the original packets. The
     headers on these need to be filled in */
  inbuf2 = orig_inbuf[threadid] + smb_off2 + 4 - smb_wct;
  outbuf2 = orig_outbuf[threadid] + SVAL(outbuf,smb_vwv1) + 4 - smb_wct;

  /* remember the original command type */
  smb_com1 = CVAL(orig_inbuf[threadid],smb_com);

  /* save the data which will be overwritten by the new headers */
  memcpy(inbuf_saved,inbuf2,smb_wct);
  memcpy(outbuf_saved,outbuf2,smb_wct);

  /* give the new packet the same header as the last part of the SMB */
  memmove(inbuf2,inbuf,smb_wct);

  /* create the in buffer */
  CVAL(inbuf2,smb_com) = smb_com2;

  /* create the out buffer */
  construct_reply_common(inbuf2, outbuf2);

//  DEBUG(3,("Chained message\n"));
//0123  show_msg(inbuf2);

  /* process the request */
  outsize2 = switch_message(smb_com2,inbuf2,outbuf2,size-chain_size[threadid],
			    bufsize-chain_size[threadid], ProcSockID, threadid);

  /* copy the new reply and request headers over the old ones, but
     preserve the smb_com field */
  memmove(orig_outbuf[threadid],outbuf2,smb_wct);
  CVAL(orig_outbuf[threadid],smb_com) = smb_com1;

  /* restore the saved data, being careful not to overwrite any
   data from the reply header */
  memcpy(inbuf2,inbuf_saved,smb_wct);
  {
    int ofs = smb_wct - PTR_DIFF(outbuf2,orig_outbuf[threadid]);
    if (ofs < 0) ofs = 0;
    memmove(outbuf2+ofs,outbuf_saved+ofs,smb_wct-ofs);

  }
  return outsize2;
}

/****************************************************************************
 Prolast_idle_closed_check[threadid] = 0; //reset static variablecess any timeout housekeeping. Return False if the caler should exit.
****************************************************************************/

static int timeout_processing(int deadtime, int threadid)
{
//0703  extern int Client;
//0703  static time_t last_keepalive_sent_time = 0;
  static time_t last_idle_closed_check[NUM_SMBTHREAD] = {0};
  time_t t;
  int allidle = True;
//0703  extern int keepalive;

  if (_forceclosethread[threadid] ==1)
    return 0;  //force closed this thread  ....By Ron 3/11/2002

//0705  *last_timeout_processing_time = t = (msclock()/1000);
  t = (msclock()/1000);

//0703  if(last_keepalive_sent_time == 0)
//    last_keepalive_sent_time = t;

  if(last_idle_closed_check[threadid] == 0)
    last_idle_closed_check[threadid] = t;

  /* automatic timeout if all connections are closed */      
//0705  if (conn_num_open()==0 && (t - last_idle_closed_check) >= IDLE_CLOSED_TIMEOUT) 
//    DEBUG( 2, ( "Closing idle connection\n" ) );
//    	  return False;
//0705  else
//      last_idle_closed_check = t;

  if (conn_num_open(threadid)==0 && 
  		(t - last_idle_closed_check[threadid]) >= IDLE_CLOSED_TIMEOUT) 
  {
//    DEBUG( 2, ( "Closing idle connection\n" ) );
	  last_idle_closed_check[threadid] = 0; //reset static variable
      return False;
  }
  else if (conn_num_open(threadid) > 0)
      last_idle_closed_check[threadid] = t;

  /* check for connection timeouts */
  allidle = conn_idle_all(t, deadtime, threadid);

//0705  if (allidle && conn_num_open()>0) {
//    DEBUG(2,("Closing idle connection 2.\n"));

  if (allidle && conn_num_open(threadid) > 0)  {
  	  last_idle_closed_check[threadid] = 0; //reset static variable	
      return False;
  }


  if (smb_read_error[threadid] == READ_EOF) 
  {
//    DEBUG(3,("end of file from client\n"));
	last_idle_closed_check[threadid] = 0; //reset static variable
    return False;
  }

  if (smb_read_error[threadid] == READ_ERROR) 
  {
//    DEBUG(3,("receive_smb error (%s) exiting\n",
//              strerror(errno)));
	last_idle_closed_check[threadid] = 0; //reset static variable
    return False;
  }


//0703  *select_timeout = setup_select_timeout();

  return True;
}

static void reset_global_variables(int threadid){
	Protocol[threadid] = PROTOCOL_COREPLUS;
	chain_size[threadid] = 0;
	num_open[threadid] = 0;
	done_sesssetup[threadid] = 0; //Flase
	smb_read_error[threadid] = 0;
	max_send[threadid] = MAX_SEND_SIZE;
    smbthread[threadid] = -1;
    preEndoffset[threadid] =-1;
    _forceclosethread[threadid] =0;
    timeout_paperout[threadid] =0;
}

/****************************************************************************
  process commands from the client
****************************************************************************/

//os void smbd_process(int ProcSockID, void *nouse1, void *nouse2)
void smbd_process (cyg_addrword_t data)
{
	int ProcSockID = data;
//0704  extern int smb_echo_count;
//0705  time_t last_timeout_processing_time = time(NULL);

/* 0416 from global variable */
//char __inbuf[SMB_BUFFER_SIZE + SAFETY_MARGIN];
//0621char __oubuf[SMB_BUFFER_SIZE + SAFETY_MARGIN];

//Jesse char __inbuf[SMB_BUFFER_SIZE];
//Jesse char __oubuf[MAX_SEND_SIZE];
char __inbuf[SMB_BUFFER_SIZE + 16];
char __oubuf[MAX_SEND_SIZE + 16];
char *InBuffer = __inbuf;
char *OutBuffer = __oubuf;
int i, threadid=0;
int testInbufsize=0, testOutbufsize=0;

	testInbufsize = sizeof(__inbuf);
	testOutbufsize = sizeof(__oubuf);
//  InBuffer += SMB_ALIGNMENT;   // ??? why ??? 7/18/2001 by ron
//  OutBuffer += SMB_ALIGNMENT;
  /* re-initialise the timezone */
//0302  TimeInit();

  for (i =0; i < NUM_SMBTHREAD; i++){
   	if (smbthread[i] == ProcSockID){
		threadid =i;
		break;
	}
  }		

  if (threadid >= NUM_SMBTHREAD)
     threadid = -1;		
  		
  thread_isused[threadid]++;  //for debug
  while (True)
  {
//    int change_notify_timeout = 1 * 1000;
    int got_smb = False;
    
//0705    int select_timeout = SMBD_SELECT_TIMEOUT*1000;//setup_select_timeout();

	check_idle_conn(RELEASE_RESOURCE_TIMEOUT, threadid);  // check Idle Connection ....Ron Add 3/8/2002
    while(!receive_message_or_smb(InBuffer,SMB_BUFFER_SIZE, &got_smb
					,ProcSockID ,threadid) || _forceclosethread[threadid] ==1)
    {
        if(!timeout_processing(SET_SMBD_TIMEOUT, threadid)){
	  	    conn_close_all(threadid);
			pipe_close_all(threadid); //7/19/2001 by ron
		    reset_global_variables(threadid);
//Jesse		    close_s(ProcSockID); //close socket 6/29/2001 by ron
		    close(ProcSockID); //close socket
			cyg_thread_exit();
            return;
	    }	
//0418      num_smbs = 0; /* Reset smb counter. */
//os		kwait(NULL);
		cyg_thread_yield();
    } //while

	if (threadid < 0 ||ProcSockID < 0)
		{
		cyg_thread_exit();			
		return;
		}

    if(got_smb) {
      /*
       * Ensure we do timeout processing if the SMB we just got was
       * only an echo request. This allows us to set the select
       * timeout in 'receive_message_or_smb()' to any value we like
       * without worrying that the client will send echo requests
       * faster than the select timeout, thus starving out the
       * essential processing (change notify, blocking locks) that
       * the timeout code does. JRA.
       */ 
//0704      int num_echos = smb_echo_count;

      process_smb(InBuffer, OutBuffer, ProcSockID, threadid);
    } //if 
//os	kwait(NULL);
    sys_check_stack();
	cyg_thread_yield();
  } //while

}
