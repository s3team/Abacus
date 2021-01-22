#include <stdlib.h>
#include "monocypher.h"

int main()
{
    uint8_t sk[32];       /* Random secret key */
    
    for (int i = 0; i < 32; ++i)
    {
        sk[i] = i;
    }
    uint8_t pk[32];       /* Public key        */
    crypto_key_exchange_public_key(pk, sk);
    /* Wipe secrets if they are no longer needed */
    crypto_wipe(sk, 32);

    return 0;
}
