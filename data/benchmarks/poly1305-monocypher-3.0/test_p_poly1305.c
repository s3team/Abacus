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

#define POLY1305_BLOCK_SIZE 16

// Tests that authenticating bit by bit yields the same mac than
// authenticating all at once
static int p_poly1305()
{
#undef INPUT_SIZE
#define INPUT_SIZE (POLY1305_BLOCK_SIZE * 4) // total input size
    int status = 0;
    FOR(i, 0, INPUT_SIZE)
    {
        // outputs
        u8 mac_chunk[16];
        u8 mac_whole[16];
        // inputs
        RANDOM_INPUT(input, INPUT_SIZE);
        RANDOM_INPUT(key, 32);

        // Authenticate bit by bit
        crypto_poly1305_ctx ctx;
        crypto_poly1305_init(&ctx, key);
        crypto_poly1305_update(&ctx, input, i);
        crypto_poly1305_update(&ctx, input + i, INPUT_SIZE - i);
        crypto_poly1305_final(&ctx, mac_chunk);

        // Authenticate all at once
        crypto_poly1305(mac_whole, input, INPUT_SIZE, key);

        // Compare the results (must be the same)
        status |= memcmp(mac_chunk, mac_whole, 16);
    }
    //printf("%s: Poly1305 (incremental)\n", status != 0 ? "FAILED" : "OK");
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
    status |= p_poly1305();
    // printf("\n%s\n\n", status != 0 ? "SOME TESTS FAILED" : "All tests OK!");
    return status;
}
