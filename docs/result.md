# Reproduce Result

- Benchmarks: `data/benchmarks`
- Raw result: `data/matadata1.tar`
- Pin tools: `Pintools/CryptoLibrary`

You can reproduce the result in the following steps.

1. Build the pin tool.
2. Collect the execution trace.
3. Run the offline executor and get the result.

We use an example to illustrace the process.

## AES mbedTLS 2.15.1

### Build the pin tool

~~~{.sh}
cd Pintools/CryptoLibrary
make PIN_ROOT=<PATH TO PIN KIT> TARGET=ia32
~~~

### Collect the execution trace

~~~{.sh}
pin -t Pintools/CryptoLibrary/obj-ia32/AES-mbedTLS-2.15.1.so -- benchmarks/AES-mbedTLS-2.15.1/aes_128_cbc
~~~

### Run the offline executor

~~~{.sh}
QIF ./Inst_data.txt -f Function.txt -d benchmarks/AES-mbedTLS-2.15.1/aes_128_cbc -o result.txt
~~~
