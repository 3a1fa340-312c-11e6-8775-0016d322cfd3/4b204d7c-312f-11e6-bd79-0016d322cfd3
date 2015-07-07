/* snmp_vars.c - return a pointer to the named variable.
 *
 *
 */
/***********************************************************
        Copyright 1988, 1989, 1990 by Carnegie Mellon University
        Copyright 1989  TGV, Incorporated

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and TGV not be used
in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

CMU AND TGV DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL CMU OR TGV BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
******************************************************************/

/*
 * additions, fixes and enhancements for Linux by
 * Erik Schoenfelder <schoenfr@gaertner.de> and
 * Juergen Schoenwaelder <schoenw@ibr.cs.tu-bs.de>
 * 1996, 1997
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <alloc.h>
#include <cyg/kernel/kapi.h>    //615wu
#include "network.h"    //615wu
#include "pstarget.h"   //615wu
#include "psglobal.h"	//615wu
#include "psdefine.h"	//615wu
#include "eeprom.h"		//615wu
#include "prnport.h"    //615wu
#include "asn1.h"
#include "snmp.h"
#include "snmp_api.h"
#include "snmparp.h"
#include "mibmodl.h"
#include "snmpvars.h"
#include "snmpgrup.h"
#include "snmptrap.h"
#include "nps.h"		//615wu

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///////////////////////////     615wu       //////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern char 			Hostname[];	//615wu
struct mib_entry {
	char *name;
	union {
		int32 integer;
	} value;
};
struct mib_entry Ip_mib[20] = {
    "",                 0,
    "ipForwarding",     1,
    "ipDefaultTTL",     32,
    "ipInReceives",     0,
    "ipInHdrErrors",    0,
    "ipInAddrErrors",   0,
    "ipForwDatagrams",  0,
    "ipInUnknownProtos",0,
    "ipInDiscards",     0,
    "ipInDelivers",     0,
    "ipOutRequests",    0,
    "ipOutDiscards",    0,
    "ipOutNoRoutes",    0,
    "ipReasmTimeout",   30,
    "ipReasmReqds",     0,
    "ipReasmOKs",       0,
    "ipReasmFails",     0,
    "ipFragOKs",        0,
    "ipFragFails",      0,
    "ipFragCreates",    0,
};
#define	ipForwarding		Ip_mib[1].value.integer
#define	ipDefaultTTL		Ip_mib[2].value.integer
#define	ipInReceives		Ip_mib[3].value.integer
#define	ipInHdrErrors		Ip_mib[4].value.integer
#define	ipInAddrErrors		Ip_mib[5].value.integer
#define	ipForwDatagrams		Ip_mib[6].value.integer
#define	ipInUnknownProtos	Ip_mib[7].value.integer
#define	ipInDiscards		Ip_mib[8].value.integer
#define	ipInDelivers		Ip_mib[9].value.integer
#define	ipOutRequests		Ip_mib[10].value.integer
#define	ipOutDiscards		Ip_mib[11].value.integer
#define	ipOutNoRoutes		Ip_mib[12].value.integer
#define	ipReasmTimeout		Ip_mib[13].value.integer
#define	ipReasmReqds		Ip_mib[14].value.integer
#define	ipReasmOKs		Ip_mib[15].value.integer
#define	ipReasmFails		Ip_mib[16].value.integer
#define	ipFragOKs		Ip_mib[17].value.integer
#define	ipFragFails		Ip_mib[18].value.integer
#define	ipFragCreates		Ip_mib[19].value.integer

struct mib_entry Icmp_mib[] = {
	"",			0,
	"icmpInMsgs",		0,
	"icmpInErrors",		0,
	"icmpInDestUnreachs",	0,
	"icmpInTimeExcds",	0,
	"icmpInParmProbs",	0,
	"icmpInSrcQuenchs",	0,
	"icmpInRedirects",	0,
	"icmpInEchos",		0,
	"icmpInEchoReps",	0,
	"icmpInTimestamps",	0,
	"icmpInTimestampReps",	0,
	"icmpInAddrMasks",	0,
	"icmpInAddrMaskReps",	0,
	"icmpOutMsgs",		0,
	"icmpOutErrors",	0,
	"icmpOutDestUnreachs",	0,
	"icmpOutTimeExcds",	0,
	"icmpOutParmProbs",	0,
	"icmpOutSrcQuenchs",	0,
	"icmpOutRedirects",	0,
	"icmpOutEchos",		0,
	"icmpOutEchoReps",	0,
	"icmpOutTimestamps",	0,
	"icmpOutTimestampReps",	0,
	"icmpOutAddrMasks",	0,
	"icmpOutAddrMaskReps",	0,
};
#define	icmpInMsgs		Icmp_mib[1].value.integer
#define	icmpInErrors		Icmp_mib[2].value.integer
#define icmpInDestUnreachs	Icmp_mib[3].value.integer
#define icmpInTimeExcds		Icmp_mib[4].value.integer
#define icmpInParmProbs		Icmp_mib[5].value.integer
#define icmpInSrcQuenchs	Icmp_mib[6].value.integer
#define icmpInRedirects		Icmp_mib[7].value.integer
#define icmpInEchos		Icmp_mib[8].value.integer
#define icmpInEchoReps		Icmp_mib[9].value.integer
#define icmpInTimestamps	Icmp_mib[10].value.integer
#define icmpInTimestampReps	Icmp_mib[11].value.integer
#define icmpInAddrMasks		Icmp_mib[12].value.integer
#define icmpInAddrMaskReps	Icmp_mib[13].value.integer
#define icmpOutMsgs		Icmp_mib[14].value.integer
#define icmpOutErrors		Icmp_mib[15].value.integer
#define icmpOutDestUnreachs	Icmp_mib[16].value.integer
#define icmpOutTimeExcds	Icmp_mib[17].value.integer
#define icmpOutParmProbs	Icmp_mib[18].value.integer
#define icmpOutSrcQuenchs	Icmp_mib[19].value.integer
#define icmpOutRedirects	Icmp_mib[20].value.integer
#define icmpOutEchos		Icmp_mib[21].value.integer
#define icmpOutEchoReps		Icmp_mib[22].value.integer
#define icmpOutTimestamps	Icmp_mib[23].value.integer
#define icmpOutTimestampReps	Icmp_mib[24].value.integer
#define icmpOutAddrMasks	Icmp_mib[25].value.integer
#define icmpOutAddrMaskReps	Icmp_mib[26].value.integer

struct mib_entry Tcp_mib[] = {
    NULL,             0,
    "tcpRtoAlgorithm",4,  /* Van Jacobsen's algorithm */
    "tcpRtoMin",      500L,  /* No lower bound */
    "tcpRtoMax",      0xffffffff,   /* No upper bound */
    "tcpMaxConn",     -1L,
    "tcpActiveOpens", 0,
    "tcpPassiveOpens",0,
    "tcpAttemptFails",0,
    "tcpEstabResets", 0,
    "tcpCurrEstab",   0,
    "tcpInSegs",      0,
    "tcpOutSegs",     0,
    "tcpRetransSegs", 0,
    NULL,             0,  /* Connection state goes here */
    "tcpInErrs",      0,
    "tcpOutRsts",     0,
};
#define	tcpRtoAlgorithm	Tcp_mib[1].value.integer
#define	tcpRtoMin	Tcp_mib[2].value.integer
#define	tcpRtoMax	Tcp_mib[3].value.integer
#define	tcpMaxConn	Tcp_mib[4].value.integer
#define	tcpActiveOpens	Tcp_mib[5].value.integer
#define tcpPassiveOpens	Tcp_mib[6].value.integer
#define	tcpAttemptFails	Tcp_mib[7].value.integer
#define	tcpEstabResets	Tcp_mib[8].value.integer
#define	tcpCurrEstab	Tcp_mib[9].value.integer
#define	tcpInSegs	Tcp_mib[10].value.integer
#define	tcpOutSegs	Tcp_mib[11].value.integer
#define	tcpRetransSegs	Tcp_mib[12].value.integer
#define	tcpInErrs	Tcp_mib[14].value.integer
#define	tcpOutRsts	Tcp_mib[15].value.integer

struct mib_entry Udp_mib[] = {
	"",			      	    0,
	"udpInDatagrams", 		0,
	"udpNoPorts",	  		0,
	"udpInErrors",	  		0,
	"udpOutDatagrams",		0,
};
#define	udpInDatagrams	Udp_mib[1].value.integer
#define	udpNoPorts	Udp_mib[2].value.integer
#define	udpInErrors	Udp_mib[3].value.integer
#define	udpOutDatagrams	Udp_mib[4].value.integer

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define TALLOC(T)       ((T *) calloc (1, sizeof(T)))
#define Interface_Scan_Get_Count()  1   //interface number

#define DEFAULT_MAX_MTU 1500

#ifdef PC_OUTPUT
extern void ErrorBeep(void); //unknow.c
extern void AtSay(int x,int y,char *); //unknow.c
extern void AtSaySpace(int x,int y, int repeat); //unknow.c
#endif

static struct subtree *subtrees;
static oid newname[MAX_NAME_LEN];

static long      long_return;
static uint8 return_buf[SNMP_MAX_STRING];

static uint8 InterfaceIfDescr[] = "Ethernet";

#ifdef WEBADMIN
#define STATUSBYTES 19
#define SNMP_PRINTSERVER_NAME_LEN 48
#endif WEBADMIN

#ifdef WEBADMIN // modified ---- Arius
extern	uint8 PSMode2; 
extern uint8   JetPort;
extern uint8   Snmp_Change_IP;
extern uint8   Snmp_WriteDataTo_EEPROM;
extern uint8   Snmp_Restart;
uint8 JetModel[] = "C6";
uint8 Snmp_PJLStatus = 1;
uint8 UsingSoftwareReset = 0;
int  Snmp_JobTimeOut = 30;
int  Snmp_ScanTimeOut = 0;
int  Snmp_PortDesiredMode = 0;
int  Snmp_PortHandshanking = 0;
long Snmp_ProtocolSet = 1140850714;
int  Snmp_StatusPageLanguage = 0;
int  Snmp_PrintStatusPage = 0;
int  Snmp_ErrorBehavior = 0;
int  Snmp_IdelTimeOut = 0;
oid SnmpVersionID[SNMP_VER_LEN] = {1, 3, 6, 1, 4, 1, 11, 2, 3, 9 ,1};
#else
#ifdef O_AXIS
oid SnmpVersionID[SNMP_VER_LEN] = {1, 3, 6, 1, 4, 1, 368, 1, 1};
#else
oid SnmpVersionID[SNMP_VER_LEN] = {1, 3, 6, 1, 4, 1, 722, 2, 6, 1};
#endif O_AXIS
#endif WEBADMIN

uint8  version_descr[] =
#if (NUM_OF_PRN_PORT >= 3)
#ifdef WEBADMIN // modified ---- Arius
 "ETHERNET MULTI_ENVIRONMENT,ROM H_07_19,JETDIRECT EX,JD28,EEPROM 3.51";
#else
 "3 port Print Server";
#endif WEBADMIN
#elif (NUM_OF_PRN_PORT >=2)
 "2 port Print Server";
#else
#ifdef WEBADMIN // modified ---- Arius
 "ETHERNET MULTI_ENVIRONMENT,ROM H_07_19,JETDIRECT EX,JD28,EEPROM 3.51";
#else
#ifdef O_AXIS
 "AXIS OfficeBasic USB Wireless G Print Server";
#else
 "1 port Print Server";
#endif
#endif WEBADMIN
#endif

//Define for Interface MIBII
uint32 mib_ifInOctets;
uint32 mib_ifInUcastPkts;
uint32 mib_ifInNucastPkts;
uint32 mib_ifInDiscards;
//uint32 mib_ifInErrors;
uint32 mib_ifInUnknowProtos;

uint32 mib_ifOutOctets;
uint32 mib_ifOutUcastPkts;
uint32 mib_ifOutNucastPkts;
//uint32 mib_ifOutDiscards;
uint32 mib_ifOutErrors;
uint32 mib_ifOutQLen;


//*
//* counters for the snmp group:
//*
int32 snmp_inpkts;
int32 snmp_outpkts;
int32 snmp_inbadversions;
int32 snmp_inbadcommunitynames;
//int32 snmp_inbadcommunityuses;
int32 snmp_inasnparseerrors;
int32 snmp_intoobigs;
// int32 snmp_innosuchnames;
int32 snmp_inbadvalues;
int32 snmp_inreadonlys;
int32 snmp_ingenerrs;
int32 snmp_intotalreqvars;
// int32 snmp_intotalsetvars;
int32 snmp_ingetrequests;
int32 snmp_ingetnexts;
int32 snmp_insetrequests;
// int32 snmp_ingetresponses;
// int32 snmp_intraps;
// int32 snmp_outtoobigs;
int32 snmp_outnosuchnames;
// int32 snmp_outbadvalues;
// int32 snmp_outgenerrs;

int32 snmp_outgetrequests;  //Simon 11/5/98
int32 snmp_outgetnexts;   //Simon 11/5/98

int32 snmp_outsetrequests;  //Simon 11/5/98

int32 snmp_outgetresponses;
int32 snmp_outtraps;             //Simon 11/5/98
//int32 snmp_enableauthentraps = 2;      //2: disable  replace by EEPROM's _SnmpAuthenTrap

static void mib_register (
        oid *oid_base,
        int oid_base_len,
        struct variable *mib_variables,
        int mib_variables_len,
        int mib_variables_width
);

