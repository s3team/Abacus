#!/bin/bash

echo "Collecting Trace Files..."
/abacus/Intel-Pin-Archive/pin -t /abacus/Pintools/CryptoLibrary/obj-ia32/AES-mbedTLS-2.5.so -- /abacus/data/benchmarks/AES-mbedTLS-2.5/aes_128_cbc mbedtls_aes_setkey_enc

echo "Analyzing"
/abacus/QIF-new /abacus/script/AES_MBEDTLS_2.5/Inst_data.txt -o aes_mbedtls_2.5.txt

