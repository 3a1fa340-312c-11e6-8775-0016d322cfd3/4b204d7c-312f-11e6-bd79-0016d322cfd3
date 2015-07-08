/*
 * vars for the MIB-II snmp group
 *
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

		      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES
OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/


#ifndef SNMP_GROUPVARS_H
#define SNMP_GROUPVARS_H

//Define for Interface MIBII
extern uint32 mib_ifInOctets;
extern uint32 mib_ifInUcastPkts;
extern uint32 mib_ifInNucastPkts;
extern uint32 mib_ifInDiscards;
//extern uint32 mib_ifInErrors;
extern uint32 mib_ifInUnknowProtos;

extern uint32 mib_ifOutOctets;
extern uint32 mib_ifOutUcastPkts;
extern uint32 mib_ifOutNucastPkts;
//extern uint32 mib_ifOutDiscards;
extern uint32 mib_ifOutErrors;
extern uint32 mib_ifOutQLen;

/////////////////////////////////////

extern int32 snmp_inpkts;
#define snmpInPkts snmp_inpkts              //{ snmp 1 }

extern int32 snmp_outpkts;
#define snmpOutPkts snmp_outpkts               //{ snmp 2 }

extern int32 snmp_inbadversions;
#define snmpInBadVersions snmp_inbadversions   //{ snmp 3 }

extern int32 snmp_inbadcommunitynames;
#define snmpInBadCommunityNames snmp_inbadcommunitynames  //{ snmp 4 }

// extern int32 snmp_inbadcommunityuses;
//             snmpInBadCommunityUses ;    //{ snmp 5 }

extern int32 snmp_inasnparseerrors;
#define snmpInASNParseErrs snmp_inasnparseerrors        //{ snmp 6 }

                                                    //{ snmp 7 } is not used
extern int32 snmp_intoobigs;
#define snmpInTooBigs snmp_intoobigs             //{ snmp 8 }

// extern int32 snmp_innosuchnames;
//#define snmpInNoSuchNames snmp_innosuchnames         //{ snmp 9 }

extern int32 snmp_inbadvalues;
#define snmpInBadValues snmp_inbadvalues           //{ snmp 10 }

extern int32 snmp_inreadonlys;
#define snmpInReadOnlys snmp_inreadonlys           //{ snmp 11 }

extern int32 snmp_ingenerrs;
#define snmpInGenErrs snmp_ingenerrs             //{ snmp 12 }

extern int32 snmp_intotalreqvars;
#define snmpInTotalReqVars snmp_intotalreqvars        //{ snmp 13 }

// extern int32 snmp_intotalsetvars;
//#define snmpInTotalSetVars snmp_intotalsetvars       //{ snmp 14 }

extern int32 snmp_ingetrequests;
#define snmpInGetRequests snmp_ingetrequests         //{ snmp 15 }

extern int32 snmp_ingetnexts;
#define snmpInGetNexts snmp_ingetnexts            //{ snmp 16 }

extern int32 snmp_insetrequests;
#define snmpInSetRequests snmp_insetrequests        //{ snmp 17 }


// extern int32 snmp_ingetresponses;
//#define snmpInGetResponses snmp_ingetresponses        //{ snmp 18 }

// extern int32 snmp_intraps;
//extern snmpInTraps snmp_intraps              //{ snmp 19 }

// extern int32 snmp_outtoobigs;
//#define snmpOutTooBigs snmp_outtoobigs            //{ snmp 20 }

extern int32 snmp_outnosuchnames;
#define snmpOutNoSuchNames snmp_outnosuchnames        //{ snmp 21 }

// extern int32 snmp_outbadvalues;
//#define snmpOutBadValues snmp_outbadvalues          //{ snmp 22 }

                                                  //{ snmp 23 } is not used
// extern int32 snmp_outgenerrs;
//#define snmpOutGenErrs snmp_outgenerrs            //{ snmp 24 }

extern int32 snmp_outgetrequests;
#define snmpOutGetRequests snmp_outgetrequests        //{ snmp 25 }	Simon

extern int32 snmp_outgetnexts;
#define snmpOutGetNexts snmp_outgetnexts           //{ snmp 26 }  Simon

extern int32 snmp_outsetrequests;
#define snmpOutSetRequests snmp_outsetrequests        //{ snmp 27 }	Simon

extern int32 snmp_outgetresponses;
#define snmpOutGetResponses snmp_outgetresponses       //{ snmp 28 }

extern int32 snmp_outtraps;
#define snmpOutTraps snmp_outtraps              //{ snmp 29 }  Simon

extern int32 snmp_enableauthentraps;
#define snmpEnableAuthenTraps snmp_enableauthentraps  //{ snmp 30 }

#endif /* SNMP_GROUPVARS_H */
