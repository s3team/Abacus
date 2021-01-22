#include <iostream>
#include <bitset>
#include "BitVector.hpp"

using namespace tana;

void test_concat()
{
    uint32_t num1 = 0b11111111111;
    uint32_t num2 = 0b1010;

    auto bit1 = std::bitset<tana::REGISTER_SIZE>(num1);
    auto bit2 = std::bitset<tana::REGISTER_SIZE>(num2);

    std::cout << "Test concat" << std::endl;
    std::cout << "num1:   " << bit1 << std::endl;
    std::cout << "num2:   " << bit2 << std::endl;

    auto res = tana::BitVector::concat(num1, num2, 25, 7);

    auto bit_res = std::bitset<tana::REGISTER_SIZE>(res);

    std::cout << "Result:     " << bit_res << std::endl;

}

void test_substr()
{
    uint32_t num1 = 0b101111;
    uint32_t num2 = 0b1010;
    uint32_t num1_size = 10;
    uint32_t num2_size = 5;
    auto bit1 = std::bitset<tana::REGISTER_SIZE>(num1);
    auto bit2 = std::bitset<tana::REGISTER_SIZE>(num2);

    std::cout << "Test substr" << std::endl;
    std::cout << "num1:   " << bit1 << std::endl;
    std::cout << "num2:   " << bit2 << std::endl;

    std::string bit1_str = bit1.to_string();
    std::string bit2_str = bit2.to_string();

    auto bit1_str_size = bit1_str.size();
    auto bit2_str_size = bit2_str.size();

    std::cout << "String:" << std::endl;
    std::cout << "num1:   " << bit1_str << std::endl;
    std::cout << "num2:   " << bit2_str << std::endl;

    std::cout << "Sub String:" << std::endl;
    std::cout << "num1:   " << bit1_str.substr(bit1_str_size-num1_size, bit1_str_size) << std::endl;
    std::cout << "num2:   " << bit2_str.substr(bit2_str_size-num2_size, bit2_str_size) << std::endl;

    auto c = bit1_str[bit1_str_size - 5];

}


void test_signext()
{
    uint32_t num1 = 0b101010;
    auto bit1 = std::bitset<tana::REGISTER_SIZE>(num1);
    std::cout << "Sign Extension" << std::endl;
    std::cout << "num1:   " << bit1 << std::endl;

    uint32_t result = tana::BitVector::signext(num1, 6, 32);
    auto bit_result = std::bitset<tana::REGISTER_SIZE>(result);
    std::cout << "After:  " << bit_result << std::endl;

}



int main()
{

    uint32_t num =  0b0101;
    auto res = tana::BitVector::extract(num, 4, 2);
    auto bit_before = std::bitset<tana::REGISTER_SIZE>(num);
    auto bit_after = std::bitset<tana::REGISTER_SIZE>(res);

    bool test1 = bit_before[0];
    bool test2 = bit_before[1];
    bool test3 = bit_before[2];

    bool test4 = BitVector::bit(num, 0);
    bool test5 = BitVector::bit(num, 1);
    bool test6 = BitVector::bit(num, 2);


    std::cout << "Test extract" << std::endl;
    std::cout << "Before: " << bit_before << std::endl;
    std::cout << "After:  "<< bit_after << std::endl;

    test_concat();
    test_substr();
    test_signext();
    return 1;
}
