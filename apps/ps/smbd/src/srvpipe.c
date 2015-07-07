/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-1998,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1998,
 *  Copyright (C) Jeremy Allison                                    1999.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <cyg/kernel/kapi.h>
#include "network.h"
#include <stdlib.h>
#include "pstarget.h"
#include "psglobal.h"
#include "psdefine.h"
#include "eeprom.h"

#include "btorder.h"

#include "smbinc.h"
#include "smb.h"

#include "dlinlist.h"

//for pipes_struct must include
#include "rpc_misc.h"
#include "rpc_dce.h"
#include "ntdomain.h"


//from srvsvc.c
extern int api_srvsvc_rpc(pipes_struct *p, prs_struct *data);

//from srvwksvc.c
extern int api_wkssvc_rpc(pipes_struct *p, prs_struct *data);

//from srvreg.c
extern int api_reg_rpc(pipes_struct *p, prs_struct *data);

//from parseprs.c
extern void prs_mem_free(prs_struct *ps);
extern int prs_init(prs_struct *ps, uint32 size, uint8 align, int io);
extern void prs_give_memory(prs_struct *ps, char *buf, uint32 size, int is_dynamic);
extern char *prs_data_p(prs_struct *ps);
extern uint32 prs_offset(prs_struct *ps);
extern int prs_append_data(prs_struct *dst, char *src, uint32 len);
extern int prs_set_offset(prs_struct *ps, uint32 offset);
extern void prs_switch_type(prs_struct *ps, int io);
extern int prs_append_prs_data(prs_struct *dst, prs_struct *src);

//from parserpc.c
extern int smb_io_rpc_hdr(char *desc,  RPC_HDR *rpc, prs_struct *ps, int depth);
extern int smb_io_rpc_hdr_req(char *desc, RPC_HDR_REQ *rpc, prs_struct *ps, int depth);
extern int smb_io_rpc_hdr_rb(char *desc, RPC_HDR_RB *rpc, prs_struct *ps, int depth);
extern void init_rpc_hdr_ba(RPC_HDR_BA *rpc, uint16 max_tsize, uint16 max_rsize, uint32 assoc_gid,
				char *pipe_addr, uint8 num_results, uint16 result, uint16 reason, RPC_IFACE *transfer);
extern int smb_io_rpc_hdr_ba(char *desc, RPC_HDR_BA *rpc, prs_struct *ps, int depth);
extern void init_rpc_hdr(RPC_HDR *hdr, enum RPC_PKT_TYPE pkt_type, uint8 flags, uint32 call_id, int data_len, int auth_len);


//from spipe.c
extern int api_pipe_request(pipes_struct *p);
extern int create_next_pdu(pipes_struct *p);

pipes_struct *get_rpc_pipe(int pnum, int threadid);

#define PIPE            "\\PIPE\\"
#define PIPELEN         strlen(PIPE)

//0430 extern int DEBUGLEVEL;
// static pipes_struct *chain_p;
//0627 static int pipes_open;

#ifndef MAX_OPEN_PIPES
//0611#define MAX_OPEN_PIPES 64
#define MAX_OPEN_PIPES 16
#endif

pipes_struct *Pipes[NUM_SMBTHREAD];
//static struct bitmap *bmap;
/* this must be larger than the sum of the open files and directories */
//0628static int pipe_handle_offset;
// copy from ntrrans.c 4/30/2001
static char *known_nt_pipes[] = {
 "\\LANMAN",
 "\\srvsvc",
//0509  "\\samr",
  "\\wkssvc",
//0509  "\\NETLOGON",
//0509  "\\ntlsa",
  "\\ntsvcs",
//0509  "\\lsass",
//0509  "\\lsarpc",
  "\\winreg",
  NULL
};

//copy from spipe.c 0504/2001
/*******************************************************************
 The switch table for the pipe names and the functions to handle them.
 *******************************************************************/

struct api_cmd
{
  char * pipe_clnt_name;
  char * pipe_srv_name;
  int (*fn) (pipes_struct *, prs_struct *);
};

static struct api_cmd api_fd_commands[] =
{
//    { "lsarpc",   "lsass",   api_ntlsa_rpc },
//    { "samr",     "lsass",   api_samr_rpc },
    { "srvsvc",   "ntsvcs",  api_srvsvc_rpc },
    { "wkssvc",   "ntsvcs",  api_wkssvc_rpc },
//    { "NETLOGON", "lsass",   api_netlog_rpc },
#if 1 /* DISABLED_IN_2_0 JRATEST */
    { "winreg",   "winreg",  api_reg_rpc },
#endif
    { NULL,       NULL,      NULL }
//	{ NULL,       NULL}
};

int api_pipe_bind_req(pipes_struct *p, prs_struct *rpc_in_p);

/****************************************************************************
 Set the pipe_handle_offset. Called from smbd/files.c
****************************************************************************/
#if 0 //0628
void set_pipe_handle_offset(int max_open_files)
{
  if(max_open_files < 0x7000)
    pipe_handle_offset = 0x7000;
  else
    pipe_handle_offset = max_open_files + 10; /* For safety. :-) */
}
#endif //0
/****************************************************************************
 Reset pipe chain handle number.
****************************************************************************/
void reset_chain_p(void)
{
//0509	chain_p = NULL;
}

/****************************************************************************
 Initialise pipe handle states.
****************************************************************************/

void init_rpc_pipe_hnd(void)
{


}