uint8 * var_system(struct variable *vp, oid *name, int   *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_ifEntry(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_atEntry(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_ip(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_ipAddrEntry(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_ipRouteEntry(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_icmp(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_tcp(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_udp(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

uint8 * var_snmp(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());

// Adding for enterprise's MIB data. Here print server will support
// HP jetadmin/web jetadmin
// 05/12/2000 ---- add by Arius
#ifdef WEBADMIN // modified ---- Arius
uint8 * var_devstatus(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_gdstatusEntry(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npsys(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npconns(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npcfg(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_nptcp(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npctl(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npnpi(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npipx(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npipxEntry(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npcard11(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npcard12(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npcard13(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
uint8 * var_npcard17(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
/* uint8 * var_npcard18(struct variable *vp, oid *name, int *length,
                    int exact, int *var_len, int (**write_method)());
*/
#endif WEBADMIN // modified ---- Arius
// ------ End of here  >>>> By Arius


static int compare_tree(oid *name1, int len1, oid *name2, int len2);
static int compare(oid  *name1, int len1, oid *name2, int len2);


struct variable2 system_variables[] = {
        {VERSION_DESCR, STRING,    RONLY,  var_system, 1, {1}}
        ,{VERSION_ID,   OBJID,     RONLY,  var_system, 1, {2}}
        ,{UPTIME,       TIMETICKS, RONLY,  var_system, 1, {3}}
        ,{SYSCONTACT,   STRING,    RWRITE, var_system, 1, {4}}
        ,{SYSYSNAME,    STRING,    RWRITE, var_system, 1, {5}}
        ,{SYSLOCATION,  STRING,    RWRITE, var_system, 1, {6}}
        ,{SYSSERVICES,  INTEGER,   RONLY,  var_system, 1, {7}}
//Simon remarked ,{SYSORLASTCHANGE, TIMETICKS, RONLY, var_system, 1, {8}}
};

struct variable4 interface_variables[] = {
        {IFNUMBER,          INTEGER,   RONLY,  var_system,  1, {1}      },
        {IFINDEX,           INTEGER,   RONLY,  var_ifEntry, 3, {2, 1, 1}},
        {IFDESCR,           STRING,    RONLY,  var_ifEntry, 3, {2, 1, 2}},
        {IFTYPE,            INTEGER,   RONLY,  var_ifEntry, 3, {2, 1, 3}},
        {IFMTU,             INTEGER,   RONLY,  var_ifEntry, 3, {2, 1, 4}},
        {IFSPEED,           GAUGE,     RONLY,  var_ifEntry, 3, {2, 1, 5}},
        {IFPHYSADDRESS,     STRING,    RONLY,  var_ifEntry, 3, {2, 1, 6}},
        {IFADMINSTATUS,     INTEGER,   RONLY,  var_ifEntry, 3, {2, 1, 7}},
        {IFOPERSTATUS,      INTEGER,   RONLY,  var_ifEntry, 3, {2, 1, 8}},
        {IFLASTCHANGE,      TIMETICKS, RONLY,  var_ifEntry, 3, {2, 1, 9}},
        {IFINOCTETS,        COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 10}},
        {IFINUCASTPKTS,     COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 11}},
        {IFINNUCASTPKTS,    COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 12}},
        {IFINDISCARDS,      COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 13}},
        {IFINERRORS,        COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 14}},
        {IFINUNKNOWNPROTOS, COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 15}},
        {IFOUTOCTETS,       COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 16}},
        {IFOUTUCASTPKTS,    COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 17}},
        {IFOUTNUCASTPKTS,   COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 18}},
        {IFOUTDISCARDS,     COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 19}},
        {IFOUTERRORS,       COUNTER,   RONLY,  var_ifEntry, 3, {2, 1, 20}},
        {IFOUTQLEN,         GAUGE,     RONLY,  var_ifEntry, 3, {2, 1, 21}},
        {IFSPECIFIC,        OBJID,     RONLY,  var_ifEntry, 3, {2, 1, 22}}
};

struct variable2 at_variables[] = {
        {ATIFINDEX,     INTEGER,   RONLY, var_atEntry, 1, {1}},
        {ATPHYSADDRESS, STRING,    RONLY, var_atEntry, 1, {2}},
        {ATNETADDRESS,  IPADDRESS, RONLY, var_atEntry, 1, {3}}
};

struct variable4 ip_variables[] = {
        {IPFORWARDING,        INTEGER,   RONLY,  var_ip, 1, {1 }},
        {IPDEFAULTTTL,        INTEGER,   RONLY,  var_ip, 1, {2 }},
        {IPINRECEIVES,        COUNTER,   RONLY,  var_ip, 1, {3 }},
        {IPINHDRERRORS,       COUNTER,   RONLY,  var_ip, 1, {4 }},
        {IPINADDRERRORS,      COUNTER,   RONLY,  var_ip, 1, {5 }},
        {IPFORWDATAGRAMS,     COUNTER,   RONLY,  var_ip, 1, {6 }},
        {IPINUNKNOWNPROTOS,   COUNTER,   RONLY,  var_ip, 1, {7 }},
        {IPINDISCARDS,        COUNTER,   RONLY,  var_ip, 1, {8 }},
        {IPINDELIVERS,        COUNTER,   RONLY,  var_ip, 1, {9 }},
        {IPOUTREQUESTS,       COUNTER,   RONLY,  var_ip, 1, {10 }},
        {IPOUTDISCARDS,       COUNTER,   RONLY,  var_ip, 1, {11 }},
        {IPOUTNOROUTES,       COUNTER,   RONLY,  var_ip, 1, {12 }},
        {IPREASMTIMEOUT,      INTEGER,   RONLY,  var_ip, 1, {13 }},
        {IPREASMREQDS,        COUNTER,   RONLY,  var_ip, 1, {14 }},
        {IPREASMOKS,          COUNTER,   RONLY,  var_ip, 1, {15 }},
        {IPREASMFAILS,        COUNTER,   RONLY,  var_ip, 1, {16 }},
        {IPFRAGOKS,           COUNTER,   RONLY,  var_ip, 1, {17 }},
        {IPFRAGFAILS,         COUNTER,   RONLY,  var_ip, 1, {18 }},
        {IPFRAGCREATES,       COUNTER,   RONLY,  var_ip, 1, {19 }},
        {IPADADDR,            IPADDRESS, RONLY,  var_ipAddrEntry, 3, {20, 1, 1}},
        {IPADIFINDEX,         INTEGER,   RONLY,  var_ipAddrEntry, 3, {20, 1, 2}},
        {IPADNETMASK,         IPADDRESS, RONLY,  var_ipAddrEntry, 3, {20, 1, 3}},
        {IPADBCASTADDR,       INTEGER,   RONLY,  var_ipAddrEntry, 3, {20, 1, 4}},
        {IPADENTREASMMAXSIZE, INTEGER,   RONLY,  var_ipAddrEntry, 3, {20, 1, 5}}
#if 0
        ,{IPROUTEDEST,         IPADDRESS, RWRITE, var_ipRouteEntry, 3, {21, 1, 1}}
        ,{IPROUTEIFINDEX,      INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 2}}
        ,{IPROUTEMETRIC1,      INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 3}}
        ,{IPROUTEMETRIC2,      INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 4}}
        ,{IPROUTEMETRIC3,      INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 5}}
        ,{IPROUTEMETRIC4,      INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 6}}
        ,{IPROUTENEXTHOP,      IPADDRESS, RWRITE, var_ipRouteEntry, 3, {21, 1, 7}}
        ,{IPROUTETYPE,         INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 8}}
        ,{IPROUTEPROTO,        INTEGER,   RONLY,  var_ipRouteEntry, 3, {21, 1, 9}}
        ,{IPROUTEAGE,          INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 10}}

        ,{IPROUTEMASK,    IPADDRESS, RWRITE, var_ipRouteEntry, 3, {21, 1, 11}}
        ,{IPROUTEMETRIC5, INTEGER,   RWRITE, var_ipRouteEntry, 3, {21, 1, 12}}
/** XXX: not yet: **/
/** XXX:    , {IPROUTEINFO, OBJID, RONLY, var_ipRouteEntry, 3, {21, 1, 13}} **/
 ,{IPNETTOMEDIAIFINDEX,  INTEGER,   /* W */ RONLY, var_ntomEntry, 3, {22, 1, 1}}
 ,{IPNETTOMEDIAPHYSADDR, STRING,    /* W */ RONLY, var_ntomEntry, 3, {22, 1, 2}}
 ,{IPNETTOMEDIANETADDR,  IPADDRESS, /* W */ RONLY, var_ntomEntry, 3, {22, 1, 3}}
 ,{IPNETTOMEDIATYPE,     INTEGER,   /* W */ RONLY, var_ntomEntry, 3, {22, 1, 4}}
#endif 0
};

struct variable2 icmp_variables[] = {
        {ICMPINMSGS,           COUNTER, RONLY, var_icmp, 1, {1}},
        {ICMPINERRORS,         COUNTER, RONLY, var_icmp, 1, {2}},
        {ICMPINDESTUNREACHS,   COUNTER, RONLY, var_icmp, 1, {3}},
        {ICMPINTIMEEXCDS,      COUNTER, RONLY, var_icmp, 1, {4}},
        {ICMPINPARMPROBS,      COUNTER, RONLY, var_icmp, 1, {5}},
        {ICMPINSRCQUENCHS,     COUNTER, RONLY, var_icmp, 1, {6}},
        {ICMPINREDIRECTS,      COUNTER, RONLY, var_icmp, 1, {7}},
        {ICMPINECHOS,          COUNTER, RONLY, var_icmp, 1, {8}},
        {ICMPINECHOREPS,       COUNTER, RONLY, var_icmp, 1, {9}},
        {ICMPINTIMESTAMPS,     COUNTER, RONLY, var_icmp, 1, {10}},
        {ICMPINTIMESTAMPREPS,  COUNTER, RONLY, var_icmp, 1, {11}},
        {ICMPINADDRMASKS,      COUNTER, RONLY, var_icmp, 1, {12}},
        {ICMPINADDRMASKREPS,   COUNTER, RONLY, var_icmp, 1, {13}},
        {ICMPOUTMSGS,          COUNTER, RONLY, var_icmp, 1, {14}},
        {ICMPOUTERRORS,        COUNTER, RONLY, var_icmp, 1, {15}},
        {ICMPOUTDESTUNREACHS,  COUNTER, RONLY, var_icmp, 1, {16}},
        {ICMPOUTTIMEEXCDS,     COUNTER, RONLY, var_icmp, 1, {17}},
        {ICMPOUTPARMPROBS,     COUNTER, RONLY, var_icmp, 1, {18}},
        {ICMPOUTSRCQUENCHS,    COUNTER, RONLY, var_icmp, 1, {19}},
        {ICMPOUTREDIRECTS,     COUNTER, RONLY, var_icmp, 1, {20}},
        {ICMPOUTECHOS,         COUNTER, RONLY, var_icmp, 1, {21}},
        {ICMPOUTECHOREPS,      COUNTER, RONLY, var_icmp, 1, {22}},
        {ICMPOUTTIMESTAMPS,    COUNTER, RONLY, var_icmp, 1, {23}},
        {ICMPOUTTIMESTAMPREPS, COUNTER, RONLY, var_icmp, 1, {24}},
        {ICMPOUTADDRMASKS,     COUNTER, RONLY, var_icmp, 1, {25}},
        {ICMPOUTADDRMASKREPS,  COUNTER, RONLY, var_icmp, 1, {26}}
};

struct variable13 tcp_variables[] = {
        {TCPRTOALGORITHM,     INTEGER,   RONLY, var_tcp, 1, {1}},
        {TCPRTOMIN,           INTEGER,   RONLY, var_tcp, 1, {2}},
        {TCPRTOMAX,           INTEGER,   RONLY, var_tcp, 1, {3}},
        {TCPMAXCONN,          INTEGER,   RONLY, var_tcp, 1, {4}},
        {TCPACTIVEOPENS,      COUNTER,   RONLY, var_tcp, 1, {5}},
        {TCPPASSIVEOPENS,     COUNTER,   RONLY, var_tcp, 1, {6}},
        {TCPATTEMPTFAILS,     COUNTER,   RONLY, var_tcp, 1, {7}},
        {TCPESTABRESETS,      COUNTER,   RONLY, var_tcp, 1, {8}},
        {TCPCURRESTAB,        GAUGE,     RONLY, var_tcp, 1, {9}},
        {TCPINSEGS,           COUNTER,   RONLY, var_tcp, 1, {10}},
        {TCPOUTSEGS,          COUNTER,   RONLY, var_tcp, 1, {11} },
        {TCPRETRANSSEGS,      COUNTER,   RONLY, var_tcp, 1, {12}},
        {TCPCONNSTATE,        INTEGER,   RONLY, var_tcp, 3, {13, 1, 1}},
        {TCPCONNLOCALADDRESS, IPADDRESS, RONLY, var_tcp, 3, {13, 1, 2}},
        {TCPCONNLOCALPORT,    INTEGER,   RONLY, var_tcp, 3, {13, 1, 3}},
        {TCPCONNREMADDRESS,   IPADDRESS, RONLY, var_tcp, 3, {13, 1, 4}},
        {TCPCONNREMPORT,      INTEGER,   RONLY, var_tcp, 3, {13, 1, 5}},
        {TCPINERRS,           COUNTER,   RONLY, var_tcp, 1, {14}},
        {TCPOUTRSTS,          COUNTER,   RONLY, var_tcp, 1, {15}}
};

struct variable13 udp_variables[] = {
        {UDPINDATAGRAMS,  COUNTER,   RONLY, var_udp, 1, {1}},
        {UDPNOPORTS,      COUNTER,   RONLY, var_udp, 1, {2}},
        {UDPINERRORS,     COUNTER,   RONLY, var_udp, 1, {3}},
        {UDPOUTDATAGRAMS, COUNTER,   RONLY, var_udp, 1, {4}},
        {UDPLOCALADDRESS, IPADDRESS, RONLY, var_udp, 3, {5, 1, 1}},
        {UDPLOCALPORT,    INTEGER,   RONLY, var_udp, 3, {5, 1, 2}}
};

struct variable2 snmp_variables[] = {
        {SNMPINPKTS,              COUNTER, RONLY,  var_snmp, 1, {1}},
        {SNMPOUTPKTS,             COUNTER, RONLY,  var_snmp, 1, {2}},
        {SNMPINBADVERSIONS,       COUNTER, RONLY,  var_snmp, 1, {3}},
        {SNMPINBADCOMMUNITYNAMES, COUNTER, RONLY,  var_snmp, 1, {4}},
        {SNMPINBADCOMMUNITYUSES,  COUNTER, RONLY,  var_snmp, 1, {5}},
        {SNMPINASNPARSEERRORS,    COUNTER, RONLY,  var_snmp, 1, {6}},
        {SNMPINTOOBIGS,           COUNTER, RONLY,  var_snmp, 1, {8}},
        {SNMPINNOSUCHNAMES,       COUNTER, RONLY,  var_snmp, 1, {9}},
        {SNMPINBADVALUES,         COUNTER, RONLY,  var_snmp, 1, {10}},
        {SNMPINREADONLYS,         COUNTER, RONLY,  var_snmp, 1, {11}},
        {SNMPINGENERRS,           COUNTER, RONLY,  var_snmp, 1, {12}},
        {SNMPINTOTALREQVARS,      COUNTER, RONLY,  var_snmp, 1, {13}},
        {SNMPINTOTALSETVARS,      COUNTER, RONLY,  var_snmp, 1, {14}},
        {SNMPINGETREQUESTS,       COUNTER, RONLY,  var_snmp, 1, {15}},
        {SNMPINGETNEXTS,          COUNTER, RONLY,  var_snmp, 1, {16}},
        {SNMPINSETREQUESTS,       COUNTER, RONLY,  var_snmp, 1, {17}},
        {SNMPINGETRESPONSES,      COUNTER, RONLY,  var_snmp, 1, {18}},
        {SNMPINTRAPS,             COUNTER, RONLY,  var_snmp, 1, {19}},
        {SNMPOUTTOOBIGS,          COUNTER, RONLY,  var_snmp, 1, {20}},
        {SNMPOUTNOSUCHNAMES,      COUNTER, RONLY,  var_snmp, 1, {21}},
        {SNMPOUTBADVALUES,        COUNTER, RONLY,  var_snmp, 1, {22}},
        {SNMPOUTGENERRS,          COUNTER, RONLY,  var_snmp, 1, {24}},
        {SNMPOUTGETREQUESTS,      COUNTER, RONLY,  var_snmp, 1, {25}},
        {SNMPOUTGETNEXTS,         COUNTER, RONLY,  var_snmp, 1, {26}},
        {SNMPOUTSETREQUESTS,      COUNTER, RONLY,  var_snmp, 1, {27}},
        {SNMPOUTGETRESPONSES,     COUNTER, RONLY,  var_snmp, 1, {28}},
        {SNMPOUTTRAPS,            COUNTER, RONLY,  var_snmp, 1, {29}},
        {SNMPENABLEAUTHENTRAPS,   INTEGER, RWRITE, var_snmp, 1, {30}}
};

// Adding for enterprise's MIB data. Here print server will support
// HP jetadmin/web jetadmin
// 05/12/2000 ---- add by Arius
#ifdef WEBADMIN // modified ---- Arius
    //  --- The Status Group
struct variable20 GeneralDeviceStatus_variables[] = {
        {GDSTATUSBYTES,       INTEGER,   RONLY, var_devstatus, 1, {1}},
        {GDSTATUSLINESTATE,                INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 1}},
        {GDSTATUSPAPAERSTATE,              INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 2}},
        {GDSTATUSINTERVENTIONSTATE,        INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 3}},
        {GDSTATUSNEWMODE,                  INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 4}},
        {GDSTATUSCONNECTIONTERMINATIONACK, INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 5}},
        {GDSTATUSPERIPHERALERROR,          INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 6}},
        {GDSTATUSPAPEROUT,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 8}},
        {GDSTATUSPAPERJAM,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 9}},
        {GDSTATUSTONERLOW,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 10}},
        {GDSTATUSPAGEPUNT,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 11}},
        {GDSTATUSMEMEORYOUT,               INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 12}},
        {GDSTATUSIOACTIVE,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 13}},
        {GDSTATUSBUSY,                     INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 14}},
        {GDSTATUSWAIT,                     INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 15}},
        {GDSTATUSINITIALIZE,               INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 16}},
        {GDSTATUSDOOROPEN,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 17}},
        {GDSTATUSPRINTING,                 INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 18}},
        {GDSTATUSPAPEROUTPUT,              INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 19}},
        {GDSTATUSRESERVED,                 STRING,    RONLY, var_gdstatusEntry, 2, {2, 20}},
        {GDSTATUSNOVBUSY,                  INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 21}},
        {GDSTATUSLLCBUSY,                  INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 22}},
        {GDSTATUSTCPBUSY,                  INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 23}},
        {GDSTATUSATBUSY,                   INTEGER,   RONLY, var_gdstatusEntry, 2, {2, 24}},
        {GDSTATUSDISPLAY,     STRING,    RONLY, var_devstatus, 1, {3}},
        {GDSTATUSJOBNAME,     STRING,    RONLY, var_devstatus, 1, {4}},
        {GDSTATUSSOURCE,      STRING,    RONLY, var_devstatus, 1, {5}},
        {GDSTATUSPAPSTATUS,   STRING,    RONLY, var_devstatus, 1, {6}},
        {GDSTATUSID,          STRING,    RONLY, var_devstatus, 1, {7}},
        {GDSTATUSDISPLAYCODE, INTEGER,   RONLY, var_devstatus, 1, {8}},
        {GDSTATUSNLSCODE,     INTEGER,   RONLY, var_devstatus, 1, {9}},
    //      The Unknown data
        {GDSTATUS10,          INTEGER,   RWRITE,var_devstatus, 1, {10}},
        {GDSTATUS11,          INTEGER,   RWRITE,var_devstatus, 1, {11}},
        {GDSTATUS13,          STRING,    RWRITE,var_devstatus, 1, {13}},
        {GDSTATUS18,          STRING,    RWRITE,var_devstatus, 1, {18}},
        {GDSTATUS19,          STRING,    RONLY, var_devstatus, 1, {19}}
};
    //  --- The HP Network Peripheral Card(npCard) Group
    //      The System subgroup of npCard
