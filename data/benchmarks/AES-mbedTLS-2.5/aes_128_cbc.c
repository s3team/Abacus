#include <stdio.h>
#include <string.h>
#include <mbedtls/aes.h>

static const unsigned char key[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};

#ifdef DEBUG
void print_hex(const char* s, uint8_t* data, size_t len)
{
    printf("%s: ", s);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}
#endif

int main()
{
     unsigned char plaintext[16];
     unsigned char iv[16];
     unsigned char ciphertext[16];
     unsigned char dec_out[16];
     mbedtls_aes_context ctx;

     mbedtls_aes_init(&ctx);

     memset(plaintext, 0x07, sizeof(plaintext));
     memset(iv, 0x01, sizeof(iv));

     int ret = mbedtls_aes_setkey_enc(&ctx, key, sizeof(key)*8 );
     if(ret != 0) {
		printf("Failed to set aes key for encrpytion\n");
		return -1;
     }

     ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, sizeof(plaintext), iv, plaintext, ciphertext);
     if(ret != 0) {
	printf("Failed to aes-cbc encrypt\n");
        return -1;
     }

#ifdef DEBUG
	memset(iv, 0x01, sizeof(iv));

	ret = mbedtls_aes_setkey_dec(&ctx, key, sizeof(key)*8 );
	if(ret != 0) {
		printf("Failed to set aes key for decryption\n");
		return -1;
	}

	ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, sizeof(ciphertext), iv, ciphertext, dec_out);
	if(ret != 0) {
		printf("Failed to aes-cbc decrypt\n");
		return -1;
	}

	print_hex("plaintext", plaintext, sizeof(plaintext));
	print_hex("ciphertext", ciphertext, sizeof(ciphertext));
	print_hex("decrypted", dec_out, sizeof(dec_out));

	mbedtls_aes_free(&ctx);
#endif
        return 0;
} 
