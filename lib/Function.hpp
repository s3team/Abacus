/*************************************************************************
	> File Name: Function.h
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include "ins_types.hpp"

namespace tana {

    class Routine {
    public:
        tana_type::T_ADDRESS start_addr; // start address of the function
        tana_type::T_ADDRESS end_addr;   // end address of the function
        std::string rtn_name;
        std::string module_name;
        tana_type::T_ADDRESS size;

        Routine() : start_addr(0), end_addr(0), rtn_name(), module_name(), size(0) {}

        Routine(tana_type::T_ADDRESS p1, tana_type::T_ADDRESS p2, \
                const std::string &p3, const std::string &p4, \
                tana_type::T_ADDRESS p5): start_addr(p1), end_addr(p2), rtn_name(p3),
                                          module_name(p4), size(p5)
                {}
    };

	class Function {
	private:
		std::vector<std::shared_ptr<Routine>> fun_rtns;
		std::set<tana_type::T_ADDRESS> existing_rtn;

        std::shared_ptr<Routine> ptr_cache;



    public:
		explicit Function(std::ifstream &function_file);

		explicit Function(const std::string& function_file);

		std::string getFunctionAndLibrary(tana_type::T_ADDRESS addr);

		std::string getFunName(tana_type::T_ADDRESS addr);

		std::shared_ptr<Routine> getFunRoutine(tana_type::T_ADDRESS addr);

		std::shared_ptr<Routine> pickOneRandomElement();

		~Function() = default;
	};

}

