#ifndef _SNMPVARS_H
#define _SNMPVARS_H

#define SNMP_SYS_MAX_RW_LEN  30  //doesn't include zero

#define SNMP_MAX_STRING   512

#define MIB_BASE  1, 3, 6, 1, 2, 1
// 05/12/2000 ---- add by Arius
#ifdef WEBADMIN
#define ENTERPRISE 1, 3, 6, 1, 4, 1
#define HP         11
#define NM         11, 2
#define SYSTEM     11, 2, 3
#define INTERFACE  11, 2, 4
#define GENERALDEVICESTATUS 11, 2, 3, 9, 1, 1
#define NPCARD     11, 2, 4, 3
#endif WEBADMIN
// ------ End of here  >>>> By Arius

/*
 * These are unit magic numbers for each variable.
 */

#define VERSION_DESCR   0
#define VERSION_ID          1
#define IFNUMBER        2
#define UPTIME          3
#define SYSCONTACT          4
#define SYSYSNAME       5
#define SYSLOCATION         6
#define SYSSERVICES         7
#define SYSORLASTCHANGE 8

#define IFINDEX         1
#define IFDESCR         2
#define IFTYPE          3
#define IFMTU           4
#define IFSPEED         5
#define IFPHYSADDRESS   6
#define IFADMINSTATUS   7
#define IFOPERSTATUS    8
#define IFLASTCHANGE    9
#define IFINOCTETS      10
#define IFINUCASTPKTS   11
#define IFINNUCASTPKTS  12
#define IFINDISCARDS    13
#define IFINERRORS      14
#define IFINUNKNOWNPROTOS 15
#define IFOUTOCTETS     16
#define IFOUTUCASTPKTS  17
#define IFOUTNUCASTPKTS 18
#define IFOUTDISCARDS   19
#define IFOUTERRORS     20
#define IFOUTQLEN       21
#define IFSPECIFIC      22

#define ATIFINDEX       0
#define ATPHYSADDRESS   1
#define ATNETADDRESS    2

#define IPFORWARDING    0
#define IPDEFAULTTTL    1
#define IPINRECEIVES    2
#define IPINHDRERRORS   3
#define IPINADDRERRORS  4
#define IPFORWDATAGRAMS 5
#define IPINUNKNOWNPROTOS 6
#define IPINDISCARDS    7
#define IPINDELIVERS    8
#define IPOUTREQUESTS   9
#define IPOUTDISCARDS   10
#define IPOUTNOROUTES   11
#define IPREASMTIMEOUT  12
#define IPREASMREQDS    13
#define IPREASMOKS      14
#define IPREASMFAILS    15
#define IPFRAGOKS       16
#define IPFRAGFAILS     17
#define IPFRAGCREATES   18

#define IPADADDR        1
#define IPADIFINDEX     2
#define IPADNETMASK     3
#define IPADBCASTADDR   4
#define IPADENTREASMMAXSIZE     5

#define IPROUTEDEST     0
#define IPROUTEIFINDEX  1
#define IPROUTEMETRIC1  2
#define IPROUTEMETRIC2  3
#define IPROUTEMETRIC3  4
#define IPROUTEMETRIC4  5
#define IPROUTENEXTHOP  6
#define IPROUTETYPE     7
#define IPROUTEPROTO    8
#define IPROUTEAGE      9

#define IPNETTOMEDIAIFINDEX     1
#define IPNETTOMEDIAPHYSADDR    2
#define IPNETTOMEDIANETADDR     3
#define IPNETTOMEDIATYPE        4

#define ICMPINMSGS           0
#define ICMPINERRORS         1
#define ICMPINDESTUNREACHS   2
#define ICMPINTIMEEXCDS      3
#define ICMPINPARMPROBS      4
#define ICMPINSRCQUENCHS     5
#define ICMPINREDIRECTS      6
#define ICMPINECHOS          7
#define ICMPINECHOREPS       8
#define ICMPINTIMESTAMPS     9
#define ICMPINTIMESTAMPREPS 10
#define ICMPINADDRMASKS     11
#define ICMPINADDRMASKREPS  12
#define ICMPOUTMSGS         13
#define ICMPOUTERRORS       14
#define ICMPOUTDESTUNREACHS 15
#define ICMPOUTTIMEEXCDS    16
#define ICMPOUTPARMPROBS    17
#define ICMPOUTSRCQUENCHS   18
#define ICMPOUTREDIRECTS    19
#define ICMPOUTECHOS        20
#define ICMPOUTECHOREPS     21
#define ICMPOUTTIMESTAMPS   22
#define ICMPOUTTIMESTAMPREPS 23
#define ICMPOUTADDRMASKS    24
#define ICMPOUTADDRMASKREPS 25

