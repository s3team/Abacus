#include "DynSEEngine.hpp"
#include "VarMap.hpp"
#include "error.hpp"
#include "ins_types.hpp"
#include <algorithm>
#include <cassert>
#include <limits.h>
#include <sstream>

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {

bool isTree(std::shared_ptr<BitVector> v) {

  std::list<std::shared_ptr<BitVector>> list_que;
  list_que.push_back(v);
  uint32_t count = 0;
  while (!list_que.empty()) {
    std::shared_ptr<BitVector> v = list_que.front();
    list_que.pop_front();
    ++count;
    const std::unique_ptr<Operation> &op = v->opr;
    if (op != nullptr) {
      if (op->val[0] != nullptr)
        list_que.push_back(op->val[0]);
      if (op->val[1] != nullptr)
        list_que.push_back(op->val[1]);
      if (op->val[2] != nullptr)
        list_que.push_back(op->val[2]);
    }
    if ((list_que.size() > FORMULA_MAX_LENGTH) || (count > FORMULA_MAX_LENGTH))
      return false;
  }
  return true;
}

DynSEEngine::DynSEEngine() : SEEngine(false) {
  m_ctx = {{"eax", nullptr}, {"ebx", nullptr}, {"ecx", nullptr},
           {"edx", nullptr}, {"esi", nullptr}, {"edi", nullptr},
           {"esp", nullptr}, {"ebp", nullptr}};
}

void DynSEEngine::initAllRegSymol(
    std::vector<std::unique_ptr<Inst_Base>>::iterator it1,
    std::vector<std::unique_ptr<Inst_Base>>::iterator it2) {
  m_ctx["eax"] = std::make_shared<BitVector>(ValueType::SYMBOL, "eax");
  m_ctx["ebx"] = std::make_shared<BitVector>(ValueType::SYMBOL, "ebx");
  m_ctx["ecx"] = std::make_shared<BitVector>(ValueType::SYMBOL, "ecx");
  m_ctx["edx"] = std::make_shared<BitVector>(ValueType::SYMBOL, "edx");
  m_ctx["esi"] = std::make_shared<BitVector>(ValueType::SYMBOL, "esi");
  m_ctx["edi"] = std::make_shared<BitVector>(ValueType::SYMBOL, "edi");
  m_ctx["esp"] = std::make_shared<BitVector>(ValueType::SYMBOL, "esp");
  m_ctx["ebp"] = std::make_shared<BitVector>(ValueType::SYMBOL, "ebp");

  this->start = it1;
  this->end = it2;
}

std::vector<std::shared_ptr<BitVector>> DynSEEngine::getAllOutput() {
  std::vector<std::shared_ptr<BitVector>> outputs;
  std::shared_ptr<BitVector> v;

  // symbols in registers
  v = m_ctx["eax"];
  if ((v->opr != nullptr) && (isTree(v)))
    outputs.push_back(v);
  v = m_ctx["ebx"];
  if ((v->opr != nullptr) && (isTree(v)))
    outputs.push_back(v);
  v = m_ctx["ecx"];
  if ((v->opr != nullptr) && (isTree(v)))
    outputs.push_back(v);
  v = m_ctx["edx"];
  if ((v->opr != nullptr) && (isTree(v)))
    outputs.push_back(v);

  // symbols in m_memory
  for (auto const &x : m_memory) {

    v = x.second;
    if (v == nullptr) {
      continue;
    }
    if ((v->opr != nullptr) && (isTree(v)))
      outputs.push_back(v);
  }
  return outputs;
}

bool DynSEEngine::memory_find(uint32_t addr) {
  auto ii = m_memory.find(addr);
  if (ii == m_memory.end())
    return false;
  else
    return true;
}

std::shared_ptr<BitVector> DynSEEngine::readReg(const std::string reg) {
  x86::x86_reg reg_id = x86::reg_string2id(reg);
  return readReg(reg_id);
}

std::shared_ptr<BitVector> DynSEEngine::readReg(const x86::x86_reg reg) {
  RegType type = Registers::getRegType(reg);
  if (type == FULL) {
    auto index = Registers::getRegIndex(reg);
    std::string strName = Registers::convertRegID2RegName(index);
    std::shared_ptr<BitVector> res = m_ctx[strName];
    return res;
  }

  if (type == HALF) {
    auto index = Registers::getRegIndex(reg);
    std::string strName = Registers::convertRegID2RegName(index);
    std::shared_ptr<BitVector> origin = m_ctx[strName];
    std::shared_ptr<BitVector> res = SEEngine::Extract(origin, 1, 16);
    return res;
  }

  if (type == QLOW) {
    auto index = Registers::getRegIndex(reg);
    std::string strName = Registers::convertRegID2RegName(index);
    std::shared_ptr<BitVector> origin = m_ctx[strName];
    std::shared_ptr<BitVector> res = SEEngine::Extract(origin, 1, 8);
    return res;
  }

  if (type == QHIGH) {
    auto index = Registers::getRegIndex(reg);
    std::string strName = Registers::convertRegID2RegName(index);
    std::shared_ptr<BitVector> origin = m_ctx[strName];
    std::shared_ptr<BitVector> res = SEEngine::Extract(origin, 9, 16);
    return res;
  }

  return nullptr;
}

bool DynSEEngine::writeReg(const x86::x86_reg reg,
                           std::shared_ptr<tana::BitVector> v) {
  RegType type = Registers::getRegType(reg);
  uint32_t reg_index = Registers::getRegIndex(reg);
  std::string index_name = Registers::convertRegID2RegName(reg_index);
  if (type == FULL) {
    m_ctx[index_name] = v;
    return true;
  }
  if (type == HALF) {
    auto origin = m_ctx[index_name];
    auto reg_part = Extract(origin, 17, 32);
    assert(v->size() == (REGISTER_SIZE / 2));
    auto v_reg = Concat(reg_part, v);
    assert(v_reg->size() == REGISTER_SIZE);
    m_ctx[index_name] = v_reg;
    return true;
  }
  if (type == QLOW) {
    auto origin = m_ctx[index_name];
    auto reg_part = Extract(origin, 9, 32);
    assert(v->size() == (REGISTER_SIZE / 4));
    auto v_reg = Concat(reg_part, v);
    assert(v_reg->size() == REGISTER_SIZE);
    m_ctx[index_name] = v_reg;
    return true;
  }

  if (type == QHIGH) {
    auto origin = m_ctx[index_name];
    auto reg_part1 = Extract(origin, 1, 8);
    auto reg_part2 = Extract(origin, 17, 32);
    assert(v->size() == (REGISTER_SIZE / 4));
    auto v_reg = Concat(reg_part2, v, reg_part1);
    assert(v_reg->size() == REGISTER_SIZE);
    m_ctx[index_name] = v_reg;
    return true;
  }
  ERROR("Unkown reg type");
  return false;
}

bool DynSEEngine::writeReg(const std::string reg,
                           std::shared_ptr<tana::BitVector> v) {
  x86::x86_reg reg_id = x86::reg_string2id(reg);
  return writeReg(reg_id, v);
}

std::shared_ptr<BitVector> DynSEEngine::readMem(std::string memory_address_str,
                                                tana_type::T_SIZE size) {
  uint32_t memory_address = std::stoul(memory_address_str, nullptr, 16);
  std::shared_ptr<BitVector> v0;
  uint32_t offset = memory_address % 4;

  if (memory_find(memory_address - offset)) {
    v0 = m_memory[memory_address - offset];
  } else {
    std::stringstream ss;
    ss << "Mem:" << std::hex << memory_address << std::dec;
    v0 = std::make_shared<BitVector>(ValueType::SYMBOL, ss.str());
    m_memory[memory_address - offset] = v0;
  }
  if (size == T_BYTE_SIZE * T_DWORD) {
    assert(memory_address % 4 == 0);
    return v0;
  }

  if (size == T_BYTE_SIZE * T_WORD) {
    offset = memory_address % 2;
    std::shared_ptr<BitVector> v1;
    if (offset == 0) {
      v1 = DynSEEngine::Extract(v0, 1, 16);
    } else {
      v1 = DynSEEngine::Extract(v0, 17, 32);
    }
    return v1;
  }
  assert(size == T_BYTE_SIZE);
  offset = memory_address % 4;
  std::shared_ptr<BitVector> v1;
  if (offset == 0) {
    v1 = DynSEEngine::Extract(v0, 1, 8);
  } else if (offset == 1) {
    v1 = DynSEEngine::Extract(v0, 9, 16);
  } else if (offset == 2) {
    v1 = DynSEEngine::Extract(v0, 17, 24);
  } else {
    v1 = DynSEEngine::Extract(v0, 25, 32);
  }
  return v1;
}

bool DynSEEngine::writeMem(std::string memory_address_str,
                           tana::tana_type::T_SIZE addr_size,
                           std::shared_ptr<tana::BitVector> v) {
  assert(v->size() == addr_size || !v->isSymbol());
  uint32_t memory_address = std::stoul(memory_address_str, nullptr, 16);
  std::shared_ptr<BitVector> v_origin, v_mem;
  uint32_t offset = memory_address % 4;

  if (addr_size == T_BYTE_SIZE * T_DWORD) {
    assert(memory_address % 4 == 0);
    m_memory[memory_address] = v;
    return true;
  }
  if (memory_find(memory_address - offset)) {
    v_origin = m_memory[memory_address - offset];
  } else {
    std::stringstream ss;
    ss << "Mem:" << std::hex << memory_address << std::dec;
    v_origin = std::make_shared<BitVector>(ValueType::SYMBOL, ss.str());
    m_memory[memory_address - offset] = v_origin;
  }

  if (addr_size == T_BYTE_SIZE * T_WORD) {
    offset = memory_address % 2;
    if (offset == 0) {
      std::shared_ptr<BitVector> v1 = DynSEEngine::Extract(v_origin, 1, 16);
      if (!v->isSymbol()) {
        v = DynSEEngine::Extract(v, 1, 16);
      }
      v_mem = Concat(v1, v);
    } else {
      std::shared_ptr<BitVector> v1 = DynSEEngine::Extract(v_origin, 17, 32);
      if (!v->isSymbol()) {
        v = DynSEEngine::Extract(v, 1, 16);
      }
      v_mem = Concat(v, v1);
    }
    m_memory[memory_address - offset] = v_mem;
    return true;
  }

  if (addr_size == T_BYTE_SIZE * T_BYTE) {
    offset = memory_address % 4;
    if (offset == 0) {
      std::shared_ptr<BitVector> v1 = DynSEEngine::Extract(v_origin, 9, 32);
      if (!v->isSymbol()) {
        v = DynSEEngine::Extract(v, 1, 8);
      }
      v_mem = Concat(v1, v);

    } else if (offset == 1) {
      std::shared_ptr<BitVector> v1 = DynSEEngine::Extract(v_origin, 1, 8);
      std::shared_ptr<BitVector> v2 = DynSEEngine::Extract(v_origin, 17, 32);
      if (!v->isSymbol()) {
        v = DynSEEngine::Extract(v, 1, 8);
      }
      v_mem = Concat(v2, v, v1);
    } else if (offset == 2) {
      std::shared_ptr<BitVector> v1 = DynSEEngine::Extract(v_origin, 1, 16);
      std::shared_ptr<BitVector> v2 = DynSEEngine::Extract(v_origin, 25, 32);
      if (!v->isSymbol()) {
        v = DynSEEngine::Extract(v, 1, 8);
      }
      v_mem = Concat(v2, v, v1);
    } else {
      std::shared_ptr<BitVector> v1 = DynSEEngine::Extract(v_origin, 1, 24);
      if (!v->isSymbol()) {
        v = DynSEEngine::Extract(v, 1, 8);
      }
      v_mem = Concat(v, v1);
    }
    m_memory[memory_address - offset] = v_mem;
    return true;
  }
  return false;
}

} // namespace tana