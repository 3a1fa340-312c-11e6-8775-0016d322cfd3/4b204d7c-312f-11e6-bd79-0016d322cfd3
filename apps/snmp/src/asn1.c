/*
 * $Log$
 */
/*****************************************************************
 * asn1.c : a modification version of CMU's SNMP develop kit
 *****************************************************************/

#include      <string.h>
#include <cyg/kernel/kapi.h>   //615wu
#include "network.h"    //615wu
#include "pstarget.h"   //615wu
#include "psglobal.h"	//615wu
#include      "asn1.h"
#include      "snmp.h"
#include      "snmpgrup.h"

//----------------------------------------------------------------
// Abstract Syntax Notation One, ASN.1
// As defined in ISO/IS 8824 and ISO/IS 8825
// This implements a subset of the above International Standards
//      that is sufficient to implement SNMP.
//
// Encodes abstract data types into a machine independent stream
//      of bytes.
//
// Decodes abstract data types from a machine independent stream
//      of bytes.
//----------------------------------------------------------------

#include        <stdio.h>

//=============================================================================
//
//=============================================================================


//---------------------------------------------------------------------------------
// asn_parse_int - pulls a long out of an ASN int type.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the end of this object.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_parse_int(data, datalength, type, intp, intsize)
register uint8 *data;           /* IN     - pointer to start of object */
register int    *datalength;    /* IN/OUT - number of valid bytes left in buffer */
uint8           *type;          /* OUT    - asn type of object */
long            *intp;          /* IN/OUT - pointer to start of output buffer */
int             intsize;        /* IN     - size of output buffer */
{
    /*
        *       ASN.1 integer ::= 0x02 asnlength byte {byte}*
        */
        register uint8  *bufp = data;
        uint32          asn_length;
        register long   value = 0;

        if (intsize != sizeof (long)) {
                //    printf ("Not long   ") ;
                return NULL;    //----not long.
        }
        *type = *bufp++;
        bufp  = asn_parse_length(bufp, &asn_length);
        if (bufp == NULL) {
                //   printf ("Bad length   ") ;
                return NULL; //------bad length.
        }

        if ( (int)(asn_length+(long)(bufp-data)) > *datalength ) {
                snmpInASNParseErrs++ ;
                //      printf ("Overflow message") ;
                return NULL;       //------overflow of message.
        }

        if ((int)asn_length > intsize) {
                snmpInASNParseErrs++ ;
                //      printf ("Large integer   ") ;
                return NULL;    //------I don't support such large integers.
        }

        *datalength -= (int)asn_length + (int)(bufp - data);
        if (*bufp & (char)0x80) value = -1; /* integer is negative */
        while(asn_length--) value = (value << 8) | (long)(*bufp++);
        *intp = value;
        return bufp;
}


//---------------------------------------------------------------------------------
// asn_build_int - builds an ASN object containing an integer.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the end of this object.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_build_int(
register uint8 *data,           /* IN     - pointer to start of output buffer */
register int    *datalength,    /* IN/OUT - number of valid bytes left in buffer */
uint8           type,           /* IN     - asn type of object */
register long   *intp,          /* IN     - pointer to start of long integer */
register int    intsize)        /* IN     - size of *intp */
{
    /*
        * ASN.1 integer ::= 0x02 asnlength byte {byte}*
        */

    register long       integer;
    register uint32     mask;

    if (intsize != sizeof (long)) return NULL;
    integer = *intp;

    /*
        * Truncate "unnecessary" bytes off of the most significant end of this 2's
        * complement integer.
        * There should be no sequence of 9 consecutive 1's or 0's at the most
        * significant end of the integer.
        */
    mask = (uint32)0x1FFL << ((8 * (sizeof(long) - 1)) - 1);
    /* mask is 0xFF800000 on a big-endian machine */

    while((((integer & mask) == 0) || ((integer & mask) == mask)) && intsize > 1) {
        intsize--;
        integer <<= 8;
    }

    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL) return NULL;
    if (*datalength < intsize) return NULL;
    *datalength -= intsize;

    mask = (uint32)0xFFL << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--) {
        *data++ = (uint8)((integer & mask) >> (8 * (sizeof(long) - 1)));
        integer <<= 8;
    }
    return data;
}

