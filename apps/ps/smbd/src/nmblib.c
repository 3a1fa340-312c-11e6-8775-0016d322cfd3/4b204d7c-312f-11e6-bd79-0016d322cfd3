/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   NBT netbios library routines
   Copyright (C) Andrew Tridgell 1994-1998
   
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


#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"

#include "btorder.h"

static char pkt_buf[768];

#define QSORT_CAST (int (*)(const void *, const void *))
//extern qsort(char *base,int num,int size,int  (*cmp) ());

//form utilsock.c
extern int read_udp_socket(int fd,char *buf,uint32 len);

//form psmain.c
extern uint32 msclock();
extern char * strupr ( char * string );

/*******************************************************************
  handle "compressed" name pointers
  ******************************************************************/
static int handle_name_ptrs(unsigned char *ubuf,int *offset,int length,
			     int *got_pointer,int *ret)
{
	int loop_count=0;
  
	while ((ubuf[*offset] & 0xC0) == 0xC0) {
		if (!*got_pointer) (*ret) += 2;
		(*got_pointer)=TRUE;
		(*offset) = ((ubuf[*offset] & ~0xC0)<<8) | ubuf[(*offset)+1];

		if (loop_count++ == 10 || (*offset) < 0 || (*offset)>(length-2)) {
			return(FALSE);
		}
	}
	return(TRUE);
}

/*******************************************************************
  parse a nmb name from "compressed" format to something readable
  return the space taken by the name, or 0 if the name is invalid
  ******************************************************************/
static int parse_nmb_name(char *inbuf,int offset,int length, struct nmb_name *name)
{
	unsigned char *ubuf = (unsigned char *)inbuf;
	int loop_count=0;
	int m,n=0;
	int ret = 0;
	int got_pointer=FALSE;

	if (length - offset < 2)
		return(0);  

	/* handle initial name pointers */
	if (!handle_name_ptrs(ubuf,&offset,length,&got_pointer,&ret))
		return(0);
  
	m = ubuf[offset];

	if (!m)
		return(0);
	if ((m & 0xC0) || offset+m+2 > length)
		return(0);

//2/8/2002	memset((char *)name,'\0',sizeof(*name));
	memset((char *)name,'\0',sizeof(struct nmb_name));

	/* the "compressed" part */
	if (!got_pointer)
		ret += m + 2;
	offset++;
	while (m > 0) {
		unsigned char c1,c2;
		c1 = ubuf[offset++]-'A';
		c2 = ubuf[offset++]-'A';
		if ((c1 & 0xF0) || (c2 & 0xF0) || (n > sizeof(name->name)-1))
			return(0);
		name->name[n++] = (c1<<4) | c2;
		m -= 2;
	}
	name->name[n] = 0;

	if (n==16) {
    	/* parse out the name type, 
       	   its always in the 16th byte of the name */
		name->name_type = ((unsigned char)name->name[15]) & 0xff;
  
		/* remove trailing spaces */
		name->name[15] = 0;
		n = 14;
		while (n && name->name[n]==' ')
			name->name[n--] = 0;  
	}

	/* now the domain parts (if any) */
	n = 0;
	while (ubuf[offset]) {
		/* we can have pointers within the domain part as well */
		if (!handle_name_ptrs(ubuf,&offset,length,&got_pointer,&ret))
			return(0);

		m = ubuf[offset];
		/*
		 * Don't allow null domain parts.
		 */
		if (!m)
			return(0);
		if (!got_pointer)
			ret += m+1;
		if (n)
			name->scope[n++] = '.';
		if (m+2+offset>length || n+m+1>sizeof(name->scope))
			return(0);
		offset++;
		while (m--)
			name->scope[n++] = (char)ubuf[offset++];

		/*
		 * Watch for malicious loops.
		 */
		if (loop_count++ == 10)
			return 0;
	}
	name->scope[n++] = 0;  

	return(ret);
}


/*******************************************************************
  put a compressed nmb name into a buffer. return the length of the
  compressed name

  compressed names are really weird. The "compression" doubles the
  size. The idea is that it also means that compressed names conform
  to the doman name system. See RFC1002.
  ******************************************************************/
