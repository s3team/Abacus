#include "Trace2ELF.hpp"

using namespace tana;
using namespace std;


int main(int argc, char** argv){

    if(argc != 2) {
        std::cout << "Usage: tutorial <elf_file>" << std::endl;
        return 0;
    }

    string obj_name(argv[1]);

    string objdump_cmd = "objdump --dwarf=decodedline " + obj_name;

    string objdump_result = cmd::exec(objdump_cmd.c_str());

    std::cout << objdump_result << std::endl;

    std::cout << "---------------------------";
    auto test_debuginfo_openssl = DebugInfo(objdump_result);

    auto target = test_debuginfo_openssl.locateSym(0x80486ab);
    cout << "\nSuppose to get 0x80486ab\t"
         << boolalpha << (target->address == 0x80486ab)
         << "\t0x" << hex << target->address
         << '\t' << dec << target->line_number
         << '\t' << target->file_name
         << '\t' << target->pwd << endl;


    target = test_debuginfo_openssl.locateSym(0x8048704);
    cout << "\nSuppose to get 0x8048704\t"
         << boolalpha << (target->address == 0x8048704)
         << "\t0x" << hex << target->address
         << '\t' << dec << target->line_number
         << '\t' << target->file_name
         << '\t' << target->pwd << endl;

    return 0;


}