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
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"




#ifdef WITH_SSL
#include <ssl.h>
#undef Realloc  /* SSLeay defines this and samba has a function of this name */
extern SSL  *ssl;
extern int  sslFd;
#endif  /* WITH_SSL */

/* the client file descriptor */
//int Client[NUM_SMB_SOCKID];  //6/29/2001

/* the last IP received from */
struct in_addr lastip;

/* the last port received from */
int lastport=0;

int smb_read_error[NUM_SMBTHREAD] = {0};
/****************************************************************************
 Determine if a file descriptor is in fact a socket.
****************************************************************************/

int is_a_socket(int fd)
{
//0609  int v,l;
//0609  l = sizeof(int);
  return 1;  
}

unsigned long inet_address(unsigned long addr)
{
    return htonl(addr);
}

enum SOCK_OPT_TYPES {OPT_BOOL,OPT_INT,OPT_ON};

/*
struct
{
  char *name;
  int level;
  int option;
  int value;
  int opttype;
} socket_options[10];
//}socket_options[]
// = {
//  {"SO_KEEPALIVE",      SOL_SOCKET,    SO_KEEPALIVE,    0,                 OPT_BOOL},
//  {"SO_REUSEADDR",      SOL_SOCKET,    SO_REUSEADDR,    0,                 OPT_BOOL},
//  {"SO_BROADCAST",      SOL_SOCKET,    SO_BROADCAST,    0,                 OPT_BOOL},
#ifdef TCP_NODELAY
//  {"TCP_NODELAY",       IPPROTO_TCP,   TCP_NODELAY,     0,                 OPT_BOOL},
#endif
#ifdef IPTOS_LOWDELAY
//  {"IPTOS_LOWDELAY",    IPPROTO_IP,    IP_TOS,          IPTOS_LOWDELAY,    OPT_ON},
#endif
#ifdef IPTOS_THROUGHPUT
//  {"IPTOS_THROUGHPUT",  IPPROTO_IP,    IP_TOS,          IPTOS_THROUGHPUT,  OPT_ON},
#endif
#ifdef SO_REUSEPORT
//  {"SO_REUSEPORT",      SOL_SOCKET,    SO_REUSEPORT,    0,                 OPT_BOOL},
#endif
#ifdef SO_SNDBUF
//  {"SO_SNDBUF",         SOL_SOCKET,    SO_SNDBUF,       0,                 OPT_INT},
#endif
#ifdef SO_RCVBUF
//  {"SO_RCVBUF",         SOL_SOCKET,    SO_RCVBUF,       0,                 OPT_INT},
#endif
#ifdef SO_SNDLOWAT
//  {"SO_SNDLOWAT",       SOL_SOCKET,    SO_SNDLOWAT,     0,                 OPT_INT},
#endif
#ifdef SO_RCVLOWAT
//  {"SO_RCVLOWAT",       SOL_SOCKET,    SO_RCVLOWAT,     0,                 OPT_INT},
#endif
#ifdef SO_SNDTIMEO
//  {"SO_SNDTIMEO",       SOL_SOCKET,    SO_SNDTIMEO,     0,                 OPT_INT},
#endif
#ifdef SO_RCVTIMEO
//  {"SO_RCVTIMEO",       SOL_SOCKET,    SO_RCVTIMEO,     0,                 OPT_INT},
#endif
//  {NULL,0,0,0,0}};
*/

/****************************************************************************
 Read from a socket.
****************************************************************************/

int read_udp_socket(int fd,char *buf,uint32 len)
{
	struct sockaddr_in sock;
	int ret;
	int socklen;
  
	socklen = sizeof(sock);
	memset((char *)&sock,'\0',socklen);
	memset((char *)&lastip,'\0',sizeof(lastip));

	ret = recvfrom(fd,buf,len,0,(struct sockaddr *)&sock,&socklen);

	if (ret <= 0) {
		// DEBUG(2,("read socket failed. ERRNO=%s\n",strerror(errno)));
		return(0);
	}

	lastip.s_addr = inet_address(sock.sin_addr.s_addr);
	lastport = htons (sock.sin_port);

	//  DEBUG(10,("read_udp_socket: lastip %s lastport %d read: %d\n",
	//        inet_ntoa(lastip), lastport, ret));

	return(ret);
}

