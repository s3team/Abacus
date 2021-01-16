

#pragma once
#include "Blocks.hpp"
#include "ins_types.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tana {

class Routine {
public:
  uint64_t start_addr; // start address of the function
  uint64_t end_addr;   // end address of the function
  std::string rtn_name;
  std::string module_name;
  uint64_t size;

  Routine() : start_addr(0), end_addr(0), rtn_name(), module_name(), size(0) {}

  Routine(uint64_t p1, uint64_t p2,
          const std::string &p3, const std::string &p4, uint64_t p5)
      : start_addr(p1), end_addr(p2), rtn_name(p3), module_name(p4), size(p5) {}
};

class DynamicFunction {
private:
  std::vector<std::shared_ptr<Routine>> fun_rtns;
  std::set<tana_type::T_ADDRESS> existing_rtn;

  std::shared_ptr<Routine> ptr_cache;

public:
  explicit DynamicFunction(std::ifstream &function_file);

  explicit DynamicFunction(const std::string &function_file);

  std::string getFunctionAndLibrary(uint64_t addr);

  std::string getFunName(uint64_t addr);

  std::shared_ptr<Routine> getFunRoutine(uint64_t addr);

  std::shared_ptr<Routine> pickOneRandomElement();

  ~DynamicFunction() = default;
};

// Each entry in the database
class EachEntry{
public:
  std::string m_entrykey;
  uint32_t m_block_start_addr;
  uint32_t m_block_end_addr;
  std::string m_function_name;
  EachEntry(){};
};


class StaticFunction {
private:
  const std::string m_function_name;
  const tana_type::T_SIZE m_function_size;
  const tana_type::T_ADDRESS m_start_address;
  const tana_type::T_ADDRESS m_end_address;
  const uint32_t m_block_number;
  bool after_symbolic_execution = false;

public:
  std::vector<std::unique_ptr<tana::StaticBlock>> blocks;
  std::vector<std::vector<uint8_t>> m_cfg;
  StaticFunction(std::string &function_name, tana_type::T_SIZE function_addr,
                 tana_type::T_ADDRESS function_size, uint32_t block_number);
  bool symbolicExecution();

  std::vector<int> get_input_number();

  std::vector<std::unique_ptr<EachEntry>> generate_entries();

  friend std::ostream &operator<<(std::ostream &os, const StaticFunction &fun);

};

} // namespace tana
