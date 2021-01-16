
#include "ins_semantics.hpp"
#include "BitVector.hpp"
#include "Constrains.hpp"
#include "Engine.hpp"
#include "Register.hpp"
#include "error.hpp"
#include "ins_types.hpp"
#include <sstream>

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)
#define WARN(MESSAGE) tana::default_warn_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {

void updateZF(SEEngine *se, std::shared_ptr<BitVector> b) {
  auto res = buildop2(BVOper::equal, b, 0);
  se->updateFlags("ZF", res);
}

void updateSF(SEEngine *se, std::shared_ptr<BitVector> b, uint32_t op_size) {
  uint32_t max_num_size = 0;
  if (op_size == 32) {
    max_num_size = 0x7fffffff;

  } else if (op_size == 16) {
    max_num_size = 0x7fff;

  } else if (op_size == 8) {
    max_num_size = 0x7f;
  } else {
    ERROR("Invalid operand size");
  }
  auto SF = buildop2(BVOper::greater, b, max_num_size);
  se->updateFlags("SF", SF);
}

void updateOFadd(SEEngine *se, std::shared_ptr<BitVector> A,
                 std::shared_ptr<BitVector> B, uint32_t op_size) {
  // https://www.doc.ic.ac.uk/~eedwards/compsys/arithmetic/
  // Add Overflow occurs if
  // Situation 1: (+A) + (+B) = -C
  // Situation 2: (-A) + (-B) = +C
  uint32_t max_num_size = 0;
  if (op_size == 32) {
    max_num_size = 0x7fffffff;

  } else if (op_size == 16) {
    max_num_size = 0x7fff;

  } else if (op_size == 8) {
    max_num_size = 0x7f;
  } else {
    ERROR("Invalid operand size");
  }
  // Situation 1
  auto con1 = buildop2(BVOper::less, A, max_num_size);
  auto con2 = buildop2(BVOper::less, B, max_num_size);
  auto C1 = buildop2(BVOper::bvadd, A, B);
  auto C1_cons = buildop2(BVOper::greater, C1, max_num_size);
  auto S1 = buildop3(BVOper::bvand3, con1, con2, C1_cons);

  // Situation 2
  auto con3 = buildop2(BVOper::greater, A, max_num_size);
  auto con4 = buildop2(BVOper::greater, B, max_num_size);
  auto sum2 = buildop2(BVOper::bvadd, A, B);
  auto sum2_cons = buildop2(BVOper::less, C1, max_num_size);
  auto S2 = buildop3(BVOper::bvand3, con3, con4, sum2_cons);

  // S1 AND S2
  auto OF = buildop2(BVOper::bvor, S1, S2);
  se->updateFlags("OF", OF);
}

void updateOFsub(SEEngine *se, std::shared_ptr<BitVector> A,
                 std::shared_ptr<BitVector> B, uint32_t op_size) {

  // Sub Overflow occurs if
  // Situation1 :(+A) − (−B) = −C
  // Situation2: (−A) − (+B) = +C
  uint32_t max_num_size = 0;
  if (op_size == 32) {
    max_num_size = 0x7fffffff;

  } else if (op_size == 16) {
    max_num_size = 0x7fff;

  } else if (op_size == 8) {
    max_num_size = 0x7f;
  } else {
    ERROR("Invalid operand size");
  }
  // Situation 1
  auto con1 = buildop2(BVOper::less, A, max_num_size);
  auto con2 = buildop2(BVOper::greater, B, max_num_size);
  auto C1 = buildop2(BVOper::bvsub, A, B);
  auto C1_cons = buildop2(BVOper::greater, C1, max_num_size);
  auto S1 = buildop3(BVOper::bvand3, con1, con2, C1_cons);

  // Situation 2
  auto con3 = buildop2(BVOper::greater, A, max_num_size);
  auto con4 = buildop2(BVOper::less, B, max_num_size);
  auto sum2 = buildop2(BVOper::bvsub, A, B);
  auto sum2_cons = buildop2(BVOper::less, C1, max_num_size);
  auto S2 = buildop3(BVOper::bvand3, con3, con4, sum2_cons);

  // S1 AND S2
  auto OF = buildop2(BVOper::bvor, S1, S2);
  se->updateFlags("OF", OF);
}

void updateCFadd(SEEngine *se, std::shared_ptr<BitVector> b1,
                 std::shared_ptr<BitVector> b2, uint32_t op_size) {
  uint32_t max_num_size;
  if (op_size == 32) {
    max_num_size = 0xffffffff;

  } else if (op_size == 16) {
    max_num_size = 0xffff;

  } else if (op_size == 8) {
    max_num_size = 0xff;
  } else {
    ERROR("Invalid operand size");
  }
  auto max_num_v =
      std::make_shared<BitVector>(ValueType::CONCRETE, max_num_size);
  auto left = buildop2(BVOper::bvsub, max_num_v, b2);

  auto res = buildop2(BVOper::greater, b1, left);
  se->updateFlags("CF", res);
}

void updateCFsub(SEEngine *se, std::shared_ptr<BitVector> b1,
                 std::shared_ptr<BitVector> b2) {
  auto res = buildop2(BVOper::less, b1, b2);
  se->updateFlags("CF", res);
}

std::unique_ptr<Inst_Base> Inst_Factory::makeInst(tana::x86::x86_insn id,
                                                  bool isStatic) {
  return Inst_Factory::makeInst(id, isStatic, nullptr, 0);
}

std::unique_ptr<Inst_Base>
Inst_Factory::makeRepInst(tana::x86::x86_insn id, bool isStatic,
                          const std::shared_ptr<DynamicFunction> &fun,
                          uint32_t addr) {
  if (x86::isSSE(id)) {
    return std::make_unique<INST_X86_INS_SSE>(isStatic);
  }

  switch (id) {
  case x86::x86_insn::X86_INS_STOSD:
    return std::make_unique<INST_X86_INS_REP_STOSD>(isStatic);

  case x86::x86_insn::X86_INS_STOSB:
    return std::make_unique<INST_X86_INS_REP_STOSB>(isStatic);

  case x86::x86_insn::X86_INS_NOP:
    return std::make_unique<INST_X86_INS_NOP>(isStatic);

  default: {
    if (fun != nullptr) {
      std::string info = "unrecognized inst: " + fun->getFunName(addr);
      WARN(info.c_str());
      std::cout << x86::insn_id2string(id) << std::endl;
      return std::make_unique<Inst_Base>(isStatic);
    }

    WARN("unrecognized instructions");
    std::cout << x86::insn_id2string(id) << std::endl;
    return std::make_unique<Inst_Base>(isStatic);
  }
  }
}

