
#include "Function.hpp"
#include "error.hpp"
#include "ins_types.hpp"
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <string>

using namespace std;

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {
DynamicFunction::DynamicFunction(ifstream &function_file) {
  string line;
  uint32_t num_fun = 0;
  while (function_file.good()) {
    getline(function_file, line);
    if (line.empty()) {
      continue;
    }

    istringstream strbuf(line);
    string addr, function_name, module_name, fun_size;
    getline(strbuf, addr, ';');
    getline(strbuf, module_name, ';');
    getline(strbuf, function_name, ';');
    getline(strbuf, fun_size, ';');

    auto func = std::make_shared<Routine>();
    if (function_name.front() == ' ') {
      function_name.erase(0, 1);
    }

    func->rtn_name = function_name;
    func->module_name = module_name;
    func->start_addr = stoul(addr, nullptr, 16);
    func->size = stoul(fun_size, nullptr, 10);
    func->end_addr = func->start_addr + func->size;

    auto pos = existing_rtn.find(func->start_addr);
    if (pos != existing_rtn.end()) {
      continue;
    }

    existing_rtn.insert(func->start_addr);
    fun_rtns.push_back(func);
    ++num_fun;
    if (!(num_fun % 1000)) {
      std::cout << "Parsing Functions: " << num_fun << std::endl;
    }
  }
  ptr_cache = fun_rtns.front();
}

DynamicFunction::DynamicFunction(const std::string &function_file_name) {
  ifstream function_file(function_file_name);
  if (!function_file.is_open()) {
    ERROR("Open file failed!");
    exit(0);
  }

  string line;
  uint32_t num_fun = 0;
  while (function_file.good()) {
    getline(function_file, line);
    if (line.empty()) {
      continue;
    }

    istringstream strbuf(line);
    string addr, function_name, module_name, fun_size;
    getline(strbuf, addr, ';');
    getline(strbuf, module_name, ';');
    getline(strbuf, function_name, ';');
    getline(strbuf, fun_size, ';');

    auto func = std::make_shared<Routine>();
    func->rtn_name = function_name;
    func->module_name = module_name;
    func->start_addr = stoul(addr, nullptr, 16);

    if (!fun_size.empty()) {
      func->size = stoul(fun_size, nullptr, 10);
      func->end_addr = func->start_addr + func->size;
    } else {
      func->size = 0;
      func->end_addr = 0;
    }

    auto pos = existing_rtn.find(func->start_addr);
    if (pos != existing_rtn.end()) {
      continue;
    }

    existing_rtn.insert(func->start_addr);
    fun_rtns.push_back(func);
    ++num_fun;
  }
  ptr_cache = fun_rtns.front();
}

std::string DynamicFunction::getFunctionAndLibrary(uint64_t addr) {
  for (const auto &iter : fun_rtns) {
    if ((addr >= (iter->start_addr)) && (addr <= (iter->end_addr))) {
      return "Function Name: " + iter->rtn_name +
             " Module Name: " + iter->module_name +
             " Offset: " + std::to_string(addr - iter->start_addr);
    }
  }
  return "NOT Found";
}

std::string DynamicFunction::getFunName(uint64_t addr) {
  if ((addr >= (ptr_cache->start_addr)) && (addr <= (ptr_cache->end_addr))) {
    return ptr_cache->rtn_name;
  }
  for (const auto &iter : fun_rtns) {
    if ((addr >= (iter->start_addr)) && (addr <= (iter->end_addr))) {
      ptr_cache = iter;
      return iter->rtn_name;
    }
  }
  return "NOT Found";
}

std::shared_ptr<Routine> DynamicFunction::getFunRoutine(uint64_t addr) {
  if ((addr >= (ptr_cache->start_addr)) && (addr <= (ptr_cache->end_addr))) {
    return ptr_cache;
  }
  for (const auto &iter : fun_rtns) {
    if ((addr >= (iter->start_addr)) && (addr <= (iter->end_addr))) {
      ptr_cache = iter;
      return iter;
    }
  }
  return nullptr;
}

std::shared_ptr<Routine> DynamicFunction::pickOneRandomElement() {
  std::random_device random_device;
  std::mt19937 engine{random_device()};
  std::uniform_int_distribution<int> dist(0, fun_rtns.size() - 1);
  return fun_rtns[dist(engine)];
}

StaticFunction::StaticFunction(std::string &function_name,
                               tana_type::T_SIZE function_addr,
                               tana_type::T_ADDRESS function_size,
                               uint32_t block_number)
    : m_function_name(function_name), m_start_address(function_addr),
      m_function_size(function_size), m_block_number(block_number),
      m_end_address(function_addr + function_size),
      m_cfg(block_number, vector<uint8_t>(block_number, 0))

{}

ostream &operator<<(std::ostream &os, const StaticFunction &fun) {
  os << "Function name: " << fun.m_function_name << "\n";
  os << "Start Address: " << hex << fun.m_start_address << dec << "\n";
  for (const auto &block : fun.blocks) {
    os << *block;
  }

  return os;
}

bool StaticFunction::symbolicExecution() {
  for (const auto &block : blocks) {
    block->symbolic_execution();
  }
  after_symbolic_execution = true;

  return true;
}

vector<int> StaticFunction::get_input_number() {
  vector<int> result;
  for (auto &block : blocks) {
    auto block_input = block->get_input_number();
    result.insert(result.end(), block_input.begin(), block_input.end());
  }
  return result;
}

string vector2string(vector<uint32_t> &input) {
  stringstream stream;

  for (auto &each : input) {
    stream << std::hex << each << std::dec;
    string debug = stream.str();
    // cout <<"raw:" << each <<" string:"<< debug << endl;
  }

  string result = stream.str();

  return result;
}

vector<unique_ptr<EachEntry>> StaticFunction::generate_entries() {
  vector<unique_ptr<EachEntry>> result;

  if (!after_symbolic_execution) {
    ERROR("Symbolic execution is not done!");
    exit(0);
  }

  for (auto &block : blocks) {
    for (auto &formula : block->result) {
      auto entry = std::make_unique<EachEntry>();
      vector<uint32_t> entry_hash = formula.second->get_bitvector_hash();
      entry->m_entrykey = vector2string(entry_hash);
      entry->m_block_start_addr = block->addr;
      entry->m_block_end_addr = block->end_addr;
      entry->m_function_name = this->m_function_name;
      result.push_back(std::move(entry));
    }
  }
  return result;
}

} // namespace tana