#define TCPRTOALGORITHM      1
#define TCPRTOMIN            2
#define TCPRTOMAX            3
#define TCPMAXCONN           4
#define TCPACTIVEOPENS       5
#define TCPPASSIVEOPENS      6
#define TCPATTEMPTFAILS      7
#define TCPESTABRESETS       8
#define TCPCURRESTAB         9
#define TCPINSEGS           10
#define TCPOUTSEGS          11
#define TCPRETRANSSEGS      12
#define TCPCONNSTATE        13
#define TCPCONNLOCALADDRESS 14
#define TCPCONNLOCALPORT    15
#define TCPCONNREMADDRESS   16
#define TCPCONNREMPORT      17

#define TCPINERRS       18
#define TCPOUTRSTS          19


#define UDPINDATAGRAMS      0
#define UDPNOPORTS          1
#define UDPINERRORS         2
#define UDPOUTDATAGRAMS     3
#define UDPLOCALADDRESS     4
#define UDPLOCALPORT        5

#define SNMPINPKTS              1
#define SNMPOUTPKTS             2
#define SNMPINBADVERSIONS       3
#define SNMPINBADCOMMUNITYNAMES 4
#define SNMPINBADCOMMUNITYUSES  5
#define SNMPINASNPARSEERRORS    6
#define SNMPINTOOBIGS           8
#define SNMPINNOSUCHNAMES       9
#define SNMPINBADVALUES         10
#define SNMPINREADONLYS         11
#define SNMPINGENERRS           12
#define SNMPINTOTALREQVARS      13
#define SNMPINTOTALSETVARS      14
#define SNMPINGETREQUESTS       15
#define SNMPINGETNEXTS          16
#define SNMPINSETREQUESTS       17
#define SNMPINGETRESPONSES      18
#define SNMPINTRAPS             19
#define SNMPOUTTOOBIGS          20
#define SNMPOUTNOSUCHNAMES      21
#define SNMPOUTBADVALUES        22
#define SNMPOUTGENERRS          24
#define SNMPOUTGETREQUESTS      25
#define SNMPOUTGETNEXTS         26
#define SNMPOUTSETREQUESTS      27
#define SNMPOUTGETRESPONSES     28
#define SNMPOUTTRAPS            29
#define SNMPENABLEAUTHENTRAPS   30


// Adding for enterprise's MIB data. Here print server will support
// HP jetadmin/web jetadmin
// 05/15/2000 ---- add by Arius
#ifdef WEBADMIN
#define GDSTATUSBYTES           1
#define GDSTATUSDISPLAY         3
#define GDSTATUSJOBNAME         4
#define GDSTATUSSOURCE          5
#define GDSTATUSPAPSTATUS       6
#define GDSTATUSID              7
#define GDSTATUSDISPLAYCODE     8
#define GDSTATUSNLSCODE         9
    //      The Unknown data
#define GDSTATUS10              10
#define GDSTATUS11              11
#define GDSTATUS13              13
#define GDSTATUS18              18
#define GDSTATUS19              19

#define GDSTATUSLINESTATE                  1
#define GDSTATUSPAPAERSTATE                2
#define GDSTATUSINTERVENTIONSTATE          3
#define GDSTATUSNEWMODE                    4
#define GDSTATUSCONNECTIONTERMINATIONACK   5
#define GDSTATUSPERIPHERALERROR            6
#define GDSTATUSPAPEROUT                   8
#define GDSTATUSPAPERJAM                   9
#define GDSTATUSTONERLOW                   10
#define GDSTATUSPAGEPUNT                   11
#define GDSTATUSMEMEORYOUT                 12
#define GDSTATUSIOACTIVE                   13
#define GDSTATUSBUSY                       14
#define GDSTATUSWAIT                       15
#define GDSTATUSINITIALIZE                 16
#define GDSTATUSDOOROPEN                   17
#define GDSTATUSPRINTING                   18
#define GDSTATUSPAPEROUTPUT                19
#define GDSTATUSRESERVED                   20
#define GDSTATUSNOVBUSY                    21
#define GDSTATUSLLCBUSY                    22
#define GDSTATUSTCPBUSY                    23
#define GDSTATUSATBUSY                     24
    //  --- The HP Network Peripheral Card(npCard) Group
    //      The System subgroup of npCard
