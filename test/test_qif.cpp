#include <stdio.h>
#include <stdint.h>



void encrypt(char *output, char * key, uint32_t len)
{
    char temp[len];
    int i, j;
    for(i = 0; i < len; ++i)
    {
        temp[i] = key[i] + 3;
    }

    for(j = 0; j < len; ++j)
    {
        if(temp[j] > 'b') {
            output[j] = 1;
        } else {
            output[j] = 0;
        }
    }

}

int main()
{
    char key[4];
    key[0] = 'a';
    key[1] = 'b';
    key[2] = 'c';
    key[3] = 'd';
    char output[4];

    encrypt(output, key, 4);

    return 1;
}