/****************************************************************************
 Initialise an outgoing packet.
****************************************************************************/

static int pipe_init_outgoing_data(output_data *o_data)
{
	/* Reset the offset counters. */
	o_data->data_sent_length = 0;
	o_data->current_pdu_len = 0;
	o_data->current_pdu_sent = 0;

	memset(o_data->current_pdu, '\0', sizeof(o_data->current_pdu));

	/* Free any memory in the current return data buffer. */
	prs_mem_free(&o_data->rdata);

	/*
	 * Initialize the outgoing RPC data buffer.
	 * we will use this as the raw data area for replying to rpc requests.
	 */     
	if(!prs_init(&o_data->rdata, MAX_PDU_FRAG_LEN, 4, MARSHALL)) {
//		DEBUG(0,("pipe_init_outgoing_data: malloc fail.\n"));
		return False;
	}

	return True;
}



/****************************************************************************
 Find first available pipe slot.
****************************************************************************/

pipes_struct *open_rpc_pipe_p(char *pipe_name, uint16 vuid, int threadid)
{
	static uint16 pipe_handle_offset =0x7000; // 0x7000 from set_pipe_handle_offset()
	pipes_struct *p;
//0509	static int next_pipe;

//0502	DEBUG(4,("Open pipe requested %s (pipes_open=%d)\n",
//0502		 pipe_name, pipes_open));
	
	/* not repeating pipe numbers makes it easier to track things in 
	   log files and prevents client bugs where pipe numbers are reused
	   over connection restarts */
//0430	if (next_pipe == 0)
//		next_pipe = (getpid() ^ time(NULL)) % MAX_OPEN_PIPES;

//0430	i = bitmap_find(bmap, next_pipe);

//0626	if (i == -1) {
//0430		DEBUG(0,("ERROR! Out of pipe structures\n"));
//		return NULL;
//	}

//0509	next_pipe = (i+1) % MAX_OPEN_PIPES;

//0502	for (p = Pipes; p; p = p->next)
//0502		DEBUG(5,("open pipes: name %s pnum=%x\n", p->name, p->pnum));  

	p = (pipes_struct *)malloc(sizeof(pipes_struct));

	if (!p)
		return NULL;

//2/8/2002	ZERO_STRUCTP(p);
	memset((char *)p ,0, sizeof(pipes_struct));		
	DLIST_ADD(Pipes[threadid], p);

	/*
	 * Initialize the incoming RPC data buffer with one PDU worth of memory.
	 * We cheat here and say we're marshalling, as we intend to add incoming
	 * data directly into the prs_struct and we want it to auto grow. We will
	 * change the type to UNMARSALLING before processing the stream.
	 */

	if(!prs_init(&p->in_data.data, MAX_PDU_FRAG_LEN, 4, MARSHALL)) {
//0430		DEBUG(0,("open_rpc_pipe_p: malloc fail for in_data struct.\n"));
		if (p){
			DLIST_REMOVE(Pipes[threadid], p);
//2/8/2002   			ZERO_STRUCTP(p);
			memset((char *)p ,0, sizeof(pipes_struct));		
			free (p);
		}	                      //add by ron 7/19/2001
		return NULL;
	}

//0430	bitmap_set(bmap, i);
//0628	i += pipe_handle_offset;

//0627	pipes_open++;

//0620	p->pnum = i;
	p->pnum = (pipe_handle_offset&0xFF); //since NT File ID use 2 bytes
	pipe_handle_offset++;
	p->open = True;
	p->device_state = 0;
	p->priority = 0;
//0430	p->conn = conn;
	p->vuid  = vuid;

	p->max_trans_reply = 0;
	
	p->ntlmssp_chal_flags = 0;
	p->ntlmssp_auth_validated = False;
	p->ntlmssp_auth_requested = False;

	p->pipe_bound = False;
	p->fault_state = False;

	/*
	 * Initialize the incoming RPC struct.
	 */

	p->in_data.pdu_needed_len = 0;
	p->in_data.pdu_received_len = 0;

	/*
	 * Initialize the outgoing RPC struct.
	 */

	p->out_data.current_pdu_len = 0;
	p->out_data.current_pdu_sent = 0;
	p->out_data.data_sent_length = 0;

	/*
	 * Initialize the outgoing RPC data buffer with no memory.
	 */     
	prs_init(&p->out_data.rdata, 0, 4, MARSHALL);
	
//0430	p->uid = (uid_t)-1;
//0430	p->gid = (gid_t)-1;

	fstrcpy(p->name, pipe_name);
//0604	fstrcpy(p->name, "wkssvc");  //0502	
	
//0502	DEBUG(4,("Opened pipe %s with handle %x (pipes_open=%d)\n",
//0502		 pipe_name, i, pipes_open));
	
//0508	chain_p = p;
	
	/* OVERWRITE p as a temp variable, to display all open pipes */ 
//0502	for (p = Pipes; p; p = p->next)
//0502		DEBUG(5,("open pipes: name %s pnum=%x\n", p->name, p->pnum));  

//0509	return chain_p;
	return p;
}

/****************************************************************************
 Sets the fault state on incoming packets.
****************************************************************************/

static void set_incoming_fault(pipes_struct *p)
{
	prs_mem_free(&p->in_data.data);
	p->in_data.pdu_needed_len = 0;
	p->in_data.pdu_received_len = 0;
	p->fault_state = True;
//0502	DEBUG(10,("set_incoming_fault: Setting fault state on pipe %s : pnum = 0x%x\n",
//0502		p->name, p->pnum ));
}

