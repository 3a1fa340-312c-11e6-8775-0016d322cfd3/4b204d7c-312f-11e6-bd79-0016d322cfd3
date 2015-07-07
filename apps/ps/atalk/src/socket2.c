#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "atalkd.h"
#include "apple_mbuf.h"
#include "socket2.h"
#include "ddpsock.h"

uint32 AT_SOCKET2_INIT = 0;

#define DEFNSOCK	100	/* Default number of sockets */
unsigned Nsock = DEFNSOCK;	// Number of socket entries, move from config.c	3/24/98
static struct usock **Usock;		/* Socket entry array */	//eCos
const char Inet_eol[] = "\r\n";
#define	_fd_seq(fd)	((fd) & 8191)
extern extern int send_pkt(int intno,uint8 *buffer,unsigned int length);

/* Socket-protocol interface table */
struct socklink Socklink[] = {
   /* type,
	* socket,		bind,			listen,			connect,
	* accept,		recv,			send,			qlen,
	* kick,			shut,			close,			check,
	* error,		state,			status,			eol_seq
	*/
	
	TYPE_DDP,
	NULL,			so_ddp_bind,	NULL,			NULL,
	NULL,			so_ddp_recv,	so_ddp_send,	NULL,
	NULL,	        NULL,	        so_ddp_close,	NULL,
	NULL,           NULL,           NULL,           Inet_eol,

	-1
};

///////////////////////////////////Apple MBuffer/////////////////////////////////////////////

uint32 pullup2(struct ambuf **bp,void *buf,uint32 cnt, char *buffer)
{
	struct ambuf *temp, *bph;
	int n;
	uint8 *obp = buf;
	
	if((bp == NULL) ||(*bp == NULL) || (cnt == 0))
		return 0;
		
	if ((cnt > AMbufSize) || ((*bp)->cnt > AMbufSize)){//615wu
		return 0;
	}
		
	bph = *bp;	
	n = min(cnt,bph->cnt);
	
	temp = buffer;
	memcpy(temp->data, bph->data, bph->cnt);
	temp->cnt = bph->cnt;
	
	temp->cnt -= n;
	
	if(obp != NULL){
		if(n == 1){	/* Common case optimization */
			*obp++ = *bph->data;
		} else if(n > 1){
			memcpy(obp,bph->data,n);
			obp += n;
		}
	}
	
	if (temp->cnt <= 0){
		free_p(&bph);
		*bp = NULL;
	}else{
		memcpy(bph->data, (uint32)(temp->data) + n, temp->cnt);
		bph->cnt = temp->cnt;
	}

	return n;
}

uint32 pushdown2(struct ambuf **bp,void *buf,uint32 size, char *buffer)
{
	struct ambuf *temp, *bpp;

	if(bp == NULL || size == 0)
		return 1;
//Jesse Modify 2008/02/18
	if ((size > AMbufSize) || ((*bp)->cnt > AMbufSize) || (((*bp)->cnt + size) > AMbufSize)){//615wu
		return 1;
	}

	if((bpp = *bp) != NULL){
		temp = buffer;
		memcpy(temp->data, bpp->data, bpp->cnt);	
		temp->cnt = bpp->cnt;
	
		if(buf != NULL){
		memcpy(bpp->data, buf, size);
		memcpy(((uint32)(bpp->data) + size) ,temp->data, temp->cnt);
		bpp->cnt = temp->cnt + size;
		}else
			return 1;
	
	}else{
		(*bp) = malloc(sizeof(struct ambuf));
		if (*bp == NULL)
			return 1;
			
		bpp = *bp;
		memset(bpp, 0x00, sizeof(struct ambuf));
		bpp->size = AMbufSize;
		bpp->cnt = 0;
		
		if(buf != NULL){
			memcpy(bpp->data, buf, size);
			bpp->cnt = size;
		}else
			return 1;
	}
	
	return 0;
}

