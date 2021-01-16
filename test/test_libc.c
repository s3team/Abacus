/*************************************************************************
	> File Name: test_libc.c
	> Author: 
	> Mail: 
	> Created Time: Wed Feb  5 16:35:11 2020
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>


void test_calloc(int i)
{
    int * pData;

    pData = (int*)calloc(i, sizeof(int));

    printf("pData = %p\n", pData);

}


int main(){
    int i = 10, n;

    test_calloc(i);

    free(pData);

    return 0;
}

