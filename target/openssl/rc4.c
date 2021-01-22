#include <openssl/rc4.h>
#include <stdlib.h>

#define KEY "HELLO"
#define KEY_LEN (sizeof(KEY)-1)

int main(){
    RC4_KEY key;
    char data[] = "Hello World";
    char out_put[] = "asdasdasdasdasd";
    RC4_set_key(&key, KEY_LEN, KEY);
    RC4(&key, sizeof(data) - 1, data, out_put);
   
#ifdef DEBUG 
    printf("Encrypt: %s\n", out_put);

    RC4(&key, sizeof(out_put) - 1, out_put, out_put);

    printf("Decrypt: %s\n", out_put); 
#endif
    return 0;
}     