std::unique_ptr<Inst_Base>
Inst_Factory::makeInst(tana::x86::x86_insn id, bool isStatic,
                       const std::shared_ptr<DynamicFunction> &func,
                       uint32_t addr) {

  if (x86::isSSE(id)) {
    return std::make_unique<INST_X86_INS_SSE>(isStatic);
  }

  switch (id) {
  case x86::x86_insn::X86_INS_NOP:
    return std::make_unique<INST_X86_INS_NOP>(isStatic);

  case x86::x86_insn::X86_INS_PUSH:
    return std::make_unique<INST_X86_INS_PUSH>(isStatic);

  case x86::x86_insn::X86_INS_POP:
    return std::make_unique<INST_X86_INS_POP>(isStatic);

  case x86::x86_insn::X86_INS_NEG:
    return std::make_unique<INST_X86_INS_NEG>(isStatic);

  case x86::x86_insn::X86_INS_NOT:
    return std::make_unique<INST_X86_INS_NOT>(isStatic);

  case x86::x86_insn::X86_INS_INC:
    return std::make_unique<INST_X86_INS_INC>(isStatic);

  case x86::x86_insn::X86_INS_DEC:
    return std::make_unique<INST_X86_INS_DEC>(isStatic);

  case x86::x86_insn::X86_INS_MOVZX:
    return std::make_unique<INST_X86_INS_MOVZX>(isStatic);

  case x86::x86_insn::X86_INS_MOVSX:
    return std::make_unique<INST_X86_INS_MOVSX>(isStatic);

  case x86::x86_insn::X86_INS_CMOVB:
    return std::make_unique<INST_X86_INS_CMOVB>(isStatic);

  case x86::x86_insn::X86_INS_MOV:
    return std::make_unique<INST_X86_INS_MOV>(isStatic);

  case x86::x86_insn::X86_INS_LEA:
    return std::make_unique<INST_X86_INS_LEA>(isStatic);

  case x86::x86_insn::X86_INS_XCHG:
    return std::make_unique<INST_X86_INS_XCHG>(isStatic);

  case x86::x86_insn::X86_INS_SBB:
    return std::make_unique<INST_X86_INS_SBB>(isStatic);

  case x86::x86_insn::X86_INS_IMUL:
    return std::make_unique<INST_X86_INS_IMUL>(isStatic);

  case x86::x86_insn::X86_INS_SHLD:
    return std::make_unique<INST_X86_INS_SHLD>(isStatic);

  case x86::x86_insn::X86_INS_SHRD:
    return std::make_unique<INST_X86_INS_SHRD>(isStatic);

  case x86::x86_insn::X86_INS_ADD:
    return std::make_unique<INST_X86_INS_ADD>(isStatic);

  case x86::x86_insn::X86_INS_SUB:
    return std::make_unique<INST_X86_INS_SUB>(isStatic);

  case x86::x86_insn::X86_INS_AND:
    return std::make_unique<INST_X86_INS_AND>(isStatic);

  case x86::x86_insn::X86_INS_ADC:
    return std::make_unique<INST_X86_INS_ADC>(isStatic);

  case x86::x86_insn::X86_INS_ROR:
    return std::make_unique<INST_X86_INS_ROR>(isStatic);

  case x86::x86_insn::X86_INS_ROL:
    return std::make_unique<INST_X86_INS_ROL>(isStatic);

  case x86::x86_insn::X86_INS_OR:
    return std::make_unique<INST_X86_INS_OR>(isStatic);

  case x86::x86_insn::X86_INS_XOR:
    return std::make_unique<INST_X86_INS_XOR>(isStatic);

  case x86::x86_insn::X86_INS_SHL:
    return std::make_unique<INST_X86_INS_SHL>(isStatic);

  case x86::x86_insn::X86_INS_SHR:
    return std::make_unique<INST_X86_INS_SHR>(isStatic);

  case x86::x86_insn::X86_INS_SAR:
    return std::make_unique<INST_X86_INS_SAR>(isStatic);

  case x86::x86_insn::X86_INS_CALL:
    return std::make_unique<INST_X86_INS_CALL>(isStatic);

  case x86::x86_insn::X86_INS_RET:
    return std::make_unique<INST_X86_INS_RET>(isStatic);

  case x86::x86_insn::X86_INS_LEAVE:
    return std::make_unique<INST_X86_INS_LEAVE>(isStatic);

  case x86::x86_insn::X86_INS_ENTER:
    return std::make_unique<INST_X86_INS_ENTER>(isStatic);

  case x86::x86_insn::X86_INS_DIV:
    return std::make_unique<INST_X86_INS_DIV>(isStatic);

  case x86::x86_insn::X86_INS_TEST:
    return std::make_unique<INST_X86_INS_TEST>(isStatic);

  case x86::x86_insn::X86_INS_CMP:
    return std::make_unique<INST_X86_INS_CMP>(isStatic);

  case x86::x86_insn::X86_INS_JMP:
    return std::make_unique<INST_X86_INS_JMP>(isStatic);

  case x86::x86_insn::X86_INS_JA:
    return std::make_unique<INST_X86_INS_JA>(isStatic);

  case x86::x86_insn::X86_INS_JAE:
    return std::make_unique<INST_X86_INS_JAE>(isStatic);

  case x86::x86_insn::X86_INS_JB:
    return std::make_unique<INST_X86_INS_JB>(isStatic);

  case x86::x86_insn::X86_INS_JBE:
    return std::make_unique<INST_X86_INS_JBE>(isStatic);

  case x86::x86_insn::X86_INS_JCXZ:
    return std::make_unique<INST_X86_INS_JC>(isStatic);

  case x86::x86_insn::X86_INS_JE:
    return std::make_unique<INST_X86_INS_JE>(isStatic);

  case x86::x86_insn::X86_INS_JG:
    return std::make_unique<INST_X86_INS_JG>(isStatic);

  case x86::x86_insn::X86_INS_JGE:
    return std::make_unique<INST_X86_INS_JGE>(isStatic);

  case x86::x86_insn::X86_INS_JL:
    return std::make_unique<INST_X86_INS_JL>(isStatic);

  case x86::x86_insn::X86_INS_JLE:
    return std::make_unique<INST_X86_INS_JLE>(isStatic);

  case x86::x86_insn::X86_INS_JNLE:
    return std::make_unique<INST_X86_INS_JNLE>(isStatic);

  case x86::x86_insn::X86_INS_JNA:
    return std::make_unique<INST_X86_INS_JNA>(isStatic);

  case x86::x86_insn::X86_INS_JNAE:
    return std::make_unique<INST_X86_INS_JNAE>(isStatic);

  case x86::x86_insn::X86_INS_JNB:
    return std::make_unique<INST_X86_INS_JNB>(isStatic);

  case x86::x86_insn::X86_INS_JNBE:
    return std::make_unique<INST_X86_INS_JNBE>(isStatic);

  case x86::x86_insn::X86_INS_JNC:
    return std::make_unique<INST_X86_INS_JNC>(isStatic);

  case x86::x86_insn::X86_INS_JNE:
    return std::make_unique<INST_X86_INS_JNE>(isStatic);

  case x86::x86_insn::X86_INS_JNG:
    return std::make_unique<INST_X86_INS_JNG>(isStatic);

  case x86::x86_insn::X86_INS_JNL:
    return std::make_unique<INST_X86_INS_JNL>(isStatic);

  case x86::x86_insn::X86_INS_JNO:
    return std::make_unique<INST_X86_INS_JNO>(isStatic);

  case x86::x86_insn::X86_INS_JNS:
    return std::make_unique<INST_X86_INS_JNS>(isStatic);

  case x86::x86_insn::X86_INS_JNZ:
    return std::make_unique<INST_X86_INS_JNZ>(isStatic);

  case x86::x86_insn::X86_INS_JO:
    return std::make_unique<INST_X86_INS_JO>(isStatic);

  case x86::x86_insn::X86_INS_JS:
    return std::make_unique<INST_X86_INS_JS>(isStatic);

  case x86::x86_insn::X86_INS_JZ:
    return std::make_unique<INST_X86_INS_JZ>(isStatic);

  case x86::x86_insn::X86_INS_STOSD:
    return std::make_unique<INST_X86_INS_STOSD>(isStatic);

  case x86::x86_insn::X86_INS_STOSB:
    return std::make_unique<INST_X86_INS_STOSB>(isStatic);

  case x86::x86_insn::X86_INS_CMOVZ:
    return std::make_unique<INST_X86_INS_CMOVZ>(isStatic);

  case x86::x86_insn::X86_INS_CMOVNS:
    return std::make_unique<INST_X86_INS_CMOVNS>(isStatic);

  case x86::x86_insn::X86_INS_SETE:
  case x86::x86_insn::X86_INS_SETZ:
    return std::make_unique<INST_X86_INS_SETZ>(isStatic);

  case x86::x86_insn::X86_INS_SETNZ:
    return std::make_unique<INST_X86_INS_SETNZ>(isStatic);

  case x86::x86_insn::X86_INS_BT:
    return std::make_unique<INST_X86_INS_BT>(isStatic);

  case x86::x86_insn::X86_INS_MOVSB:
    return std::make_unique<INST_X86_INS_MOVSB>(isStatic);

  case x86::x86_insn::X86_INS_MOVSD:
    return std::make_unique<INST_X86_INS_MOVSD>(isStatic);

  case x86::x86_insn::X86_INS_MUL:
    return std::make_unique<INST_X86_INS_MUL>(isStatic);

  case x86::x86_insn::X86_INS_CLD:
    return std::make_unique<INST_X86_INS_CLD>(isStatic);

  case x86::x86_insn::X86_INS_CPUID:
    return std::make_unique<INST_X86_INS_CPUID>(isStatic);

  case x86::x86_insn::X86_INS_CMOVNB:
    return std::make_unique<INST_X86_INS_CMOVNB>(isStatic);

  case x86::x86_insn::X86_INS_CDQ:
    return std::make_unique<INST_X86_INS_CDQ>(isStatic);

  case x86::x86_insn::X86_INS_CMOVBE:
    return std::make_unique<INST_X86_INS_CMOVBE>(isStatic);

  case x86::x86_insn::X86_INS_SYSENTER:
    return std::make_unique<INST_X86_INS_SYSENTER>(isStatic);

  case x86::x86_insn::X86_INS_JECXZ:
    return std::make_unique<INST_X86_INS_JECXZ>(isStatic);

  case x86::x86_insn::X86_INS_CMPXCHG:
    return std::make_unique<INST_X86_INS_CMPXCHG>(isStatic);

  case x86::x86_insn::X86_INS_SETB:
    return std::make_unique<INST_X86_INS_SETB>(isStatic);

  case x86::x86_insn::X86_INS_JP:
    return std::make_unique<INST_X86_INS_JP>(isStatic);

  case x86::x86_insn::X86_INS_MOVSW:
    return std::make_unique<INST_X86_INS_MOVSW>(isStatic);

  case x86::x86_insn::X86_INS_BSWAP:
    return std::make_unique<INST_X86_INS_BSWAP>(isStatic);

  case x86::x86_insn::X86_INS_CMOVNE:
  case x86::x86_insn::X86_INS_CMOVNZ:
    return std::make_unique<INST_X86_INS_CMOVNZ>(isStatic);

  case x86::x86_insn::X86_INS_SETNBE:
    return std::make_unique<INST_X86_INS_SETNBE>(isStatic);

  case x86::x86_insn::X86_INS_SETBE:
    return std::make_unique<INST_X86_INS_SETBE>(isStatic);

  case x86::x86_insn::X86_INS_LODSD:
    return std::make_unique<INST_X86_INS_LODSD>(isStatic);

  case x86::x86_insn::X86_INS_CMOVNBE:
    return std::make_unique<INST_X86_INS_CMOVNBE>(isStatic);

  case x86::x86_insn::X86_INS_BSF:
    return std::make_unique<INST_X86_INS_BSF>(isStatic);

  case x86::x86_insn::X86_INS_SETL:
    return std::make_unique<INST_X86_INS_SETL>(isStatic);

  case x86::x86_insn::X86_INS_SETLE:
    return std::make_unique<INST_X86_INS_SETLE>(isStatic);

  case x86::x86_insn::X86_INS_RDTSC:
    return std::make_unique<INST_X86_INS_RDTSC>(isStatic);

  case x86::x86_insn::X86_INS_SETNB:
    return std::make_unique<INST_X86_INS_SETNB>(isStatic);

  case x86::x86_insn::X86_INS_CMOVS:
    return std::make_unique<INST_X86_INS_CMOVS>(isStatic);

  case x86::x86_insn::X86_INS_SETNLE:
    return std::make_unique<INST_X86_INS_SETNLE>(isStatic);

  case x86::x86_insn::X86_INS_CMOVLE:
    return std::make_unique<INST_X86_INS_CMOVLE>(isStatic);

  case x86::x86_insn::X86_INS_CMOVNL:
    return std::make_unique<INST_X86_INS_CMOVNL>(isStatic);

  case x86::x86_insn::X86_INS_CMOVL:
    return std::make_unique<INST_X86_INS_CMOVL>(isStatic);

  case x86::x86_insn::X86_INS_SETNL:
    return std::make_unique<INST_X86_INS_SETNL>(isStatic);

  case x86::x86_insn::X86_INS_IDIV:
    return std::make_unique<INST_X86_INS_IDIV>(isStatic);

  case x86::x86_insn::X86_INS_PUSHAL:
    return std::make_unique<INST_X86_INS_PUSHAL>(isStatic);

  default: {
    if (func != nullptr) {
      std::string info = "unrecognized inst: " + func->getFunName(addr);
      WARN(info.c_str());
      return std::make_unique<Inst_Base>(isStatic);
    }

    WARN("unrecognized instructions");
    return std::make_unique<Inst_Base>(isStatic);
  }
  }
}

std::shared_ptr<BitVector>
inst_dyn_details::two_operand(SEEngine *se, Inst_Base *inst, BVOper bvoper) {
  std::shared_ptr<Operand> op0 = inst->oprd[0];
  std::shared_ptr<Operand> op1 = inst->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = inst->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(inst->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(bvoper, v0, v1);
    se->writeReg(op0->field[0], res);
    return res;
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(inst->get_memory_address(), op0->bit);
    res = buildop2(bvoper, v0, v1);
    se->writeMem(inst->get_memory_address(), op0->bit, res);
    return res;
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  return nullptr;
}

bool UPDATE_SYMBOLS::symbolic_execution(SEEngine *se) {
  se->updateSecrets(this->m_key_symbol, this->m_key_value);
  return true;
}

bool INST_X86_INS_NOP::symbolic_execution(tana::SEEngine *se) { return true; }

bool INST_X86_INS_PUSH::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<BitVector> v0;
  std::shared_ptr<BitVector> esp = se->readReg("esp");
  uint32_t dec = op0->bit / T_BYTE_SIZE;
  esp = buildop2(BVOper::bvsub, esp, dec);
  se->writeReg("esp", esp);
  if (op0->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op0->field[0], nullptr, 16);
    v0 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    se->writeMem(this->get_memory_address(), op0->bit, v0);
    return true;
  }

  if (op0->type == Operand::Reg) {
    auto regV = se->readReg(op0->field[0]);
    se->writeMem(this->get_memory_address(), op0->bit, regV);
    return true;
  }
  if (op0->type == Operand::Mem) {
    // The memaddr in the trace is the read address
    // We need to compute the write address
    auto reg = Registers::convert2RegID("esp");
    uint32_t esp_index = Registers::getRegIndex(reg);
    uint32_t esp_value = this->vcpu.gpr[esp_index];
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    auto mem_esp_v = esp_value - 4;
    std::stringstream sstream;
    sstream << std::hex << mem_esp_v << std::dec;
    se->writeMem(sstream.str(), op0->bit, v0);
    return true;
  }

  ERROR("push error: the operand is not Imm, Reg or Mem!");
  return false;
}

