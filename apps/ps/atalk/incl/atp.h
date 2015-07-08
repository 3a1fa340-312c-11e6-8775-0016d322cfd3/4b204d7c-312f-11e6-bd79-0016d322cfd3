#ifndef _ATP_H
#define _ATP_H

/* ATP packet format

 |----------------|
 | link header    |
 | {EtherTalk	  |
 |  = 14 + 8(SNAP)|
 |	}			  |
 |----------------|
 | DDP header     |
 |    (13 )       |
 |   type = 3     |
 |----------------|	  ------------------------------------------------------+--
 | control info   | --> bits 7,6: function code                             |
 |----------------|            5: XO bit                                    |
 | bitmap/seq no. |            4: EOM bit
 |----------------|            3: STS bit
 | TID (MSB)      |        2,1,0: release timer code (ignored under phase I)
 |----------------|
 | TID (LSB)      |
 |----------------|                    (8 BYTES)
 | user bytes (4) |                                                         |
 |----------------|  -------------------------------------------------------+--
 | data (0-578)   |
 |      ...       |
 |----------------|

 MAX PACKET LEN = 22 + 13 + 8 + 578 = 621 #
 BUT PAP only need 512 Bytes ==> so max packet size = 555 #
*/

#define ATP_DDPTYPE   3

struct atphdr {
    uint8	atphd_ctrlinfo;	/* control information */
    uint8	atphd_bitmap;   /* bitmap or sequence number */
    uint16	atphd_tid;	/* transaction id. */
};

// ATP protocol parameters
//
#define ATP_MAXDATA	(578+4)	// maximum ATP data size
#define ATP_BUFSIZ	587     // maximum packet size

#define ATP_HDRSIZE	5       // includes DDP type field

#define ATP_TRELMASK 0x07   // mask all but TREL
#define ATP_RELTIME	 30	    // base release timer (in secs)

#define ATP_TREL30	0x0	    // release time codes
#define ATP_TREL1M	0x1	    // these are passed in flags of
#define ATP_TREL2M	0x2	    // atp_sreq call, and set in the
#define ATP_TREL4M	0x3	    // packet control info.
#define ATP_TREL8M	0x4

#define ATP_TRIES_INFINITE	-1	// for atp_sreq, etc

#define	IOV_BASE(in,index)  ((in).iobuf[(index)].iov_base)
#define IOV_LEN(in,index)	((in).iobuf[(index)].iov_len)

struct atpxobuf {
    uint16 atpxo_tid;
    uint32 atpxo_tv;	   //timeval (ticks)
    int	   atpxo_reltime;
    struct atpbuf  *atpxo_packet[8];
};

struct atpbuf {
	struct atpbuf *atpbuf_next;		// next buffer in chain
	short         atpbuf_dlen;		// data length <= ATP_BUFSIZ
	struct sockaddr_at atpbuf_addr;	// net address sent/recvd
	union {
		char   atpbuf_data[ ATP_BUFSIZ ]; // the data
		struct atpxobuf	atpbuf_xo;        // for XO requests
	} atpbuf_info;
};

struct atp_handle {
    int			atph_socket;		/* ddp socket */
    struct sockaddr_at	atph_saddr;		/* address */
    uint16		atph_tid;		/* last tid used */
    uint16		atph_rtid;		/* last received (rreq) */
    uint8		atph_rxo;		/* XO flag from last rreq */
    int			atph_rreltime;		/* release time (secs) */
    struct atpbuf	*atph_sent;		/* packets we send (XO) */
    struct atpbuf	*atph_queue;		/* queue of pending packets */
    int			atph_reqtries;		/* retry count for request */
    int16		atph_reqto;		   // retry timeout for request (secs)
	int			atph_rrespcount;	/* expected # of responses */
    uint8		atph_rbitmap;		/* bitmap for request */
    struct atpbuf	*atph_reqpkt;		/* last request packet */
    uint32 atph_reqtv;		// when we last sent request (timeval, ticks)
    struct atpbuf	*atph_resppkt[8];	/* response to request */
};

typedef struct atp_handle *ATP;

