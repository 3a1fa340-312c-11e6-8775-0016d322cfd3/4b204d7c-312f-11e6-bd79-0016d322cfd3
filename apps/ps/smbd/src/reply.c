/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Main SMB reply routines
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
/*
   This file handles most of the reply_ calls that the server
   makes to handle specific protocols
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

#include "prnqueue.h" //0620
#include "prnport.h" //0620
#include "smbinc.h"
#include "smb.h"

#include "version.h"

//0603#include "nterr.h"

//from psmain.c
extern char * strupr ( char * string );
extern uint32 msclock();

//from smbutil.c
extern void smb_setlen(char *buf,int len);
extern int set_message(char *buf,int num_words,int num_bytes,int zero);
extern int printingerror(int SMBStatus, char *outbuf);
extern int TempPrintingData(int SockID, char *Srcdata, int threadid, int ret, int numtocpy, uint8 ispaperout, int treeid);

//from conn.c
extern void close_cnum(connection_struct *conn, int threadid);
extern connection_struct *conn_new(pstring *dev, int threadid);

//form smbutil
extern int error_packet(char *inbuf,char *outbuf,int error_class,uint32 error_code); //line just for debug
extern int ClearRecvBuf(int SocketId, char *revbuf, int ntotalclr, int bufsize);
extern void SMBPaperoutprocess(char *outbuf, char *inbuf, int OS, uint32 Startoffset, int ProcSockID
                                   , char *data, int ret, uint32 numtowrite, int threadid, int treeid );
extern int CopytempbuftoPrnQueue(struct prnbuf *smbprnbuf, int ProcSockID, char *inbuf, int threadid
                       ,int ret, int preEndoffset, uint32 Startoffset, uint32 numtowrite, int treeid);

//from process.c
extern int chain_reply(char *inbuf,char *outbuf,int size,int bufsize, int ProcSockID ,int threadid);

//from pipes.c
extern int reply_pipe_write(char *inbuf,char *outbuf,int length,int dum_bufsize, int threadid);
extern int reply_pipe_close(char *inbuf,char *outbuf, int threadid);
extern int reply_pipe_read_and_X(char *inbuf,char *outbuf,int length,int bufsize, 
							int ProcSockID, int threadid);

//from utilsock.c
extern int send_smb(int fd,char *buffer);

//from unistr.c
extern char *dos_unistrn2(uint16 *src, int len);

//from nttrans
extern int nt_open_pipe(char *fname, char *inbuf, char *outbuf, int *ppnum, int threadid);

/* look in server.c for some explanation of these variables */
extern int Protocol[];
//0703extern int DEBUGLEVEL;
extern int max_send[];
extern int max_recv;
//0703extern char magic_char;
//0620extern int case_sensitive;
//0620extern int case_preserve;
//0620extern int short_case_preserve;
extern pstring sesssetup_user;
extern fstring global_myname;
//extern fstring global_myworkgroup;
//extern int Client;
//0704unsigned int smb_echo_count = 0;
//0702extern int smbprnportid; /* ""PrnPortID when printing"".define in ipc.c , 
//			   					get in function api_DosPrintQGetInfo() */
//static uint16 *ucs2_to_doscp; //0504
extern int smb_read_error[NUM_SMBTHREAD];
extern BYTE G_PortReady; //define at 1284.c 
int done_sesssetup[NUM_SMBTHREAD] = {False}; 
uint16 printing_conn_tid[3];
struct Tempdata *Gtempdata[NUM_SMBTHREAD];
int preEndoffset[NUM_SMBTHREAD];
int timeout_paperout[NUM_SMBTHREAD];

// George Add March 6, 2007
BYTE	btTimeout_SMB;

/****************************************************************************
  reply to an special message 
****************************************************************************/

int reply_special(char *inbuf,char *outbuf)
{
	int outsize = 4;
	int msg_type = CVAL(inbuf,0);
	int msg_flags = CVAL(inbuf,1);
//	pstring name1,name2;
//	extern fstring remote_machine;
//	extern fstring local_machine;
	int len;
	char name_type = 0;
	int testlen = smb_size;
	
//	*name1 = *name2 = 0;
	
	memset(outbuf,'\0',smb_size);

	smb_setlen(outbuf,0);
	
	switch (msg_type) {
	case 0x81: /* session request */
		CVAL(outbuf,0) = 0x82;
		CVAL(outbuf,3) = 0;
//		fstrcpy(remote_machine,name2);
//		remote_machine[15] = 0;
//		strlwr(remote_machine);		

//		fstrcpy(local_machine,name1);
//		len = strlen(local_machine);
//		if (len == 16) {
//			name_type = local_machine[15];
//			local_machine[15] = 0;
//		}
//		strlwr(local_machine);

//		if (name_type == 'R') {
			/* We are being asked for a pathworks session --- 
			   no thanks! */
//			CVAL(outbuf, 0) = 0x83;
//			break;
//		}
		break;
		
	case 0x89: /* session keepalive request 
		      (some old clients produce this?) */
		CVAL(outbuf,0) = 0x85;
		CVAL(outbuf,3) = 0;
		break;
		
	case 0x82: /* positive session response */
	case 0x83: /* negative session response */
	case 0x84: /* retarget session response */
		break;
		
	case 0x85: /* session keepalive */
	default:
		return(0);
	}
	
	return(outsize);
}

/****************************************************************************
 Reply to a tcon and X.
****************************************************************************/


