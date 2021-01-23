# Reproduce Result

- Benchmarks: `data/benchmarks`
- Raw result: `data/matadata1.tar`
- Pin tools: `Pintools/CryptoLibrary`

You can reproduce the result in the following steps.

1. Build the pin tool.
2. Collect the execution trace.
3. Run the offline executor and get the result.

We use an example to illustrate the process.

## Build


~~~{.sh}
git clone https://github.com/s3team/Abacus.git
cd Abacus
./docker.sh
./build.sh
~~~

It can automatically build Abacus and Pin Tools.

## Run Experiments
We use AES in mbedTLS 2.5 as an example to illustrate the steps.
~~~{.sh}
cd /abacus/script/AES_MBEDTLS_2.5
./start.sh
~~~
