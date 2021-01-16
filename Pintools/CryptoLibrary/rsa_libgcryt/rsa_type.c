// #include "../../target/libcrypt_1.8.5/libgcrypt-1.8.5/config.h"

// // #include "../../target/libcrypt_1.8.5/libgcrypt-1.8.5/src/gcrypt.h"

// #define BYTES_PER_MPI_LIMB (SIZEOF_UNSIGNED_INT)
// // #if __GNUC__ >= 3 && defined(__x86_64__) && defined(__ILP32__)
// // #define BYTES_PER_MPI_LIMB 8
// // #else
// // #define BYTES_PER_MPI_LIMB (SIZEOF_UNSIGNED_LONG)
// // #endif

// #ifndef BITS_PER_MPI_LIMB
// #if BYTES_PER_MPI_LIMB == SIZEOF_UNSIGNED_INT
// typedef unsigned int mpi_limb_t;
// typedef signed int mpi_limb_signed_t;
// #elif BYTES_PER_MPI_LIMB == SIZEOF_UNSIGNED_LONG
// typedef unsigned long int mpi_limb_t;
// typedef signed long int mpi_limb_signed_t;
// #elif BYTES_PER_MPI_LIMB == SIZEOF_UNSIGNED_LONG_LONG
// typedef unsigned long long int mpi_limb_t;
// typedef signed long long int mpi_limb_signed_t;
// #elif BYTES_PER_MPI_LIMB == SIZEOF_UNSIGNED_SHORT
// typedef unsigned short int mpi_limb_t;
// typedef signed short int mpi_limb_signed_t;
// #else
// #error BYTES_PER_MPI_LIMB does not match any C type
// #endif
// #define BITS_PER_MPI_LIMB (8 * BYTES_PER_MPI_LIMB)
// #endif /*BITS_PER_MPI_LIMB*/

struct gcry_mpi
{
  int alloced;        /* Array size (# of allocated limbs). */
  int nlimbs;         /* Number of valid limbs. */
  int sign;           /* Indicates a negative number and is also used
		          for opaque MPIs to store the length.  */
  unsigned int flags; /* Bit 0: Array to be allocated in secure memory space.*/
                      /* Bit 2: The limb is a pointer to some m_alloced data.*/
                      /* Bit 4: Immutable MPI - the MPI may not be modified.  */
                      /* Bit 5: Constant MPI - the MPI will not be freed.  */
  // mpi_limb_t *d;      /* Array with the limbs */
  unsigned int *d;
};

/* The data objects used to hold multi precision integers.  */
typedef struct gcry_mpi *gcry_mpi_t;

typedef struct
{
  gcry_mpi_t n; /* modulus */
  gcry_mpi_t e; /* exponent */
} RSA_public_key;

typedef struct
{
  gcry_mpi_t n; /* public modulus */
  gcry_mpi_t e; /* public exponent */
  gcry_mpi_t d; /* exponent */
  gcry_mpi_t p; /* prime  p. */
  gcry_mpi_t q; /* prime  q. */
  gcry_mpi_t u; /* inverse of p mod q. */
} RSA_secret_key;