int reply_tcon_and_X(connection_struct *conn, char *inbuf,char *outbuf,int length,
						int bufsize, int ProcSockID, int threadid)
{
	fstring service;
	pstring devicename;
	int passlen = SVAL(inbuf,smb_vwv3);
	char *path;
	char *p;
	int i;
	*service = *devicename = 0;

	/* we might have to close an old one */
	if ((SVAL(inbuf,smb_vwv2) & 0x1) && conn) { // I don't know how can make it ? Ron 3/8/2002
 		for ( i =0; i< 3 ; i++){
 			if (printing_conn_tid[i] == conn->cnum){
        		if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed )
 					PrnSetNoUse(conn->smbprnportid);
 			}	
 		}
		close_cnum(conn, threadid);
	}

	path = smb_buf(inbuf) + passlen;
	fstrcpy(service,path+2);
	p = strchr(service,'\\');
	if (!p){
		return(ERROR(ERRSRV,ERRinvnetname));
	}

	*p = 0;
	fstrcpy(service,p+1);
	p = strchr(service,'%');
	if (p) {
		*p++ = 0;
	}

	strncpy(devicename,path + strlen(path) + 1,6);	

  	if (!strcmp(service,"IPC$"))
		pstrcpy(devicename,"IPC");

	if (*devicename == '?' || !*devicename) {
		pstrcpy(devicename,"LPT1:");
	}

	if (Protocol[threadid] < PROTOCOL_NT1) {
		set_message(outbuf,2,strlen(devicename)+1,True);
		pstrcpy(smb_buf(outbuf),devicename);
	} else {
		
		char *fsname = "IPC";

		set_message(outbuf,3,3,True);

		p = smb_buf(outbuf);
		pstrcpy(p,devicename); p += strlen(p)+1; /* device name */
		pstrcpy(p,fsname); p += strlen(p)+1; /* filesystem type e.g NTFS */
		
		set_message(outbuf,3,PTR_DIFF(p,smb_buf(outbuf)),False);

 		conn = conn_new(devicename, threadid);

  		if (!conn)
			return 0;
//0704   			return(connection_error(inbuf,outbuf,ecode));


/* If Client send Printing Tree Connect request, service can get which PrnPOrt
		will be printing,so we let this Connection's (this Tid) conn->smbprnportid
		save this PrnPort   7/2/2001 by ron   */

	for (i=0; i< NUM_OF_PRN_PORT; i++){
		if(!strcmp(strupr(service),strupr(Smbprinterserver[i]))){
		  conn->smbprnportid = i;
		  break;
		}  
	}

/* If client send service name dosn't match any service witch server have. 
	( That is conn->smbprnportid is still default (-1) )	
	  Maybe client setting printer name error, I permit use Port1 to Print. */	

//0704	if (conn->smbprnportid == -1)
//		conn->smbprnportid = 0;

//		if (conn->smbprnportid == -1)
//			return(ERROR(ERRSRV,ERRinvnetname));
  
		/* what does setting this bit do? It is set by NT4 and
		   may affect the ability to autorun mounted cdroms */
	 	SSVAL(inbuf,smb_tid,conn->cnum);
  		SSVAL(outbuf,smb_tid,conn->cnum);
//0626		SSVAL(outbuf,smb_tid,1);


 	   SSVAL(outbuf, smb_vwv2, SMB_SUPPORT_SEARCH_BITS); 
	}
	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
}

/****************************************************************************
  reply to an unknown type
****************************************************************************/
int reply_unknown(char *inbuf,char *outbuf)
{
//0709	int type;
//	type = CVAL(inbuf,smb_com);
  
//	DEBUG(0,("unknown command type (%s): type=%d (0x%X)\n",
//		 smb_fn_name(type), type, type));
  
	return(ERROR(ERRSRV,ERRunknownsmb));
}


/****************************************************************************
  reply to an ioctl
****************************************************************************/
int reply_ioctl(connection_struct *conn, char *inbuf,char *outbuf, int dum_size,
				 int dum_buffsize, int ProcSockID, int threadid)
{
	uint16 device     = SVAL(inbuf,smb_vwv1);
	uint16 function   = SVAL(inbuf,smb_vwv2);
	uint32 ioctl_code = (((uint32) device) << 16) + function;
	int replysize, outsize;
	char *p;

	switch (ioctl_code)
	{
	    case IOCTL_QUERY_JOB_INFO:
		replysize = 32;
		break;
	    default: ;
	}

	outsize = set_message(outbuf,8,replysize+1,True);
	SSVAL(outbuf,smb_vwv1,replysize); /* Total data bytes returned */
	SSVAL(outbuf,smb_vwv5,replysize); /* Data bytes this buffer */
	SSVAL(outbuf,smb_vwv6,52);        /* Offset to data */
	p = smb_buf(outbuf) + 1;          /* Allow for alignment */

	switch (ioctl_code)
	{
	    case IOCTL_QUERY_JOB_INFO:
		SSVAL(p,0,1);                            /* Job number */
		strncpy(p+2, global_myname, 15);         /* Our NetBIOS name */ 
		strncpy(p+18, Smbprinterserver[conn->smbprnportid], 13); /* Service name */
		
		break;
	}

	return outsize;
}

/****************************************************************************
reply to a session setup command
****************************************************************************/

int reply_sesssetup_and_X(connection_struct *conn, char *inbuf,char *outbuf,
						int length,int bufsize, int ProcSockID, int threadid)
{
  uint16 sess_vuid;
  int   smb_bufsize;    
//0709  pstring user;
//  pstring orig_user;

  /* change done_sesssetup[threadid] to global variable, in order to reset 
  	 it to False when this thread will be closed. 7/6/2001 by ron */	
//0706  static int done_sesssetup[NUM_SMBTHREAD] = False;

//0609  int guest=False;
  smb_bufsize = SVAL(inbuf,smb_vwv2);
  /* it's ok - setup a reply */
  if (Protocol[threadid] < PROTOCOL_NT1) {
    set_message(outbuf,3,0,True);
  } else {
    char *p;
    set_message(outbuf,3,3,True);
    p = smb_buf(outbuf);
    pstrcpy(p,"Unix"); p += strlen(p)+1;
    pstrcpy(p,"Samba "); pstrcat(p,VERSION); p += strlen(p)+1;
// By testing, the workgroup is WIN98 ---3/27/2001
    pstrcpy(p,_MyWorkgroup); p += strlen(p)+1;  
    set_message(outbuf,3,PTR_DIFF(p,smb_buf(outbuf)),False);
    /* perhaps grab OS version here?? */ 
  }

  /* Set the correct uid in the outgoing and incoming packets
     We will use this on future requests to determine which
     user we should become.
     */

  /* register the name and uid as being validated, so further connections
     to a uid can get through without a password, on the same VC */
//0119  sess_vuid = register_vuid(uid,gid,user,sesssetup_user,guest);
		sess_vuid = 100; //0328/2001
//0418  SSVAL(outbuf,smb_uid,sess_vuid); //0328/2001
  CVAL(outbuf,smb_uid+9) = 1; //0328/2001 To change Action 1
  SSVAL(inbuf,smb_uid,sess_vuid);
  SSVAL(outbuf, smb_vwv2,1); //0613/2001 login as guest
  if (!done_sesssetup[threadid])
    max_send[threadid] = MIN(max_send[threadid],smb_bufsize);


  done_sesssetup[threadid] = True;

  return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
}

