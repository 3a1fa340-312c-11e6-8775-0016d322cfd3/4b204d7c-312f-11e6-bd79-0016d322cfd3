/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   NBT netbios routines and daemon - version 2
   Copyright (C) Andrew Tridgell 1994-1998
   Copyright (C) Luke Kenneth Casson Leighton 1994-1998
   Copyright (C) Jeremy Allison 1994-1998
   
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

#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"

#include "btorder.h"


//from psmain.c
extern uint32 msclock();

//from n_reg.c
extern void register_name_timeout_response(struct subnet_record *subrec, struct response_record *rrec);

//from nmblib.c
extern int send_packet(struct packet_struct *p);
extern void free_packet(struct packet_struct *packet);
extern struct packet_struct *read_packet(int fd,enum packet_type packet_type);
extern void make_nmb_name( struct nmb_name *n, const char *name, int type );

//from smbutil.c
extern int same_net(struct in_addr ip1,struct in_addr ip2,struct in_addr mask);
extern int set_message(char *buf,int num_words,int num_bytes,int zero);

//form n_name.c
extern struct name_record *find_name_on_subnet( struct subnet_record *subrec,
                                         struct nmb_name      *nmbname,
                                         int                  self_only );
                                         
//form n_req.c                                         
extern void process_name_query_request(struct subnet_record *subrec, struct packet_struct *p);    
//Jesse
extern void process_node_status_request(struct subnet_record *subrec, struct packet_struct *p);

//form intface.c
extern int is_local_net(struct in_addr from);                                     
extern int ismyip(struct in_addr ip);

/*******************************************************************
  The global packet linked-list. Incoming entries are 
  added to the end of this list. It is supposed to remain fairly 
  short so we won't bother with an end pointer.
******************************************************************/

static struct packet_struct *packet_queue = NULL;

/***************************************************************************
Utility function to find the specific fd to send a mailslot packet out on.
**************************************************************************/

static int find_subnet_mailslot_fd_for_address( struct in_addr local_ip )
{
  struct subnet_record *subrec;
  int i;
//0626  for( subrec = FIRST_SUBNET; subrec; subrec = NEXT_SUBNET_EXCLUDING_UNICAST(subrec))
  for( i = 0; i < MAX_SUBNETS; i++ )
	{  
  	  if( _subnetlist[i].bInUsed && (&_subnetlist[i] != unicast_subnet) )
		{
			subrec = &_subnetlist[i];
		    if(ip_equal(local_ip, subrec->myip))
      			return subrec->dgram_sock;
		}
	}	
  return ClientDGRAM;
}

/***************************************************************************
Get/Set problematic nb_flags as network byte order 16 bit int.
**************************************************************************/

uint16 get_nb_flags(char *buf)
{
	return ((((uint16)*buf)&0xFFFF) & NB_FLGMSK);
}

void set_nb_flags(char *buf, uint16 nb_flags)
{
	*buf++ = ((nb_flags & NB_FLGMSK) & 0xFF);
	*buf = '\0';
}

/***************************************************************************
  Generates the unique transaction identifier
**************************************************************************/

static uint16 name_trn_id=0;

static uint16 generate_name_trn_id(void)
{
	if( !name_trn_id )
		name_trn_id = (unsigned)msclock()%(unsigned)0x7FFF;
	name_trn_id = (name_trn_id+1) % (unsigned)0x7FFF;

	return name_trn_id;
}

/***************************************************************************
 Either loops back or sends out a completed NetBIOS packet.
**************************************************************************/

static int send_netbios_packet(struct packet_struct *p)
{
	if (!send_packet(p))
	{
		return FALSE;
	}
	
	return TRUE;  
} 

/****************************************************************************
  Reply to a netbios name packet.  see rfc1002.txt
****************************************************************************/

