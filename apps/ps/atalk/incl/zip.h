#ifndef _ZIP_H
#define _ZIP_H

#define ZIPOP_QUERY			1
#define ZIPOP_REPLY			2
#define ZIPOP_TAKEDOWN		3	/* XXX */
#define ZIPOP_BRINGUP		4	/* XXX */
#define ZIPOP_GNI			5
#define ZIPOP_GNIREPLY		6
#define ZIPOP_NOTIFY		7
#define ZIPOP_EREPLY		8
#define ZIPOP_GETMYZONE		7
#define ZIPOP_GETZONELIST 	8
#define ZIPOP_GETLOCALZONES 9

#define ZIPGNI_INVALID	    0x80
#define ZIPGNI_USEBROADCAST	0x40
#define ZIPGNI_ONEZONE	    0x20

#define ZIP_DDPTYPE         6
#define ZIP_SOCKET_NUM      6

void zip_info_query(cyg_addrword_t data);

#endif  _ZIP_H