struct variable20 NpSys_variables[] = {
        {NPSYSSTATE,                INTEGER,   RONLY, var_npsys, 1, {1}},
        {NPSYSSTATUSMESSAGE,        STRING,    RONLY, var_npsys, 1, {2}},
        {NPSYSPERIPHERALFATALERROR, INTEGER,   RONLY, var_npsys, 1, {3}},
        {NPSYSCARDFATALERROR,       INTEGER,   RONLY, var_npsys, 1, {4}},
        {NPSYSMAXIMUMWRITEBUFFERS,  INTEGER,   RONLY, var_npsys, 1, {5}},
        {NPSYSMAXIMUMREADBUFFERS,   INTEGER,   RONLY, var_npsys, 1, {6}},
        {NPSYSTOTALBYTESRECVS,      COUNTER,   RONLY, var_npsys, 1, {7}},
        {NPSYSTOTALBYTESENTS,       COUNTER,   RONLY, var_npsys, 1, {8}},
        {NPSYSCURRREADREQ,          GAUGE,     RONLY, var_npsys, 1, {9}}
};
    //      The Card Connection Statistics subgroup of npCard
struct variable20 NpConns_variables[] = {
        {NPCONNSACCEPTS,            COUNTER,   RONLY, var_npconns, 1, {1}},
        {NPCONNSREFUSED,            COUNTER,   RONLY, var_npconns, 1, {2}},
        {NPCONNSDENYS,              COUNTER,   RONLY, var_npconns, 1, {3}},
        {NPCONNSDENYSIP,            IPADDRESS, RONLY, var_npconns, 1, {4}},
        {NPCONNSABORTS,             COUNTER,   RONLY, var_npconns, 1, {5}},
        {NPCONNSABORTREASON,        STRING,    RONLY, var_npconns, 1, {6}},
        {NPCONNSABORTIP,            IPADDRESS, RONLY, var_npconns, 1, {7}},
        {NPCONNSABORTPORT,          INTEGER,   RONLY, var_npconns, 1, {8}},
        {NPCONNSABORTTIME,          TIMETICKS, RONLY, var_npconns, 1, {9}},
        {NPCONNSSTATE,              INTEGER,   RONLY, var_npconns, 1, {10}},
        {NPCONNSIP,                 IPADDRESS, RONLY, var_npconns, 1, {11}},
        {NPCONNSPORT,               INTEGER,   RONLY, var_npconns, 1, {12}},
        {NPCONNSPERIPCLOSE,         INTEGER,   RONLY, var_npconns, 1, {13}},
        {NPCONNSIDLETIMEOUTS,       INTEGER,   RONLY, var_npconns, 1, {14}},
        {NPCONNSNMCLOSE,            INTEGER,   RONLY, var_npconns, 1, {15}},
        {NPCONNSBYTESRECVD,         COUNTER,   RONLY, var_npconns, 1, {16}},
        {NPCONNSBYTESSENTS,         COUNTER,   RONLY, var_npconns, 1, {17}}
};
    //      The Card Configuration subgroup of npCard
struct variable20 NpCfg_variables[] = {
        {NPCFGSOURCE,               INTEGER,   RWRITE,var_npcfg, 1, {1}},
        {NPCFGYIADDR,               IPADDRESS, RWRITE,var_npcfg, 1, {2}},
        {NPCFGSIADDR,               IPADDRESS, RONLY, var_npcfg, 1, {3}},
        {NPCFGGIADDR,               IPADDRESS, RONLY, var_npcfg, 1, {4}},
        {NPCFGLOGSERVER,            IPADDRESS, RONLY, var_npcfg, 1, {5}},
        {NPCFGSYSLOGFACILITY,       INTEGER,   RONLY, var_npcfg, 1, {6}},
        {NPCFGACCESSSTATE,          INTEGER,   RONLY, var_npcfg, 1, {7}},
        {NPCFGACCESSLISTNUM,        INTEGER,   RONLY, var_npcfg, 1, {8}},
        {NPCFGACCESSLISTINDEX,      INTEGER,   RONLY, var_npcfg, 3, {9,1,1}},
        {NPCFGACCESSLISTADDRESS,    IPADDRESS, RONLY, var_npcfg, 3, {9,1,2}},
        {NPCFGACCESSLISTADDRMASK,   IPADDRESS, RONLY, var_npcfg, 3, {9,1,3}},
        {NPCFGIDLETIMEOUT,          INTEGER,   RWRITE,var_npcfg, 1, {10}},
        {NPCFGLOCALSUBNETS,         INTEGER,   RONLY, var_npcfg, 1, {11}},
    //      The Unknown data
        {NPCFG12,                   IPADDRESS, RWRITE,var_npcfg, 1, {12}},
        {NPCFG13,                   IPADDRESS, RWRITE,var_npcfg, 1, {13}}
};
    //      The TCP subgroup of npCard
struct variable20 NpTcp_variables[] = {
        {NPTCPINSEGINORDER,         COUNTER,   RONLY, var_nptcp, 1, {1}},
        {NPTCPINSEGOUTOFORDER,      COUNTER,   RONLY, var_nptcp, 1, {2}},
        {NPTCPINSEGZEROPROBE,       COUNTER,   RONLY, var_nptcp, 1, {3}},
        {NPTCPINDISCARDS,           COUNTER,   RONLY, var_nptcp, 1, {4}}
};
    //      The Card Control subgroup of npCard
struct variable20 NpCtl_variables[] = {
        {NPCTLRECONFIGIP,           IPADDRESS, RONLY,  var_npctl, 1, {1}},
        {NPCTLRECONFIGPORT,         INTEGER,   RONLY,  var_npctl, 1, {2}},
        {NPCTLRECONFIGTIME,         TIMETICKS, RONLY,  var_npctl, 1, {3}},
        {NPCTLCLOSEIP,              IPADDRESS, RONLY,  var_npctl, 1, {4}},
        {NPCTLCLOSEPORT,            INTEGER,   RONLY,  var_npctl, 1, {5}},
        {NPCTLIMAGEDUMP,            INTEGER,   RWRITE, var_npctl, 1, {6}},
        {NPCTLCLOSECONNECTION,      INTEGER,   RWRITE, var_npctl, 1, {7}},
        {NPCTLRECONFIG,             INTEGER,   RWRITE, var_npctl, 1, {8}},
        {NPCTLPROTOCOLSET,          INTEGER,   RWRITE, var_npctl, 1, {9}},
    //      The Unknown data
        {NPCTL10,                   INTEGER,   RWRITE, var_npctl, 1, {10}},
        {NPCTL11,                   INTEGER,   RWRITE, var_npctl, 1, {11}},
        {NPCTL12,                   INTEGER,   RWRITE, var_npctl, 1, {12}}
};
    //  --- The HP Modular Input/Output (MIO) subgroup of npCard
struct variable20 NpNpi_variables[] = {
         //  --- The Card Status Entry
        {NPNPICSEDATASTATE,         INTEGER,   RONLY, var_npnpi, 2, {1,1}},
        {NPNPICSEERRORCODE,         INTEGER,   RONLY, var_npnpi, 2, {1,2}},
        {NPNPICSELINKEVENT,         INTEGER,   RONLY, var_npnpi, 2, {1,3}},
        {NPNPICSEREADMODE,          INTEGER,   RONLY, var_npnpi, 2, {1,4}},
        {NPNPICSEWRITEMODE,         INTEGER,   RONLY, var_npnpi, 2, {1,5}},
        {NPNPICSEWARNINGCODE,       INTEGER,   RONLY, var_npnpi, 2, {1,6}},
        {NPNPICSECONNECTIONSTATE,   INTEGER,   RONLY, var_npnpi, 2, {1,7}},
        {NPNPICSENOVWARNINGCODE,    INTEGER,   RONLY, var_npnpi, 2, {1,8}},
        {NPNPICSELLCWARNINGCODE,    INTEGER,   RONLY, var_npnpi, 2, {1,9}},
        {NPNPICSETCPWARNINGCODE,    INTEGER,   RONLY, var_npnpi, 2, {1,10}},
        {NPNPICSEATKWARNINGCODE,    INTEGER,   RONLY, var_npnpi, 2, {1,11}},
        //   --- The Peripheral Attribute Entry
        {NPNPIPERIPHERALATTRIBUTECOUNT,  INTEGER,   RONLY, var_npnpi, 1, {2}},
        {NPNPIPAELINKDIRECTION,     INTEGER,   RONLY, var_npnpi, 2, {3,1}},
        {NPNPIPAECLASS,             INTEGER,   RONLY, var_npnpi, 2, {3,2}},
        {NPNPIPAEIDENTIFICATION,    INTEGER,   RONLY, var_npnpi, 2, {3,3}},
        {NPNPIPAEREVISION,          INTEGER,   RONLY, var_npnpi, 2, {3,4}},
        {NPNPIPAEAPPLETALK,         INTEGER,   RONLY, var_npnpi, 2, {3,5}},
        {NPNPIPAEMESSAGE,           INTEGER,   RONLY, var_npnpi, 2, {3,6}},
        {NPNPIPAERESERVED,          INTEGER,   RONLY, var_npnpi, 2, {3,7}},
        {NPNPIPAEMULTICHAN,         INTEGER,   RONLY, var_npnpi, 2, {3,8}},
        {NPNPIPAEPAD,               INTEGER,   RONLY, var_npnpi, 2, {3,9}},
        //   --- The Card Attribute Entry
        {NPNPICAELINKDIRECTION,     INTEGER,   RONLY, var_npnpi, 2, {4,1}},
        {NPNPICAECLASS,             INTEGER,   RONLY, var_npnpi, 2, {4,2}},
        {NPNPICAEIDENTIFCATION,     INTEGER,   RONLY, var_npnpi, 2, {4,3}},
        {NPNPICAEREVISION,          INTEGER,   RONLY, var_npnpi, 2, {4,4}},
        {NPNPICAEAPPLETALK,         INTEGER,   RONLY, var_npnpi, 2, {4,5}},
        {NPNPICAEMESSAGE,           INTEGER,   RONLY, var_npnpi, 2, {4,6}},
        {NPNPICAEREVERSED,          INTEGER,   RONLY, var_npnpi, 2, {4,7}},
        {NPNPICAEMULTICHAN,         INTEGER,   RONLY, var_npnpi, 2, {4,8}}
};
    //      The IPX subgroup of npCard
struct variable20 NpIpx_variables[] = {
        {NPIPXGETUNITCFGRESP,       STRING,    RONLY, var_npipx, 1, {1}},
        {NPIPX8022FRAMETYPE,        INTEGER,   RONLY, var_npipx, 1, {2}},
        {NPIPXSNAPFRAMETYPE,        INTEGER,   RONLY, var_npipx, 1, {3}},
        {NPIPXETHERNETFRAMETYPE,    INTEGER,   RONLY, var_npipx, 1, {4}},
        {NPIPX8023RAWFRAMETYPE,     INTEGER,   RONLY, var_npipx, 1, {5}},
    //      The Unknown data
        {NPIPX6,                    STRING,    RONLY, var_npipx, 1, {6}},
        {NPIPX7,                    STRING,    RONLY, var_npipx, 1, {7}},
        {NPIPX8,                    STRING,    RWRITE,var_npipx, 1, {8}},
        {NPIPX9,                    STRING,    RWRITE,var_npipx, 1, {9}},
        {NPIPX10Entry_1,            STRING,    RWRITE,var_npipxEntry, 2, {10,1}},
//   {NPIPX13,                    STRING,    RWRITE,var_npipx, 1, {13}},
        {NPIPX16,                   INTEGER,   RWRITE,var_npipx, 1, {16}}
};

    //      The Unknown subgroup of npCard
struct variable20 NpCard11_variables[] = {
        {NPCARD11_1,       INTEGER,    RONLY, var_npcard11, 1, {1}}
};

    //      The Unknown subgroup of npCard
struct variable20 NpCard12_variables[] = {
        {NPCARD12_7,       INTEGER,    RONLY, var_npcard12, 1, {7}},
        {NPCARD12_8,       STRING,     RONLY, var_npcard12, 1, {8}}
};

    //      The Unknown subgroup of npCard
struct variable20 NpCard13_variables[] = {
        {NPCARD13_1,       INTEGER,    RONLY, var_npcard13, 1, {1}},
        {NPCARD13_3,       INTEGER,    RWRITE,var_npcard13, 1, {3}},
        {NPCARD13_4,       INTEGER,    RWRITE,var_npcard13, 1, {4}}
};

    //      The Unknown subgroup of npCard
struct variable20 NpCard17_variables[] = {
        {NPCARD17_1,       INTEGER,    RWRITE,var_npcard17, 1, {1}}
};

    //      The Unknown subgroup of npCard
/* struct variable20 NpCard18_variables[] = {
        {NPCARD18_1,       INTEGER,    RONLY,var_npcard18, 1, {1}}
}; */
#endif WEBADMIN // modified ---- Arius
// ------ End of here  >>>> By Arius


