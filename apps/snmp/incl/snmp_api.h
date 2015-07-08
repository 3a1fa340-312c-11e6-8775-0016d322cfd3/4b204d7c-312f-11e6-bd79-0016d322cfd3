/*
 * $Log$
 */
/*********************************
 * snmp_api.h
 *********************************/

#ifndef         _SNMP_API_H
#define         _SNMP_API_H


//***************
// Error codes:
//***************
//*
//* These must not clash with SNMP error codes (all positive).
//*

#define PARSE_ERROR     -1
#define BUILD_ERROR     -2

//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
#define         SNMP_DEFAULT_ADDRESS            0
#define         SNMP_DEFAULT_ENTERPRISE_LENGTH  0
#define         SNMP_DEFAULT_TIME               0
#define         SNMP_DEFAULT_RETRIES            5
#define         SNMP_DEFAULT_TIMEOUT            5000L
#define         SNMP_DEFAULT_REMPORT            0
#define         SNMP_VERSION_1                  0
#define         SNMP_MAX_NAME_LEN                       32
#define         FALSE                           0
#define         TRUE                            1
#define         SNMP_MAX_LEN            768
#define     SNMP_MAX_TRAP_LEN   256


//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
#define         SNMP_GET                        (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x0)
#define         SNMP_GET_NEXT                   (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x1)
#define         SNMP_GET_RESPONSE               (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x2)
#define         SNMP_SET                        (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x3)
#define         SNMP_TRAP                       (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4)

//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
#define         SNMP_TRAP_COLDSTART             (0x0)
#define         SNMP_TRAP_WARMSTART             (0x1)
#define         SNMP_TRAP_LINKDOWN              (0x2)
#define         SNMP_TRAP_LINKUP                (0x3)
#define         SNMP_TRAP_AUTHFAIL              (0x4)
#define         SNMP_TRAP_EGPNEIGHBORLOSS       (0x5)
#define         SNMP_TRAP_ENTERPRISESPECIFIC    (0x6)


//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
#define         RECEIVED_MESSAGE                (1)
#define         TIMED_OUT                       (2)

//#define SNMP_READ         1
//#define SNMP_WRITE        0

//*************** For Config Simon 11/24/98 ******************************//
#define SNMP_AUTH_FAIL "Authentication Failure"
#define _SendAuthFailTrap()     SendTrap(SNMP_TRAP_AUTHFAIL, SNMP_TRAP_AUTHFAIL,SNMP_AUTH_FAIL)

#define _SnmpAccessFlag    (EEPROM_Data.SnmpAccessFlag)
#define _SnmpSysContact    (EEPROM_Data.SnmpSysContact)
#define _SnmpSysLocation   (EEPROM_Data.SnmpSysLocation)
#define _SnmpCommunityAuthName (EEPROM_Data.SnmpCommunityAuthName)
#define _SnmpTrapTargetIP  (EEPROM_Data.SnmpTrapTargetIP)

#define _SnmpTrapEnable      (_SnmpAccessFlag.SnmpTrapEnable)
#define _SnmpAuthenTrap      (_SnmpAccessFlag.SnmpAuthenTrap)
#define _SnmpComm0AccessMode (_SnmpAccessFlag.SnmpComm0AccessMode)
#define _SnmpComm1AccessMode (_SnmpAccessFlag.SnmpComm1AccessMode)


#define SNMP_TRAP_ENABLE   1
#define SNMP_TRAP_DISABLE  0

#define SNMP_TRAP_AUTH_ENABLE  1
#define SNMP_TRAP_AUTH_DISABLE 2

#define SNMP_ACCESS_READ   0x1
#define SNMP_ACCESS_WRITE  0x2
#define SNMP_ACCESS_RW     (SNMP_ACCESS_READ | SNMP_ACCESS_WRITE)

//typedef struct {
//      uint32 IP;
//      BYTE Community[MAX_COMMUNITY_LEN+1];
//} TrapTarget;

typedef struct {
        BYTE AccessMode;  //SNMP_ACCESS_READ, SNMP_ACCESS_WRITE
        BYTE *Community;
} CommunityAuth;

//************************************************************************




#define RESERVE1    0
#define RESERVE2    1
#define COMMIT      2
#define ACTION      3
#define COMM_FREE   4

