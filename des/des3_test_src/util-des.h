#ifndef __UTIL_TOMCRYPT_H__
#define __UTIL_TOMCRYPT_H__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/* max size of either a cipher/hash block or symmetric key [largest of the two] */
#define MAXBLOCKSIZE  128

/* descriptor table size */
#define TAB_SIZE      32

/* error codes [will be expanded in future releases] */
enum {
   CRYPT_OK=0,             /* Result OK */
   CRYPT_ERROR,            /* Generic Error */
   CRYPT_NOP,              /* Not a failure but no operation was performed */

   CRYPT_INVALID_KEYSIZE,  /* Invalid key size given */
   CRYPT_INVALID_ROUNDS,   /* Invalid number of rounds */
   CRYPT_FAIL_TESTVECTOR,  /* Algorithm failed test vectors */

   CRYPT_BUFFER_OVERFLOW,  /* Not enough space for output */
   CRYPT_INVALID_PACKET,   /* Invalid input packet given */

   CRYPT_INVALID_PRNGSIZE, /* Invalid number of bits for a PRNG */
   CRYPT_ERROR_READPRNG,   /* Could not read enough from PRNG */

   CRYPT_INVALID_CIPHER,   /* Invalid cipher specified */
   CRYPT_INVALID_HASH,     /* Invalid hash specified */
   CRYPT_INVALID_PRNG,     /* Invalid PRNG specified */

   CRYPT_MEM,              /* Out of memory */

   CRYPT_PK_TYPE_MISMATCH, /* Not equivalent types of PK keys */
   CRYPT_PK_NOT_PRIVATE,   /* Requires a private PK key */

   CRYPT_INVALID_ARG,      /* Generic invalid argument */
   CRYPT_FILE_NOTFOUND,    /* File Not Found */

   CRYPT_PK_INVALID_TYPE,  /* Invalid type of PK key */
   CRYPT_PK_INVALID_SYSTEM,/* Invalid PK system specified */
   CRYPT_PK_DUP,           /* Duplicate key already in key ring */
   CRYPT_PK_NOT_FOUND,     /* Key not found in keyring */
   CRYPT_PK_INVALID_SIZE,  /* Invalid size input for PK parameters */

   CRYPT_INVALID_PRIME_SIZE,/* Invalid size of prime requested */
   CRYPT_PK_INVALID_PADDING /* Invalid padding on input */
};

/* this is the "32-bit at least" data type
 * Re-define it to suit your platform but it must be at least 32-bits
 */
#if defined(__x86_64__) || (defined(__sparc__) && defined(__arch64__))
   typedef unsigned ulong32;
#else
   typedef unsigned long ulong32;
#endif

struct des_key {
	ulong32 ek[32], dk[32];
};

struct des3_key {
	ulong32 ek[3][32], dk[3][32];
};

typedef union Symmetric_key
{
   struct des_key des;
   struct des3_key des3;
   void   *data;
} symmetric_key;

int  des_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int  des_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey);
int  des_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey);
int  des_test(void);
void des_done(symmetric_key *skey);
int  des_keysize(int *keysize);
int  des3_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int  des3_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey);
int  des3_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey);
int  des3_test(void);
void des3_done(symmetric_key *skey);
int  des3_keysize(int *keysize);


#ifdef __cplusplus
   }
#endif

#endif /* TOMCRYPT_H_ */
