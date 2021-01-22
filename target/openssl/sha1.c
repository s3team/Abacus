#include <openssl/sha.h>
#include <stdio.h>

int main(){
    unsigned char *md5_result = NULL;
    md5_result = malloc(20);
    char hello[6] = "Hello";
    SHA1(hello, 6, md5_result);

#ifdef DEBUG
    int i;
    for(i=0; i < 20; ++i){
        printf("%02x", md5_result[i]);
    }

#endif
    return 0;
}

