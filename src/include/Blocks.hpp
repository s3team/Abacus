

#pragma once

#include "BitVector.hpp"
#include "ins_types.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tana {

class StaticBlock {
public:
  std::vector<std::unique_ptr<Inst_Base>> inst_list;
  uint32_t addr;
  uint32_t end_addr;
  uint32_t inputs;
  uint32_t ninstr;
  uint32_t outputs;
  uint32_t size;
  int id = 0;
  bool traced;

  std::vector<int> number_symbol;

  std::map<std::string, std::shared_ptr<BitVector>> result;

  StaticBlock(uint32_t n_addr, uint32_t n_end_addr,
              uint32_t n_ninstr,  uint32_t n_size);

  bool init(std::vector<std::unique_ptr<Inst_Base>> &fun_inst);

  friend std::ostream &operator<<(std::ostream &os, const StaticBlock &block);

  bool symbolic_execution();

  std::vector<int> get_input_number();

};

class DynamicBlock {
private:
  static uint32_t block_seed_id;

public:
  uint32_t block_id;
  uint32_t m_start_inst_index;
  uint32_t m_end_inst_index;
  std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> m_inst_list_ptr;

  bool m_match_similar;
  uint32_t total_num_inst;

  bool operator==(const DynamicBlock &block);
  bool operator!=(const DynamicBlock &block);
  bool operator<(const DynamicBlock &block);
  bool operator>(const DynamicBlock &block);

  void print() const;

  uint32_t size() const;

  DynamicBlock(uint32_t start_inst_index, uint32_t end_inst_index,
               std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> m);

  static std::shared_ptr<std::vector<std::shared_ptr<DynamicBlock>>>
  parse_dyn_block(
      std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> &inst_list);
};

} // namespace tana