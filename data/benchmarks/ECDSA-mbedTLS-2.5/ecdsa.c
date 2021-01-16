/*
 *  Example ECDSA program
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#include "mbedtls/config.h"

#include <stdio.h>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"

#include <string.h>

/*
 * Uncomment to force use of a specific curve
 */
#define ECPARAMS MBEDTLS_ECP_DP_SECP192R1

#if !defined(ECPARAMS)
#define ECPARAMS mbedtls_ecp_curve_list()->grp_id
#endif

int main(int argc, char *argv[])
{
    int ret;
    mbedtls_ecdsa_context ctx_sign, ctx_verify;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_sha256_context sha256_ctx;
    unsigned char message[100];
    unsigned char hash[32];
    unsigned char sig[MBEDTLS_ECDSA_MAX_LEN];
    size_t sig_len;
    const char *pers = "ecdsa";
    ((void)argv);

    mbedtls_ecdsa_init(&ctx_sign);
    mbedtls_ecdsa_init(&ctx_verify);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_sha256_init(&sha256_ctx);

    memset(sig, 0, sizeof(sig));
    memset(message, 0x25, sizeof(message));
    ret = 1;

    /*
     * Generate a key pair for signing
     */
    // mbedtls_printf( "\n  . Seeding the random number generator..." );
    // fflush( stdout );

    mbedtls_entropy_init(&entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *)pers,
                                     strlen(pers))) != 0)
    {
        // mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    // mbedtls_printf( " ok\n  . Generating key pair..." );
    // fflush( stdout );

    if ((ret = mbedtls_ecdsa_genkey(&ctx_sign, ECPARAMS,
                                    mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        // mbedtls_printf( " failed\n  ! mbedtls_ecdsa_genkey returned %d\n", ret );
        goto exit;
    }

    // mbedtls_printf( " ok (key size: %d bits)\n", (int) ctx_sign.grp.pbits );

    // dump_pubkey( "  + Public key: ", &ctx_sign );

    /*
     * Compute message hash
     */
    // mbedtls_printf( "  . Computing message hash..." );
    // fflush( stdout );

    mbedtls_sha256_starts(&sha256_ctx, 0);
    mbedtls_sha256_update(&sha256_ctx, message, sizeof(message));
    mbedtls_sha256_finish(&sha256_ctx, hash);

    // mbedtls_printf( " ok\n" );

    // dump_buf( "  + Hash: ", hash, sizeof( hash ) );

    /*
     * Sign message hash
     */
    // mbedtls_printf( "  . Signing message hash..." );
    // fflush( stdout );

    if ((ret = mbedtls_ecdsa_write_signature(&ctx_sign, MBEDTLS_MD_SHA256,
                                             hash, sizeof(hash),
                                             sig, &sig_len,
                                             mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        // mbedtls_printf( " failed\n  ! mbedtls_ecdsa_genkey returned %d\n", ret );
        goto exit;
    }
    // mbedtls_printf( " ok (signature length = %u)\n", (unsigned int) sig_len );

    // dump_buf( "  + Signature: ", sig, sig_len );

    // /*
    //  * Transfer public information to verifying context
    //  *
    //  * We could use the same context for verification and signatures, but we
    //  * chose to use a new one in order to make it clear that the verifying
    //  * context only needs the public key (Q), and not the private key (d).
    //  */
    // // mbedtls_printf( "  . Preparing verification context..." );
    // // fflush( stdout );

    // if( ( ret = mbedtls_ecp_group_copy( &ctx_verify.grp, &ctx_sign.grp ) ) != 0 )
    // {
    //     // mbedtls_printf( " failed\n  ! mbedtls_ecp_group_copy returned %d\n", ret );
    //     goto exit;
    // }

    // if( ( ret = mbedtls_ecp_copy( &ctx_verify.Q, &ctx_sign.Q ) ) != 0 )
    // {
    //     mbedtls_printf( " failed\n  ! mbedtls_ecp_copy returned %d\n", ret );
    //     goto exit;
    // }

    // ret = 0;

    // /*
    //  * Verify signature
    //  */
    // mbedtls_printf( " ok\n  . Verifying signature..." );
    // fflush( stdout );

    // if( ( ret = mbedtls_ecdsa_read_signature( &ctx_verify,
    //                                   hash, sizeof( hash ),
    //                                   sig, sig_len ) ) != 0 )
    // {
    //     mbedtls_printf( " failed\n  ! mbedtls_ecdsa_read_signature returned %d\n", ret );
    //     goto exit;
    // }

    // mbedtls_printf( " ok\n" );

exit:

    mbedtls_ecdsa_free(&ctx_verify);
    mbedtls_ecdsa_free(&ctx_sign);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_sha256_free(&sha256_ctx);

    return (ret);
}