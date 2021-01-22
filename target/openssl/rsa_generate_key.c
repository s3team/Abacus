#include <openssl/rsa.h>
#include <openssl/pem.h>


int main(){
    const int kBits = 32;
    const int kExp = 3;

    int keylen;
    char *pem_key;

    RSA *rsa = RSA_generate_key(kBits, kExp, 0, 0);
#ifdef DEBUG
    BIO *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);

    keylen = BIO_pending(bio);
    pem_key = calloc(keylen+1, 1);
    BIO_read(bio, pem_key, keylen);

    printf("%s", pem_key);
    BIO_free_all(bio);
    RSA_free(rsa);
    free(pem_key);
#endif
    return 0;
}
