/*
 * $Log$
 */

#include  <stdio.h>
#include  <string.h>
//#include  <alloc.h>
#include  <stdlib.h>
#include <cyg/kernel/kapi.h>    //615wu
#include "network.h"    //615wu
#include "pstarget.h"   //615wu
#include  "psglobal.h"
#include  "asn1.h"
#include "eeprom.h"		//615wu
#include  "snmp_api.h"
#include  "snmpaget.h"
#include  "snmpvars.h"
#include  "snmptrap.h"

snmp_session *snmp_open (snmp_session *session);
//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void	    AtSay (int,int,char *) ;
long	    bioscnt (void) ;
void	    ClearScreen (int, int, int, int) ;
char	    *UpTimeString (unsigned long, char *) ;
char	    *ObjectIdToAscii (char *, oid *, int) ;

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
snmp_session	trap_send_session = {
    NULL,           /* community name */
    6,				/* community name length */
//    SNMP_DEFAULT_RETRIES,	/* retries */
//    SNMP_DEFAULT_TIMEOUT,	/* how long to retry */
//    NULL,			/* peer host name */
	162,
    0,				/* snmp daemon port */
//    NULL,			/* authentication function call */
//    (void *)TrapCallBack,	/* call back routine */
	NULL,	        /* call back routine */
    NULL,			/* call back message that I want to sepecify */
    NULL,			// socket file description
    {NULL},			// peer socket_internet_address
    {NULL},			// request list
};

static BYTE SnmpTrapTargetCommunity[]= "public";

static BYTE InitTrapSession;

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
void SendTrap(int generic_trap, int specific_trap, char *description)
{
	variable_list  *var ;
	snmp_pdu	   *pdu ;
	int i,j;

	if(!InitTrapSession) {
		InitTrapSession = 1;
		snmp_open(&trap_send_session) ;
	}

	if(_SnmpTrapEnable) {
		for(i = 0; i < NUM_OF_SNMP_TRAP_IP; i++) {

			if((trap_send_session.r_addr.sin_addr.s_addr = NGET32(_SnmpTrapTargetIP[i])) == 0)
				continue;
			trap_send_session.community = SnmpTrapTargetCommunity;
			trap_send_session.community_len = strlen(SnmpTrapTargetCommunity);

			if( (pdu = (snmp_pdu *)calloc(1,sizeof(snmp_pdu))) == NULL) return;

			pdu->command	= SNMP_TRAP ;
//			pdu->enterprise_len = SNMP_DEFAULT_ENTERPRISE_LENGTH ;
		   	pdu->agent_addr.sin_addr.s_addr = NGET32(EEPROM_Data.BoxIPAddress);	//615wu
			pdu->generic_trap	= generic_trap ;
			pdu->specific_trap	= specific_trap ;
			pdu->time_stamp	= msclock() / 10;

			pdu->variables = var = (variable_list *)calloc(1,sizeof(variable_list)) ;
			var->next	      = NULL ;
			var->name_len     = SNMP_VER_LEN; //Simon

		    var->name	      = (void *)calloc(var->name_len,sizeof(oid));

			for(j = 0 ; j < var->name_len ; j++)
			   var->name[j] = SnmpVersionID[j]; 	     //Simon

		    var->type	      = ASN_OCTET_STRING ;
		    var->val.string   = (void *)calloc(strlen(description)+1,sizeof(char));
		    strcpy (var->val.string,description) ;
		    var->val_len      = strlen (description) ;

		    snmp_send_trap (&trap_send_session,pdu) ;
		}
	}
}
