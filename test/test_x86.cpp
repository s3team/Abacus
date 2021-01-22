#include <string>
#include <vector>
#include <iostream>


#include "x86.hpp"


using namespace std;
using namespace tana;

int main()
{
    vector<string> test_insn_data = {"mov", "movzx", "add", "sub", "dsffsd", "m"};

    for(const auto& str : test_insn_data)
    {
        auto res = x86::insn_string2id(str);
        std::cout << "Instruction ID: " << res << std::endl;
    }

    vector<string> test_reg_data = {"ebp", "ax", "bx", "dl", "das", "a"};

    for(const auto& str : test_reg_data)
    {
        auto res = x86::reg_string2id(str);
        auto reg_size = x86::get_reg_size(res);
        std::cout << "Register ID: " << res << " Size: " << reg_size<< std::endl;
    }

    vector<int> test_id = {-12, 0, 1, 2, 3, 5, 999, 12};

    for(const auto& id : test_id)
    {
        auto insn_id = static_cast<x86::x86_insn>(id);
        auto insn = x86::insn_id2string(insn_id);
        std::cout << "Instruction name: " << insn  <<std::endl;
    }

    for(const auto& id : test_id)
    {
        auto reg_id = static_cast<x86::x86_reg>(id);
        auto insn = x86::reg_id2string(reg_id);
        std::cout << "Instruction name: " << insn << std::endl;
    }
}