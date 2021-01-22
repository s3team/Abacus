#include <stdio.h>
#include <string.h>
#include <stdint.h>

// From https://raw.githubusercontent.com/anthonywei/rc4/master/c/rc4.c
// Major mods by gregz

typedef unsigned long ULONG;

void rc4_init(unsigned char *s, unsigned char *key, unsigned long Len) //初始化函数
{
    int i =0, j = 0;
    unsigned char k[256] = {0};
    unsigned char tmp = 0;
    for (i=0;i<256;i++) {
        s[i] = i;
        k[i] = key[i%Len];
    }
    for (i=0; i<256; i++) {
        j=(j+s[i]+k[i])%256;
        tmp = s[i];
        s[i] = s[j]; //交换s[i]和s[j]
        s[j] = tmp;
    }
 }

void rc4_crypt(unsigned char *s, unsigned char *Data, unsigned long Len) //加解密
{
    int i = 0, j = 0, t = 0;
    unsigned long k = 0;
    unsigned char tmp;
    for(k=0;k<Len;k++) {
        i=(i+1)%256;
        j=(j+s[i])%256;
        tmp = s[i];
        s[i] = s[j]; //交换s[x]和s[y]
        s[j] = tmp;
        t=(s[i]+s[j])%256;
        Data[k] ^= s[t];
     }
} 

void print_hex(char* label, uint8_t* data, size_t len) {
    printf("%s", label);
    for(size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}


int main()
{ 
    unsigned char s[256] = {0}; //state
    unsigned char key[10]; 
    unsigned char pData[53];
    ULONG len = (ULONG) sizeof(pData);

    memset(key, 0xaa, sizeof(key));
    memset(pData, 0x04, sizeof(pData));
#if DEBUG
    print_hex("key ", key, sizeof(key));
    print_hex("raw ", pData, sizeof(pData));
#endif
    
    rc4_init(s,(unsigned char *)key,sizeof(key)); //已经完成了初始化
    /*
    int i;
    for (i=0; i<256; i++)
    {
        printf("%-3d ",s[i]);
    }
    printf("\n");
    */
    rc4_crypt(s,(unsigned char *)pData,len);//加密
#if DEBUG
    print_hex("encrypt ", pData, len);
#endif
    rc4_init(s,(unsigned char *)key, strlen(key)); //初始化密钥
    rc4_crypt(s,(unsigned char *)pData,len);//解密
#if DEBUG
    print_hex("decrypt ",pData, len);
#endif
    return 0;
}
