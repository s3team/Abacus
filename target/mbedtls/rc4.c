#include <stdio.h>
#include <string.h>
#include <mbedtls/arc4.h>

int main()
{
    unsigned char key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
                             0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };
    unsigned char plaintext[27];
    unsigned char ciphertext[27];
    mbedtls_arc4_context ctx;

    memset(plaintext, 0x07, sizeof(plaintext));
    mbedtls_arc4_init(&ctx);

    mbedtls_arc4_setup(&ctx, key, sizeof(key) * 8);
    int ret = mbedtls_arc4_crypt(&ctx, sizeof(plaintext), plaintext, ciphertext);
    if(ret != 0) {
        return ret;
    }

#ifdef DEBUG
    mbedtls_arc4_setup(&ctx, key, sizeof(key) * 8);
    ret = mbedtls_arc4_crypt(&ctx, sizeof(ciphertext), ciphertext, plaintext);
    if(ret != 0) {
        return ret;
    }
    for(int i = 0; i < sizeof(plaintext); i++) {
        if(plaintext[i] != 0x07) {
            printf("RC4: Incorrect result\n");
            return -1;
        }
    }
    printf("RC4 result correct\n");
#endif

    mbedtls_arc4_free(&ctx);
    return 0;

}