/* Append packet to end of packet queue */
void enqueue2(struct ambuf **q,struct ambuf **bpp)
{
	struct ambuf *p;

	if(q == NULL || bpp == NULL || *bpp == NULL)
		return;
	
	if(*q == NULL){
		/* List is empty, stick at front */
		*q = *bpp;
	} else {
		for(p = *q ; p->anext != NULL ; p = p->anext)
			;
		p->anext = *bpp;
	}
	*bpp = NULL;	/* We've consumed it */		

}

/* Unlink a packet from the head of the queue */
struct ambuf * dequeue2(struct ambuf **q)
{
	struct ambuf *bp;

	if(q == NULL)
		return NULL;

	if((bp = *q) != NULL){
		*q = bp->anext;
		bp->anext = NULL;
	}
	
	return bp;
}

int trim_mbuf2(struct ambuf **bpp,uint length)
{
	uint tot = 0;

	if(bpp == NULL || *bpp == NULL)
		return -1;	/* Nothing to trim */

	if((length == 0) || (length > AMbufSize)){	//615wu
		/* Toss the whole thing */
		free_p(bpp);
		return -1;
	}
	/* Find the point at which to trim. If length is greater than
	 * the packet, we'll just fall through without doing anything
	 */
	if ( (*bpp)->cnt > length )
		(*bpp)->cnt = length;
}

/* Copy user data into an mbuf */
struct ambuf *
qdata(void *data,uint cnt)
{
	struct ambuf *bp;

	bp = malloc(sizeof(struct ambuf));
	if (bp == NULL)	//eCos
		return NULL;
	memset(bp, 0x00, sizeof(struct ambuf));
	memcpy(bp->data,data,cnt);
	bp->size = AMbufSize;
	bp->cnt = cnt;
	return bp;
}

void free_p(struct ambuf **buf)
{
	if(buf == NULL || (*buf) == NULL)
		return;

	free(*buf);
	*buf = NULL;
}

/* Free entire queue of packets (of mbufs) */
void free_q(struct ambuf **q)
{
	struct ambuf *bp;

	while((bp = dequeue2(q)) != NULL)
		free_p(&bp);
}
///////////////////////////////////Apple MBuffer END /////////////////////////////////////////////


/* Convert a socket index to an internal user socket structure pointer */
struct usock *
itop(s)
int s;	/* Socket index */
{
	if(s < 0)
		return NULL;	/* Valid only for sockets */
	if(s >= Nsock)
		return NULL;

	return Usock[s];
}


/********************************************************************/
/* optname : SO_RCV_BLOCK (DEFAULT), SO_RCV_NOBLOCK, SO_RCV_TIMEOUT */
/* now only for UPD socket		5/21/98								*/
/********************************************************************/
int setsocketopt2(int  s,char optname)
{
	struct usock *up;

	if((up = itop(s)) == NULL){
		return -1;
	}

	up->noblock = optname;

	return (0);
}

/* Higher level receive routine, intended for datagram sockets. Can also
 * be used for connection-oriented sockets, although from and fromlen are
 * ignored.
 */
int recvfrom2(s,buf,len,flags,from,fromlen)
int s;			/* Socket index */
void *buf;		/* User buffer */
int len;		/* Maximum length */
int flags;		/* Unused; will eventually select oob data, etc */
struct sockaddr *from;	/* Source address, only for datagrams */
int *fromlen;		/* Length of source address */
{
	struct ambuf *bp = NULL;
	int cnt;

	cnt = recv_mbuf2(s,&bp,flags,from,fromlen);
	if(cnt > 0){
		cnt = min(cnt,len);
		memcpy(buf, bp->data, cnt);		
			free_p(&bp); 
	}
	return cnt;
}

/* High level send routine, intended for datagram sockets. Can be used on
 * connection-oriented sockets, but "to" and "tolen" are ignored.
 */