/****************************************************************************
  reply to an open
****************************************************************************/
#if 0 //0410
int reply_open(connection_struct *conn, char *inbuf,char *outbuf, int dum_size, int dum_buffsize)
{
  pstring fname;
  int outsize = 0;
  int fmode=0;
  int share_mode;
  SMB_OFF_T size = 0;
  time_t mtime=0;
  mode_t unixmode;
  int rmode=0;
//0719  SMB_STRUCT_STAT sbuf;
  int bad_path = False;
  files_struct *fsp;
  int oplock_request = CORE_OPLOCK_REQUEST(inbuf);
 
  share_mode = SVAL(inbuf,smb_vwv0);

  pstrcpy(fname,smb_buf(inbuf)+1);
    
  fsp = file_new();
  if (!fsp){
  }	      
  open_file_shared(fsp,fname,share_mode,(FILE_FAIL_IF_NOT_EXIST|FILE_EXISTS_OPEN),
                   1, oplock_request,&rmode,NULL);

  size = sbuf.st_size;
  mtime = sbuf.st_mtime;
  outsize = set_message(outbuf,7,0,True);
  SSVAL(outbuf,smb_vwv0,fsp->fnum);
  SSVAL(outbuf,smb_vwv1,fmode);
  SIVAL(outbuf,smb_vwv4,(uint32)size);
  SSVAL(outbuf,smb_vwv6,rmode);

  return(outsize);
}
#endif

/****************************************************************************
  reply to an open and X
****************************************************************************/
int reply_open_and_X(connection_struct *conn, char *inbuf,char *outbuf,
						int length,int bufsize, int ProcSockID, int threadid)
{
//0628  pstring fname;
//0609  int smb_mode = SVAL(inbuf,smb_vwv3);
//0609  int smb_attr = SVAL(inbuf,smb_vwv5);
//0609  int smb_ofun = SVAL(inbuf,smb_vwv8);
  SMB_OFF_T size=0;
//0609  int fmode=0,mtime=0,rmode=0;
  int fmode=0;	
//0718  SMB_STRUCT_STAT sbuf;
  int smb_action = 0;
//0628  pstrcpy(fname,smb_buf(inbuf));

//0718  size = sbuf.st_size;
//0609  mtime = sbuf.st_mtime;
	int i;
	int Status;

  conn->ClientOs = Windows98;
  set_message(outbuf,15,0,True);
  switch (ReadPortStatus(conn->smbprnportid)){  //Ron Add 2/25/2002
    case PORT_OFF_LINE:
    case PORT_FAULT:
    case PORT_NO_CONNECT:
	  CVAL(outbuf,smb_rcls) = ERRHRD;
      SSVAL(outbuf,smb_err,ERRnotready);  
	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);   
	break;
	
	case PORT_PAPER_OUT:
	  conn->prnpaperout =1;
	break;
	
	default:
	break;
  }

  switch (Status =PrnGetPrinterStatus(conn->smbprnportid)){
    case PrnNoUsed:	//Start print
	break;

	case SMBUsed:
		if (printing_conn_tid[conn->smbprnportid] != conn->cnum){
          SSVAL(outbuf,smb_rcls,ERRSRV); //Error Command Add by Ron 2/21/2002          
          SSVAL(outbuf,smb_err,ERRqfull); //Prn queue full Add by Ron 2/21/2002          
	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
		}
	break;
	default:
        SSVAL(outbuf,smb_rcls,ERRSRV); //Error Command Add by Ron 2/21/2002          
        SSVAL(outbuf,smb_err,ERRqfull); //Prn queue full Add by Ron 2/21/2002          
	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);          
  }

  SSVAL(outbuf,smb_vwv3,fmode);
  SIVAL(outbuf,smb_vwv6,(uint32)size);
  SSVAL(outbuf,smb_vwv8, 1); //"Write Only"copy from open.c open_file_share()  
  SSVAL(outbuf,smb_vwv11,smb_action);

  return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
}

/****************************************************************************
  reply to a write
****************************************************************************/
int reply_write(connection_struct *conn, char *inbuf,char *outbuf,int size,
					int dum_buffsize, int ProcSockID, int threadid)
{
  uint32 numtowrite =0;
  int nwritten = 0;
  char *data = NULL;
  struct prnbuf *smbprnbuf = NULL;
  BYTE GetSMBStatus=0;
  int outsize = 0;
  int ret=0, recvlen=0;
  int nwritetoprnbuf=0;
  int idatashift=0;
  int queuetime, t;
  int PortId=0;
  int SMBReadStatus =0;
  char *ptrStartoffset = NULL;
  uint32 Startoffset =0;
  struct Tempdata *dataptr= NULL, *next =NULL;
  
  uint32	startime;
  
  if (IS_IPC(conn)){
	return reply_pipe_write(inbuf, outbuf, size, dum_buffsize, threadid);
  }	

  PortId = conn->smbprnportid;
/* code belowe is printing process , Ron Command */
  if (conn->ClientOs == WindowsCE || conn->ClientOs == Linux)
  	numtowrite = dum_buffsize;  // When reply_printwrite().. I use this parament to send the numtowrite ...6/4/2002 by Ron
  else
    numtowrite = SVAL(inbuf,smb_vwv1);
  data = smb_buf(inbuf) + 3;  
  ptrStartoffset = smb_buf(inbuf) - 8;  //data start offset Add by Ron 2/28/2002
  memcpy(&Startoffset, ptrStartoffset, 4);  
  
  idatashift = data - inbuf -4;  // netbios data has 4 bytes
  recvlen = min(numtowrite +idatashift,SMB_BUFFER_SIZE) ;  	  
  ret = recvlen - idatashift; // the data bytes that inbuf (SMB_BUFFER_SIZE bytes defined at  process.c )received

  GetSMBStatus = PrnGetPrinterStatus(PortId);	
  switch (GetSMBStatus){
    case PrnNoUsed:	//Start print
		  PrnSetSMBInUse(PortId);
			#ifdef SUPPORT_JOB_LOG							//Joseph 2004/06/17
				JL_PutList(6, PortId, global_myname, 32);
			#endif SUPPORT_JOB_LOG
		  printing_conn_tid[PortId] = conn->cnum; 
	break;

	case SMBUsed:
		if (printing_conn_tid[PortId] != conn->cnum){
    	  outsize = set_message(outbuf,1,0,True);  
          SSVAL(outbuf,smb_rcls,ERRSRV); //Error Command Add by Ron 2/21/2002          
          SSVAL(outbuf,smb_err,ERRqfull); //Prn queue full Add by Ron 2/21/2002          
          ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);
          return(outsize);          			
		}
	break;
	
	default:
    	outsize = set_message(outbuf,1,0,True);  
        SSVAL(outbuf,smb_rcls,ERRSRV); //Error Command Add by Ron 2/21/2002          
        SSVAL(outbuf,smb_err,ERRqfull); //Prn queue full Add by Ron 2/21/2002          
        ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);        
    return(outsize);          			
  }
  
/* if paper out in the start of printing  */
  if (conn->prnpaperout == 1){
  	if ( ReadPortStatus(PortId) !=  PORT_PAPER_OUT )
  	  conn->prnpaperout = 0;  // the printer can start printing
  	  
    SMBPaperoutprocess(outbuf, inbuf, conn->ClientOs, Startoffset,ProcSockID, data, ret, numtowrite, threadid, conn->cnum);  					  	
    outsize = set_message(outbuf,1,0,True);        
    preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////  
    return (outsize);               	  
  }	
  
