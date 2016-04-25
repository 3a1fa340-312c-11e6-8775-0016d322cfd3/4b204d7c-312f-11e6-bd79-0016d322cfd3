#ifndef _ATALKD_H
#define _ATALKD_H

#include "at.h"
#include "ddp.h"
#include "phase2.h"
#include "rtmp.h"
#include "zip.h"
#include "nbp.h"
#include "aep.h"

extern cyg_sem_t ATD_INIT_OK;
#define bcopy(x,y,z)  memcpy((y),(x),(z))
#define mtod(m,t) ((t)((m)->data))	//615wu

//atalkd.c
extern struct AT_IFACE at_iface;
//void atalkd_init(void);
void atalkd_init(cyg_addrword_t data);

//aarp.c
void aarp_output(uint16 aarp_op, struct at_addr *addr);
uint16 aarpresolve(struct ambuf *bp,	struct at_addr *sat_addr, uint8 *dest_enaddr);
void aarp_input(struct ambuf **bpp);
void aarp_probe(void);
int16 probe_last_addr(void);

//ddp.c
struct ddp_cb * open_ddp(uint8 atport);
int send_ddp(struct at_socket *lsocket, struct at_socket *fsocket, struct ambuf **bpp);
int recv_ddp(struct ddp_cb *up, struct at_socket *fsocket,struct ambuf **bp);
int del_ddp(struct ddp_cb *conn);
void ddp_input(struct ambuf **bpp);
int ddp_output(struct ambuf *bp,struct at_addr *dest);
uint8 GetATport(void);

//ddpsock.c
int ddp_select(int s);

#endif  _ATALKD_H