static int put_nmb_name(char *buf,int offset,struct nmb_name *name)
{
	char buf1[128];
	char *p;
	int ret,m;

	if (strcmp(name->name,"*") == 0) {
    	/* special case for wildcard name */
		memset(buf1,'\0',20);
		buf1[0] = '*';
		buf1[15] = name->name_type;
	} else {
		sprintf(buf1, "%-15.15s%c",name->name,name->name_type);
	}

	buf[offset] = 0x20;

	ret = 34;

	for (m=0;m<16;m++) {
		buf[offset+1+2*m] = 'A' + ((buf1[m]>>4)&0xF);
		buf[offset+2+2*m] = 'A' + (buf1[m]&0xF);
	}
	offset += 33;

	buf[offset] = 0;

	if (name->scope[0]) {
    	/* XXXX this scope handling needs testing */
    	ret += strlen(name->scope) + 1;
    	pstrcpy(&buf[offset+1],name->scope);  
  
    	p = &buf[offset+1];
    	while ((p = strchr(p,'.'))) {
      		buf[offset] = p - &buf[offset+1];
      		offset += (buf[offset] + 1);
      		p = &buf[offset+1];
    	}
		buf[offset] = strlen(&buf[offset+1]);
  	}

	return(ret);
}

/*******************************************************************
  put a resource record into a packet
  ******************************************************************/
static int put_res_rec(char *buf,int offset,struct res_rec *recs,int count)
{
	int ret=0;
	int i;

	for (i=0;i<count;i++) {
		int l = put_nmb_name(buf,offset,&recs[i].rr_name);
		offset += l;
		ret += l;
		RSSVAL(buf,offset,recs[i].rr_type);
		RSSVAL(buf,offset+2,recs[i].rr_class);
		RSIVAL(buf,offset+4,recs[i].ttl);
		RSSVAL(buf,offset+8,recs[i].rdlength);
		memcpy(buf+offset+10,recs[i].rdata,recs[i].rdlength);
		offset += 10+recs[i].rdlength;
		ret += 10+recs[i].rdlength;
	}

	return(ret);
}

/*******************************************************************
  put a compressed name pointer record into a packet
  ******************************************************************/
static int put_compressed_name_ptr(unsigned char *buf,int offset,struct res_rec *rec,int ptr_offset)
{  
	int ret=0;
	buf[offset] = (0xC0 | ((ptr_offset >> 8) & 0xFF));
	buf[offset+1] = (ptr_offset & 0xFF);
	offset += 2;
	ret += 2;
	RSSVAL(buf,offset,rec->rr_type);
	RSSVAL(buf,offset+2,rec->rr_class);
	RSIVAL(buf,offset+4,rec->ttl);
	RSSVAL(buf,offset+8,rec->rdlength);
	memcpy(buf+offset+10,rec->rdata,rec->rdlength);
	offset += 10+rec->rdlength;
	ret += 10+rec->rdlength;
    
	return(ret);
}

/*******************************************************************
  parse a dgram packet. Return False if the packet can't be parsed 
  or is invalid for some reason, True otherwise 

  this is documented in section 4.4.1 of RFC1002
  ******************************************************************/
static int parse_dgram(char *inbuf,int length,struct dgram_packet *dgram)
{
	int offset;
	int flags;

//2/8/2002	memset((char *)dgram,'\0',sizeof(*dgram));
	memset((char *)dgram,'\0',sizeof(struct dgram_packet));	

	if (length < 14) return(FALSE);

	dgram->header.msg_type = CVAL(inbuf,0);
	flags = CVAL(inbuf,1);
	dgram->header.flags.node_type = (enum node_type)((flags>>2)&3);
	if (flags & 1) dgram->header.flags.more = TRUE;
	if (flags & 2) dgram->header.flags.first = TRUE;
	dgram->header.dgm_id = RSVAL(inbuf,2);
	putip((char *)&dgram->header.source_ip,inbuf+4);
	dgram->header.source_port = RSVAL(inbuf,8);
	dgram->header.dgm_length = RSVAL(inbuf,10);
	dgram->header.packet_offset = RSVAL(inbuf,12);

	offset = 14;

	if (dgram->header.msg_type == 0x10 ||
		dgram->header.msg_type == 0x11 ||
		dgram->header.msg_type == 0x12) {      
		offset += parse_nmb_name(inbuf,offset,length,&dgram->source_name);
		offset += parse_nmb_name(inbuf,offset,length,&dgram->dest_name);
	}

	if (offset >= length || (length-offset > sizeof(dgram->data))) 
		return(FALSE);

	dgram->datasize = length-offset;
	memcpy(dgram->data,inbuf+offset,dgram->datasize);

	return(TRUE);
}