/****************************************************************************
 Ensures we have at least RPC_HEADER_LEN amount of data in the incoming buffer.
****************************************************************************/

static int fill_rpc_header(pipes_struct *p, char *data, uint32 data_to_copy)
{
	uint32 len_needed_to_complete_hdr = MIN(data_to_copy, RPC_HEADER_LEN - p->in_data.pdu_received_len);

//0430	DEBUG(10,("fill_rpc_header: data_to_copy = %u, len_needed_to_complete_hdr = %u, receive_len = %u\n",
//			(unsigned int)data_to_copy, (unsigned int)len_needed_to_complete_hdr,
//			(unsigned int)p->in_data.pdu_received_len ));

	memcpy((char *)&p->in_data.current_in_pdu[p->in_data.pdu_received_len], data, len_needed_to_complete_hdr);
	p->in_data.pdu_received_len += len_needed_to_complete_hdr;

  return (int)len_needed_to_complete_hdr;
  

}

/****************************************************************************
 Unmarshalls a new PDU header. Assumes the raw header data is in current_in_pdu.
****************************************************************************/

static int unmarshall_rpc_header(pipes_struct *p)
{
	/*
	 * Unmarshall the header to determine the needed length.
	 */

	prs_struct rpc_in;

	if(p->in_data.pdu_received_len != RPC_HEADER_LEN) {
//		DEBUG(0,("unmarshall_rpc_header: assert on rpc header length failed.\n"));
//		set_incoming_fault(p);
		return -1;
	}

	prs_init( &rpc_in, 0, 4, UNMARSHALL);
	prs_give_memory( &rpc_in, (char *)&p->in_data.current_in_pdu[0],
					p->in_data.pdu_received_len, False);

	/*
	 * Unmarshall the header as this will tell us how much
	 * data we need to read to get the complete pdu.
	 */

	if(!smb_io_rpc_hdr("", &p->hdr, &rpc_in, 0)) {
//		DEBUG(0,("unmarshall_rpc_header: failed to unmarshall RPC_HDR.\n"));
//		set_incoming_fault(p);
		return -1;
	}

	/*
	 * Validate the RPC header.
	 */

//0430  	if(p->hdr.major != 5 && p->hdr.minor != 0) {
//0430		DEBUG(0,("unmarshall_rpc_header: invalid major/minor numbers in RPC_HDR.\n"));
//0430		set_incoming_fault(p);
//0430		return -1;
//0430	}

	/*
	 * If there is no data in the incoming buffer and it's a requst pdu then
	 * ensure that the FIRST flag is set. If not then we have
	 * a stream missmatch.
	 */

//	if((p->hdr.pkt_type == RPC_REQUEST) && (prs_offset(&p->in_data.data) == 0) && !(p->hdr.flags & RPC_FLG_FIRST)) {
//0430		DEBUG(0,("unmarshall_rpc_header: FIRST flag not set in first PDU !\n"));
//0430		set_incoming_fault(p);
//		return -1;
//	}

	/*
	 * Ensure that the pdu length is sane.
	 */

//0430	if((p->hdr.frag_len < RPC_HEADER_LEN) || (p->hdr.frag_len > MAX_PDU_FRAG_LEN)) {
//0430		DEBUG(0,("unmarshall_rpc_header: assert on frag length failed.\n"));
//0430		set_incoming_fault(p);
//0430		return -1;
//0430	}

//0430	DEBUG(10,("unmarshall_rpc_header: type = %u, flags = %u\n", (unsigned int)p->hdr.pkt_type,
//0430			(unsigned int)p->hdr.flags ));

	/*
	 * Adjust for the header we just ate.
	 */
	p->in_data.pdu_received_len = 0;
	p->in_data.pdu_needed_len = (uint32)p->hdr.frag_len - RPC_HEADER_LEN;

	/*
	 * Null the data we just ate.
	 */

	memset((char *)&p->in_data.current_in_pdu[0], '\0', RPC_HEADER_LEN);

	return 0; /* No extra data processed. */
}

/****************************************************************************
 Processes a request pdu. This will do auth processing if needed, and
 appends the data into the complete stream if the LAST flag is not set.
****************************************************************************/