/****************************************************************************
 Read data from a socket with a timout in msec.
 mincount = if timeout, minimum to read before returning
 maxcount = number to be read.
 time_out = timeout in milliseconds
****************************************************************************/
#if 0 //0716
static int read_socket_with_timeout(int fd,char *buf,uint32 mincnt
			,uint32 maxcnt,unsigned int time_out, int threadid)
{
//  fd_set fds;
  int fds;
  int selrtn;
  int readret;
  uint32 nread = 0;
  struct timeval timeout;

  /* just checking .... */
  if (maxcnt <= 0)
    return(0);

  smb_read_error[threadid] = 0;

  /* Blocking read */
  if (time_out <= 0) {
    if (mincnt == 0) mincnt = maxcnt;

    while (nread < mincnt) {
#ifdef WITH_SSL
      if(fd == sslFd){
        readret = SSL_read(ssl, buf + nread, maxcnt - nread);
      }else{
        readret = recv(fd, buf + nread, maxcnt - nread, 0);
      }
#else /* WITH_SSL */
      readret = recv(fd, buf + nread, maxcnt - nread, 0);
#endif /* WITH_SSL */

      if (readret == 0) {
//        DEBUG(5,("read_socket_with_timeout: blocking read. EOF from client.\n"));
        smb_read_error[threadid] = READ_EOF;
        return -1;
      }

      if (readret == -1) {
//        DEBUG(0,("read_socket_with_timeout: read error = %s.\n", strerror(errno) ));
        smb_read_error[threadid] = READ_ERROR;
        return -1;
      }
      nread += readret;
    }
    return((int)nread);
  }
  
  /* Most difficult - timeout read */
  /* If this is ever called on a disk file and 
     mincnt is greater then the filesize then
     system performance will suffer severely as 
     select always returns true on disk files */

  /* Set initial timeout */
  timeout.tv_sec = (time_t)(time_out / 1000);
  timeout.tv_usec = (long)(1000 * (time_out % 1000));

  for (nread=0; nread < mincnt; ) {      
    /* Check if error */
    if(selrtn == -1) {
      /* something is wrong. Maybe the socket is dead? */
      smb_read_error[threadid] = READ_ERROR;
      return -1;
    }

    /* Did we timeout ? */
    if (selrtn == 0) {
      smb_read_error[threadid] = READ_TIMEOUT;
      return -1;
    }
      
#ifdef WITH_SSL
    if(fd == sslFd){
      readret = SSL_read(ssl, buf + nread, maxcnt - nread);
    }else{
      readret = recv(fd, buf + nread, maxcnt - nread, 0);
    }
#else /* WITH_SSL */
    readret = recv(fd, buf+nread, maxcnt-nread, 0);
#endif /* WITH_SSL */

    if (readret == 0) {
      /* we got EOF on the file descriptor */
      smb_read_error[threadid] = READ_EOF;
      return -1;
    }

    if (readret == -1) {
      /* the descriptor is probably dead */
      smb_read_error[threadid] = READ_ERROR;
      return -1;
    }
      
    nread += readret;
  }

  /* Return the number we got */
  return((int)nread);
}
#endif //0
/****************************************************************************
 Read data from a fd with a timout in msec.
 mincount = if timeout, minimum to read before returning
 maxcount = number to be read.
 time_out = timeout in milliseconds
****************************************************************************/
#if 0 //0424
int read_with_timeout(int fd,char *buf,uint32 mincnt,uint32 maxcnt,unsigned int time_out)
{
//  fd_set fds;
  int fds;
  int selrtn;
  int readret;
  uint32 nread = 0;
  struct timeval timeout;

  /* just checking .... */
  if (maxcnt <= 0)
    return(0);

  /* Blocking read */
  if (time_out <= 0) {
    if (mincnt == 0) mincnt = maxcnt;

    while (nread < mincnt) {
#ifdef WITH_SSL
      if(fd == sslFd){
        readret = SSL_read(ssl, buf + nread, maxcnt - nread);
      }else{
        readret = recv(fd, buf + nread, maxcnt - nread, 0);
      }
#else /* WITH_SSL */
      readret = recv(fd, buf + nread, maxcnt - nread, 0);
#endif /* WITH_SSL */

      if (readret <= 0)
	return readret;

      nread += readret;
    }
    return((int)nread);
  }
  
  /* Most difficult - timeout read */
  /* If this is ever called on a disk file and 
     mincnt is greater then the filesize then
     system performance will suffer severely as 
     select always returns true on disk files */

  /* Set initial timeout */
  timeout.tv_sec = (time_t)(time_out / 1000);
  timeout.tv_usec = (long)(1000 * (time_out % 1000));

  for (nread=0; nread < mincnt; ) {      
//0305    FD_ZERO(&fds);
//0305    FD_SET(fd,&fds);
      
//0305    selrtn = sys_select(fd+1,&fds,&timeout);
//0305    selrtn = select(fd+1,&fds,&timeout);    

    if(selrtn <= 0)
      return selrtn;
      
#ifdef WITH_SSL
    if(fd == sslFd){
      readret = SSL_read(ssl, buf + nread, maxcnt - nread);
    }else{
      readret = recv(fd, buf + nread, maxcnt - nread, 0);
    }
#else /* WITH_SSL */
    readret = recv(fd, buf+nread, maxcnt-nread, 0);
#endif /* WITH_SSL */

    if (readret <= 0)
      return readret;

    nread += readret;
  }

  /* Return the number we got */
  return((int)nread);
}
#endif //0
/****************************************************************************
send a keepalive packet (rfc1002)
****************************************************************************/
#if 0 //0419
int send_keepalive(int client)
{
  unsigned char buf[4];

  buf[0] = 0x85;
  buf[1] = buf[2] = buf[3] = 0;

  return(write_socket_data(client,(char *)buf,4) == 4);
}
#endif //0
/****************************************************************************
  read data from the client, reading exactly N bytes. 
****************************************************************************/
#if 0 //0424
int read_data(int fd,char *buffer,uint32 N)
{
  int  ret;
  uint32 total=0;  
 
  smb_read_error[threadid] = 0;

  while (total < N)
  {
#ifdef WITH_SSL
    if(fd == sslFd){
      ret = SSL_read(ssl, buffer + total, N - total);
    }else{
      ret = recv(fd,buffer + total,N - total,0);
    }
#else /* WITH_SSL */
    ret = recv(fd,buffer + total,N - total,0);
#endif /* WITH_SSL */

    if (ret == 0)
    {
//      DEBUG(10,("read_data: read of %d returned 0. Error = %s\n", (int)(N - total), strerror(errno) ));
      smb_read_error[threadid] = READ_EOF;
      return 0;
    }
    if (ret == -1)
    {
//      DEBUG(0,("read_data: read failure for %d. Error = %s\n", (int)(N - total), strerror(errno) ));
      smb_read_error[threadid] = READ_ERROR;
      return -1;
    }
    total += ret;
  }
  return ((int)total);
}
#endif //0
/****************************************************************************
 Read data from a socket, reading exactly N bytes. 
****************************************************************************/

