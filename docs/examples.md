# Second Example

In this example, we will show you how to quantify the address-based
side-channels with Abacus. Besides, we will demonstrate Abacus with concrete
examples to show how Abacus detect the side-channel leakage.
You can find the example in the source tree under `examples/example2.c`.

## Example

Here is the example. 

```C
uint8_t LookUpTable[64] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                            1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                            1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                            1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};

void example1(uint8_t input) { // Note that while the input key is 16 bits, this
                               // function only receives 8 bit of the key
  uint8_t key = input;
  uint32_t a = key, b;
  b = a >> 8;
  if (b) {
    printf("branch 1\n");
  } else {
    printf("branch 2\n");
  }
}

int example2(uint8_t input) {
  uint8_t key = input;
  uint32_t a = key, b;
  b = a >> 7;
  if (b) {
    printf("branch 1\n");
  } else {
    printf("branch 2\n");
  }
}

// If the input secret is less than 8, then the leakage will be more serious
// than the secret is larger than 8
int example3(uint8_t secret) {
  if (secret < 8) {
    printf("branch 1\n"); // If an attacker observes that the branch 1 is
                          // executed, it means the 5 bits are zero
  } else {
    printf("branch 2\n");
  }
  if (secret < 128) {
    printf("branch 3\n"); // one bit is zero.
  } else {
    printf("branch 4\n");
  }
  return 0;
}

int example4(uint16_t secret) {
  if (secret < 16) {
    printf("branch 1"); // A serious leakage, the top 12 bits are leaked
  }
  return 0;
}

uint64_t example5(uint16_t secret) {
  int index = secret % 64;
  uint64_t res = LookUpTable[index];
  return res;
}

uint64_t example6(uint16_t secret) {
  int res;
  if (secret > 8) {
    res = 0;
  } else {
    res = 1;
  }
  return res;
}

int main() {
  uint16_t secret = 6;
  // uint8_t secret = 8;
  char *type = "1";
  abacus_make_symbolic(type, &secret, 2); // The length of the key is two bytes
  example1(secret);
  example2(secret);
  example3(secret);
  example4(secret);
  uint64_t res = example5(secret);
  res += example6(secret);
  return res;
}

```

The first step is to compile the source code into the binary executable.

### Build the example

```bash
gcc -m32 -O3 -g example2.c
```
Different from the hello world example, we use `O3` to turn on the compiler
optimization. Because most software also uses the release build. 

### Run Abacus
First, we collect the executation trace.
```bash
pin -t Pintools/obj-ia32/MyPinToolLinux.so -- ./examples/a.out 
```
Then we run the tool to identify and quantify each leakage site.

```bash
./build/App/QIF/QIF ./Inst_data.txt -f Function.txt -d ./examples/a.out -o result.txt
```
Here is the result.
```
Start Computing Constraints
Total Constraints: 4
Control Transfer: 4
Data Access: 0
Information Leak for each address: 
Address: 80489a0 Leaked:1.0 bits Type: CF  Num of Satisfied: 2501 Total Sampling Numbers: 5000
Source code: ./examples/a.out: /data/qub14/shoujiki/examples/example2.c line number: 32
Function Name:  example2 Module Name:  /data/qub14/shoujiki/examples/a.out Offset: 20

Address: 80489e8 Leaked:5.0 bits Type: CF  Num of Satisfied: 160 Total Sampling Numbers: 5000
Source code: ./examples/a.out: /data/qub14/shoujiki/examples/example2.c line number: 42
Function Name:  example3 Module Name:  /data/qub14/shoujiki/examples/a.out Offset: 24

Address: 8048a00 Leaked:1.0 bits Type: CF  Num of Satisfied: 2523 Total Sampling Numbers: 5000
Source code: ./examples/a.out: /data/qub14/shoujiki/examples/example2.c line number: 48
Function Name:  example3 Module Name:  /data/qub14/shoujiki/examples/a.out Offset: 48

Address: 80482f7 Leaked:11.4 bits Type: CF  Num of Satisfied: 11 Total Sampling Numbers: 30000
Source code: ./examples/a.out: /data/qub14/shoujiki/examples/example2.c line number: 57
Function Name:  example4 Module Name:  /data/qub14/shoujiki/examples/a.out Offset: 114
```