void reply_netbios_packet(struct packet_struct *orig_packet,
                          int rcode, enum netbios_reply_type_code rcv_code, int opcode,
                          int ttl, char *data,int len)
{
	struct packet_struct packet;
	struct nmb_packet *nmb = NULL;
	struct res_rec answers;
	struct nmb_packet *orig_nmb = &orig_packet->packet.nmb;
	/* Check if we are sending to or from ourselves. */
  
	nmb = &packet.packet.nmb;

	/* Do a partial copy of the packet. We clear the locked flag and
	   the resource record pointers. */
	packet = *orig_packet;   /* Full structure copy. */
	packet.locked = FALSE;
	nmb->answers = NULL;
	nmb->nsrecs = NULL;
	nmb->additional = NULL;

	switch (rcv_code)
	{
	case NMB_STATUS:
		nmb->header.nm_flags.recursion_desired = FALSE;
		nmb->header.nm_flags.recursion_available = FALSE;
		break;
	case NMB_QUERY:
		nmb->header.nm_flags.recursion_desired = TRUE;
		nmb->header.nm_flags.recursion_available = TRUE;
		break;
	case NMB_REG:
	case NMB_REG_REFRESH:
		nmb->header.nm_flags.recursion_desired = TRUE;
		nmb->header.nm_flags.recursion_available = TRUE;
		break;
	case NMB_REL:
		nmb->header.nm_flags.recursion_desired = FALSE;
		nmb->header.nm_flags.recursion_available = FALSE;
		break;
	case NMB_WAIT_ACK:
		nmb->header.nm_flags.recursion_desired = FALSE;
		nmb->header.nm_flags.recursion_available = FALSE;
		break;
	case WINS_REG:
		nmb->header.nm_flags.recursion_desired = TRUE;
		nmb->header.nm_flags.recursion_available = TRUE;
		break;
	case WINS_QUERY:
		nmb->header.nm_flags.recursion_desired = TRUE;
		nmb->header.nm_flags.recursion_available = TRUE;
		break;
	default:
		return;
	}

	nmb->header.name_trn_id = orig_nmb->header.name_trn_id;
	nmb->header.opcode = opcode;
	nmb->header.response = TRUE;
	nmb->header.nm_flags.bcast = FALSE;
	nmb->header.nm_flags.trunc = FALSE;
	nmb->header.nm_flags.authoritative = TRUE;
  
	nmb->header.rcode = rcode;
	nmb->header.qdcount = 0;
	nmb->header.ancount = 1;
	nmb->header.nscount = 0;
	nmb->header.arcount = 0;
  
	memset((char*)&nmb->question,'\0',sizeof(nmb->question));
  
	nmb->answers = &answers;
	memset((char*)nmb->answers,'\0',sizeof(struct res_rec));
  
	nmb->answers->rr_name  = orig_nmb->question.question_name;
	nmb->answers->rr_type  = orig_nmb->question.question_type;
	nmb->answers->rr_class = orig_nmb->question.question_class;
	nmb->answers->ttl      = ttl;
  
	if (data && len)
	{
		nmb->answers->rdlength = len;
		memcpy(nmb->answers->rdata, data, len);
	}
  
	packet.packet_type = NMB_PACKET;
	/* Ensure we send out on the same fd that the original
	   packet came in on to give the correct source IP address. */
	packet.fd = orig_packet->fd;
	packet.timestamp = (msclock()/10);
	if (!send_packet(&packet)) 
	{
		// DEBUG(0,("reply_netbios_packet: send_packet to IP %s port %d failed\n",
		//    inet_ntoa(packet.ip),packet.port));
	}
}

/*******************************************************************
  Queue a packet into a packet queue
******************************************************************/
static void queue_packet(struct packet_struct *packet)
{
	struct packet_struct *p;

	if (!packet_queue) 
	{
		packet->prev = NULL;
		packet->next = NULL;
		packet_queue = packet;
		return;
	}
  
	/* find the bottom */
	for (p=packet_queue;p->next;p=p->next)
		;

	p->next = packet;
	packet->next = NULL;
	packet->prev = p;
}