static int read_socket_data(int fd,char *buffer,uint32 N, int threadid)
{
  int ret=0;
  int total=0;
  int i=0;  
 
  smb_read_error[threadid] = 0;

  while (total < N)
  {
//os	kalarm(30000L); //7/17/2001 by ron
    ret = recv(fd,buffer + total, N - total, 0);
//os	kalarm(0L);

    if (ret == 0)
    {
//      DEBUG(10,("read_socket_data: recv of %d returned 0. Error = %s\n", (int)(N - total), strerror(errno) ));
	    smb_read_error[threadid] = READ_EOF;
	    return 0;
    }
    if (ret == -1)
    {
//      DEBUG(0,("read_socket_data: recv failure for %d. Error = %s\n", (int)(N - total), strerror(errno) ));
     	smb_read_error[threadid] = READ_ERROR;
      	return -1;
    }
    total += ret;
  }
  return total;
}

/****************************************************************************
 Write data to a fd.
****************************************************************************/
#if 0 //0424
int write_data(int fd,char *buffer,uint32 N)
{
  uint32 total=0;
  int ret;

  while (total < N)
  {
#ifdef WITH_SSL
    if(fd == sslFd){
      ret = SSL_write(ssl,buffer + total,N - total);
    }else{
      ret = send(fd,buffer + total,N - total, 0);
    }
#else /* WITH_SSL */
    ret = send(fd,buffer + total,N - total, 0);
#endif /* WITH_SSL */

    if (ret == -1) {
//      DEBUG(0,("write_data: write failure. Error = %s\n", strerror(errno) ));
      return -1;
    }
    if (ret == 0) return total;

    total += ret;
  }
  return (int)total;
}
#endif //0
/****************************************************************************
 Write data to a socket - use send rather than write.
****************************************************************************/

