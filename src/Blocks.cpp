

#include "Blocks.hpp"
#include "StaticSEEngine.hpp"
#include "VarMap.hpp"
#include "ins_types.hpp"

using namespace std;

namespace tana {

StaticBlock::StaticBlock(uint32_t n_addr, uint32_t n_end_addr,
                         uint32_t n_ninstr, uint32_t n_size)
    : addr(n_addr), end_addr(n_end_addr), ninstr(n_ninstr), size(n_size) {}

bool StaticBlock::init(std::vector<std::unique_ptr<Inst_Base>> &fun_inst) {
  bool copy_flag = false;
  bool finish_flag = false;
  for (auto &ins : fun_inst) {
    auto inst_addr = ins->address;
    if (inst_addr == addr) {
      copy_flag = true;
    }

    if (!copy_flag) {
      continue;
    }

    if (inst_addr >= end_addr) {
      if (inst_addr == end_addr) {
        finish_flag = true;
      }

      break;
    }

    inst_list.push_back(std::move(ins));
  }
  auto start_pos = fun_inst.begin();
  while (start_pos != fun_inst.end()) {
    if (*start_pos == nullptr) {
      fun_inst.erase(start_pos);
    } else
      ++start_pos;
  }

  return finish_flag;
}

std::ostream &operator<<(std::ostream &os, const StaticBlock &block) {
  os << "Block: " << block.id << "\n";
  os << "Start Address: " << std::hex << block.addr
     << " End Address: " << block.end_addr << std::dec << "\n";
  os << "Block Size: " << block.size << "\n";
  for (auto const &inst : block.inst_list) {
    os << *inst << "\n";
  }

  os << "\n";
  return os;
}

bool StaticBlock::symbolic_execution() {
  auto se = new StaticSEEngine(false);
  se->initAllRegSymol(inst_list.begin(), inst_list.end());

  se->run();

  if (se->readReg("eax") != nullptr) {
    result["eax"] = se->readReg("eax");
  }

  if (se->readReg("ebx") != nullptr) {
    result["ebx"] = se->readReg("ebx");
  }

  if (se->readReg("ecx") != nullptr) {
    result["ecx"] = se->readReg("ecx");
  }

  if (se->readReg("edx") != nullptr) {
    result["edx"] = se->readReg("edx");
  }

  if (se->readReg("esi") != nullptr) {
    result["esi"] = se->readReg("esi");
  }

  if (se->readReg("edi") != nullptr) {
    result["edi"] = se->readReg("edi");
  }

  auto memory = se->getMemory();

  result.insert(memory.begin(), memory.end());

  delete se;

  return true;
}
std::vector<int> StaticBlock::get_input_number() {
  vector<int> res;
  for (auto const &x : result) {
    res.push_back(x.second->symbol_num());
  }

  return res;
}

uint32_t DynamicBlock::block_seed_id = 0;

DynamicBlock::DynamicBlock(
    uint32_t start_inst_index, uint32_t end_inst_index,
    std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> m_ptr)
    : m_end_inst_index(end_inst_index), m_start_inst_index(start_inst_index),
      m_inst_list_ptr(m_ptr), m_match_similar(false) {
  this->block_id = ++block_seed_id;
  total_num_inst = m_end_inst_index - m_start_inst_index + 1;
}

void DynamicBlock::print() const {
  uint32_t inst_index = 0;
  uint32_t block_size = this->size();
  std::cout << "Block: " << this->block_id << "\n";
  std::cout << "Start address: " << std::hex
            << (*m_inst_list_ptr)[m_start_inst_index]->address;
  std::cout << " End address: " << (*m_inst_list_ptr)[m_end_inst_index]->address
            << std::dec << "\n";
  std::cout << "Block Size: " << this->size() << "\n";
  while (inst_index < block_size) {
    std::cout << *(*m_inst_list_ptr)[m_end_inst_index + inst_index] << "\n";
    ++inst_index;
  }

  std::cout << "\n";
}

bool DynamicBlock::operator==(const DynamicBlock &block) {
  if (size() != block.size()) {
    return false;
  }
  uint32_t block_size = size();
  uint32_t inst_index = 0;

  while (inst_index < block_size) {
    auto &inst1 =
        (*m_inst_list_ptr)[m_start_inst_index + inst_index]->instruction_id;
    auto &inst2 =
        (*(block.m_inst_list_ptr))[block.m_start_inst_index + inst_index]
            ->instruction_id;
    if (inst1 != inst2) {
      return false;
    }
    ++inst_index;
  }
  return true;
}

bool DynamicBlock::operator!=(const DynamicBlock &block) {
  return !((*this) == block);
}

uint32_t DynamicBlock::size() const {
  return m_end_inst_index - m_start_inst_index + 1;
}

shared_ptr<vector<shared_ptr<DynamicBlock>>> DynamicBlock::parse_dyn_block(
    shared_ptr<vector<unique_ptr<Inst_Base>>> &inst_list) {

  uint64_t current_last_inst = 0;
  uint64_t visit_inst_index = 0;
  const uint64_t inst_size = (*inst_list).size();

  auto res = make_shared<vector<shared_ptr<DynamicBlock>>>();

  shared_ptr<DynamicBlock> last_block = nullptr;

  while (true) {
    if ((visit_inst_index + 1) == inst_size) {
      auto block = make_shared<DynamicBlock>(current_last_inst,
                                             visit_inst_index, inst_list);
      if (last_block == nullptr) {
        last_block = block;
      } else {
        if (*last_block == *block) {
          uint32_t tmp_id = block->block_id;
          block = last_block;
          block->block_id = tmp_id;
        }
        last_block = block;
      }

      res->push_back(block);
      break;
    }

    unique_ptr<Inst_Base> &inst = (*inst_list)[visit_inst_index];
    bool is_current_jmp = x86::isInstJump(inst->instruction_id);

    if (!is_current_jmp) {
      ++visit_inst_index;
      continue;
    }

    auto block = make_shared<DynamicBlock>(current_last_inst, visit_inst_index,
                                           inst_list);
    if (last_block == nullptr) {
      last_block = block;
    } else {
      if (*last_block == *block) {
        uint32_t tmp_id = block->block_id;
        block = last_block;
        block->block_id = tmp_id;
      }
      last_block = block;
    }
    res->push_back(block);

    ++visit_inst_index;
    current_last_inst = visit_inst_index;
  }

  return res;
}

} // namespace tana