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
    int i, j;
    for(i = 0; i < len; ++i)
    {
        if(key[i] > 6)
        {
            output[0] = output[0] + 1;
        }

        if(key[i] == 2)
        {
            output[1] = output[1] - 1;
        }
    }

}

int main(){
    char key[] = {1,2,3,4,5,6,7,8,9,10,11,12};
    char output[2] = {0, 0};

    encrypt(output, key, 12);
    printf("%d mod %d = %d\n", 10, 4, 10%4);
    return 1;
}