static int process_request_pdu(pipes_struct *p, prs_struct *rpc_in_p)
{
	int auth_verify = IS_BITS_SET_ALL(p->ntlmssp_chal_flags, NTLMSSP_NEGOTIATE_SIGN);
	uint32 data_len = p->hdr.frag_len - RPC_HEADER_LEN - RPC_HDR_REQ_LEN -
				(auth_verify ? RPC_HDR_AUTH_LEN : 0) - p->hdr.auth_len;

//0430	if(!p->pipe_bound) {
//0430		DEBUG(0,("process_request_pdu: rpc request with no bind.\n"));
//0430		set_incoming_fault(p);
//0430		return False;
//0430	}

	/*
	 * Check if we need to do authentication processing.
	 * This is only done on requests, not binds.
	 */

	/*
	 * Read the RPC request header.
	 */

	if(!smb_io_rpc_hdr_req("req", &p->hdr_req, rpc_in_p, 0)) {
//0430		DEBUG(0,("process_request_pdu: failed to unmarshall RPC_HDR_REQ.\n"));
		set_incoming_fault(p);
		return False;
	}

//0430	if(p->ntlmssp_auth_validated && !api_pipe_auth_process(p, rpc_in_p)) {
//0430		DEBUG(0,("process_request_pdu: failed to do auth processing.\n"));
//0430		set_incoming_fault(p);
//0430		return False;
//0430	}

//0508	if (p->ntlmssp_auth_requested && !p->ntlmssp_auth_validated) {

		/*
		 * Authentication _was_ requested and it already failed.
		 */

//0502		DEBUG(0,("process_request_pdu: RPC request received on pipe %s where \
//0502authentication failed. Denying the request.\n", p->name));
//		set_incoming_fault(p);
//	return False;
//    }

	/*
	 * Check the data length doesn't go over the 1Mb limit.
	 */
	
//0509	if(prs_data_size(&p->in_data.data) + data_len > 1024*1024) {
//0430		DEBUG(0,("process_request_pdu: rpc data buffer too large (%u) + (%u)\n",
//0430				(unsigned int)prs_data_size(&p->in_data.data), (unsigned int)data_len ));
//0509		set_incoming_fault(p);
//0509		return False;
//0509	}

	/*
	 * Append the data portion into the buffer and return.
	 */

	{
		char *data_from = prs_data_p(rpc_in_p) + prs_offset(rpc_in_p);

		if(!prs_append_data(&p->in_data.data, data_from, data_len)) {
//0430			DEBUG(0,("process_request_pdu: Unable to append data size %u to parse buffer of size %u.\n",
//0430					(unsigned int)data_len, (unsigned int)prs_data_size(&p->in_data.data) ));
			set_incoming_fault(p);
			return False;
		}

	}

	if(p->hdr.flags & RPC_FLG_LAST) {
		int ret = False;
		/*
		 * Ok - we finally have a complete RPC stream.
		 * Call the rpc command to process it.
		 */

		/*
		 * Set the parse offset to the start of the data and set the
		 * prs_struct to UNMARSHALL.
		 */

		prs_set_offset(&p->in_data.data, 0);
		prs_switch_type(&p->in_data.data, UNMARSHALL);

		/*
		 * Process the complete data stream here.
		 */

		if(pipe_init_outgoing_data(&p->out_data))
			ret = api_pipe_request(p);

		/*
		 * We have consumed the whole data stream. Set back to
		 * marshalling and set the offset back to the start of
		 * the buffer to re-use it (we could also do a prs_mem_free()
		 * and then re_init on the next start of PDU. Not sure which
		 * is best here.... JRA.
		 */

		prs_switch_type(&p->in_data.data, MARSHALL);
		prs_set_offset(&p->in_data.data, 0);
		return ret;
	}

	return True;
}
//copy from spipe.c 5/4/2001
/*******************************************************************
 Respond to a pipe bind request.
*******************************************************************/