bool INST_X86_INS_POP::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];

  std::shared_ptr<BitVector> esp = se->readReg("esp");
  uint32_t add_size = op0->bit / T_BYTE_SIZE;
  esp = buildop2(BVOper::bvadd, esp, add_size);

  se->writeReg("esp", esp);
  if (op0->type == Operand::Reg) {
    // assert(Registers::getRegType(op0->field[0]) == FULL);
    auto v0 = se->readMem(this->get_memory_address(), op0->bit);
    se->writeReg(op0->field[0], v0);
    return true;
  }

  ERROR("pop error: the operand is not Reg!");
  return false;
}

bool INST_X86_INS_NEG::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);
  std::shared_ptr<BitVector> v0, res;
  bool status = false;
  uint32_t size = 0;

  if (op0->type == Operand::Reg) {
    size = Registers::getRegSize(op0->field[0]) * T_BYTE_SIZE;
    v0 = se->readReg(op0->field[0]);
    res = buildop1(BVOper::bvneg, v0);
    se->writeReg(op0->field[0], res);
    status = true;
  }

  if (op0->type == Operand::Mem) {
    size = op0->bit;
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop1(BVOper::bvneg, v0);
    se->writeMem(this->get_memory_address(), op0->bit, res);
    status = true;
  }

  if (se->eflags) {
    auto zero = std::make_shared<BitVector>(ValueType::CONCRETE, 0);
    // Update CF
    updateCFsub(se, zero, v0);

    // Update SF
    updateSF(se, res, size);

    // Update ZF
    updateZF(se, res);

    // Update OF
    updateOFsub(se, zero, v0, op0->bit);
  }

  if (status)
    return true;

  ERROR("neg error: the operand is not Reg!");
  return false;
}

bool INST_X86_INS_NOT::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];

  std::shared_ptr<BitVector> v0, res;
  bool status = false;

  if (op0->type == Operand::Reg) {
    v0 = se->readReg(op0->field[0]);
    res = buildop1(BVOper::bvnot, v0);
    se->writeReg(op0->field[0], res);
    status = true;
  }

  if (status)
    return true;

  ERROR("neg error: the operand is not Reg!");
  return false;
}

bool INST_X86_INS_INC::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);
  std::shared_ptr<BitVector> v0, res;

  if (op0->type == Operand::Reg) {
    v0 = se->readReg(op0->field[0]);
  }

  if (op0->type == Operand::Mem || op0->type == Operand::Label) {
    v0 = se->readMem(this->get_memory_address(), op0->bit);
  }

  auto bit_vec_one = std::make_shared<BitVector>(ValueType::CONCRETE, 1);
  res = buildop2(BVOper::bvadd, v0, bit_vec_one);

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], res);
  }

  if (op0->type == Operand::Mem) {
    // m_memory[it->memory_address] = res;
    se->writeMem(this->get_memory_address(), op0->bit, res);
  }

  if (se->eflags) {
    // Update CF
    // CF is not affected

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    auto one = std::make_shared<BitVector>(ValueType::CONCRETE, 1);
    updateOFadd(se, one, v0, op0->bit);
  }

  return true;
}

bool INST_X86_INS_DEC::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);
  std::shared_ptr<BitVector> v0, res;

  if (op0->type == Operand::Reg) {
    v0 = se->readReg(op0->field[0]);
  }

  if (op0->type == Operand::Mem) {
    v0 = se->readMem(this->get_memory_address(), op0->bit);
  }

  auto bit_vec_one = std::make_shared<BitVector>(ValueType::CONCRETE, 1);
  res = buildop2(BVOper::bvsub, v0, bit_vec_one);

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], res);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), op0->bit, res);
  }

  if (se->eflags) {
    // Update CF
    // CF is not affected

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    auto one = std::make_shared<BitVector>(ValueType::CONCRETE, 1);
    updateOFadd(se, v0, one, op0->bit);
  }

  return true;
}

bool INST_X86_INS_MOVSX::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op1->type == Operand::Reg) {
    auto reg = Registers::convert2RegID(op1->field[0]);
    v1 = se->readReg(reg);

    v1 = se->SignExt(v1, op1->bit, op0->bit);
    se->writeReg(op0->field[0], v1);
    return true;
  }
  if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
    v1 = se->SignExt(v1, op1->bit, op0->bit);
    se->writeReg(op0->field[0], v1);
    return true;
  }
  return false;
}

bool INST_X86_INS_MOVZX::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op1->type == Operand::Reg) {
    auto reg = Registers::convert2RegID(op1->field[0]);
    v1 = se->readReg(reg);
    v1 = se->ZeroExt(v1, op0->bit);
    se->writeReg(op0->field[0], v1);
    return true;
  }
  if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
    v1 = se->ZeroExt(v1, op0->bit);
    se->writeReg(op0->field[0], v1);
    return true;
  }

  ERROR("MOVZX");
  return false;
}

bool INST_X86_INS_CMOVB::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;
  auto opcode_id = this->instruction_id;
  bool CF = false;

  if (!(this->vcpu.eflags_state))
    WARN("CMOVB doesn't have eflags information");

  CF = this->vcpu.CF();
  if (!CF)
    return true;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete));
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(this->get_memory_address(), op0->bit,
                   std::make_shared<BitVector>(ValueType::CONCRETE,
                                               temp_concrete,
                                               se->isImmSym(temp_concrete)));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

bool INST_X86_INS_MOV::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      if (v1 == nullptr){
        return false;
      }
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

bool INST_X86_INS_LEA::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;
  /* lea reg, ptr [edx+eax*1]
     interpret lea instruction based on different address type
     1. op0 must be reg
     2. op1 must be addr
  */
  if (op0->type != Operand::Reg || op1->type != Operand::Mem) {
    ERROR("lea format error!");
    return false;
  }

  switch (op1->tag) {
  case 5: {
    std::shared_ptr<BitVector> f0, f1,
        f2; // corresponding field[0-2] in operand
    f0 = se->readReg(op1->field[0]);
    f1 = se->readReg(op1->field[1]);
    if (op1->field[2] == "1") {
      res = buildop2(BVOper::bvadd, f0, f1);
      // m_ctx[getRegName(op0->field[0])] = res;
      se->writeReg(op0->field[0], res);
      return true;
    }
    uint32_t temp_concrete = stoul(op1->field[2], nullptr, 16);
    f2 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    res = buildop2(BVOper::bvimul, f1, f2);
    res = buildop2(BVOper::bvadd, f0, res);
    se->writeReg(op0->field[0], res);
    return true;
  }
  case 7: {
    std::shared_ptr<BitVector> f0, f1, f2, f3; // addr7: eax+ebx*2+0xfffff1
    // f0 = m_ctx[getRegName(op1->field[0])];       //eax
    // f1 = m_ctx[getRegName(op1->field[1])];       //ebx

    f0 = se->readReg(op1->field[0]);
    f1 = se->readReg(op1->field[1]);

    uint32_t temp_concrete1 = stoul(op1->field[2], nullptr, 16);
    f2 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete1,
                                     se->isImmSym(temp_concrete1)); // 2
    std::string sign = op1->field[3];                               //+
    uint32_t temp_concrete2 = stoul(op1->field[4], nullptr, 16);
    f3 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete2,
                                     se->isImmSym(temp_concrete2)); // 0xfffff1
    assert((sign == "+") || (sign == "-"));
    if (op1->field[2] == "1") {
      res = buildop2(BVOper::bvadd, f0, f1);
    } else {
      res = buildop2(BVOper::bvimul, f1, f2);
      res = buildop2(BVOper::bvadd, f0, res);
    }
    if (sign == "+")
      res = buildop2(BVOper::bvadd, res, f3);
    else
      res = buildop2(BVOper::bvsub, res, f3);
    // m_ctx[getRegName(op0->field[0])] = res;

    se->writeReg(op0->field[0], res);

    return true;
  }
  case 4: {
    std::shared_ptr<BitVector> f0, f1; // addr4: eax+0xfffff1
    // f0 = m_ctx[getRegName(op1->field[0])];       //eax
    f0 = se->readReg(op1->field[0]);
    uint32_t temp_concrete = stoul(op1->field[2], nullptr, 16);
    f1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete)); // 0xfffff1
    std::string sign = op1->field[1];                              //+
    if (sign == "+")
      res = buildop2(BVOper::bvadd, f0, f1);
    else
      res = buildop2(BVOper::bvsub, f0, f1);
    // m_ctx[getRegName(op0->field[0])] = res;

    se->writeReg(op0->field[0], res);
    return true;
  }
  case 6: {
    std::shared_ptr<BitVector> f0, f1, f2; // addr6: eax*2+0xfffff1
    // f0 = m_ctx[getRegName(op1->field[0])];
    f0 = se->readReg(op1->field[0]);
    uint32_t temp_concrete1 = stoul(op1->field[1], nullptr, 16);
    uint32_t temp_concrete2 = stoul(op1->field[3], nullptr, 16);

    f1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete1,
                                     se->isImmSym(temp_concrete1));
    f2 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete2,
                                     se->isImmSym(temp_concrete2));
    std::string sign = op1->field[2];
    if (op1->field[1] == "1") {
      res = f0;
    } else {
      res = buildop2(BVOper::bvimul, f0, f1);
    }
    if (sign == "+")
      res = buildop2(BVOper::bvadd, res, f2);
    else
      res = buildop2(BVOper::bvsub, res, f2);
    se->writeReg(op0->field[0], res);
    return true;
  }
  case 3: {
    std::shared_ptr<BitVector> f0, f1; // addr3: eax*2
    // f0 = m_ctx[getRegName(op1->field[0])];
    f0 = se->readReg(op1->field[0]);

    uint32_t temp_concrete = stoul(op1->field[1], nullptr, 16);
    f1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    res = buildop2(BVOper::bvimul, f0, f1);
    // m_ctx[getRegName(op0->field[0])] = res;
    se->writeReg(op0->field[0], res);
    return true;
  }

  case 2: {
    v1 = se->readReg(op1->field[0]);
    se->writeReg(op0->field[0], v1);
    return true;
  }

  case 1: {
    std::shared_ptr<BitVector> f0;
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    f0 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    se->writeReg(op0->field[0], f0);
    return true;
  }
  default:
    ERROR("Other tags in addr is not ready for lea!");
    return false;
  }
}

