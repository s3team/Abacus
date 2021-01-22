#include <stdio.h>
#include <openssl/des.h>

int main(int argc,char **argv)
{
    DES_cblock key;
    /**//* DES_random_key(&key); */ /**//* generate a random key */
    DES_string_to_key("pass", &key);

    DES_key_schedule schedule;
    DES_set_key_checked(&key, &schedule); 

    const_DES_cblock input = "hehehe";
    DES_cblock output;

#ifdef DEBUG

    printf("cleartext:%s ", input);
#endif

    DES_ecb_encrypt(&input, &output, &schedule, DES_ENCRYPT);

#ifdef DEBUG
    printf("ciphertext:");
    int i;
    for (i = 0; i < sizeof(input); i++)
         printf("%02x", output[i]);
    printf(" ");
#endif

    DES_ecb_encrypt(&output, &input, &schedule, DES_DECRYPT);

#ifdef DEBUG
    printf("cleartext:%s ", input);
#endif
    return 0;
} 