int api_pipe_bind_req(pipes_struct *p, prs_struct *rpc_in_p)
{
	RPC_HDR_BA hdr_ba;
	RPC_HDR_RB hdr_rb;
//	RPC_HDR_AUTH auth_info;
	uint16 assoc_gid;
	fstring ack_pipe_name;
//0508	char ack_pipe_name[128];
	prs_struct out_hdr_ba;
	prs_struct out_auth;
	prs_struct outgoing_rpc;
	int i = 0;
	int auth_len = 0;
	enum RPC_PKT_TYPE reply_pkt_type;
	p->ntlmssp_auth_requested = False;
//0508	DEBUG(5,("api_pipe_bind_req: decode request. %d\n", __LINE__));

	/*
	 * Try and find the correct pipe name to ensure
	 * that this is a pipe name we support.
	 */

	for (i = 0; api_fd_commands[i].pipe_clnt_name; i++) {
 //0508		if (strequal(api_fd_commands[i].pipe_clnt_name, p->name) &&
//		    api_fd_commands[i].fn != NULL) {
		if (!strcmp(api_fd_commands[i].pipe_clnt_name, p->name)) {
//0508			DEBUG(3,("api_pipe_bind_req: \\PIPE\\%s -> \\PIPE\\%s\n",
//0508				   api_fd_commands[i].pipe_clnt_name,
//0508				   api_fd_commands[i].pipe_srv_name));
			fstrcpy(p->pipe_srv_name, api_fd_commands[i].pipe_srv_name);
			break;
		}
	}

//	if (api_fd_commands[i].fn == NULL) {
//0508		DEBUG(3,("api_pipe_bind_req: Unknown pipe name %s in bind request.\n",
//0508			p->name ));
//		if(!setup_bind_nak(p))
//			return False;
//		return True;
//	}

	/* decode the bind request */
	if(!smb_io_rpc_hdr_rb("", &hdr_rb, rpc_in_p, 0))  {
//0508		DEBUG(0,("api_pipe_bind_req: unable to unmarshall RPC_HDR_RB struct.\n"));
		return False;
	}

	/*
	 * Check if this is an authenticated request.
	 */

	if (p->hdr.auth_len != 0) {
		RPC_AUTH_VERIFIER auth_verifier;
		RPC_AUTH_NTLMSSP_NEG ntlmssp_neg;

		/* 
		 * Decode the authentication verifier.
		 */

//		if(!smb_io_rpc_hdr_auth("", &auth_info, rpc_in_p, 0)) {
//0508			DEBUG(0,("api_pipe_bind_req: unable to unmarshall RPC_HDR_AUTH struct.\n"));
//			return False;
//		}

		/*
		 * We only support NTLMSSP_AUTH_TYPE requests.
		 */

//0709		if(auth_info.auth_type != NTLMSSP_AUTH_TYPE) {
//0508			DEBUG(0,("api_pipe_bind_req: unknown auth type %x requested.\n",
//0508				auth_info.auth_type ));
//0709			return False;
//		}

//		if(!smb_io_rpc_auth_verifier("", &auth_verifier, rpc_in_p, 0)) {
//0508			DEBUG(0,("api_pipe_bind_req: unable to unmarshall RPC_HDR_AUTH struct.\n"));
//			return False;
//		}

//0508		if(!strequal(auth_verifier.signature, "NTLMSSP")) {
		if(!strcmp(auth_verifier.signature, "NTLMSSP")) {
//0508			DEBUG(0,("api_pipe_bind_req: auth_verifier.signature != NTLMSSP\n"));
			return False;
		}

		if(auth_verifier.msg_type != NTLMSSP_NEGOTIATE) {
//0508			DEBUG(0,("api_pipe_bind_req: auth_verifier.msg_type (%d) != NTLMSSP_NEGOTIATE\n",
//0508				auth_verifier.msg_type));
			return False;
		}

//		if(!smb_io_rpc_auth_ntlmssp_neg("", &ntlmssp_neg, rpc_in_p, 0)) {
//0508			DEBUG(0,("api_pipe_bind_req: Failed to unmarshall RPC_AUTH_NTLMSSP_NEG.\n"));
//			return False;
//		}

		p->ntlmssp_chal_flags = SMBD_NTLMSSP_NEG_FLAGS;
		p->ntlmssp_auth_requested = True;
	}

	switch(p->hdr.pkt_type) {
		case RPC_BIND:
			/* name has to be \PIPE\xxxxx */
			fstrcpy(ack_pipe_name, "\\PIPE\\");
			fstrcat(ack_pipe_name, p->pipe_srv_name);
			reply_pkt_type = RPC_BINDACK;
			break;
		case RPC_ALTCONT:
			/* secondary address CAN be NULL
			 * as the specs say it's ignored.
			 * It MUST NULL to have the spoolss working.
			 */
			fstrcpy(ack_pipe_name,"");
			reply_pkt_type = RPC_ALTCONTRESP;
			break;
		default:
			return False;
	}

//0508	DEBUG(5,("api_pipe_bind_req: make response. %d\n", __LINE__));

	/* 
	 * Marshall directly into the outgoing PDU space. We
	 * must do this as we need to set to the bind response
	 * header and are never sending more than one PDU here.
	 */

	prs_init( &outgoing_rpc, 0, 4, MARSHALL);
	prs_give_memory( &outgoing_rpc, (char *)p->out_data.current_pdu, sizeof(p->out_data.current_pdu), False);

	/*
	 * Setup the memory to marshall the ba header, and the
	 * auth footers.
	 */

	if(!prs_init(&out_hdr_ba, 1024, 4, MARSHALL)) {
//0508		DEBUG(0,("api_pipe_bind_req: malloc out_hdr_ba failed.\n"));
		return False;
	}

	if(!prs_init(&out_auth, 1024, 4, MARSHALL)) {
//0508		DEBUG(0,("pi_pipe_bind_req: malloc out_auth failed.\n"));
		prs_mem_free(&out_hdr_ba);
		return False;
	}

	if (p->ntlmssp_auth_requested)
		assoc_gid = 0x7a77;
	else
//Jesse		assoc_gid = hdr_rb.bba.assoc_gid;
		assoc_gid = hdr_rb.bba.assoc_gid ? hdr_rb.bba.assoc_gid : 0x53f0;

	/*
	 * Create the bind response struct.
	 */

	init_rpc_hdr_ba(&hdr_ba,
			MAX_PDU_FRAG_LEN,
			MAX_PDU_FRAG_LEN,
			assoc_gid,
			ack_pipe_name,
			0x1, 0x0, 0x0,
			&hdr_rb.transfer);

	/*
	 * and marshall it.
	 */

	if(!smb_io_rpc_hdr_ba("", &hdr_ba, &out_hdr_ba, 0)) {
//0508		DEBUG(0,("api_pipe_bind_req: marshalling of RPC_HDR_BA failed.\n"));
		goto err_exit;
	}

	/*
	 * Now the authentication.
	 */

	if (p->ntlmssp_auth_requested) {
		RPC_AUTH_VERIFIER auth_verifier;
		RPC_AUTH_NTLMSSP_CHAL ntlmssp_chal;

//		generate_random_buffer(p->challenge, 8, False);

		/*** Authentication info ***/

//		init_rpc_hdr_auth(&auth_info, NTLMSSP_AUTH_TYPE, NTLMSSP_AUTH_LEVEL, RPC_HDR_AUTH_LEN, 1);
//		if(!smb_io_rpc_hdr_auth("", &auth_info, &out_auth, 0)) {
//0508			DEBUG(0,("api_pipe_bind_req: marshalling of RPC_HDR_AUTH failed.\n"));
//			goto err_exit;
//		}

		/*** NTLMSSP verifier ***/

//		init_rpc_auth_verifier(&auth_verifier, "NTLMSSP", NTLMSSP_CHALLENGE);
//		if(!smb_io_rpc_auth_verifier("", &auth_verifier, &out_auth, 0)) {
//0508			DEBUG(0,("api_pipe_bind_req: marshalling of RPC_AUTH_VERIFIER failed.\n"));
//			goto err_exit;
//		}

		/* NTLMSSP challenge ***/

//0509		init_rpc_auth_ntlmssp_chal(&ntlmssp_chal, p->ntlmssp_chal_flags, p->challenge);
//		if(!smb_io_rpc_auth_ntlmssp_chal("", &ntlmssp_chal, &out_auth, 0)) {
//0508			DEBUG(0,("api_pipe_bind_req: marshalling of RPC_AUTH_NTLMSSP_CHAL failed.\n"));
//			goto err_exit;
//		}

		/* Auth len in the rpc header doesn't include auth_header. */
		auth_len = prs_offset(&out_auth) - RPC_HDR_AUTH_LEN;
	}

	/*
	 * Create the header, now we know the length.
	 */

	init_rpc_hdr(&p->hdr, reply_pkt_type, RPC_FLG_FIRST | RPC_FLG_LAST,
			p->hdr.call_id,
			RPC_HEADER_LEN + prs_offset(&out_hdr_ba) + prs_offset(&out_auth),
			auth_len);

	/*
	 * Marshall the header into the outgoing PDU.
	 */

	if(!smb_io_rpc_hdr("", &p->hdr, &outgoing_rpc, 0)) {
//0508		DEBUG(0,("pi_pipe_bind_req: marshalling of RPC_HDR failed.\n"));
		goto err_exit;
	}

	/*
	 * Now add the RPC_HDR_BA and any auth needed.
	 */

	if(!prs_append_prs_data( &outgoing_rpc, &out_hdr_ba)) {
//0508		DEBUG(0,("api_pipe_bind_req: append of RPC_HDR_BA failed.\n"));
		goto err_exit;
	}

	if(p->ntlmssp_auth_requested && !prs_append_prs_data( &outgoing_rpc, &out_auth)) {
//0508		DEBUG(0,("api_pipe_bind_req: append of auth info failed.\n"));
		goto err_exit;
	}

	if(!p->ntlmssp_auth_requested)
		p->pipe_bound = True;

	/*
	 * Setup the lengths for the initial reply.
	 */

	p->out_data.data_sent_length = 0;
	p->out_data.current_pdu_len = prs_offset(&outgoing_rpc);
	p->out_data.current_pdu_sent = 0;

	prs_mem_free(&out_hdr_ba);
	prs_mem_free(&out_auth);

	return True;

  err_exit:

	prs_mem_free(&out_hdr_ba);
	prs_mem_free(&out_auth);
	return False;
}