bool INST_X86_INS_XCHG::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;
  auto opcode_id = this->instruction_id;

  if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
    if (op0->type == Operand::Reg) {
      v0 = se->readReg(op0->field[0]);
      v1 = se->readReg(op1->field[0]);

      se->writeReg(op1->field[0], v0);
      se->writeReg(op0->field[0], v1);

      return true;
    }
    if (op0->type == Operand::Mem) {
      v0 = se->readMem(this->get_memory_address(), op0->bit);
      v1 = se->readReg(op1->field[0]);

      // m_ctx[getRegName(op1->field[0])] = v0; // xchg mem, reg
      // m_memory[it->memory_address] = v1;
      se->writeReg(op1->field[0], v0);
      se->writeMem(this->get_memory_address(), op0->bit, v1);

      return true;
    }
    ERROR("xchg error: 1");
    return false;
  }
  if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
    if (op0->type == Operand::Reg) {
      v0 = se->readReg(op0->field[0]);
      se->writeReg(op0->field[0], v1); // xchg reg, mem
      se->writeMem(this->get_memory_address(), op1->bit, v0);
    }
    ERROR("xchg error 3");
    return false;
  }
  ERROR("xchg error: 2");
  return false;
}

bool INST_X86_INS_SBB::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];

  std::shared_ptr<BitVector> v0, v1, res;
  bool flags = false;
  if (!this->vcpu.eflags_state) {
    WARN("SBB doesn't have eflags information");
    return false;
  }
  auto CF = this->vcpu.CF();

  if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  }

  if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  }

  if (op1->type == Operand::ImmValue) {
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, op1->field[0]);
  }

  if (op0->type == Operand::Reg) {
    v0 = se->readReg(op0->field[0]);
    flags = true;
  }

  if (op0->type == Operand::Mem) {
    v0 = se->readMem(this->get_memory_address(), op1->bit);
    flags = true;
  }

  if (CF) {
    auto one_bit = std::make_shared<BitVector>(ValueType::CONCRETE, 1);
    res = buildop2(BVOper::bvsub, v0, v1);
    res = buildop2(BVOper::bvsub, res, one_bit);
  } else {
    res = buildop2(BVOper::bvsub, v0, v1);
  }

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], res);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), op0->bit, res);
  }

  if (se->eflags) {
    // Update CF
    updateCFsub(se, v0, v1);

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    updateOFsub(se, v0, v1, op0->bit);
  }

  return flags;
}

bool INST_X86_INS_IMUL::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<Operand> op2 = this->oprd[2];
  std::shared_ptr<BitVector> v1, v2, v3, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);
  if (this->get_operand_number() == 1) {
    uint32_t eax_c = se->getRegisterConcreteValue("eax");
    uint32_t edx_c = se->getRegisterConcreteValue("edx");

    auto eax_v = std::make_shared<BitVector>(ValueType::CONCRETE, eax_c);
    auto edx_v = std::make_shared<BitVector>(ValueType::CONCRETE, edx_c);

    se->writeReg("eax", eax_v);
    se->writeReg("edx", edx_v);
    return true;
  }

  if (this->get_operand_number() == 2) {
    assert(op0->type == Operand::Reg);
    v1 = se->readReg(op0->field[0]);
    if (op1->type == Operand::Reg) {
      v2 = se->readReg(op1->field[0]);
    } else {
      v2 = se->readMem(this->get_memory_address(), op1->bit);
    }

    res = buildop2(BVOper::bvimul, v1, v2);
    se->writeReg(op0->field[0], res);
    return true;
  }

  if (op0->type == Operand::Reg && op1->type == Operand::Reg &&
      op2->type == Operand::ImmValue) { // imul reg, reg, imm
    v1 = se->readReg(op1->field[0]);
    uint32_t temp_concrete = stoul(op2->field[0], 0, 16);
    v2 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    res = buildop2(BVOper::bvimul, v1, v2);
    se->writeReg(op0->field[0], res);
    return true;
  }

  ERROR("three operands instructions other than imul are not handled!");
  return false;
}

bool INST_X86_INS_SHLD::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<Operand> op2 = this->oprd[2];
  std::shared_ptr<BitVector> v1, v2, v3, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);

  if (op0->type == Operand::Reg && op1->type == Operand::Reg &&
      op2->type == Operand::ImmValue) { // shld shrd reg, reg, imm
    v1 = se->readReg(op0->field[0]);
    v2 = se->readReg(op1->field[0]);
    uint32_t temp_concrete = stoul(op2->field[0], 0, 16);
    v3 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    res = buildop3(BVOper::bvshld, v1, v2, v3);
    se->writeReg(op0->field[0], res);
    return true;
  }

  ERROR("shld is not handled!");
  return false;
}

bool INST_X86_INS_SHRD::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<Operand> op2 = this->oprd[2];
  std::shared_ptr<BitVector> v1, v2, v3, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);

  if (op0->type == Operand::Reg && op1->type == Operand::Reg &&
      op2->type == Operand::ImmValue) { // shld shrd reg, reg, imm
    v1 = se->readReg(op0->field[0]);
    v2 = se->readReg(op1->field[0]);
    uint32_t temp_concrete = stoul(op2->field[0], 0, 16);
    v3 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
    res = buildop3(BVOper::bvshrd, v1, v2, v3);
    se->writeReg(op0->field[0], res);
    return true;
  }

  ERROR("shrd is not handled!");
  return false;
}

bool INST_X86_INS_ADD::symbolic_execution(SEEngine *se) {

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvadd, v0, v1);
    se->writeReg(op0->field[0], res);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvadd, v0, v1);
    se->writeMem(this->get_memory_address(), op0->bit, res);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    updateCFadd(se, v1, v0, op0->bit);

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    updateOFadd(se, v0, v1, op0->bit);
  }

  return true;
}

bool INST_X86_INS_SUB::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvsub, v0, v1);
    se->writeReg(op0->field[0], res);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvsub, v0, v1);
    se->writeMem(this->get_memory_address(), op0->bit, res);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    updateCFsub(se, v0, v1);

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    updateOFsub(se, v0, v1, op0->bit);
  }

  return true;
}

bool INST_X86_INS_AND::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvand, v0, v1);
    se->writeReg(op0->field[0], res);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvand, v0, v1);
    se->writeMem(this->get_memory_address(), op0->bit, res);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    se->clearFlags("CF");

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    se->clearFlags("OF");
  }

  return true;
}

bool INST_X86_INS_ADC::symbolic_execution(SEEngine *se) {

  bool CF = false, status = false;
  if (this->vcpu.eflags_state)
    CF = this->vcpu.CF();

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = x86::insn_id2string(opcode_id);

  std::shared_ptr<BitVector> v_one =
      std::make_shared<BitVector>(ValueType::CONCRETE, 1, se->isImmSym(1));

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
    return false;
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvadd, v0, v1);
    if (CF) {
      res = buildop2(BVOper::bvadd, res, v_one);
    }
    se->writeReg(op0->field[0], res);
    status = true;
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvadd, v0, v1);
    if (CF) {
      res = buildop2(BVOper::bvadd, res, v_one);
    }
    se->writeMem(this->get_memory_address(), op0->bit, res);
    status = true;
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
    return false;
  }

  if (se->eflags) {
    // Update CF
    updateCFadd(se, v1, v0, op0->bit);

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    updateOFadd(se, v0, v1, op0->bit);
  }

  return true;
}

bool INST_X86_INS_ROR::symbolic_execution(SEEngine *se) {
  auto res = inst_dyn_details::two_operand(se, this, BVOper::bvror);
  // Update ZF
  updateZF(se, res);
  return true;
}

bool INST_X86_INS_ROL::symbolic_execution(SEEngine *se) {
  auto res = inst_dyn_details::two_operand(se, this, BVOper::bvrol);
  // Update ZF
  updateZF(se, res);
  return true;
}

bool INST_X86_INS_SHL::symbolic_execution(SEEngine *se) {
  auto res = inst_dyn_details::two_operand(se, this, BVOper::bvshl);
  // Update ZF
  updateZF(se, res);
  return true;
}

bool INST_X86_INS_SHR::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvshr, v0, v1);
    se->writeReg(op0->field[0], res);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvshr, v0, v1);
    se->writeMem(this->get_memory_address(), op0->bit, res);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  // Update ZF
  updateZF(se, res);
  return true;
}

bool INST_X86_INS_SAR::symbolic_execution(SEEngine *se) {
  auto res = inst_dyn_details::two_operand(se, this, BVOper::bvsar);

  // Update ZF
  updateZF(se, res);
  return true;
}

bool INST_X86_INS_OR::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvor, v0, v1);
    se->writeReg(op0->field[0], res);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvor, v0, v1);
    se->writeMem(this->get_memory_address(), op0->bit, res);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    se->clearFlags("CF");

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    se->clearFlags("OF");
  }

  return true;
}

bool INST_X86_INS_XOR::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  auto opcode_id = this->instruction_id;
  auto opcstr = "bv" + x86::insn_id2string(opcode_id);

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    if (v0 == v1) {
      res = std::make_shared<BitVector>(ValueType::CONCRETE, 0);
    } else {
      res = buildop2(BVOper::bvxor, v0, v1);
    }
    se->writeReg(op0->field[0], res);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    if (v0 == v1) {
      res = std::make_shared<BitVector>(ValueType::CONCRETE, 0);
    } else {
      res = buildop2(BVOper::bvxor, v0, v1);
    }
    se->writeMem(this->get_memory_address(), op0->bit, res);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    se->clearFlags("CF");

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    se->clearFlags("OF");
  }

  return true;
}

bool INST_X86_INS_CALL::symbolic_execution(SEEngine *se) {
  std::shared_ptr<BitVector> esp = se->readReg("esp");
  uint32_t sub_size = 4;
  esp = buildop2(BVOper::bvsub, esp, sub_size);
  se->writeReg("esp", esp);
  uint32_t eip = this->address + 5; // X86 call 0xf77268ed
  auto v_eip = std::make_shared<BitVector>(ValueType::CONCRETE, eip);

  se->writeMem(this->get_memory_address(), REGISTER_SIZE, v_eip);
  return true;
}

