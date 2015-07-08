/*
 * $Log$
 */
/*********************************************
 *
 * asn1.h
 *
 *********************************************/

#ifndef 	_ASN1_H
#define 	_ASN1_H

typedef 	uint16			oid;
#define 	MAX_SUBID		0xFFFF

#define 	MAX_OID_LEN		64	/* max length in bytes of an encoded oid */

/*
 *--------------------------------------------
 *
 * Define ASN.1 Classes
 *
 *--------------------------------------------
 */
#define 	ASN_UNIVERSAL		(0x00)
#define 	ASN_APPLICATION 	(0x40)
#define 	ASN_CONTEXT		(0x80)
#define 	ASN_PRIVATE		(0xC0)

/*
 *--------------------------------------------
 *
 * Define Primitive/Constructor
 *
 *--------------------------------------------
 */
#define 	ASN_PRIMITIVE		(0x00)
#define 	ASN_CONSTRUCTOR 	(0x20)

/*
 *--------------------------------------------
 *
 * Define Universal Tags (also types)
 *
 *--------------------------------------------
 */
#define 	ASN_BOOLEAN		(0x01)
#define 	ASN_INTEGER		(0x02)
#define 	ASN_BIT_STRING		(0x03)
#define 	ASN_OCTET_STRING	(0x04)
#define 	ASN_NULL		     (0x05)
#define 	ASN_OBJECT_ID		(0x06)
#define 	ASN_SEQUENCE		(0x10)
#define 	ASN_SET 		     (0x11)

/*
 *--------------------------------------------
 *
 * Define High Tag Number
 *
 *--------------------------------------------
 */
#define 	ASN_EXTENSION_ID	(0x1F)
#define 	ASN_LONG_LENGTH 	(0x80)
#define 	ASN_INDEFINITE_FORM	(0x80)
#define 	ASN_BIT8		(0x80)


/*
 *--------------------------------------------
 *
 * Define Convenient Macro
 *
 *--------------------------------------------
 */
#define 	IS_CONSTRUCTOR(byte)	((byte) & ASN_CONSTRUCTOR)
#define 	IS_EXTENSION_ID(byte)	(((byte) & ASN_EXTENSION_ID) == ASN_EXTENSION_ID)
#define 	IS_LONG_LENGTH(byte)	((byte) & ASN_LONG_LENGTH)

/*
 *--------------------------------------------
 *
 * Define ASN.1 primitive functions
 *
 *--------------------------------------------
 */

uint8	*asn_parse_int	  (uint8 *, int *, uint8 *, long *,int) ;
uint8	*asn_build_int	  (uint8 *, int *, uint8, long *, int) ;
uint8	*asn_parse_string (uint8 *, int *, uint8 *, uint8 *, int *) ;
uint8	*asn_build_string (uint8 *, int *, uint8, uint8 *,int) ;
uint8	*asn_parse_header (uint8 *, int *, uint8 *) ;
uint8	*asn_build_header (uint8 *, int *, uint8, int) ;
uint8	*asn_parse_length (uint8 *, uint32 *) ;
uint8	*asn_build_length (uint8 *, int *, int) ;
uint8	*asn_parse_objid  (uint8 *, int *, uint8 *, oid *,int *) ;
uint8	*asn_build_objid  (uint8 *, int *, uint8, oid *, int) ;
uint8	*asn_parse_null   (uint8 *, int *, uint8 *) ;
uint8	*asn_build_null   (uint8 *, int *, uint8) ;

uint8   *asn_build_sequence (uint8 *, int *, uint8, int);
uint8	*asn_build_unsigned_int (uint8 *,int *,uint8,uint32 *intp,int);
uint8   *asn_build_bitstring(uint8 *, int *, uint8, uint8 *, int);
#endif		/* _ASN1_H */
