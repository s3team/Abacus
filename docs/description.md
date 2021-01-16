# Description

## Motivation
There are too many side-channel leakages. We want to have a tool
that can quantify each leakage site and identify those severe leakages.

## Quantification
We quantify the amount of each leakage site based on the search space.
For example, if the length of the key is 128 bits, an attacker needs to
brute force 2^128 possible keys without any domain knowledge. However, 
supposed an attacker observe some infomration to reduce the serach space
to 2^120. Then we can conclude 8 bits of the information are leaked. 

## Binary Analysis
Abacus works on the machine intruction level. So it can indentify 
side-channel leakages introduced by compilers.

## Tested Libraries
We have tested Abacus successfully on the following libraries:

* OpenSSL: 0.9.7, 1.0.2f, 1.0.2k, 1.1.0f, 1.1.1, 1.1.1g
* MbedTLS: 2.5, 2.15
* Libgcrypt: 1.8.5
* Monocyper: 3.0

