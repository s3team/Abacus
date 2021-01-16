
#include "LibcModel.hpp"
#include "BitVector.hpp"
#include "Engine.hpp"
#include "error.hpp"
#include "ins_types.hpp"
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace tana;
#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace libc {

LibcID stringToLibcID(const string &libc) {

  if (libc.rfind("calloc") != string::npos) {
    return LibcID ::X86_Calloc;
  }

  if (libc.rfind("malloc") != string::npos) {
    return LibcID ::X86_Malloc;
  }

  if (libc.rfind("memcpy") != string::npos) {
    return LibcID ::X86_Memcpy;
  }

  if (libc.rfind("mempcpy") != string::npos) {
    return LibcID ::X86_Mempcpy;
  }

  if (libc.rfind("memset") != string::npos) {
    return LibcID ::X86_Memset;
  }

  return LibcID ::Invalid;
}

string uint32ToStr(uint32_t addr) {
  stringstream sstream;
  sstream << hex << addr << dec;
  return sstream.str();
}

vector<string> split(string str, string token) {
  vector<string> result;
  while (str.size()) {
    int index = str.find(token);
    if (index != string::npos) {
      result.push_back(str.substr(0, index));
      str = str.substr(index + token.size());
      if (str.size() == 0)
        result.push_back(str);
    } else {
      result.push_back(str);
      str = "";
    }
  }
  return result;
}

} // namespace libc

unique_ptr<Inst_Base> LibC_Factory::parseLibc(const std::string &line) {
  istringstream strbuf(line);
  vector<string> result = libc::split(line, ";");
  LibcID libc_id = libc::stringToLibcID(result[1]);
  switch (libc_id) {
  case LibcID::X86_Calloc: {
    uint32_t num = std::stoul(result[2], nullptr, 16);
    uint32_t size = std::stoul(result[3], nullptr, 16);
    uint32_t ret = std::stoul(result[6], nullptr, 16);

    return std::make_unique<LIBC_X86_Calloc>(num, size, ret);
  }
  case LibcID::X86_Malloc: {
    // TODO
    break;
  }
  case LibcID::X86_Memcpy: {
    uint32_t destination = std::stoul(result[2], nullptr, 16);
    uint32_t source = std::stoul(result[3], nullptr, 16);
    uint32_t num = std::stoul(result[4], nullptr, 16);
    uint32_t ret = std::stoul(result[7], nullptr, 16);

    return std::make_unique<LIBC_X86_Memcpy>(destination, source, num, ret);
  }
  case LibcID::X86_Mempcpy: {
    uint32_t destination = std::stoul(result[2], nullptr, 16);
    uint32_t source = std::stoul(result[3], nullptr, 16);
    uint32_t num = std::stoul(result[4], nullptr, 16);
    uint32_t ret = std::stoul(result[6], nullptr, 16);

    return std::make_unique<LIBC_X86_Mempcpy>(destination, source, num, ret);
  }
  case LibcID::X86_Memset: {
    uint32_t ptr = std::stoul(result[2], nullptr, 16);
    uint32_t value = std::stoul(result[3], nullptr, 16);
    uint32_t num = std::stoul(result[4], nullptr, 16);
    uint32_t ret = std::stoul(result[7], nullptr, 16);

    return std::make_unique<LIBC_X86_Memset>(ptr, value, num, ret);
  }
  case LibcID::Invalid: {
    ERROR("Invalid libc functions");
    exit(0);
  }
  }
  return nullptr;
}

bool LIBC_X86_Calloc::symbolic_execution(tana::SEEngine *se) {
  assert(!this->is_static);
  uint32_t num, size, ret;
  num = this->m_num;
  size = this->m_size;
  ret = this->m_ret_value;
  uint32_t num_bytes = size * num;

  assert(ret % 4 == 0);
  string addr_str;
  for (uint32_t i = 0; i < num_bytes / T_DWORD; ++i) {
    auto data = std::make_shared<BitVector>(ValueType::CONCRETE, 0);
    uint32_t addr = ret + i * T_DWORD;
    addr_str = libc::uint32ToStr(addr);
    se->writeMem(addr_str, T_BYTE_SIZE * T_DWORD, data);
  }

  // We think eax register store the return value
  auto ret_v = std::make_shared<BitVector>(ValueType::CONCRETE, ret);
  se->writeReg("eax", ret_v);

  return true;
}

bool LIBC_X86_Malloc::symbolic_execution(tana::SEEngine *se) {
  assert(!this->is_static);
  uint32_t ret;
  ret = this->m_ret_value;
  auto ret_v = std::make_shared<BitVector>(ValueType::CONCRETE, ret);
  se->writeReg("eax", ret_v);
  return true;
}

bool LIBC_X86_Memcpy::symbolic_execution(tana::SEEngine *se) {
  assert(!this->is_static);
  uint32_t destination, source, num, ret;
  destination = m_destination;
  source = m_source;
  num = m_num;
  ret = m_ret_value;

  assert(num % 4 == 0);
  uint32_t index;
  shared_ptr<BitVector> data;
  string source_addr_str, dest_addr_str;
  for (index = 0; index < num / T_DWORD; ++index) {
    source_addr_str = libc::uint32ToStr(source + index * T_DWORD);
    dest_addr_str = libc::uint32ToStr(destination + index * T_DWORD);

    data = se->readMem(source_addr_str, T_DWORD * T_BYTE_SIZE);
    se->writeMem(dest_addr_str, T_DWORD * T_BYTE_SIZE, data);
  }

  auto rev_v = std::make_shared<BitVector>(ValueType ::CONCRETE, destination);
  se->writeReg("eax", rev_v);
  return true;
}

bool LIBC_X86_Mempcpy::symbolic_execution(tana::SEEngine *se) {
  assert(!this->is_static);
  uint32_t destination, source, num, ret;
  destination = m_destination;
  source = m_source;
  num = m_num;
  ret = m_ret_value;
  uint32_t index;
  shared_ptr<BitVector> data;
  string source_addr_str, dest_addr_str;
  assert(num % 4 == 0);
  for (index = 0; index < num / T_DWORD; ++index) {
    source_addr_str = libc::uint32ToStr(source + index * T_DWORD);
    dest_addr_str = libc::uint32ToStr(destination + index * T_DWORD);

    data = se->readMem(source_addr_str, T_DWORD * T_BYTE_SIZE);
    se->writeMem(dest_addr_str, T_DWORD * T_BYTE_SIZE, data);
  }

  auto rev_v = std::make_shared<BitVector>(ValueType ::CONCRETE,
                                           destination + index * T_DWORD);
  se->writeReg("eax", rev_v);
  return true;
}

bool LIBC_X86_Memset::symbolic_execution(tana::SEEngine *se) {
  assert(!this->is_static);
  uint32_t ptr, value, num, ret;
  ptr = m_ptr;
  value = m_value;
  num = m_num;
  ret = m_ret_value;

  uint32_t index;
  string addr_str;
  shared_ptr<BitVector> data;
  assert(num % 4 == 0);
  for (index = 0; index < num / T_DWORD; ++index) {
    data = make_shared<BitVector>(ValueType::CONCRETE, value);
    addr_str = libc::uint32ToStr(ptr + index * T_DWORD);
    se->writeMem(addr_str, T_DWORD * T_BYTE_SIZE, data);
  }

  auto rev_v = std::make_shared<BitVector>(ValueType ::CONCRETE, ret);
  se->writeReg("eax", rev_v);

  return true;
}