#define NPSYSSTATE                         1
#define NPSYSSTATUSMESSAGE                 2
#define NPSYSPERIPHERALFATALERROR          3
#define NPSYSCARDFATALERROR                4
#define NPSYSMAXIMUMWRITEBUFFERS           5
#define NPSYSMAXIMUMREADBUFFERS            6
#define NPSYSTOTALBYTESRECVS               7
#define NPSYSTOTALBYTESENTS                8
#define NPSYSCURRREADREQ                   9

    //      The Card Connection Statistics subgroup of npCard
#define NPCONNSACCEPTS        1
#define NPCONNSREFUSED        2
#define NPCONNSDENYS          3
#define NPCONNSDENYSIP        4
#define NPCONNSABORTS         5
#define NPCONNSABORTREASON    6
#define NPCONNSABORTIP        7
#define NPCONNSABORTPORT      8
#define NPCONNSABORTTIME      9
#define NPCONNSSTATE          10
#define NPCONNSIP             11
#define NPCONNSPORT           12
#define NPCONNSPERIPCLOSE     13
#define NPCONNSIDLETIMEOUTS   14
#define NPCONNSNMCLOSE        15
#define NPCONNSBYTESRECVD     16
#define NPCONNSBYTESSENTS     17

    //      The Card Configuration subgroup of npCard
#define NPCFGSOURCE                1
#define NPCFGYIADDR                2
#define NPCFGSIADDR                3
#define NPCFGGIADDR                4
#define NPCFGLOGSERVER             5
#define NPCFGSYSLOGFACILITY        6
#define NPCFGACCESSSTATE           7
#define NPCFGACCESSLISTNUM         8
#define NPCFGACCESSLISTINDEX       91
#define NPCFGACCESSLISTADDRESS     92
#define NPCFGACCESSLISTADDRMASK    93
#define NPCFGIDLETIMEOUT           10
#define NPCFGLOCALSUBNETS          11
    //      The Unknown data
#define NPCFG12                    12
#define NPCFG13                    13

    //      The TCP subgroup of npCard
#define NPTCPINSEGINORDER       1
#define NPTCPINSEGOUTOFORDER    2
#define NPTCPINSEGZEROPROBE     3
#define NPTCPINDISCARDS         4

    //      The Card Control subgroup of npCard
#define NPCTLRECONFIGIP         1
#define NPCTLRECONFIGPORT       2
#define NPCTLRECONFIGTIME       3
#define NPCTLCLOSEIP            4
#define NPCTLCLOSEPORT          5
#define NPCTLIMAGEDUMP          6
#define NPCTLCLOSECONNECTION    7
#define NPCTLRECONFIG           8
#define NPCTLPROTOCOLSET        9
    //      The Unknown data
#define NPCTL10                 10
#define NPCTL11                 11
#define NPCTL12                 12

    //  --- The HP Modular Input/Output (MIO) subgroup of npCard
         //  --- The Card Status Entry
#define NPNPICSEDATASTATE         1
#define NPNPICSEERRORCODE         2
#define NPNPICSELINKEVENT         3
#define NPNPICSEREADMODE          4
#define NPNPICSEWRITEMODE         5
#define NPNPICSEWARNINGCODE       6
#define NPNPICSECONNECTIONSTATE   7
#define NPNPICSENOVWARNINGCODE    8
#define NPNPICSELLCWARNINGCODE    9
#define NPNPICSETCPWARNINGCODE    10
#define NPNPICSEATKWARNINGCODE    11
        //   --- The Peripheral Attribute Entry
#define NPNPIPERIPHERALATTRIBUTECOUNT  21
#define NPNPIPAELINKDIRECTION     31
#define NPNPIPAECLASS             32
#define NPNPIPAEIDENTIFICATION    33
#define NPNPIPAEREVISION          34
#define NPNPIPAEAPPLETALK         35
#define NPNPIPAEMESSAGE           36
#define NPNPIPAERESERVED          37
#define NPNPIPAEMULTICHAN         38
#define NPNPIPAEPAD               39
        //   --- The Card Attribute Entry
#define NPNPICAELINKDIRECTION     41
#define NPNPICAECLASS             42
#define NPNPICAEIDENTIFCATION     43
#define NPNPICAEREVISION          44
#define NPNPICAEAPPLETALK         45
#define NPNPICAEMESSAGE           46
#define NPNPICAEREVERSED          47
#define NPNPICAEMULTICHAN         48

    //      The IPX subgroup of npCard
#define NPIPXGETUNITCFGRESP       1
#define NPIPX8022FRAMETYPE        2
#define NPIPXSNAPFRAMETYPE        3
#define NPIPXETHERNETFRAMETYPE    4
#define NPIPX8023RAWFRAMETYPE     5
    //      The Unknown data