/****************************************************************************
  Determine if a packet is for us on port 138. Note that to have any chance of
  being efficient we need to drop as many packets as possible at this
  stage as subsequent processing is expensive. 
****************************************************************************/

static int listening(struct packet_struct *p,struct nmb_name *nbname)
{
	struct subnet_record *subrec = NULL;
	int i;

	for( i = 0; i < MAX_SUBNETS; i++ )
	{
		if( _subnetlist[i].bInUsed )
		{
			subrec = &_subnetlist[i];
			if(same_net(p->ip, subrec->myip, subrec->mask_ip))
				break;
		}
	}

	if(subrec == NULL)
		subrec = unicast_subnet;

	return (find_name_on_subnet(subrec, nbname, FIND_SELF_NAME) != NULL);
}

/****************************************************************************
  Process udp 138 datagrams
****************************************************************************/
static void process_dgram(struct packet_struct *p)
{
	struct dgram_packet *dgram = &p->packet.dgram;
	char *buf;
	char *buf2;
	int len;

	/* If we aren't listening to the destination name then ignore the packet */
	if (!listening(p,&dgram->dest_name))
	{
		return;
	}

	if (dgram->header.msg_type != 0x10 &&
		dgram->header.msg_type != 0x11 &&
		dgram->header.msg_type != 0x12) 
	{
		return;
	}

	buf = &dgram->data[0];
	buf -= 4; /* XXXX for the pseudo tcp length - 
	       		someday I need to get rid of this */

	if (CVAL(buf,smb_com) != SMBtrans)
		return;

	len = SVAL(buf,smb_vwv11);
	buf2 = smb_base(buf) + SVAL(buf,smb_vwv12);

	if (len <= 0)
    	return;

	/* Datagram packet received for the browser mailslot */
	if (!strcmp(smb_buf(buf),BROWSE_MAILSLOT))
	{
		//Charles
		//process_browse_packet(p,buf2,len);
		return;
	}

	/* Datagram packet received for the LAN Manager mailslot */
	if (!strcmp(smb_buf(buf),LANMAN_MAILSLOT))
	{
		//Charles
		//process_lanman_packet(p,buf2,len);
		return;
	}

	/* Datagram packet received for the domain logon mailslot */
	if (!strcmp(smb_buf(buf),NET_LOGON_MAILSLOT))
	{
		//Charles
		//process_logon_packet(p,buf2,len,NET_LOGON_MAILSLOT);
		return;
	}

	/* Datagram packet received for the NT domain logon mailslot */
	if (!strcmp(smb_buf(buf),NT_LOGON_MAILSLOT))
	{
		//Charles
		//process_logon_packet(p,buf2,len,NT_LOGON_MAILSLOT);
		return;
	}
}

/****************************************************************************
  Validate a request nmb packet.
****************************************************************************/

static int validate_nmb_packet( struct nmb_packet *nmb )
{
	int ignore = FALSE;

	switch (nmb->header.opcode) 
	{
	case NMB_NAME_REG_OPCODE:
	case NMB_NAME_REFRESH_OPCODE_8: /* ambiguity in rfc1002 about which is correct. */
	case NMB_NAME_REFRESH_OPCODE_9: /* WinNT uses 8 by default. */
	case NMB_NAME_MULTIHOMED_REG_OPCODE:
		if (nmb->header.qdcount==0 || nmb->header.arcount==0)
		{
			// DEBUG(0,("validate_nmb_packet: Bad REG/REFRESH Packet. "));
			ignore = TRUE;
		}
		break;

	case NMB_NAME_QUERY_OPCODE:
		if ((nmb->header.qdcount == 0) || 
			((nmb->question.question_type != QUESTION_TYPE_NB_QUERY) &&
			(nmb->question.question_type != QUESTION_TYPE_NB_STATUS)))
		{
			// DEBUG(0,("validate_nmb_packet: Bad QUERY Packet. "));
			ignore = TRUE;
		}
		break;

	case NMB_NAME_RELEASE_OPCODE:
		if (nmb->header.qdcount==0 || nmb->header.arcount==0)
		{
			// DEBUG(0,("validate_nmb_packet: Bad RELEASE Packet. "));
			ignore = TRUE;
		}
		break;
	default:
		// DEBUG(0,("validate_nmb_packet: Ignoring packet with unknown opcode %d.\n",        nmb->header.opcode));
		return TRUE;
	}
 
	if(ignore)
	{
		// DEBUG(0,("validate_nmb_packet: Ignoring request packet with opcode %d.\n", nmb->header.opcode));
	}

	return ignore;
}

