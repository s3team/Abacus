#include <openssl/md4.h>
#include <stdio.h>

int main(){
    unsigned char *md4_result = NULL;
    md4_result = malloc(MD4_DIGEST_LENGTH);
    char hello[6] = "Hello";
    MD4(hello, 6, md4_result);

#ifdef DEBUG
    int i;
    for(i=0; i < MD4_DIGEST_LENGTH; ++i){
        printf("%02x", md4_result[i]);
    }

#endif
    return 0;
}