#define NPIPX6                    6
#define NPIPX7                    7
#define NPIPX8                    8
#define NPIPX9                    9
//#define NPIPX13                   13
#define NPIPX16                   16
#define NPIPX10Entry_1            1

    //      The Unknown subgroup of npCard
#define NPCARD11_1  1

    //      The Unknown subgroup of npCard
#define NPCARD12_7  7
#define NPCARD12_8  8

    //      The Unknown subgroup of npCard
#define NPCARD13_1  1
#define NPCARD13_3  3
#define NPCARD13_4  4

    //      The Unknown subgroup of npCard
#define NPCARD17_1  1

    //      The Unknown subgroup of npCard
//#define NPCARD18_1  1
#endif WEBADMIN
// ------ End of here  >>>> By Arius


//*
//* The subtree structure contains a subtree prefix which applies to
//* all variables in the associated variable list.
//* No subtree may be a subtree of another subtree in this list.  i.e.:
//* 1.2
//* 1.2.0
//*
#ifdef WEBADMIN //--- Arius 5/15/2000
#define SNMP_MAX_BASE_LEN  12
#else
#define SNMP_MAX_BASE_LEN  9
#endif WEBADMIN

struct variable {
        uint8  magic;         // passed to function as a hint
        char    type;          // type of variable
// See important comment in snmpvars.c relating to acl
        uint16  acl;           // access control list for variable
        uint8  *(*findVar)(struct variable *,oid *, int  *,int,int *,int (**)()); // function that finds variable
        uint8  namelen;       // length of above */
        oid     name[32];          // object identifier of variable */
};

struct subtree {
        oid                     name[SNMP_MAX_BASE_LEN]; // objid prefix of subtree
        uint8           namelen;             // number of subid's in name above
    struct variable     *variables;  // pointer to variables array
    int                 variables_len;   // number of entries in above array
    int                 variables_width; // sizeof each variable entry */
    struct subtree *next; };


struct variable2 {
        uint8  magic;         // passed to function as a hint
        char    type;          // type of variable
// See important comment in snmpvars.c relating to acl
        uint16  acl;           // access control list for variable
        uint8   *(*findVar)(struct variable *,oid *, int  *,int,int *,int (**)()); // function that finds variable
        uint8   namelen;       // length of above */
        oid     name[2];           // object identifier of variable */
};

struct variable4 {
        uint8  magic;         // passed to function as a hint
        char    type;          // type of variable
// See important comment in snmpvars.c relating to acl
        uint16  acl;           // access control list for variable
        uint8  *(*findVar)(struct variable *,oid *, int  *,int,int *,int (**)()); // function that finds variable
        uint8  namelen;       // length of above */
        oid     name[4];           // object identifier of variable */
};

struct variable7 {
        uint8  magic;         // passed to function as a hint
        char    type;          // type of variable
// See important comment in snmpvars.c relating to acl
        uint16  acl;           // access control list for variable
        uint8  *(*findVar)(struct variable *,oid *, int  *,int,int *,int (**)()); // function that finds variable
        uint8  namelen;       // length of above */
        oid     name[7];           // object identifier of variable */
};

struct variable13 {
        uint8  magic;         // passed to function as a hint
        char    type;          // type of variable
// See important comment in snmpvars.c relating to acl
        uint16  acl;           // access control list for variable
        uint8  *(*findVar)(struct variable *,oid *, int  *,int,int *,int (**)()); // function that finds variable
        uint8  namelen;       // length of above */
        oid     name[13];          // object identifier of variable */
};


// Adding for enterprise's MIB data. Here print server will support
// HP jetadmin/web jetadmin
// 05/12/2000 ---- add by Arius
#ifdef WEBADMIN
struct variable20 {
        uint8  magic;         // passed to function as a hint
        char    type;          // type of variable
// See important comment in snmpvars.c relating to acl
        uint16  acl;           // access control list for variable
        uint8  *(*findVar)(struct variable *,oid *, int  *,int,int *,int (**)()); // function that finds variable
        uint8  namelen;       // length of above */
        oid     name[20];          // object identifier of variable */
};
#endif WEBADMIN
// ------ End of here  >>>> By Arius

void snmp_vars_init(void);
uint8   *getStatPtr(oid *name,int *namelen,uint8  *type,
                    int *len,uint16 *acl,
                    int exact,int (**write_method)(),int snmpversion,
                    int *noSuchObject,int view);
#ifdef WEBADMIN // modified ---- Arius 5/15/2000
#define SNMP_VER_LEN 11
#else
#define SNMP_VER_LEN 10
#endif
extern oid SnmpVersionID[SNMP_VER_LEN];

#endif _SNMPVARS_H
