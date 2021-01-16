
#include<iostream>
#include "BitVector.hpp"


using namespace std;
using namespace tana;


int main(){
    uint32_t a = 0xfffffff0;
    uint32_t b = 0xfffffff0;

    uint32_t high = BitVector::bvmul32_h(a, b);
    uint32_t low = BitVector::bvmul32_l(a, b);

    uintmax_t t = a*b;

    cout << hex << a << "*" << b << "\n";
    cout << "High: " << hex << high << dec << endl;
    cout << "Low: " << hex << low << dec << endl;
    cout << "True: " << hex << t << dec;
}