/*******************************************************************
  parse a nmb packet. Return False if the packet can't be parsed 
  or is invalid for some reason, True otherwise 
  ******************************************************************/
static int parse_nmb(char *inbuf,int length,struct nmb_packet *nmb)
{
	int nm_flags,offset;

	memset((char *)nmb,'\0',sizeof(struct nmb_packet));

	if (length < 12) return(FALSE);

	/* parse the header */
	nmb->header.name_trn_id = RSVAL(inbuf,0);

	nmb->header.opcode = (CVAL(inbuf,2) >> 3) & 0xF;
	nmb->header.response = ((CVAL(inbuf,2)>>7)&1)?TRUE:FALSE;
	nm_flags = ((CVAL(inbuf,2) & 0x7) << 4) + (CVAL(inbuf,3)>>4);
	nmb->header.nm_flags.bcast = (nm_flags&1)?TRUE:FALSE;
	nmb->header.nm_flags.recursion_available = (nm_flags&8)?TRUE:FALSE;
	nmb->header.nm_flags.recursion_desired = (nm_flags&0x10)?TRUE:FALSE;
	nmb->header.nm_flags.trunc = (nm_flags&0x20)?TRUE:FALSE;
	nmb->header.nm_flags.authoritative = (nm_flags&0x40)?TRUE:FALSE;  
	nmb->header.rcode = CVAL(inbuf,3) & 0xF;
	nmb->header.qdcount = RSVAL(inbuf,4);
	nmb->header.ancount = RSVAL(inbuf,6);
	nmb->header.nscount = RSVAL(inbuf,8);
	nmb->header.arcount = RSVAL(inbuf,10);
  
	if (nmb->header.qdcount) {
		offset = parse_nmb_name(inbuf,12,length,&nmb->question.question_name);
		if (!offset) return(FALSE);

		if (length - (12+offset) < 4) return(FALSE);
		nmb->question.question_type = RSVAL(inbuf,12+offset);
		nmb->question.question_class = RSVAL(inbuf,12+offset+2);

		offset += 12+4;
	} else {
		offset = 12;
	}
	return(TRUE);
}

 
/*******************************************************************
  free up any resources associated with an nmb packet
  ******************************************************************/
static void free_nmb_packet(struct nmb_packet *nmb)
{  
	if (nmb->answers) {
		free(nmb->answers);
		nmb->answers = NULL;
	}
	if (nmb->nsrecs) {
		free(nmb->nsrecs);
		nmb->nsrecs = NULL;
	}
	if (nmb->additional) {
		free(nmb->additional);
		nmb->additional = NULL;
	}
}

/*******************************************************************
  free up any resources associated with a dgram packet
  ******************************************************************/
static void free_dgram_packet(struct dgram_packet *nmb)
{  
  /* We have nothing to do for a dgram packet. */
}

/*******************************************************************
  free up any resources associated with a packet
  ******************************************************************/
void free_packet(struct packet_struct *packet)
{  
	if (packet->locked) 
		return;
	if (packet->packet_type == NMB_PACKET)
		free_nmb_packet(&packet->packet.nmb);
	else if (packet->packet_type == DGRAM_PACKET)
		free_dgram_packet(&packet->packet.dgram);
//2/8/2002	ZERO_STRUCTPN(packet);
	memset((char *)packet, 0 , sizeof(struct packet_struct));	

	free(packet);
}

/*******************************************************************
  read a packet from a socket and parse it, returning a packet ready
  to be used or put on the queue. This assumes a UDP socket
  ******************************************************************/
struct packet_struct *read_packet(int fd,enum packet_type packet_type)
{
	struct packet_struct *packet;
	int length;
	int ok=FALSE;
  
	length = read_udp_socket(fd,pkt_buf,sizeof(pkt_buf));
	if (length < MIN_DGRAM_SIZE) return(NULL);

	packet = (struct packet_struct *)malloc(sizeof(struct packet_struct));
	if (!packet) return(NULL);

