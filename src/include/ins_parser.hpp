

#pragma once

#include "Blocks.hpp"
#include "Function.hpp"
#include "ins_semantics.hpp"
#include "ins_types.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <tuple>

namespace tana {

uint64_t file_inst_num(std::ifstream &trace_file);

std::shared_ptr<Operand> createOperand(const std::string &s, uint32_t addr);

std::shared_ptr<Operand> createOperandStatic(const std::string &s,
                                             uint32_t addr);

std::unique_ptr<Inst_Base>
parseInst(const std::string &line, bool isStatic,
          const std::shared_ptr<DynamicFunction> &fun);

bool parse_trace(std::ifstream &trace_file,
                 std::vector<std::unique_ptr<Inst_Base>> &L,
                 tana_type::T_ADDRESS &addr_taint,
                 tana_type::T_SIZE &size_taint, uint32_t num);

bool parse_trace(std::ifstream &trace_file,
                 std::vector<std::unique_ptr<Inst_Base>> &L);

bool parse_trace(std::ifstream &trace_file, tana_type::T_ADDRESS &addr_taint,
                 tana_type::T_SIZE &size_taint,
                 std::vector<std::unique_ptr<Inst_Base>> &L);

bool parse_static_trace(std::ifstream &trace_file, std::ifstream &json_file,
                        std::vector<std::unique_ptr<StaticBlock>> &L);

bool parse_trace_qif(std::ifstream &trace_file,
                     std::vector<std::unique_ptr<Inst_Base>> &L,
                     uint64_t inst_size,
                     const std::shared_ptr<DynamicFunction> &fun);

} // namespace tana