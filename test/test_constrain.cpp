/*************************************************************************
	> File Name: test_constrain.cpp
	> Author: 
	> Mail: 
	> Created Time: Tue Apr 23 12:21:10 2019
 ************************************************************************/

#include<iostream>
#include "BitVector.hpp"
#include "Constrains.hpp"

using namespace std;
using namespace tana;

int main() {
    std::shared_ptr<BitVector> bit_num = std::make_shared<BitVector>(ValueType ::SYMBOL, "K1");

    Constrain c = Constrain(1, bit_num, BVOper ::greater, 123);

    cout << c;

    return 1;


}
