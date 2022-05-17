// This file is dual-licensed.  Choose whichever licence you want from
// the two licences listed below.
//
// The first licence is a regular 2-clause BSD licence.  The second licence
// is the CC-0 from Creative Commons. It is intended to release Monocypher
// to the public domain.  The BSD licence serves as a fallback option.
//
// SPDX-License-Identifier: BSD-2-Clause OR CC0-1.0
//
// ------------------------------------------------------------------------
//
// Copyright (c) 2017-2019, Loup Vaillant
// All rights reserved.
//
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ------------------------------------------------------------------------
//
// Written in 2017-2019 by Loup Vaillant
//
// To the extent possible under law, the author(s) have dedicated all copyright
// and related neighboring rights to this software to the public domain
// worldwide.  This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along
// with this software.  If not, see
// <https://creativecommons.org/publicdomain/zero/1.0/>

#include "monocypher.h"
#include "utils.h"
// #include "vectors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int p_chacha20_ctr()
{
    int status = 0;
    RANDOM_INPUT(key, 32);
    RANDOM_INPUT(nonce, 24);
    RANDOM_INPUT(plain, 128);
    u8 out_full[128];
    u8 out1[64];
    u8 out2[64];
    crypto_chacha20(out_full, plain, 128, key, nonce);
    crypto_chacha20_ctr(out1, plain + 0, 64, key, nonce, 0);
    crypto_chacha20_ctr(out2, plain + 64, 64, key, nonce, 1);
    status |= memcmp(out_full, out1, 64);
    status |= memcmp(out_full + 64, out2, 64);

    crypto_ietf_chacha20(out_full, plain, 128, key, nonce);
    crypto_ietf_chacha20_ctr(out1, plain + 0, 64, key, nonce, 0);
    crypto_ietf_chacha20_ctr(out2, plain + 64, 64, key, nonce, 1);
    status |= memcmp(out_full, out1, 64);
    status |= memcmp(out_full + 64, out2, 64);

    crypto_xchacha20(out_full, plain, 128, key, nonce);
    crypto_xchacha20_ctr(out1, plain + 0, 64, key, nonce, 0);
    crypto_xchacha20_ctr(out2, plain + 64, 64, key, nonce, 1);
    status |= memcmp(out_full, out1, 64);
    status |= memcmp(out_full + 64, out2, 64);

    // printf("%s: Chacha20 (ctr)\n", status != 0 ? "FAILED" : "OK");
    return status;
}

int main(int argc, char *argv[])
{
    // if (argc > 1)
    // {
    //     sscanf(argv[1], "%" PRIu64 "", &random_state);
    // }
    // printf("\nRandom seed: %" PRIu64 "\n", random_state);

    // printf("\nProperty based tests");
    // printf("\n--------------------\n");
    int status = 0;
    status |= p_chacha20_ctr();
    // printf("\n%s\n\n", status != 0 ? "SOME TESTS FAILED" : "All tests OK!");
    return status;
}
