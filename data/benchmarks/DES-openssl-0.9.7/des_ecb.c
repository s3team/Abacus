#include <stdio.h>
#include <openssl/des.h>
#include <openssl/rand.h>

int main(int argc,char **argv)
{
    DES_cblock key;
    DES_cblock seed = {0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};

    DES_key_schedule schedule;

    RAND_seed(seed, sizeof(DES_cblock));
    DES_random_key(&key);

    DES_set_key(&key, &schedule); 

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

    DES_ecb_encrypt(&output, &input, &schedule, DES_DECRYPT);

    printf("cleartext:%s ", input);
#endif
    return 0;
} 