void snmp_vars_init(void)
{
        {
                static oid base[] = {MIB_BASE, 1};

                mib_register(base, 7, (struct variable *)system_variables,
                    sizeof(system_variables)/sizeof(*system_variables),
                    sizeof(*system_variables));
        }

        {
                static oid base[] = {MIB_BASE, 2};

                mib_register (base, 7, (struct variable *)interface_variables,
                    sizeof(interface_variables)/sizeof(*interface_variables),
                    sizeof(*interface_variables));
        }

        {
                static oid base[] = {MIB_BASE, 3, 1, 1};

                mib_register (base, 9, (struct variable *)at_variables,
                    sizeof(at_variables)/sizeof(*at_variables),
                    sizeof(*at_variables));
        }

    {
                static oid base[] = {MIB_BASE, 4};

                mib_register (base, 7, (struct variable *)ip_variables,
                    sizeof(ip_variables)/sizeof(*ip_variables),
                    sizeof(*ip_variables));
        }

    {
                static oid base[] = {MIB_BASE, 5};

                mib_register (base, 7, (struct variable *)icmp_variables,
                    sizeof(icmp_variables)/sizeof(*icmp_variables),
                    sizeof(*icmp_variables));
        }

        {
                static oid base[] = {MIB_BASE, 6};

                mib_register (base, 7, (struct variable *)tcp_variables,
                        sizeof(tcp_variables)/sizeof(*tcp_variables),
                        sizeof(*tcp_variables));
        }

        {
                static oid base[] = {MIB_BASE, 7};

                mib_register (base, 7, (struct variable *)udp_variables,
                        sizeof(udp_variables)/sizeof(*udp_variables),
                        sizeof(*udp_variables));
        }

/*        {
                static oid base[] = {MIB_BASE, 11};

                mib_register (base, 7, (struct variable *)snmp_variables,
                        sizeof(snmp_variables)/sizeof(*snmp_variables),
                        sizeof(*snmp_variables));
        }   */


// Adding for enterprise's MIB data. Here print server will support
// HP jetadmin/web jetadmin
// 05/12/2000 ---- add by Arius
#ifdef WEBADMIN // modified ---- Arius

        // ENTERPRISE = 1, 3, 6, 1, 4, 1
if( PSMode2 & PS_WEBJETADMIN_ON )
{
        {
             // GENERALDEVICESTATUS = 11, 2, 3, 9, 1, 1
             static oid base[] = {ENTERPRISE, GENERALDEVICESTATUS};

             mib_register(base, 12, (struct variable *)GeneralDeviceStatus_variables,
                 sizeof(GeneralDeviceStatus_variables)/sizeof(*GeneralDeviceStatus_variables),
                 sizeof(*GeneralDeviceStatus_variables));
        }

        // NPCARD     11, 2, 4, 3
        {
             // NPSYS      11, 2, 4, 3 ,1
             static oid base[] = {ENTERPRISE, NPCARD, 1};

             mib_register(base, 11, (struct variable *)NpSys_variables,
                 sizeof(NpSys_variables)/sizeof(*NpSys_variables),
                 sizeof(*NpSys_variables));
        }

        {
             // NPCONNS      11, 2, 4, 3 ,4
             static oid base[] = {ENTERPRISE, NPCARD, 4};

             mib_register(base, 11, (struct variable *)NpConns_variables,
                 sizeof(NpConns_variables)/sizeof(*NpConns_variables),
                 sizeof(*NpConns_variables));
        }

        {
             // NPCFG      11, 2, 4, 3 ,5
             static oid base[] = {ENTERPRISE, NPCARD, 5};

             mib_register(base, 11, (struct variable *)NpCfg_variables,
                 sizeof(NpCfg_variables)/sizeof(*NpCfg_variables),
                 sizeof(*NpCfg_variables));
        }

        {
             // NPTCP      11, 2, 4, 3 ,6
             static oid base[] = {ENTERPRISE, NPCARD, 6};

             mib_register(base, 11, (struct variable *)NpTcp_variables,
                 sizeof(NpTcp_variables)/sizeof(*NpTcp_variables),
                 sizeof(*NpTcp_variables));
        }

        {
             // NPCTL      11, 2, 4, 3 ,7
             static oid base[] = {ENTERPRISE, NPCARD, 7};

             mib_register(base, 11, (struct variable *)NpCtl_variables,
                 sizeof(NpCtl_variables)/sizeof(*NpCtl_variables),
                 sizeof(*NpCtl_variables));
        }

        {
             // NPNPI      11, 2, 4, 3 ,8
             static oid base[] = {ENTERPRISE, NPCARD, 8};

             mib_register(base, 11, (struct variable *)NpNpi_variables,
                 sizeof(NpNpi_variables)/sizeof(*NpNpi_variables),
                 sizeof(*NpNpi_variables));
        }

        {
             // NPIPX      11, 2, 4, 3 ,10
             static oid base[] = {ENTERPRISE, NPCARD, 10};

             mib_register(base, 11, (struct variable *)NpIpx_variables,
                 sizeof(NpIpx_variables)/sizeof(*NpIpx_variables),
                 sizeof(*NpIpx_variables));
        }

        {
             // Unknown(NPCARD 11)      11, 2, 4, 3 ,11
             static oid base[] = {ENTERPRISE, NPCARD, 11};

             mib_register(base, 11, (struct variable *)NpCard11_variables,
                 sizeof(NpCard11_variables)/sizeof(*NpCard11_variables),
                 sizeof(*NpCard11_variables));
        }

        {
             // Unknown(NPCARD 12)      11, 2, 4, 3 ,12
             static oid base[] = {ENTERPRISE, NPCARD, 12};

             mib_register(base, 11, (struct variable *)NpCard12_variables,
                 sizeof(NpCard12_variables)/sizeof(*NpCard12_variables),
                 sizeof(*NpCard12_variables));
        }

        {
             // Unknown(NPCARD 13)      11, 2, 4, 3 ,13
             static oid base[] = {ENTERPRISE, NPCARD, 13};

             mib_register(base, 11, (struct variable *)NpCard13_variables,
                 sizeof(NpCard13_variables)/sizeof(*NpCard13_variables),
                 sizeof(*NpCard13_variables));
        }

        {
             // Unknown(NPCARD 17)      11, 2, 4, 3 ,17
             static oid base[] = {ENTERPRISE, NPCARD, 17};

             mib_register(base, 11, (struct variable *)NpCard17_variables,
                 sizeof(NpCard17_variables)/sizeof(*NpCard17_variables),
                 sizeof(*NpCard17_variables));
        }

//        {
//             // Unknown(NPCARD 18)      11, 2, 4, 3 ,18
//             static oid base[] = {ENTERPRISE, NPCARD, 18};
//
//             mib_register(base, 11, (struct variable *)NpCard18_variables,
//                 sizeof(NpCard18_variables)/sizeof(*NpCard18_variables),
//                 sizeof(*NpCard18_variables));
//        }
}

#endif WEBADMIN // modified ---- Arius
// ------ End of here  >>>> By Arius
}


//*
//* add an mib-entry to the subtrees list.
//* chain in at correct position.
//*

void mib_register (
        oid *oid_base,
        int oid_base_len,
        struct variable *mib_variables,
        int mib_variables_len,
        int mib_variables_width
)
{
        struct subtree **sptr;
        struct subtree *new_subtree = TALLOC(struct subtree);

#if defined(SNMP_DUMP) && defined(PC_OUTPUT)
        if (new_subtree == NULL) {
                AtSaySpace(0,20,40);
                printf("error: registering mib: out of memory...aborting.\n");
                ErrorBeep();
                exit(1);
        }
#endif

#ifdef PC_OUTPUT
        if(oid_base_len > SNMP_MAX_BASE_LEN) {
                AtSaySpace(0,20,40);
                printf("mib_register: Desgin Error !!!");
                ErrorBeep();
                exit(1);
        }
#endif

        //*
        //* fill in new subtree element:
        //*
        memcpy (new_subtree->name, oid_base, oid_base_len * sizeof(oid));
        new_subtree->namelen = oid_base_len;
        new_subtree->variables = mib_variables;
        new_subtree->variables_len = mib_variables_len;
        new_subtree->variables_width = mib_variables_width;

        //*
        //* now hop along the subtrees and chain in:
        //*
        for (sptr = &subtrees; *sptr; sptr = &(*sptr)->next) {
                if (compare ((*sptr)->name, (*sptr)->namelen,
                    new_subtree->name, new_subtree->namelen) > 0) break;
        }
        new_subtree->next = *sptr;
        *sptr = new_subtree;
}

int compare(oid *name1, int len1, oid *name2, int len2)
{
        int    len;

        // len = minimum of len1 and len2
        if (len1 < len2)        len = len1;
        else                len = len2;

    // find first non-matching byte
        while(len-- > 0){
                if (*name1 < *name2) {
//              cmpprintf (" giving -1\n");
                        return -1;
                }
                if (*name2++ < *name1++) {
//              cmpprintf (" giving 1\n");
                        return 1;
                }
        }

    // bytes match up to length of shorter string
        if (len1 < len2) {
//              cmpprintf (" giving -1\n");
                return -1;  /* name1 shorter, so it is "less" */
        }

        if (len2 < len1) {
//      cmpprintf (" giving 1\n");
                return 1;
        }

//      cmpprintf (" giving 0\n");

        return 0;       // both strings are equal
}

//*
//* getStatPtr - return a pointer to the named variable, as well as it's
//* type, length, and access control list.
//*
//* If an exact match for the variable name exists, it is returned.  If not,
//* and exact is false, the next variable lexicographically after the
//* requested one is returned.
//*
//* If no appropriate variable can be found, NULL is returned.
//*
uint8   *
getStatPtr(
        oid         *name,          // IN - name of var, OUT - name matched
        int         *namelen,   // IN -number of sub-ids in name, OUT - subid-is in matched name
        uint8  *type,      // OUT - type of matched variable
        int         *len,       // OUT - length of matched variable
        uint16  *acl,       // OUT - access control list
        int         exact,      // IN - TRUE if exact match wanted
        int         (**write_method)(), // OUT - pointer to function called to set variable, otherwise 0
        int         snmpversion,
        int         *noSuchObject,
        int         view
)
{
        struct subtree  *tp;
        struct variable *vp = 0;
        struct variable compat_var;
        struct variable *cvp = &compat_var;
        int     x;
        uint8  *access = NULL;
        int         result, treeresult;
        oid     *suffix;
        int         suffixlen;
        int     found = FALSE;
        oid         save[MAX_NAME_LEN];
        int         savelen = 0;

        if(view == 0 )  {
                if(_SnmpTrapEnable == SNMP_TRAP_ENABLE && _SnmpAuthenTrap == SNMP_TRAP_AUTH_ENABLE)
                        _SendAuthFailTrap();
                write_method = NULL;
                return NULL; //no access right
        }

        if (!exact){
                bcopy(name, save, *namelen * sizeof(oid));
                savelen = *namelen;
        }

        *write_method = NULL;
        for (tp = subtrees; tp; tp = tp->next) {

                treeresult = compare_tree(name, *namelen, tp->name, (int)tp->namelen);

                // if exact and treerresult == 0 ; if next  and treeresult <= 0
                if (treeresult == 0 || (!exact && treeresult < 0)){

                        result = treeresult;
                        suffixlen = *namelen - tp->namelen;
                        suffix = name + tp->namelen;

                        //
                        //                              +----> suffix
                        //+---------------------V---------------+
                        //| name ........       |               |
                        //+---------------------+---------------+
                        //+ Same as tp->name  --+-- suffixlen --+
                        //                             |
                        //                                                         +----> Compare with vp->name

                        // the following is part of the setup for the compatability
                        // structure below that has been moved out of the main loop.
                        bcopy((char *)tp->name, (char *)cvp->name,tp->namelen * sizeof(oid));

                        for(x = 0, vp = tp->variables; x < tp->variables_len;
                            vp =(struct variable *)((char *)vp +tp->variables_width), x++)
                        {
                                // if exact and ALWAYS ; if next  and result >= 0
                                if (exact || result >= 0){
                                        result = compare_tree(suffix, suffixlen, vp->name,
                                                 (int)vp->namelen);
                                }

                                // if exact and result == 0; if next  and result <= 0
                                if ((!exact && (result <= 0)) || (exact && (result == 0))){
                                        //!!! find it !!!

                                        // builds an old (long) style variable structure to retain
                                        // compatability with var_* functions written previously.
                                        bcopy((char *)vp->name, (char *)(cvp->name + tp->namelen),
                                               vp->namelen * sizeof(oid));
                                        cvp->namelen = tp->namelen + vp->namelen;
                                        cvp->type = vp->type;
                                        cvp->magic = vp->magic;
                                        cvp->acl = vp->acl;
                                        cvp->findVar = vp->findVar;

                                        access = (*(vp->findVar))(cvp, name, namelen, exact,
                                                 len, write_method);

                                        if (write_method) *acl = vp->acl;

                                        if(!access)
                                        {
                                                access = NULL;
                                                *write_method = NULL;
                                        } else if (exact){
                                                found = TRUE;
                                        }

                                        // this code is incorrect if there is
                                        // a view configuration that exludes a particular
                                        // instance of a variable.  It would return noSuchObject,
                                        // which would be an error
                                        if (access != NULL)     break;
                                }

                                // if exact and result <= 0
                                if (exact && (result  <= 0)){
                                        *type = vp->type;
                                        *acl = vp->acl;
                                        if (found) *noSuchObject = FALSE;
                                        else *noSuchObject = TRUE;
                                        return NULL;
                                }
                        }
                        if (access != NULL)     break;
                }
        }
        if (! tp ){
                if (!access && !exact){
                        bcopy(save, name, savelen * sizeof(oid));
                        *namelen = savelen;
                }
                if (found)  *noSuchObject = FALSE;
                else        *noSuchObject = TRUE;

                return NULL;
        }
        // vp now points to the approprate struct
        *type = vp->type;
        *acl = vp->acl;
        return access;
}

static int compare_tree(oid *name1, int len1, oid *name2, int len2)
{
        int    len;

        // len = minimum of len1 and len2
        if (len1 < len2) len = len1;
        else len = len2;

    // find first non-matching byte
        while(len-- > 0){
                if (*name1 < *name2) return -1;
                if (*name2++ < *name1++) return 1;
        }

        // bytes match up to length of shorter string
        if (len1 < len2) return -1;  //name1 shorter, so it is "less"

        // name1 matches name2 for length of name2, or they are equal
        return 0;
}

//-------------- MIB-II Var-Find function ------------------------

static int writeSystem(int action, uint8 *var_val, uint8 var_val_type,
                       int var_val_len, uint8 *statP,
                       oid *name, int name_len);

static int EnableAuthenTraps(int action, uint8 *var_val, uint8 var_val_type,
                       int var_val_len, uint8 *statP,
                       oid *name, int name_len);

//*
//* rfc 1907; the system group.
//*

uint8 *
var_system(
        struct variable *vp,       // IN - pointer to variable entry that points here
        oid        *name,              // IN/OUT - input name requested, output name found
        int        *length,            // IN/OUT - length of input and output oid's
        int        exact,              // IN - TRUE if an exact match was requested.
        int        *var_len,           // OUT - length of variable or 0 if function returned.
        int        (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[8] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
                return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic){
                case VERSION_DESCR:
                        *var_len = strlen(version_descr);

                        // not writable:
                        // *write_method = writeVersion;

                        return (uint8 *)version_descr;
                case VERSION_ID:
                        *var_len = sizeof(SnmpVersionID);
                        return SnmpVersionID;
                case UPTIME:
                case SYSORLASTCHANGE:
                        long_return = msclock() / 10;
                        return (uint8 *)&long_return;
                case IFNUMBER:
                    long_return = Interface_Scan_Get_Count();
                    return (uint8 *) &long_return;
                case SYSCONTACT:
                    *var_len = strlen(_SnmpSysContact);
                    *write_method = writeSystem;
                    return (uint8 *)_SnmpSysContact;
                case SYSYSNAME:
                    *var_len = strlen(Hostname);
                    *write_method = writeSystem;
                    return (uint8 *)Hostname;
        case SYSLOCATION:
                    *var_len = strlen(_SnmpSysLocation);
                    *write_method = writeSystem;
                    return (uint8 *)_SnmpSysLocation;
                case SYSSERVICES:
                        long_return = 79;
                        return (uint8 *)&long_return;
                default:
                        ERROR("");
        }
        return NULL;
}

