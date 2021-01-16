#include <stdio.h>
#include <stdint.h>

static const uint8_t T[16] = {
    0x52U,
    0x09U,
    0x6aU,
    0xd5U,
    0x30U,
    0x36U,
    0xa5U,
    0x38U,
    0xbfU,
    0x40U,
    0xa3U,
    0x9eU,
    0x81U,
    0xf3U,
    0xd7U,
    0xfbU,
};

void encrypt(uint32_t *output, uint32_t *key, uint32_t len)
{
    for (int i = 0; i < len / 4; i+=4)
    {
        output[i] = (T[(key[i] >> 24)] << 24) ^
                    (T[(key[i + 1] >> 16) & 0xff] << 16) ^
                    (T[(key[i + 2] >> 8) & 0xff] << 8) ^
                    (T[(key[i + 3]) & 0xff]);
    }
}

int main()
{
    // char key[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    static const uint32_t key[16] = {
        0xc66363a5U,
        0xf87c7c84U,
        0xee777799U,
        0xf67b7b8dU,
        0xfff2f20dU,
        0xd66b6bbdU,
        0xde6f6fb1U,
        0x91c5c554U,
        0x60303050U,
        0x02010103U,
        0xce6767a9U,
        0x562b2b7dU,
        0xe7fefe19U,
        0xb5d7d762U,
        0x4dababe6U,
        0xec76769aU,
    };
    uint32_t output[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    encrypt(output, key, 16);
    int i, res = 0;
    for (i = 0; i < 16; ++i)
    {
        res += output[i];
    }
    return res;
}
