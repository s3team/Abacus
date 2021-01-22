/*************************************************************************
	> File Name: Tainter.h
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#pragma once

#include <list>
#include <iostream>
#include <set>
#include <algorithm>
#include "ins_types.hpp"
#include "Register.hpp"

namespace tana {
    class Tainter {
    private:
        std::list<tana_type::T_ADDRESS> taintedAdress;
        bool taintedRegisters[GPR_NUM][REGISTER_SIZE];


    public:
        Tainter(tana_type::T_ADDRESS, tana_type::T_SIZE m_size);

        void taint(tana_type::T_ADDRESS addr);

        void taint(tana_type::T_ADDRESS addr, tana_type::T_SIZE size);

        void taint(x86::x86_reg reg);

        void untaint(tana_type::T_ADDRESS addr);

        void untaint(x86::x86_reg reg);

        void untaint(tana_type::T_ADDRESS addr, tana_type::T_SIZE size);

        bool isTainted(tana_type::T_ADDRESS addr);

        bool isTainted(x86::x86_reg reg);

        std::list<tana_type::T_ADDRESS> getTaintedAddress();

        // Imediate to Memory
        void spreadTaintImediate2Memory(tana_type::T_ADDRESS addr, tana_type::T_SIZE m_size);

        // Imediate to Register
        void spreadTaintImediate2Register(x86::x86_reg reg);

        // Register to Register
        void spreadTaintRegister2Register(x86::x86_reg dest, x86::x86_reg src);

        // Memory to m_memory
        void spreadTaintMemory2Memory(tana_type::T_ADDRESS ip_addr, tana_type::T_ADDRESS src, tana_type::T_ADDRESS dest,
                                      tana_type::T_SIZE m_size);

        // Register to Memory
        void spreadTaintRegister2Memory(tana_type::T_ADDRESS ip_addr, x86::x86_reg reg, tana_type::T_ADDRESS addr);

        // Memory to Register
        void spreadTaintMemory2Register(tana_type::T_ADDRESS ip_addr, tana_type::T_ADDRESS addr, x86::x86_reg reg);

        void taintIns(Inst_Base &ins);

        void cleanALL();

    };
};