//setsocketopt() option name , 5/12/98
#define SO_RCV_BLOCK   0   //defalut value
#define SO_RCV_NOBLOCK 1
#define SO_RCV_TIMEOUT 2

#define	AF_APPLETALK	16		/* Apple Talk */

#define DDP_RECV_TIME_OUT  1 // (1 sec)



union cb {
	struct ddp_cb *ddp;
	void *p;
};

/* User sockets */
struct usock {
	unsigned index;
	int refcnt;
	char noblock;
	enum {
		NOTUSED,
		TYPE_DDP,
	} type;
	struct socklink *sp;
	int rdysock;
	union cb cb;
	struct sockaddr *name;
	int namelen;
	struct sockaddr *peername;
	int peernamelen;
	uint8 errcodes[4];	/* Protocol-specific error codes */
	uint8 tos;		/* Internet type-of-service */
	int flag;		/* Mode flags, defined in socket.h */
	cyg_sem_t sem_f;	//eCos
	char name_buffer[32];	//615wu
};

struct socklink {
	int type;		/* Socket type */
	int (*socket)(struct usock *,int);
	int (*bind)(struct usock *);
	int (*listen)(struct usock *,int);
	int (*connect)(struct usock *);
	int accept;
	int (*recv)(struct usock *,struct ambuf *,struct sockaddr *,int *);
	int (*send)(struct usock *,struct ambuf *,struct sockaddr *);
	int (*qlen)(struct usock *,int);
	int (*kick)(struct usock *);
	int (*shut)(struct usock *,int);
	int (*close)(struct usock *);
	int (*check)(struct sockaddr *,int);
	char **error;
	char *(*state)(struct usock *);
	int (*status)(struct usock *);
	const char *eol;
};

uint32 pullup2(struct ambuf **bp,void *buf,uint32 cnt, char *buffer);
uint32 pushdown2(struct ambuf **bp,void *buf,uint32 size, char *buffer);
void enqueue2(struct ambuf **q,struct ambuf **bpp);
struct ambuf * dequeue2(struct ambuf **q);
struct usock * itop(int s);
int setsocketopt2(int  s,char optname);
int recvfrom2(int s,void *buf,int len,int flags,struct sockaddr *from,int *fromlen);
int sendto2(int s,void *buf,int len,int flags,struct sockaddr *to,int tolen);
void sockinit2(void);
int socket2(int af,int type,int protocol);
int bind2(int s,struct sockaddr *name,int namelen);
int recv_mbuf2(int s,struct ambuf **bpp,int flags,struct sockaddr *from,int *fromlen);
int send_mbuf2(int s,struct ambuf **bpp,int flags,struct sockaddr *to,int tolen);
int close_s2(int s);
int trim_mbuf2(struct ambuf **bpp,uint length);
void free_p(struct ambuf ** buf);
void free_q(struct ambuf ** q);