/****************************************************************************
  Find a subnet (and potentially a response record) for a packet.
****************************************************************************/

static struct subnet_record *find_subnet_for_nmb_packet( struct packet_struct *p,
                                                         struct response_record **pprrec)
{
	struct nmb_packet *nmb = &p->packet.nmb;
	struct subnet_record *subrec = NULL;
	int i;

	if(pprrec != NULL)
		*pprrec = NULL;
	/* Try and see what subnet this packet belongs to. */

	/* WINS server ? */
	//if(packet_is_for_wins_server(p))
	//	return wins_server_subnet;

	/* If it wasn't a broadcast packet then send to the UNICAST subnet. */
	if(nmb->header.nm_flags.bcast == FALSE)
		return unicast_subnet;

	/* Go through all the broadcast subnets and see if the mask matches. */
	for( i = 0; i < MAX_SUBNETS; i++ )
	{
		if( _subnetlist[i].bInUsed )
		{
			subrec = &_subnetlist[i];
			if(same_net(p->ip, subrec->myip, subrec->mask_ip))
				return subrec;
		}
	}

	/* If none match it must have been a directed broadcast - assign
		the remote_broadcast_subnet. */
	return remote_broadcast_subnet;
}

/****************************************************************************
  Process a nmb request packet - validate the packet and route it.
****************************************************************************/

static void process_nmb_request(struct packet_struct *p)
{
	struct nmb_packet *nmb = &p->packet.nmb;
	struct subnet_record *subrec = NULL;

	/* Ensure we have a good packet. */
	if(validate_nmb_packet(nmb))
		return;

	/* Allocate a subnet to this packet - if we cannot - fail. */
	if((subrec = find_subnet_for_nmb_packet(p, NULL))==NULL)
		return;

	switch (nmb->header.opcode) 
	{
	case NMB_NAME_REG_OPCODE:
		break;
	case NMB_NAME_REFRESH_OPCODE_8: /* ambiguity in rfc1002 about which is correct. */
	case NMB_NAME_REFRESH_OPCODE_9:
		break;
	case NMB_NAME_MULTIHOMED_REG_OPCODE:
		break;
	case NMB_NAME_QUERY_OPCODE:
		switch (nmb->question.question_type)
		{
		case QUESTION_TYPE_NB_QUERY:
				process_name_query_request(subrec, p);
			break;
		case QUESTION_TYPE_NB_STATUS:
//Jesse
				process_node_status_request(subrec, p);
          break;
		}
		break;
      
	case NMB_NAME_RELEASE_OPCODE:
		break;
	}
}

/*******************************************************************
  Run elements off the packet queue till its empty
******************************************************************/

void run_packet_queue(void)
{
	struct packet_struct *p;

	while((p = packet_queue))
	{
		packet_queue = p->next;
		if (packet_queue)
			packet_queue->prev = NULL;
		p->next = p->prev = NULL;

		switch (p->packet_type)
		{
		case NMB_PACKET:
			if(p->packet.nmb.header.response){
//0420				process_nmb_response(p);
			}	
			else
				process_nmb_request(p);
			break;

		case DGRAM_PACKET:
			process_dgram(p);
			break;
		}
		free_packet(p);
	}
} 