/*
 * asn_build_unsigned_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
uint8 *
asn_build_unsigned_int(
    uint8  *data,       /* IN - pointer to start of output buffer */
    int    *datalength, /* IN/OUT - number of valid bytes left in buffer */
    uint8  type,        /* IN - asn type of object */
    uint32 *intp,       /* IN - pointer to start of long integer */
    int    intsize)     /* IN - size of *intp */
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */

    uint32 integer;
    uint32 mask;
    int add_null_byte = 0;

    if (intsize != sizeof (long)) {
        ERROR("not long");
        return NULL;
    }
    integer = *intp;
    mask = (uint32)0xFF << (8 * (sizeof(int32) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    if ((uint8)((integer & mask) >> (8 * (sizeof(int32) - 1))) & 0x80){
        /* if MSB is set */
        add_null_byte = 1;
        intsize++;
    }
    /*
     * Truncate "unnecessary" bytes off of the most significant end of this 2's complement integer.
     * There should be no sequence of 9 consecutive 1's or 0's at the most significant end of the
     * integer.
     */
    mask = (uint32) 0x1FF << ((8 * (sizeof(int32) - 1)) - 1);
    /* mask is 0xFF800000 on a big-endian machine */
    while((((integer & mask) == 0) || ((integer & mask) == mask)) && intsize > 1){
        intsize--;
        integer <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL)
        return NULL;
    if (*datalength < intsize)
        return NULL;
    *datalength -= intsize;
    if (add_null_byte == 1){
        *data++ = '\0';
        intsize--;
    }
    mask = (uint32) 0xFF << (8 * (sizeof(int32) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--){
        *data++ = (uint8)((integer & mask) >> (8 * (sizeof(int32) - 1)));
        integer <<= 8;
    }
    return data;
}



//---------------------------------------------------------------------------------
// asn_parse_string - pulls an octet string out of an ASN octet string type.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the beginning of the next object.
//
//  "string" is filled with the octet string.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_parse_string(data, datalength, type, string, strlength)
uint8           *data;          /* IN     - pointer to start of object */
register int    *datalength;    /* IN/OUT - number of valid bytes left in buffer */
uint8           *type;          /* OUT    - asn type of object */
uint8           *string;        /* IN/OUT - pointer to start of output buffer */
register int    *strlength;     /* IN/OUT - size of output buffer */
{
    /*
        * ASN.1 octet string ::= primstring | cmpdstring
        * primstring ::= 0x04 asnlength byte {byte}*
     * cmpdstring ::= 0x24 asnlength string {string}*
     * This doesn't yet support the compound string.
     */
    register uint8      *bufp = data;
    uint32              asn_length;

    *type = *bufp++;
    bufp  = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
        return (NULL) ;

    if (asn_length + (bufp - data) > *datalength) {
        snmpInASNParseErrs++ ;
        return NULL;    //------overflow of message.
    }

    if (asn_length > *strlength) {
        snmpInASNParseErrs++ ;
        return NULL;    //------I don't support such long strings.
    }

    bcopy((char *)bufp, (char *)string, (int)asn_length);
    *strlength   = (int)asn_length;
    *datalength -= (int)asn_length + (int)(bufp - data);
    return ((uint8 *)((uint32)bufp+asn_length)) ;
}


//---------------------------------------------------------------------------------
// asn_build_string - Builds an ASN octet string object containing the input string.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the beginning of the next object.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_build_string(
uint8           *data,          /* IN     - pointer to start of object */
register int    *datalength,    /* IN/OUT - number of valid bytes left in buffer */
uint8           type,           /* IN     - ASN type of string */
uint8           *string,        /* IN     - pointer to start of input buffer */
register int    strlength)      /* IN     - size of input buffer */
{
    /*
     * ASN.1 octet string ::= primstring | cmpdstring
     * primstring ::= 0x04 asnlength byte {byte}*
        * cmpdstring ::= 0x24 asnlength string {string}*
     * This code will never send a compound string.
     */

    data = asn_build_header(data, datalength, type, strlength);
    if (data == NULL) return NULL;
    if (*datalength < strlength) return NULL;
    bcopy((char *)string, (char *)data, strlength);
    *datalength -= strlength;
    return (data + strlength);
}


//---------------------------------------------------------------------------------
// asn_parse_header - interprets the ID and length of the current object.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   in this object following the id and length.
//
//  Returns a pointer to the first byte of the contents of this object.
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_parse_header(data, datalength, type)
uint8           *data;          /* IN     - pointer to start of object */
int             *datalength;    /* IN/OUT - number of valid bytes left in buffer */
uint8           *type;          /* OUT    - ASN type of object */
{
    register uint8      *bufp = data;
    register            header_len;
    uint32              asn_length;

    /* this only works on data types < 30, i.e. no extension octets */
    if (IS_EXTENSION_ID(*bufp)) {
        snmpInASNParseErrs++ ;
        return NULL;    //------can't process ID >= 30.
    }

    *type = *bufp++;
    bufp  = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
        return (NULL) ;

    header_len = (int)(bufp - data) ;
    if (header_len + asn_length > *datalength) {
        snmpInASNParseErrs++ ;
        return NULL;    //------asn length too long.
    }
    *datalength = (int)asn_length;
    return bufp;
}


//---------------------------------------------------------------------------------
// asn_build_header - builds an ASN header for an object with the ID and
// length specified.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   in this object following the id and length.
//
//  This only works on data types < 30, i.e. no extension octets.
//  The maximum length is 0xFFFF;
//
//  Returns a pointer to the first byte of the contents of this object.
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_build_header(
register uint8 *data,           /* IN     - pointer to start of object */
int             *datalength,    /* IN/OUT - number of valid bytes left in buffer */
uint8           type,           /* IN     - ASN type of object */
int             length)         /* IN     - length of object */
{
    if (*datalength < 1) return NULL;
    *data++ = type;
    (*datalength)--;
    return asn_build_length(data, datalength, length);

}

/*
 * asn_build_sequence - builds an ASN header for a sequence with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
uint8 *
asn_build_sequence(
        uint8  *data,       /* IN - pointer to start of object */
        int    *datalength, /* IN/OUT - number of valid bytes left in buffer */
        uint8  type,        /* IN - ASN type of object */
        int    length      /* IN - length of object */
)
{
#ifdef WEBADMIN
// modifield by ---- Arius 5/17/2000
int shiftBytes = 0;
int index;

if (!length)
{
// length 0 means that we will not do anything.
// Using original function
#endif WEBADMIN
        if(*datalength < 4) return NULL;

    *datalength -= 4;

    *data++ = type;
    *data++ = (uint8)(0x02 | ASN_LONG_LENGTH);
    *data++ = (uint8)((length >> 8) & 0xFF);
    *data++ = (uint8)(length & 0xFF);
    return data;
#ifdef WEBADMIN
}
else
{ // length > 0 means that we need to adjust spaces.

  *data++ = type;
  /* no indefinite lengths sent */
  if (length < (int)0x80)
  {
    // shift two bytes.    Data format---> "30 length"
    if(*datalength < 2) return NULL;
    *datalength -= 2;
    shiftBytes = 2;
    *data++ = (uint8)length;
  }
  else if (length <= (int)0xFF)
       {
         // shift one byte.    Data format---> "30 81 length"
         if(*datalength < 3) return NULL;
         *datalength -= 3;
         shiftBytes = 1;
         *data++ = (uint8)((uint8)0x01 | (uint8)ASN_LONG_LENGTH);
         *data++ = (uint8)length;
       }
       else
       { /* 0xFF < length <= 0xFFFF */
         // shift zero byte.    Data format---> "30 82 00 length"
         if(*datalength < 4) return NULL;
         *datalength -= 4;
         shiftBytes = 0;
         *data++ = (uint8)(0x02 | ASN_LONG_LENGTH);
         *data++ = (uint8)((length >> 8) & 0xFF);
         *data++ = (uint8)(length & 0xFF);
       }
  // move data
  for (index = data-data; index < length; index++)
     *(data+index) = *(data+index+shiftBytes);

  return data+length;
}  // end of if
#endif WEBADMIN

}