/* Now I never Put any data in PrnQueue, and I can reply Client the error command. Ron Add 2/25/2002*/
  t = msclock()/1000;
  while((smbprnbuf = PrnGetInQueueBuf(PortId)) == NULL) {	
//os	kwait(0);
	cyg_thread_yield();
	if (printingerror(SMBReadStatus= ReadPortStatus(PortId), outbuf) ){
	  if (SMBReadStatus == PORT_PAPER_OUT ){
  	    SMBPaperoutprocess(outbuf, inbuf, conn->ClientOs, Startoffset,ProcSockID, data, ret, numtowrite, threadid, conn->cnum);  					  	
  	    outsize = set_message(outbuf,1,0,True);        
        preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////  
  	    return (outsize);               	  
	  }
      else{  //without temp data
        ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);
        outsize = set_message(outbuf,1,0,True);        
        preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////
        return(outsize);               
      } 	
	}  
	else if (timeout_paperout[threadid] >2){
	  SMBPaperoutprocess(outbuf, inbuf, conn->ClientOs, Startoffset,ProcSockID, data, ret, numtowrite, threadid, conn->cnum);  					  	
  	  outsize = set_message(outbuf,1,0,True);        
      preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////  
  	  return (outsize);               	  
	}
    else if ((queuetime = msclock()/1000) -t > SMB_GET_PRNQUEUE_TIMEOUT ){  // can not get rpnqueue, maybe busy
      timeout_paperout[threadid]++;
      if ( (conn->ClientOs == Windows98) && ( Startoffset == preEndoffset[threadid] )){      	
	    if (!TempPrintingData(ProcSockID, data, threadid, ret, numtowrite, 0, conn->cnum)){  // more than SMB_GET_PRNQUEUE_TIMEOUT, I temp it, and reply Client    		
    	  CVAL(outbuf,smb_rcls) = ERRHRD;
          SSVAL(outbuf,smb_err,ERRwrite);  	      		
      	}
        nwritten = numtowrite;  	
        SSVAL(outbuf,smb_vwv0,nwritten);
        outsize = set_message(outbuf,1,0,True);        
        preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////
        return(outsize);               	  
      } 
      else{      
        ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);
        SSVAL(outbuf,smb_vwv0,nwritten);
        outsize = set_message(outbuf,1,0,True);        
        preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////
        return(outsize);          
      }
    }
  } //while  

/* Copy the temp data when I get Prnqueue */
  if (conn->ClientOs == Windows98 && Gtempdata[threadid] != NULL && Gtempdata[threadid]->treeid == conn->cnum){
  	if (!CopytempbuftoPrnQueue(smbprnbuf, ProcSockID, inbuf, threadid, ret
  	           , preEndoffset[threadid], Startoffset, numtowrite, conn->cnum )){
  	  CVAL(outbuf,smb_rcls) = ERRHRD;
      SSVAL(outbuf,smb_err,ERRwrite);  	      		
      outsize = set_message(outbuf,1,0,True);  
      preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////
      return(outsize);             	  		
  	}	  
	PrnPutOutQueueBuf(PortId,smbprnbuf,PRN_Q_NORMAL);   //copy from IPPD\IPPD.c
	
	for (dataptr = Gtempdata[threadid]; dataptr ;dataptr = next){
	  if ((next = dataptr->next) == NULL)
	  	break;	  
	}	
/*  find the lastest tempdata pointer...
    I suggest if the lastest temp is by paperout, then now prnport status is paperout  3/5/2002*/ 
	if ((dataptr != NULL) && (dataptr->paperout == 1)){
	  CVAL(outbuf,smb_rcls) = ERRHRD;
      SSVAL(outbuf,smb_err,ERRnopaper); 	
    }
	else {  	  	  
	  nwritten = numtowrite;  	
      SSVAL(outbuf,smb_vwv0,nwritten);		
	}	
    outsize = set_message(outbuf,1,0,True);  
    preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////
    return(outsize);             	  	
  }
  timeout_paperout[threadid] =0;  
/* Normal Process */  
  memcpy(smbprnbuf->data, data, ret);
  data = (smbprnbuf->data) + ret;
  nwritetoprnbuf += ret;
  nwritten += ret;

  if (nwritten == numtowrite)  // if nwritten == numtowrite, we need not to receive any data, and put the data out
  {
	smbprnbuf->size = nwritetoprnbuf;
	PrnPutOutQueueBuf(PortId,smbprnbuf,PRN_Q_NORMAL);   //copy from IPPD\IPPD.c    	
  }	
  while(nwritten < numtowrite )
  {
	while ((nwritten < numtowrite)&&(nwritetoprnbuf <BLOCKSIZE )) //BLOCKSIZE is PrnQueue buffer size
	{
//os	  kalarm(30000L);
		btTimeout_SMB = 0; 
      ret = recv(ProcSockID, data, min(numtowrite - nwritten, BLOCKSIZE - nwritetoprnbuf),0);      
//os	  kalarm(0L);
      if (ret == 0 )
		break;
      if (ret == -1 )
        return -1;		
	  nwritetoprnbuf += ret;	
	  nwritten += ret;
	  data += ret;	 	
    }
	smbprnbuf->size = nwritetoprnbuf;

#ifdef SUPPORT_JOB_LOG						//Joseph 2004/06/17
		JL_AddSize(PortId, nwritetoprnbuf);
	#endif SUPPORT_JOB_LOG
    PrnPutOutQueueBuf(PortId,smbprnbuf,PRN_Q_NORMAL);   //copy from IPPD\IPPD.c  
/********************************************************************************************    
     If (nwritten < numtowrite) now, it must be call PrnGetInQueueBuf get next prnqueue
     and recv() data continuously. Now one prnqueue.data size is BLOCKSIZE == 1460 *7,
     so the smb printing data (must < 4096 bytes) must be received completely  	... Ron 9/2/2003 
********************************************************************************************/    	
	}	// end of while (nwritten < numtowrite )

  outsize = set_message(outbuf,1,0,True);  
  SSVAL(outbuf,smb_vwv0,nwritten);
  preEndoffset[threadid] = Startoffset + numtowrite; ///////////////////////////////////////
  return(outsize);
}
//#if 0
/****************************************************************************
  reply to a write and X
****************************************************************************/
int reply_write_and_X(connection_struct *conn, char *inbuf,char *outbuf,int dum_size
											,int bufsize, int ProcSockID, int threadid)
{
  size_t numtowrite = SVAL(inbuf,smb_vwv10);

  /* If it's an IPC, pass off the pipe handler. */
  if (IS_IPC(conn)) {
    return reply_pipe_write_and_X(inbuf,outbuf,dum_size,bufsize, ProcSockID, threadid );
  }

  conn->ClientOs = Linux;        
/* numtowrite got at different position of inbuf from reply_write()
	, so I must send parament (use parament dummy_buffersize )   6/4/2002 by Ron */
  reply_write(conn, inbuf, outbuf, dum_size, numtowrite, ProcSockID, threadid);	  

  set_message(outbuf,6,0,True);  
  return chain_reply(inbuf,outbuf,dum_size,bufsize, ProcSockID, threadid);
}
//#endif //0
/****************************************************************************
 Reply to a close - has to deal with closing a directory opened by NT SMB's.
****************************************************************************/
int reply_close(connection_struct *conn, char *inbuf,char *outbuf, int size,
                int dum_buffsize, int ProcSockID, int threadid)
{
	int outsize = 0;
	/* If it's an IPC, pass off to the pipe handler. */
	if (IS_IPC(conn))
		return reply_pipe_close(inbuf,outbuf, threadid);

	if (printing_conn_tid[conn->smbprnportid] == conn->cnum){
		if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed )
			PrnSetNoUse(conn->smbprnportid);
#ifdef SUPPORT_JOB_LOG							// Joseph 2004/06/17
			if(btTimeout_SMB == 1)
			{
				JL_EndList(conn->smbprnportid, 3);	// Timeout. George Add March 6, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_TPLINA) && !defined(O_LS)
				SendEOF(conn->smbprnportid);	    // Send the EOF page. George Add Junuary 10, 2008
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
			}
			else
				JL_EndList(conn->smbprnportid, 0);	// Completed. George Add January 26, 2007
			#endif SUPPORT_JOB_LOG
	}	

	outsize = set_message(outbuf,0,0,True);
	return(outsize);
}
/****************************************************************************
  reply to a tdis
****************************************************************************/
int reply_tdis(connection_struct *conn, char *inbuf,char *outbuf, int dum_size,
				 int dum_buffsize, int ProcSockID, int threadid)
{
	int outsize = set_message(outbuf,0,0,True);
	
	if (!conn) {
//		DEBUG(4,("Invalid connection in tdis\n"));
		return(ERROR(ERRSRV,ERRinvnid));
	}
	
	if (printing_conn_tid[conn->smbprnportid] == conn->cnum){
		if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed )
			PrnSetNoUse(conn->smbprnportid);
	}	

