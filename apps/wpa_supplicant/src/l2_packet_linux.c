/*
 * WPA Supplicant - Layer2 packet handling with Linux packet sockets
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
//eason 20100407	#include <sys/ioctl.h>
//eason 20100407	#include <netpacket/packet.h>
//eason 20100407	#include <net/if.h>

#include "common.h"
#include "eloop.h"
#include "l2_packet.h"
#include "netif/iface.h"	//eason 20100407
#define IFNAMSIZ	16

typedef unsigned char        UINT8;	//eason 20100407
extern UINT8   mvEDDMAC[6];	//eason 20100407

struct l2_packet_data {
	int fd; /* packet socket for EAPOL frames */
	char ifname[IFNAMSIZ + 1];
	int ifindex;
	u8 own_addr[ETH_ALEN];
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len);
	void *rx_callback_ctx;
	int l2_hdr; /* whether to include layer 2 (Ethernet) header data
		     * buffers */
};


int l2_packet_get_own_addr(struct l2_packet_data *l2, u8 *addr)
{
	os_memcpy(addr, l2->own_addr, ETH_ALEN);
	return 0;
}


unsigned char eap_send_buf[2300]={0};	//eason 20100407
int l2_packet_send(struct l2_packet_data *l2, const u8 *dst_addr, u16 proto,
		   const u8 *buf, size_t len)
{
	int ret = 0;
	u16 protocol = htons(proto);	//eason 20100407
	
	if (l2 == NULL)
		return -1;

#if 0 //eason 20100407		
	if (l2->l2_hdr) {
		ret = send(l2->fd, buf, len, 0);
		if (ret < 0)
			perror("l2_packet_send - send");
	} else {
		struct sockaddr_ll ll;
		os_memset(&ll, 0, sizeof(ll));
		ll.sll_family = AF_PACKET;
		ll.sll_ifindex = l2->ifindex;
		ll.sll_protocol = htons(proto);
		ll.sll_halen = ETH_ALEN;
		os_memcpy(ll.sll_addr, dst_addr, ETH_ALEN);
		ret = sendto(l2->fd, buf, len, 0, (struct sockaddr1 *) &ll,
			     sizeof(ll));
		if (ret < 0)
			perror("l2_packet_send - sendto");
	}
#else
	if (l2->l2_hdr) {
	//eason 20100715	if (send_pkt(0, buf, len) < 0)
		if (send_eap_pkt(0, buf, len) < 0)	
		{
			return -1;
		}
	} else {
		memset(eap_send_buf, 0, 2300);
		memcpy(eap_send_buf, dst_addr, 6);
		memcpy(&eap_send_buf[6], mvEDDMAC, 6);
		memcpy(&eap_send_buf[12], &protocol, 2);
		memcpy(&eap_send_buf[14], buf, len);
	//eason 20100715	if (send_pkt(0, eap_send_buf, len+14) < 0)
		if (send_eap_pkt(0, eap_send_buf, len+14) < 0)
		{
			return -1;
		}
	}	
#endif //eason 20100407	
	return ret;
}

struct EAP_mbuf
{
	struct EAP_mbuf *next;	/* Links packets on queues */
	u8 *data;
	unsigned int len;
};

u8 eap_recv_buf[2300]={0};
extern struct EAP_mbuf *EapQueue;	//eason 20100407
static void l2_packet_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct l2_packet_data *l2 = eloop_ctx;
//ZOT	u8 buf[2300];
	int res;
	u8 s_addr[6];	//eason 20100407
	struct EAP_mbuf *bp;	//eason 20100407
	struct ether *ethHeader;	//eason 20100407	
	int i_state;	//ZOT	
//eason 20100407	struct sockaddr_ll ll;
//eason 20100407	socklen_t fromlen;
#if 0 //eason 20100407
	os_memset(&ll, 0, sizeof(ll));
	fromlen = sizeof(ll);
	res = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr1 *) &ll,
		       &fromlen);
	if (res < 0) {
		perror("l2_packet_receive - recvfrom");
		return;
	}
	l2->rx_callback(l2->rx_callback_ctx, ll.sll_addr, buf, res);