	packet->next = NULL;
	packet->prev = NULL;
	packet->ip = lastip;
	packet->port = lastport;
	packet->fd = fd;
	packet->locked = FALSE;
	packet->timestamp = (msclock()/10);
	packet->packet_type = packet_type;
	switch (packet_type) 
    {
    case NMB_PACKET:
		ok = parse_nmb(pkt_buf,length,&packet->packet.nmb);
		break;

    case DGRAM_PACKET:
		ok = parse_dgram(pkt_buf,length,&packet->packet.dgram);
		break;
    }
	if (!ok){
		free_packet(packet);
		return(NULL);
	}

	return(packet);
}
					 

/*******************************************************************
  send a udp packet on a already open socket
  ******************************************************************/
static int nsend_udp(int fd,char *buf,int len,struct in_addr ip,int port)
{
	struct sockaddr_in sock_out;
	int ret;

	/* set the address and port */
	memset((char *)&sock_out,'\0',sizeof(sock_out));
//Jesse	putip((char *)&sock_out.sin_addr,(char *)&ip);
	sock_out.sin_addr.s_addr = htonl(ip.s_addr);
	sock_out.sin_family = AF_INET;
	sock_out.sin_port =  htons(port);
  	
	ret = (sendto(fd,buf,len,0,(struct sockaddr *)&sock_out,
			sizeof(sock_out)) >= 0);
	return(ret);
}
/*******************************************************************
  build a dgram packet ready for sending

  XXXX This currently doesn't handle packets too big for one
  datagram. It should split them and use the packet_offset, more and
  first flags to handle the fragmentation. Yuck.
  ******************************************************************/
static int build_dgram(char *buf,struct packet_struct *p)
{
  struct dgram_packet *dgram = &p->packet.dgram;
  unsigned char *ubuf = (unsigned char *)buf;
  int offset=0;

  /* put in the header */
  ubuf[0] = dgram->header.msg_type;
  ubuf[1] = (((int)dgram->header.flags.node_type)<<2);
  if (dgram->header.flags.more) ubuf[1] |= 1;
  if (dgram->header.flags.first) ubuf[1] |= 2;
  RSSVAL(ubuf,2,dgram->header.dgm_id);
  putip(ubuf+4,(char *)&dgram->header.source_ip);
  RSSVAL(ubuf,8,dgram->header.source_port);
  RSSVAL(ubuf,12,dgram->header.packet_offset);

  offset = 14;

  if (dgram->header.msg_type == 0x10 ||
      dgram->header.msg_type == 0x11 ||
      dgram->header.msg_type == 0x12) {      
    offset += put_nmb_name((char *)ubuf,offset,&dgram->source_name);
    offset += put_nmb_name((char *)ubuf,offset,&dgram->dest_name);
  }

  memcpy(ubuf+offset,dgram->data,dgram->datasize);
  offset += dgram->datasize;

  /* automatically set the dgm_length */
  dgram->header.dgm_length = offset;
  RSSVAL(ubuf,10,dgram->header.dgm_length); 

  return(offset);
}

/*******************************************************************
  build a nmb name
 *******************************************************************/
void make_nmb_name( struct nmb_name *n, const char *name, int type )
{
	memset( (char *)n, '\0', sizeof(struct nmb_name) );
	strncpy( n->name, name, 15 );
	strupr( n->name );
	n->name_type = (unsigned int)type & 0xFF;
}

/*******************************************************************
  Compare two nmb names
  ******************************************************************/

int nmb_name_equal(struct nmb_name *n1, struct nmb_name *n2)
{
	return ((n1->name_type == n2->name_type) &&
         !strcmp(n1->name ,n2->name ) &&
         !strcmp(n1->scope,n2->scope));
}

/*******************************************************************
  build a nmb packet ready for sending

  XXXX this currently relies on not being passed something that expands
  to a packet too big for the buffer. Eventually this should be
  changed to set the trunc bit so the receiver can request the rest
  via tcp (when that becomes supported)
  ******************************************************************/
