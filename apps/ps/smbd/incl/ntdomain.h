/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell 1992-1997
   Copyright (C) Luke Kenneth Casson Leighton 1996-1997
   Copyright (C) Paul Ashton 1997
   
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

#ifndef _NT_DOMAIN_H /* _NT_DOMAIN_H */
#define _NT_DOMAIN_H 

/* 
 * A bunch of stuff that was put into smb.h
 * in the NTDOM branch - it didn't belong there.
 */
 
typedef struct _prs_struct 
{
	int io; /* parsing in or out of data stream */
	/* 
	 * If the (incoming) data is big-endian. On output we are
	 * always little-endian.
	 */ 
	int bigendian_data;
	uint8 align; /* data alignment */
	int is_dynamic; /* Do we own this memory or not ? */
	uint32 data_offset; /* Current working offset into data. */
	uint32 buffer_size; /* Current size of the buffer. */
	char *data_p; /* The buffer itself. */
} prs_struct;

/*
 * Defines for io member of prs_struct.
 */

#define MARSHALL 0
#define UNMARSHALL 1

#define MARSHALLING(ps) (!(ps)->io)
#define UNMARSHALLING(ps) ((ps)->io)

typedef struct _output_data {
	/* 
	 * Raw RPC output data. This does not include RPC headers or footers.
	 */
	prs_struct rdata;

	/* The amount of data sent from the current rdata struct. */
	uint32 data_sent_length;

	/* 
	 * The current PDU being returned. This inclues
	 * headers, data and authentication footer.
	 */
//0430        unsigned char current_pdu[MAX_PDU_FRAG_LEN];
        unsigned char current_pdu[384];

	/* The amount of data in the current_pdu buffer. */
	uint32 current_pdu_len;

	/* The amount of data sent from the current PDU. */
	uint32 current_pdu_sent;
} output_data;

typedef struct _input_data {
    /*
     * This is the current incoming pdu. The data here
     * is collected via multiple writes until a complete
     * pdu is seen, then the data is copied into the in_data
     * structure. The maximum size of this is 0x1630 (MAX_PDU_FRAG_LEN).
     */
//0430       unsigned char current_in_pdu[MAX_PDU_FRAG_LEN];
       unsigned char current_in_pdu[384];

    /*
     * The amount of data needed to complete the in_pdu.
     * If this is zero, then we are at the start of a new
     * pdu.
     */
    uint32 pdu_needed_len;

    /*
     * The amount of data received so far in the in_pdu.
     * If this is zero, then we are at the start of a new
     * pdu.
     */
    uint32 pdu_received_len;

    /*
     * This is the collection of input data with all
     * the rpc headers and auth footers removed.
     * The maximum length of this (1Mb) is strictly enforced.
     */
    prs_struct data;
} input_data;

typedef struct pipes_struct
{
	struct pipes_struct *next, *prev;
	int pnum;
//0509	connection_struct *conn;
	uint16 vuid;
	int open; /* open connection */
	uint16 device_state;
	uint16 priority;
	fstring name;
	fstring pipe_srv_name;

    RPC_HDR hdr; /* Incoming RPC header. */
        RPC_HDR_REQ hdr_req; /* Incoming request header. */

	uint32 ntlmssp_chal_flags; /* Client challenge flags. */
	int ntlmssp_auth_requested; /* If the client wanted authenticated rpc. */
	int ntlmssp_auth_validated; /* If the client *got* authenticated rpc. */
//0509	unsigned char challenge[8];
//0509	unsigned char ntlmssp_hash[258];
//0509	uint32 ntlmssp_seq_num;

	/*
	 * Windows user info.
	 */
//	fstring user_name;
//	fstring domain;
//	fstring wks;

	/*
	 * Unix user name and credentials.
	 */
//0509	fstring unix_user_name;
//0509        uid_t uid;
//0509        gid_t gid;

	/*
	 * Set to true when an RPC bind has been done on this pipe.
	 */

	int pipe_bound;

	/*
	 * Set to true when we should return fault PDU's for everything.
	 */

	int fault_state;

	/*
	 * Struct to deal with multiple pdu inputs.
	 */

	input_data in_data;

	/*
	 * Struct to deal with multiple pdu outputs.
	 */

	output_data  out_data;

	/* When replying to an SMBtrans, this is the maximum amount of
           data that can be sent in the initial reply. */
	int max_trans_reply;
} pipes_struct;

struct api_struct
{  
  char *name;
  uint8 opnum;
  int (*fn) (uint16 vuid, prs_struct*, prs_struct*);
};

typedef struct
{  
	uint32 rid;
	char *name;

} rid_name;

struct acct_info
{
    fstring acct_name; /* account name */
    uint32 smb_userid; /* domain-relative RID */
};

#endif /* _NT_DOMAIN_H */