/* let global variables initial ....by ron*/
	close_cnum(conn, threadid);
	return outsize;
}

/****************************************************************************
  reply to a echo
****************************************************************************/
int reply_echo(connection_struct *conn, char *inbuf,char *outbuf, int dum_size,
					 int dum_buffsize, int ProcSockID, int threadid)
{
	int smb_reverb = SVAL(inbuf,smb_vwv0);
	int seq_num;
	int data_len = smb_buflen(inbuf);
	int outsize = set_message(outbuf,1,data_len,True);
	
	/* copy any incoming data back out */
	if (data_len > 0)
		memcpy(smb_buf(outbuf),smb_buf(inbuf),data_len);

	if (smb_reverb > 100) {
//		DEBUG(0,("large reverb (%d)?? Setting to 100\n",smb_reverb));
		smb_reverb = 100;
	}

	for (seq_num =1 ; seq_num <= smb_reverb ; seq_num++) {
		SSVAL(outbuf,smb_vwv0,seq_num);

		smb_setlen(outbuf,outsize - 4);

		send_smb(ProcSockID,outbuf);
	}

//	DEBUG(3,("echo %d times\n", smb_reverb));

//0704	smb_echo_count++;

	return -1;
}


/****************************************************************************
  reply to a printopen
****************************************************************************/
int reply_printopen(connection_struct *conn, char *inbuf,char *outbuf, 
				int dum_size, int dum_buffsize, int ProcSockID, int threadid)
{
	int outsize = 0;
	int leng = 0;
	
	if (!CAN_PRINT(conn)){
		return(ERROR(ERRDOS,ERRnoaccess));
    }
    switch (ReadPortStatus(conn->smbprnportid)){  //Ron Add 2/25/2002
      case PORT_OFF_LINE:
      case PORT_FAULT:
      case PORT_NO_CONNECT:
	    CVAL(outbuf,smb_rcls) = ERRHRD;
        SSVAL(outbuf,smb_err,ERRnotready);  
        outsize = set_message(outbuf,1,0,True);
	  return(outsize);

      case PORT_PAPER_OUT:
	    conn->prnpaperout =1;
	  break;
	  
	  default :
	  break;
    }
    
    conn->ClientOs = Windows2000;        
	outsize = set_message(outbuf,1,0,True);
//0426	SSVAL(outbuf,smb_vwv0,fsp->fnum);
	SSVAL(outbuf,smb_vwv0,100);  //0426
	
//eason 20101222
	memset(global_myname, 0, 128);	
	leng = strlen((inbuf)+ 44);
	if(leng >= 127)
		leng = 127;
	memcpy(global_myname ,(inbuf)+ 44,leng);

	return(outsize);
}


/****************************************************************************
  reply to a printclose
****************************************************************************/

int reply_printclose(connection_struct *conn, char *inbuf,char *outbuf, 
				int dum_size, int dum_buffsize, int ProcSockID, int threadid)
{
	int outsize = set_message(outbuf,0,0,True);
//0417	files_struct *fsp = file_fsp(inbuf,smb_vwv0);
//0426	int close_err = 0;

//0305	CHECK_FSP(fsp,conn);
//0305	CHECK_ERROR(fsp);

	if (!CAN_PRINT(conn)){
		return(ERROR(ERRDOS,ERRnoaccess));
    }  
//	DEBUG(3,("printclose fd=%d fnum=%d\n",
//		 fsp->fd_ptr->fd,fsp->fnum));
  
//0417	close_err = close_file(fsp,True);

//0426	if(close_err != 0) {
//0426		errno = close_err;
//0302		return(UNIXERROR(ERRHRD,ERRgeneral));
//0426	}

	if (printing_conn_tid[conn->smbprnportid] == conn->cnum){
		if (PrnGetPrinterStatus(conn->smbprnportid) == SMBUsed )
			PrnSetNoUse(conn->smbprnportid);
#ifdef SUPPORT_JOB_LOG							// Joseph 2005/04/17
			if(btTimeout_SMB == 1)
			{
				JL_EndList(conn->smbprnportid, 3);	// Timeout. George Add March 6, 2007
#if !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_TPLINA) && !defined(O_LS)
				SendEOF(conn->smbprnportid);	    // Send the EOF page. George Add Junuary 10, 2008
#endif	// !defined(O_TPLINK) && !defined(O_TPLINM) && !defined(O_TPLINS) && !defined(O_LS)
			}
			else
				JL_EndList(conn->smbprnportid, 0);	// Completed. George Add January 26, 2007
			#endif SUPPORT_JOB_LOG		
	}	


/* let global variables initial */
//0702 	conn->smbprnportid = -1; 

	return(outsize);
}

