#include <string.h>
#include <stdint.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define KEY_LENGTH       1024
#define PUBLIC_EXPONENT  17     //Public exponent should be a prime number.
#define PUBLIC_KEY_PEM   1
#define PRIVATE_KEY_PEM  0


RSA * create_RSA(RSA * keypair, int pem_type, char *file_name) {

    RSA   *rsa = NULL;
    FILE  *fp  = NULL;

    if(pem_type == PUBLIC_KEY_PEM) {

        fp = fopen(file_name, "w");
        PEM_write_RSAPublicKey(fp, keypair);
        fclose(fp);

        fp = fopen(file_name, "rb");
        PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL);
        fclose(fp);

    }
    else if(pem_type == PRIVATE_KEY_PEM) {

        fp = fopen(file_name, "w");
        PEM_write_RSAPrivateKey(fp, keypair, NULL, NULL, NULL, NULL, NULL);
        fclose(fp);

        fp = fopen(file_name, "rb");
        PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
        fclose(fp);

    }

    return rsa;
}

int public_encrypt(int flen, unsigned char* from, unsigned char* to, RSA* key, int padding) {
    
    int result = RSA_public_encrypt(flen, from, to, key, padding);
    return result;
}

int private_decrypt(int flen, unsigned char* from, unsigned char* to, RSA* key, int padding) {

    int result = RSA_private_decrypt(flen, from, to, key, padding);
    return result;
}

int pin_private_decrypt(RSA *key, uint32_t key_size, int flen, unsigned char* from, unsigned char* to , int padding){
    int result = private_decrypt(flen, from, to, key, padding);
    return result + key_size;

}

void create_encrypted_file(char* encrypted, RSA* key_pair) {

    FILE* encrypted_file = fopen("encrypted_file.bin", "w");
    fwrite(encrypted, sizeof(*encrypted), RSA_size(key_pair), encrypted_file);
    fclose(encrypted_file);
}


int main() {
    RSA *private_key;
    RSA *public_key;

    char message[KEY_LENGTH / 8] = "hello";

    char *encrypt = NULL;
    char *decrypt = NULL;

    char private_key_pem[12] = "private_key";
    char public_key_pem[11]  = "public_key";

    RSA *keypair = RSA_generate_key(KEY_LENGTH, PUBLIC_EXPONENT, NULL, NULL);

    private_key = create_RSA(keypair, PRIVATE_KEY_PEM, private_key_pem);
    public_key  = create_RSA(keypair, PUBLIC_KEY_PEM, public_key_pem);

    encrypt = (char*)malloc(RSA_size(public_key));

    int encrypt_length = public_encrypt(strlen(message) + 1, (unsigned char*)message, (unsigned char*)encrypt, public_key, RSA_PKCS1_OAEP_PADDING);
    
    if(encrypt_length == -1) {
        printf("An error occurred in public_encrypt() method\n");
    }

    create_encrypted_file(encrypt, public_key);

    decrypt = (char *)malloc(encrypt_length);

    printf("Key length = %d\n", sizeof(*private_key));
    int decrypt_length = pin_private_decrypt(private_key, sizeof(*private_key), encrypt_length, (unsigned char*)encrypt, (unsigned char*)decrypt, RSA_PKCS1_OAEP_PADDING);
    if(decrypt_length == -1) {
        printf("An error occurred in private_decrypt() method\n");
    }

   // printf("Decrypted: %s", decrypt);

    return 0;
}
