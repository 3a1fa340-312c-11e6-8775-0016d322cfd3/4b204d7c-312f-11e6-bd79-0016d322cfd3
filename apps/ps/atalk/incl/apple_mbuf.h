#ifndef _APPLE_MBUF_H_
#define _APPLE_MBUF_H_

/* Basic message buffer structure */
#define AMbufSize	2000
struct ambuf {
	struct ambuf *anext;	/* Links packets on queues */
	uint32 size;		/* Size of associated data buffer */
	uint8 data[AMbufSize];		/* Active working*/
	uint32 cnt;
};

#endif _APPLE_MBUF_H_