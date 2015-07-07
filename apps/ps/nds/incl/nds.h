#ifndef _NDS_H
#define _NDS_H


#define NDSPsSocket         0x2040 // 4020
#define NDSWatchDogSocket   0x2140 // 4021

#define NDS_MAX_RETRY_TIME ((BYTE)(TICKS_PER_SEC/2))  //0.5 sec
#define NDS_SAP_RETRY_COUNT	     (4)
#define NDS_MAX_PACKET_SIZE (1500-8)
#define IPX_WITH_NCP_SIZE       (52)

#define MAX_NDS_FS              (32)
#define MAX_NDS_QUEUE_PER_PRN   (MAX_QUEUE)	  //2/29/00 changed

struct e_ncpndsstruct
{
	BYTE   FunCode;		//
	BYTE   SubFunCode;
	uint32 FragHand;
	uint32 MaxFragSize;
	uint32 MessageSize;
	uint32 FragFlag;
	uint32 VerbNumber;
	uint32 ReplyBufSize;
	BYTE   NDSReqData[1];
}__attribute__ ((aligned(1), packed));

typedef struct e_ncpndsstruct NCP_NDS_STRUCT;

typedef unsigned short uni_char;

//NDS external status message (for Web) ////////////////////////////////////
#define NDS_E_BEGIN_LOGIN_TO_SERVER             1
#define	NDS_E_CAN_NOT_GET_TREE_SERVER           2
#define	NDS_E_CAN_NOT_ATTACH_TO_SERVER          3
#define NDS_E_CAN_NOT_LOGIN_TO_SERVER           4
#define NDS_E_PASSWORD_HAS_EXPIRED              5

#define NDS_E_CAN_NOT_GET_PRINTER_OBJECT        6
#define NDS_E_CAN_NOT_GET_QUEUE_OBJECT          7
#define NDS_E_CAN_NOT_GET_PRINTER_ENTRY         8
#define NDS_E_CAN_NOT_GET_QUEUE_ENTRY           9
#define NDS_E_CAN_NOT_GET_FS_ENTRY             10
#define NDS_E_NO_ANY_QUEUE_ATTACHED            11
#define NDS_E_TOO_MANY_PRINTERS                12
#define NDS_E_TOO_MANY_QUEUE                   13
#define NDS_E_CAN_NOT_ATTACH_BINDERY_QUEUE     14

#define NDS_E_GENERAL_SERVER_ERROR             15
#define NDS_E_INTERNAL_ERROR                   16

#define NDS_E_PRE_CONNECTED                    254
#define NDS_E_CONNECTED_OK                     255

////////////////////////////////////////////////////////////////////////////
#define NDS_PRINTSERVER_ERROR	(0x8700)
#define NDS_NO_QUEUE_ATTACH     (NDS_PRINTSERVER_ERROR | 0x01)

#define NDS_REQUESTER_ERROR     (0x8800)
#define NDS_BUFFER_OVERFLOW     (NDS_REQUESTER_ERROR | 0x0E)
#define NDS_NCP_PACKET_LENGTH   (NDS_REQUESTER_ERROR | 0x16)

#define NDS_SERVER_ERROR        (0x8900)
#define NDS_PASSWORD_EXPIRED    (NDS_SERVER_ERROR | 0xDF)
#define NDS_SERVER_FAILURE      (NDS_SERVER_ERROR | 0xFF)

#define NDS_NOMEM               (-301)
#define NDS_INVALID_RESPONSE    (-330)
#define NDS_NO_SUCH_ENTRY       (-601)
#define NDS_NO_SUCH_OBJECT      (-603)

#endif  _NDS_H
