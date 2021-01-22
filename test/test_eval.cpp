/*************************************************************************
	> File Name: test_eval.cpp
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Sun Oct  6 22:00:47 2019
 ************************************************************************/

#include <iostream>
#include <memory>
#include "BitVector.hpp"
#include "Engine.hpp"

using namespace std;
using namespace tana;

int main(){

    auto v0 = std::make_shared<BitVector>(ValueType::CONCRETE, 1);
    auto v1 = std::make_shared<BitVector>(ValueType::CONCRETE, 2);

    auto v2 = buildop2(BVOper::bvadd, v0, v1);

    auto v3 = std::make_shared<BitVector>(ValueType::CONCRETE, 2);

    auto v4 = buildop2(BVOper::bvadd, v2, v3);

    auto v5 = buildop2(BVOper::bvsar, v4, v3);

    auto v6 = buildop2(BVOper::bvconcat, v5, v4);

    auto v7 = buildop3(BVOper::bvand3, v5, v4, v6);



    map<int, uint32_t > fake;

    for(int i = 0; i < 100000; ++i) {

        SEEngine::eval(v6, fake);
        SEEngine::eval_fast(v6, fake);

    }
    return 1;
}

