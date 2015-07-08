/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Samba utility functions
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

#include "prnport.h"
#include "prnqueue.h"

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"

extern struct Tempdata *Gtempdata[];
extern int preEndoffset[NUM_SMBTHREAD];


#if (defined(HAVE_NETGROUP) && defined (WITH_AUTOMOUNT))
#ifdef WITH_NISPLUS_HOME
#ifdef BROKEN_NISPLUS_INCLUDE_FILES
/*
 * The following lines are needed due to buggy include files
 * in Solaris 2.6 which define GROUP in both /usr/include/sys/acl.h and
 * also in /usr/include/rpcsvc/nis.h. The definitions conflict. JRA.
 * Also GROUP_OBJ is defined as 0x4 in /usr/include/sys/acl.h and as
 * an enum in /usr/include/rpcsvc/nis.h.
 */

#if defined(GROUP)
#undef GROUP
#endif

#if defined(GROUP_OBJ)
#undef GROUP_OBJ
#endif

#endif /* BROKEN_NISPLUS_INCLUDE_FILES */

#include <rpcsvc/nis.h>

#else /* !WITH_NISPLUS_HOME */

#include "rpcsvc/ypclnt.h"

#endif /* WITH_NISPLUS_HOME */
#endif /* HAVE_NETGROUP && WITH_AUTOMOUNT */

#ifdef WITH_SSL
#include <ssl.h>
#undef Realloc  /* SSLeay defines this and samba has a function of this name */
extern SSL  *ssl;
extern int  sslFd;
#endif  /* WITH_SSL */

int Protocol[NUM_SMBTHREAD];

/* a default finfo structure to ensure all fields are sensible */
//file_info def_finfo = {-1,0,0,0,0,0,0,""};

/* the client file descriptor */
//extern int Client;

/* this is used by the chaining code */
int chain_size[NUM_SMBTHREAD] = {0};

 fstring remote_machine="";
// fstring local_machine="";
 pstring sesssetup_user="";
 fstring global_myname = "";
// fstring global_myworkgroup = "";


//copy from util_str.c 0604/2001 by ron
/****************************************************************************
substitute a string for a pattern in another string. Make sure there is 
enough room!

This routine looks for pattern in s and replaces it with 
insert. It may do multiple replacements.

any of " ; ' $ or ` in the insert string are replaced with _
if len==0 then no length check is performed

Return True if a change was made, False otherwise.
****************************************************************************/
int string_sub(char *s,const char *pattern,const char *insert, uint32 len)
{
	int changed = False;
	char *p;
	int ls,lp,li, i;

	if (!insert || !pattern || !s) return False;

	ls = (int)strlen(s);
	lp = (int)strlen(pattern);
	li = (int)strlen(insert);

	if (!*pattern) return False;
	
	while (lp <= ls && (p = strstr(s,pattern))) {
		changed = True;
		if (len && (ls + (li-lp) >= len)) {
//			DEBUG(0,("ERROR: string overflow by %d in string_sub(%.50s, %d)\n", 
//				 (int)(ls + (li-lp) - len),
//				 pattern, (int)len));
			break;
		}
		if (li != lp) {
			memmove(p+li,p+lp,strlen(p+lp)+1);
		}
		for (i=0;i<li;i++) {
			switch (insert[i]) {
			case '`':
			case '"':
			case '\'':
			case ';':
			case '$':
			case '%':
			case '\r':
			case '\n':
				p[i] = '_';
				break;
			default:
				p[i] = insert[i];
			}
		}
		s = p + li;
		ls += (li-lp);
	}

	return changed;
}



/*******************************************************************
  set the length and marker of an smb packet
********************************************************************/
void smb_setlen(char *buf,int len)
{
  _smb_setlen(buf,len);

  CVAL(buf,4) = 0xFF;
  CVAL(buf,5) = 'S';
  CVAL(buf,6) = 'M';
  CVAL(buf,7) = 'B';
}

/*******************************************************************
  setup the word count and byte count for a smb message
********************************************************************/
int set_message(char *buf,int num_words,int num_bytes,int zero)
{
  if (zero)
    memset(buf + smb_size,'\0',num_words*2 + num_bytes);
  CVAL(buf,smb_wct) = num_words;
  SSVAL(buf,smb_vwv + num_words*SIZEOFWORD,num_bytes);  
  smb_setlen(buf,smb_size + num_words*2 + num_bytes - 4);
  return (smb_size + num_words*2 + num_bytes);
}

/****************************************************************************
return the total storage length of a mangled name
****************************************************************************/
int name_len(char *s1)
{
	/* NOTE: this argument _must_ be unsigned */
	unsigned char *s = (unsigned char *)s1;
	int len;

	/* If the two high bits of the byte are set, return 2. */
	if (0xC0 == (*s & 0xC0))
		return(2);

	/* Add up the length bytes. */
	for (len = 1; (*s); s += (*s) + 1) {
		len += *s + 1;
	}

	return(len);
} /* name_len */