#else
	i_state = dirps();
	bp = dequeue(&EapQueue);
	if (bp == NULL){
		restore(i_state);	
		return;
	}
	restore(i_state);	
//ZOT	
	memset(eap_recv_buf, 0, 2300);
	memcpy(eap_recv_buf, bp->data, bp->len);
	res = bp->len;	

	ethHeader = (struct ether *)eap_recv_buf;
	memcpy(s_addr, ethHeader->source, 6);
	
	l2->rx_callback(l2->rx_callback_ctx, s_addr, &eap_recv_buf[sizeof(struct ether)], res - sizeof(struct ether));
	free(bp->data);
	free(bp);
#endif //eason 20100407	
}


struct l2_packet_data * l2_packet_init(
	const char *ifname, const u8 *own_addr, unsigned short protocol,
	void (*rx_callback)(void *ctx, const u8 *src_addr,
			    const u8 *buf, size_t len),
	void *rx_callback_ctx, int l2_hdr)
{
	struct l2_packet_data *l2;
//eason 20100407	struct ifreq ifr;
//eason 20100407	struct sockaddr_ll ll;

	l2 = os_zalloc(sizeof(struct l2_packet_data));
	if (l2 == NULL)
		return NULL;
	memset(l2, 0, sizeof(*l2));	
	os_strlcpy(l2->ifname, ifname, sizeof(l2->ifname));
	l2->rx_callback = rx_callback;
	l2->rx_callback_ctx = rx_callback_ctx;
	l2->l2_hdr = l2_hdr;
#if 0 //eason 20100407
	l2->fd = socket(PF_PACKET, l2_hdr ? SOCK_RAW : SOCK_DGRAM,
			htons(protocol));
	if (l2->fd < 0) {
		perror("socket(PF_PACKET)");
		os_free(l2);
		return NULL;
	}
	os_memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, l2->ifname, sizeof(ifr.ifr_name));
	if (ioctl(l2->fd, SIOCGIFINDEX, &ifr) < 0) {
		perror("ioctl[SIOCGIFINDEX]");
		close(l2->fd);
		os_free(l2);
		return NULL;
	}
	l2->ifindex = ifr.ifr_ifindex;

	os_memset(&ll, 0, sizeof(ll));
	ll.sll_family = PF_PACKET;
	ll.sll_ifindex = ifr.ifr_ifindex;
	ll.sll_protocol = htons(protocol);
	if (bind(l2->fd, (struct sockaddr1 *) &ll, sizeof(ll)) < 0) {
		perror("bind[PF_PACKET]");
		close(l2->fd);
		os_free(l2);
		return NULL;
	}

	if (ioctl(l2->fd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("ioctl[SIOCGIFHWADDR]");
		close(l2->fd);
		os_free(l2);
		return NULL;
	}
	os_memcpy(l2->own_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);	
#else
	l2->fd = 1;	
	l2->ifindex = 1;	
	memcpy(l2->own_addr, mvEDDMAC, ETH_ALEN);	
#endif	//eason 20100407
	eloop_register_read_sock(l2->fd, l2_packet_receive, l2, NULL);
	return l2;
}


void l2_packet_deinit(struct l2_packet_data *l2)
{
	if (l2 == NULL)
		return;

	if (l2->fd >= 0) {
		eloop_unregister_read_sock(l2->fd);
		close(l2->fd);
	}
		
	os_free(l2);
}

#if 0 //eason 20100407
int l2_packet_get_ip_addr(struct l2_packet_data *l2, char *buf, size_t len)
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	size_t res;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	os_memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, l2->ifname, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			perror("ioctl[SIOCGIFADDR]");
		close(s);
		return -1;
	}
	close(s);
	saddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (saddr->sin_family != AF_INET)
		return -1;
	res = os_strlcpy(buf, inet_ntoa(saddr->sin_addr), len);
	if (res >= len)
		return -1;
	return 0;
}
#endif //eason 20100407

void l2_packet_notify_auth_start(struct l2_packet_data *l2)
{
}