int write_socket_data(int fd,char *buffer,uint32 N)
{
  uint32 total=0;
  int ret;

  while (total < N)
  {
#ifdef WITH_SSL
    if(fd == sslFd){
      ret = SSL_write(ssl,buffer + total,N - total);
    }else{
      ret = send(fd,buffer + total,N - total, 0);
    }
#else /* WITH_SSL */
    ret = send(fd,buffer + total,N - total,0);
#endif /* WITH_SSL */

    if (ret == -1) {
//      DEBUG(0,("write_socket_data: write failure. Error = %s\n", strerror(errno) ));
      return -1;
    }
    if (ret == 0) return total;

    total += ret;
  }
  return (int)total;
}

/****************************************************************************
write to a socket
****************************************************************************/

int write_socket(int fd,char *buf,uint32 len)
{
  int ret=0;

  ret = write_socket_data(fd,buf,len);
      
  if(ret <= 0) ;
//    DEBUG(0,("write_socket: Error writing %d bytes to socket %d: ERRNO = %s\n", 
//       (int)len, fd, strerror(errno) ));

  return(ret);
}

/****************************************************************************
read 4 bytes of a smb packet and return the smb length of the packet
store the result in the buffer
This version of the function will return a length of zero on receiving
a keepalive packet.
timeout is in milliseconds.
****************************************************************************/

static int read_smb_length_return_keepalive(int fd,char *inbuf,
				unsigned int timeout, int threadid)
{
  int len=0;
  int msg_type;
  int ok = False;

  while (!ok)
  {
      ok = (read_socket_data(fd,inbuf,4, threadid) == 4);

    if (!ok) 
      return(-1);

    len = smb_len(inbuf);
    msg_type = CVAL(inbuf,0);

    if (msg_type == 0x85) ;
//      DEBUG(5,("Got keepalive packet\n"));
  }

//  DEBUG(10,("got smb length of %d\n",len));

  return(len);
}

/****************************************************************************
read 4 bytes of a smb packet and return the smb length of the packet
store the result in the buffer. This version of the function will
never return a session keepalive (length of zero).
timeout is in milliseconds.
****************************************************************************/

int read_smb_length(int fd,char *inbuf,unsigned int timeout, int threadid)
{
  int len;

  for(;;)
  {
    len = read_smb_length_return_keepalive(fd, inbuf, timeout, threadid);

    if(len < 0)
      return len;

    /* Ignore session keepalives. */
    if(CVAL(inbuf,0) != 0x85)
      break;
  }

//  DEBUG(10,("read_smb_length: got smb length of %d\n",len));

  return len;
}

/****************************************************************************
  read an smb from a fd. Note that the buffer *MUST* be of size
  SMB_BUFFER_SIZE+SAFETY_MARGIN.
  The timeout is in milliseconds. 
  This function will return on a
  receipt of a session keepalive packet.
****************************************************************************/

int receive_smb(int fd,char *buffer, unsigned int timeout, int threadid)
{
  int len=0,ret=0;
  int recvlen= 0;

  smb_read_error[threadid] = 0;

  memset(buffer,'\0',smb_size + 100);

  len = read_smb_length_return_keepalive(fd,buffer,timeout, threadid);
  if (len < 0)
  {
//    DEBUG(10,("receive_smb: length < 0!\n"));
    return(False);
  }

//  if (len > SMB_BUFFER_SIZE) {
//    DEBUG(0,("Invalid packet length! (%d bytes).\n",len));
//		return (False);
//0305	exit(1);
//  }

  if(len > 0) {
    ret = read_socket_data(fd,buffer+4, recvlen = min(len, SMB_BUFFER_SIZE), threadid);
    if (ret != recvlen) {
      smb_read_error[threadid] = READ_ERROR;
      return False;
    }
  }
  return(True);
}

/****************************************************************************
  send an smb to a fd 
****************************************************************************/

int send_smb(int fd,char *buffer)
{
  uint32 len;
  uint32 nwritten=0;
  int ret;
  len = smb_len(buffer) + 4;

  while (nwritten < len)
  {
    ret = write_socket(fd,buffer+nwritten,len - nwritten);
    if (ret <= 0)
    {
//      DEBUG(0,("Error writing %d bytes to client. %d. Exiting\n",(int)len,(int)ret));
//0305      close_sockets();
//0305      exit(1);
    }
    nwritten += ret;
  }

  return True;
}