/****************************************************************************
 Processes a finished PDU stored in current_in_pdu. The RPC_HEADER has
 already been parsed and stored in p->hdr.
****************************************************************************/

int process_complete_pdu(pipes_struct *p)
{
	prs_struct rpc_in;
	uint32 data_len = p->in_data.pdu_received_len;
	char *data_p = (char *)&p->in_data.current_in_pdu[0];
	int reply = False;

//	if(p->fault_state) {
//0502		DEBUG(10,("process_complete_pdu: pipe %s in fault state.\n",
//0502			p->name ));
//		set_incoming_fault(p);
//0430	setup_fault_pdu(p);
//		return (int)data_len;
//	}

	prs_init( &rpc_in, 0, 4, UNMARSHALL);
	prs_give_memory( &rpc_in, data_p, (uint32)data_len, False);

//0430	DEBUG(10,("process_complete_pdu: processing packet type %u\n",
//0430			(unsigned int)p->hdr.pkt_type ));

	switch (p->hdr.pkt_type) {
		case RPC_BIND:
		case RPC_ALTCONT:
			/*
			 * We assume that a pipe bind is only in one pdu.
			 */
			if(pipe_init_outgoing_data(&p->out_data))
				reply = api_pipe_bind_req(p, &rpc_in);
			break;
		case RPC_BINDRESP:
			/*
			 * We assume that a pipe bind_resp is only in one pdu.
			 */
			if(pipe_init_outgoing_data(&p->out_data))
//0504				reply = api_pipe_bind_auth_resp(p, &rpc_in);
			break;
		case RPC_REQUEST:
			reply = process_request_pdu(p, &rpc_in);
			break;
		default:
//0502			DEBUG(0,("process_complete_pdu: Unknown rpc type = %u received.\n", (unsigned int)p->hdr.pkt_type ));
			break;
	}

	if (!reply) {
//0430		DEBUG(3,("process_complete_pdu: DCE/RPC fault sent on pipe %s\n", p->pipe_srv_name));
//0430		set_incoming_fault(p);
//0504		setup_fault_pdu(p);
	} else {
		/*
		 * Reset the lengths. We're ready for a new pdu.
		 */
		p->in_data.pdu_needed_len = 0;
		p->in_data.pdu_received_len = 0;
	}

	return (int)data_len;
}

/****************************************************************************
 Accepts incoming data on an rpc pipe. Processes the data in pdu sized units.
****************************************************************************/