int
writeSystem(
   int     action,
   uint8   *var_val,
   uint8   var_val_type,
   int     var_val_len,
   uint8   *statP,
   oid     *name,
   int     name_len
)
{
        uint8 buf[SNMP_SYS_MAX_RW_LEN+1], *cp;
        int count, size;
        int bigsize = 1000;

        if(var_val_type != STRING){
#ifdef PC_OUTPUT
                printf("not string\n");
#endif
                return SNMP_ERR_WRONGTYPE;
        }


        if(var_val_len > SNMP_SYS_MAX_RW_LEN){
#ifdef PC_OUTPUT
                printf("bad length\n");
#endif
                return SNMP_ERR_WRONGLENGTH;
        }

        size = sizeof(buf);

        asn_parse_string(var_val, &bigsize, &var_val_type, buf, &size);
        for(cp = buf, count = 0; count < size; count++, cp++){
                if (!isprint(*cp)){
#ifdef PC_OUTPUT
                printf("not print %x\n", *cp);
#endif PC_OUTPUT
                        return SNMP_ERR_WRONGVALUE;
                }
    }

        buf[size] = 0;

        switch((char)name[7]){
                case 4:
                        if(size >= SNMP_SYSCONTACT_LEN){
#ifdef PC_OUTPUT
                                printf("Bad length:SysContact\n");
#endif
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        break;
                case 5:
                        if(size > LENGTH_OF_BOX_NAME){
#ifdef PC_OUTPUT
                                printf("Bad length:HostName\n");
#endif
                                return SNMP_ERR_WRONGLENGTH;
                        }
                    break;
                case 6:
                        if(size >= SNMP_SYSLOCATION_LEN){
#ifdef PC_OUTPUT
                                printf("Bad length:SysLocation\n");
#endif
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        break;
                default:
                        return SNMP_ERR_NOSUCHNAME;
        }

        if (action == COMMIT){
                size++;
                switch((char)name[7]){
                case 4:
                        memcpy(_SnmpSysContact, buf,size);
                        break;
                case 5:
                        memcpy(Hostname, buf,size);
                        memcpy(_BoxName,Hostname,LENGTH_OF_BOX_NAME);
                    break;
                case 6:
                        memcpy(_SnmpSysLocation, buf,size);
                        break;
                default:
                        return SNMP_ERR_NOSUCHNAME;
                }

#ifndef _PC
 			if(WriteToEEPROM(&EEPROM_Data) != 0) LightOnForever(Status_Lite); // Write EEPROM Data
#endif
        }
        return SNMP_ERR_NOERROR;
}

//--------- interface group --------------------

uint8 *
var_ifEntry(
        struct variable *vp,    // IN - pointer to variable entry that points here
        oid     *name,              // IN/OUT - input name requested, output name found
        int *length,            // IN/OUT - length of input and output oid's
        int     exact,              // IN - TRUE if an exact match was requested.
        int     *var_len,           // OUT - length of variable or 0 if function returned.
        int     (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int     interface,i;

        static char Name[80];
        char *cp;

        int result, count;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        // find "next" interface
        count = Interface_Scan_Get_Count();

        for(interface = 1; interface <= count; interface++){
                newname[10] = (oid)interface;
                result = compare(name, *length, newname, (int)vp->namelen + 1);
                if ((exact && (result == 0)) || (!exact && (result < 0))) break;
        }
    if (interface > count) return NULL;

        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);

    switch (vp->magic){
        case IFINDEX:
                long_return = interface;
                return (uint8 *) &long_return;
        case IFDESCR:
                *var_len = sizeof(InterfaceIfDescr)-1;
            return (uint8 *)InterfaceIfDescr;
        case IFTYPE:
                long_return = 6;        //ethernetCsmacd
                return (uint8 *) &long_return;
        case IFMTU:
                long_return = (long) DEFAULT_MAX_MTU;
                return (uint8 *) &long_return;
        case IFSPEED:
                long_return = 10000000;
                return (uint8 *) &long_return;
        case IFPHYSADDRESS:
                memcpy(return_buf,MyPhysNodeAddress,6);
                *var_len = 6;
                return(uint8 *) return_buf;
        case IFADMINSTATUS:
                long_return = 1;  //UP
                return (uint8 *) &long_return;
        case IFOPERSTATUS:
                long_return = 1;  //UP
                return (uint8 *) &long_return;
        case IFLASTCHANGE:
                long_return = 0;                        /* XXX */
                return (uint8 *) &long_return;
        case IFINOCTETS:
                long_return = mib_ifInOctets;
                return (uint8 *) &long_return;
        case IFINUCASTPKTS:
                long_return = mib_ifInUcastPkts;
                return (uint8 *) &long_return;
        case IFINNUCASTPKTS:
                long_return = mib_ifInNucastPkts;
                return (uint8 *) &long_return;
        case IFINDISCARDS:
                long_return = mib_ifInDiscards;
                return (uint8 *) &long_return;
        case IFINERRORS:
                long_return = 0;
                return (uint8 *) &long_return;
        case IFINUNKNOWNPROTOS:
                long_return = mib_ifInUnknowProtos;
                return (uint8 *) &long_return;
        case IFOUTOCTETS:
                long_return = mib_ifOutOctets;
                return (uint8 *) &long_return;
        case IFOUTUCASTPKTS:
                long_return = mib_ifOutUcastPkts;
                return (uint8 *) &long_return;
        case IFOUTNUCASTPKTS:
            	long_return = mib_ifOutNucastPkts;
                return (uint8 *) &long_return;
        case IFOUTDISCARDS:
                long_return = 0;
                return (uint8 *) &long_return;
        case IFOUTERRORS:
                long_return = mib_ifOutErrors;
                return (uint8 *) &long_return;
        case IFOUTQLEN:
//615wu
				return (uint8 *) &long_return;
#if 0 
                {
                        struct mbuf *TmpQueueBuf = Hopper;
                        long_return = 0;
                        while(TmpQueueBuf != NULL) {
                            long_return++;
                                TmpQueueBuf = TmpQueueBuf->anext;
                        }
                        return (uint8 *) &long_return;
                }
#endif
        case IFSPECIFIC:
                *var_len = sizeof(oid)* 2;
                for(i = 0 ; i < *var_len;i++) return_buf[i] = 0;  // { 0, 0}
                return (uint8 *) return_buf;
        default:
                ERROR("");
        }
        return NULL;
}

//--- at group (ARP table)

uint8 *
var_atEntry(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid     *name,       // IN/OUT - input name requested, output name found
        int         *length,     // IN/OUT - length of input and output oid's
        int         exact,           // IN - TRUE if an exact match was requested.
        int         *var_len,    // OUT - length of variable or 0 if function returned.
        int         (**write_method)() // OUT - pointer to function to set variable, otherwise 0 */
)
{
    //*
    //* object identifier is of form:
    //* 1.3.6.1.2.1.3.1.1.1.interface.1.A.B.C.D,  where A.B.C.D is IP address.
    //* Interface is at offset 10,
    //* IPADDR starts at offset 12.
    //*

    uint8                   *cp;
    oid                     lowest[16];
    oid                     current[16];
        static uint8    PhysAddr[6], LowPhysAddr[6];
    uint32                  Addr, LowAddr;

    /* fill in object part of name for current (less sizeof instance part) */

    bcopy((char *)vp->name, (char *)current, (int)vp->namelen * sizeof(oid));

    LowAddr = -1;      // Don't have one yet
//615wu
#if 0
        ARP_Scan_Init();
        for (;;) {
                if(ARP_Scan_Next(&Addr, PhysAddr) == 0) break;
                current [10] = 1;  // IfIndex == 1
                //1.3.6.1.2.1.3.1.1.1.interface.1.A.B.C.D
                //                       +
                //                                               +----> current[10]

                current[11] = 1;
                cp = (uint8 *)&Addr;
                current[12] = cp[3];
                current[13] = cp[2];
                current[14] = cp[1];
                current[15] = cp[0];

                if(exact){
                        if(compare(current, 16, name, *length) == 0){
                                bcopy((char *)current, (char *)lowest, 16 * sizeof(oid));
                                LowAddr = Addr;
                                bcopy(PhysAddr, LowPhysAddr, sizeof(PhysAddr));
                                break;  // no need to search further
                        }
                } else {
                        if((compare(current, 16, name, *length) > 0) &&
                                 ((LowAddr == -1) || (compare(current, 16, lowest, 16) < 0))){
                                //*
                                //* if new one is greater than input and closer to input than
                                //* previous lowest, save this one as the "next" one.
                                //*
                                bcopy((char *)current, (char *)lowest, 16 * sizeof(oid));
                                LowAddr = Addr;
                                bcopy(PhysAddr, LowPhysAddr, sizeof(PhysAddr));
                        }
                }
    }
#endif
    
    if (LowAddr == -1) return(NULL);

        bcopy((char *)lowest, (char *)name, 16 * sizeof(oid));
        *length = 16;
        *write_method = 0;
        switch(vp->magic){
        case ATIFINDEX:
                *var_len = sizeof(long_return);
                long_return = 1;
                return (uint8 *)&long_return;
        case ATPHYSADDRESS:
                *var_len = sizeof(LowPhysAddr);
                return (uint8 *)LowPhysAddr;
        case ATNETADDRESS:
                long_return = DWordSwap(LowAddr);
                *var_len = sizeof(long_return);
                return (uint8 *)&long_return;
        default:
                ERROR("");
        }
        return NULL;
}

//-------- ip group

uint8 *
var_ip(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,    // IN/OUT - input name requested, output name found
        int    *length,  // IN/OUT - length of input and output oid's.
        int    exact,    // IN - TRUE if an exact match was requested.
        int    *var_len, // OUT - length of variable or 0 if function returned.
        int    (**write_method)() // OUT - pointer to function to set variable, otherwise 0
)
{
    int result;

    //1.3.6.1.2.1.4.X.0

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[8] = 0;

        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if((exact && (result != 0)) || (!exact && (result >= 0))) return NULL;

        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;

        *write_method = 0;
        *var_len = sizeof(long);        /* default length */
        //*
        //*     Get the IP statistics from the kernel...
        //*

    switch (vp->magic){
        case IPFORWARDING:          return (uint8 *) &ipForwarding;
        case IPDEFAULTTTL:      return (uint8 *) &ipDefaultTTL;
        case IPINRECEIVES:      return (uint8 *) &ipInReceives;
        case IPINHDRERRORS:     return (uint8 *) &ipInHdrErrors;
        case IPINADDRERRORS:    return (uint8 *) &ipInAddrErrors;
        case IPFORWDATAGRAMS:   return (uint8 *) &ipForwDatagrams;
        case IPINUNKNOWNPROTOS: return (uint8 *) &ipInUnknownProtos;
        case IPINDISCARDS:      return (uint8 *) &ipInDiscards;
        case IPINDELIVERS:      return (uint8 *) &ipInDelivers;
        case IPOUTREQUESTS:     return (uint8 *) &ipOutRequests;
        case IPOUTDISCARDS:     return (uint8 *) &ipOutDiscards;
        case IPOUTNOROUTES:     return (uint8 *) &ipOutNoRoutes;
        case IPREASMTIMEOUT:    return (uint8 *) &ipReasmTimeout;
        case IPREASMREQDS:      return (uint8 *) &ipReasmReqds;
        case IPREASMOKS:        return (uint8 *) &ipReasmOKs;
        case IPREASMFAILS:      return (uint8 *) &ipReasmFails;
        case IPFRAGOKS:         return (uint8 *) &ipFragOKs;
        case IPFRAGFAILS:       return (uint8 *) &ipFragFails;
        case IPFRAGCREATES:     return (uint8 *) &ipFragCreates;
        default:
            ERROR("");
    }
    return NULL;
}

uint8 *
var_ipAddrEntry(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid     *name,           // IN/OUT - input name requested, output name found
        int     *length,         // IN/OUT - length of input and output oid's
        int     exact,           // IN - TRUE if an exact match was requested.
        int     *var_len,        // OUT - length of variable or 0 if function returned.
        int (**write_method)() // OUT - pointer to function to set variable, otherwise 0
)
{
    //*
    //* object identifier is of form:
    //* 1.3.6.1.2.1.4.20.1.?.A.B.C.D,  where A.B.C.D is IP address.
    //* IPADDR starts at offset 10.
    //*

    oid                     lowest[14];
    oid                     current[14];
    uint8                   *cp;
    int                     interface, lowinterface=0;

        // fill in object part of name for current (less sizeof instance part)

        bcopy((char *)vp->name, (char *)current, (int)vp->namelen * sizeof(oid));

        //* 1.3.6.1.2.1.4.20.1.?.A.B.C.D,  where A.B.C.D is IP address.
        //  0 1 2 3 4 5 6 7  8 9 10

        interface = 1;

//      op = current + 10;

//615wu
        current[10] = EEPROM_Data.BoxIPAddress[0];
        current[11] = EEPROM_Data.BoxIPAddress[1];
        current[12] = EEPROM_Data.BoxIPAddress[2];
        current[13] = EEPROM_Data.BoxIPAddress[3];
#if 0
        current[10] = cp[3];
        current[11] = cp[2];
        current[12] = cp[1];
        current[13] = cp[0];
#endif
        if (exact){
                if (compare(current, 14, name, *length) == 0){
                        bcopy((char *)current, (char *)lowest, 14 * sizeof(oid));
                        lowinterface = interface;
                        //break;
            }
        } else {
                if ((compare(current, 14, name, *length) > 0) &&
                 (!lowinterface || (compare(current, 14, lowest, 14) < 0))){
                        //*
                        //* if new one is greater than input and closer to input than
                        //* previous lowest, save this one as the "next" one.
                        //*/
                        lowinterface = interface;
                        bcopy((char *)current, (char *)lowest, 14 * sizeof(oid));
                }
        }

    if (!lowinterface) return(NULL);

        bcopy((char *)lowest, (char *)name, 14 * sizeof(oid));
        *length = 14;
        *write_method = 0;
        *var_len = sizeof(long_return);
        switch(vp->magic){
        case IPADADDR:
				long_return = NGET32(EEPROM_Data.BoxIPAddress);	//615wu
                return(uint8 *) &long_return;
        case IPADIFINDEX:
                long_return = lowinterface;
                return(uint8 *) &long_return;
        case IPADNETMASK:
                long_return = NGET32(EEPROM_Data.SubNetMask);	//615wu
                return(uint8 *) &long_return;
        case IPADBCASTADDR:
                long_return = (0xffffffffL & 1);	//615wu
                return(uint8 *) &long_return;
        case IPADENTREASMMAXSIZE:
                long_return = (long) DEFAULT_MAX_MTU;
                return (uint8 *) &long_return;
        default:
                ERROR("");
        }
        return NULL;
}

//----- icmp group -----------

uint8 *
var_icmp(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,    // IN/OUT - input name requested, output name found
        int    *length,  // IN/OUT - length of input and output oid's
        int    exact,    // IN - TRUE if an exact match was requested.
        int    *var_len, // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
    int result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[8] = 0;

        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0))) return NULL;

        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;

        *write_method = 0;
        *var_len = sizeof(long); // all following variables are sizeof long

        //*
        //*     Get the ICMP statistics from the kernel...
        //*
    switch (vp->magic){
        case ICMPINMSGS:           return (uint8 *) &icmpInMsgs;
        case ICMPINERRORS:         return (uint8 *) &icmpInErrors;
        case ICMPINDESTUNREACHS:   return (uint8 *) &icmpInDestUnreachs;
        case ICMPINTIMEEXCDS:      return (uint8 *) &icmpInTimeExcds;
        case ICMPINPARMPROBS:      return (uint8 *) &icmpInParmProbs;
        case ICMPINSRCQUENCHS:     return (uint8 *) &icmpInSrcQuenchs;
        case ICMPINREDIRECTS:      return (uint8 *) &icmpInRedirects;
        case ICMPINECHOS:          return (uint8 *) &icmpInEchos;
        case ICMPINECHOREPS:       return (uint8 *) &icmpInEchoReps;
        case ICMPINTIMESTAMPS:     return (uint8 *) &icmpInTimestamps;
        case ICMPINTIMESTAMPREPS:  return (uint8 *) &icmpInTimestampReps;
        case ICMPINADDRMASKS:      return (uint8 *) &icmpInAddrMasks;
        case ICMPINADDRMASKREPS:   return (uint8 *) &icmpInAddrMaskReps;
        case ICMPOUTMSGS:          return (uint8 *) &icmpOutMsgs;
        case ICMPOUTERRORS:        return (uint8 *) &icmpOutErrors;
        case ICMPOUTDESTUNREACHS:  return (uint8 *) &icmpOutDestUnreachs;
        case ICMPOUTTIMEEXCDS:     return (uint8 *) &icmpOutTimeExcds;
        case ICMPOUTPARMPROBS:     return (uint8 *) &icmpOutParmProbs;
        case ICMPOUTSRCQUENCHS:    return (uint8 *) &icmpOutSrcQuenchs;
        case ICMPOUTREDIRECTS:     return (uint8 *) &icmpOutRedirects;
        case ICMPOUTECHOS:         return (uint8 *) &icmpOutEchos;
        case ICMPOUTECHOREPS:      return (uint8 *) &icmpOutEchoReps;
        case ICMPOUTTIMESTAMPS:    return (uint8 *) &icmpOutTimestamps;
        case ICMPOUTTIMESTAMPREPS: return (uint8 *) &icmpOutTimestampReps;
        case ICMPOUTADDRMASKS:     return (uint8 *) &icmpOutAddrMasks;
        case ICMPOUTADDRMASKREPS:  return (uint8 *) &icmpOutAddrMaskReps;
    default:
                ERROR("");
        }
    return NULL;
}

