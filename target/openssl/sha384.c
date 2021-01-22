#include <openssl/sha.h>
#include <stdio.h>

int main(){
    unsigned char *sha384_result = NULL;
    sha384_result = malloc(SHA384_DIGEST_LENGTH);
    char hello[6] = "Hello";
    SHA384(hello, 6, sha384_result);

#ifdef DEBUG
    int i;
    for(i=0; i < SHA384_DIGEST_LENGTH; ++i){
        printf("%02x", sha384_result[i]);
    }

#endif
    return 0;
}

