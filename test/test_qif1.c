/*************************************************************************
	> File Name: test_qif1.c
	> Author: 
	> Mail: 
	> Created Time: Tue May  7 11:01:10 2019
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>

void encrypt(char *output, char * key, uint32_t len)
{
    if(key[0] % 2 == 0)
    {
        output[0] = 0xb;
    } else
    {
        output[0] = 0xf;
    }

    if(key[1] % 2 == 0)
    {
        output[1] = 0xb;
    } else
    {
        output[1] = 0xf;
    }

}

int main(){
    char key[2];
    key[0] = 5;
    key[1] = 6;
    char output[2];

    encrypt(output, key, 2);
    printf("%d mod %d = %d\n", 10, 4, 10%4);
    return 1;
}