// recover on 6/4/2002, since WinCE use .... Ron
/****************************************************************************
  reply to a printwrite
****************************************************************************/
int reply_printwrite(connection_struct *conn, char *inbuf,char *outbuf
				, int dum_size, int dum_buffsize, int ProcSockID, int threadid)
{
  uint32 numtowrite =0;
  
  if (!CAN_PRINT(conn)) {
    return ERROR(ERRDOS,ERRnoaccess);
  }
  conn->ClientOs = WindowsCE;        
  numtowrite = SVAL(smb_buf(inbuf),1);
/* numtowrite got at different position of inbuf from reply_write()
	, so I must send parament (use parament dummy_buffersize )   6/4/2002 by Ron */
  return reply_write(conn, inbuf, outbuf, dum_size, numtowrite, ProcSockID, threadid);	  
}

/****************************************************************************
  reply to a read and X
****************************************************************************/
int reply_read_and_X(connection_struct *conn, char *inbuf,char *outbuf,
			 	int length,int bufsize, int ProcSockID, int threadid)
{
//0430  files_struct *fsp = file_fsp(inbuf,smb_vwv2);
//0430  SMB_OFF_T startpos = IVAL(inbuf,smb_vwv3);
//0430  uint32 smb_maxcnt = SVAL(inbuf,smb_vwv5);
//0430  uint32 smb_mincnt = SVAL(inbuf,smb_vwv6);
//0609  int nread = -1;
//0719  char *data;

  /* If it's an IPC, pass off the pipe handler. */
  if (IS_IPC(conn))
    return reply_pipe_read_and_X(inbuf,outbuf,length,bufsize, 
					ProcSockID,  threadid);

//0430  CHECK_FSP(fsp,conn);
//0430  CHECK_READ(fsp);
//0430  CHECK_ERROR(fsp);
#if 0 //0430
  set_message(outbuf,12,0,True);
  data = smb_buf(outbuf);
  if(CVAL(inbuf,smb_wct) == 12) {
#ifdef LARGE_SMB_OFF_T
    /*
     * This is a large offset (64 bit) read.
     */
    startpos |= (((SMB_OFF_T)IVAL(inbuf,smb_vwv10)) << 32);

#else /* !LARGE_SMB_OFF_T */

    /*
     * Ensure we haven't been sent a >32 bit offset.
     */

    if(IVAL(inbuf,smb_vwv10) != 0) {
      DEBUG(0,("reply_read_and_X - large offset (%x << 32) used and we don't support \
64 bit offsets.\n", (unsigned int)IVAL(inbuf,smb_vwv10) ));
      return(ERROR(ERRDOS,ERRbadaccess));
    }

#endif /* LARGE_SMB_OFF_T */

  }

//0430  if (is_locked(fsp,conn,smb_maxcnt,startpos, F_RDLCK))
//0430    return(ERROR(ERRDOS,ERRlock));
//0430  nread = read_file(fsp,data,startpos,smb_maxcnt);
  
//0430  if (nread < 0)
//0430    return(UNIXERROR(ERRDOS,ERRnoaccess));
  nread = 10; //for test 4/30
  SSVAL(outbuf,smb_vwv5,nread);
  SSVAL(outbuf,smb_vwv6,(PTR_DIFF(data,outbuf+4) 
  				+ chain_size[threadid]));//smb_offset(p,buf)
  
  SSVAL(smb_buf(outbuf),-2,nread);
  
//0430  DEBUG( 3, ( "readX fnum=%d min=%d max=%d nread=%d\n",
//0430	      fsp->fnum, (int)smb_mincnt, (int)smb_maxcnt, (int)nread ) );

  return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
#endif
return(ERROR(ERRDOS,ERRbadaccess)); 
}

/****************************************************************************
 Reply to an NT create and X call.
copy from nttrans.c --- 4/30/2001 by ron
****************************************************************************/


/****************************************************************************
 Utility function to map create disposition.
****************************************************************************/
#if 0 //0709
static int map_create_disposition( uint32 create_disposition)
{
  int ret;

  switch( create_disposition ) {
  case FILE_CREATE:
    /* create if not exist, fail if exist */
    ret = (FILE_CREATE_IF_NOT_EXIST|FILE_EXISTS_FAIL);
    break;
  case FILE_SUPERSEDE:
  case FILE_OVERWRITE_IF:
    /* create if not exist, trunc if exist */
    ret = (FILE_CREATE_IF_NOT_EXIST|FILE_EXISTS_TRUNCATE);
    break;
  case FILE_OPEN:
    /* fail if not exist, open if exists */
    ret = (FILE_FAIL_IF_NOT_EXIST|FILE_EXISTS_OPEN);
    break;
  case FILE_OPEN_IF:
    /* create if not exist, open if exists */
    ret = (FILE_CREATE_IF_NOT_EXIST|FILE_EXISTS_OPEN);
    break;
  case FILE_OVERWRITE:
    /* fail if not exist, truncate if exists */
    ret = (FILE_FAIL_IF_NOT_EXIST|FILE_EXISTS_TRUNCATE);
    break;
  default:
//0618    DEBUG(0,("map_create_disposition: Incorrect value for create_disposition = %d\n",
//0618	     create_disposition ));
    return -1;
  }

//0618  DEBUG(10,("map_create_disposition: Mapped create_disposition %lx to %x\n",
//0618	(unsigned long)create_disposition, ret ));

  return ret;
}
#endif
// copy from nttrans.c 
/****************************************************************************
 (Hopefully) temporary call to fix bugs in NT5.0beta2. This OS sends unicode
 strings in NT calls AND DOESN'T SET THE UNICODE BIT !!!!!!!
****************************************************************************/

void get_filename( char *fname, char *inbuf, int data_offset, int data_len, int fname_len)
{
  /*
   * We need various heuristics here to detect a unicode string... JRA.
   */

//  DEBUG(10,("get_filename: data_offset = %d, data_len = %d, fname_len = %d\n",
//	   data_offset, data_len, fname_len ));

  if(data_len - fname_len > 1) {
    /*
     * NT 5.0 Beta 2 has kindly sent us a UNICODE string
     * without bothering to set the unicode bit. How kind.
     *
     * Firstly - ensure that the data offset is aligned
     * on a 2 byte boundary - add one if not.
     */
    fname_len = fname_len/2;
    if(data_offset & 1)
      data_offset++;
    pstrcpy(fname, dos_unistrn2((uint16 *)(inbuf+data_offset), fname_len));
  } else {
    strcpy(fname,inbuf+data_offset);
    fname[fname_len] = '\0';
  }
}