/*******************************************************************
 Retransmit or timeout elements from all the outgoing subnet response
 record queues. NOTE that this code must also check the WINS server
 subnet for response records to timeout as the WINS server code
 can send requests to check if a client still owns a name.
 (Patch from Andrey Alekseyev <fetch@muffin.arcadia.spb.ru>).
******************************************************************/

void retransmit_or_expire_resp_records(time_t t)
{
	struct subnet_record *subrec;
	int i;

	for( i = 0; i < MAX_SUBNETS; i++ )
	{
		if( _subnetlist[i].bInUsed && &_subnetlist[i] != unicast_subnet )
		{
			struct response_record *rrec = NULL;

			subrec = &_subnetlist[i];
			register_name_timeout_response(subrec, rrec);
		}	
	}
}

/****************************************************************************
  Listens for NMB or DGRAM packets, and queues them.
***************************************************************************/

int listen_for_packets()
{
	struct packet_struct *packet;
	static int listen_number = 0;
	static int sock_array[MAX_SUBNETS*2+2];
	static int listen_set = FALSE;
	//struct timeval timeout;
	int i;

	if( !listen_set )
	{
		/* Add in the broadcast socket on 137. */
		sock_array[listen_number++] = ClientNMB;

		for( i = 0; i < MAX_SUBNETS; i++ )
		{
			if( _subnetlist[i].bInUsed )
				sock_array[listen_number++] = _subnetlist[i].nmb_sock;
		}

		/* Add in the broadcast socket on 138. */
		sock_array[listen_number++] = ClientDGRAM;

		for( i = 0; i < MAX_SUBNETS; i++ )
		{
			if( _subnetlist[i].bInUsed )
				sock_array[listen_number++] = _subnetlist[i].dgram_sock;
		}

		listen_set = TRUE;
	}

	/* 
	 * During elections and when expecting a netbios response packet we
	 * need to send election packets at tighter intervals.
	 * Ideally it needs to be the interval (in ms) between time now and
	 * the time we are expecting the next netbios packet.
	 */

	//timeout.tv_sec = (run_election||num_response_packets) ? 1 : NMBD_SELECT_LOOP;
	//timeout.tv_usec = 0;

	for( i = 0; i < listen_number; i++ )
	{
		if(i < (listen_number/2))
		{
			/* Processing a 137 socket. */
			packet = read_packet(sock_array[i], NMB_PACKET);
			if (packet)
			{
				/*
				 * If we got a packet on the broadcast socket and interfaces
				 * only is set then check it came from one of our local nets. 
				 */
				if( (sock_array[i] == ClientNMB) && (!is_local_net(packet->ip)))
				{
					// DEBUG(7,("discarding nmb packet sent to broadcast socket from %s:%d\n",
					//   inet_ntoa(packet->ip),packet->port));	  
					free_packet(packet);
				}
				else if ((ip_equal(loopback_ip, packet->ip) || 
              		ismyip(packet->ip)) && packet->port == NMB_PORT)
				{
					// DEBUG(7,("discarding own packet from %s:%d\n",
					//   inet_ntoa(packet->ip),packet->port));	  
					free_packet(packet);
				}
				else
				{
					// Save the file descriptor this packet came in on.
					packet->fd = sock_array[i];
					queue_packet(packet);
				}
			}
		}
		else
		{
			/* Processing a 138 socket. */
			packet = read_packet(sock_array[i], DGRAM_PACKET);
			if (packet)
			{
				/*
				 * If we got a packet on the broadcast socket and interfaces
				 * only is set then check it came from one of our local nets. 
				 */
				if( (sock_array[i] == ClientDGRAM) && (!is_local_net(packet->ip)))
				{
					// DEBUG(7,("discarding dgram packet sent to broadcast socket from %s:%d\n",
					//   inet_ntoa(packet->ip),packet->port));	  
					free_packet(packet);
				}
				else if ((ip_equal(loopback_ip, packet->ip) || 
					ismyip(packet->ip)) && packet->port == DGRAM_PORT)
				{
					// DEBUG(7,("discarding own packet from %s:%d\n",
					//   inet_ntoa(packet->ip),packet->port));	  
					free_packet(packet);
				}
				else
				{
					// Save the file descriptor this packet came in on.
					packet->fd = sock_array[i];
					queue_packet(packet);
				}
			}
		}
	}

	return FALSE;
}

