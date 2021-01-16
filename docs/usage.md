# Usage

Abacus consists of two components, a pintool that is used to collect the execution trace, and
the offline executor that analyze the trace file and the binary executable to identify and
quantify the amount of each leakage site.

## The Pintool
If we use the `head` command to print the top 5 lines of the trace file `Inst_data.txt` in the 
previous example, then we can find the first two lines record the start address, the length,
and the value of the secret data. 

```
Start; 0xffb4d66a; 16;
6; 0;
804895c;call 0x8048aab;80ac04c,80d9000,ffb4d690,6,80d9000,80481a8,ffb4d650,ffb4d678,ffb4d64c,246,8049818
8048aab;mov eax, dword ptr [esp];80ac04c,80d9000,ffb4d690,6,80d9000,80481a8,ffb4d64c,ffb4d678,ffb4d64c,246,8048961
8048aae;ret ;8048961,80d9000,ffb4d690,6,80d9000,80481a8,ffb4d64c,ffb4d678,ffb4d64c,246,8048961
```
We have two ways to mark the secret. The first approach needs to add an annotation on the source
code. The second approach doesn't need to change the source code. But it needs to adjust the pin
tool.

### Marking The Secret Input
#### Method 1: Adding An Annotation On The Source Code
The first step is to mark the secret input. So Abacus can know which variable or buffer is the
secret. In the previous example, we use the function `abacus_make_symbolic` to mark the secret.
The approach is similar to other symbolic execution engine like KLEE.

#### Method 2: Telling Pintool The Address Of The Secret Data

However, we can also achieve the same goal without changing the source code. We can do it by using 
a new Pintool and passing the start address of the secret data and its length to Pintool. 
Let's explain it with a concrete example.

The below example encrypts data `plaintext` with the DES-ECB mode and stores the result in `ciphertext`.
Here the secret data is "key". One ways is to disable ASLR and get the address by reverse engineering.
However, we use a different approach. 

```C
#include <stdio.h>
#include <string.h>
#include <mbedtls/des.h>

static const unsigned char key[MBEDTLS_DES_KEY_SIZE] = { 0x80, 0x4a, 0x57, 0x54, 0x40, 0x83, 0x73, 0xb2 };

int main()
{
  unsigned char plaintext[1024] = "hello world";
  unsigned char ciphertext[1024];
  unsigned char dec_out[8];
  mbedtls_des_context ctx;
  mbedtls_des_init(&ctx);
  memset(plaintext, 0x07, sizeof(plaintext));
  memset(ciphertext, 0x01, sizeof(ciphertext));
  memset(dec_out, 0x02, sizeof(dec_out));
  int ret = mbedtls_des_setkey_enc(&ctx, key);
  if(ret != 0) {
     printf("Failed to set DES key for encrpytion\n");
     return -1;
  }
  ret = mbedtls_des_crypt_ecb(&ctx, plaintext, ciphertext);
  return 0;
}
```

We notice that function `mbedtls_des_setkey_enc` takes `key` as the second input argument. So we can use the
function `RTN_InsertCall` to instrument the binary exectubale at the run-time and get the address. Also, the
length of `key` is known.

```C
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) RTN_start,
               IARG_ADDRINT, function_name.c_str(),
               IARG_ADDRINT, IMG_Name(IMG_FindByAddress(rtn_start)).c_str(),
               IARG_FUNCARG_ENTRYPOINT_VALUE, 1,    // Key Address
               IARG_UINT32, 64,                     // The Length
               IARG_END);
```

## The Offline Executor
`QIF` takes the below arguments. However, only `traces.txt` is required.

~~~~{.sh}
Usage: ./QIF <traces.txt> <options>
-o <file>                               Place the output into <file>
-f <Function Name>                      The location of the function file
-d <Debug Info> -f <Function Name>      The location of the ELF and the location of the function name
~~~~ 
