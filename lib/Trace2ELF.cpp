#include <string>
#include "Trace2ELF.hpp"

using namespace tana;
using namespace std;

Trace2ELF::Trace2ELF(const std::string &obj_name, const std::string &function_file_name)
{
    string debug;
    string objdump_cmd = "objdump --dwarf=decodedline " + obj_name;

    string objdump_result = cmd::exec(objdump_cmd.c_str());

    debug_info = std::make_unique<DebugInfo>(objdump_result);

    func = std::make_unique<Function>(function_file_name);

    elf_sym_info = std::make_unique<ELF::ELF_Symbols>(obj_name);

}


std::shared_ptr<DebugSymbol> Trace2ELF::locateSym(uint32_t mem_addr)
{
    auto fun = func->getFunRoutine(mem_addr);

    uint32_t offset = mem_addr - fun->start_addr;

    auto sym = elf_sym_info->findSymbol(fun->rtn_name);

    uint32_t elf_addr = sym->value + offset;

    return debug_info->locateSym(elf_addr);
}