bool INST_X86_INS_RET::symbolic_execution(SEEngine *se) {
  std::shared_ptr<BitVector> esp = se->readReg("esp");
  uint32_t add_size = 4;
  if (oprd[0] != nullptr) {
    assert(oprd[0]->type == Operand::ImmValue);
    add_size = add_size + std::stoul(oprd[0]->field[0], nullptr, 16);
  }
  esp = buildop2(BVOper::bvadd, esp, add_size);
  se->writeReg("esp", esp);

  if (this->is_static)
    return true;

  auto ecx_v = se->readReg("ecx");
  if (ecx_v->symbol_num() == 0) {
    auto ecx_c = se->getRegisterConcreteValue("ecx");
    ecx_v = std::make_shared<BitVector>(ValueType::CONCRETE, ecx_c);
    se->writeReg("ecx", ecx_v);
  }

  auto edx_v = se->readReg("edx");
  if (edx_v->symbol_num() == 0) {
    auto edx_c = se->getRegisterConcreteValue("edx");
    edx_v = std::make_shared<BitVector>(ValueType::CONCRETE, edx_c);
    se->writeReg("edx", edx_v);
  }

  auto esi_v = se->readReg("esi");
  if (esi_v->symbol_num() == 0) {
    auto esi_c = se->getRegisterConcreteValue("esi");
    esi_v = std::make_shared<BitVector>(ValueType::CONCRETE, esi_c);
    se->writeReg("esi", esi_v);
  }

  auto edi_v = se->readReg("edi");
  if (edi_v->symbol_num() == 0) {
    auto edi_c = se->getRegisterConcreteValue("edi");
    edi_v = std::make_shared<BitVector>(ValueType::CONCRETE, edi_c);
    se->writeReg("edi", edi_v);
  }

  return true;
}

bool INST_X86_INS_LEAVE::symbolic_execution(SEEngine *se) {

  // EBP = ESP
  auto v_ebp = se->readReg("ebp");
  v_ebp = buildop2(BVOper::bvadd, v_ebp, 4);
  se->writeReg("esp", v_ebp);

  // POP EBP
  auto v0 = se->readMem(this->get_memory_address(), REGISTER_SIZE);
  se->writeReg("ebp", v0);

  return true;
}

bool INST_X86_INS_ENTER::symbolic_execution(SEEngine *se) { return true; }

bool INST_X86_INS_DIV::symbolic_execution(SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  auto operand_size = op0->bit;
  assert(operand_size == 8 || operand_size == 16 || operand_size == 32);
  std::shared_ptr<BitVector> dividend = nullptr, divisor = nullptr,
                             quotient = nullptr, remainder = nullptr;

  if (op0->type == Operand::Mem) {
    divisor = se->readMem(get_memory_address(), op0->bit);
  }

  if (op0->type == Operand::Reg) {
    divisor = se->readReg(op0->field[0]);
  }

  if (operand_size == 8) {
    dividend = se->readReg("ax");

    quotient = buildop2(BVOper::bvquo, dividend, divisor);
    remainder = buildop2(BVOper::bvrem, dividend, divisor);

    quotient = SEEngine::Extract(quotient, 1, 8);
    remainder = SEEngine::Extract(remainder, 1, 8);
    se->writeReg("al", quotient);
    se->writeReg("ah", remainder);

    return true;
  }
  if (operand_size == 16) {
    auto temp1 = se->readReg("ax");
    auto temp2 = se->readReg("dx");
    dividend = se->Concat(temp2, temp1);

    quotient = buildop2(BVOper::bvquo, dividend, divisor);
    remainder = buildop2(BVOper::bvrem, dividend, divisor);

    quotient = SEEngine::Extract(quotient, 1, 16);
    remainder = SEEngine::Extract(remainder, 1, 16);
    se->writeReg("ax", quotient);
    se->writeReg("dx", remainder);

    return true;
  }
  if (operand_size == 32) {
    auto eax_v = se->readReg("eax");
    auto edx_v = se->readReg("edx");

    quotient = buildop3(BVOper::bvdiv32_quo, edx_v, eax_v, divisor);
    remainder = buildop3(BVOper::bvdiv32_rem, edx_v, eax_v, divisor);

    se->writeReg("eax", quotient);
    se->writeReg("edx", remainder);

    return true;
  }

  return false;
}

// The And instrucntion without storing the result
bool INST_X86_INS_TEST::symbolic_execution(tana::SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvand, v0, v1);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvand, v0, v1);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    se->clearFlags("CF");

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    se->clearFlags("OF");
  }

  return true;
}

bool INST_X86_INS_CMP::symbolic_execution(tana::SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;
  std::string opcstr = "bvsub";

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("other instructions: op1 is not ImmValue, Reg, or Mem!");
  }

  if (op0 == nullptr) {
    return true;
  }
  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
    res = buildop2(BVOper::bvsub, v0, v1);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
    res = buildop2(BVOper::bvsub, v0, v1);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  if (se->eflags) {
    // Update CF
    updateCFsub(se, v0, v1);

    // Update SF
    updateSF(se, res, op0->bit);

    // Update ZF
    updateZF(se, res);

    // Update OF
    updateOFsub(se, v0, v1, op0->bit);
  }

  return true;
}

bool INST_X86_INS_JMP::symbolic_execution(tana::SEEngine *se) { return true; }

// JA CF = 0 and ZF = 0
bool INST_X86_INS_JA::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");

  if (this->vcpu.CF() == 0 && this->vcpu.ZF() == 0) {
    auto CF_O = buildop2(BVOper::equal, CF, 0);
    auto ZF_O = buildop2(BVOper::equal, ZF, 0);
    auto constrains =
        std::make_shared<Constrain>(this->id, CF_O, BVOper::bvand, ZF_O);
    se->updateCFConstrains(constrains);
  } else {
    auto CF_O = buildop2(BVOper::equal, CF, 1);
    auto ZF_O = buildop2(BVOper::equal, ZF, 1);
    auto constrains =
        std::make_shared<Constrain>(this->id, CF_O, BVOper::bvor, ZF_O);
    se->updateCFConstrains(constrains);
  }
  return true;
}

// JAE CF = 0
bool INST_X86_INS_JAE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<Constrain> constrains;
  if (vcpu.CF() == 0) {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 0);
  } else {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 1);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JB CF = 1
bool INST_X86_INS_JB::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.CF()) {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 1);
  } else {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 0);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JBE CF = 1 or ZF = 1
bool INST_X86_INS_JBE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");

  if (vcpu.CF() == 1 || vcpu.ZF() == 1) {
    auto CF_1 = buildop2(BVOper::equal, CF, 1);
    auto ZF_1 = buildop2(BVOper::equal, ZF, 1);
    auto constrains =
        std::make_shared<Constrain>(this->id, CF_1, BVOper::bvor, ZF_1);
    se->updateCFConstrains(constrains);
  } else {
    auto CF_0 = buildop2(BVOper::equal, CF, 0);
    auto ZF_0 = buildop2(BVOper::equal, ZF, 0);
    auto constrains =
        std::make_shared<Constrain>(this->id, CF_0, BVOper::bvand, ZF_0);
    se->updateCFConstrains(constrains);
  }
  return true;
}

// JC CF = 1
bool INST_X86_INS_JC::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<Constrain> constrains;
  if (vcpu.CF() == 1) {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 1);
  } else {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 0);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JE ZF = 1
bool INST_X86_INS_JE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<Constrain> constrains;
  if (vcpu.ZF() == 1) {
    constrains = std::make_shared<Constrain>(this->id, ZF, BVOper::equal, 1);
  } else {
    constrains = std::make_shared<Constrain>(this->id, ZF, BVOper::equal, 0);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JG ZF = 0 and SF = OF
bool INST_X86_INS_JG::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if ((vcpu.ZF() == 0) && (vcpu.SF() == vcpu.OF())) {
    auto ZF_O = buildop2(BVOper::equal, ZF, 0);
    auto SF_OF = buildop2(BVOper::equal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, SF_OF, BVOper::bvand, ZF_O);
  } else {
    auto ZF_1 = buildop2(BVOper::equal, ZF, 1);
    auto SFnOF = buildop2(BVOper::noequal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, SFnOF, BVOper::bvor, ZF_1);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JGE SF = OF
bool INST_X86_INS_JGE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;

  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.SF() == vcpu.OF()) {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::equal, OF);
  } else {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::noequal, OF);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JL SF != OF
bool INST_X86_INS_JL::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;

  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.SF() != vcpu.OF()) {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::noequal, OF);
  } else {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::equal, OF);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JLE ZF or SF != OF
bool INST_X86_INS_JLE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.ZF() || (vcpu.SF() != vcpu.OF())) {
    auto ZF_1 = buildop2(BVOper::equal, ZF, 1);
    auto SF_OF = buildop2(BVOper::noequal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, SF_OF, BVOper::bvor, ZF_1);
  } else {
    auto ZF_0 = buildop2(BVOper::equal, ZF, 0);
    auto SF_OF = buildop2(BVOper::equal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, SF_OF, BVOper::bvand, ZF_0);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNA CF or ZF
bool INST_X86_INS_JNA::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.CF() || vcpu.ZF()) {
    auto CF_1 = buildop2(BVOper::equal, CF, 1);
    auto ZF_1 = buildop2(BVOper::equal, ZF, 1);
    constrains =
        std::make_shared<Constrain>(this->id, CF_1, BVOper::bvor, ZF_1);
  } else {
    auto CF_0 = buildop2(BVOper::equal, CF, 0);
    auto ZF_0 = buildop2(BVOper::equal, ZF, 0);
    constrains =
        std::make_shared<Constrain>(this->id, CF_0, BVOper::bvand, ZF_0);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNAE CF
bool INST_X86_INS_JNAE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.CF()) {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 1);
  } else {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 0);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNB CF = 0
bool INST_X86_INS_JNB::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  if ((!this->is_static) && (CF->symbol_num() == 0)) {
    return true;
  }

  std::shared_ptr<Constrain> constrains;

  if (vcpu.CF() == 0) {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 0);
  } else {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 1);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNBE CF = 0 and ZF = 0
