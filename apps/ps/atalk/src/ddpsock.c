#include <cyg/kernel/kapi.h>
#include "network.h"
#include "psglobal.h"
#include "apple_mbuf.h"
#include "socket2.h"
#include "atalkd.h"
#include "ddpsock.h"

int so_ddp_bind(struct usock *up)
{
	int s;
	struct sockaddr_at	*sp;

	s = up->index;
	sp = (struct sockaddr_at *)up->name;
	if(sp->sat_port == ATADDR_ANYNODE) sp->sat_port = GetATport();
	up->cb.ddp = open_ddp(sp->sat_port);
	if (up->cb.ddp == NULL)		//eCos
		return -1;
	up->cb.ddp->user = s;

	return 0;
}

int so_ddp_recv(
	struct usock *up,	
	struct ambuf **bpp,	
	struct sockaddr *from,
	int *fromlen
){
	int cnt;
	struct ddp_cb *ddp;
	struct sockaddr_at *remote;
	struct at_socket fsocket;
	int ErrTmp;

	while((ddp = up->cb.ddp) != NULL && (cnt = recv_ddp(ddp,&fsocket,bpp)) == -1)
	{			
		if(up->noblock == SO_RCV_NOBLOCK){
#ifdef __ERRNO_H
			errno = EWOULDBLOCK;
#endif
			return -1;
		} else {	
			if(up->noblock == SO_RCV_TIMEOUT){
				cyg_semaphore_init(&up->sem_f,0);
				ErrTmp = (int)cyg_semaphore_timed_wait(&up->sem_f,cyg_current_time() + ((DDP_RECV_TIME_OUT*1000L) / MSPTICK));
			}
			else{
				cyg_semaphore_init(&up->sem_f,0);
				cyg_semaphore_wait(&up->sem_f);
				ErrTmp = 1;
			}
			if(ErrTmp == 0)
				return -1;		
		}
	}
	if(ddp == NULL){
		// Connection went away
#ifdef __ERRNO_H
		errno = ENOTCONN;
#endif
		return -1;
	}

	if(from != NULL && fromlen != (int *)NULL && *fromlen >= sizeof(struct sockaddr_at)){
		remote = (struct sockaddr_at *)from;
		remote->sat_family = AF_APPLETALK;
		remote->sat_port = fsocket.port;
		remote->sat_addr.s_net  = fsocket.addr.s_net;
		remote->sat_addr.s_node = fsocket.addr.s_node;
		*fromlen = sizeof(struct sockaddr_at);
	}

	return cnt;
}

int so_ddp_send(
	struct usock *up,	
	struct ambuf **bpp,
	struct sockaddr *to
){
	struct sockaddr_at *local,*remote;
	struct at_socket lsock,fsock;

	if(up->name == NULL) {
		free_p(bpp);
#ifdef __ERRNO_H
		errno = ENOTCONN;
#endif
		return -1;
	}

	local = (struct sockaddr_at *)up->name;

	if(to != NULL) remote = (struct sockaddr_at *)to;
	else {
		free_p(bpp);			
#ifdef __ERRNO_H
		errno = ENOTCONN;
#endif
		return -1;
	}

	lsock.addr.s_net  =	local->sat_addr.s_net;
	lsock.addr.s_node =	local->sat_addr.s_node;
	lsock.port = local->sat_port;

	fsock.addr.s_net  =	remote->sat_addr.s_net;
	fsock.addr.s_node =	remote->sat_addr.s_node;
	fsock.port = remote->sat_port;

	return send_ddp(&lsock,&fsock,bpp);
//	return 0;
}

int so_ddp_close(struct usock *up)
{
	if(up->cb.ddp != NULL){
		del_ddp(up->cb.ddp);
	}
	return 0;
}

//int ddp_select(int s, uint16 timer)
int ddp_select(int s)
{
	struct usock *up;
	struct ddp_cb *ddp;

	if((up = itop(s)) == NULL || (ddp = up->cb.ddp) == NULL){
		return (-1);
	}

//	if(ddp->rcvcnt == 0 && timer) {
//		kalarm(timer*1000L);
//		kwait(up);
//		kalarm(0L);
//	}
	return (ddp->rcvcnt);
}