static int process_incoming_data(pipes_struct *p, char *data, uint32 n)
{
	uint32 data_to_copy = MIN(n, MAX_PDU_FRAG_LEN - p->in_data.pdu_received_len);
//0504 uint32 data_to_copy = n;

//0430	DEBUG(10,("process_incoming_data: Start: pdu_received_len = %u, pdu_needed_len = %u, incoming data = %u\n",
//0430		(unsigned int)p->in_data.pdu_received_len, (unsigned int)p->in_data.pdu_needed_len,
//0430		(unsigned int)n ));

	if(data_to_copy == 0) {
		/*
		 * This is an error - data is being received and there is no
		 * space in the PDU. Free the received data and go into the fault state.
		 */
//0430		DEBUG(0,("process_incoming_data: No space in incoming pdu buffer. Current size = %u \
//0430incoming data size = %u\n", (unsigned int)p->in_data.pdu_received_len, (unsigned int)n ));		set_incoming_fault(p);
		return -1;
	}

	/*
	 * If we have no data already, wait until we get at least a RPC_HEADER_LEN
	 * number of bytes before we can do anything.
	 */

	if((p->in_data.pdu_needed_len == 0) && (p->in_data.pdu_received_len < RPC_HEADER_LEN)) {
		/*
		 * Always return here. If we have more data then the RPC_HEADER
		 * will be processed the next time around the loop.
		 */
		return fill_rpc_header(p, data, data_to_copy);
	}

	/*
	 * At this point we know we have at least an RPC_HEADER_LEN amount of data
	 * stored in current_in_pdu.
	 */

	/*
	 * If pdu_needed_len is zero this is a new pdu. 
	 * Unmarshall the header so we know how much more
	 * data we need, then loop again.
	 */

	if(p->in_data.pdu_needed_len == 0)
		return unmarshall_rpc_header(p);

	/*
	 * Ok - at this point we have a valid RPC_HEADER in p->hdr.
	 * Keep reading until we have a full pdu.
	 */

	data_to_copy = MIN(data_to_copy, p->in_data.pdu_needed_len);

	/*
	 * Copy as much of the data as we need into the current_in_pdu buffer.
	 */

	memcpy( (char *)&p->in_data.current_in_pdu[p->in_data.pdu_received_len], data, data_to_copy);
	p->in_data.pdu_received_len += data_to_copy;

	/*
	 * Do we have a complete PDU ?
	 */

	if(p->in_data.pdu_received_len == p->in_data.pdu_needed_len)
		return process_complete_pdu(p);

//0502	DEBUG(10,("process_incoming_data: not a complete PDU yet. pdu_received_len = %u, pdu_needed_len = %u\n",
//0502		(unsigned int)p->in_data.pdu_received_len, (unsigned int)p->in_data.pdu_needed_len ));

	return (int)data_to_copy;

}

/****************************************************************************
 Accepts incoming data on an rpc pipe.
****************************************************************************/

int write_to_pipe(pipes_struct *p, char *data, uint32 n)
{
	uint32 data_left = n;

//0502	DEBUG(6,("write_to_pipe: %x", p->pnum));

//0502	DEBUG(6,(" name: %s open: %s len: %d\n",
//0502		 p->name, BOOLSTR(p->open), (int)n));

//0430	dump_data(50, data, n);

	while(data_left) {
		int data_used;

//0502		DEBUG(10,("write_to_pipe: data_left = %u\n", (unsigned int)data_left ));

		data_used = process_incoming_data(p, data, data_left);

//0502		DEBUG(10,("write_to_pipe: data_used = %d\n", (int)data_used ));

		if(data_used < 0)
			return -1;

		data_left -= data_used;
		data += data_used;
	}       

	return n;
}


/****************************************************************************
 Replyies to a request to read data from a pipe.

 Headers are interspersed with the data at PDU intervals. By the time
 this function is called, the start of the data could possibly have been
 read by an SMBtrans (file_offset != 0).

 Calling create_rpc_reply() here is a hack. The data should already
 have been prepared into arrays of headers + data stream sections.

 ****************************************************************************/

int read_from_pipe(pipes_struct *p, char *data, uint32 n)
{
	uint32 pdu_remaining = 0;
	int data_returned = 0;

//0507	if (!p || !p->open) {
//0502		DEBUG(0,("read_from_pipe: pipe not open\n"));
//		return -1;              
//	}

//0502	DEBUG(6,("read_from_pipe: %x", p->pnum));

//0502	DEBUG(6,(" name: %s len: %u\n", p->name, (unsigned int)n));

	/*
	 * We cannot return more than one PDU length per
	 * read request.
	 */

//0430	if(n > MAX_PDU_FRAG_LEN) {
//0430		DEBUG(0,("read_from_pipe: loo large read (%u) requested on pipe %s. We can \
//0430only service %d sized reads.\n", (unsigned int)n, p->name, MAX_PDU_FRAG_LEN ));
//0430		return -1;
//0430	}

	/*
	 * Determine if there is still data to send in the
	 * pipe PDU buffer. Always send this first. Never
	 * send more than is left in the current PDU. The
	 * client should send a new read request for a new
	 * PDU.
	 */

	if((pdu_remaining = p->out_data.current_pdu_len - p->out_data.current_pdu_sent) > 0) {
		data_returned = (int)MIN(n, pdu_remaining);
//0509		data_returned = 68;
//0502		DEBUG(10,("read_from_pipe: %s: current_pdu_len = %u, current_pdu_sent = %u \
//0502returning %d bytes.\n", p->name, (unsigned int)p->out_data.current_pdu_len, 
//0502			(unsigned int)p->out_data.current_pdu_sent, (int)data_returned));

		memcpy( data, &p->out_data.current_pdu[p->out_data.current_pdu_sent], (uint32)data_returned);
		p->out_data.current_pdu_sent += (uint32)data_returned;
		return data_returned;
	}

	/*
	 * At this point p->current_pdu_len == p->current_pdu_sent (which
	 * may of course be zero if this is the first return fragment.
	 */

//0430	DEBUG(10,("read_from_pipe: %s: fault_state = %d : data_sent_length \
//0430= %u, prs_offset(&p->out_data.rdata) = %u.\n",
//0430		p->name, (int)p->fault_state, (unsigned int)p->out_data.data_sent_length, (unsigned int)prs_offset(&p->out_data.rdata) ));

	if(p->out_data.data_sent_length >= prs_offset(&p->out_data.rdata)) {
		/*
		 * We have sent all possible data. Return 0.
		 */
		return 0;
	}

	/*
	 * We need to create a new PDU from the data left in p->rdata.
	 * Create the header/data/footers. This also sets up the fields
	 * p->current_pdu_len, p->current_pdu_sent, p->data_sent_length
	 * and stores the outgoing PDU in p->current_pdu.
	 */

	if(!create_next_pdu(p)) {
//0430		DEBUG(0,("read_from_pipe: %s: create_next_pdu failed.\n",
//0430			 p->name));
		return -1;
	}

	data_returned = MIN(n, p->out_data.current_pdu_len);


	memcpy( data, p->out_data.current_pdu, (uint32)data_returned);
	p->out_data.current_pdu_sent += (uint32)data_returned;
	return data_returned;
}