int
sendto2(
int s,			/* Socket index */
void *buf,		/* User buffer */
int len,		/* Length of buffer */
int flags,		/* Unused; will eventually select oob data, etc */
struct sockaddr *to,	/* Destination, only for datagrams */
int tolen		/* Length of destination */
){
	struct ambuf *bp;

	bp = qdata(buf,(uint16)len);
	if (bp == NULL)	//eCos
		return -1;
	return send_mbuf2(s,&bp,flags,to,tolen);
}


/* Initialize user socket array */
void
sockinit2(void)
{
#ifdef SMALLSIZE
	if(Usock != (struct usock **)NULL)
		return;	/* Already initialized */
#endif
	Usock = (struct usock **)mallocw(Nsock * sizeof(struct usock *));
}

/* Create a user socket, return socket index
 * The mapping to actual protocols is as follows:
 *
 *
 * ADDRESS FAMILY	Stream		Datagram	Raw	    Seq. Packet
 *
 * AF_INET		TCP		UDP		IP
 * AF_AX25		I-frames	UI-frames
 * AF_NETROM						NET/ROM L3  NET/ROM L4
 * AF_LOCAL		stream loopback	packet loopback
 */
int
socket2(
int af,		/* Address family */
int type,	/* Stream or datagram */
int protocol	/* Used for raw IP sockets */
){
	struct usock *up;
	struct socklink *sp;
	int s;

	cyg_scheduler_lock();	//615wu

	if (!AT_SOCKET2_INIT){
		sockinit2();
		AT_SOCKET2_INIT = 1;
	}
	for(s=0;s<Nsock;s++)
	    if(Usock[s] == NULL)
			break;
	if(s == Nsock){
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	
	Usock[s] = up = (struct usock *)malloc(sizeof(struct usock));
	if (up == NULL){			//eCos
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	memset(up, 0x00, sizeof(struct usock));

	up->index = s;
	up->refcnt = 1;
	up->rdysock = -1;
	
	cyg_semaphore_init(&up->sem_f,0);	//615wu
	
	switch(af){

	case AF_APPLETALK:
		switch(type) {
		case SOCK_DGRAM:
			up->type = TYPE_DDP;
			break;
		default:
			break;
		}
		break;

	}
	/* Look for entry in protocol table */
	for(sp = Socklink;sp->type != -1;sp++){
		if(up->type == sp->type)
			break;
	}
	up->sp = sp;

	if(sp->type == -1 ){ //modified by Simon 11/20/97
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	cyg_scheduler_unlock();	//615wu
	return s;
}

/* Attach a local address/port to a socket. If not issued before a connect
 * or listen, will be issued automatically
 */
int
bind2(
int s,			/* Socket index */
struct sockaddr *name,	/* Local name */
int namelen		/* Length of name */
){
	struct usock *up;
	struct socklink *sp;

	cyg_scheduler_lock();	//615wu

	if((up = itop(s)) == NULL){
#ifdef __ERRNO_H
		errno = EBADF;
#endif
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	if(name == NULL){
#ifdef __ERRNO_H
		errno = EFAULT;
#endif
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	if(up->name != NULL){
		/* Bind has already been issued */
#ifdef __ERRNO_H
		errno = EINVAL;
#endif
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	sp = up->sp;
	if(sp->check != NULL && (*sp->check)(name,namelen) == -1){
		/* Incorrect length or family for chosen protocol */
#ifdef __ERRNO_H
		errno = EAFNOSUPPORT;
#endif
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	/* Stash name in an allocated block */
	up->namelen = namelen;
//eCos	up->name = malloc(namelen);
	up->name = up->name_buffer;	//eCos
	memcpy(up->name,name,namelen);

	/* a bind routine is optional - don't fail if it isn't present */
	if(sp->bind != NULL && (*sp->bind)(up) == -1){
#ifdef __ERRNO_H
		errno = EOPNOTSUPP;
#endif
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	cyg_scheduler_unlock();	//615wu	
	return 0;
}

/* Low-level receive routine. Passes mbuf back to user; more efficient than
 * higher-level functions recv() and recvfrom(). Datagram sockets ignore
 * the len parameter.
 */
int
recv_mbuf2(
int s,			/* Socket index */
struct ambuf **bpp,	/* Place to stash receive buffer */
int flags,		/* Unused; will control out-of-band data, etc */
struct sockaddr *from,		/* Peer address (only for datagrams) */
int *fromlen		/* Length of peer address */
){
	struct usock *up;
	struct socklink *sp;

	if((up = itop(s)) == NULL){
#ifdef __ERRNO_H
		errno = EBADF;
#endif
#ifdef PC_OUTPUT
		AtSaySpace(40,19,40);
		printf("\a \a \a(TCP/IP) Warning-1: RECV_BUF return(-1) !\a \a \a");
#endif
		return -1;
	}
	sp = up->sp;
	/* Fail if recv routine isn't present */
	if(sp == NULL || sp->recv == NULL){
#ifdef __ERRNO_H
		errno = EOPNOTSUPP;
#endif
#ifdef PC_OUTPUT
		AtSaySpace(40,19,40);
		printf("\a \a \a(TCP/IP) Warning-2: RECV_BUF return(-1) !\a \a \a");
#endif
		return -1;
	}
	return (*sp->recv)(up,bpp,from,fromlen);
}
/* Low level send routine; user supplies mbuf for transmission. More
 * efficient than send() or sendto(), the higher level interfaces.
 * The "to" and "tolen" parameters are ignored on connection-oriented
 * sockets.
 *
 * In case of error, bp is freed so the caller doesn't have to worry about it.
 */
int
send_mbuf2(
int s,			/* Socket index */
struct ambuf **bpp,	/* Buffer to send */
int flags,		/* not currently used */
struct sockaddr *to,		/* Destination, only for datagrams */
int tolen		/* Length of destination */
){
	struct usock *up;
	int cnt;
	struct socklink *sp;

	if((up = itop(s)) == NULL){
		free_p(bpp);
#ifdef __ERRNO_H
		errno = EBADF;
#endif
		return -1;
	}
	sp = up->sp;
	/* Fail if send routine isn't present (shouldn't happen) */
	if(sp->send == NULL){
		free_p(bpp);
		return -1;
	}
	/* If remote address is supplied, check it */
	if(to != NULL && (sp->check != NULL)
	 && (*sp->check)(to,tolen) == -1){
		free_p(bpp);
#ifdef __ERRNO_H
		errno = EAFNOSUPPORT;
#endif
		return -1;
	}
	/* The proto send routine is expected to free the buffer
	 * we pass it even if the send fails
	 */
	if((cnt = (*sp->send)(up,bpp,to)) == -1){
#ifdef __ERRNO_H
		errno = EOPNOTSUPP;
#endif		
		return -1;
	}
	return cnt;
}

/* Close a socket, freeing it for reuse. Try to do a graceful close on a
 * TCP socket, if possible
 */
int
close_s2(
int s		/* Socket index */
){
	struct usock *up;
	struct socklink *sp;

	cyg_scheduler_lock();	//615wu

	if((up = itop(s)) == NULL){
#ifdef __ERRNO_H
		errno = EBADF;
#endif
		cyg_scheduler_unlock();	//615wu
		return -1;
	}
	if(--up->refcnt > 0){
		cyg_scheduler_unlock();	//615wu
		return 0;	/* Others are still using it */
	}
	/* Call proto-specific close routine if there is one */
	if((sp = up->sp) != NULL && sp->close != NULL)
		(*sp->close)(up);

//eCos	free(up->name);		//up->name_buffer no need to free
	
	cyg_semaphore_destroy(&up->sem_f);	

	Usock[_fd_seq(up->index)] = NULL;
	free(up);
	cyg_scheduler_unlock();	//615wu
	return 0;
}