//----- TCP group -----

uint8 *
var_tcp(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        oid lowest[MAX_NAME_LEN];
        uint8 *cp;
        register struct tcb *inpcb, *Lowinpcb;
        int State, i, LowState, result;

    //*
    //* Allow for a kernel w/o TCP
    //*
        if ((vp->magic < TCPCONNSTATE) || (vp->magic == TCPINERRS) || (vp->magic == TCPOUTRSTS)) {

                bcopy((char *)vp->name, (char *)newname,(int)vp->namelen * sizeof(oid));
                newname[8] = 0;

                result = compare(name, *length, newname, (int)vp->namelen + 1);
                if ((exact && (result != 0)) || (!exact && (result >= 0))) return NULL;
                bcopy((char *)newname, (char *)name,((int)vp->namelen + 1) * sizeof(oid));
                *length = vp->namelen + 1;

                *write_method = 0;
                *var_len = sizeof(long);    /* default length */
                //*
                //*  Get the TCP statistics from the kernel...
                //*

                switch (vp->magic){
                case TCPRTOALGORITHM:
                        return (uint8 *) &tcpRtoAlgorithm;
                case TCPRTOMIN:
                        return (uint8 *) &tcpRtoMin;
                case TCPRTOMAX:
                        return (uint8 *) &tcpRtoMax;
                case TCPMAXCONN:
                        return (uint8 *) &tcpMaxConn;
                case TCPACTIVEOPENS:
                        return (uint8 *) &tcpActiveOpens;
                case TCPPASSIVEOPENS:
                        return (uint8 *) &tcpPassiveOpens;
                case TCPATTEMPTFAILS:
                        return (uint8 *) &tcpAttemptFails;
                case TCPESTABRESETS:
                        return (uint8 *) &tcpEstabResets;
                case TCPCURRESTAB:
                        return (uint8 *) &tcpCurrEstab;
                case TCPINSEGS:
                        return (uint8 *) &tcpInSegs;
                case TCPOUTSEGS:
                        return (uint8 *) &tcpOutSegs;
                case TCPRETRANSSEGS:
                        return (uint8 *) &tcpRetransSegs;
                case TCPINERRS:
                        return (uint8 *) &tcpInErrs;
                case TCPOUTRSTS:
                        return (uint8 *) &tcpOutRsts;
                default:
                        ERROR("");
                }
    } 
//615wu    
#if 0    
    else {
                // Info about a particular connection
                bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
                // find "next" connection

                LowState = -1;      // Don't have one yet

                for(inpcb=Tcbs;inpcb != NULL;inpcb = inpcb->next){
                        if(inpcb->conn.remote.address == 0) continue;
                        State = inpcb->state;
                        cp = (uint8 *)&Lanface->addr;
                        newname[10] = cp[3];
                        newname[11] = cp[2];
                        newname[12] = cp[1];
                        newname[13] = cp[0];

                        newname[14] = (inpcb->conn).local.port; //???? 16 bits

                        cp = (uint8 *)&(inpcb->conn).remote.address;
                        newname[15] = cp[3];
                        newname[16] = cp[2];
                        newname[17] = cp[1];
                        newname[18] = cp[0];

                        newname[19] = (inpcb->conn).remote.port;

                        if (exact){
                                if(compare(newname, 20, name, *length) == 0){
                                        bcopy((char *)newname, (char *)lowest, 20 * sizeof(oid));
                                        LowState = State;
                                        Lowinpcb = inpcb;
                                        break;  // no need to search further
                                }
                        } else {
                                if ((compare(newname, 20, name, *length) > 0) &&
                                      ((LowState < 0) || (compare(newname, 20, lowest, 20) < 0))){
                                        //*
                                        //* if new one is greater than input and closer to input than
                                        //* previous lowest, save this one as the "next" one.
                                        //*
                                        bcopy((char *)newname, (char *)lowest, 20 * sizeof(oid));
                                        LowState = State;
                                        Lowinpcb = inpcb;
                                }
                        }
                } //for (

                if (LowState < 0) return(NULL);

                bcopy((char *)lowest, (char *)name, ((int)vp->namelen + 10) * sizeof(oid));
                *length = vp->namelen + 10;
                *write_method = 0;
                *var_len = sizeof(long);
                switch (vp->magic) {
                case TCPCONNSTATE:
                        long_return = State;
                        return (uint8 *) &long_return;
                case TCPCONNLOCALADDRESS:
                        long_return = DWordSwap(Lanface->addr);
                        return (uint8 *) &long_return;
                case TCPCONNLOCALPORT:
                        long_return = (Lowinpcb->conn).local.port;
                        return (uint8 *) &long_return;
                case TCPCONNREMADDRESS:
                        long_return = DWordSwap((Lowinpcb->conn).remote.address);
                        return (uint8 *) &long_return;
                case TCPCONNREMPORT:
                        long_return = (Lowinpcb->conn).remote.port;
                        return (uint8 *) &long_return;
                }
        }
#endif        
        return NULL;
}

//---- UDP group

uint8 *
var_udp(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        oid lowest[MAX_NAME_LEN];
        uint8 *cp;
        struct udp_cb *inpcb, *Lowinpcb;
        int i, LowState, result;

        if (vp->magic <= UDPOUTDATAGRAMS) {
                bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
                newname[8] = 0;
                result = compare(name, *length, newname, (int)vp->namelen + 1);
                if((exact && (result != 0)) || (!exact && (result >= 0))) return NULL;

                bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
                *length = vp->namelen + 1;

                *write_method = 0;
                *var_len = sizeof(long);        /* default length */

        //*
            //* Get the IP statistics from the kernel...
            //*

                switch (vp->magic){
                case UDPINDATAGRAMS:
                        return (uint8 *) &udpInDatagrams;
                case UDPNOPORTS:
                        return (uint8 *) &udpNoPorts;
                case UDPOUTDATAGRAMS:
                        return (uint8 *) &udpOutDatagrams;
                case UDPINERRORS:
                        return (uint8 *) &udpInErrors;
                default:
                        ERROR("");
                }
            //*
        //* cloned from var_tcp(): return udp listener:
            //*
    } 
//615wu    
#if 0    
    else {
                // Info about a particular connection
                bcopy ((char *) vp->name, (char *) newname,
                       (int) vp->namelen * sizeof (oid));

                // find "next" listener
                LowState = -1;      /* Don't have one yet */

                for(inpcb = Udps;inpcb != NULL; inpcb = inpcb->next){
                    cp = (uint8 *) &Lanface->addr;
                    newname[10] = cp[3];
                    newname[11] = cp[2];
                    newname[12] = cp[1];
                    newname[13] = cp[0];
                        newname[14] = inpcb->socket.port;

                        if (exact){
                                if (compare(newname, 15, name, *length) == 0){
                                        bcopy((char *)newname, (char *)lowest, 15 * sizeof(oid));
                                        LowState = 1;
                                        Lowinpcb = inpcb;
                                        break;  /* no need to search further */
                                }
                        } else {
                                if ((compare(newname, 15, name, *length) > 0) &&
                                     ((LowState < 0) || (compare(newname, 15, lowest, 15) < 0)))
                                {
                                        //*
                                        //* if new one is greater than input and closer to input
                                        //* than previous lowest, save this one as the "next" one.
                                        //*
                                        bcopy((char *)newname, (char *)lowest, 15 * sizeof(oid));
                                        LowState = 1;
                                        Lowinpcb = inpcb;
                                }
                        }
                } //for(inpcb = Udps ...)

                if (LowState < 0) return(NULL);

                bcopy ((char *) lowest, (char *) name,
                                ((int) vp->namelen + 5) * sizeof(oid));

                *length = vp->namelen + 5;
                *write_method = 0;
                *var_len = sizeof(long);

                switch (vp->magic) {
            case UDPLOCALADDRESS:
                long_return = DWordSwap(Lanface->addr);
                        return (uint8 *) &long_return;
            case UDPLOCALPORT:
                        long_return = Lowinpcb->socket.port;
                        return (uint8 *) &long_return;
                }
        } //if (vp->magic <= ...)
#endif
        return NULL;
}

//*
//* rfc 1907; the SNMP group.
//*

uint8 *
var_snmp(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[8] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0))) return NULL;

        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;

        *write_method = 0;
        *var_len = sizeof(long);        /* default length */

    // default value:
        long_return = 0;

        switch (vp->magic){
        case SNMPINPKTS:
                long_return = snmp_inpkts;
                break;
        case SNMPOUTPKTS:
                long_return = snmp_outpkts;
                break;
        case SNMPINBADVERSIONS:
                long_return = snmp_inbadversions;
                break;
        case SNMPINBADCOMMUNITYNAMES:
                long_return = snmp_inbadcommunitynames;
                break;
        case SNMPINBADCOMMUNITYUSES:
                break;
        case SNMPINASNPARSEERRORS:
                long_return = snmp_inasnparseerrors;
                break;
        case SNMPINTOOBIGS:
                long_return = snmp_intoobigs;
                break;
        case SNMPINNOSUCHNAMES:
                break;
        case SNMPINBADVALUES:
                long_return = snmp_inbadvalues;
                break;
        case SNMPINREADONLYS:
                long_return = snmp_inreadonlys;
                break;
        case SNMPINGENERRS:
                long_return = snmp_ingenerrs;
                break;
        case SNMPINTOTALREQVARS:
                long_return = snmp_intotalreqvars;
                break;
        case SNMPINTOTALSETVARS:
                break;
        case SNMPINGETREQUESTS:
                long_return = snmp_ingetrequests;
                break;
        case SNMPINGETNEXTS:
                long_return = snmp_ingetnexts;
                break;
        case SNMPINSETREQUESTS:
                long_return = snmp_insetrequests;
                break;
        case SNMPINGETRESPONSES:
                break;
        case SNMPINTRAPS:
                break;
        case SNMPOUTTOOBIGS:
                break;
        case SNMPOUTNOSUCHNAMES:
                long_return = snmp_outnosuchnames;
                break;
        case SNMPOUTBADVALUES:
                break;
        case SNMPOUTGENERRS:
                break;
        case SNMPOUTGETREQUESTS:
                long_return = snmp_outgetrequests;      //Simon 11/5/98
                break;
        case SNMPOUTGETNEXTS:
                long_return = snmp_outgetnexts;   //Simon 11/5/98
                break;
        case SNMPOUTSETREQUESTS:
                long_return = snmp_outsetrequests;  //Simon 11/5/98
                break;
        case SNMPOUTGETRESPONSES:
                long_return = snmp_outgetresponses;
                break;
        case SNMPOUTTRAPS:
                long_return = snmp_outtraps;             //Simon 11/5/98
                break;
        case SNMPENABLEAUTHENTRAPS:
            *write_method = EnableAuthenTraps;
                long_return = _SnmpAuthenTrap;
                break;
        default:
                ERROR("unknown snmp var");
            return NULL;
        }

        return (uint8 *) &long_return;
}

int
EnableAuthenTraps(
   int     action,
   uint8   *var_val,
   uint8   var_val_type,
   int     var_val_len,
   uint8   *statP,
   oid     *name,
   int     name_len
)
{
        long buf;
        int bigsize = 100;

        if(var_val_type != INTEGER){
#ifdef PC_OUTPUT
                printf("not Integer\n");
#endif
                return SNMP_ERR_WRONGTYPE;
        }

        if(var_val_len > sizeof(long)){
#ifdef PC_OUTPUT
                printf("bad length\n");
#endif
                return SNMP_ERR_WRONGLENGTH;
        }

        asn_parse_int(var_val, &bigsize, &var_val_type, &buf, sizeof(buf));

        if(buf != 1 && buf != 2) return SNMP_ERR_WRONGVALUE;

        if(action == COMMIT) {

        _SnmpAuthenTrap = buf;  //Set into EEPROM buffer

#ifndef _PC
		if(WriteToEEPROM(&EEPROM_Data) != 0) LightOnForever(Status_Lite); // Write EEPROM Data
#endif
        }

        return SNMP_ERR_NOERROR;
}


// Adding for enterprise's MIB data. Here print server will support
// HP jetadmin/web jetadmin
// 05/12/2000 ---- add by Arius
#ifdef WEBADMIN // modified ---- Arius

static int writeNpCardMib(int action, uint8 *var_val,
                          uint8 var_val_type, int var_val_len, uint8 *statP,
                          oid *name, int name_len);
static int writeDeviceMib(int action, uint8 *var_val,
                          uint8 var_val_type, int var_val_len, uint8 *statP,
                          oid *name, int name_len);

uint8 *
var_devstatus(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;
        char unknow[22]="MODEL:Unknown Printer";

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[13] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        memset(return_buf,0x00,SNMP_MAX_STRING);
        switch (vp->magic)
        {
           case GDSTATUSBYTES:
                    long_return = STATUSBYTES;
                    return (uint8 *)&long_return;
           case GDSTATUSDISPLAY: // Get Data from Front Panel
                    ReadDataFromFrontPanel(return_buf,JetPort-1);
                    *var_len = strlen(return_buf);
                    return (uint8 *)return_buf;
           case GDSTATUSJOBNAME:        // This Version will not support
           case GDSTATUSSOURCE:         // This Version will not support
           case GDSTATUSPAPSTATUS:      // This Version will not support
                    return NULL;
           case GDSTATUSID:    // Get device information
                    ReadPrintModelAndID(return_buf,JetPort-1);
                    if (return_buf[0] == NULL)
//                      strcpy(return_buf,"MODEL:Unknown Printer");
                      	memcpy(return_buf,&unknow,22);
                    *var_len = strlen(return_buf);;
                    return (uint8 *)return_buf;
           case GDSTATUSDISPLAYCODE:    // This Version will not support
           case GDSTATUSNLSCODE:        // This Version will not support
                    return NULL;
           //      The Unknown data
           case GDSTATUS10:       // Status Job Time Out
//                    long_return = Snmp_JobTimeOut;
                    long_return = 0;
                    *write_method = writeDeviceMib;
                    return (uint8 *)&long_return;
           case GDSTATUS11:       // Status PJL ustatus
                                  // 0: disable
                                  // 1: enable
                    long_return = Snmp_PJLStatus;
                    *write_method = writeDeviceMib;
                    return (uint8 *)&long_return;
           case GDSTATUS13:       // Device Password
                    memset(return_buf,0x00,8+5);   // 5 bytes will be fill =108;
                    *var_len = 8;
                    if (strlen(EEPROM_Data.Password) >= 8)
                    {
                      memcpy(return_buf,EEPROM_Data.Password,8);
                      memcpy(return_buf+8,"=108;",5);
                      *var_len = 8+5;
                    }
                    else
                    {
                      if (strlen(EEPROM_Data.Password) != 0)
                      {
                        strcpy(return_buf,EEPROM_Data.Password);
                        memcpy(return_buf+strlen(EEPROM_Data.Password),"=108;",5);
                        *var_len = 8+5;
                      }
                    }
                    *write_method = writeDeviceMib;
                    return (uint8 *)return_buf;
           case GDSTATUS18:       // Apple Talk Zone Name
                    strcpy(return_buf,EEPROM_Data.ATZoneName);
                    *var_len = strlen(return_buf);;
                    *write_method = writeDeviceMib;
                    return (uint8 *)return_buf;
           case GDSTATUS19:       // Apple Talk Printer Type
                    strcpy(return_buf,EEPROM_Data.ATPortType[JetPort-1]);
                    *var_len = strlen(return_buf);;
                    return (uint8 *)return_buf;
           default:
                        ERROR("");
        }
        return NULL;
}



