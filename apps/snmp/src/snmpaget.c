/*
 * $Log$
 */

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include <cyg/kernel/kapi.h>	//615wu
#include "network.h"   //615wu
#include "pstarget.h"   //615wu
#include    "psglobal.h"
#include "psdefine.h"	//615wu
#include    "asn1.h"
#include	"eeprom.h"		//615wu
#include    "snmp_api.h"
#include    "snmp.h"
#include    "snmpgrup.h"
#include    "snmpvars.h"
#include    "snmptrap.h"
#include    "snmpaget.h"

#define COMMUNITY_MAX_LEN 80
uint8   community[COMMUNITY_MAX_LEN+1];  //Input community name
uint8   *packet_end;
// Add here for arius 6/19/2000
#ifdef WEBADMIN // modified ---- Arius
uint8   JetPort = 1;
extern	uint8 PSMode2; 
#endif WEBADMIN

// end of here

CommunityAuth SnmpCommunityAuth[NO_OF_SNMP_COMMUNITY];

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
snmp_session    session = {
        community,              // community name
        0,                          // community name length
//    SNMP_DEFAULT_RETRIES,     // retries
//    SNMP_DEFAULT_TIMEOUT,     // how long to retry
//    NULL,                     // peer host name
        SNMP_DEFAULT_REMPORT,   // peer host port
        161,                    // snmp daemon port
//    NULL,                     // authentication function call
        NULL,
    NULL,                       // call back message that I want to sepecify
    NULL,                       // socket file description
    {NULL},                     // peer socket_internet_address
    {NULL},                     // request list
        0               //access
};

int snmp_agent_parse(snmp_session *session, uint8 *indata,int inlength,uint8 *outdata,int *outlength);
snmp_session *snmp_open (snmp_session *session);
void snmp_read (snmp_session *session);
int parse_var_op_list(snmp_session *session, uint8 *data,int length,uint8 *out_data,int out_length,long *index,int msgtype,int action);
int create_identical(snmp_session *session, uint8 *snmp_in,uint8 *snmp_out,int snmp_length,long errstat,long errindex);
int community_auth(snmp_session *session);
static int goodValue(uint8 inType,int inLen,uint8 actualType,int actualLen);
void setVariable(uint8 *var_val,uint8  var_val_type,int var_val_len,uint8 *statP,int statLen);

//-------------------------------------------------------------------
//                        SNMP AGENT process
//-------------------------------------------------------------------
void SnmpAgent(cyg_addrword_t data)	//615wu
{
        if(snmp_open(&session) == NULL) return;

        for (;;)  {
                snmp_read (&session);
                sys_check_stack();
				cyg_thread_yield();	//615wu
        }
}

//---------------------------------------------------------------------------------
// Function Name: snmp_open
// Description  : Create a snmp session with information provided by the user. Then
//                opens and binds the necessary UDP port. And return the created
//                session handle.
// Return Value : NULL - if fail to open a session handle
//                ptr of session - if open it successfully
// Called By    : user
//---------------------------------------------------------------------------------
snmp_session *snmp_open (snmp_session *session)
{
        struct sockaddr_in      me;

        if (session == NULL) return NULL ; //------SNMP: Input a null session.

        if (session->remote_port != 0) {
                session->r_addr.sin_family = AF_INET ;	
				session->r_addr.sin_port = htons(session->remote_port);		//615wu
				session->r_addr.sin_len = sizeof(struct sockaddr_in); //615wu
//              session->r_addr.sin_addr.s_addr = TrapDestIP;
        }
    else
                session->r_addr.sin_addr.s_addr = 0 ;   //---without specify peername--

    //------------------------Set up connection.-----------------------------
        if ((session->sfd=socket(AF_INET,SOCK_DGRAM,0)) == -1 ) {
//              snmp_close(session) ;
                return NULL ;           //---SNMP: Cannot open a socket.----
        }
        
        memset(&me , 0x0, sizeof(me));	//615wu
        
        me.sin_family      = AF_INET ;
		me.sin_addr.s_addr = htonl(INADDR_ANY) ;	//615wu
		
        if (session->local_port == 0) return NULL;
		me.sin_port = htons (session->local_port);	//615wu
        me.sin_len = sizeof(me); //615wu

        if ( bind(session->sfd,(struct sockaddr *)&me,sizeof(me)) == -1 ) {
//              snmp_close (session) ;
                return NULL ;           //---SNMP: Binding port error.------
        }
        return (session) ;
}

