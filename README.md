# Abacus

![Ubuntu](https://github.com/1c0e/Abacus/workflows/Ubuntu/badge.svg)
![macOS](https://github.com/1c0e/Abacus/workflows/macOS/badge.svg)

***Please note that Abacus is not production ready and is still under active developement.***
## Introduction

Abacus is an address-based side-channel vulnerabilities detection tool. Different from previous tools, it can also 
give an estimation of the amount of the leaked information for each leakage site.

## Getting Started

### Build Intructions
**Supported OS**: Linux 18.04, Ubuntu 20.04

**Prerequistes**: 
-  cmake 3.12 or newer
-  pkg-config
-  gcc-7

Abacus is consisted of two components:
-  A Pin tool
-  The offline executors
#### PinTool
The Pintool is located in the `pintool` folder:
~~~~{.sh}
$ cd pintool
$ make TAGERT=ia32 PIN_ROOT=<path to Pin kit>
~~~~

To learn more about Intel Pin, please refer <https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool>

You will get an `.so` library for each pin tool.

#### The Offline Executors
~~~~{.sh}
$ mkdir build
$ cd build
$ cmake ..
$ make all
~~~~

## A Hello World Example
In the hello word example, we will guide you through the main steps needed to test a simple function
with Abacus.


```C
void is_odd(uint32_t secret) {
  int res = secret % 2;        
  if (res) {             
    printf("Odd Number\n");   // Branch 1
  } else {
    printf("Even Number\n");  // Branch 2
  }
}
```
It is a simple function. It takes a 32 bit integer as the secret input and check the last digit
of the integer. As an attacker, he can know the last digit bit of the input integer by observing
which branch is actually executed. So in the above function, we think the code has one 
secret-dependent control-flow vulnerability and it can leak one bit of information. 

The source code of the example is located at `examples\example1.c`.

#### Marking secret data as symbolic
In order to test this function with Abacus, we need to tell Abacus which variable is the secret
data. We use the function `abacus_make_symbolic`. The function takes three arguments: the type of
the symbolic, the address of the secrest, and the length of the secret input. In the below example,
the secret input is the variable `secret`, and its length is two bytes.

```C
int main() {
  uint16_t secret = 6;
  // uint8_t secret = 8;
  char *type = "1";
  abacus_make_symbolic(type, &secret, 2);   // The length of the key is two bytes
  is_odd(secret);
  return 0;
}
```

#### Build the example
Abacus works directly on the machine instruction. Here we build it into a 32-bit ELF exectuable.
Note while Abacus can work on stripped bianries without the source code. However, in the example, 
we use debug informmation to get a more clear result.
~~~~{.sh}
$ cd examples
$ gcc -m32 -g example1.c
~~~~

#### Collect the trace
We use the pin tool to collect the execution trace. The tool can automatically collect the trace and other
necessary runtime information.
~~~~{.sh}
$ pin -t Pintools/obj-ia32/MyPinToolLinux.so -- ./examples/a.out 
~~~~
You will get two files `Function.txt` and `Inst_data.txt`, which are the inputs of the Abacus.

#### Quantify the leakage 
To run Abacus on the execution trace and get the analysis result.
~~~~{.sh}
$ ./build/App/QIF/QIF ./Inst_data.txt -f Function.txt -d ./examples/a.out  -o result.txt
~~~~

You should see the following output:
```
Start Computing Constraints
Total Constraints: 1
Control Transfer: 1
Data Access: 0
Information Leak for each address:
Address: 5664259b Leaked:1.0 bits Type: CF  Num of Satisfied: 2541 Total Sampling Numbers: 5000
Source code: ./examples/a.out: /data/shoujiki/examples/example1.c line number: 13
Function Name:  is_odd Module Name:  /data/shoujiki/examples/a.out Offset: 30

1.0
Time taken by SE: 0.8 seconds
Time taken by QIF: 0.8 seconds
```
As expected, it shows that the function `is_odd` has one secret-dependent control-flows at line `13`.
Also, Abacus show the vulnerability leaks 1 bit information.

A more detailed report can be found at `result.txt`

```
Total Constrains: 1
Control Transfer: 1
Data Access: 0
START:
Address: 5664259b Leaked:1.0 bits Type: CF  Num of Satisfied: 2541 Total Sampling Numbers: 5000
Source code: ./examples/a.out: /data/shoujiki/examples/example1.c line number: 13
DETAILS:
------------------------------------------------------------
Address: 5664259b Leaked:1.0 bits Type: CF  Num of Satisfied: 2541 Total Sampling Numbers: 5000
Source code: ./examples/a.out: /data/shoujiki/examples/example1.c line number: 13
Function Name:  is_odd Module Name:  /data/shoujiki/examples/a.out Offset: 30

Number of m_constrains: 1
Instruction Addr: 1d: equal(bvsub(bvand(bvzeroext(bvconcat(bvextract(bvzeroext(bvconcat((SYM_ID15, Key1),(SYM_ID9, Key0))),9,16),bvextract(bvzeroext(bvconcat((SYM_ID15, Key1),(SYM_ID9, Key0))),1,8))),0x1),0x0),0x0) == 0x1 Input Symbol Number: 2------------------------------------------------------------
Total Instructions:: 68165
Time taken by SE: 0.781423 seconds
Time taken by QIF: 0.787265 seconds
```

## Documentation
If you are interested in the tool, you can find more details about Abacus in the documentation.
* [More Examples](https://github.com/1c0e/Abacus/blob/master/docs/examples.md)
* [Usage](https://github.com/1c0e/Abacus/blob/master/docs/usage.md)
* [Reproduce the Result](https://github.com/1c0e/Abacus/blob/master/docs/result.md)
* [Descritption](https://github.com/1c0e/Abacus/blob/master/docs/description.md)

## Contributing
The paper is still under review for a double-blinded conferecne. At the present, we do not accept any pull requests.
However, feature requests are welcome.
