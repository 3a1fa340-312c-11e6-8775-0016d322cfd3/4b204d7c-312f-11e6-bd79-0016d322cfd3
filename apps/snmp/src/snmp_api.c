/*
 * $Log$
 */
/*****************************************************************
 * snmp_api.c : a modification version of CMU's SNMP develop kit
 *****************************************************************/

#include  <stdio.h>
//#include  <alloc.h>
#include <cyg/kernel/kapi.h>    //615wu
#include "network.h"    //615wu
#include "pstarget.h"   //615wu
#include "psglobal.h"	//615wu
#include "psdefine.h"	//615wu
#include "eeprom.h"		//615wu
#include "prnport.h"    //615wu
#include  "asn1.h"
#include  "snmpgrup.h"
#include  "snmp_api.h"
#include  "snmp.h"
#include  "snmpvars.h"

#define         SNMP_PORT               161

//--------------------------------------------------------------
//
//--------------------------------------------------------------
void  At (int,int) ;

//--------------------------------------------------------------
//                   internal used variables
//--------------------------------------------------------------
static uint32   Reqid = 0;

//---- enterprises.zeroone.device.hub -----
//oid   default_enterprise[] = {1, 3, 6, 1, 4, 1, 500, 1, 1, 0};

//---------------------------------------------------------------------------------
// Function Name: dump1
// Description  : Dump 'data' content in heximal code with length 'length'
// Return Value : NONE
//---------------------------------------------------------------------------------
#ifdef          DUMP_DATA

void            dump1(uint8 *data,int length)
{
    int         i;

    for (i = 0; i < length; i++) {
        printf("%02X ", data[i]);
        if ((i % 16) == 15) printf("\n");
    }
    printf("\n\n");
}

#endif          // DUMP_DATA


//-------------------------------------------------------------------------
// Function Name: free_pdu
// Description  : Release memory hold by pdu
// Return Value : NONE
//-------------------------------------------------------------------------
int   free_pdu(snmp_pdu *pdu)
{
    variable_list       *ovp, *vp;

    if (pdu == NULL)
            return  0;
    vp = pdu->variables ;
    while (vp) {
            ovp = vp;
            vp  = vp->next;
            if (ovp->name != NULL)
                    free (ovp->name) ;
            if (ovp->val.string != NULL)
                    free (ovp->val.string) ;
            free (ovp) ;
    }
//    if (pdu->enterprise != NULL )
//          free (pdu->enterprise) ;
    free (pdu) ;
    return 1;
}

//*
//*---------------------------------------------------------------------------------
//* Function Name: free_request_list
//* Description : Release outstanding pdus (snmp messages)
//* Return Value : NONE
//* Calling To  : free_pdu
//* Called by   : snmp_close
//*---------------------------------------------------------------------------------
//*
int free_request_list(request_list      *req)
{
    free_pdu (req->pdu) ;
    if (req) {
            free (req) ;
            req = NULL ;
    }
    return 0 ;
}

//*
//*---------------------------------------------------------------------------------
//* Function Name: udp_read
//* Description  : Get message from an UDP port which is specified by a session and
//*        prepares a NULL pdu for it. If defines DUMP_DATA, the content of
//*        received message will be shown.
//* Return Value : pointer of new pdu
//* Calling To   : dump1
//* Called By    : snmp_read
//*---------------------------------------------------------------------------------
//*
int udp_read(snmp_session  *session, uint8 *data,int *length)
{
        ipaddr    *from = &session->r_addr;
    int        from_len ;

        from_len = sizeof (*from) ;
        *length = recvfrom(session->sfd,(void *)data,*length,0,(struct sockaddr *)from,&from_len);
        if(*length < 0) return(0);

        return (1);
}

//*
//*---------------------------------------------------------------------------------
//* Function Name: udp_send
//* Description  : Send out message through an UDP port. If defines DUMP_DATA, the
//*        contents of message will be shown.
//* Return Value : FALSE if fails to send, TRUE if sends it success
//* Calling To   : dump1
//* Called By    : snmp_send
//*---------------------------------------------------------------------------------
//*
int     snmp_udp_send(snmp_session *session, uint8 *data,int length)
{
        if(sendto(session->sfd,(void *)data,length,0,(struct sockaddr *)&session->r_addr,sizeof(session->r_addr)) < 0)
                return(FALSE);
    return(TRUE);
}