Abacus detects four leakages in total. You may wonder why the above program has four leakages.
Let's analyze the result step by step.

### Example 1
```C
void example1(uint8_t input) { 
  uint8_t key = input;
  uint32_t a = key, b;
  b = a >> 8;
  if (b) {
    printf("branch 1\n");
  } else {
    printf("branch 2\n");
  }
}
```
Abacus reports no leakage in the above function. The above function take 8 bits of the secret data. However, we
notice the value `b` should aways equal to 0. So it is not a secret-dependent control-flow.

### Example 2
```C
int example2(uint8_t input) {
  uint8_t key = input;
  uint32_t a = key, b;
  b = a >> 7;
  if (b) {
    printf("branch 1\n");
  } else {
    printf("branch 2\n");
  }
}
```
Abacus reports 1.0 bit of information can be leaked at the if branch. Different from the example 1, the highest digit
of the `input` can affect the value of `b`. As an attacker, he can know one bit in the original buffer is `0` or `1` by
observing the branch. So we think it is a one-bit leakage.

### Example 3
```C
int example3(uint8_t secret) {
  if (secret < 8) {.      // Line 42
    printf("branch 1\n"); // If an attacker observes that the branch 1 is
                          // executed, it means the 5 bits are zero
  } else {
    printf("branch 2\n");
  }
  if (secret < 128) {.    // Line 48
    printf("branch 3\n"); // one bit is zero.
  } else {
    printf("branch 4\n");
  }
  return 0;
}
```
Abacus reports there are two leakage sites in function `example3`. Moreover, it reports the leakage at line 42 is more dangerous
than the leakage at line 48. It is reasonable. At line 42, if an attacker observes the code hit branch 1, then he can know the 
top five digits of the `secret` are zeros. At line 48, he can know the highest digit is zero.


### Example 4
```C
int example4(uint16_t secret) {
  if (secret < 16) {
    printf("branch 1"); // A serious leakage, the top 12 bits are leaked
  }
  return 0;
}
```
Abacus reports that 11.4 bits of the information are leaked.

### Example 5
```C
uint64_t example5(uint16_t secret) {
  int index = secret % 64;
  uint64_t res = LookUpTable[index];  // Line 65
  return res;
}
```
Abacus doesn't find a leakage here. However, it is supposed to be a secret-dependent data access at line 65.
The reason is that the default granularity of Abacus is a cache line. In which case, the size is 64 bytes.
Because the array `LookUpTable` can be filled in one single cache line. So Abacus does not think it is a
secret-dependent data access. We can change the default settings of Abacus at `src/include/ins_types.hpp`.
```C++
namespace config_parameter {
//const static uint32_t DATA_GRANULARITY = 6;
const static uint32_t DATA_GRANULARITY = 1;
}
```
After that, we rebuild the Abacus and run the experiments. Then we can get the expected result.
```
Address: 80482fe Leaked:5.2 bits Type: DA  Num of Satisfied: 751 Total Sampling Numbers: 25000
Source code: ./examples/a.out: /data/shoujiki/examples/example2.c line number: 65
Function Name:  example5 Module Name:  /data/shoujiki/examples/a.out Offset: 121
```

### Example 6
```C
uint64_t example6(uint16_t secret) {
  int res;
  if (secret > 8) {
    res = 0;
  } else {
    res = 1;
  }
  return res;
}
```
Abacus doesn't report a leakage here. Desipte it seems to have a secret-dependent 
control-flow transfer. The reason is the compiler. If we disassemble the binary
executable, then we will find the compiler has remove the branch.

```
 8048a9f:	31 c0                	xor    %eax,%eax
 8048aa1:	66 83 7c 24 04 08    	cmpw   $0x8,0x4(%esp)
 8048aa7:	0f 96 c0             	setbe  %al
 8048aaa:	31 d2                	xor    %edx,%edx
 8048aac:	c3                   	ret       
```
