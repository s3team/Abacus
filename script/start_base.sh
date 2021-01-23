#!/bin/bash

dir_str=$(basename $(pwd))
dir_arr=($(echo $dir_str | tr '_' ' '))

alg_str=${dir_arr[0]}
alg_str=${alg_str/Ed25519/ed25519}
alg_str=${alg_str/POLY1305/poly1305}
alg_str=${alg_str/ARGON2i/argon2i}
alg_str=${alg_str/CHACHA20/chacha20}

lib_str_upc=${dir_arr[1]}
lib_str=${lib_str_upc/MBEDTLS/mbedTLS}
lib_str=${lib_str/OPENSSL/openssl}
lib_str=${lib_str/MONOCYPHER/monocypher}

ver_str_ori=${dir_arr[2]}
ver_str=$ver_str_ori
# if [[ $ver_str_ori = "2.15" ]]; then
#     ver_str="2.15.1"
# fi

bin_str=$(find /abacus/data/benchmarks/$alg_str-$lib_str-$ver_str/ -executable -type f)

qif="/abacus/QIF-old"

case $alg_str in
# "AES")
#     case $lib_str in
#     "mbedTLS")
#         fuc_str="mbedtls_aes_setkey_enc"
#         ;;
#     "openssl")
#         fuc_str="AES_set_encrypt_key"
#         ;;
#     esac
#     ;;
"ARGON2i") fuc_str="crypto_argon2i" ;;
"CHACHA20") fuc_str="crypto_chacha20" ;;
    # "DES")
    #     case $lib_str in
    #     "mbedTLS") fuc_str="mbedtls_des_crypt_ecb" ;;
    #     "openssl") fuc_str="DES_set_key" ;;
    #     esac
    #     ;;
"ECDSA")
    ver_str=$ver_str-nonce
    qif="/abacus/QIF-new"
    #     case $lib_str in
    #     "mbedTLS") fuc_str="mbedtls_ecdsa_write_signature" ;;
    #     "openssl") fuc_str="ECDSA_do_sign" ;;
    #     esac
    ;;
    # "ed25519") fuc_str="crypto_key_exchange_public_key" ;;
    # "poly1305") fuc_str="crypto_poly1305" ;;
    # "RSA")
    #     case $lib_str in
    #     "mbedTLS") fuc_str="" ;;
    #     "openssl") fuc_str="" ;;
    #     "libgcrypt") fuc_str="" ;;
    #     esac
    #     ;;
esac
# out_str=$alg_str\_$lib_str_upc\_$ver_str_ori

if [[ $DEBUG = true ]]; then
    file /abacus/Pintools/CryptoLibrary/obj-ia32/$alg_str-$lib_str-$ver_str.so
    file $bin_str
    echo $fuc_str

    echo "/abacus/script/$dir_str/Inst_data.txt"
    tmp_str=$dir_str
    echo "${tmp_str,,}.txt"
fi
