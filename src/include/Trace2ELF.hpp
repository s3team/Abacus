
#pragma once

#include "DebugInfo.hpp"
#include "ELFInfo.hpp"
#include "Function.hpp"
#include "cmd.hpp"
#include <memory>

namespace tana {

class Trace2ELF {
private:
  std::unique_ptr<DebugInfo> debug_info;
  std::unique_ptr<ELF::ELF_Symbols> elf_sym_info;
  std::unique_ptr<DynamicFunction> func;

public:
  explicit Trace2ELF(const std::string &obj_name,
                     const std::string &function_file_name);
  std::shared_ptr<DebugSymbol> locateSym(uint64_t mem_addr);
};

} // namespace tana