bool INST_X86_INS_JNBE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<Constrain> constrains;

  if ((vcpu.CF() == 0) && (vcpu.ZF() == 0)) {
    auto CF_0 = buildop2(BVOper::equal, CF, 0);
    auto ZF_0 = buildop2(BVOper::equal, ZF, 0);
    constrains =
        std::make_shared<Constrain>(this->id, CF_0, BVOper::bvand, ZF_0);
  } else {
    auto CF_1 = buildop2(BVOper::equal, CF, 1);
    auto ZF_1 = buildop2(BVOper::equal, ZF, 1);
    constrains =
        std::make_shared<Constrain>(this->id, CF_1, BVOper::bvor, ZF_1);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNC CF = 0
bool INST_X86_INS_JNC::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> CF = se->getFlags("CF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.CF() == 0) {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 0);
  } else {
    constrains = std::make_shared<Constrain>(this->id, CF, BVOper::equal, 1);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNE ZF = 0
bool INST_X86_INS_JNE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<Constrain> constrains;
  if (vcpu.ZF() == 0) {
    constrains = std::make_shared<Constrain>(this->id, ZF, BVOper::equal, 0);
  } else {
    constrains = std::make_shared<Constrain>(this->id, ZF, BVOper::equal, 1);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNG ZF or SF != OF
bool INST_X86_INS_JNG::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.ZF() || (vcpu.ZF() != vcpu.OF())) {
    std::shared_ptr<BitVector> ZF_1 = buildop2(BVOper::equal, ZF, 1);
    std::shared_ptr<BitVector> SF_OF = buildop2(BVOper::noequal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, ZF_1, BVOper::bvor, SF_OF);
  } else {
    std::shared_ptr<BitVector> ZF_0 = buildop2(BVOper::equal, ZF, 0);
    std::shared_ptr<BitVector> SF_OF = buildop2(BVOper::equal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, ZF_0, BVOper::bvand, SF_OF);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNGE SF != OF
bool INST_X86_INS_JNGE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.SF() != vcpu.OF()) {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::noequal, OF);
  } else {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::equal, OF);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNL SF = OF
bool INST_X86_INS_JNL::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if (vcpu.SF() == vcpu.OF()) {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::equal, OF);
  } else {
    constrains = std::make_shared<Constrain>(this->id, SF, BVOper::noequal, OF);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNLE ZF = 0 and SF = OF
bool INST_X86_INS_JNLE::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  if ((vcpu.ZF() == 0) && (vcpu.SF() == vcpu.OF())) {
    std::shared_ptr<BitVector> ZF_0 = buildop2(BVOper::equal, ZF, 0);
    std::shared_ptr<BitVector> SF_OF = buildop2(BVOper::equal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, ZF_0, BVOper::bvand, SF_OF);
  } else {
    std::shared_ptr<BitVector> ZF_1 = buildop2(BVOper::equal, ZF, 1);
    std::shared_ptr<BitVector> SFnOF = buildop2(BVOper::noequal, SF, OF);
    constrains =
        std::make_shared<Constrain>(this->id, ZF_1, BVOper::bvor, SFnOF);
  }
  se->updateCFConstrains(constrains);
  return true;
}

// JNO OF = 0
bool INST_X86_INS_JNO::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  constrains =
      std::make_shared<Constrain>(this->id, OF, BVOper::equal, vcpu.OF());
  se->updateCFConstrains(constrains);
  return true;
}

// JNS SF = 0
bool INST_X86_INS_JNS::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<Constrain> constrains;

  constrains =
      std::make_shared<Constrain>(this->id, SF, BVOper::equal, vcpu.SF());
  se->updateCFConstrains(constrains);
  return true;
}

// JNZ ZF = 0
bool INST_X86_INS_JNZ::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<Constrain> constrains;

  constrains =
      std::make_shared<Constrain>(this->id, ZF, BVOper::equal, vcpu.ZF());
  se->updateCFConstrains(constrains);
  return true;
}

// JO OF
bool INST_X86_INS_JO::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> OF = se->getFlags("OF");
  std::shared_ptr<Constrain> constrains;

  constrains =
      std::make_shared<Constrain>(this->id, OF, BVOper::equal, vcpu.OF());
  se->updateCFConstrains(constrains);
  return true;
}

// JS SF
bool INST_X86_INS_JS::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> SF = se->getFlags("SF");
  std::shared_ptr<Constrain> constrains;

  constrains =
      std::make_shared<Constrain>(this->id, SF, BVOper::equal, vcpu.SF());
  se->updateCFConstrains(constrains);
  return true;
}

// JZ ZF
bool INST_X86_INS_JZ::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;
  std::shared_ptr<BitVector> ZF = se->getFlags("ZF");
  std::shared_ptr<Constrain> constrains;

  constrains =
      std::make_shared<Constrain>(this->id, ZF, BVOper::equal, vcpu.ZF());

  se->updateCFConstrains(constrains);
  return true;
}

bool INST_X86_INS_REP_STOSB::symbolic_execution(tana::SEEngine *se) {

  assert(oprd[0]->type == Operand::Mem);
  // Update register
  auto v_reg = se->readReg(oprd[0]->field[0]);
  if (v_reg->symbol_num() == 0) {
    uint32_t reg_concrete = se->getRegisterConcreteValue(oprd[0]->field[0]);
    auto reg_v = std::make_shared<BitVector>(ValueType::CONCRETE, reg_concrete);
    se->writeReg(oprd[0]->field[0], reg_v);
  } else {
    v_reg = buildop2(BVOper::bvadd, v_reg, 4);
    se->writeReg(oprd[0]->field[0], v_reg);
  }
  // Store the contents of eax into the m_memory
  auto v_eax = se->readReg("ax");
  se->writeMem(this->get_memory_address(), oprd[0]->bit, v_eax);

  return true;
}

bool INST_X86_INS_REP_STOSD::symbolic_execution(tana::SEEngine *se) {

  // ecx = ecx - 1
  auto v_ecx = se->readReg("ecx");

  if (v_ecx->symbol_num() == 0) {
    uint32_t reg_concrete = se->getRegisterConcreteValue("ecx");
    auto reg_v = std::make_shared<BitVector>(ValueType::CONCRETE, reg_concrete);
    se->writeReg("ecx", reg_v);
  } else {
    v_ecx = buildop2(BVOper::bvsub, v_ecx, 1);
    se->writeReg("ecx", v_ecx);
  }

  assert(oprd[0]->type == Operand::Mem);
  // Update register
  auto v_reg = se->readReg(oprd[0]->field[0]);
  if (v_reg->symbol_num() == 0) {
    uint32_t reg_concrete = se->getRegisterConcreteValue(oprd[0]->field[0]);
    auto reg_v = std::make_shared<BitVector>(ValueType::CONCRETE, reg_concrete);
    se->writeReg(oprd[0]->field[0], reg_v);
  } else {
    v_reg = buildop2(BVOper::bvadd, v_reg, 4);
    se->writeReg(oprd[0]->field[0], v_reg);
  }
  // Store the contents of eax into the m_memory
  auto v_eax = se->readReg("eax");
  se->writeMem(this->get_memory_address(), oprd[0]->bit, v_eax);

  return true;
}

bool INST_X86_INS_STOSB::symbolic_execution(tana::SEEngine *se) {

  assert(oprd[0]->type == Operand::Mem);
  // Update register

  if (this->is_static) {
    auto v_reg = se->readReg(oprd[0]->field[0]);

    if (vcpu.DF() == 0) {
      v_reg = buildop2(BVOper::bvadd, v_reg, 1);
    } else {
      v_reg = buildop2(BVOper::bvsub, v_reg, 1);
    }
    se->writeReg(oprd[0]->field[0], v_reg);
  } else {
    auto v_reg = se->readReg(oprd[0]->field[0]);
    if (v_reg->val_type == ValueType::CONCRETE) {
      uint32_t concrete_value = se->getRegisterConcreteValue(oprd[0]->field[0]);
      v_reg = std::make_shared<BitVector>(ValueType::CONCRETE, concrete_value);
    } else {
      if (vcpu.DF() == 0) {
        v_reg = buildop2(BVOper::bvadd, v_reg, 1);
      } else {
        v_reg = buildop2(BVOper::bvsub, v_reg, 1);
      }
    }

    auto ecx_v = se->readReg("ecx");
    if (ecx_v->symbol_num() == 0) {
      uint32_t ecx_con = se->getRegisterConcreteValue("ecx");
      auto ecx_new = std::make_shared<BitVector>(ValueType::CONCRETE, ecx_con);
      se->writeReg("ecx", ecx_new);
    }

    se->writeReg(oprd[0]->field[0], v_reg);
  }

  // Store the contents of eax into the m_memory
  auto v_al = se->readReg("al");
  se->writeMem(this->get_memory_address(), oprd[0]->bit, v_al);
  return true;
}

bool INST_X86_INS_CMOVZ::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;

  bool zf = vcpu.ZF();
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> src;
  if (zf) {
    if (op0->type == Operand::Mem) {
      src = se->readMem(this->get_memory_address(), op0->bit);
    }
    if (op0->type == Operand::Reg) {
      src = se->readReg(op1->field[0]);
    }

    assert(op0->type == Operand::Reg);
    se->writeReg(op0->field[0], src);
    return true;

  } else {
    return true;
  }
}

bool INST_X86_INS_CMOVNS::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;

  bool sf = vcpu.SF();
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> src;
  if (!sf) {
    if (op0->type == Operand::Mem) {
      src = se->readMem(this->get_memory_address(), op0->bit);
    }
    if (op0->type == Operand::Reg) {
      src = se->readReg(op1->field[0]);
    }

    assert(op0->type == Operand::Reg);
    se->writeReg(op0->field[0], src);
    return true;

  } else {
    return true;
  }
}

bool INST_X86_INS_SETZ::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;

  std::shared_ptr<Operand> op0 = this->oprd[0];
  auto ZF = se->getFlags("ZF");
  ZF->high_bit = T_BYTE_SIZE;
  if (op0->type == Operand::Reg) {
    auto v = SEEngine::Extract(ZF, 1, op0->bit);
    se->writeReg(op0->field[0], v);
    return true;
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), op0->bit, ZF);
    return true;
  }
  return false;
}

bool INST_X86_INS_SETNZ::symbolic_execution(tana::SEEngine *se) {
  if (!se->eflags)
    return false;

  auto ZF = se->getFlags("ZF");
  auto value = buildop1(BVOper::bvbitnot, ZF);
  std::shared_ptr<Operand> op0 = this->oprd[0];
  value = se->Extract(value, 1, op0->bit);

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), op0->bit, value);
    return true;
  }
  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], value);
    return true;
  }

  return false;
}

bool INST_X86_INS_BT::symbolic_execution(tana::SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v1, v0, res;

  if (op1->type == Operand::ImmValue) {
    uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                     se->isImmSym(temp_concrete));
  } else if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else {
    ERROR("The second operand of bt should be IMM or Reg");
  }

  if (op0->type == Operand::Reg) { // dest op is reg
    v0 = se->readReg(op0->field[0]);
  } else if (op0->type == Operand::Mem) { // dest op is mem
    v0 = se->readMem(this->get_memory_address(), op0->bit);
  } else {
    ERROR("other instructions: op2 is not ImmValue, Reg, or Mem!");
  }

  res = buildop2(BVOper::bvbit, v0, v1);

  se->updateFlags("CF", res);

  return true;
}

