#include <stdio.h>
#include <mbedtls/md.h>

// gcc -m32 -o md5 md5.c -I./mbedtls/build/include -L./mbedtls/build/library -lmbedcrypto

int main()
{
    const unsigned char* input = "String to be hashed with MD5";
    unsigned char output[16];

    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(  MBEDTLS_MD_MD5 );
    int ret = mbedtls_md( md_info, input, sizeof(input), output);

#ifdef DEBUG
    printf("md5: ");
    for(size_t i = 0; i < sizeof(output); i++) {
        printf("%02X", output[i]);
    }
    printf("\n");
#endif

    return 0;

}