//----------------------------------------------------------------------
// asn_parse_length - interprets the length of the current object.
//  On exit, length contains the value of this length field.
//
//  Returns a pointer to the first byte after this length
//  field (aka: the start of the data field).
//  Returns NULL on any error.
//----------------------------------------------------------------------
uint8           *asn_parse_length(data, length)
uint8           *data;          /* IN  - pointer to start of length field */
uint32          *length;        /* OUT - value of length field */
{
    register uint8      lengthbyte = *data;
    int i ;

    if (lengthbyte & ASN_LONG_LENGTH) {
        lengthbyte &= ~ASN_LONG_LENGTH; /* turn MSb off */
        if (lengthbyte == 0) {
            snmpInASNParseErrs++ ;
            return NULL;  //-----We don't support indefinite lengths.
        }
        if (lengthbyte > sizeof(long)) {
            snmpInASNParseErrs++ ;
            return NULL;    //-----we can't support data lengths that long.
        }
        *length = 0L ;
        for (i=0 ; i < (int)lengthbyte ; i++)
            *length = (*length << 8) | (long)data[i+1] ;
        return (data+lengthbyte+1) ;
    }
    else { /* short asnlength */
            *length = (long)lengthbyte;
            return (data + 1);
    }
}


//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
uint8           *asn_build_length(data, datalength, length)
register uint8 *data;           /* IN     - pointer to start of object */
int             *datalength;    /* IN/OUT - number of valid bytes left in buffer */
register int    length;         /* IN     - length of object */
{
    uint8               *start_data = data;

    /* no indefinite lengths sent */
    if (length < (int)0x80) {
                *data++ = (uint8)length;
    }
    else if (length <= (int)0xFF) {
                *data++ = (uint8)((uint8)0x01 | (uint8)ASN_LONG_LENGTH);
                *data++ = (uint8)length;
    }
    else { /* 0xFF < length <= 0xFFFF */
                *data++ = (uint8)(0x02 | (uint8)ASN_LONG_LENGTH);
                *data++ = (uint8)((length >> 8) & (int)0xFF);
                *data++ = (uint8)(length & (int)0xFF);
    }

    if (*datalength < (data - start_data))
        return NULL;    //-----build_length.
    *datalength -= (int)(data - start_data);
    return data;

}


