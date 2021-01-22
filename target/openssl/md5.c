#include <openssl/md5.h>
#include <stdio.h>

int main(){
    unsigned char *md5_result = NULL;
    md5_result = malloc(MD5_DIGEST_LENGTH);
    char hello[6] = "Hello";
    MD5(hello, 6, md5_result);

#ifdef DEBUG
    int i;
    for(i=0; i < MD5_DIGEST_LENGTH; ++i){
        printf("%02x", md5_result[i]);
    }

#endif
    return 0;
}

