#include <iostream>
#include <string>
#include "ELFInfo.hpp"


using namespace std;
using namespace tana;
using namespace tana::ELF;

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Usage: tutorial <elf_file>" << std::endl;
        return 0;
    }

    int t = 5;

    string elfName(argv[1]);

    auto elf = new ELF_Symbols(elfName);

    string sym_name;

    while (t > 0)
    {
        cin >> sym_name;

        std::cout << "Symbol Name: " << sym_name
                  << ": "<< std::hex << elf->findSymAddr(sym_name) << std::dec << "\n";
        --t;
    }

    delete elf;
    return 0;
}