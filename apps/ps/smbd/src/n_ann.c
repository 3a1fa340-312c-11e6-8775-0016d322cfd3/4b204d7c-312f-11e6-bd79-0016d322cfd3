/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   NBT netbios routines and daemon - version 2
   Copyright (C) Andrew Tridgell 1994-1998
   Copyright (C) Luke Kenneth Casson Leighton 1994-1998
   Copyright (C) Jeremy Allison 1994-1998

   SMB Version handling
   Copyright (C) John H Terpstra 1995-1998
   
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

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"

#include "nameserv.h"

//0625extern int DEBUGLEVEL;
extern pstring global_myname;
//extern fstring global_myworkgroup;
//0625 extern char **my_netbios_names;
//0625 extern int  updatecount;
extern int found_lm_clients;

//form psmain.c
extern char * strupr ( char * string );

//form unistr.c
extern char *skip_string(char *buf,uint32 n);

/****************************************************************************
  Broadcast an announcement.
  **************************************************************************/

static void send_announcement(struct subnet_record *subrec, int announce_type,
                              char *from_name, char *to_name, int to_type, struct in_addr to_ip,
                              time_t announce_interval,
                              char *server_name, int server_type, char *server_comment)
{
  pstring outbuf;
  char *p;
  struct in_addr src_ip;

  memset(outbuf,'\0',sizeof(outbuf));
  p = outbuf+1;

  CVAL(outbuf,0) = announce_type;

  /* Announcement parameters. */
//0625  CVAL(p,0) = updatecount;
  SIVAL(p,1,announce_interval*1000); /* Milliseconds - despite the spec. */

  strncpy(p+5,server_name,15);
  strupr(p+5);

//0625  CVAL(p,21) = lp_major_announce_version(); /* Major version. */
//0625  CVAL(p,22) = lp_minor_announce_version(); /* Minor version. */
  CVAL(p,21) = 0x04; /* Major version. */
  CVAL(p,22) = 0x02; /* Minor version. */

  SIVAL(p,23,server_type & ~SV_TYPE_LOCAL_LIST_ONLY);
  /* Browse version: got from NT/AS 4.00  - Value defined in smb.h (JHT). */
  SSVAL(p,27,BROWSER_ELECTION_VERSION);
  SSVAL(p,29,BROWSER_CONSTANT); /* Browse signature. */

  pstrcpy(p+31,server_comment);
  p += 31;
  p = skip_string(p,1);
  src_ip.s_addr = htonl(subrec->myip.s_addr);

  send_mailslot(False,BROWSE_MAILSLOT, outbuf, PTR_DIFF(p,outbuf),
//0626                from_name, 0x0, to_name, to_type, to_ip, subrec->myip,
                from_name, 0x0, to_name, to_type, to_ip, src_ip,
		DGRAM_PORT);
}

/****************************************************************************
 Announce the given host to WORKGROUP<1d>.
****************************************************************************/

//0718static void send_host_announcement(struct subnet_record *subrec, struct work_record *work,
//                                   struct server_record *servrec)
static void send_host_announcement(struct subnet_record *subrec
				, struct work_record *work)

{

  /* Ensure we don't have the prohibited bits set. */
//0626  uint32 type = servrec->serv.type & ~SV_TYPE_LOCAL_LIST_ONLY;
  uint32 type = 104963;

//0625  DEBUG(3,("send_host_announcement: type %x for host %s on subnet %s for workgroup %s\n",
//0625            type, servrec->serv.name, subrec->subnet_name, work->work_group));

//0626  send_announcement(subrec, ANN_HostAnnouncement,
//0626                    servrec->serv.name,              /* From nbt name. */
//0626                    work->work_group, 0x1d,          /* To nbt name. */
//0626                    subrec->bcast_ip,                /* To ip. */
//0626                    work->announce_interval,         /* Time until next announce. */
//0626                    servrec->serv.name,              /* Name to announce. */
//0626                    type,                            /* Type field. */
//0626                    servrec->serv.comment);
  send_announcement(subrec, ANN_HostAnnouncement,
                    my_netbios_names[0],              /* From nbt name. */
// for test                   "WIN98", 0x1d,          /* To nbt name. */
                    _MyWorkgroup, 0x1d,          /* To nbt name. */                    
                    subrec->bcast_ip,                /* To ip. */
                    work->announce_interval,         /* Time until next announce. */
                    my_netbios_names[0],              /* Name to announce. */
                    type,                            /* Type field. */
                    "PrinterServer");

}

/****************************************************************************
  Announce a server record.
  ****************************************************************************/

//0718static void announce_server(struct subnet_record *subrec, struct work_record *work,
//                     struct server_record *servrec)
static void announce_server(struct subnet_record *subrec
			,struct work_record *work)

{
  /* Only do domain announcements if we are a master and it's
     our primary name we're being asked to announce. */

//0718	send_host_announcement(subrec, work, servrec);
	send_host_announcement(subrec, work);

/*0626
  if (AM_LOCAL_MASTER_BROWSER(work) && !strcmp(global_myname,servrec->serv.name))
  {
//0625    send_local_master_announcement(subrec, work, servrec);
//0625    send_workgroup_announcement(subrec, work);
  }
  else
  {
    send_host_announcement(subrec, work, servrec);
  }
*/
}

/****************************************************************************
  Go through all my registered names on all broadcast subnets and announce
  them if the timeout requires it.
  **************************************************************************/

void announce_my_server_names(time_t t)
{
  struct subnet_record *subrec;
  struct work_record *work = NULL;
  static time_t _lastannounce_time = 0;
  static int _announce_interval = 0;
  static int _needannounce = 1;
  int i;
//0625  for (subrec = FIRST_SUBNET; subrec; subrec = NEXT_SUBNET_EXCLUDING_UNICAST(subrec))
  for( i = 0; i < MAX_SUBNETS; i++ )
	{  
  	  if( _subnetlist[i].bInUsed && (&_subnetlist[i] != unicast_subnet) )
		{
			if ((work = (struct work_record *)malloc(sizeof(struct work_record))) == NULL)
				return;
			work->needannounce = _needannounce;
			work->announce_interval = _announce_interval;
			work->lastannounce_time = _lastannounce_time;
			subrec = &_subnetlist[i];


  		  	if(work)
    		{
//0718      			struct server_record *servrec;

      			if (work->needannounce) //I never used this if ........By Ron 3/8/2002
      			{
        /* Drop back to a max 3 minute announce. This is to prevent a
           single lost packet from breaking things for too long. */

        			work->announce_interval = MIN(work->announce_interval,
                                    CHECK_TIME_MIN_HOST_ANNCE*60);
        			work->lastannounce_time = t - (work->announce_interval+1);
        			_needannounce = False;
      			}

      /* Announce every minute at first then progress to every 12 mins */  
      			if ((t - work->lastannounce_time) < work->announce_interval)
        			continue;
// I have redefined CHECK_TIME_MAX_HOST_ANNCE from 12 to 1. In order to reduce announce interval.  By Ron 3/8/2002
      			if (work->announce_interval < (CHECK_TIME_MAX_HOST_ANNCE * 60))  
        			_announce_interval += 60;

      			_lastannounce_time = t;

//      			for (servrec = work->serverlist; servrec; servrec = servrec->next)
//      			{
//        			if (is_myname(servrec->serv.name))
//0718          				announce_server(subrec, work, servrec);
	    				announce_server(subrec, work);

//      			}
    		} /* if work */
		} //if _subnetlist	
  	} /* for subrec */
	if (work)
		free(work);
}