/****************************************************************************
send a single packet to a port on another machine
****************************************************************************/
#if 0 //0419
int send_one_packet(char *buf,int len,struct in_addr ip,int port,int type)
{
  int ret;
  int out_fd;
  struct sockaddr_in sock_out;

  if (passive)
    return(True);

  /* create a socket to write to */
  out_fd = socket(AF_INET, type, 0);
  if (out_fd == -1) 
    {
//      DEBUG(0,("socket failed"));
      return False;
    }

  /* set the address and port */
  memset((char *)&sock_out,'\0',sizeof(sock_out));
  putip((char *)&sock_out.sin_addr,(char *)&ip);
//0302  sock_out.sin_port = htons( port );
	sock_out.sin_port =  port ;  
  sock_out.sin_family = AF_INET;
  
//  if (DEBUGLEVEL > 0)
//    DEBUG(3,("sending a packet of len %d to (%s) on port %d of type %s\n",
//	     len,inet_ntoa(ip),port,type==SOCK_DGRAM?"DGRAM":"STREAM"));
	
  /* send it */
  ret = (sendto(out_fd,buf,len,0,(struct sockaddr *)&sock_out,sizeof(sock_out)) >= 0);

  if (!ret)
//    DEBUG(0,("Packet send to %s(%d) failed ERRNO=%s\n",
//	     inet_ntoa(ip),port,strerror(errno)));

//0305  close(out_fd);
  return(ret);
}
#endif //0
/****************************************************************************
open a socket of the specified type, port and address for incoming data
****************************************************************************/

int open_socket_in(int type, int port, int dlevel,uint32 socket_addr, int rebind)
{
	struct sockaddr_in sock;
	int res;
  
	memset((char *)&sock,'\0',sizeof(sock));

	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = htonl (socket_addr);
	sock.sin_port = htons(port);

	res = socket(AF_INET, type, 0);

//ZOTIPS	armond_printf("open socket cnt = %d \n",res);

	if(res==-1)
		return -1;
/*	
	{
		int val=1;
		if(rebind)
			val=1;
		else
			val=0;
		setsockopt(res,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
			
		#ifdef SO_REUSEPORT
		setsockopt(res,SOL_SOCKET,SO_REUSEPORT,&val,sizeof(val));
			
		#endif // SO_REUSEPORT 
	  }	
*/
	/* now we've got a socket - we need to bind it */
	if (bind(res, (struct sockaddr * ) &sock,sizeof(sock)) < 0) 
    { 
		
		if (port)
		{
			if (port == SMB_PORT || port == NMB_PORT)
			{
				//	DEBUG(dlevel,("bind failed on port %d socket_addr=%s (%s)\n",
				//   	port,inet_ntoa(sock.sin_addr),strerror(errno))); 
			}
			close(res);

			if (dlevel > 0 && port < 1000)
				port = 7999;

			if (port >= 1000 && port < 9000)
				return(open_socket_in(type,port+1,dlevel,socket_addr,rebind));
		}
		
//Jesse		close_s(res);
//		close(res);
		return(-1); 
    }
	return res;
}

/****************************************************************************
  create an outgoing socket. timeout is in milliseconds.
  **************************************************************************/
#if 0 //0424
int open_socket_out(int type, struct in_addr *addr, int port ,int timeout)
{
  struct sockaddr_in sock_out;
  int res,ret;
  int connect_loop = 250; /* 250 milliseconds */
  int loops = (timeout) / connect_loop;

  /* create a socket to write to */
//  res = socket(PF_INET, type, 0);
  res = socket(0, type, 0);
  if (res == -1) ;
//    { DEBUG(0,("socket error\n")); return -1; }

  if (type != SOCK_STREAM) return(res);
  
  memset((char *)&sock_out,'\0',sizeof(sock_out));
  putip((char *)&sock_out.sin_addr,(char *)addr);
  
//0302  sock_out.sin_port = htons( port );
  sock_out.sin_port =  port ;  
//  sock_out.sin_family = PF_INET;
  sock_out.sin_family = 0;

  /* and connect it to the destination */
  connect_again:
  ret = connect(res,(struct sockaddr *)&sock_out,sizeof(sock_out));


#ifdef EISCONN
  if (ret < 0 && errno == EISCONN) {
    errno = 0;
    ret = 0;
  }
#endif

  if (ret < 0) {
//    DEBUG(1,("error connecting to %s:%d (%s)\n",
//	     inet_ntoa(*addr),port,strerror(errno)));
//0305    close(res);
    return -1;
  }

  /* set it blocking again */
//0305  set_blocking(res,True);

  return res;
}
#endif //0

/*******************************************************************
 Reset the 'done' variables so after a client process is created
 from a fork call these calls will be re-done. This should be
 expanded if more variables need reseting.
 ******************************************************************/