//---------------------------------------------------------------------------------
// asn_parse_objid - pulls an object indentifier out of an ASN object identifier
//                   type.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the beginning of the next object.
//
//  "objid" is filled with the object identifier.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_parse_objid(data, datalength, type, objid, objidlength)
uint8           *data;          /* IN     - pointer to start of object */
int             *datalength;    /* IN/OUT - number of valid bytes left in buffer */
uint8           *type;          /* OUT    - ASN type of object */
oid             *objid;         /* IN/OUT - pointer to start of output buffer */
int             *objidlength;   /* IN/OUT - number of sub-id's in objid */
{
    /*
     * ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
     * subidentifier ::= {leadingbyte}* lastbyte
     * leadingbyte ::= 1 7bitvalue
     * lastbyte ::= 0 7bitvalue
     */
    register uint8      *bufp = data;
    register oid        *oidp = objid + 1;
    register uint32     subidentifier;
    register long       length;
    uint32              asn_length;

    *type = *bufp++;
    bufp  = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL) return NULL;
    if (asn_length + (bufp - data) > *datalength) {
        snmpInASNParseErrs++ ;
        return NULL;    //-----overflow of message.
    }
    *datalength -= (int)asn_length + (int)(bufp - data);

    length = asn_length;
    (*objidlength)--;   /* account for expansion of first byte */
    while (length > 0 && (*objidlength)-- > 0) {
        subidentifier = 0;
        do {    /* shift and add in low order 7 bits */
            subidentifier = (subidentifier << 7) + (*(uint8 *)bufp & ~ASN_BIT8);
            length--;
        } while (*(uint8 *)bufp++ & ASN_BIT8); /* last byte has high bit clear */
        if (subidentifier > MAX_SUBID) {
            snmpInASNParseErrs++ ;
            return NULL;        //-----subidentifier too long.
        }
        *oidp++ = (oid)subidentifier;
    }

    /*
     * The first two subidentifiers are encoded into the first component
        * with the value (X * 40) + Y, where:
     *  X is the value of the first subidentifier.
     *  Y is the value of the second subidentifier.
     */
    subidentifier = (uint32)objid[1];
    objid[1]      = (uint8)(subidentifier % 40);
    objid[0]      = (uint8)((subidentifier - objid[1]) / 40);

    *objidlength  = (int)(oidp - objid);
    return bufp;
}


//---------------------------------------------------------------------------------
// asn_build_objid - Builds an ASN object identifier object containing the input
//                   string.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the beginning of the next object.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------