#define RONLY   0xAAAA  /* read access for everyone */
#define RWRITE  0xAABA  /* add write access for community private */
#define NOACCESS 0x0000 /* no access for anybody */

#define INTEGER     ASN_INTEGER
#define STRING      ASN_OCTET_STRING
#define OBJID       ASN_OBJECT_ID
#define NULLOBJ     ASN_NULL

//---------------------------------------------------------------------------------
//  SNMP application defined types (from the SMI, RFC 1065)
//---------------------------------------------------------------------------------
#define         IPADDRESS                       (ASN_APPLICATION | 0)
#define         COUNTER                         (ASN_APPLICATION | 1)
#define         GAUGE                           (ASN_APPLICATION | 2)
#define         TIMETICKS                       (ASN_APPLICATION | 3)
#define         OPAQUE                          (ASN_APPLICATION | 4)

#define     NSAP                (ASN_APPLICATION | 5)
#define     COUNTER64           (ASN_APPLICATION | 6)
#define     UINTEGER            (ASN_APPLICATION | 7)

//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
typedef struct sockaddr_in      ipaddr;

//----------------------------------------------
typedef struct _var_list {
    struct _var_list    *next;          /* null for last variable */
    oid                 *name;          /* variable name (object id) */
    int                 name_len;
    uint8               type;           /* ASN type of variable */

    union {                             /* value of variable */
        long            *integer;
        uint8           *string;
        oid             *objid;
    } val;
    int                 val_len;
} variable_list;

//----------------------------------------------
typedef struct _snmp_pdu {
    ipaddr              r_addr;         /* peer address */
    uint8               command;        /* type of pdu */
    uint32              reqid;          /* request id */
    uint32              errstatus;      /* error status */
    uint32              errindex;       /* error index */

    /* trap information */
//    oid               *enterprise;    /* system object ID */
//    int               enterprise_len;
    ipaddr              agent_addr;     /* address of object generating trap */
    long                generic_trap;
    long                specific_trap;
    uint32              time_stamp;
    variable_list       *variables;
} snmp_pdu;

//----------------------------------------------
typedef struct {
    uint32         reqid ;
//    int          retries ;
//    uint32       timeout ;
//    uint32       interval ;
    snmp_pdu       *pdu ;
} request_list ;

//----------------------------------------------
typedef struct {
    uint8          *community;
    int            community_len;
//    int          retries;
//    long         timeout;
//    char         *peername;
        uint16     remote_port;
        uint16     local_port;
//    uint8        *(*authenticator)(void *, int *, void *, int);
    void           (*callback)(int, void *, void *);
    void           *callback_magic;
        int        sfd ;          //---socket descriptor for this session-----
        ipaddr     r_addr;        //---Address of connected peer--------------
        request_list   *requests;
        uint8      access;
} snmp_session ;


//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
int             free_pdu (snmp_pdu *) ;
int       free_request_list (request_list *) ;
int     udp_read (snmp_session *, uint8 *, int *) ;
int            snmp_udp_send (snmp_session *, uint8 *,int) ;
uint8           *snmp_parse_auth (uint8 *, int *, uint8 *, int *, long *) ;
uint8           *snmp_build_auth (uint8 *, int *, uint8 *, int, long, int) ;
variable_list   *snmp_parse_var (uint8 *, int) ;
uint8           *snmp_build_var (uint8 *, variable_list *, int *) ;
int             snmp_build_trap (snmp_session *, snmp_pdu *, uint8 *, int *) ;
int             snmp_send_trap (snmp_session *, snmp_pdu *) ;
int             snmp_close (snmp_session *) ;
#ifdef WEBADMIN
BYTE snmp_ReadPrintStatus(int port);
void snmp_ReadCardAndPeripheralStatus(int port,char *buf);
BYTE snmp_GetCfgSource(void);
void snmp_SetCfgSource(int value);
long snmp_SetProtocol(void);
int  snmp_CheckValueType(uint8 var_type,int var_len);
void ReadDataFromFrontPanel(char *buf,int Port);
void ReadPrintModelAndID(char *buf,int Port);
void RunChangeIPAddress(void);
void RunWriteDataToEEPROM(void);
void RunRestart(void);
#endif WEBADMIN
#endif
