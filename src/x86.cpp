
#include "x86.hpp"
#include "error.hpp"
#include <set>

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {
namespace x86 {
x86_insn insn_string2id(const std::string &insn_name) {
  x86_insn insn_id = X86_INS_INVALID;
  for (const auto &inst_map : insn_name_maps) {
    if (inst_map.name == insn_name) {
      insn_id = inst_map.id;
      return insn_id;
    }
  }
  ERROR(insn_name.c_str());
  return insn_id;
}

x86_reg reg_string2id(const std::string &reg_name) {
  x86_reg reg_id = X86_REG_INVALID;
  for (const auto &reg_map : reg_name_maps) {
    if (reg_map.name == reg_name) {
      reg_id = reg_map.id;
      return reg_id;
    }
  }

  return reg_id;
}

std::string insn_id2string(const x86_insn id) {
  std::string x86_opc = "None";
  for (const auto &insn_map : insn_name_maps) {
    if (insn_map.id == id) {
      x86_opc = insn_map.name;
    }
  }

  return x86_opc;
}

std::string reg_id2string(x86_reg id) {
  std::string x86_reg;
  for (const auto &reg_map : reg_name_maps) {
    if (reg_map.id == id) {
      x86_reg = reg_map.name;
    }
  }
  return x86_reg;
}

uint32_t get_reg_size(x86_reg reg_id) {
  int id = static_cast<int>(reg_id);
  uint8_t reg_size = regsize_map_32[id];
  return static_cast<uint32_t>(reg_size);
}

bool SymbolicExecutionNoEffect(const x86::x86_insn &insn) {
  using namespace x86;
  std::set<x86::x86_insn> no_effect_inst{
      X86_INS_TEST, X86_INS_JMP, X86_INS_CMP,     X86_INS_NOP,    X86_INS_INT,
      X86_INS_JA,   X86_INS_JAE, X86_INS_JAE,     X86_INS_JB,     X86_INS_JBE,
      X86_INS_JCXZ, X86_INS_JE,  X86_INS_JECXZ,   X86_INS_JG,     X86_INS_JGE,
      X86_INS_JS,   X86_INS_JNE, X86_INS_JNO,     X86_INS_JNP,    X86_INS_JNS,
      X86_INS_JZ,   X86_INS_JNZ, X86_INS_INVALID, X86_INS_DATA16, X86_INS_JLE,
      X86_INS_JL};
  const bool found = no_effect_inst.find(insn) != no_effect_inst.end();

  return found != 0;
}

bool isInstCall(const x86::x86_insn &inst) { return inst == X86_INS_CALL; }

bool isInstRet(const x86::x86_insn &inst) { return inst == X86_INS_RET; }

bool isInstLEA(const x86::x86_insn &inst) { return inst == X86_INS_LEA; }

bool isSSE(const x86::x86_insn &inst) {
  using namespace x86;
  std::set<x86::x86_insn> sse_inst{
      X86_INS_MOVDQU,  X86_INS_MOVD,   X86_INS_PSHUFD, X86_INS_PXOR,
      X86_INS_PTEST,   X86_INS_MOVDQA, X86_INS_MOVQ,   X86_INS_PMOVMSKB,
      X86_INS_PCMPEQB, X86_INS_MOVSD,  X86_INS_MOVSB};
  const bool found = sse_inst.find(inst) != sse_inst.end();

  return found != 0;
}

bool isInstJump(const x86::x86_insn &inst) {
  using namespace x86;
  std::set<x86::x86_insn> jump_inst{
      X86_INS_JMP,  X86_INS_JA,    X86_INS_JAE, X86_INS_JB,    X86_INS_JBE,
      X86_INS_JCXZ, X86_INS_JECXZ, X86_INS_JE,  X86_INS_JGE,   X86_INS_JG,
      X86_INS_JLE,  X86_INS_JNLE,  X86_INS_JL,  X86_INS_JNA,   X86_INS_JNAE,
      X86_INS_JNE,  X86_INS_JNLE,  X86_INS_JNB, X86_INS_JNBE,  X86_INS_JNC,
      X86_INS_JNG,  X86_INS_JNL,   X86_INS_JNO, X86_INS_JNP,   X86_INS_JNS,
      X86_INS_JNZ,  X86_INS_JO,    X86_INS_JP,  X86_INS_JRCXZ, X86_INS_JS,
      X86_INS_JZ,
  };
  const bool found = jump_inst.find(inst) != jump_inst.end();

  return found != 0;
}

} // namespace x86
} // namespace tana