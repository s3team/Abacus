#include <iostream>
#include <fstream>
#include "DebugInfo.hpp"

using namespace std;
using namespace tana;

int main(int argc, char **argv)
{
     if (argc != 2) {
        fprintf(stderr, "usage: %s elf-file\n", argv[0]);
        return 2;
     }
     string debug_file(argv[1]);
     auto test_debuginfo_openssl = DebugInfo(debug_file);

     // Mid in the line
     auto target = test_debuginfo_openssl.locateSym(0x8b1c0);
     cout << "Suppose to get 0x8b1c0\t"
          << boolalpha << (target->address == 0x8b1c0)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;
     // Smaller than mid in the line
     target = test_debuginfo_openssl.locateSym(0x8b1c0 - 1);
     cout << "Suppose to get 0x8b1ae\t"
          << boolalpha << (target->address == 0x8b1ae)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;
     // Bigger than mid in the line
     target = test_debuginfo_openssl.locateSym(0x8b1c0 + 1);
     cout << "Suppose to get 0x8b1c0\t"
          << boolalpha << (target->address == 0x8b1c0)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;

     // First in the line
     target = test_debuginfo_openssl.locateSym(0x2cca0);
     cout << "Suppose to get 0x2cca0\t"
          << boolalpha << (target->address == 0x2cca0)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;
     // Smaller than First in the line
     target = test_debuginfo_openssl.locateSym(0x2cca0 - 1);
     cout << "Suppose to get None\t"
          << boolalpha << (target == nullptr) << endl;
     // Bigger than First in the line
     target = test_debuginfo_openssl.locateSym(0x2cca0 + 1);
     cout << "Suppose to get 0x2cca0\t"
          << boolalpha << (target->address == 0x2cca0)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;

     // Last in the line
     target = test_debuginfo_openssl.locateSym(0xe7cbf);
     cout << "Suppose to get 0xe7cbf\t"
          << boolalpha << (target->address == 0xe7cbf)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;
     // Smaller than Last in the line
     target = test_debuginfo_openssl.locateSym(0xe7cbf - 1);
     cout << "Suppose to get 0xe7cb0\t"
          << boolalpha << (target->address == 0xe7cb0)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;
     // Bigger than Last in the line
     target = test_debuginfo_openssl.locateSym(0xe7cbf + 1);
     cout << "Suppose to get 0xe7cbf\t"
          << boolalpha << (target->address == 0xe7cbf)
          << "\t0x" << hex << target->address
          << '\t' << dec << target->line_number
          << '\t' << target->file_name
          << '\t' << target->pwd << endl;


     return 0;
}
