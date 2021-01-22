/*************************************************************************
	> File Name: Function.cpp
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/


#include <iostream>
#include <string>
#include <list>
#include <random>
#include "Function.hpp"
#include "ins_types.hpp"
#include "error.hpp"



using namespace std;

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)


namespace tana {
    Function::Function(ifstream &function_file) {
        string line;
        uint32_t num_fun = 0;
        while (function_file.good()) {
            getline(function_file, line);
            if (line.empty()) {
                continue;
            }

            istringstream strbuf(line);
            string addr, function_name, module_name, fun_size;
            getline(strbuf, addr, ';');
            getline(strbuf, module_name, ';');
            getline(strbuf, function_name, ';');
            getline(strbuf, fun_size, ';');


            auto func = std::make_shared<Routine>();
            if(function_name.front() == ' ')
            {
                function_name.erase(0,1);
            }

            func->rtn_name = function_name;
            func->module_name = module_name;
            func->start_addr = stoul(addr, nullptr, 16);
            func->size = stoul(fun_size, nullptr, 10);
            func->end_addr = func->start_addr + func->size;

            auto pos = existing_rtn.find(func->start_addr);
            if(pos != existing_rtn.end())
            { continue;}

            existing_rtn.insert(func->start_addr);
            fun_rtns.push_back(func);
            ++num_fun;
            if (!(num_fun % 1000)) {
                std::cout << "Parsing Functions: " << num_fun << std::endl;
            }
        }
        ptr_cache = fun_rtns.front();
    }

    Function::Function(const std::string &function_file_name)
    {
        ifstream function_file(function_file_name);
        if(!function_file.is_open())
        {
            ERROR("Open file failed!");
            exit(0);
        }

        string line;
        uint32_t num_fun = 0;
        while (function_file.good()) {
            getline(function_file, line);
            if (line.empty()) {
                continue;
            }

            istringstream strbuf(line);
            string addr, function_name, module_name, fun_size;
            getline(strbuf, addr, ';');
            getline(strbuf, module_name, ';');
            getline(strbuf, function_name, ';');
            getline(strbuf, fun_size, ';');


            auto func = std::make_shared<Routine>();
            func->rtn_name = function_name;
            func->module_name = module_name;
            func->start_addr = stoul(addr, nullptr, 16);
            func->size = stoul(fun_size, nullptr, 10);
            func->end_addr = func->start_addr + func->size;

            auto pos = existing_rtn.find(func->start_addr);
            if(pos != existing_rtn.end())
            { continue;}

            existing_rtn.insert(func->start_addr);
            fun_rtns.push_back(func);
            ++num_fun;
            if (!(num_fun % 1000)) {
                std::cout << "Parsing Functions: " << num_fun << std::endl;
            }
        }
        ptr_cache = fun_rtns.front();

    }

    std::string Function::getFunctionAndLibrary(tana_type::T_ADDRESS addr) {
        for (const auto& iter : fun_rtns) {
            if ((addr >= (iter->start_addr)) && (addr <= (iter->end_addr))) {
                return "Function Name: " + iter->rtn_name + " Module Name: " + iter->module_name + " Offset: "
                                         + std::to_string(addr - iter->start_addr);
            }
        }
        return "NOT Found";
    }

    std::string Function::getFunName(tana::tana_type::T_ADDRESS addr) {
        if ((addr >= (ptr_cache->start_addr)) && (addr <= (ptr_cache->end_addr))) {
            return ptr_cache->rtn_name;
        }
        for (const auto& iter : fun_rtns) {
            if ((addr >= (iter->start_addr)) && (addr <= (iter->end_addr))) {
                ptr_cache = iter;
                return iter->rtn_name;
            }
        }
        return "NOT Found";
    }

    std::shared_ptr<Routine> Function::getFunRoutine(tana::tana_type::T_ADDRESS addr){
        if ((addr >= (ptr_cache->start_addr)) && (addr <= (ptr_cache->end_addr))) {
            return ptr_cache;
        }
        for (const auto& iter : fun_rtns) {
            if ((addr >= (iter->start_addr)) && (addr <= (iter->end_addr))) {
                ptr_cache = iter;
                return iter;
            }
        }
        return nullptr;

    }

    std::shared_ptr<Routine> Function::pickOneRandomElement()
    {
        std::random_device random_device;
        std::mt19937 engine{random_device()};
        std::uniform_int_distribution<int> dist(0, fun_rtns.size() - 1);
        return fun_rtns[dist(engine)];

    }



}