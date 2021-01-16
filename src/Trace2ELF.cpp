#include "Trace2ELF.hpp"
#include <string>

using namespace tana;
using namespace std;

Trace2ELF::Trace2ELF(const std::string &obj_name,
                     const std::string &function_file_name) {

  debug_info = std::make_unique<DebugInfo>(obj_name);

  func = std::make_unique<DynamicFunction>(function_file_name);

  elf_sym_info = std::make_unique<ELF::ELF_Symbols>(obj_name);
}

std::shared_ptr<DebugSymbol> Trace2ELF::locateSym(uint64_t mem_addr) {
  auto fun = func->getFunRoutine(mem_addr);

  if (fun == nullptr) {
    return nullptr;
  }

  uint64_t offset = mem_addr - fun->start_addr;

  auto sym = elf_sym_info->findSymbol(fun->rtn_name);
  if (sym == nullptr) {
    return nullptr;
  }

  uint64_t elf_addr = sym->value + offset;

  return debug_info->locateSym(elf_addr);
}