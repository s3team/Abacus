#include <stdio.h>
#include <stdint.h>

static const uint32_t T[4] = {
    0xc66363a5U,
    0xf87c7c84U,
    0xee777799U,
    0xf67b7b8dU,
};

void encrypt(char *output, char *key, uint32_t len)
{
    int i, j;
    for (i = 0; i < len; ++i)
    {
        output[i] = T[key[i]];
    }
}

int main()
{
    char key[] = {0, 1, 2, 3};
    char output[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    encrypt(output, key, 4);
    int i, res = 0;
    for (int i = 0; i < 12; ++i)
    {
        res += output[i];
    }
    return res;
}
