#include "includes.h"

typedef struct bignum_st BIGNUM;
typedef struct bn_mont_ctx_st BN_MONT_CTX;

#define BN_ULONG	unsigned int
#define INT_MAX     ((unsigned long)0x7FFFFFFF)	

#define BN_BITS		64
#define BN_BYTES	4
#define BN_BITS2	32
#define BN_BITS4	16
#define BN_MASK		(0xffffffffffffffffLL)
#define BN_MASK2	(0xffffffffL)
#define BN_MASK2l	(0xffff)
#define BN_MASK2h1	(0xffff8000L)
#define BN_MASK2h	(0xffff0000L)
#define BN_TBIT		(0x80000000L)
#define BN_CTX_POOL_SIZE	16
#define EXP_TABLE_SIZE			32
#define BN_FLG_MALLOCED			0x01
#define BN_FLG_STATIC_DATA		0x02
#define BN_FLG_EXP_CONSTTIME	0x04 /* avoid leaking exponent information through timings
                            	      * (BN_mod_exp_mont() will call BN_mod_exp_mont_consttime) */

/* BN_mod_exp_mont_conttime is based on the assumption that the
 * L1 data cache line width of the target processor is at least
 * the following value.
 */
#define MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH	( 64 )
#define MOD_EXP_CTIME_MIN_CACHE_LINE_MASK	(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH - 1)

struct bignum_st
{
	BN_ULONG *d;	/* Pointer to an array of 'BN_BITS2' bit chunks. */
	int top;	/* Index of last used d +1. */
	/* The next are internal book keeping for bn_expand. */
	int dmax;	/* Size of the d array. */
	int flags;
};

/* Used for montgomery multiplication */
struct bn_mont_ctx_st
{
	int ri;        /* number of bits in R */
	BIGNUM RR;     /* used to convert to montgomery form */
	BIGNUM N;      /* The modulus */
	BN_ULONG n0;   /* least significant word of Ni */
	int flags;
};
	
/* A bundle of bignums that can be linked with other bundles */
typedef struct bignum_pool_item
{
	/* The bignum values */
	BIGNUM vals[BN_CTX_POOL_SIZE];
	/* Linked-list admin */
	struct bignum_pool_item *prev, *next;
}BN_POOL_ITEM;

/* A linked-list of bignums grouped in bundles */
typedef struct bignum_pool
{
	/* Linked-list admin */
	BN_POOL_ITEM *head, *currentvar, *tail;
	/* Stack depth and allocation size */
	unsigned long used, size;
}BN_POOL;
/*dh_key.h END*/

#define AES_MAXNR 14
#define AES_LONG
/* This should be a hidden type, but EVP requires that the size be known */
struct aes_key_st {
#ifdef AES_LONG
    unsigned long rd_key[4 *(AES_MAXNR + 1)];
#else
    UINT32 rd_key[4 *(AES_MAXNR + 1)];
#endif
    signed long rounds;
};
typedef struct aes_key_st AES_KEY;