uint8           *asn_build_objid(
register uint8 *data,           /* IN     - pointer to start of object */
int             *datalength,    /* IN/OUT - number of valid bytes left in buffer */
uint8           type,           /* IN     - ASN type of object */
oid             *objid,         /* IN     - pointer to start of input buffer */
int             objidlength)    /* IN     - number of sub-id's in objid */
{
    //--- ASN.1 objid   ::= 0x06 asnlength subidentifier {subidentifier}*
    //--- subidentifier ::= {leadingbyte}* lastbyte
    //--- leadingbyte   ::= 1 7bitvalue
    //--- lastbyte      ::= 0 7bitvalue

    uint8               buf[MAX_OID_LEN];
    uint8               *bp = buf;
    oid                 objbuf[MAX_OID_LEN];
    oid                 *op = objbuf;
    register int        asnlength;
    register uint32     subid, mask, testmask;
    register int        bits, testbits;

    bcopy((char *)objid, (char *)objbuf, objidlength * sizeof(oid));

    /* transform size in bytes to size in subid's */
    /* encode the first two components into the first subidentifier */
    op[1] = op[1] + (op[0] * 40);
    op++;
    objidlength--;

    while(objidlength-- > 0) {
        subid = *op++;
        mask = 0x7F; /* handle subid == 0 case */
        bits = 0;

        //----- testmask *MUST* !!!! be of an unsigned type ---------
        for(testmask = 0x7F, testbits = 0; testmask != 0;
            testmask <<= 7, testbits += 7) {
            if (subid & testmask) {     //----- if any bits set -----
                mask = testmask;
                bits = testbits;
            }
        }
        //----- mask can't be zero here -----
        for(;mask != 0x7F; mask >>= 7, bits -= 7) {
            *bp++ = ((subid & mask) >> bits) | ASN_BIT8;
        }
        *bp++ = subid & mask;
    }
    asnlength = (int)(bp - buf) ;
    data = asn_build_header(data, datalength, type, asnlength);
    if (data == NULL) return NULL;
    if (*datalength < asnlength) return NULL;
    bcopy((char *)buf, (char *)data, asnlength);
    *datalength -= asnlength;
    return (data + asnlength);
}


//-----------------------------------------------------------------------------
// asn_parse_null - Interprets an ASN null type.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the beginning of the next object.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//-----------------------------------------------------------------------------
uint8           *asn_parse_null(data, datalength, type)
uint8           *data;          /* IN     - pointer to start of object */
int             *datalength;    /* IN/OUT - number of valid bytes left in buffer */
uint8           *type;          /* OUT    - ASN type of object */
{
    //----- ASN.1 null ::= 0x05 0x00 -----

    register uint8      *bufp = data;
    uint32              asn_length;

    *type = *bufp++ ;
    bufp  = asn_parse_length(bufp, &asn_length) ;
    if (bufp == NULL)
         return NULL ;
    if (asn_length != 0) {
         snmpInASNParseErrs++ ;
         return NULL ;    //-----Malformed NULL.-----
    }
    *datalength -= (int)(bufp - data) ;
    return ((uint8 *)((uint32)bufp+asn_length)) ;
}



//---------------------------------------------------------------------------------
// asn_build_null - Builds an ASN null object.
//  On entry, datalength is input as the number of valid bytes following
//   "data".  On exit, it is returned as the number of valid bytes
//   following the beginning of the next object.
//
//  Returns a pointer to the first byte past the end
//   of this object (i.e. the start of the next object).
//  Returns NULL on any error.
//---------------------------------------------------------------------------------
uint8           *asn_build_null(
uint8           *data,          /* IN - pointer to start of object */
int             *datalength,    /* IN/OUT - number of valid bytes left in buffer */
uint8           type)           /* IN - ASN type of object */
{
    //----- ASN.1 null ::= 0x05 0x00 -----

    return asn_build_header(data, datalength, type, 0);
}

/*
 * asn_build_bitstring - Builds an ASN bit string object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
uint8 *
asn_build_bitstring(
    uint8  *data,          /* IN - pointer to start of object */
    int    *datalength,    /* IN/OUT - number of valid bytes left in buffer */
    uint8  type,           /* IN - ASN type of string */
    uint8  *string,        /* IN - pointer to start of input buffer */
    int    strlength)      /* IN - size of input buffer */
{
/*
 * ASN.1 bit string ::= 0x03 asnlength unused {byte}*
 */
    if (strlength < 1 || /** *string < 0 || **/ *string > 7){
        ERROR("Building invalid bitstring");
        return NULL;
    }
    data = asn_build_header(data, datalength, type, strlength);
    if (data == NULL)
        return NULL;
    if (*datalength < strlength)
        return NULL;
    bcopy((char *)string, (char *)data, strlength);
    *datalength -= strlength;
    return data + strlength;
}