uint8 *
var_gdstatusEntry(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[14] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case GDSTATUSLINESTATE:
                    if (snmp_ReadPrintStatus(JetPort-1) == PORT_READY)
                      long_return = 0;
                    else
                      if (snmp_ReadPrintStatus(JetPort-1) == PORT_OFF_LINE)
                        long_return = 2;
                    return (uint8 *)&long_return;
           case GDSTATUSPAPAERSTATE:
                    if (snmp_ReadPrintStatus(JetPort-1) == PORT_PAPER_OUT)
                      long_return = 1;
                    else
                      long_return = 0;
                    return (uint8 *)&long_return;
           case GDSTATUSINTERVENTIONSTATE:
                    if (snmp_ReadPrintStatus(JetPort-1) == PORT_READY)
                      long_return = 0;
                    else
                      if (snmp_ReadPrintStatus(JetPort-1) == PORT_OFF_LINE)
                        long_return = 2;
                    return (uint8 *)&long_return;
           case GDSTATUSNEWMODE:                   // This Version will not support
           case GDSTATUSCONNECTIONTERMINATIONACK:  // This Version will not support
                    return NULL;
           case GDSTATUSPERIPHERALERROR:
           case GDSTATUSPAPEROUT:
           case GDSTATUSPAPERJAM:
           case GDSTATUSTONERLOW:
           case GDSTATUSPAGEPUNT:
           case GDSTATUSMEMEORYOUT:
           case GDSTATUSIOACTIVE:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case GDSTATUSBUSY:
                    if (snmp_ReadPrintStatus(JetPort-1) == PORT_PRINTING)
                      long_return = 2;
                    else
                      long_return = 0;
                    return (uint8 *)&long_return;
           case GDSTATUSWAIT:
           case GDSTATUSINITIALIZE:
           case GDSTATUSDOOROPEN:
           case GDSTATUSPRINTING:
           case GDSTATUSPAPEROUTPUT:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case GDSTATUSRESERVED:                  // This Version will not support
                    return NULL;
           case GDSTATUSNOVBUSY:
           case GDSTATUSLLCBUSY:
           case GDSTATUSTCPBUSY:
           case GDSTATUSATBUSY:
                    long_return = 0;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npsys(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPSYSSTATE:                // This Version will not support
                    return NULL;
           case NPSYSSTATUSMESSAGE:
//                    snmp_ReadCardAndPeripheralStatus(JetPort-1,return_buf);
                    return_buf[0] = 0x00;
                    *var_len = strlen(return_buf);
                    return (uint8 *)return_buf;
           case NPSYSPERIPHERALFATALERROR: // This Version will not support
           case NPSYSCARDFATALERROR:       // This Version will not support
           case NPSYSMAXIMUMWRITEBUFFERS:  // This Version will not support
           case NPSYSMAXIMUMREADBUFFERS:   // This Version will not support
                    return NULL;
           case NPSYSTOTALBYTESRECVS:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPSYSTOTALBYTESENTS:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPSYSCURRREADREQ:          // This Version will not support
                    return NULL;
           default:
                        ERROR("");
        }
        return NULL;

}