static int build_nmb(char *buf,struct packet_struct *p)
{
	struct nmb_packet *nmb = &p->packet.nmb;
	unsigned char *ubuf = (unsigned char *)buf;
	int offset=0;

	/* put in the header */
	RSSVAL(ubuf,offset,nmb->header.name_trn_id);
	ubuf[offset+2] = (nmb->header.opcode & 0xF) << 3;
	if (nmb->header.response) ubuf[offset+2] |= (1<<7);
	if (nmb->header.nm_flags.authoritative && 
		nmb->header.response) ubuf[offset+2] |= 0x4;
	if (nmb->header.nm_flags.trunc) ubuf[offset+2] |= 0x2;
	if (nmb->header.nm_flags.recursion_desired) ubuf[offset+2] |= 0x1;
	if (nmb->header.nm_flags.recursion_available &&
		nmb->header.response) ubuf[offset+3] |= 0x80;
	if (nmb->header.nm_flags.bcast) ubuf[offset+3] |= 0x10;
	ubuf[offset+3] |= (nmb->header.rcode & 0xF);

	RSSVAL(ubuf,offset+4,nmb->header.qdcount);
	RSSVAL(ubuf,offset+6,nmb->header.ancount);
	RSSVAL(ubuf,offset+8,nmb->header.nscount);
	RSSVAL(ubuf,offset+10,nmb->header.arcount);
  
	offset += 12;
	if (nmb->header.qdcount) {
		/* XXXX this doesn't handle a qdcount of > 1 */
		offset += put_nmb_name((char *)ubuf,offset,&nmb->question.question_name);
    	RSSVAL(ubuf,offset,nmb->question.question_type);
    	RSSVAL(ubuf,offset+2,nmb->question.question_class);
    	offset += 4;
	}

	if (nmb->header.ancount)
	{
    	offset += put_res_rec((char *)ubuf,offset,nmb->answers,
				nmb->header.ancount);
	}

	if (nmb->header.nscount)
	{
		offset += put_res_rec((char *)ubuf,offset,nmb->nsrecs,
			  nmb->header.nscount);
	}

	/*
	 * The spec says we must put compressed name pointers
	 * in the following outgoing packets :
	 * NAME_REGISTRATION_REQUEST, NAME_REFRESH_REQUEST,
	 * NAME_RELEASE_REQUEST.
	 */

	if((nmb->header.response == FALSE) &&
		((nmb->header.opcode == NMB_NAME_REG_OPCODE) ||
		(nmb->header.opcode == NMB_NAME_RELEASE_OPCODE) ||
		(nmb->header.opcode == NMB_NAME_REFRESH_OPCODE_8) ||
		(nmb->header.opcode == NMB_NAME_REFRESH_OPCODE_9) ||
		(nmb->header.opcode == NMB_NAME_MULTIHOMED_REG_OPCODE)) &&
		(nmb->header.arcount == 1)) {

		offset += put_compressed_name_ptr(ubuf,offset,nmb->additional,12);

	} else if (nmb->header.arcount) {
		offset += put_res_rec((char *)ubuf,offset,nmb->additional,
			  nmb->header.arcount);
	}
	return(offset);
}


/*******************************************************************
  send a packet_struct
  ******************************************************************/
int send_packet(struct packet_struct *p)
{
	int len=0;

	//memset(pkt_buf,'\0',sizeof(buf));

	switch (p->packet_type) 
	{
	case NMB_PACKET:
		len = build_nmb(pkt_buf,p);
		break;

	case DGRAM_PACKET:
		len = build_dgram(pkt_buf,p);
		break;
	}

	if (!len) return(FALSE);

	return(nsend_udp(p->fd,pkt_buf,len,p->ip,p->port));
}

/****************************************************************************
return the number of bits that match between two 4 character buffers
  ***************************************************************************/
static int matching_bits(uchar *p1, uchar *p2)
{
	int i, j, ret = 0;
	for (i=0; i<4; i++) {
		if (p1[i] != p2[i]) break;
		ret += 8;
	}

	if (i==4) return ret;

	for (j=0; j<8; j++) {
		if ((p1[i] & (1<<(7-j))) != (p2[i] & (1<<(7-j)))) break;
		ret++;
	}	
	
	return ret;
}


static uchar sort_ip[4];

/****************************************************************************
compare two query reply records
  ***************************************************************************/
static int name_query_comp(uchar *p1, uchar *p2)
{
	return matching_bits(p2+2, sort_ip) - matching_bits(p1+2, sort_ip);
}

/****************************************************************************
sort a set of 6 byte name query response records so that the IPs that
have the most leading bits in common with the specified address come first
  ***************************************************************************/
void sort_query_replies(char *data, int n, struct in_addr ip)
{
	if (n <= 1) return;

	putip(sort_ip, (char *)&ip);

	qsort(data, n, 6, QSORT_CAST name_query_comp);
}