//--------------------------------------------------------------------------
// Function Name: snmp_read
// Description  : Checks to see if any of the fd's set in the fdset belong to snmp.
//                Each socket with it's fd set has a packet read from it and
//                snmp_parse is called on the packet received. The resulting pdu is
//                passed to the callback routine for that session. If the callback
//                routine returns successfully, the pdu and it's request are
//                deleted.
// Return Value : NONE
// Calling To   : udp_read, snmp_parse, free_pdu and user's callback
//                function
// Called By    : user
// Status       : some part of this module should be mdoified
//-------------------------------------------------------------------------
void snmp_read (snmp_session *session)
{
        uint8 inpacket[SNMP_MAX_LEN], outpacket[SNMP_MAX_LEN];
        int   outlength, fromlength, inlength;

        inlength = SNMP_MAX_LEN;
        if(udp_read(session, inpacket, &inlength)) {
                snmpInPkts++ ;

                outlength = SNMP_MAX_LEN;
                if(snmp_agent_parse(session, inpacket, inlength, outpacket, &outlength))
                {

#if defined(SNMP_DUMP) && defined(PC_OUTPUT)
                        {
                        int count;

                        printf("sent %d bytes to %s:\n", (int) outlength,
                              inet_ntoa(from.sin_addr));
                        for (count = 0; count < outlength; count++) {
                                printf("%02X ", outpacket[count]);
                                if ((count % 16) == 15)
                                        printf("\n");
                                }
                        printf("\n\n");
                        }
#endif
                        if(snmp_udp_send(session, outpacket,outlength) == 0) return;
                        snmp_outpkts++;

#ifdef WEBADMIN
                          RunChangeIPAddress();
#ifndef _PC
           RunWriteDataToEEPROM();
#endif
#endif WEBADMIN
                }
        }
}


//-------------------------------------------------------------------
//                        SNMP AGENT process
//-------------------------------------------------------------------