/****************************************************************************
interpret a protocol description string, with a default
****************************************************************************/
#if 0 //0716
int interpret_protocol(char *str,int def)
{
  if (!strcmp(str,"NT1"))
    return(PROTOCOL_NT1);
  if (!strcmp(str,"LANMAN2"))
    return(PROTOCOL_LANMAN2);
  if (!strcmp(str,"LANMAN1"))
    return(PROTOCOL_LANMAN1);
  if (!strcmp(str,"CORE"))
    return(PROTOCOL_CORE);
  if (!strcmp(str,"COREPLUS"))
    return(PROTOCOL_COREPLUS);
  if (!strcmp(str,"CORE+"))
    return(PROTOCOL_COREPLUS);
  
  return(def);
}
#endif //0
/*******************************************************************
  check if an IP is the 0.0.0.0
  ******************************************************************/
int zero_ip(struct in_addr ip)
{
  uint32 a;
  putip((char *)&a,(char *)&ip);
  return(a == 0);
}

/*******************************************************************
are two IPs on the same subnet?
********************************************************************/
int same_net(struct in_addr ip1,struct in_addr ip2,struct in_addr mask)
{
  uint32 net1,net2,nmask;

  nmask = ntohl(mask.s_addr);
  net1  = ntohl(ip1.s_addr);
  net2  = ntohl(ip2.s_addr);
  nmask = (mask.s_addr);
  net1  = (ip1.s_addr);
  net2  = (ip2.s_addr);
            
  return((net1 & nmask) == (net2 & nmask));
}
/****************************************************************************
expand a pointer to be a particular size
****************************************************************************/
void *Realloc(void *p,uint32 size)
{
  	void *ret=NULL;

  	if (size == 0) {
    	if (p) free(p);
//    DEBUG(5,("Realloc asked for 0 bytes\n"));
    	return NULL;
  	}

  	if (!p)
    	ret = (void *)mallocw(size);
  	else
    	ret = (void *)realloc(p,size);

//0718#ifdef MEM_MAN
//  {
//	extern FILE *dbf;
//	smb_mem_write_info(ret, dbf);
//  }
//#endif

  	if (!ret){
//    DEBUG(0,("Memory allocation error: failed to expand to %d bytes\n",(int)size));
  	}
  	return(ret);
}

/*******************************************************************
is the name specified one of my netbios names
returns true is it is equal, false otherwise
********************************************************************/
int is_myname(char *s)
{
  	int n;
  	int ret = False;

  	for (n=0; my_netbios_names[n]; n++) {
    	if (!strcmp(my_netbios_names[n], s))
      	ret=True;
  	}
//  DEBUG(8, ("is_myname(\"%s\") returns %d\n", s, ret));
  	return(ret);
}

// copy from error.c 1/21/2002 
/****************************************************************************
  create an error packet. Normally called using the ERROR() macro
****************************************************************************/
int error_packet(char *inbuf,char *outbuf,int error_class,uint32 error_code) //line just for debug
{
  	int outsize = set_message(outbuf,0,0,True);
  	int cmd = CVAL(inbuf,smb_com);
  	int flgs2 = SVAL(outbuf,smb_flg2); 

// when negotiate action, flags2 set 32-bit error codes, It only need error codes. By Ron 3/8/2002
  	if ((flgs2 & FLAGS2_32_BIT_ERROR_CODES) == FLAGS2_32_BIT_ERROR_CODES) 
  	{
    	SIVAL(outbuf,smb_rcls,error_code);
    
//0110    DEBUG( 3, ( "32 bit error packet at line %d cmd=%d (%s) eclass=%08x [%s]\n",
//0110              line, cmd, smb_fn_name(cmd), error_code, smb_errstr(outbuf) ) );
  	}
  	else
  	{
    	CVAL(outbuf,smb_rcls) = error_class;
    	SSVAL(outbuf,smb_err,error_code);  
//    DEBUG( 3, ( "error packet at line %d cmd=%d (%s) eclass=%d ecode=%d\n",
//	      line,
//	      (int)CVAL(inbuf,smb_com),
//	      smb_fn_name(CVAL(inbuf,smb_com)),
//	      error_class,
//	      error_code ) );

  	}
  
//  if (errno != 0)
//    DEBUG(3,("error string = %s\n",strerror(errno)));
  
  	return(outsize);
}

/* To Clear the mbuf used in recv(), then the next SMB packet will be received correctly.  Ron Add 2/25/2002 */ 
int ClearRecvBuf(int SocketId, char *revbuf, int ntotalclr, int bufsize)
{
	int ntotalrecv =0;
	int ret;
	
	while (ntotalrecv < ntotalclr ){	
		//os    kalarm(30000L); 
		ret = recv(SocketId, revbuf, min(ntotalclr - ntotalrecv, bufsize),0);      
		//os    kalarm(0L);
		if (ret == 0 )
			break;  
		if (ret == -1 )
		//Jesse      return;		
			return -1;		
		ntotalrecv += ret;	
	}
	if (ntotalrecv == ntotalclr)
		return ntotalrecv;
	else 
		return 0;	
}
/* Ron Add 2/25/2002 */
int printingerror(int SMBStatus, char *outbuf)
{
	switch (SMBStatus){
		case PORT_PAPER_OUT:
	    	CVAL(outbuf,smb_rcls) = ERRHRD;
        	SSVAL(outbuf,smb_err,ERRnopaper); 
			return 1;
	  
		case PORT_OFF_LINE:
		case PORT_NO_CONNECT:
	  		CVAL(outbuf,smb_rcls) = ERRHRD;
      		SSVAL(outbuf,smb_err,ERRnotready);  
			return 1;
	
		case PORT_FAULT:
			CVAL(outbuf,smb_rcls) = ERRHRD;
      		SSVAL(outbuf,smb_err,ERRwrite);  	
			return 1;	
  	}
  	return 0;	
}