//*
//*---------------------------------------------------------------------------------
//* Function Name: snmp_parse_auth
//* Description  : Get the community name and version number by parsing snmp
//*        message. The message is decoded in BER deconding rules.
//* Return Value : Community Name and its length, version number and new pointer to
//*        next snmp fields
//* Calling To   : asn_parse_header, asn_parse_int, asn_parse_string
//* Called By    : snmp_parse
//*---------------------------------------------------------------------------------
//*
uint8           *snmp_parse_auth(
        uint8   *data,
        int     *length,
        uint8   *community,
        int         *community_len,
        long    *version
)
{
        uint8           type;

        data = asn_parse_header(data, length, &type);
        if (data == NULL) {
                return (NULL);   //------SNMP: Parse authority failure.
    }
        if (type != (ASN_SEQUENCE | ASN_CONSTRUCTOR))
                return (NULL);  //------SNMP: Wrong auth header type.

        data = asn_parse_int(data, length, &type, version, sizeof(*version));
        if (data == NULL)
                return (NULL);  //------SNMP: Bad parse of version.

        data = asn_parse_string(data, length, &type, community, community_len);
        if (data == NULL)
                return (NULL);  //------SNMP: Bad parse of community.

        community[*community_len] = '\0';
        return (data);
}

//*
//*---------------------------------------------------------------------------------
//* Function Name: snmp_build_auth
//* Description  : Encodes the community name and version number in BER encoding
//*        rules.
//* Return Value : pointer of message with community name and version number encoded
//*        in it.
//*---------------------------------------------------------------------------------
//*
uint8 *snmp_build_auth(
uint8 *outdata,
int   *outlength,
uint8 *community,
int       community_len,
long  version,
int       messagelen
)
{
        // build the the message wrapper (30 82 Length)
    // 5 bytes = version header + version value + community header

// add -- By arius 5/17/2000
#ifdef WEBADMIN
   uint8   *oldoutdata;
   int     newoutlength;

   oldoutdata = outdata;
   outdata = asn_build_sequence(outdata, outlength, (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
          0);
#else
        outdata = asn_build_sequence(outdata, outlength, (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                      messagelen + community_len + 5);
#endif WEBADMIN
// end of here

        if (outdata == NULL){
                ERROR("buildheader");
                return NULL;
        }

        // store the version field (02 length version)
        outdata = asn_build_int(outdata, outlength,
                (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                &version, sizeof(version));
        if(outdata == NULL){
                ERROR("buildint");
                return NULL;
        }

        outdata = asn_build_string(outdata, outlength,
            (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STRING),
            community, community_len);
        if(outdata == NULL){
                ERROR("buildstring");
                return NULL;
        }

// add -- By arius 5/17/2000
#ifdef WEBADMIN
   if (messagelen)
   {
     newoutlength = *outlength + (community_len + 5 + 4);  // (default 4) include 30 82 Length
     outdata = asn_build_sequence(oldoutdata, &newoutlength , (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
             messagelen + community_len + 5);
   }
#endif WEBADMIN
// end of here

        return outdata;
}

#if 0
//*
//*---------------------------------------------------------------------------------
//* Function Name: snmp_parse_var
//* Description  : Get variables by parsing snmp message. The message is decoded in
//*        BER deconding rules.
//* Return Value : Pointer of decoding variables
//* Calling To   : asn_parse_header, asn_parse_objid, asn_parse_int,
//*        asn_parse_string
//* Called By    : snmp_parse
//*---------------------------------------------------------------------------------
//*
variable_list   *snmp_parse_var(uint8 *data,int length)
{
    uint8                       type;
    variable_list               vh, *vp;
    uint8                       *varBindP;
    int                         varBind_len;
    uint8                       objId[SNMP_MAX_NAME_LEN];
    uint8                       *varVal;
    int                         parse_ok;

    data = asn_parse_header(data, &length, &type);
    if ((data == NULL) || (type != (uint8) (ASN_SEQUENCE | ASN_CONSTRUCTOR)))
        return ( NULL );

    vh.next  = NULL;
    vp       = &vh;
    varBindP = data;
    parse_ok = TRUE;

    while (length > 0) {
        varBind_len = length;

        vp->next = (variable_list *) malloc(sizeof(variable_list));
        if (vp->next == NULL) {
            parse_ok = FALSE;
            break;      //---SNMP: Memory allocation error on 'snmp_parse_var'.
        }
        else {
            vp             = vp->next;
            vp->next       = NULL;
            vp->name       = NULL;
            vp->name_len   = SNMP_MAX_NAME_LEN;
            vp->val.string = NULL;
        }

        varBindP = asn_parse_header(varBindP, &varBind_len, &type);
        if ((varBindP == NULL) ||
            (type != (uint8) (ASN_SEQUENCE | ASN_CONSTRUCTOR))) {
            parse_ok = FALSE;
            break;
        }

        //*
        //*  Parse variable bind name.
        //*
        varBindP = asn_parse_objid(varBindP, &varBind_len, &type, objId, &vp->name_len);
        if ((varBindP == NULL) ||
            (type != (uint8) (ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID))) {
            parse_ok = FALSE;
            break;
        }

        vp->name = (oid *) malloc((unsigned) vp->name_len * sizeof(oid));
        if (vp->name == NULL) {
            parse_ok = FALSE;
            break;      //-----SNMP: Memory alloc error on 'snmp_parse_var'.
        }
        else {
            bcopy((char *) objId, (char *) vp->name, vp->name_len * sizeof(oid));
        }

        //*
        //*  Parse variable bind value.
        //*
        varVal   = varBindP;
        varBindP = asn_parse_header(varBindP, &varBind_len, &vp->type);
        if (varBindP == NULL) {
            parse_ok = FALSE;
            break;
        }
        else {
            vp->val_len = varBind_len;
            varBindP    = varBindP + varBind_len;  // skip value body to next variable
            length      = length - (int)(varBindP - data);
            data        = varBindP;
            varBind_len = SNMP_MAX_LEN;
        }

        switch (vp->type) {
            case ASN_INTEGER:
            case COUNTER:
            case GAUGE:
            case TIMETICKS:
                 vp->val_len     = sizeof(long);   // support long intger only
                 vp->val.integer = (long *) malloc(vp->val_len);
                 if (vp->val.integer == NULL) {
                        parse_ok = FALSE;
                     break;  //---SNMP:Memory alloc error on 'snmp_parse_var'.
                 }
                 asn_parse_int(varVal, &varBind_len, &vp->type,
                           (long *) vp->val.integer, vp->val_len);
                 break;
            case ASN_OCTET_STRING:
            case IPADDRESS:
            case OPAQUE:
                 vp->val.string = (uint8 *) malloc(vp->val_len);
                 if (vp->val.string == NULL) {
                        parse_ok = FALSE;
                        break;  //----SNMP:Memory alloc error on 'snmp_parse_var'.
                 }
                 asn_parse_string(varVal, &varBind_len, &vp->type,
                           vp->val.string, &vp->val_len);
                 break;
            case ASN_OBJECT_ID:
                 vp->val_len = SNMP_MAX_NAME_LEN;
                 asn_parse_objid(varVal, &varBind_len, &vp->type, objId, &vp->val_len);
                 vp->val_len = vp->val_len * sizeof(oid);
                 vp->val.objid = (oid *) malloc((unsigned) vp->val_len);
                 if (vp->val.objid == NULL) {
                        parse_ok = FALSE;
                     break;   //---SNMP:Memory alloc error on 'snmp_parse_var'.
                 }
                 bcopy((char *) objId, (char *) vp->val.objid, vp->val_len);
                 break;
            case ASN_NULL:
                 break;
            default:
                 parse_ok = FALSE;  //-----Warning: Bad type received.
                 break;
        }

        if ( ! parse_ok ) break;
    }

    if ( parse_ok ) {
        return( vh.next );
    }
    else {
        while ( vh.next ) {
            vp      = vh.next;
            vh.next = vp->next;
            if (vp->name != NULL) free((char *) vp->name);
            if (vp->val.string != NULL) free((char *) vp->val.string);
            free((char *) vp);
        }
        return ( NULL );
    }
}
#endif 0

//*
//*---------------------------------------------------------------------------------
//* Function Name: snmp_build_var
//* Description  : Encodes variables in BER encoding rules.
//* Return Value : pointer of message with variables encoded in it.
//* Calling To   : asn_build_objid, asn_build_int, asn_build_string, asn_build_null,
//*        asn_build_header
//* Called By    : snmp_build
//*---------------------------------------------------------------------------------
//*
/******************
uint8           *snmp_build_var(data, vp, length)
register uint8 *data;
variable_list   *vp;
register int    *length;
{
        uint8           buf[SNMP_MAX_LEN];
    int                 total_len;
    uint8               *cp;
    int                 len = SNMP_MAX_LEN;

    cp = asn_build_objid(buf, &len,
                        (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID),
                        vp->name, vp->name_len);
    if (cp == NULL) return(NULL);

    switch (vp->type) {
        case ASN_INTEGER:
        case GAUGE:
        case COUNTER:
        case TIMETICKS:
                cp = asn_build_int(cp, &len, vp->type,
                        (long *) vp->val.integer, vp->val_len);
             break;
        case ASN_OCTET_STRING:
        case IPADDRESS:
        case OPAQUE:
                cp = asn_build_string(cp, &len, vp->type,
                        (uint8 *) vp->val.string, vp->val_len);
                break;
        case ASN_OBJECT_ID:
                cp = asn_build_objid(cp, &len, vp->type,
                        (oid *) vp->val.objid, vp->val_len / sizeof(oid));
                break;
        case ASN_NULL:
             cp = asn_build_null(cp, &len, vp->type);
             break;
        default:
             return(NULL);
    }
    if (cp == NULL) return(NULL);
    total_len = (int) (cp - buf);
    cp = asn_build_header(data, length,
                        (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), total_len);
    if ((*length) < total_len) return(NULL);
    bcopy((char *) buf, (char *) cp, total_len);
    cp = cp +  total_len;
    *length = *length - (int)(cp - data) ;
    return(cp);
}
************************************/

/*
 * Takes a session and a pdu and serializes the ASN PDU into the area
 * pointed to by packet.  out_length is the size of the data area available.
 * Returns the length of the completed packet in out_length.  If any errors
 * occur, -1 is returned.  If all goes well, 0 is returned.
 */
int snmp_build_trap(
        snmp_session *session,
        snmp_pdu *pdu,
        uint8  *packet,
        int         *out_length
)
{
        uint8  buf[SNMP_MAX_TRAP_LEN];
        uint8  *cp;
#ifdef WEBADMIN
        uint8  *newpacket;   // add -- by arius 5/18/2000
#endif WEBADMIN
        variable_list *vp;
        int         totallength;
        int         length;

        length = *out_length;
        cp = packet;
        for(vp = pdu->variables; vp; vp = vp->next){
                cp = snmp_build_var_op(cp, vp->name, &vp->name_len, vp->type, vp->val_len, (uint8 *)vp->val.string, &length);
                if (cp == NULL) return (FALSE);
        }
        totallength = cp - packet;

        length = SNMP_MAX_TRAP_LEN;
        cp = asn_build_header(buf, &length, (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), totallength);
        if (cp == NULL) return (FALSE);
        bcopy((char *)packet, (char *)cp, totallength);
        totallength += cp - buf;

        length = *out_length;
        if (pdu->command != TRP_REQ_MSG){
                // request id
                cp = asn_build_int(packet, &length,
                       (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                       (long *)&pdu->reqid, sizeof(pdu->reqid));
                if (cp == NULL) return (FALSE);
                // error status
                cp = asn_build_int(cp, &length,
                       (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                       (long *)&pdu->errstatus, sizeof(pdu->errstatus));
                if(cp == NULL) return (FALSE);
                // error index
                cp = asn_build_int(cp, &length,
                       (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
               (long *)&pdu->errindex, sizeof(pdu->errindex));
                if(cp == NULL) return (FALSE);
        } else {        // this is a trap message
                // enterprise
                cp = asn_build_objid(packet, &length,
                       (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID),
                       (oid *)SnmpVersionID, SNMP_VER_LEN);
                if (cp == NULL) return (FALSE);
                // agent-addr
                cp = asn_build_string(cp, &length, (uint8)IPADDRESS,
                       (uint8 *)&pdu->agent_addr.sin_addr.s_addr, sizeof(pdu->agent_addr.sin_addr.s_addr));
                if(cp == NULL) return (FALSE);
                // generic trap
                cp = asn_build_int(cp, &length,
                       (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                       (long *)&pdu->generic_trap, sizeof(pdu->generic_trap));
                if (cp == NULL) return (FALSE);
                // specific trap
                cp = asn_build_int(cp, &length,
                       (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                       (long *)&pdu->specific_trap, sizeof(pdu->specific_trap));
                if (cp == NULL) return (FALSE);
                // timestamp
                cp = asn_build_int(cp, &length, (uint8)TIMETICKS,
                       (long *)&pdu->time_stamp, sizeof(pdu->time_stamp));
                if (cp == NULL) return (FALSE);
        }
        if (length < totallength)       return (FALSE);
        bcopy((char *)buf, (char *)cp, totallength);
        totallength += cp - packet;

        length = SNMP_MAX_TRAP_LEN;
        cp = asn_build_header(buf, &length, (uint8)pdu->command, totallength);
        if (cp == NULL) return (FALSE);
        if (length < totallength) return (FALSE);

        bcopy((char *)packet, (char *)cp, totallength);
        totallength += cp - buf;

        length = *out_length;
// modified -- by arius 5/18/2000
#ifdef WEBADMIN
newpacket = packet;
        cp = snmp_build_auth( newpacket, &length, session->community,
                session->community_len,(long) SNMP_VERSION_1, totallength);
cp  -= totallength;   // save new pointer
#else
        cp = snmp_build_auth( packet, &length, session->community,
                session->community_len,(long) SNMP_VERSION_1, totallength);
#endif WEBADMIN
// end of here
        if (cp == NULL) return (FALSE);

        if ((*out_length - (cp - packet)) < totallength) return (FALSE);

        bcopy((char *)buf, (char *)cp, totallength);
        totallength += cp - packet;
        *out_length = totallength;

//      if( session->qoS & USEC_QOS_AUTH )
//              md5Digest( packet, totallength, cp - (session->contextLen + 16),
//              cp - (session->contextLen + 16) );

        return (TRUE);
}

//*--##############################################################################
//*--
//*--                       SNMP primitive APIs
//*--
//*--##############################################################################



//---------------------------------------------------------------------------------
// Function Name: snmp_send
// Description  : Sends the input pdu on the session after calling snmp_build to
//                create a serialized packet. If necessary, set some of the pdu
//                data from the session defaults. Add a request corresponding to
//                this pdu to the list of outstanding requests on this session,
//                then send the pdu.
// Return Value : FALSE - on any error
//                TRUE - otherwise
// Calling To   : snmp_build, udp_send and free_pdu
// Called By    : user
//---------------------------------------------------------------------------------

int             snmp_send_trap(snmp_session     *session, snmp_pdu      *pdu)
{
        uint8 data[SNMP_MAX_TRAP_LEN];
        request_list  *rp ;
        int       length = SNMP_MAX_TRAP_LEN;

        if (session == NULL)
        return(FALSE);   //------SNMP: Send message on NULL Session.-----------

    if (pdu == NULL)
        return(FALSE);   //------SNMP: Intend to send a null pdu.--------------

    if (pdu->r_addr.sin_addr.s_addr == SNMP_DEFAULT_ADDRESS) {
        if (session->r_addr.sin_addr.s_addr != SNMP_DEFAULT_ADDRESS)
              bcopy((char *)&session->r_addr,(char *)&pdu->r_addr,sizeof(pdu->r_addr));
        else
              return(FALSE);  //-------SNMP: Without specify remote ip address.
    }
    switch (pdu->command) {
        case SNMP_GET:
             snmpOutGetRequests++ ;
             break ;
        case SNMP_GET_NEXT:
             snmpOutGetNexts++ ;
                break ;
        case SNMP_SET:
                snmpOutSetRequests++ ;
             break;
        case SNMP_GET_RESPONSE:
             snmpOutGetResponses++ ;
             break;
        case SNMP_TRAP:
             snmpOutTraps++ ;
             break ;
        default:
             break ;
    }
    switch (pdu->command) {
        case SNMP_GET:
        case SNMP_GET_NEXT:
        case SNMP_SET:
                if (pdu->reqid == 0)
                         pdu->reqid     = ++Reqid ;
//------------- pdu->errstatus = 0 ; -------------------------------------
//------------- pdu->errindex  = 0 ; -------------------------------------
                rp = (request_list *)calloc(1,sizeof(request_list)) ;
                if (rp == NULL)
                 return(FALSE) ;
                session->requests = rp ;
                rp->pdu           = pdu ;
                rp->reqid         = pdu->reqid ;
//              rp->retries       = session->retries ;
//              rp->timeout       = session->timeout ;
                break;
        case SNMP_GET_RESPONSE:
                break;
        case SNMP_TRAP:
                pdu->reqid = 1; // trap do not need request id
                break;
        default:
                return(FALSE);  //-----------SNMP: Unknown message type.-------
    }
        if(!snmp_build_trap(session, pdu, data, &length))
                return(FALSE);  //------SNMP: Fail to build snmp message.------
        snmpOutPkts++ ;
        if (!snmp_udp_send(session, data, length))
                return(FALSE);  //------SNMP: Fail to send snmp message.-------
        if (pdu->command == SNMP_GET_RESPONSE || pdu->command == SNMP_TRAP)
                free_pdu (pdu) ;
    return(TRUE);
}

#if 0
//---------------------------------------------------------------------------------
// Function Name: snmp_close
// Description  : Close the input session. Free all data allocated for the session,
//                dequeue any pending requests, and close socket allocated for the
//                session.
// Return Value :
//                1 - otherwise
// Calling To   : free_request_list
// Called By    : snmp_open and user
//---------------------------------------------------------------------------------
int             snmp_close(snmp_session *session)
{
    if (session == NULL)
         return(0);      //------SNMP: Intend to close a null session.

//    if (session->community != NULL)
//       free(session->community) ;
//    if (session->peername != NULL)
//       free(session->peername);
    if (session->sfd != NULL)
         close_s(session->sfd) ;
    if (session->requests != NULL)
         free_request_list(session->requests);
    return 1 ;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void xdump (char *cp,int length,char *prefix)
{
    int col, count;
    int tmp ;

    count = 0;
    while(count < length){
            printf("%s", prefix);
            for(col = 0;count + col < length && col < 16; col++){
                    if (col != 0 && (col % 4) == 0)
                            printf(" ");
                    tmp = (int)(cp[count+col]) & (int)(0x00FF) ;
                    printf("%02X ",tmp);
            }
            while(col++ < 16){  // pad end of buffer with zeros
                    if ((col % 4) == 0)
                            printf(" ");
                    printf("   ");
            }
            printf("  ");
            for(col = 0;count + col < length && col < 16; col++){
                    if (isprint(cp[count + col]))
                            printf("%c", cp[count + col]);
                    else
                            printf(".");
            }
            printf("\n");
            count += col;
    }
}
#endif


#ifdef WEBADMIN
//#include "psglobal.h"

#define PrnPortDisConnected(port) (RealPortStatus((port)) == 0x7F)

uint8   Snmp_Change_IP = 0;
uint8   Snmp_WriteDataTo_EEPROM = 0;
uint8   Snmp_Restart = 0;
//---------------------------------------------------------------------------------
// Function Name: snmp_ReadPrintStatus
// Description  :
// Return Value :
//                0: wait for job
//                1: paper out
//                2: off line
//                3: busy (is printing)
// Calling To   : ReadPrintStatus() on the 1284.c
// Called By    : var_npipx() on the snmpvars.c
//---------------------------------------------------------------------------------
extern uint8   adjPortType[NUM_OF_PRN_PORT];
BYTE snmp_ReadPrintStatus(int port)
{
  BYTE PrintStatus, ECHO_PORT1_STATUS = 0;

  if ( PrnPortDisConnected(port) )
    return PORT_OFF_LINE;
  else
  {
    PrintStatus = ReadPrintStatus();

    PrintStatus >>= ((port- ECHO_PORT1_STATUS) << 1);
    
    if( adjPortType[port] == PORT_PAPER_OUT )
    	return PORT_PAPER_OUT;
    	
    switch(PrintStatus & 0x03)
    {
      case PORT_READY:        // Wating for job
              return PORT_READY;
      case PORT_PAPER_OUT:    // Paper out
              return PORT_PAPER_OUT;
      case PORT_OFF_LINE:    // OFF Line
              return PORT_OFF_LINE;
      case PORT_PRINTING:    // Printing
              return PORT_PRINTING;
    }
    return PORT_OFF_LINE;
  }
}

//---------------------------------------------------------------------------------
// Function Name: snmp_ReadCardAndPeripheralStatus
// Description  :
// Return Value :
//                NULL
// Calling To   : snmp_ReadPrintStatus() on the snmp_api.c
// Called By    : var_npsys() on the snmpvars.c
//---------------------------------------------------------------------------------
void snmp_ReadCardAndPeripheralStatus(int port,char *buf)
{
  switch(snmp_ReadPrintStatus(port))
  {
    case PORT_READY:        // Wating for job
            strcpy(buf,"READY");
            break;
    case PORT_PAPER_OUT:    // Paper out
            strcpy(buf,"PAPER OUT");
            break;
    case PORT_OFF_LINE:    // OFF Line
            strcpy(buf,"OFF LINE");
            break;
    case PORT_PRINTING:    // Printing
            strcpy(buf,"PRINTING");
            break;
    default:
            buf[0] = 0x00;
  }
}

//---------------------------------------------------------------------------------
// Function Name: snmp_GetCfgSource
// Description  :
// Return Value :
//                3: manual-three: read/write, must write ip_addr in same packet
//                4: bootp-four: read/write, ignores other npCfg if in same packet
//                5: dhcp: read/write, ignores other npCfg if in same packet
// Calling To   :
// Called By    : var_npcfg() on the snmpvars.c
//---------------------------------------------------------------------------------
BYTE snmp_GetCfgSource(void)
{
  if (EEPROM_Data.PrintServerMode & PS_DHCP_ON)
    return 5;   // DHCP
  else
    return 3;   // Manual
}

//---------------------------------------------------------------------------------
// Function Name: snmp_SetCfgSource
// Description  :
// Return Value :
// Calling To   :
// Called By    : writeNpCardMib() on the snmpvars.c
//---------------------------------------------------------------------------------
void snmp_SetCfgSource(int value)
{
  switch(value)
  {
    case  5:  // enable
    case  4:
            EEPROM_Data.PrintServerMode |= PS_DHCP_ON;
            break;
    case  1:
    case  3:  // disbale first bit = 11111101
            EEPROM_Data.PrintServerMode &= ~PS_DHCP_ON;
            break;
  }
}

//---------------------------------------------------------------------------------
// Function Name: snmp_SetProtocol
// Description  :
// Return Value :
//               long value
// Calling To   :
// Called By    : var_npcfg() on the snmpvars.c
//---------------------------------------------------------------------------------
long snmp_SetProtocol(void)
{
   // bits 31-28: Number of I/O channels/protocols present
   // bits 27-24: Number of I/O channels which may operate currently
   // bits 23-16: unused, reserved
   // bits 15- 1: Individual I/O channel/protocl enable bits
   //    Novel bit 1
   //    LLC   bit 2
   //    TCP   bit 3
   //    ATALK bit 4
   return 1140850714;
}

//---------------------------------------------------------------------------------
// Function Name: snmp_CheckValueType
// Description  :
// Return Value :
//               int value
// Calling To   :
// Called By    : writeNpScanMib() on the snmpvars.c
//---------------------------------------------------------------------------------
int snmp_CheckValueType(uint8 var_type,int var_len)
{
   switch(var_type)
   {
     case STRING:
                if (var_len > SNMP_MAX_STRING)
                  return SNMP_ERR_WRONGLENGTH;
                break;
     case INTEGER:
                if (var_len > sizeof(long))
                  return SNMP_ERR_WRONGLENGTH;
                break;
     case IPADDRESS:
                if (var_len > 4)
                  return SNMP_ERR_WRONGLENGTH;
                break;
     case COUNTER:
                if (var_len > 4)
                  return SNMP_ERR_WRONGLENGTH;
                break;
     default:
         return SNMP_ERR_WRONGTYPE;
   }
   return SNMP_ERR_NOERROR;
}

//---------------------------------------------------------------------------------
// Function Name: ReadDataFromFrontPanel(buffer,Port);
// Description  :
// Return Value :
//                NULL
// Calling To   : PrnStatusInfo() on the 1284.c
// Called By    : var_devstatus() on the snmpvars.c
//---------------------------------------------------------------------------------
void ReadDataFromFrontPanel(char *buf,int Port)
{
//  PrnStatusInfo(Port, buf);
    snmp_ReadCardAndPeripheralStatus(Port,buf);
//  buf[0] = 0x00;
}

//---------------------------------------------------------------------------------
// Function Name: ReadPrintModelAndID(buffer,Port);
// Description  :
// Return Value :
//                NULL
// Calling To   : PrnDeviceInfo() on the 1284.c
// Called By    : var_devstatus() on the snmpvars.c
//---------------------------------------------------------------------------------
void ReadPrintModelAndID(char *buf,int Port)
{
#ifdef DEF_IEEE1284
        PrnDeviceInfo(Port, buf);
#else
        strcpy(buf,"MANUFACTURER:Hewlett-Packard;COMMAND SET:PJL,MLC,PCL,PCLXL,POSTSCRIPT;MODEL:HP LaserJet 4000 Series;CLASS:PRINTER; DESCRIPTION:HP LaserJet 4000 Series");
#endif
}

extern void UseEEPROMIP();
//---------------------------------------------------------------------------------
// Function Name: RunChangeIPAddress();
// Description  :
// Return Value :
//                NULL
// Calling To   :
// Called By    : snmp_read() on the snmpaget.c
//---------------------------------------------------------------------------------
void RunChangeIPAddress(void)
{
   if (Snmp_Change_IP)
   {
     UseEEPROMIP();
     Snmp_Change_IP = 0;
   }
}

//---------------------------------------------------------------------------------
// Function Name: RunWriteDataToEEPROM();
// Description  :
// Return Value :
//                NULL
// Calling To   :
// Called By    : snmp_read() on the snmpaget.c
//---------------------------------------------------------------------------------
void RunWriteDataToEEPROM(void)
{
#ifndef _PC
   if (Snmp_WriteDataTo_EEPROM)
   {
	 if(WriteToEEPROM(&EEPROM_Data) != 0) LightOnForever(Status_Lite); // Write EEPROM Data
     Snmp_WriteDataTo_EEPROM = 0;

     RunRestart();
   }
#endif
}

//---------------------------------------------------------------------------------
// Function Name: RunRestart();
// Description  :
// Return Value :
//                NULL
// Calling To   :
// Called By    : RunWriteDataToEEPROM() on the snmp_api.c
//---------------------------------------------------------------------------------
void RunRestart(void)
{
	if (Snmp_Restart)
	{
    	Snmp_Restart = 0;
     //ReStartCode2();   // call restart
//os	 BOOTSTRAP();
		Reset();
	}
}
#endif WEBADMIN