/****************************************************************************
 Wait device state on a pipe. Exactly what this is for is unknown...
****************************************************************************/

int wait_rpc_pipe_hnd_state(pipes_struct *p, uint16 priority)
{
	if (p == NULL)
		return False;

	if (p->open) {
//0430		DEBUG(3,("wait_rpc_pipe_hnd_state: Setting pipe wait state priority=%x on pipe (name=%s)\n",
//0430			 priority, p->name));

		p->priority = priority;
		
		return True;
	} 

//0430	DEBUG(3,("wait_rpc_pipe_hnd_state: Error setting pipe wait state priority=%x (name=%s)\n",
//0430		 priority, p->name));
	return False;
}


/****************************************************************************
 Set device state on a pipe. Exactly what this is for is unknown...
****************************************************************************/

int set_rpc_pipe_hnd_state(pipes_struct *p, uint16 device_state)
{
	if (p == NULL)
		return False;

	if (p->open) {
//0502		DEBUG(3,("set_rpc_pipe_hnd_state: Setting pipe device state=%x on pipe (name=%s)\n",
//0502			 device_state, p->name));

		p->device_state = device_state;
		
		return True;
	} 

//0430	DEBUG(3,("set_rpc_pipe_hnd_state: Error setting pipe device state=%x (name=%s)\n",
//0430		 device_state, p->name));
	return False;
}


/****************************************************************************
 Close an rpc pipe.
****************************************************************************/

int close_rpc_pipe_hnd(pipes_struct *p, int threadid)
{
	pipes_struct *prev, *next;
	
	if (!p) {
//0502		DEBUG(0,("Invalid pipe in close_rpc_pipe_hnd\n"));
		return False;
	}

	prs_mem_free(&p->out_data.rdata);
	prs_mem_free(&p->in_data.data);

//0430	bitmap_clear(bmap, p->pnum - pipe_handle_offset);

//0627	pipes_open--;

//0502	DEBUG(4,("closed pipe name %s pnum=%x (pipes_open=%d)\n", 
//0502		 p->name, p->pnum, pipes_open));  
    prev = p->prev;
    next = p->next;
	DLIST_REMOVE(Pipes[threadid], p);	
//2/8/2002	ZERO_STRUCTP(p);
	memset((char *)p ,0, sizeof(pipes_struct));		
	free(p);
	if ((prev == NULL) && (next == NULL)) // Ron Add 3/6/2002
		Pipes[threadid] = NULL;
	return True;
}

/****************************************************************************
 Find an rpc pipe given a pipe handle in a buffer and an offset.
****************************************************************************/

pipes_struct *get_rpc_pipe_p(char *buf, int where, int threadid)
{
	int pnum = SVAL(buf,where);

//0507	if (chain_p)
//0507		return chain_p;

	return get_rpc_pipe(pnum, threadid);
}

/****************************************************************************
 Find an rpc pipe given a pipe handle.
****************************************************************************/

pipes_struct *get_rpc_pipe(int pnum, int threadid)
{
	pipes_struct *p;

//0502	DEBUG(4,("search for pipe pnum=%x\n", pnum));

//0502	for (p=Pipes;p;p=p->next)
//0502		DEBUG(5,("pipe name %s pnum=%x (pipes_open=%d)\n", 
//0502			  p->name, p->pnum, pipes_open));  

	for (p=Pipes[threadid];p;p=p->next) {
		if (p->pnum == pnum) {
//0507			chain_p = p;
			return p;
		}
	}

	return NULL;
}




/* Refer to conn.c. If the Client interruptd by some reason without 
		send close_pipe SMB packet, then the Pipes' Memory can not br free.
		So refer to Connections. When this Thread timeout, I use the function
		pipes_close_all() to free all pipes had not 
		free Memory. .............7/19/2001 by ron */ 
	   
/****************************************************************************
close all pipe structures
****************************************************************************/
 
void pipe_close_all(int threadid)
{
	pipes_struct *p, *nextp;
	for (p=Pipes[threadid];p;p=nextp) {
		nextp=p->next;
		close_rpc_pipe_hnd(p, threadid);
	}
}



