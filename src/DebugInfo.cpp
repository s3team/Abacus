
#include "DebugInfo.hpp"
#include "dwarf++.hh"
#include "elf++.hh"
#include "error.hpp"
#include <algorithm>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

using namespace std;
using namespace tana;

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

DebugInfo::DebugInfo(const std::string &elfPWD) {
  string filePWD = elfPWD;
  int fd = open(filePWD.c_str(), O_RDONLY);
  if (fd < 0) {
    ERROR("Can not open the file");
    exit(0);
  }
  elf::elf ef(elf::create_mmap_loader(fd));
  dwarf::dwarf dw(dwarf::elf::create_loader(ef));

  for (auto cu : dw.compilation_units()) {
    for (auto &line : cu.get_line_table()) {
      string file_name = line.file->path;
      uint32_t address = line.address;
      int line_num = line.line;
      auto ds = make_shared<DebugSymbol>(filePWD, file_name, address, line_num);
      line_info.push_back(ds);
    }
  }

  auto sortRuleLambda = [](shared_ptr<DebugSymbol> const &d1,
                           shared_ptr<DebugSymbol> const &d2) -> bool {
    return d1->address < d2->address;
  };

  sort(line_info.begin(), line_info.end(), sortRuleLambda);

  // for(auto &line : line_info)
  //{
  //   cout << *line << endl;
  //}
}

shared_ptr<DebugSymbol> DebugInfo::locateSym(uint64_t addr) {
  auto sortRuleLambda = [](shared_ptr<DebugSymbol> const &d1,
                           shared_ptr<DebugSymbol> const &d2) -> bool {
    return d1->address < d2->address;
  };

  auto previous = line_info.front();

  for (const auto &sym : line_info) {
    if (sym->address == addr) {
      return sym;
    }
    if (previous->address < addr && addr < sym->address) {
      return previous;
    }
    previous = sym;
  }

  return nullptr;
}

std::ostream &operator<<(ostream &os, const DebugSymbol &dt) {
  os << dt.file_name << " " << std::hex << dt.address << std::dec << " "
     << dt.line_number;
  return os;
}