int reply_ntcreate_and_X(connection_struct *conn, char *inbuf,char *outbuf,
					int length,int bufsize, int ProcSockID, int threadid)
{  
	pstring fname;
//0609	uint32 flags = IVAL(inbuf,smb_ntcreate_Flags);
//0609	uint32 desired_access = IVAL(inbuf,smb_ntcreate_DesiredAccess);
//0609	uint32 file_attributes = IVAL(inbuf,smb_ntcreate_FileAttributes);
//0609	uint32 share_access = IVAL(inbuf,smb_ntcreate_ShareAccess);
//0626	uint32 create_disposition = IVAL(inbuf,smb_ntcreate_CreateDisposition);
//0609	uint32 create_options = IVAL(inbuf,smb_ntcreate_CreateOptions);
//0430	uint32 fname_len = MIN(((uint32)SVAL(inbuf,smb_ntcreate_NameLength)),
//0430			       ((uint32)sizeof(fname)-1));
	uint32 fname_len = (uint32)SVAL(inbuf,smb_ntcreate_NameLength);
			       

//0609	uint16 root_dir_fid = (uint16)IVAL(inbuf,smb_ntcreate_RootDirectoryFid);
//0626	int smb_ofun;
//0719	int smb_open_mode;
//0602	int smb_attr = (file_attributes & SAMBA_ATTRIBUTES_MASK);
	/* Breakout the oplock request bits so we can set the
	   reply bits separately. */
//0609	int oplock_request = 0;
//0719	mode_t unixmode;
	int pnum = -1;
//0609	int fmode=0,rmode=0;
//0609	SMB_OFF_T file_len = 0;
//0719	SMB_STRUCT_STAT sbuf;
//0609	int smb_action = 0;
//0609	int bad_path = False;
//0602	files_struct *fsp=NULL;
	char *p = NULL;
//0609	int stat_open_only = False;

	/* 
	 * We need to construct the open_and_X ofun value from the
	 * NT values, as that's what our code is structured to accept.
	 */    
	
//0626	if((smb_ofun = map_create_disposition( create_disposition )) == -1)
//		return(ERROR(ERRDOS,ERRbadaccess));

	/*
	 * Get the file name.
	 */

#if 0 //0430
    if(root_dir_fid != 0) {
      /*
       * This filename is relative to a directory fid.
       */
//0602      files_struct *dir_fsp = file_fsp(inbuf,smb_ntcreate_RootDirectoryFid);
      uint32 dir_name_len;

//0602      if(!dir_fsp)
   //0602	return(ERROR(ERRDOS,ERRbadfid));

//0602      if(!dir_fsp->is_directory) {
	/* 
	 * Check to see if this is a mac fork of some kind.
	 */

	get_filename(&fname[0], inbuf, smb_buf(inbuf)-inbuf, 
		   smb_buflen(inbuf),fname_len);

	if( fname[0] == ':') {
	  SSVAL(outbuf, smb_flg2, FLAGS2_32_BIT_ERROR_CODES);
	  return(ERROR(0, 0xc0000000|NT_STATUS_OBJECT_PATH_NOT_FOUND));
	}
	return(ERROR(ERRDOS,ERRbadfid));
      }

      /*
       * Copy in the base directory name.
       */

//0602      pstrcpy( fname, dir_fsp->fsp_name );
      dir_name_len = strlen(fname);

      /*
       * Ensure it ends in a '\'.
       */

      if(fname[dir_name_len-1] != '\\' && fname[dir_name_len-1] != '/') {
	pstrcat(fname, "\\");
	dir_name_len++;
      }

      /*
       * This next calculation can refuse a correct filename if we're dealing
       * with the Win2k unicode bug, but that would be rare. JRA.
       */

      if(fname_len + dir_name_len >= sizeof(pstring))
	  		return(ERROR(ERRSRV,ERRfilespecs));

      get_filename(&fname[dir_name_len], inbuf, smb_buf(inbuf)-inbuf, 
		   smb_buflen(inbuf),fname_len);

    } else {
      
      get_filename(fname, inbuf, smb_buf(inbuf)-inbuf, 
		   smb_buflen(inbuf),fname_len);
    }
#endif
      get_filename(fname, inbuf, smb_buf(inbuf)-inbuf, 
		   smb_buflen(inbuf),fname_len); //0503

	/* If it's an IPC, use the pipe handler. */

//0430	if (IS_IPC(conn) && lp_nt_pipe_support()) {
	if (IS_IPC(conn)) {
		int ret = nt_open_pipe(fname, inbuf, outbuf, &pnum, threadid);
		if(ret != 0)
			return ret;

		/*
		 * Deal with pipe return.
		 */  

		set_message(outbuf,34,0,True);
	
		p = outbuf + smb_vwv2;
		p++;
		SSVAL(p,0,pnum);
		p += 2;
		SIVAL(p,0,FILE_WAS_OPENED);
		p += 4;
		p += 32;
		SIVAL(p,0,FILE_ATTRIBUTE_NORMAL); /* File Attributes. */
		p += 20;
		/* File type. */
		SSVAL(p,0,FILE_TYPE_MESSAGE_MODE_PIPE);
		/* Device state. */
		SSVAL(p,2, 0x5FF); /* ? */

//0430		DEBUG(5,("reply_ntcreate_and_X: open pipe = %s\n", fname));

		return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
	}

	/*
	 * Now contruct the smb_open_mode value from the filename, 
     * desired access and the share access.
	 */
#if 0 //0430	
//0430	if((smb_open_mode = map_share_mode(&stat_open_only, fname, desired_access, 
//0430					   share_access, 
//0430					   file_attributes)) == -1)
//0430		return(ERROR(ERRDOS,ERRbadaccess));

	oplock_request = (flags & REQUEST_OPLOCK) ? EXCLUSIVE_OPLOCK : 0;
	oplock_request |= (flags & REQUEST_BATCH_OPLOCK) ? BATCH_OPLOCK : 0;

	/*
	 * Ordinary file or directory.
	 */
		
	/*
	 * Check if POSIX semantics are wanted.
	 */
		
	set_posix_case_semantics(file_attributes);
		
//0430	unix_convert(fname,conn,0,&bad_path,NULL);
		
