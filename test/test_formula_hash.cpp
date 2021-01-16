
#include<iostream>
#include "BitVector.hpp"

using namespace std;
using namespace tana;


int main (){
  auto v0 = std::make_shared<BitVector>(ValueType::SYMBOL, 1);
  auto v1 = std::make_shared<BitVector>(ValueType::SYMBOL, 2);
  auto v2 = std::make_shared<BitVector>(ValueType::SYMBOL, 2);
  auto v6 = std::make_shared<BitVector>(ValueType::SYMBOL, 2);



  auto v3 = buildop2(BVOper::bvadd, v0, v1);
  auto v4 = buildop2(BVOper::bvadd, v3, v2);
  auto v5 = buildop2(BVOper::bvadd, v4, v6);


  auto result = v5->get_bitvector_hash();

  return 0;
}
