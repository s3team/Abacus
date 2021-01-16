typedef struct ec_key_method_st EC_KEY_METHOD;
typedef struct engine_st ENGINE;
typedef struct ec_group_st EC_GROUP;
typedef struct ec_point_st EC_POINT;
typedef struct ecpk_parameters_st ECPKPARAMETERS;
typedef struct ec_parameters_st ECPARAMETERS;

#define BN_ULONG unsigned int

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

typedef struct bignum_st BIGNUM;

/** Enum for the point conversion form as defined in X9.62 (ECDSA)
 *  for the encoding of a elliptic curve point (x,y) */
typedef enum
{
    /** the point is encoded as z||x, where the octet z specifies
         *  which solution of the quadratic equation y is  */
    POINT_CONVERSION_COMPRESSED = 2,
    /** the point is encoded as z||x||y, where z is the octet 0x04  */
    POINT_CONVERSION_UNCOMPRESSED = 4,
    /** the point is encoded as z||x||y, where the octet z specifies
         *  which solution of the quadratic equation y is  */
    POINT_CONVERSION_HYBRID = 6
} point_conversion_form_t;

#define STACK_OF(type) struct stack_st_##type

struct crypto_ex_data_st
{
    STACK_OF(void) * sk;
};

typedef struct crypto_ex_data_st CRYPTO_EX_DATA;

typedef int CRYPTO_REF_COUNT;

typedef void CRYPTO_RWLOCK;

struct ec_key_st
{
    const EC_KEY_METHOD *meth;
    ENGINE *engine;
    int version;
    EC_GROUP *group;
    EC_POINT *pub_key;
    BIGNUM *priv_key;
    unsigned int enc_flag;
    point_conversion_form_t conv_form;
    int references;
    int flags;
    CRYPTO_EX_DATA ex_data;
    CRYPTO_RWLOCK *lock;
};

typedef struct ec_key_st EC_KEY;
