
#pragma once

#include <elfio/elfio.hpp>
#include <iostream>
#include <memory>

namespace tana {
namespace ELF {

class Symbol {
public:
  std::string name;
  ELFIO::Elf64_Addr value;
  ELFIO::Elf_Xword size;
  unsigned char bind;
  unsigned char type;
  ELFIO::Elf_Half section_index;
  unsigned char other;

  Symbol(const std::string &s_name, ELFIO::Elf64_Addr s_value,
         ELFIO::Elf_Xword s_size, unsigned char s_bind, unsigned char s_type,
         ELFIO::Elf_Half s_section_index, unsigned char s_other)
      : name(s_name), value(s_value), size(s_size), bind(s_bind), type(s_type),
        section_index(s_section_index), other(s_other) {}
};

class ELF_Symbols {
private:
  std::vector<std::shared_ptr<Symbol>> elf_s; // ELF symbols
public:
  explicit ELF_Symbols(const std::string &filePWD);

  std::string findSymName(uint32_t addr);

  uint32_t findSymAddr(const std::string &name);

  std::shared_ptr<Symbol> findSymbol(uint32_t addr);

  std::shared_ptr<Symbol> findSymbol(const std::string &name);

  ~ELF_Symbols() = default;
};
} // namespace ELF

} // namespace tana
