

#pragma once

#include "error.hpp"
#include "x86.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace config_parameter {
const static uint32_t DATA_GRANULARITY = 6;
}

namespace tana {

class SEEngine;
const static uint32_t T_BYTE = 1;
const static uint32_t T_WORD = 2;
const static uint32_t T_DWORD = 4;
const static uint32_t T_QWORD = 8;
const static uint32_t T_OWORD = 16;

const static uint32_t GPR_NUM = 8;
const static uint32_t REGISTER_SIZE = 32;
const static uint32_t T_BYTE_SIZE = 8;

const static uint32_t BATCH_SIZE = 2000;
const static uint32_t FORMULA_MAX_LENGTH = 0xffff;
const static uint32_t FORMULA_MIN_LENGTH = 6;

const static uint32_t MIN_NUM_INPUT = 2;

const static uint32_t MAX_LOOP_HISTORY = 2000;
const static uint32_t MIN_LOOP_LENGTH = 20;

const static uint32_t LOOP_PRINT_FREQUENCY = 1000;

const static uint32_t MAX_IMM_NUMBER = 0xff;

const static uint32_t MAX_INT = 0x7fffffff;
const static uint32_t MAX_UNSIGNED_INT = 0xffffffff;

namespace tana_type {

typedef uint64_t T_ADDRESS;
typedef uint32_t T_SIZE;
typedef uint32_t R_SIZE;
typedef uint32_t RegPart;
typedef uint32_t index;
} // namespace tana_type

class vcpu_ctx {
  /*
   * General Purpose Registers
   * 0 EAX
   * 1 EBX
   * 2 ECX
   * 3 EDX
   * 4 ESI
   * 5 EDI
   * 6 ESP
   * 7 EBP
   */
  uint32_t eflags;

public:
  std::array<uint32_t, GPR_NUM> gpr;

  void set_eflags(uint32_t data) { eflags = data; }

  vcpu_ctx();

  bool CF() const;

  bool PF() const;

  bool AF() const;

  bool ZF() const;

  bool SF() const;

  bool TF() const;

  bool DF() const;

  bool OF() const;

  bool eflags_state;
};

class Operand {
public:
  enum OprType { ImmValue, Reg, Mem, Label, UINIT };
  OprType type = UINIT;
  uint32_t tag = 0;
  uint32_t bit = 0;
  std::string field[5];
  bool issegaddr = false;
  std::string segreg;

  friend std::ostream &operator<<(std::ostream &os, const Operand &opr);
};

class Inst_Base {
private:
  bool mem_data_available;
  bool x86;
  uint32_t mem_data;

public:
  bool is_static = false;
  bool is_function = false;
  bool is_updateSecret = false;
  bool is_supported = false;    // If the instruction is not supported
  tana_type::index id;          // instruction ID
  tana_type::T_ADDRESS address; // Instruction address
  x86::x86_insn instruction_id;
  std::vector<std::string> oprs;
  std::shared_ptr<Operand> oprd[3];
  vcpu_ctx vcpu;
  tana_type::T_ADDRESS memory_address;

  std::string get_opcode_operand() const;

  uint32_t get_operand_number() const;

  explicit Inst_Base(bool);

  std::string get_memory_address();

  void parseOperand();

  void set_mem_data(uint32_t mem_data);

  uint32_t read_mem_data() const;

  uint32_t read_reg_data(std::string RegName) const;

  virtual bool symbolic_execution(SEEngine *se) {
    if (log_class == LOG_TYPE::MUTE) {
      return true;
    }

    std::cout << "Index: " << id
              << " Unsupported Instruction: " << this->get_opcode_operand()
              << std::endl;
    std::cout << *this;
    return true;
  };

  virtual bool taint() {
    if (log_class == LOG_TYPE::MUTE) {
      return true;
    }
    std::cout << "Index: " << id
              << " Unsupported Instruction: " << this->get_opcode_operand()
              << std::endl;
    std::cout << *this;
    return true;
  };

  virtual ~Inst_Base() = default;

  friend std::ostream &operator<<(std::ostream &os, const Inst_Base &inst);
};

} // namespace tana