uint8 *
var_npconns(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;
        uint8 *cp;         // testing

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPCONNSACCEPTS:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPCONNSREFUSED:         // This Version will not support
           case NPCONNSDENYS:           // This Version will not support
           case NPCONNSDENYSIP:         // This Version will not support
           case NPCONNSABORTS:          // This Version will not support
           case NPCONNSABORTREASON:     // This Version will not support
           case NPCONNSABORTIP:         // This Version will not support
           case NPCONNSABORTPORT:       // This Version will not support
           case NPCONNSABORTTIME:       // This Version will not support
           case NPCONNSSTATE:           // This Version will not support
                    return NULL;
           case NPCONNSIP:
                    return_buf[0] = 0x00;
                    *var_len = strlen(return_buf);
                    return (uint8 *)return_buf;
           case NPCONNSPORT:            // This Version will not support
           case NPCONNSPERIPCLOSE:      // This Version will not support
           case NPCONNSIDLETIMEOUTS:    // This Version will not support
           case NPCONNSNMCLOSE:         // This Version will not support
                    return NULL;
           case NPCONNSBYTESRECVD:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPCONNSBYTESSENTS:
                    long_return = 0;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npcfg(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;
        uint8 *cp;         // testing

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        switch(vp->magic)
        {
          case NPCFGACCESSLISTINDEX:
          case NPCFGACCESSLISTADDRESS:
          case NPCFGACCESSLISTADDRMASK:
                  newname[14] = 0;
                  break;
          default:
                  newname[12] = 0;
        }
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        memset(return_buf,0x00,SNMP_MAX_STRING);
        switch (vp->magic)
        {
           case NPCFGSOURCE:
                    // manual-three(3): read/write, must write ip_addr in same packet
                    // bootp-four(4): read/write, ignores other npCfg if in same packet
                    // dhcp(5): read/write, ignores other npCfg if in same packet
                    long_return = snmp_GetCfgSource();
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           case NPCFGYIADDR:
                    memcpy(return_buf,EEPROM_Data.BoxIPAddress,4);
                    *var_len = 4;
                    *write_method = writeNpCardMib;
                    return (uint8 *)return_buf;
           case NPCFGSIADDR:         // This Version will not support
           case NPCFGGIADDR:         // This Version will not support
           case NPCFGLOGSERVER:      // This Version will not support
           case NPCFGSYSLOGFACILITY: // This Version will not support
                    return NULL;
           case NPCFGACCESSSTATE:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPCFGACCESSLISTNUM:       // This Version will not support
           case NPCFGACCESSLISTINDEX:     // This Version will not support
           case NPCFGACCESSLISTADDRESS:   // This Version will not support
           case NPCFGACCESSLISTADDRMASK:  // This Version will not support
                    return NULL;
           case NPCFGIDLETIMEOUT:
//                    long_return = Snmp_IdelTimeOut;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           case NPCFGLOCALSUBNETS:        // This Version will not support
                    return NULL;
    //      The Unknown data
           case NPCFG12:    // subnet mask address
                    memcpy(return_buf,EEPROM_Data.SubNetMask,4);
                    *var_len = 4;
                    *write_method = writeNpCardMib;
                    return (uint8 *)return_buf;
           case NPCFG13:    // gate address
                    memcpy(return_buf,EEPROM_Data.GetwayAddress,4);
                    *var_len = 4;
                    *write_method = writeNpCardMib;
                    return (uint8 *)return_buf;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_nptcp(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPTCPINSEGINORDER:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPTCPINSEGOUTOFORDER:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPTCPINSEGZEROPROBE:
                    long_return = 0;
                    return (uint8 *)&long_return;
           case NPTCPINDISCARDS:
                    long_return = 0;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;
}

uint8 *
var_npctl(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;
        uint8 *cp;         // testing

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPCTLRECONFIGIP:        // This Version will not support
           case NPCTLRECONFIGPORT:      // This Version will not support
           case NPCTLRECONFIGTIME:      // This Version will not support
           case NPCTLCLOSEIP:           // This Version will not support
           case NPCTLCLOSEPORT:         // This Version will not support
           case NPCTLIMAGEDUMP:         // This Version will not support
           case NPCTLCLOSECONNECTION:   // This Version will not support
           case NPCTLRECONFIG:          // This Version will not support
                    return NULL;
           case NPCTLPROTOCOLSET:
                    // bits 31-28: Number of I/O channels/protocols present
                    // bits 27-24: Number of I/O channels which may operate currently
                    // bits 23-16: unused, reserved
                    // bits 15- 1: Individual I/O channel/protocl enable bits
                    //    Novel bit 1
                    //    LLC   bit 2
                    //    TCP   bit 3
                    //    ATALK bit 4
                    Snmp_ProtocolSet = snmp_SetProtocol();
                    long_return =  Snmp_ProtocolSet;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
    //      The Unknown data
           case NPCTL10:       // Status Page Language
//                    long_return = Snmp_StatusPageLanguage;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           case NPCTL11:       // Control Print Status Page
//                    long_return = Snmp_PrintStatusPage;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           case NPCTL12:       // Error Behavior
//                    long_return = Snmp_ErrorBehavior;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npnpi(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        if (vp->magic == NPNPIPERIPHERALATTRIBUTECOUNT)
        {
          // This Version will not support
          bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
          newname[12] = 0;
          result = compare(name, *length, newname, (int)vp->namelen + 1);
          if ((exact && (result != 0)) || (!exact && (result >= 0)))
             return NULL;
          bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
          *length = vp->namelen + 1;
          *write_method = 0;
          *var_len = sizeof(long);        // default length

          return NULL;
        }
        else
        {
          bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
          newname[13] = 0;
          result = compare(name, *length, newname, (int)vp->namelen + 1);
          if ((exact && (result != 0)) || (!exact && (result >= 0)))
             return NULL;
          bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
          *length = vp->namelen + 1;
          *write_method = 0;
          *var_len = sizeof(long);        // default length

          switch (vp->magic)
          {
             //  --- The Card Status Entry
             case NPNPICSEDATASTATE:         // This Version will not support
             case NPNPICSEERRORCODE:         // This Version will not support
             case NPNPICSELINKEVENT:         // This Version will not support
             case NPNPICSEREADMODE:          // This Version will not support
             case NPNPICSEWRITEMODE:         // This Version will not support
             case NPNPICSEWARNINGCODE:       // This Version will not support
             case NPNPICSECONNECTIONSTATE:   // This Version will not support
             case NPNPICSENOVWARNINGCODE:    // This Version will not support
             case NPNPICSELLCWARNINGCODE:    // This Version will not support
             case NPNPICSETCPWARNINGCODE:    // This Version will not support
             case NPNPICSEATKWARNINGCODE:    // This Version will not support
                    return NULL;
             //   --- The Peripheral Attribute Entry
             case NPNPIPAELINKDIRECTION:     // This Version will not support
                    return NULL;
             case NPNPIPAECLASS:
                    long_return = 0;
                    return (uint8 *)&long_return;
             case NPNPIPAEIDENTIFICATION:
                    long_return = 0;
                    return (uint8 *)&long_return;
             case NPNPIPAEREVISION:          // This Version will not support
             case NPNPIPAEAPPLETALK:         // This Version will not support
             case NPNPIPAEMESSAGE:           // This Version will not support
             case NPNPIPAERESERVED:          // This Version will not support
                    return NULL;
             case NPNPIPAEMULTICHAN:    // An indication of how many MIO channels
                                        // the peripheral supports
                    long_return = 1;
                    return (uint8 *)&long_return;
             case NPNPIPAEPAD:               // This Version will not support
                    return NULL;
             //   --- The Card Attribute Entry
             case NPNPICAELINKDIRECTION:     // This Version will not support
                    return NULL;
             case NPNPICAECLASS:        // The class of this network peripheral card
                    long_return = 3;    // CSMA_CD_NETWORK
                    return (uint8 *)&long_return;
             case NPNPICAEIDENTIFCATION:     // This Version will not support
             case NPNPICAEREVISION:          // This Version will not support
             case NPNPICAEAPPLETALK:         // This Version will not support
             case NPNPICAEMESSAGE:           // This Version will not support
             case NPNPICAEREVERSED:          // This Version will not support
             case NPNPICAEMULTICHAN:         // This Version will not support
                    return NULL;
             default:
                          ERROR("");
          }
        }
        return NULL;
}


uint8 *
var_npipx(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPIPXGETUNITCFGRESP:
                    memset(return_buf,0x00,78);
                    sprintf(return_buf,"%c%c%c%c",0x01,0x00,0x13,0x04);

                    // read Port status
                    switch(snmp_ReadPrintStatus(JetPort-1))
                    {
                      case PORT_READY:        // Wating for job and On Line
                              return_buf[55]=0x13;
                              return_buf[77]=0x01;
                              break;
                      case PORT_PAPER_OUT:    // Paper out
                              return_buf[63]=0x01;
                              return_buf[69]=0x01;
                              break;
                      case PORT_OFF_LINE:    // OFF Line
                              return_buf[71]=0xFF;
                              break;
                      case PORT_PRINTING:    // Printing
                              return_buf[69]=0x01;
                              break;
                         // Manual Feeded Needed
//                         return_buf[63]=0x02;
//                         return_buf[69]=0x01;
                    }
                    *var_len = 78;
                    return (uint8 *)return_buf;
           case NPIPX8022FRAMETYPE:
                    long_return = IPX8022Recv;
                    return (uint8 *)&long_return;
           case NPIPXSNAPFRAMETYPE:
                    long_return = IPXSNAPRecv;
                    return (uint8 *)&long_return;
           case NPIPXETHERNETFRAMETYPE:
                    long_return = IPXENIIRecv;
                    return (uint8 *)&long_return;
           case NPIPX8023RAWFRAMETYPE:
                    long_return = IPX8023Recv;
                    return (uint8 *)&long_return;
    //      The Unknown data
           case NPIPX6:  // IPXSapInfo
                         //  2 bytes: bindery object type(always 030C in hi-lo order)
                         // 12 bytes: Mac address of card (ASCII)
                         //  2 bytes: frame type
                         //           also high bit (8000) is set if card is not cofigured
                         //  2 bytes: unit type (hex 81 for NetJet card)
                         // 32 bytes: node name which is:
                         //    Print Server name for Queue Server mode
                         //    Printer name for RPTR mode
                    memset(return_buf,0x00,50);     // clear
                    // First two bytes are 0x03 and 0x0c
                    return_buf[0]=0x03; return_buf[1]=0x0C;

                    // Physical Address was placed at offset 2
                    sprintf(return_buf+2,"%02X%02X%02X%02X%02X%02X",
                            MyPhysNodeAddress[0], MyPhysNodeAddress[1],
                            MyPhysNodeAddress[2], MyPhysNodeAddress[3],
                            MyPhysNodeAddress[4], MyPhysNodeAddress[5]);

                    // Unknow field (00C6)was placed at offset 14
             #if (NUM_OF_PRN_PORT > 1)
                       // port 1: 10C6, port 2: 20C6 port 3: 30C6
                       sprintf(return_buf+14,"%d%c%s",JetPort,'0',JetModel);
             #else
                       sprintf(return_buf+14,"%c%c%s",'0','0',JetModel);
             #endif
                    // PrintServerName was placed at offset 18
                    // put into hostname
//                    if (strlen(Hostname) < LENGTH_OF_BOX_NAME)
//                      memcpy(return_buf+18,Hostname,strlen(Hostname));
//                    else
//                      memcpy(return_buf+18,Hostname,LENGTH_OF_BOX_NAME);
                    strcpy(return_buf+18,EEPROM_Data.PrintServerName);
                    *var_len = 50;
                    return (uint8 *)return_buf;
           case NPIPX7:    // Get Unit Config Response 2
                    memset(return_buf,0x00,78);      // clear
                    // First 14 bytes are always fixed
                    sprintf(return_buf,"%c%c%c%c",0x13,0x00,0x01,0x01);
                    // Unknow field (00C6)was placed at offset 14
      #if (NUM_OF_PRN_PORT > 1)
                      sprintf(return_buf+14,"%d%c%s",JetPort,'0',JetModel);
      #else
                      sprintf(return_buf+14,"%c%c%s",'0','0',JetModel);
      #endif
                    // PrintServerName was placed at offset 18
//                    if (strlen(Hostname) < LENGTH_OF_BOX_NAME)
//                      memcpy(return_buf+18,Hostname,strlen(Hostname));
//                    else
//                      memcpy(return_buf+18,Hostname,LENGTH_OF_BOX_NAME);
                      strcpy(return_buf+18,EEPROM_Data.PrintServerName);

                    return_buf[55] = 0x13; return_buf[77] = 0x01;

                    *var_len = 78;
                    return (uint8 *)return_buf;
           case NPIPX8:  // Print Server Name
                    strcpy(return_buf,EEPROM_Data.PrintServerName);
//                    strcpy(return_buf,Hostname);
                    *var_len = strlen(return_buf);
                    *write_method = writeNpCardMib;
                    return (uint8 *)return_buf;
           case NPIPX9:  // NDS Tree Name
                    strcpy(return_buf,EEPROM_Data.NDSTreeName);
                    *var_len = strlen(return_buf);
                    *write_method = writeNpCardMib;
                    return (uint8 *)return_buf;
//           case NPIPX13:  // 12 octet IPX address of the novell RCFG socket
//sprintf(return_buf,"%c%c%c%c",0x11,0x11,0x80,0x22);
//sprintf(return_buf+4,"%02X%02X%02X%02X%02X%02X",
//        MyPhysNodeAddress[0], MyPhysNodeAddress[1],
//        MyPhysNodeAddress[2], MyPhysNodeAddress[3],
//        MyPhysNodeAddress[4], MyPhysNodeAddress[5]);
//sprintf(return_buf+10,"%c%c",0x40,0x0C);
//*var_len = 12;
//                    return (uint8 *)return_buf;
           case NPIPX16:        // 0: soft reset of the broad
                                // When 0, you press "Restart NetWare Server Connections"
                                // 1: hard reset of the broad
                                // When 1, you press "Complete Reinitilliazation"
                    long_return = UsingSoftwareReset;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;

           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npipxEntry(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;
        uint8 index,NameLen;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[13] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
    //      The Unknown data
           case NPIPX10Entry_1:  // NDS Fully Qualified Name
                   if ( strlen(EEPROM_Data.NDSContext) )
                   {
                     // 4 bytes header
                     sprintf(return_buf,"%c%c%c%c",0x20,0x00,0x00,0x00);
                     // Put "CN="
                     sprintf(return_buf+4,"%c%c%c%c%c%c",'C',0x00,'N',0x00,'=',0x00);
                     // IPX Name
//                     if (strlen(Hostname) < LENGTH_OF_BOX_NAME)
//                       NameLen = strlen(Hostname);
//                     else
//                       NameLen = LENGTH_OF_BOX_NAME;
                     NameLen = strlen(EEPROM_Data.PrintServerName);

                     for (index = 0; index < NameLen; index++)
                        sprintf(return_buf+10+(index*2),"%c%c",EEPROM_Data.PrintServerName[index],0x00);
                     // Put Context
                     sprintf(return_buf+10+2*NameLen,"%c%c",'.',0x00);
                     for (index = 0; index < strlen(EEPROM_Data.NDSContext); index++)
                        sprintf(return_buf+10+2*NameLen+2+(index*2),"%c%c",
                                EEPROM_Data.NDSContext[index],0x00);
                     // Put end of header
                     sprintf(return_buf+10+2*NameLen+2+2*(strlen(EEPROM_Data.NDSContext)),
                             "%c%c",0x00,0x00);
                     *var_len = 10+2*NameLen+2+2*(strlen(EEPROM_Data.NDSContext))+2;
                    }
                    else
                    {
                     return_buf[0] = 0x00;
                     *var_len = strlen(return_buf);
                    }
                    *write_method = writeNpCardMib;
                    return (uint8 *)return_buf;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npcard11(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPCARD11_1:   // number of current active Direct Mode connections
                    long_return = 1;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npcard12(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPCARD12_7:     // LLC connection state
                    long_return = 0;    // 2 means waiting for connection
                                        // 1 means disconnection
                    return (uint8 *)&long_return;
           case NPCARD12_8:     // LLC Server Address
                    memset(return_buf,0x00,12);
                    *var_len = 12;
                    return (uint8 *)return_buf;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npcard13(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPCARD13_1:   // How many pors will the print server be supported?
                    long_return = NUM_OF_PRN_PORT;
                    return (uint8 *)&long_return;
           case NPCARD13_3:      // Parallel Mode
//                    long_return = Snmp_PortDesiredMode;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           case NPCARD13_4:      // Parallel Handshaking
//                    long_return = Snmp_PortHandshanking;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;

}

uint8 *
var_npcard17(
        struct variable *vp, // IN - pointer to variable entry that points here
        oid    *name,        // IN/OUT - input name requested, output name found
        int    *length,      // IN/OUT - length of input and output oid's
        int    exact,        // IN - TRUE if an exact match was requested.
        int    *var_len,         // OUT - length of variable or 0 if function returned.
        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
)
{
        int  result;

        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
        newname[12] = 0;
        result = compare(name, *length, newname, (int)vp->namelen + 1);
        if ((exact && (result != 0)) || (!exact && (result >= 0)))
           return NULL;
        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
        *length = vp->namelen + 1;
        *write_method = 0;
        *var_len = sizeof(long);        // default length
        switch (vp->magic)
        {
           case NPCARD17_1:      // Scan TimeOut  (unit:seconds)
//                    long_return = Snmp_ScanTimeOut;
                    long_return = 0;
                    *write_method = writeNpCardMib;
                    return (uint8 *)&long_return;
           default:
                        ERROR("");
        }
        return NULL;

}


//uint8 *
//var_npcard18(
//        struct variable *vp, // IN - pointer to variable entry that points here
//        oid    *name,        // IN/OUT - input name requested, output name found
//        int    *length,      // IN/OUT - length of input and output oid's
//        int    exact,        // IN - TRUE if an exact match was requested.
//        int    *var_len,         // OUT - length of variable or 0 if function returned.
//        int    (**write_method)()  // OUT - pointer to function to set variable, otherwise 0
//)
//{
//        int  result;
//
//        bcopy((char *)vp->name, (char *)newname, (int)vp->namelen * sizeof(oid));
//        newname[12] = 0;
//        result = compare(name, *length, newname, (int)vp->namelen + 1);
//        if ((exact && (result != 0)) || (!exact && (result >= 0)))
//           return NULL;
//        bcopy((char *)newname, (char *)name, ((int)vp->namelen + 1) * sizeof(oid));
//        *length = vp->namelen + 1;
//        *write_method = 0;
//        *var_len = sizeof(long);        // default length
//        switch (vp->magic)
//        {
//           case NPCARD18_1:
//                    long_return = 1;
//                    return (uint8 *)&long_return;
//           default:
//                        ERROR("");
//        }
//        return NULL;
//
//}


int writeDeviceMib(int action, uint8 *var_val, uint8 var_val_type,
                   int var_val_len, uint8 *statP, oid *name,
                   int name_len)
{
   uint8 buf[SNMP_MAX_STRING],*cp;
   int  size,errcode,NameLen;
   long data;
   int bigsize = 1000;

   errcode = snmp_CheckValueType(var_val_type,var_val_len);
   if ( errcode != SNMP_ERR_NOERROR )
     return errcode;

   switch(var_val_type)
   {
     case INTEGER:
                  size = sizeof(data);
                  asn_parse_int(var_val, &bigsize, &var_val_type, &data, size);
                  break;
     case STRING:
                  size = sizeof(buf);
                  asn_parse_string(var_val, &bigsize, &var_val_type, buf, &size);
                  buf[size] = 0;
                  break;
   }

   switch((char)name[12])
   {
     case 10:   // Status Job Time Out
     case 11:   // Status PJL Ustatus
            if (sizeof(data) > sizeof(long))
              return SNMP_ERR_WRONGLENGTH;
            break;
     case 13:   // Password
            if (size > SNMP_MAX_STRING)
              return SNMP_ERR_WRONGLENGTH;
            size = 8;
            break;
     case 18:   // Status At Printer Name
            if (size > ATALK_ZONE_LEN)
              return SNMP_ERR_WRONGLENGTH;
            break;
     default:
            return SNMP_ERR_NOSUCHNAME;
   }

   if (action == COMMIT)
   {
     switch((char)name[12])
     {
       case 10:   // Status Job Time Out
               Snmp_JobTimeOut = (int)data;
               break;
       case 11:   // Status PJL Ustatus
               Snmp_PJLStatus = (uint8)data;
               break;
       case 13:   // Password
               NameLen = strlen(buf);
               if (NameLen)
               {
                 // 5 bytes "=108;"
                 if ((NameLen-5) >= 8)
                   memcpy(EEPROM_Data.Password,buf,8);
                 else
                 {
                   memcpy(EEPROM_Data.Password,buf,NameLen-5);
                   EEPROM_Data.Password[NameLen-5] = 0x00;
                 }
               }
               else
                EEPROM_Data.Password[0] = 0x00;
               Snmp_WriteDataTo_EEPROM = 1;

               break;
       case 18:   // Status At Printer Name
               strcpy(EEPROM_Data.ATZoneName,buf);
               Snmp_WriteDataTo_EEPROM = 1;
               break;
       default:
               return SNMP_ERR_NOSUCHNAME;
     }

//     Snmp_WriteDataTo_EEPROM = 1;
   }
   return SNMP_ERR_NOERROR;
}


int writeNpCardMib(int action, uint8 *var_val, uint8 var_val_type,
                    int var_val_len, uint8 *statP, oid *name,
                    int name_len)
{
   uint8 buf[SNMP_MAX_STRING],*cp;
   int  size,errcode,index,i,NameLen;
   long data;
   int bigsize = 1000;

   errcode = snmp_CheckValueType(var_val_type,var_val_len);
   if ( errcode != SNMP_ERR_NOERROR )
     return errcode;

   switch(var_val_type)
   {
     case INTEGER:
                  size = sizeof(data);
                  asn_parse_int(var_val, &bigsize, &var_val_type, &data, size);
                  break;
     case STRING:
     case IPADDRESS:
                  size = sizeof(buf);
                  asn_parse_string(var_val, &bigsize, &var_val_type, buf, &size);
                  buf[size] = 0;
                  break;
   }

   // which groups in npCard
   switch((char)name[10])
   {
       case  1:     // system subgroup
       case  4:     // Connection Statistics subgroup
              break;
       case  5:     // Configuration subgroup
              switch((char)name[11])
              {
                case  1:   // CfgSource
                case 10:   // Idel Time Out
                       if (sizeof(data) > sizeof(long))
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                case 2:   // CfgYiaddr
                       if (size > 4)
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                case 12:   // CfgSubnetMask
                       if (size > 4)
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                case 13:   // CfgDefaultGateway
                       if (size > 4)
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                default:
                       return SNMP_ERR_NOSUCHNAME;
              }
              break;
       case  6:     // TCP subgroup
              break;
       case  7:     // Card Control subgroup
              switch((char)name[11])
              {
                case  9:   // Protocol Set
                case 10:   // Status Page Language
                case 11:   // Print Status Page
                case 12:   // Error Behavior
                       if (sizeof(data) > sizeof(long))
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                default:
                       return SNMP_ERR_NOSUCHNAME;
              }
              break;
       case  8:     // HP Modular Input/Output subgroup
              break;
       case 10:     // IPX subgroup
              switch((char)name[11])
              {
                case 8:   // Unit Name
                       if (size >= SNMP_PRINTSERVER_NAME_LEN)
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                case 9:   // NDS Tree Name
                       if (size >= NDS_TREE_LEN)
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                case 10:   // NDS Fully Qualified Name
                       switch((char)name[12])
                       {
                         case 1:  // Name1
                                if (size >= SNMP_MAX_STRING)
                                  return SNMP_ERR_WRONGLENGTH;
                                break;
                       }
                       break;
                case 16:   // Restart
                       if (sizeof(data) > sizeof(long))
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                default:
                       return SNMP_ERR_NOSUCHNAME;
              }
              break;
       case 11:     // Direct Mode
       case 12:     // LLC
              break;
       case 13:     // Port
              switch((char)name[11])
              {
                case 3:  // Port Desired Mode
                case 4:  // Port Centronics Handshanking
                       if (sizeof(data) > sizeof(long))
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                default:
                       return SNMP_ERR_NOSUCHNAME;
              }
              break;
       case 17:     // npScan subgroup
              switch((char)name[11])
              {
                case 1:  // Scan Idel time out
                       if (sizeof(data) > sizeof(long))
                         return SNMP_ERR_WRONGLENGTH;
                       break;
                default:
                       return SNMP_ERR_NOSUCHNAME;
              }
              break;
   }

   if (action == COMMIT)
   {
     switch((char)name[10])
     {
         case  1:     // system subgroup
         case  4:     // Connection Statistics subgroup
                break;
         case  5:     // Configuration subgroup
                switch((char)name[11])
                {
                  case  1:   // CfgSource
                         snmp_SetCfgSource((int)data);
                         Snmp_WriteDataTo_EEPROM = 1;
                         break;
                  case 10:   // Idel Time Out
                         Snmp_IdelTimeOut = (int)data;
                         break;
                  case  2:   // CfgYiaddr
                         memcpy(EEPROM_Data.BoxIPAddress,buf,4);
                         Snmp_Change_IP = 1;
                         Snmp_WriteDataTo_EEPROM = 1;
                         break;
                  case 12:   // CfgSubnetMask
                         memcpy(EEPROM_Data.SubNetMask,buf,4);
                         Snmp_WriteDataTo_EEPROM = 1;
                         break;
                  case 13:   // CfgDefaultGateway
                         memcpy(EEPROM_Data.GetwayAddress,buf,4);
                         Snmp_WriteDataTo_EEPROM = 1;
                         break;
                  default:
                         return SNMP_ERR_NOSUCHNAME;
                }
                break;
         case  6:     // TCP subgroup
                break;
         case  7:     // Card Control subgroup
                switch((char)name[11])
                {
                  case  9:   // Protocol Set
                         Snmp_ProtocolSet = data;
                         break;
                  case 10:   // Status Page Language
                         Snmp_StatusPageLanguage = (int)data;
                         break;
                  case 11:   // Print Status Page
                         Snmp_PrintStatusPage = (int)data;
                         break;
                  case 12:   // Error Behavior
                         Snmp_ErrorBehavior = (int)data;
                         break;
                  default:
                         return SNMP_ERR_NOSUCHNAME;
                }
                break;
         case  8:     // HP Modular Input/Output subgroup
                break;
         case 10:     // IPX subgroup
                switch((char)name[11])
                {
                  case  8:   // Unit Name(Print Server Name)
                         strcpy(EEPROM_Data.PrintServerName,buf);
                         Snmp_WriteDataTo_EEPROM = 1;
                         break;
                  case  9:   // NDS Tree Name
                         strcpy(EEPROM_Data.NDSTreeName,buf);
                         Snmp_WriteDataTo_EEPROM = 1;
                         break;
                  case 10:   // NDS Fully Qualified Name
                         switch((char)name[12])
                         {
                           case 1:  // Name1
                                  // shift four bytes
                                  // shift (C 0x00 N 0x00 = 0x00) six bytes
                                  // shift Bytes = (IPX Name * 2)
                                  // shift two bytes(. 0x00)
                                  NameLen = strlen(EEPROM_Data.PrintServerName);
                                  for (index = (10+(2*NameLen)+2),
                                       i = 0; index < NameLen; index+=2,i++)
                                  {
                                    if (buf[index] == 0x00)
                                      break;
                                    EEPROM_Data.NDSContext[i] = buf[index];
                                  }
                                  EEPROM_Data.NDSContext[i] = 0;
                                  Snmp_WriteDataTo_EEPROM = 1;
                                  break;
                         }
                         break;
                  case 16:   // Restart
                             // 0: soft reset of the broad
                             // When 0, you press "Restart NetWare Server Connections"
                             // 1: hard reset of the broad
                             // When 1, you press "Complete Reinitilliazation"
                         UsingSoftwareReset = (int)data;
                         Snmp_Restart = 1;
                         break;
                  default:
                         return SNMP_ERR_NOSUCHNAME;
                }
                break;
         case 11:     // Direct Mode
         case 12:     // LLC
                break;
         case 13:     // Port
                switch((char)name[11])
                {
                  case 3:  // Port Desired Mode
                         Snmp_PortDesiredMode = (int)data;
                         break;
                  case 4:  // Port Centronics Handshanking
                         Snmp_PortHandshanking = (int)data;
                         break;
                  default:
                         return SNMP_ERR_NOSUCHNAME;
                }
                break;
         case 17:     // npScan subgroup
                switch((char)name[11])
                {
                  case 1:  // Scan Idel time out
                         Snmp_ScanTimeOut = (int)data;
                         break;
                  default:
                         return SNMP_ERR_NOSUCHNAME;
                }
                break;
     }

//     Snmp_WriteDataTo_EEPROM = 1;
   }
   return SNMP_ERR_NOERROR;
}

#endif WEBADMIN // modified ---- Arius
// ------ End of here  >>>> By Arius
