#include <iostream>
#include <bitset>
#include "BitVector.hpp"


using namespace std;

int main()
{
    uint32_t num = 0xffffffff;
    bool test = (num & (0x80000000u)) >> 31u;

    cout << test << endl;

    uint32_t eax = 0xf;
    uint32_t edx = 0xffff;

    tana::BitVector::bvidiv32_quo(edx, eax, 1);

    return 1;

}
