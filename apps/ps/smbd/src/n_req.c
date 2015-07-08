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
   
   This file contains all the code to process NetBIOS requests coming
   in on port 137. It does not deal with the code needed to service
   WINS server requests, but only broadcast and unicast requests.

*/



#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include <stdio.h>

#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"

//Jesse
#define QSORT_CAST (int (*)(const void *, const void *))

//from n_packet.c
extern void set_nb_flags(char *buf, uint16 nb_flags);
void reply_netbios_packet(struct packet_struct *orig_packet,
                          int rcode, enum netbios_reply_type_code rcv_code, int opcode,
                          int ttl, char *data,int len);

//from nmblib.c
extern void sort_query_replies(char *data, int n, struct in_addr ip);

//from smbutil.c
extern int same_net(struct in_addr ip1,struct in_addr ip2,struct in_addr mask);

//from n_name.c
extern struct name_record *find_name_for_remote_broadcast_subnet(
                                                   struct nmb_name *nmbname,
                                                   int             self_only );
extern struct name_record *find_name_on_subnet( struct subnet_record *subrec,
                                         struct nmb_name      *nmbname,
                                         int                  self_only );                                                   
                                                   
/***************************************************************************
Process a name query.

For broadcast name queries:

  - Only reply if the query is for one of YOUR names.
  - NEVER send a negative response to a broadcast query.

****************************************************************************/

void process_name_query_request(struct subnet_record *subrec, struct packet_struct *p)
{
	struct nmb_packet *nmb = &p->packet.nmb;
	struct nmb_name *question = &nmb->question.question_name;
	int name_type = question->name_type;
	int bcast = nmb->header.nm_flags.bcast;
	int ttl=0;
	int rcode = 0;
	char *prdata = NULL;
	char rdata[6];
	int success = False;
	struct name_record *namerec = NULL;
	int reply_data_len = 0;
	int i;
   
	/* Look up the name in the cache - if the request is a broadcast request that
		came from a subnet we don't know about then search all the broadcast subnets
		for a match (as we don't know what interface the request came in on). */

	if(subrec == remote_broadcast_subnet)
		namerec = find_name_for_remote_broadcast_subnet( question, FIND_ANY_NAME);
	else
		namerec = find_name_on_subnet(subrec, question, FIND_ANY_NAME);


	/* Check if it is a name that expired */
	if( namerec
		&& ( (namerec->data.death_time != PERMANENT_TTL)
		&& (namerec->data.death_time < p->timestamp) ) )
	{
		namerec = NULL;
	}

	if (namerec)
	{

		/* 
		 * Always respond to unicast queries.
		 * Don't respond to broadcast queries unless the query is for
		 * a name we own, a Primary Domain Controller name, or a WINS_PROXY 
		 * name with type 0 or 0x20. WINS_PROXY names are only ever added
		 * into the namelist if we were configured as a WINS proxy.
		 */

		if( !bcast
			|| ( bcast
			&& ( (name_type == 0x1b)
			|| (namerec->data.source == SELF_NAME)
			|| (namerec->data.source == PERMANENT_NAME)
			|| ( (namerec->data.source == WINS_PROXY_NAME)
			&& ( (name_type == 0) || (name_type == 0x20) ) ) ) ) )
		{
      
			/* The requested name is a directed query, or it's SELF or PERMANENT or WINS_PROXY, 
				or it's a Domain Master type. */

			/*
			 * If this is a WINS_PROXY_NAME, then ceck that none of the IP 
			 * addresses we are returning is on the same broadcast subnet 
			 * as the requesting packet. If it is then don't reply as the 
			 * actual machine will be replying also and we don't want two 
			 * replies to a broadcast query.
			 */

			if( namerec->data.source == WINS_PROXY_NAME )
			{
				for( i = 0; i < namerec->data.num_ips; i++)
				{
					if(same_net( namerec->data.ip[i], subrec->myip, subrec->mask_ip ))
					{
						//  DEBUG(5,("process_name_query_request: name %s is a WINS proxy name and is also \
						//      on the same subnet (%s) as the requestor. Not replying.\n", 
						//      nmb_namestr(&namerec->name), subrec->subnet_name ));
						return;
					}
				}
			}     

			ttl = (namerec->data.death_time != PERMANENT_TTL) ?
				namerec->data.death_time - p->timestamp : GMAX_TTL;

			/* Copy all known ip addresses into the return data. */
			/* Optimise for the common case of one IP address so 
				we don't need a malloc. */

			if( namerec->data.num_ips == 1 )
				prdata = rdata;
			else
			{
				if((prdata = (char *)malloc( namerec->data.num_ips * 6 )) == NULL)
				{
					//   DEBUG(0,("process_name_query_request: malloc fail !\n"));
					return;
				}
			}

			for( i = 0; i < namerec->data.num_ips; i++ )
			{
				set_nb_flags(&prdata[i*6],namerec->data.nb_flags);
				putip((char *)&prdata[2+(i*6)], &namerec->data.ip[i]);
			}

			sort_query_replies(prdata, i, p->ip);
      
			reply_data_len = namerec->data.num_ips * 6;
			success = True;
		}
	}

	if (!success && bcast)
	{
		if((prdata != rdata) && (prdata != NULL))
			free(prdata);
		return; /* Never reply with a negative response to broadcasts. */
	}

	/* 
	 * Final check. From observation, if a unicast packet is sent
	 * to a non-WINS server with the recursion desired bit set
	 * then never send a negative response.
	 */

	if(!success && !bcast && nmb->header.nm_flags.recursion_desired)
	{
		if((prdata != rdata) && (prdata != NULL))
			free(prdata);
		return;
	}

	if (success)
	{
		rcode = 0;
	}
	else
	{
		rcode = NAM_ERR;
	}

	/* See rfc1002.txt 4.2.13. */

	reply_netbios_packet(p,                              /* Packet to reply to. */
						rcode,                          /* Result code. */
						NMB_QUERY,                      /* nmbd type code. */
						NMB_NAME_QUERY_OPCODE,          /* opcode. */
						ttl,                            /* ttl. */
						prdata,                         /* data to send. */
						reply_data_len);                /* data length. */

	if((prdata != rdata) && (prdata != NULL))
		free(prdata);
}