int
snmp_agent_parse(
        snmp_session *session,
        uint8 *indata,
        int inlength,
        uint8 *outdata,
        int *outlength
)
{
        uint8    msgtype, type;
        long      zero = 0;
        long      reqid, errstat, errindex, dummyindex;
        uint8    *out_auth, *out_header, *out_reqid;
        uint8    *startData = indata;
        int       startLength = inlength;
        long      version;
        uint8    *origdata = indata;
        int           origlen = inlength;
//      usecEntry *ue;
        int       packet_len, ret = 0;
#ifdef WEBADMIN
        int       newoutdata;   // add -- by arius 5/17/2000
        int       len;          // add -- by arius 5/17/2000
#endif WEBADMIN

        session->community_len = COMMUNITY_MAX_LEN;
        //get community name
        indata = snmp_parse_auth(indata, &inlength, community, &session->community_len, &version);

        if (indata == NULL){
//              increment_stat( SNMP_STAT_ENCODING_ERRORS );
                ERROR("bad auth encoding");
                return 0;
        }

#if 1 //ONLY SUPPORT VERSION 1
        if(version != SNMP_VERSION_1) {
                ERROR("wrong version");
                snmp_inbadversions++;
                return 0;
        }

#else ////////////////////////////////////////////
        if( version != SNMP_VERSION_1 && version != SNMP_VERSION_2C && version != SNMP_VERSION_2 ) {
//              increment_stat( SNMP_STAT_ENCODING_ERRORS );
                ERROR("wrong version");
                snmp_inbadversions++;
                return 0;
        }

        if( version == SNMP_VERSION_2C || version == SNMP_VERSION_2 ) {
                if( version == SNMP_VERSION_2 ) {
                        ret = check_auth( session, origdata, origlen, indata - session->community_len, session->community_len, &ue );
                        *outlength = (SNMP_MAX_LEN < session->MMS) ? SNMP_MAX_LEN : session->MMS;
                        session->MMS = SNMP_MAX_LEN;
                } else if( version == SNMP_VERSION_2C ) {
                        ret = community_auth( session );
                        session->version = SNMP_VERSION_2C;
                }

                if( ret < 0 ) {
//                      increment_stat( -ret );
                        if( (indata=asn_parse_header(indata, &inlength, &msgtype))
                        &&  asn_parse_int(indata, &inlength, &type, &reqid, sizeof(reqid)) )
                        {
                                if( msgtype == REPORT_MSG ) return 0;
                                if( !(session->qoS & USEC_QOS_GENREPORT) ) return 0;
                                session->agentBoots = _agentBoots;
                                session->agentClock = _agentStartTime;
                                memcpy( session->agentID, _agentID, 12 );
                                session->MMS = SNMP_MAX_LEN;
                                create_report( session, outdata, outlength, -ret, reqid );
                                return 1;
                        } else {
                                return 0;
                        }
                } else if( ret > 0 ) {
//                      increment_stat( ret );
                        return 0;
                }

        } else  {
#endif  /////////////////////////////////////////////////////

        //VERSION 1
        if(community_auth( session )==0) return 0;
//      session->version = SNMP_VERSION_1;

#if 0 ////////
        }
#endif ///////

        indata = asn_parse_header(indata, &inlength, &msgtype);
        if (indata == NULL){
//              increment_stat( SNMP_STAT_ENCODING_ERRORS );
                ERROR("bad header");
                return 0;
        }

    // XXX: increment by total number of vars at correct place:
    snmp_intotalreqvars++;

        if(msgtype == GET_REQ_MSG)      snmp_ingetrequests++;
        else if (msgtype == GETNEXT_REQ_MSG) snmp_ingetnexts++;
        else if (msgtype ==     SET_REQ_MSG) snmp_insetrequests++;
        else return 0;

//   if (msgtype == GETBULK_REQ_MSG: //version 2

        // Request ID
        indata = asn_parse_int(indata, &inlength, &type, &reqid, sizeof(reqid));
        if (indata == NULL){
//              increment_stat( SNMP_STAT_ENCODING_ERRORS );
                ERROR("bad parse of reqid");
                return 0;
        }

        //Error Status
        indata = asn_parse_int(indata, &inlength, &type, &errstat, sizeof(errstat));
        if (indata == NULL){
//              increment_stat( SNMP_STAT_ENCODING_ERRORS );
                ERROR("bad parse of errstat");
                snmp_inasnparseerrors++;
                return 0;
        }

        //Error Index
    indata = asn_parse_int(indata, &inlength, &type, &errindex, sizeof(errindex));
    if (indata == NULL){
//              increment_stat( SNMP_STAT_ENCODING_ERRORS );
                ERROR("bad parse of errindex");
                return 0;
        }

     //
     // Now start cobbling together what is known about the output packet.
     // The final lengths are not known now, so they will have to be recomputed
     // later.
     //

    // setup for response
//Simon    time( (time_t *) &session->agentTime );
//Simon    session->agentClock = _agentStartTime;
//Simon    session->agentBoots = _agentBoots;
//Simon   memcpy( session->agentID, _agentID, 12 );

        out_auth = outdata;

        //Set Resp Community
        out_header = snmp_build_auth(out_auth, outlength,
                                     community,session->community_len,
                                     (long)SNMP_VERSION_1,0);
        if (out_header == NULL){
                ERROR("snmp_build_auth failed");
                snmp_inasnparseerrors++;
                return 0;
        }

        out_reqid = asn_build_sequence(out_header, outlength, (uint8)GET_RSP_MSG, 0);
        if (out_reqid == NULL){
                ERROR("out_reqid == NULL");
                return 0;
        }

        //Set Resp ID
        type = (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER);
        // return identical request id
        outdata = asn_build_int(out_reqid, outlength, type, &reqid, sizeof(reqid));
        if (outdata == NULL){
                ERROR("build reqid failed");
                return 0;
        }

    // assume that error status will be zero
        outdata = asn_build_int(outdata, outlength, type, &zero, sizeof(zero));
        if (outdata == NULL){
                ERROR("build errstat failed");
                return 0;
        }

    // assume that error index will be zero
        outdata = asn_build_int(outdata, outlength, type, &zero, sizeof(zero));
        if (outdata == NULL){
                ERROR("build errindex failed");
                return 0;
        }

#if 0 ///////////////////////////////////////////
        if (msgtype == GETBULK_REQ_MSG)
        errstat = bulk_var_op_list(indata, inlength, outdata, *outlength,
                                    errstat, errindex, &errindex );
    else
#endif 0 ////////////////////////////////////////

        errstat = parse_var_op_list(session, indata, inlength, outdata, *outlength,
                                    &errindex, msgtype, RESERVE1);

        if (msgtype== SET_REQ_MSG){
                if (errstat == SNMP_ERR_NOERROR)
                        errstat = parse_var_op_list(session, indata, inlength, outdata, *outlength,
                                        &errindex, msgtype, RESERVE2);
                if (errstat == SNMP_ERR_NOERROR){
                //
                //* SETS require 3-4 passes through the var_op_list.  The first two
                //* passes verify that all types, lengths, and values are valid
                //* and may reserve resources and the third does the set and a
                //* fourth executes any actions.  Then the identical GET RESPONSE
                //* packet is returned.
                //* If either of the first two passes returns an error, another
                //* pass is made so that any reserved resources can be freed.
                //*
              parse_var_op_list(session, indata, inlength, outdata, *outlength,
                                &dummyindex, msgtype, COMMIT);
              parse_var_op_list(session, indata, inlength, outdata, *outlength,
                                &dummyindex, msgtype, ACTION);
              if (create_identical(session, startData, out_auth, startLength, 0L, 0L )){
                  *outlength = packet_end - out_auth;
                  return 1;
              }
              return 0;
        } else {
              parse_var_op_list(session, indata, inlength, outdata, *outlength,
                                &dummyindex, msgtype, COMM_FREE);
        }
    }
    switch((short)errstat){
        case SNMP_ERR_NOERROR:
            // re-encode the headers with the real lengths
            *outlength = packet_end - out_header;
            packet_len = *outlength;
            outdata = asn_build_sequence(out_header, outlength, GET_RSP_MSG,
                                        packet_end - out_reqid);
// add --- By arius 5/17/2000
#ifdef WEBADMIN
newoutdata = packet_end - outdata;  // how many shifts
len = packet_end - out_reqid;
packet_end = outdata;               // save new pointer end of packet
outdata -= len;
out_reqid = out_reqid - newoutdata;
#endif WEBADMIN
// end of here
            if (outdata != out_reqid){
                ERROR("internal error: header");
                return 0;
            }

            *outlength = packet_end - out_auth;
            outdata = snmp_build_auth(out_auth, outlength,
                                          community, session->community_len,
                                          (long)SNMP_VERSION_1,
                                          packet_end - out_header );
// add --- By arius 5/17/2000
#ifdef WEBADMIN
out_header -= (packet_end - outdata);
packet_end = outdata;            // save new pointer end of packet
#endif WEBADMIN
// end of here
            *outlength = packet_end - out_auth;
#if 0
            // packet_end is correct for old SNMP.  This dichotomy needs to be fixed.
            if (session->version == SNMP_VERSION_2)
                packet_end = out_auth + packet_len;
#endif
            break;
        case SNMP_ERR_TOOBIG:
            snmp_intoobigs++;
#if notdone
            if (session->version == SNMP_VERSION_2){
                create_toobig(out_auth, *outlength, reqid, pi);
                break;
            } // else FALLTHRU
#endif
        case SNMP_ERR_NOACCESS:
        case SNMP_ERR_WRONGTYPE:
        case SNMP_ERR_WRONGLENGTH:
        case SNMP_ERR_WRONGENCODING:
        case SNMP_ERR_WRONGVALUE:
        case SNMP_ERR_NOCREATION:
        case SNMP_ERR_INCONSISTENTVALUE:
        case SNMP_ERR_RESOURCEUNAVAILABLE:
        case SNMP_ERR_COMMITFAILED:
        case SNMP_ERR_UNDOFAILED:
        case SNMP_ERR_AUTHORIZATIONERROR:
        case SNMP_ERR_NOTWRITABLE:
    case SNMP_ERR_INCONSISTENTNAME:
        case SNMP_ERR_NOSUCHNAME:
        case SNMP_ERR_BADVALUE:
        case SNMP_ERR_READONLY:
        case SNMP_ERR_GENERR:
            if (create_identical(session, startData, out_auth, startLength, errstat,
                                 errindex)){
                *outlength = packet_end - out_auth;
                return 1;
            }
            return 0;
        default:
            return 0;
    }

#if 0
    if( session->qoS & USEC_QOS_AUTH ) {
        md5Digest( out_auth, *outlength, outdata - (session->contextLen + 16),
            outdata - (session->contextLen + 16) );
    }
#endif

        return 1;
}