bool INST_X86_INS_MOVSB::symbolic_execution(tana::SEEngine *se) {
  if (is_static) {
    ERROR("Static model doesn't support the instruction movsb");
    return false;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  auto src = op1->field[0];
  auto dest = op0->field[0];
  assert(op0->type == Operand::Mem);
  assert(op1->type == Operand::Mem);

  auto src_reg = se->readReg(src);
  auto dest_reg = se->readReg(dest);
  src_reg = buildop2(BVOper::bvadd, src_reg, 1);
  se->writeReg(src, src_reg);
  dest_reg = buildop2(BVOper::bvadd, dest_reg, 1);
  se->writeReg(dest, dest_reg);

  auto src_mem_address = this->read_reg_data(src);
  auto dest_mem_address = this->read_reg_data(dest);
  std::stringstream src_ss, dest_ss;
  src_ss << std::hex << src_mem_address << std::dec;
  dest_ss << std::hex << dest_mem_address << std::dec;

  auto src_value = se->readMem(src_ss.str(), op1->bit);
  se->writeMem(dest_ss.str(), op0->bit, src_value);

  return true;
}

bool INST_X86_INS_MOVSD::symbolic_execution(tana::SEEngine *se) {
  if (is_static) {
    ERROR("Static model doesn't support the instruction movsb");
    return false;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  auto src = op1->field[0];
  auto dest = op0->field[0];
  assert(op0->type == Operand::Mem);
  assert(op1->type == Operand::Mem);

  auto src_reg = se->readReg(src);
  auto dest_reg = se->readReg(dest);
  if (src_reg->symbol_num() == 0 && dest_reg->symbol_num() == 0) {
    uint32_t src_c = se->getRegisterConcreteValue(src);
    uint32_t dest_c = se->getRegisterConcreteValue(dest);

    auto src_v = std::make_shared<BitVector>(ValueType::CONCRETE, src_c);
    auto dest_v = std::make_shared<BitVector>(ValueType::CONCRETE, dest_c);

    se->writeReg(src, src_v);
    se->writeReg(dest, dest_v);

  } else {
    src_reg = buildop2(BVOper::bvadd, src_reg, 4);
    dest_reg = buildop2(BVOper::bvadd, dest_reg, 4);
    se->writeReg(src, src_reg);
    se->writeReg(dest, dest_reg);
  }

  auto ecx_v = se->readReg("ecx");
  if (ecx_v->symbol_num() == 0) {
    uint32_t ecx_c = se->getRegisterConcreteValue("ecx");
    auto ecx_v = std::make_shared<BitVector>(ValueType::CONCRETE, ecx_c);
    se->writeReg("ecx", ecx_v);
  }

  auto src_mem_address = this->read_reg_data(src);
  auto dest_mem_address = this->read_reg_data(dest);
  std::stringstream src_ss, dest_ss;
  src_ss << std::hex << src_mem_address << std::dec;
  dest_ss << std::hex << dest_mem_address << std::dec;

  auto src_value = se->readMem(src_ss.str(), op1->bit);
  se->writeMem(dest_ss.str(), op0->bit, src_value);

  return true;
}

// ref: https://c9x.me/x86/html/file_module_x86_id_210.html
bool INST_X86_INS_MUL::symbolic_execution(tana::SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  assert(op0->type == Operand::Reg || op0->type == Operand::Mem);

  if (op0->bit == 32) {
    std::shared_ptr<BitVector> eax_v, edx_v, src_v;

    eax_v = se->readReg("eax");
    if (op0->type == Operand::Reg) {
      src_v = se->readReg(op0->field[0]);
    }

    if (op0->type == Operand::Mem) {
      src_v = se->readMem(this->get_memory_address(), op0->bit);
    }

    if (eax_v->symbol_num() == 0 && src_v->symbol_num() == 0 &&
        (!this->is_static)) {
      uint32_t eax_c = se->getRegisterConcreteValue("eax");
      uint32_t edx_c = se->getRegisterConcreteValue("edx");
      eax_v = std::make_shared<BitVector>(ValueType::CONCRETE, eax_c);
      edx_v = std::make_shared<BitVector>(ValueType::CONCRETE, edx_c);

      se->writeReg("edx", edx_v);
      se->writeReg("eax", eax_v);
      return true;
    }

    auto high = buildop2(BVOper::bvmul32_h, eax_v, src_v);
    auto low = buildop2(BVOper::bvmul32_l, eax_v, src_v);

    se->writeReg("edx", high);
    se->writeReg("eax", low);
    return true;
  }

  if (op0->bit == 16) {
    std::shared_ptr<BitVector> ax_v, dx_v, src_v;
    ax_v = se->readReg("ax");

    if (op0->type == Operand::Reg) {
      src_v = se->readReg(op0->field[0]);
    }

    if (op0->type == Operand::Mem) {
      src_v = se->readMem(this->get_memory_address(), op0->bit);
    }

    auto res = buildop2(BVOper::bvmul, ax_v, src_v);
    res->high_bit = 32;
    auto high = se->Extract(res, 17, 32);
    auto low = se->Extract(res, 1, 16);

    se->writeReg("dx", high);
    se->writeReg("ax", low);
    return true;
  }

  if (op0->bit == 8) {
    std::shared_ptr<BitVector> ax_v, al_v, src_v;
    al_v = se->readReg("al");
    if (op0->type == Operand::Reg) {
      src_v = se->readReg(op0->field[0]);
    }

    if (op0->type == Operand::Mem) {
      src_v = se->readMem(this->get_memory_address(), op0->bit);
    }

    auto res = buildop2(BVOper::bvmul, al_v, src_v);
    res->high_bit = 16;

    se->writeReg("ax", res);
    return true;
  }

  return false;
}

bool INST_X86_INS_CLD::symbolic_execution(tana::SEEngine *se) { return true; }

bool INST_X86_INS_CPUID::symbolic_execution(tana::SEEngine *se) {

  uint32_t eax_c = se->getRegisterConcreteValue("eax");
  uint32_t ebx_c = se->getRegisterConcreteValue("ebx");
  uint32_t ecx_c = se->getRegisterConcreteValue("ecx");
  uint32_t edx_c = se->getRegisterConcreteValue("edx");

  auto eax_v = std::make_shared<BitVector>(ValueType::CONCRETE, eax_c);
  auto ebx_v = std::make_shared<BitVector>(ValueType::CONCRETE, ebx_c);
  auto ecx_v = std::make_shared<BitVector>(ValueType::CONCRETE, ecx_c);
  auto edx_v = std::make_shared<BitVector>(ValueType::CONCRETE, edx_c);

  se->writeReg("eax", eax_v);
  se->writeReg("ebx", ebx_v);
  se->writeReg("ecx", ecx_v);
  se->writeReg("edx", edx_v);
  return true;
}

bool INST_X86_INS_CMOVNZ::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  if (this->vcpu.ZF())
    return true;

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

bool INST_X86_INS_CMOVNB::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  if (this->vcpu.CF())
    return true;

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

bool INST_X86_INS_CDQ::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  auto eax_v = se->readReg("eax");
  if (eax_v->symbol_num() == 0) {
    auto edx_c = se->getRegisterConcreteValue("edx");
    auto edx_v = std::make_shared<BitVector>(ValueType::CONCRETE, edx_c);
    se->writeReg("edx", edx_v);
    return true;
  }

  return true;
}

bool INST_X86_INS_CMOVBE::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  if (this->vcpu.ZF() == 0 && this->vcpu.CF() == 0)
    return true;

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  assert(op0->type == Operand::Reg);

  v0 = se->readReg(op0->field[0]);

  if (op1->type == Operand::Reg) {
    v1 = se->readReg(op1->field[0]);
  } else if (op1->type == Operand::Mem) {
    v1 = se->readMem(this->get_memory_address(), op1->bit);
  } else {
    ERROR("CMOVBE operand one error");
  }

  if (v1->symbol_num() == 0) {
    uint32_t concrete_v1 = se->getRegisterConcreteValue(op0->field[0]);
    v1 = std::make_shared<BitVector>(ValueType::CONCRETE, concrete_v1);
  }

  se->writeReg(op0->field[0], v1);

  return true;
}

bool INST_X86_INS_SYSENTER::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  uint32_t eax_c = se->getRegisterConcreteValue("eax");
  auto eax_v = std::make_shared<BitVector>(ValueType::CONCRETE, eax_c);
  se->writeReg("eax", eax_v);

  return true;
}

bool INST_X86_INS_JECXZ::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  auto ecx_v = se->readReg("ecx");
  if (ecx_v->symbol_num() == 0)
    return true;

  auto constrains =
      std::make_shared<Constrain>(this->id, ecx_v, BVOper::equal, 0);
  se->updateCFConstrains(constrains);

  return true;
}

bool INST_X86_INS_CMPXCHG::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  // TODO

  return true;
}

bool INST_X86_INS_SETB::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  if (!this->vcpu.eflags_state)
    return false;

  uint32_t con;

  if (this->vcpu.CF() == 1) {
    con = 1;
  } else {
    con = 0;
  }

  auto con_v = std::make_shared<BitVector>(ValueType::CONCRETE, con);
  auto op0 = this->oprd[0];
  con_v->high_bit = op0->bit;

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), op0->bit, con_v);
    return true;
  }

  if (op0->type == Operand::Reg) {
    con_v->high_bit = op0->bit;
    se->writeReg(op0->field[0], con_v);
    return true;
  }
  return false;
}

bool INST_X86_INS_JP::symbolic_execution(tana::SEEngine *se) { return true; }

bool INST_X86_INS_MOVSW::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  auto op0 = this->oprd[0];
  auto op1 = this->oprd[1];

  assert(op0->type == Operand::Mem);
  assert(op1->type == Operand::Mem);

  auto op0_reg = op0->field[0];
  auto op1_reg = op1->field[0];

  uint32_t op0_c = se->getRegisterConcreteValue(op0_reg);
  uint32_t op1_c = se->getRegisterConcreteValue(op1_reg);

  auto op0_v = std::make_shared<BitVector>(ValueType::CONCRETE, op0_c);
  auto op1_v = std::make_shared<BitVector>(ValueType::CONCRETE, op1_c);

  se->writeReg(op0_reg, op0_v);
  se->writeReg(op1_reg, op1_v);

  std::stringstream addr0, addr1;

  addr0 << std::hex << op0_c << std::dec;
  addr1 << std::hex << op1_c << std::dec;

  auto read_v = se->readMem(addr1.str(), op1->bit);
  se->writeMem(addr0.str(), op0->bit, read_v);

  return true;
}

bool INST_X86_INS_BSWAP::symbolic_execution(tana::SEEngine *se) {
  auto op0 = this->oprd[0];
  assert(op0->type == Operand::Reg);

  auto regName = op0->field[0];

  auto regV = se->readReg(regName);

  if (regV->symbol_num() == 0) {
    uint32_t regC = se->getRegisterConcreteValue(regName);
    auto regValue = std::make_shared<BitVector>(ValueType::CONCRETE, regC);
    se->writeReg(regName, regValue);
    return true;
  }

  auto part0 = se->Extract(regV, 1, 8);
  auto part1 = se->Extract(regV, 9, 16);
  auto part2 = se->Extract(regV, 17, 24);
  auto part3 = se->Extract(regV, 25, 32);

  auto res = se->Concat(se->Concat(part0, part1, part2), part3);
  se->writeReg(regName, res);
  return true;
}

// setnbe: set byte if CF = 0 and ZF = 0
bool INST_X86_INS_SETNBE::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  auto CF = se->getFlags("CF");
  auto notCF = buildop1(BVOper::bvbitnot, CF);

  auto ZF = se->getFlags("ZF");
  auto notZF = buildop1(BVOper::bvbitnot, ZF);

  auto notZFandnotCF = buildop2(BVOper::bvand, notZF, notCF);
  notZFandnotCF->high_bit = T_BYTE_SIZE;

  auto op0 = this->oprd[0];

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], notZFandnotCF);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, notZFandnotCF);
  }

  return true;
}