/****************************************************************************
This is used to sort names for a name status into a sensible order.
We put our own names first, then in alphabetical order.
**************************************************************************/

static int status_compare(char *n1,char *n2)
{
  extern pstring global_myname;
  int l1,l2,l3;

  /* It's a bit tricky because the names are space padded */
  for (l1=0;l1<15 && n1[l1] && n1[l1] != ' ';l1++) ;
  for (l2=0;l2<15 && n2[l2] && n2[l2] != ' ';l2++) ;
  l3 = strlen(global_myname);

  if ((l1==l3) && strncmp(n1,global_myname,l3) == 0 && 
      (l2!=l3 || strncmp(n2,global_myname,l3) != 0))
    return -1;

  if ((l2==l3) && strncmp(n2,global_myname,l3) == 0 && 
      (l1!=l3 || strncmp(n1,global_myname,l3) != 0))
    return 1;

  return memcmp(n1,n2,18);
}

/*******************************************************************
  convert a string to upper case
********************************************************************/
void strupper(char *s)
{

	while (*s)
	{
    	if (islower(*s))
          *s = toupper(*s);
 	     s++;
	}

}

/****************************************************************************
  Process a node status query
  ****************************************************************************/

void process_node_status_request(struct subnet_record *subrec, struct packet_struct *p)
{
  struct nmb_packet *nmb = &p->packet.nmb;
  char *qname   = nmb->question.question_name.name;
  int ques_type = nmb->question.question_name.name_type;
  char rdata[MAX_DGRAM_SIZE]={0};
  char *countptr, *buf, *bufend, *buf0;
  int names_added,i;
  struct name_record *namerec;

  if((namerec = find_name_on_subnet(subrec, &nmb->question.question_name,
                                    FIND_SELF_NAME)) == 0)
  {
    return;
  }
 
  /* this is not an exact calculation. the 46 is for the stats buffer
     and the 60 is to leave room for the header etc */
  bufend = &rdata[MAX_DGRAM_SIZE] - (18 + 46 + 60);
  countptr = buf = rdata;
  buf += 1;
  buf0 = buf;

  names_added = 0;

 //Jesse namerec = (struct name_record *)ubi_trFirst( subrec->namelist );
	namerec = (struct name_record *)( subrec->namelist );	
	
  while (buf < bufend) 
  {
    if( (namerec->data.source == SELF_NAME)
     || (namerec->data.source == PERMANENT_NAME) )
    {
      int name_type = namerec->name.name_type;

/*
	if (!strequal(namerec->name.name,"*") &&
          !strequal(namerec->name.name,"__SAMBA__") &&
          (name_type < 0x1b || name_type >= 0x20 || 
           ques_type < 0x1b || ques_type >= 0x20 ||
           strequal(qname, namerec->name.name)))*/
// change strequal to strcmp            
	  if (strcmp(namerec->name.name,"*") &&
	      strcmp(namerec->name.name,"__SAMBA__") &&
          (name_type < 0x1b || name_type >= 0x20 || 
           ques_type < 0x1b || ques_type >= 0x20 ||
           !strcmp(qname, namerec->name.name)))
      
      {
        /* Start with the name. */
        memset(buf,'\0',18);
//Jesse        slprintf(buf, 17, "%-15.15s",namerec->name.name);
		if(strlen(namerec->name.name) < 18 )
			sprintf(buf, "%-15.15s",namerec->name.name);
		else
		    memcpy( buf, namerec->name.name, 18);
        strupper(buf);
        
        /* Put the name type and netbios flags in the buffer. */
        buf[15] = name_type;
        set_nb_flags( &buf[16],namerec->data.nb_flags );
        buf[16] |= NB_ACTIVE; /* all our names are active */

        buf += 18;

        names_added++;
      }
    }

    /* Remove duplicate names. */
    qsort( buf0, names_added, 18, QSORT_CAST status_compare );

    for( i=1; i < names_added ; i++ )
    {
      if (memcmp(buf0 + 18*i,buf0 + 18*(i-1),16) == 0) 
      {
        names_added--;
        if (names_added == i)
          break;
        memmove(buf0 + 18*i,buf0 + 18*(i+1),18*(names_added-i));
        i--;
      }
    }

    buf = buf0 + 18*names_added;

//Jesse    namerec = (struct name_record *)ubi_trNext( namerec );
namerec = namerec->next;

    if (!namerec)
    {
      /* End of the subnet specific name list. Now 
         add the names on the unicast subnet . */
//Jesse      struct subnet_record *uni_subrec = unicast_subnet;

//Jesse      if (uni_subrec != subrec)
//Jesse      {
//Jesse        subrec = uni_subrec;
//Jesse        namerec = (struct name_record *)ubi_trFirst( subrec->namelist );
//Jesse      }
    }
    if (!namerec)
      break;

  }
  
  SCVAL(countptr,0,names_added);
  
  /* We don't send any stats as they could be used to attack
     the protocol. */
  memset(buf,'\0',46);
  
  buf += 46;
  
  /* Send a NODE STATUS RESPONSE */
  reply_netbios_packet(p,                            /* Packet to reply to. */
                       0,                            /* Result code. */
                       NMB_STATUS,                   /* nmbd type code. */
                       NMB_NAME_QUERY_OPCODE,        /* opcode. */
		       0,                            /* ttl. */
                       rdata,                        /* data to send. */
                       PTR_DIFF(buf,rdata));         /* data length. */
}