//0602	fsp = file_new();
//0602	if (!fsp) {
//0602		restore_case_semantics(file_attributes);
//0602		return(ERROR(ERRSRV,ERRnofids));
//0602	}
		
	if (!check_name(fname,conn)) { 
		if((errno == ENOENT) && bad_path) {
			unix_ERR_class = ERRDOS;
			unix_ERR_code = ERRbadpath;
		}
//0602		file_free(fsp);
		
		restore_case_semantics(file_attributes);
		
		return(UNIXERROR(ERRDOS,ERRnoaccess));
	} 
		
	unixmode = unix_mode(conn,smb_attr | aARCH, fname);
    
	/* 
	 * If it's a request for a directory open, deal with it separately.
	 */

	if(create_options & FILE_DIRECTORY_FILE) {
		oplock_request = 0;
		
//0602		open_directory(fsp, conn, fname, smb_ofun, 
//0602			       unixmode, &smb_action);
			
		restore_case_semantics(file_attributes);

//0602		if(!fsp->open) {
//0602			file_free(fsp);
//0602			return(UNIXERROR(ERRDOS,ERRnoaccess));
//0602		}
	} else {
		/*
		 * Ordinary file case.
		 */

		/* NB. We have a potential bug here. If we
		 * cause an oplock break to ourselves, then we
		 * could end up processing filename related
		 * SMB requests whilst we await the oplock
		 * break response. As we may have changed the
		 * filename case semantics to be POSIX-like,
		 * this could mean a filename request could
		 * fail when it should succeed. This is a rare
		 * condition, but eventually we must arrange
		 * to restore the correct case semantics
		 * before issuing an oplock break request to
		 * our client. JRA.  */

//0602		open_file_shared(fsp,conn,fname,smb_open_mode,
//0602				 smb_ofun,unixmode, oplock_request,&rmode,&smb_action);

		if (!fsp->open) { 
			/* We cheat here. There are two cases we
			 * care about. One is a directory rename,
			 * where the NT client will attempt to
			 * open the source directory for
			 * DELETE access. Note that when the
			 * NT client does this it does *not*
			 * set the directory bit in the
			 * request packet. This is translated
			 * into a read/write open
			 * request. POSIX states that any open
			 * for write request on a directory
			 * will generate an EISDIR error, so
			 * we can catch this here and open a
			 * pseudo handle that is flagged as a
			 * directory. The second is an open
			 * for a permissions read only, which
			 * we handle in the open_file_stat case. JRA.
			 */

			if(errno == EISDIR) {

				/*
				 * Fail the open if it was explicitly a non-directory file.
				 */

				if (create_options & FILE_NON_DIRECTORY_FILE) {
					file_free(fsp);
					restore_case_semantics(file_attributes);
					SSVAL(outbuf, smb_flg2, FLAGS2_32_BIT_ERROR_CODES);
					return(ERROR(0, 0xc0000000|NT_STATUS_FILE_IS_A_DIRECTORY));
				}
	
				oplock_request = 0;
				open_directory(fsp, conn, fname, smb_ofun, unixmode, &smb_action);
				
				if(!fsp->open) {
					file_free(fsp);
					restore_case_semantics(file_attributes);
					return(UNIXERROR(ERRDOS,ERRnoaccess));
				}
#ifdef EROFS
			} else if (((errno == EACCES) || (errno == EROFS)) && stat_open_only) {
#else /* !EROFS */
			} else if (errno == EACCES && stat_open_only) {
#endif
				/*
				 * We couldn't open normally and all we want
				 * are the permissions. Try and do a stat open.
				 */

				oplock_request = 0;

				open_file_stat(fsp,conn,fname,smb_open_mode,&sbuf,&smb_action);

				if(!fsp->open) {
					file_free(fsp);
					restore_case_semantics(file_attributes);
					return(UNIXERROR(ERRDOS,ERRnoaccess));
				}

			} else {

				if((errno == ENOENT) && bad_path) {
					unix_ERR_class = ERRDOS;
					unix_ERR_code = ERRbadpath;
				}
				
				file_free(fsp);
				
				restore_case_semantics(file_attributes);
				
				return(UNIXERROR(ERRDOS,ERRnoaccess));
			}
		} 
	}
		
	if(fsp->is_directory) {
		if(dos_stat(fsp->fsp_name, &sbuf) != 0) {
			close_file(fsp,True);
			restore_case_semantics(file_attributes);
			return(ERROR(ERRDOS,ERRnoaccess));
		}
	} else {
		if (!fsp->stat_open && sys_fstat(fsp->fd_ptr->fd,&sbuf) != 0) {
			close_file(fsp,False);
			restore_case_semantics(file_attributes);
			return(ERROR(ERRDOS,ERRnoaccess));
		} 
	}
		
	restore_case_semantics(file_attributes);
		
	file_len = sbuf.st_size;
	fmode = dos_mode(conn,fname,&sbuf);
	if(fmode == 0)
		fmode = FILE_ATTRIBUTE_NORMAL;
	if (!fsp->is_directory && (fmode & aDIR)) {
		close_file(fsp,False);
		return(ERROR(ERRDOS,ERRnoaccess));
	} 
	
	/* 
	 * If the caller set the extended oplock request bit
	 * and we granted one (by whatever means) - set the
	 * correct bit for extended oplock reply.
	 */
	
	if (oplock_request && lp_fake_oplocks(SNUM(conn)))
		smb_action |= EXTENDED_OPLOCK_GRANTED;
	
	if(oplock_request && EXCLUSIVE_OPLOCK_TYPE(fsp->oplock_type))
		smb_action |= EXTENDED_OPLOCK_GRANTED;

	set_message(outbuf,34,0,True);
	
	p = outbuf + smb_vwv2;
	
	/*
	 * Currently as we don't support level II oplocks we just report
	 * exclusive & batch here.
	 */

    if (smb_action & EXTENDED_OPLOCK_GRANTED)   
		SCVAL(p,0, BATCH_OPLOCK_RETURN);
	else if (LEVEL_II_OPLOCK_TYPE(fsp->oplock_type))
	SCVAL(p,0, LEVEL_II_OPLOCK_RETURN);
	else
		SCVAL(p,0,NO_OPLOCK_RETURN);
	
	p++;
	SSVAL(p,0,fsp->fnum);
	p += 2;
	SIVAL(p,0,smb_action);
	p += 4;
	
	/* Create time. */  
	put_long_date(p,get_create_time(&sbuf,lp_fake_dir_create_times(SNUM(conn))));
	p += 8;
	put_long_date(p,sbuf.st_atime); /* access time */
	p += 8;
	put_long_date(p,sbuf.st_mtime); /* write time */
	p += 8;
	put_long_date(p,sbuf.st_mtime); /* change time */
	p += 8;
	SIVAL(p,0,fmode); /* File Attributes. */
	p += 4;
	SOFF_T(p, 0, file_len);
	p += 8;
	SOFF_T(p,0,file_len);
	p += 12;
	SCVAL(p,0,fsp->is_directory ? 1 : 0);
	
	DEBUG(5,("reply_ntcreate_and_X: fnum = %d, open name = %s\n", fsp->fnum, fsp->fsp_name));

	return chain_reply(inbuf,outbuf,length,bufsize, ProcSockID, threadid);
#endif
}                 
