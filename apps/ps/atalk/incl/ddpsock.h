#ifndef _DDPSOCK_H
#define _DDPSOCK_H

int so_ddp_bind(struct usock *up);
int so_ddp_recv(struct usock *up,struct ambuf **bpp,struct sockaddr *from,int *fromlen);
int so_ddp_send(struct usock *up,struct ambuf **bpp,struct sockaddr *to);
int so_ddp_close(struct usock *up);

#endif  _DDPSOCK_H