//*
//* Parse_var_op_list goes through the list of variables and retrieves each one,
//* placing it's value in the output packet.  In the case of a set request,
//* if action is RESERVE, the value is just checked for correct type and
//* value, and resources may need to be reserved.  If the action is COMMIT,
//* the variable is set.  If the action is FREE, an error was discovered
//* somewhere in the previous RESERVE pass, so any reserved resources
//* should be FREE'd.
//* If any error occurs, an error code is returned.
//*
int
parse_var_op_list(
        snmp_session *session,
        uint8 *data,      //point to variable-binding of in-packet data
        int    length,     //in packet data length
        uint8 *out_data,  //point to variable-binding of out-packet data
        int    out_length, //out packet buffer length
        long   *index,     //error-index (output)
        int    msgtype,    //message type
        int    action      //???
)
{
        uint8  type;
        oid         var_name[SNMP_MAX_NAME_LEN];
        int         var_name_len, var_val_len;
        uint8  var_val_type, *var_val, statType;
        uint8 *statP;
        int         statLen;
        uint16  acl;
        int         (*write_method)();
        uint8  *headerP, *var_list_start;
        int         dummyLen;
#ifdef WEBADMIN
        int         dummyLen1;      // add -- By arius 5/17/2000
        uint8       *newpacket_end; // add -- By arius 5/17/2000
#endif WEBADMIN
        int         noSuchObject;
        int     (*WriteMethod)(int,uint8 *,uint8,int,uint8 *,oid *,int);
        int         err, exact;

//      if (msgtype== SET_REQ_MSG)      rw = SNMP_WRITE;
//  else rw = SNMP_READ;

        if (msgtype == GETNEXT_REQ_MSG) exact = FALSE;
        else exact = TRUE;

        //Parse variable-binging header
        data = asn_parse_header(data, &length, &type);

        if (data == NULL){
                ERROR("not enough space for varlist");
                return PARSE_ERROR;
        }
        if (type != (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR)){
                ERROR("wrong type");
                return PARSE_ERROR;
    }

        headerP = out_data;
        //Set Resp variable-binging header
    out_data = asn_build_sequence(out_data, &out_length,
                                (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 0);

        if (out_data == NULL){
                ERROR("not enough space in output packet");
                return BUILD_ERROR;
        }

    var_list_start = out_data;

        *index = 1;
        while((int)length > 0){
                // parse the name, value pair
                var_name_len = SNMP_MAX_NAME_LEN;

                data = snmp_parse_var_op(data, var_name, &var_name_len, &var_val_type,
                                         &var_val_len, &var_val, (int *)&length);

                if (data == NULL)  return PARSE_ERROR;

                // now attempt to retrieve the variable on the local entity
                statP = getStatPtr(var_name, &var_name_len, &statType,
                           &statLen, &acl, exact,
                           &write_method, SNMP_VERSION_1, &noSuchObject,
                           (msgtype==SET_REQ_MSG?
                             (session->access & SNMP_ACCESS_WRITE):
                             (session->access & SNMP_ACCESS_READ)    )
                        );

                if (/* session->version == SNMP_VERSION_1 &&*/
                                    statP == NULL &&
                                    (msgtype != SET_REQ_MSG || !write_method))
                {
                        //** XXX: happens when running
                        //   XXX: snmpwalk localhost public .1
                        //   XXX: fix me some day.
                        //**
                        //** ERROR("warning: internal v1_error"); **
                        return SNMP_ERR_NOSUCHNAME;
                }

                // check if this variable is read-write (in the MIB sense).
                if( msgtype == SET_REQ_MSG && acl != RWRITE )
                        return SNMP_ERR_READONLY;
//          return session->version == SNMP_VERSION_1 ? SNMP_ERR_NOSUCHNAME : SNMP_ERR_NOTWRITABLE;

                //* Its bogus to check here on getnexts - the whole packet shouldn't
                //   be dumped - this should should be the loop in getStatPtr
                //   luckily no objects are set unreadable.  This can still be
                //  useful for sets to determine which are intrinsically writable

                if (msgtype== SET_REQ_MSG){
                        if (write_method == NULL){
                                if (statP != NULL){
                                        // see if the type and value is consistent with this entity's variable
                                        if (!goodValue(var_val_type, var_val_len, statType,statLen))
                                        {
//                                              if (session->version != SNMP_VERSION_1)
//                                                      return SNMP_ERR_WRONGTYPE; // poor approximation
//                                              else
                                                {
                                                        snmp_inbadvalues++;
                                                        return SNMP_ERR_BADVALUE;
                                                }
                                    }
                                    // actually do the set if necessary
                                        if (action == COMMIT)
                                                setVariable(var_val, var_val_type, var_val_len,statP, statLen);
                                } else {
//                                  if (session->version != SNMP_VERSION_1)
//                                              return SNMP_ERR_NOCREATION;
//                                  else
                                                return SNMP_ERR_NOSUCHNAME;
                                }
                        } else {
                                WriteMethod = (int (*)(int,uint8 *,uint8,int,uint8 *,oid *,int)) write_method;

                                err = (*WriteMethod)(action, var_val, var_val_type,
                                          var_val_len, statP, var_name,
                                          var_name_len);

                                //*
                                //* Map the SNMPv2 error codes to SNMPv1 error codes (RFC 2089).
                                //*

//                              if (session->version == SNMP_VERSION_1) {
                                    switch (err) {
                                        case SNMP_ERR_NOERROR:
                                                        // keep the no-error error:
                                                        break;
                                            case SNMP_ERR_WRONGVALUE:
                                            case SNMP_ERR_WRONGENCODING:
                                            case SNMP_ERR_WRONGTYPE:
                                            case SNMP_ERR_WRONGLENGTH:
                                            case SNMP_ERR_INCONSISTENTVALUE:
                                                        err = SNMP_ERR_BADVALUE;
                                                        break;
                                            case SNMP_ERR_NOACCESS:
                                            case SNMP_ERR_NOTWRITABLE:
                                            case SNMP_ERR_NOCREATION:
                                            case SNMP_ERR_INCONSISTENTNAME:
                                            case SNMP_ERR_AUTHORIZATIONERROR:
                                                        err = SNMP_ERR_NOSUCHNAME;
                                                        break;
                                            default:
                                                        err = SNMP_ERR_GENERR;
                                                        break;
//                                      }
                                }//if(session->version == SNMP_VERSION_1) ....

                                if (err != SNMP_ERR_NOERROR){
//                                      if (session->version == SNMP_VERSION_1) {
                                                snmp_inbadvalues++;
//                                      }
                                        return err;
                                }
                        }
                } else {
                        //* retrieve the value of the variable and place it into the
                        //* outgoing packet
                        if (statP == NULL){
                                statLen = 0;
                                if (exact){
                                        if (noSuchObject == TRUE){
                                                statType = SNMP_NOSUCHOBJECT;
                                        } else {
                                                statType = SNMP_NOSUCHINSTANCE;
                                        }
                                } else {
                                        statType = SNMP_ENDOFMIBVIEW;
                                }
                        } //if (statP == NULL) ...

                        out_data = snmp_build_var_op(out_data, var_name, &var_name_len,
                                                     statType, statLen, statP,
                                                     &out_length);
                        if (out_data == NULL){
                                return SNMP_ERR_TOOBIG;
                        }
                } //if (msgtype== SET_REQ_MSG) ...

                (*index)++;
        } //while((int)length > 0) ....
        if(msgtype!= SET_REQ_MSG){
                // save a pointer to the end of the packet
                packet_end = out_data;

                // Now rebuild header with the actual lengths
                dummyLen = packet_end - var_list_start;

// modified by ---- Arius 5/17/2000
#ifdef WEBADMIN
  dummyLen1 = packet_end - headerP;
  newpacket_end = asn_build_sequence(headerP, &dummyLen1,
                    (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), dummyLen);
  var_list_start -= (packet_end - newpacket_end);
  out_length += (packet_end - newpacket_end);
  // save new pointer to the end of the packet when the sapces will be adjust
  packet_end = newpacket_end;
  if (headerP == NULL)
#else
                if (asn_build_sequence(headerP, &dummyLen,
                                       (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                                       dummyLen) == NULL)
#endif WEBADMIN
// end of here
                {
                        return SNMP_ERR_TOOBIG; // bogus error ????
                }
        }
        *index = 0;
        return SNMP_ERR_NOERROR;
}

//*
//* create a packet identical to the input packet, except for the error status
//* and the error index which are set according to the input variables.
//* Returns 1 upon success and 0 upon failure.
//*
int
create_identical(
        snmp_session *session,
    uint8 *snmp_in,
    uint8 *snmp_out,
    int   snmp_length,
    long  errstat,
    long  errindex
)
{
        uint8  *data;
        uint8  type;
        uint32 dummy;
        uint8  *headerPtr, *reqidPtr, *errstatPtr, *errindexPtr, *varListPtr;
        int        headerLength, length;

        bcopy((char *)snmp_in, (char *)snmp_out, snmp_length);
        length = snmp_length;
        headerPtr = snmp_parse_auth(snmp_out, &length, community, &session->community_len, (long *)&dummy);
        community[session->community_len] = 0;
        if (headerPtr == NULL) return 0;

    reqidPtr = asn_parse_header(headerPtr, &length, (uint8 *)&dummy);
    if (reqidPtr == NULL)
        return 0;
    headerLength = length;
    errstatPtr = asn_parse_int(reqidPtr, &length, &type, (long *)&dummy, sizeof dummy); // request id
    if (errstatPtr == NULL)
        return 0;
    errindexPtr = asn_parse_int(errstatPtr, &length, &type, (long *)&dummy, sizeof dummy);      // error status
    if (errindexPtr == NULL)
        return 0;
    varListPtr = asn_parse_int(errindexPtr, &length, &type, (long *)&dummy, sizeof dummy);      // error index
    if (varListPtr == NULL)
        return 0;

#if 0
    data = asn_build_header(headerPtr, &headerLength, GET_RSP_MSG, headerLength);
    if (data != reqidPtr)
        return 0;
#else
         /*bug fixed */
    *headerPtr = GET_RSP_MSG;
#endif

    length = snmp_length;
    type = (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER);
    data = asn_build_int(errstatPtr, &length, type, &errstat, sizeof errstat);
    if (data != errindexPtr)
        return 0;
    data = asn_build_int(errindexPtr, &length, type, &errindex, sizeof errindex);
    if (data != varListPtr)
        return 0;
    packet_end = snmp_out + snmp_length;
    return 1;
}

#ifdef KINETICS
struct pbuf *
definitelyGetBuf(){
    struct pbuf *p;

    K_PGET(PT_DATA, p);
    while(p == 0){
#ifdef notdef
        if (pq->pq_head != NULL){
            K_PDEQ(SPLIMP, pq, p);
            if (p) K_PFREE(p);
        } else if (sendq->pq_head != NULL){
            K_PDEQ(SPLIMP, sendq, p);
            if (p) K_PFREE(p);
        }
#endif
        K_PGET(PT_DATA, p);
    }
    return p;
}
#endif

int community_auth(snmp_session *session)
{
	int i;
// modifiled for WEBAdmin/JetAdmin
// More than one ports in JetAdmin and WebAdmin will use this(internal2,Public2)
#ifdef WEBADMIN // modified ---- Arius, 
	// Charles 2001/06/13
	session->access = 0;
	JetPort = 1;
	// Charles 2002/03/06
	if( PSMode2 & PS_WEBJETADMIN_ON )
	{
		if(!strcmp(community,"internal3") || !strcmp(community,"internal.3"))
		{
			session->access |= SNMP_ACCESS_RW;
			JetPort = 3;
		}
		else if(!strcmp(community,"internal2") || !strcmp(community,"internal.2"))
		{
			session->access |= SNMP_ACCESS_RW;
			JetPort = 2;
		}
		else if(!strcmp(community,"internal") )
		{
			session->access |= SNMP_ACCESS_RW;
			JetPort = 1;
		}
		else if(!strcmp(community,"public3") || !strcmp(community,"public.3"))
		{
			session->access |= SNMP_ACCESS_READ;
			JetPort = 3;
		}		
		else if(!strcmp(community,"public2") || !strcmp(community,"public.2"))
		{
			session->access |= SNMP_ACCESS_READ;
			JetPort = 2;
		}
		else if(!strcmp(community,"public"))
		{
			session->access |= SNMP_ACCESS_READ;
			JetPort = 1;
		}
	}	
	for(i = 0 ; i < NO_OF_SNMP_COMMUNITY; i++) {
		if(strcmp(community,SnmpCommunityAuth[i].Community) == 0) {
			session->access |= SnmpCommunityAuth[i].AccessMode;
			return (1);
		}
	}
	if( session->access != 0 )
		return (1);
	if(_SnmpTrapEnable == SNMP_TRAP_ENABLE && _SnmpAuthenTrap == SNMP_TRAP_AUTH_ENABLE)
		_SendAuthFailTrap();
	return (0);
#else
	for(i = 0 ; i < NO_OF_SNMP_COMMUNITY; i++) {
		if(strcmp(community,SnmpCommunityAuth[i].Community) == 0) {
			session->access = SnmpCommunityAuth[i].AccessMode;
			return (1);
		}
	}
	if(_SnmpTrapEnable == SNMP_TRAP_ENABLE && _SnmpAuthenTrap == SNMP_TRAP_AUTH_ENABLE)
		_SendAuthFailTrap();
	return (0);
#endif WEBADMIN
}

static int goodValue(uint8 inType,int inLen,uint8 actualType,int actualLen)
{
    if (inLen > actualLen)
        return FALSE;
    return (inType == actualType);
}


void setVariable(uint8 *var_val,uint8 var_val_type,int var_val_len,uint8  *statP,int statLen)
{
	int 	buffersize = 1000;

	switch(var_val_type){
		case ASN_INTEGER:
		case COUNTER:
		case GAUGE:
		case TIMETICKS:
			asn_parse_int(var_val, &buffersize, &var_val_type, (long *)statP, statLen);
			break;
		case ASN_OCTET_STRING:
		case IPADDRESS:
		case OPAQUE:
			asn_parse_string(var_val, &buffersize, &var_val_type, statP, &statLen);
			break;
		case ASN_OBJECT_ID:
			asn_parse_objid(var_val, &buffersize, &var_val_type, (oid *)statP, &statLen);
			break;
	}
}
