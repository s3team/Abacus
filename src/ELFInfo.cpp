

#include "ELFInfo.hpp"
#include "error.hpp"
#include <algorithm>
#include <cassert>
#include <elfio/elfio.hpp>
#include <iostream>
#include <memory>

using namespace std;
using namespace tana::ELF;
using namespace ELFIO;

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

std::string &ltrim(std::string &s) {
  auto it = std::find_if(s.begin(), s.end(), [](char c) {
    return !std::isspace<char>(c, std::locale::classic());
  });
  s.erase(s.begin(), it);
  return s;
}

std::string &rtrim(std::string &s) {
  auto it = std::find_if(s.rbegin(), s.rend(), [](char c) {
    return !std::isspace<char>(c, std::locale::classic());
  });
  s.erase(it.base(), s.end());
  return s;
}

std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

ELF_Symbols::ELF_Symbols(const std::string &filePWD) {

  elfio reader;

  if (!reader.load(filePWD)) {
    std::cout << "Can't find or process ELF file " << filePWD << std::endl;
    ERROR("Wrong ELF format");
    exit(0);
  }

  assert(reader.get_class() == ELFCLASS32 || reader.get_class() == ELFCLASS64);
  Elf_Half sec_num = reader.sections.size();

  for (int i = 0; i < sec_num; ++i) {
    section *psec = reader.sections[i];
    // Check section type
    if (psec->get_type() == SHT_SYMTAB) {
      const symbol_section_accessor symbols(reader, psec);
      for (unsigned long j = 0; j < symbols.get_symbols_num(); ++j) {
        std::string name;
        Elf64_Addr value;
        Elf_Xword size;
        unsigned char bind;
        unsigned char type;
        Elf_Half section_index;
        unsigned char other;

        // Read symbol properties
        symbols.get_symbol(j, name, value, size, bind, type, section_index,
                           other);

        auto sym = std::make_shared<Symbol>(name, value, size, bind, type,
                                            section_index, other);
        elf_s.push_back(sym);
      }
    }
  }
}

std::string ELF_Symbols::findSymName(uint32_t addr) {
  for (const auto &s : elf_s) {
    if (s->value == addr) {
      return s->name;
    }
  }

  return "Not Found";
}

uint32_t ELF_Symbols::findSymAddr(const std::string &name) {
  for (const auto &s : elf_s) {
    if (s->name == name) {
      return s->value;
    }
  }

  return 0;
}

std::shared_ptr<Symbol> ELF_Symbols::findSymbol(uint32_t addr) {
  for (auto &s : elf_s) {
    if (s->value == addr) {
      return s;
    }
  }

  return nullptr;
}

std::shared_ptr<Symbol> ELF_Symbols::findSymbol(const std::string &name) {
  for (auto &s : elf_s) {
    string s1, s2;
    s1 = s->name;
    s2 = name;
    s1 = trim(s1);
    s2 = trim(s2);

    if (s1 == s2) {
      return s;
    }
  }

  return nullptr;
}