/****************************************************************************
  Construct and send a netbios DGRAM.
**************************************************************************/
int send_mailslot(int unique, char *mailslot,char *buf,int len,
                   char *srcname, int src_type,
                   char *dstname, int dest_type,
                   struct in_addr dest_ip,struct in_addr src_ip,
				   int dest_port)
{
	struct packet_struct *lo_packet = NULL;
	struct packet_struct p;
	struct dgram_packet *dgram = &p.packet.dgram;
	int loopback_this_packet = FALSE;
	char *ptr,*p2;
	char tmp[4];

	memset((char *)&p,'\0',sizeof(p));

	if(ismyip(dest_ip))
		loopback_this_packet = TRUE;

	generate_name_trn_id();

	/* DIRECT GROUP or UNIQUE datagram. */
	dgram->header.msg_type = unique ? 0x10 : 0x11; 
	dgram->header.flags.node_type = M_NODE;
	dgram->header.flags.first = TRUE;
	dgram->header.flags.more = FALSE;
	dgram->header.dgm_id = name_trn_id;
	dgram->header.source_ip = src_ip;
	dgram->header.source_port = DGRAM_PORT;
	dgram->header.dgm_length = 0; /* Let build_dgram() handle this. */
	dgram->header.packet_offset = 0;
  
	make_nmb_name(&dgram->source_name,srcname,src_type);
	make_nmb_name(&dgram->dest_name,dstname,dest_type);

	ptr = &dgram->data[0];

	/* Setup the smb part. */
	ptr -= 4; /* XXX Ugliness because of handling of tcp SMB length. */
	memcpy(tmp,ptr,4);
	set_message(ptr,17,17 + len,TRUE);
	memcpy(ptr,tmp,4);

	CVAL(ptr,smb_com) = SMBtrans;
	SSVAL(ptr,smb_vwv1,len);
	SSVAL(ptr,smb_vwv11,len);
	SSVAL(ptr,smb_vwv12,70 + strlen(mailslot));
	SSVAL(ptr,smb_vwv13,3);
	SSVAL(ptr,smb_vwv14,1);
	SSVAL(ptr,smb_vwv15,1);
	SSVAL(ptr,smb_vwv16,2);
	p2 = smb_buf(ptr);
	pstrcpy(p2,mailslot);
	p2 += strlen(p2)+1;

	memcpy(p2,buf,len);
	p2 += len;

	dgram->datasize = PTR_DIFF(p2,ptr+4); /* +4 for tcp length. */

	p.ip = dest_ip;
	p.port = dest_port;
	p.fd = find_subnet_mailslot_fd_for_address( src_ip );
//0709	p.timestamp = time(NULL);
	p.timestamp = (msclock()/1000);
	p.packet_type = DGRAM_PACKET;

	//DEBUG(4,("send_mailslot: Sending to mailslot %s from %s IP %s ", mailslot,
	//    nmb_namestr(&dgram->source_name), inet_ntoa(src_ip)));
	//DEBUG(4,("to %s IP %s\n", nmb_namestr(&dgram->dest_name), inet_ntoa(dest_ip)));

	//debug_browse_data(buf, len);

	if(loopback_this_packet)
	{
		//DEBUG(5,("send_mailslot: sending packet to ourselves.\n"));
//0625    	if((lo_packet = copy_packet(&p)) == NULL)
//      		return FALSE;
//		queue_packet(lo_packet);
//		return TRUE;
	}
	else
		return(send_packet(&p));
}

