#include<iostream>
#include "Trace2ELF.hpp"
#include "InputParser.hpp"
#include "Function.hpp"
#include "Trace2ELF.hpp"
#include "InputParser.hpp"
using namespace std;
using namespace tana;

int main(int argc, char *argv[]) {

    if (argc < 3) {
        cout << "Usage: " << argv[0] << " -f Function -d Debug -a Address[hex]\n";
        exit(0);
    }

    std::shared_ptr<DynamicFunction> func = nullptr;
    std::shared_ptr<Trace2ELF> t2e = nullptr;
    InputParser input(argc, argv);

    bool flags = false;
    string address;

    if (input.cmdOptionExists("-f") &&
        input.cmdOptionExists("-d") &&
        input.cmdOptionExists("-a") ){

        const std::string &fun_name = input.getCmdOption("-f");
        func = std::make_shared<DynamicFunction>(fun_name);

        const std::string &obj_name = input.getCmdOption("-d");
        t2e = std::make_shared<Trace2ELF>(obj_name, fun_name);

        address = input.getCmdOption("-a");
        flags = true;

    }

    if(!flags)
    {
        cout << "Usage: " << argv[0] << " -f Function -d Debug -a Address[hex]\n";
        exit(0);
    }

    uint32_t addr = std::stoul(address, nullptr, 16);

    auto res = t2e->locateSym(addr);

    std::cout << "Source code: " << res->pwd << ": "
              << res->file_name << " line number: "
              << res->line_number << std::endl;

    return 1;

}