#include <stdio.h>
#include <openssl/crypto.h>
#include <openssl/bn.h>

int main(int argc, char *argv[])
{
        static const char num1[] = "18446744073709551616";
        static const char num2[] = "36893488147419103232";

        BIGNUM *bn1 = NULL;
        BIGNUM *bn2 = NULL;

        BN_CTX *ctx = BN_CTX_new();

        BN_dec2bn(&bn1, num1); // convert the string to BIGNUM
        BN_dec2bn(&bn2, num2);

        BN_add(bn1, bn1, bn2); // bn1 = bn1 + bn2

        char *result_str = BN_bn2dec(bn1);  // convert the BIGNUM back to string
        //printf("%s + %s = %s\n", num1, num2, result_str);
        OPENSSL_free(result_str);

        BN_free(bn1);
        BN_free(bn2);
        BN_CTX_free(ctx);

        return 0;
}
