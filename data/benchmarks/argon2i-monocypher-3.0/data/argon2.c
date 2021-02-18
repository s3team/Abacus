#include <stdlib.h>
#include "monocypher.h"

int main()
{
    uint8_t pass[5] = {0,1,2,3,4};

    uint8_t hash[32];                           /* Output hash     */
    uint8_t *password = pass;                    /* User's password */
    uint8_t password_size = 5;                      /* Password length */
    const uint8_t salt[16];                     /* Random salt     */
    const uint32_t nb_blocks = 8;          /* 100 megabytes   */
    const uint32_t nb_iterations = 1;           /* 3 iterations    */
    void *work_area = malloc(nb_blocks * 1024); /* Work area       */
    if (work_area == NULL)
    {
        /* Handle malloc() failure */
    }
    crypto_argon2i(hash, 32,
                   work_area, nb_blocks, nb_iterations,
                   password, password_size,
                   salt, 16);
    /* Wipe secrets if they are no longer needed */
    crypto_wipe(password, password_size);
   
    return 0;
}
