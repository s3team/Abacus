#include <stdio.h>
#include <mbedtls/md.h>

int main()
{
    const unsigned char* input = "String to be hashed with SHA512";
    unsigned char output[64];

    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(  MBEDTLS_MD_SHA512 );
    int ret = mbedtls_md( md_info, input, sizeof(input), output);

#ifdef DEBUG
    printf("SHA512: ");
    for(size_t i = 0; i < sizeof(output); i++) {
        printf("%02X", output[i]);
    }
    printf("\n");
#endif

    return 0;
}