#define atp_sockaddr( h )	(&(h)->atph_saddr)
#define atp_fileno(x)		((x)->atph_socket)

struct sreq_st {
    char	    *atpd_data;		/* request data */
    int		    atpd_dlen;
    int		    atpd_tries;		/* max. retry count */
    int		    atpd_to;		/* retry interval */
};

struct iovec {
	char  *iov_base;  //A pointer to a buffer
	int16  iov_len;	  //The size of the buffer to which iov_base points
};

struct rres_st {
    struct iovec    *atpd_iov;		/* for response */
    int		    atpd_iovcnt;
};

struct rreq_st {
    char	    *atpd_data;		/* request data */
    int		    atpd_dlen;
};

struct sres_st {
    struct iovec    *atpd_iov;		/* for response */
    int		    atpd_iovcnt;
};

struct atp_block {
    struct sockaddr_at	*atp_saddr;		/* from/to address */
    union {
	struct sreq_st	sreqdata;
#define atp_sreqdata	atp_data.sreqdata.atpd_data
#define atp_sreqdlen	atp_data.sreqdata.atpd_dlen
#define atp_sreqtries	atp_data.sreqdata.atpd_tries
#define atp_sreqto	atp_data.sreqdata.atpd_to

	struct rres_st	rresdata;
#define atp_rresiov	atp_data.rresdata.atpd_iov
#define atp_rresiovcnt	atp_data.rresdata.atpd_iovcnt

	struct rreq_st	rreqdata;
#define atp_rreqdata	atp_data.rreqdata.atpd_data
#define atp_rreqdlen	atp_data.rreqdata.atpd_dlen

	struct sres_st	sresdata;
#define atp_sresiov	atp_data.sresdata.atpd_iov
#define atp_sresiovcnt	atp_data.sresdata.atpd_iovcnt
    } atp_data;
    uint8		atp_bitmap;	/* response buffer bitmap */
};


/* flags for ATP options (and control byte)
*/
#define ATP_STS		(1<<3)		/* Send Transaction Status */
#define ATP_EOM		(1<<4)		/* End Of Message */
#define ATP_XO		(1<<5)		/* eXactly Once mode */

/* function codes
*/
#define ATP_FUNCMASK	(3<<6)		/* mask all but function */

#define ATP_TREQ	(1<<6)		/* Trans. REQuest */
#define ATP_TRESP	(2<<6)		/* Trans. RESPonse */
#define ATP_TREL	(3<<6)		/* Trans. RELease */
#define ATP_TIMEOUT	(4<<6)      /* Trans. TimeOut */

//atp_bufs.c
struct atpbuf *alloc_atpbuf(void);
int free_atpbuf(struct atpbuf *bp);
void atpbuf_garbage(void);

//atp_pkt.c
void build_req_packet( struct atpbuf *pktbuf,uint16 tid,uint8 ctrl,struct atp_block *atpb);
void build_resp_packet(struct atpbuf *pktbuf,uint16	tid,uint8 ctrl,struct atp_block	*atpb,int16	seqnum);
int at_addr_eq(struct sockaddr_at *paddr,struct sockaddr_at	*saddr);
int recv_atp(ATP ah,      struct sockaddr_at *fromaddr,
             uint8 *func, uint16 tid,
             char *rbuf,  int16 wait);

//atp_open.c
ATP atp_open(uint8 port);

//atp_clos.c
int atp_close(ATP ah);

//atp_rreq.c
int atp_rreq(ATP ah, struct atp_block *atpb);

//atp_rsel.c
int atp_rsel(ATP ah, struct sockaddr_at *faddr, int func);
int resend_request(ATP ah);

//atp_rrsp.c
int atp_rresp(ATP ah, struct atp_block *atpb);

//atp_sreq.c
int atp_sreq(ATP ah, struct atp_block *atpb, int respcount, uint8 flags);

//atp_srsp.c
int atp_sresp(ATP ah, struct atp_block *atpb);

//#define atp_select(x,y)    ddp_select((x),(y))
#define atp_select(x)    ddp_select((x))

#endif  _ATP_H