int TempPrintingData(int SockID, char *Srcdata, int threadid, int ret, int numtocpy, uint8 ispaperout, int treeid)
{
	struct Tempdata *tempdata =NULL;
	struct Tempdata *dataptr= NULL, *next =NULL;
	int ntoread =0;
	char *data;
	
	tempdata = (struct Tempdata *)mallocw(sizeof(struct Tempdata));
    memset(tempdata, 0 ,sizeof(struct Tempdata));
	tempdata->datasize = numtocpy;
	tempdata->paperout = ispaperout;
	tempdata->treeid = treeid;
	tempdata->next = NULL;   //new *tempdata must be linked on the last 	
	if ((tempdata->data = malloc(tempdata->datasize)) == NULL)
	  return 0;
	data = tempdata->data;  
	memcpy(data, Srcdata ,ret);
	ntoread += ret;
	data += ret;			
	while ( ntoread < numtocpy){
//os	  kalarm(30000L); 
      ret = recv(SockID, data, numtocpy - ntoread ,0);      
//os	  kalarm(0L);
      if (ret == 0 )
		return 0;		
      if (ret == -1 )
        return 0;		
	  ntoread += ret;	
	  data += ret;	 	
	}		 
	for (dataptr = Gtempdata[threadid]; dataptr ;dataptr = next){
	  if ((next = dataptr->next) == NULL)
	  	break;	  
	}	
	if (dataptr == NULL)  // there is no temp data, then this *tempdata will be the first one
	  Gtempdata[threadid] = tempdata;
	else { 
      dataptr->next =tempdata;
    }  
	return 1;		
}	  	

int CopytempbuftoPrnQueue(struct prnbuf *smbprnbuf, int ProcSockID, char *inbuf, int threadid
                       ,int ret, int preEndoffset, uint32 Startoffset, uint32 numtowrite, int treeid)
{  	
	int ncopy =0;
	char *data = smbprnbuf->data;
	char *prndata = smb_buf(inbuf) + 3;
	struct Tempdata *next = NULL;

	if (Startoffset + numtowrite <= preEndoffset)  // I don't need this data since Offset not more than data I temp
    	ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);       	  
	else{ // the data is new, must temp
    	if (!TempPrintingData(ProcSockID, prndata, threadid, ret, numtowrite, 0, treeid))    
      		return 0;  // memory not enough or receive error	  		
  	}	
  	while (ncopy + Gtempdata[threadid]->datasize < BLOCKSIZE){	
    	memcpy (data, Gtempdata[threadid]->data, Gtempdata[threadid]->datasize);
    	ncopy += Gtempdata[threadid]->datasize;
   		data += Gtempdata[threadid]->datasize;
    
    	free(Gtempdata[threadid]->data);
    	next = Gtempdata[threadid]->next;
    	memset(Gtempdata[threadid], 0 , sizeof(struct Tempdata));
    	free(Gtempdata[threadid]);
    	if (next != NULL)
     		Gtempdata[threadid] = next;
    	else {
      		Gtempdata[threadid] = NULL;
      		break;  	
    	}	     	    
  	}  
 	smbprnbuf->size = ncopy;
  
  	return 1;	
}	  

void SMBPaperoutprocess(char *outbuf, char *inbuf, int OS, uint32 Startoffset, int ProcSockID
                                   , char *data, int ret, uint32 numtowrite, int threadid, int treeid )
{  	             			
 			
	CVAL(outbuf,smb_rcls) = ERRHRD;
	SSVAL(outbuf,smb_err,ERRnopaper);       	
  	
  	if (OS == Windows98){
    	if ( Startoffset == preEndoffset[threadid] || preEndoffset[threadid] == -1){ // if needed temp data     		
	  		if (!TempPrintingData(ProcSockID, data, threadid, ret, numtowrite, 1, treeid)){      		
    			CVAL(outbuf,smb_rcls) = ERRHRD;
        		SSVAL(outbuf,smb_err,ERRwrite);  	      		
      		}
      		return ;               	  
    	} 
    	else{  
      		ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);
      		return ;         
    	}  	
	} 
  	else if (OS == Windows2000){
    	ClearRecvBuf(ProcSockID, inbuf, numtowrite - ret, SMB_BUFFER_SIZE);
    	return ;         
  	}  	  
}