// setnle: set byte if ZF = 0 and SF=OF
bool INST_X86_INS_SETNLE::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static) {
    return true;
  }

  auto ZF = se->getFlags("ZF");
  auto notZF = buildop1(BVOper::bvbitnot, ZF);

  auto SF = se->getFlags("SF");
  auto OF = se->getFlags("OF");

  auto SFequalOF = buildop2(BVOper::equal, SF, OF);

  auto notZFandSFequalOF = buildop2(BVOper::bvand, notZF, SFequalOF);
  notZFandSFequalOF->high_bit = T_BYTE_SIZE;

  auto op0 = this->oprd[0];

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], notZFandSFequalOF);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, notZFandSFequalOF);
  }

  return true;
}

// setbe: set byte if CF = 1 or ZF = 1
bool INST_X86_INS_SETBE::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  auto CF = se->getFlags("CF");
  auto ZF = se->getFlags("ZF");

  auto CForZF = buildop2(BVOper::bvor, CF, ZF);
  CForZF->high_bit = T_BYTE_SIZE;
  auto op0 = this->oprd[0];

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], CForZF);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, CForZF);
  }

  return true;
}

// Set the byte SF != OF
bool INST_X86_INS_SETL::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;
  std::shared_ptr<Operand> op0 = this->oprd[0];

  auto SF = se->getFlags("SF");
  auto OF = se->getFlags("OF");

  auto SFnonequalOF = buildop2(BVOper::noequal, SF, OF);

  SFnonequalOF->high_bit = T_BYTE_SIZE;

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], SFnonequalOF);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, SFnonequalOF);
  }

  return true;
}

// ZF = 1 SF != OF
bool INST_X86_INS_SETLE::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  std::shared_ptr<Operand> op0 = this->oprd[0];

  auto ZF = se->getFlags("ZF");

  auto SF = se->getFlags("SF");
  auto OF = se->getFlags("OF");

  auto SFnonequalOF = buildop2(BVOper::noequal, SF, OF);
  auto ZForSFnonequalOF = buildop2(BVOper::bvor, ZF, SFnonequalOF);
  ZForSFnonequalOF->high_bit = T_BYTE_SIZE;
  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], ZForSFnonequalOF);
  }

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, ZForSFnonequalOF);
  }

  return true;
}

bool INST_X86_INS_LODSD::symbolic_execution(tana::SEEngine *se) {
  auto op0 = this->oprd[0];
  assert(op0->type == Operand::Mem);

  auto v_mem = se->readMem(this->get_memory_address(), op0->bit);

  se->writeReg("eax", v_mem);

  uint32_t esi = se->getRegisterConcreteValue("esi");

  auto esi_v = std::make_shared<BitVector>(ValueType::CONCRETE, esi);

  se->writeReg("esi", esi_v);

  return true;
}

std::map<std::string, int> INST_X86_INS_SSE::sse_map;

bool INST_X86_INS_SSE::symbolic_execution(tana::SEEngine *se) {

  std::string inst_name = x86::insn_id2string(this->instruction_id);
  if (sse_map.find(inst_name) == sse_map.end()) {
    sse_map[inst_name] = 1;
    return true;
  } else {
    sse_map[inst_name] = sse_map[inst_name] + 1;
    return true;
  }
}

// Mov if CF = 0 and ZF = 0
bool INST_X86_INS_CMOVNBE::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static)
    return true;

  if (this->vcpu.CF() || this->vcpu.ZF())
    return true;

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

bool INST_X86_INS_BSF::symbolic_execution(tana::SEEngine *se) {
  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> bv1;
  if (op1->type == Operand::Mem) {
    bv1 = se->readMem(this->get_memory_address(), op1->bit);
  }
  if (op1->type == Operand::Reg) {
    bv1 = se->readReg(op1->field[0]);
  }

  auto res = buildop1(BVOper::bvbsf, bv1);
  se->writeReg(op0->field[0], res);
  return true;
}

bool INST_X86_INS_STOSD::symbolic_execution(tana::SEEngine *se) {

  auto v_reg = se->readReg(oprd[0]->field[0]);
  if (v_reg->symbol_num() == 0) {
    uint32_t reg_concrete = se->getRegisterConcreteValue(oprd[0]->field[0]);
    auto reg_v = std::make_shared<BitVector>(ValueType::CONCRETE, reg_concrete);
    se->writeReg(oprd[0]->field[0], reg_v);
  } else {
    v_reg = buildop2(BVOper::bvadd, v_reg, 4);
    se->writeReg(oprd[0]->field[0], v_reg);
  }

  auto v_eax = se->readReg("eax");
  se->writeMem(this->get_memory_address(), oprd[0]->bit, v_eax);
  return true;
}

bool INST_X86_INS_RDTSC::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static) {
    return true;
  }
  uint32_t eax = se->getRegisterConcreteValue("eax");
  uint32_t edx = se->getRegisterConcreteValue("edx");

  auto eax_v = std::make_shared<BitVector>(ValueType::CONCRETE, eax);
  auto edx_v = std::make_shared<BitVector>(ValueType::CONCRETE, edx);

  se->writeReg("eax", eax_v);
  se->writeReg("edx", edx_v);

  return true;
}

// Sets the byte in the operand to 1 if the Carry Flag is clear

bool INST_X86_INS_SETNB::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static) {
    return true;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];

  auto CF = se->getFlags("CF");

  auto notCF = buildop1(BVOper::bvbitnot, CF);

  // std::cout << "setnb: " << se->debugEval(notCF) << std::endl;
  // std::cout << "ecx before " << std::hex << se->debugEval(se->readReg("ecx"))
  // << std::dec << std::endl;

  notCF->high_bit = T_BYTE_SIZE;
  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], notCF);
  }

  // std::cout << "ecx after  " << std::hex <<se->debugEval(se->readReg("ecx"))
  // << std::dec << std::endl;

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, notCF);
  }

  return true;
}

bool INST_X86_INS_CMOVS::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static) {
    return true;
  }

  if (!this->vcpu.SF()) {
    return true;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

// mov if ZF = 1 or SF != OF
bool INST_X86_INS_CMOVLE::symbolic_execution(tana::SEEngine *se) {

  if (this->is_static) {
    return true;
  }

  bool ZF = this->vcpu.ZF();
  bool SF = this->vcpu.SF();
  bool OF = this->vcpu.OF();

  if (!ZF && (SF == OF)) {
    return true;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

// mov SF = OF
bool INST_X86_INS_CMOVNL::symbolic_execution(tana::SEEngine *se) {

  if (this->is_static) {
    return true;
  }

  bool SF = this->vcpu.SF();
  bool OF = this->vcpu.OF();
  if (SF != OF) {
    return true;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

// cmovl SF != OF
bool INST_X86_INS_CMOVL::symbolic_execution(tana::SEEngine *se) {
  if (this->is_static) {
    return true;
  }

  bool SF = this->vcpu.SF();
  bool OF = this->vcpu.OF();
  if (SF == OF) {
    return true;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];
  std::shared_ptr<Operand> op1 = this->oprd[1];
  std::shared_ptr<BitVector> v0, v1, res;

  if (op0->type == Operand::Reg) {
    if (op1->type == Operand::ImmValue) { // mov reg, 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], nullptr, 16);
      v1 = std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                       se->isImmSym(temp_concrete), op0->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Reg) { // mov reg, reg
      v1 = se->readReg(op1->field[0]);
      se->writeReg(op0->field[0], v1);
      return true;
    }
    if (op1->type == Operand::Mem) { // mov reg, dword ptr [ebp+0x1]
      /* 1. Get mem address
      2. check whether the mem address has been accessed
      3. if not, create a new value
      4. else load the value in that m_memory
      */
      v1 = se->readMem(this->get_memory_address(), op1->bit);
      se->writeReg(op0->field[0], v1);
      return true;
    }

    ERROR("op1 is not ImmValue, Reg or Mem");
    return false;
  }
  if (op0->type == Operand::Mem) {
    if (op1->type == Operand::ImmValue) { // mov dword ptr [ebp+0x1], 0x1111
      uint32_t temp_concrete = stoul(op1->field[0], 0, 16);
      se->writeMem(
          this->get_memory_address(), op0->bit,
          std::make_shared<BitVector>(ValueType::CONCRETE, temp_concrete,
                                      se->isImmSym(temp_concrete), op0->bit));
      return true;
    } else if (op1->type == Operand::Reg) { // mov dword ptr [ebp+0x1], reg
      // m_memory[it->memory_address] = m_ctx[getRegName(op1->field[0])];
      v1 = se->readReg(op1->field[0]);
      se->writeMem(this->get_memory_address(), op1->bit, v1);
      return true;
    }
  }
  ERROR("Error: The first operand in MOV is not Reg or Mem!");
  return false;
}

// setnl set the byte if SF=OF
bool INST_X86_INS_SETNL::symbolic_execution(SEEngine *se) {
  if (this->is_static) {
    return true;
  }

  std::shared_ptr<Operand> op0 = this->oprd[0];

  auto SF = se->getFlags("SF");
  auto OF = se->getFlags("OF");

  auto SFequalOF = buildop2(BVOper::equal, SF, OF);
  SFequalOF->high_bit = T_BYTE_SIZE;

  if (op0->type == Operand::Reg) {
    se->writeReg(op0->field[0], SFequalOF);
  }

  // std::cout << "ecx after  " << std::hex <<se->debugEval(se->readReg("ecx"))
  // << std::dec << std::endl;

  if (op0->type == Operand::Mem) {
    se->writeMem(this->get_memory_address(), T_BYTE_SIZE, SFequalOF);
  }

  return true;
}

bool INST_X86_INS_IDIV::symbolic_execution(SEEngine *se) {

  if (this->is_static) {
    return true;
  }

  auto eax_v = se->readReg("eax");
  auto edx_v = se->readReg("edx");

  std::shared_ptr<BitVector> divisor;

  std::shared_ptr<Operand> op0 = this->oprd[0];

  if (op0->type == Operand::Reg) {
    divisor = se->readReg(op0->field[0]);
  }

  if (op0->type == Operand::Mem) {
    divisor = se->readMem(this->get_memory_address(), op0->bit);
  }

  auto quo = buildop3(BVOper::bvidiv32_quo, edx_v, eax_v, divisor);
  auto rem = buildop3(BVOper::bvidiv32_rem, edx_v, eax_v, divisor);

  se->writeReg("eax", quo);
  se->writeReg("edx", rem);
  return true;
}

bool INST_X86_INS_PUSHAL::symbolic_execution(tana::SEEngine *se) {
  // TODO
  return true;
}

} // namespace tana