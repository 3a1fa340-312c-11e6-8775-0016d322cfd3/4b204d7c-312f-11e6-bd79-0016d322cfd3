/*
 * Simple Network Management Protocol (RFC 1067).
 *
 */
/**********************************************************************
        Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/kernel/kapi.h>   //615wu
#include "network.h"    //6135wu
#include "pstarget.h"   //615wu
#include "psglobal.h"	//615wu
#include "eeprom.h"		//615wu
#include "asn1.h"
#include "snmp_api.h"
#include "snmp.h"

uint8 *
snmp_parse_var_op(
        uint8 *data,         // IN - pointer to the start of object
        oid    *var_name,     // OUT - object id of variable
        int        *var_name_len, // IN/OUT - length of variable name
        uint8 *var_val_type, // OUT - type of variable (int or octet string) (one byte)
        int        *var_val_len,  // OUT - length of variable
        uint8 **var_val,          // OUT - pointer to ASN1 encoded value of variable
        int        *listlength    // IN/OUT - number of valid bytes left in var_op_list
)
{
        uint8       var_op_type;
        int                 var_op_len = *listlength;
        uint8       *var_op_start = data;

        //parse Name:Value pair header (ex: 30,82, XX, XX, Name , value)
        data = asn_parse_header(data, &var_op_len, &var_op_type);
        if (data == NULL){
                ERROR("snmp_parse_var_op(): 1 asn_parse_header() == NULL");
                return NULL;
        }

        if (var_op_type != (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR)) return NULL;

    //Parse Name:Vale
        data = asn_parse_objid(data, &var_op_len, &var_op_type, var_name, var_name_len);
        if (data == NULL){
                ERROR("snmp_parse_var_op(): asn_parse_objid() == NULL");
                return NULL;
        }

    if (var_op_type != (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID))
                return NULL;

        *var_val = data;        /* save pointer to this object */
    /* find out what type of object this is */
    data = asn_parse_header(data, &var_op_len, var_val_type);
        if (data == NULL){
                ERROR("snmp_parse_var_op(): 2 asn_parse_header() == NULL");
                return NULL;
        }
        *var_val_len = var_op_len;
        data += var_op_len;
        *listlength -= (int)(data - var_op_start);
        return data;
}

uint8 *
snmp_build_var_op(
        uint8   *data,        //IN - pointer to the beginning of the output buffer
        oid             *var_name,    //IN - object id of variable
        int             *var_name_len,//IN - length of object id
        uint8   var_val_type, //IN - type of variable
        int             var_val_len,  //IN - length of variable
        uint8   *var_val,         //IN - value of variable
        int             *listlength       //IN/OUT - number of valid bytes left in output buffer
)
{
        int                 dummyLen, headerLen;
        uint8       *dataPtr;
#ifdef WEBADMIN
        static int     length1;  // add -- by arius 5/17/2000
        uint8   *newdata; // add -- by arius 5/17/2000
#endif WEBADMIN

        dummyLen = *listlength;
        dataPtr = data;
#if 0
    data = asn_build_sequence(data, &dummyLen,
                              (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 0);
    if (data == NULL){
        ERROR("");
        return NULL;
    }
#endif
        data += 4;
        dummyLen -=4;
        if (dummyLen < 0) return NULL;

        headerLen = data - dataPtr;
        *listlength -= headerLen;
        data = asn_build_objid(data, listlength,
                  (uint8)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID),
                  var_name, *var_name_len);
        if (data == NULL){
                ERROR("");
                return NULL;
        }
    switch(var_val_type){
        case ASN_INTEGER:
            data = asn_build_int(data, listlength, var_val_type,
                    (long *)var_val, var_val_len);
            break;
        case GAUGE:
        case COUNTER:
        case TIMETICKS:
        case UINTEGER:
            data = asn_build_unsigned_int(data, listlength, var_val_type,
                                          (uint32 *)var_val, var_val_len);
            break;
#if 0
        case COUNTER64:
            data = asn_build_unsigned_int64(data, listlength, var_val_type,
                                           (struct counter64 *)var_val,
                                            var_val_len);
            break;
#endif 0
        case ASN_OCTET_STRING:
        case IPADDRESS:
        case OPAQUE:
        case NSAP:
            data = asn_build_string(data, listlength, var_val_type,
                    var_val, var_val_len);
            break;
        case ASN_OBJECT_ID:
            data = asn_build_objid(data, listlength, var_val_type,
                    (oid *)var_val, var_val_len / sizeof(oid));
            break;
        case ASN_NULL:
            data = asn_build_null(data, listlength, var_val_type);
            break;
        case ASN_BIT_STRING:
            data = asn_build_bitstring(data, listlength, var_val_type,
                    var_val, var_val_len);
            break;
        case SNMP_NOSUCHOBJECT:
        case SNMP_NOSUCHINSTANCE:
        case SNMP_ENDOFMIBVIEW:
            data = asn_build_null(data, listlength, var_val_type);
            break;
        default:
            ERROR("wrong type");
            return NULL;
    }
    if (data == NULL){
        ERROR("");
        return NULL;
    }
    dummyLen = (data - dataPtr) - headerLen;
// modified by ---- Arius 5/17/2000
#ifdef WEBADMIN
length1 = (data - dataPtr);
newdata = asn_build_sequence(dataPtr, &length1,
                       (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), dummyLen);
*listlength += (data - newdata);
data = newdata;
#else
    asn_build_sequence(dataPtr, &dummyLen,
                       (uint8)(ASN_SEQUENCE | ASN_CONSTRUCTOR), dummyLen);
#endif WEBADMIN
// end of here
    return data;
}
