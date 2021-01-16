#include <openssl/bn.h>
#include <openssl/ossl_typ.h>

struct bignum_st
{
    BN_ULONG *d; /* Pointer to an array of 'BN_BITS2' bit
                                 * chunks. */
    int top;     /* Index of last used d +1. */
    /* The next are internal book keeping for bn_expand. */
    int dmax; /* Size of the d array. */
    int neg;  /* one if the number is negative */
    int flags;
};

struct rsa_st {
    /*
     * The first parameter is used to pickup errors where this is passed
     * instead of an EVP_PKEY, it is set to 0
     */
    int pad;
    int32_t version;
    const RSA_METHOD *meth;
    /* functional reference if 'meth' is ENGINE-provided */
    ENGINE *engine;
    BIGNUM *n;
    BIGNUM *e;
    BIGNUM *d;
    BIGNUM *p;
    BIGNUM *q;
    BIGNUM *dmp1;
    BIGNUM *dmq1;
    BIGNUM *iqmp;
    /* for multi-prime RSA, defined in RFC 8017 */
    STACK_OF(RSA_PRIME_INFO) *prime_infos;
    /* If a PSS only key this contains the parameter restrictions */
    RSA_PSS_PARAMS *pss;
    /* be careful using this if the RSA structure is shared */
    CRYPTO_EX_DATA ex_data;
    int references;
    int flags;
    /* Used to cache montgomery values */
    BN_MONT_CTX *_method_mod_n;
    BN_MONT_CTX *_method_mod_p;
    BN_MONT_CTX *_method_mod_q;
    /*
     * all BIGNUM values are actually in the following data, if it is not
     * NULL
     */
    char *bignum_data;
    BN_BLINDING *blinding;
    BN_BLINDING *mt_blinding;
    CRYPTO_RWLOCK *lock;
};

typedef struct rsa_st RSA;
typedef struct rsa_meth_st RSA_METHOD;
