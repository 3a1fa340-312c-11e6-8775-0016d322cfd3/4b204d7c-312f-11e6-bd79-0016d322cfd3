
#ifndef CYGONCE_NETWORK_H
#define CYGONCE_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

/* For Ethernet Header */
#define HW_ADDR_LEN				6
#define ETH_HDR_LEN             14
#define ETH_IP_TYPE             0x800
#define ETH_ARP_TYPE            0x806
#define ETH_EAP_TYPE            0x888E

#define IS_BROADCAST_HWADDR(x) ( (*(unsigned long *)(x + 2) == 0xffffffff)  \
                                         && (*(unsigned short *)x == 0xffff) )

#ifdef __cplusplus
}
#endif

#endif // #ifndef CYGONCE_NETWORK_H
