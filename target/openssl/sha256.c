#include <openssl/sha.h>
#include <stdio.h>

int main(){
    unsigned char *sha256_result = NULL;
    sha256_result = malloc(SHA256_DIGEST_LENGTH);
    char hello[6] = "Hello";
    SHA256(hello, 6, sha256_result);

#ifdef DEBUG
    int i;
    for(i=0; i < SHA256_DIGEST_LENGTH; ++i){
        printf("%02x", sha256_result[i]);
    }

#endif
    return 0;
}

