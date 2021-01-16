

#pragma once

#include "BitVector.hpp"
#include "Function.hpp"
#include "ins_types.hpp"

namespace tana {

class Inst_Factory {
public:
  static std::unique_ptr<Inst_Base>
  makeInst(x86::x86_insn id, bool isStatic,
           const std::shared_ptr<DynamicFunction> &func, uint32_t addr);

  static std::unique_ptr<Inst_Base> makeInst(x86::x86_insn id, bool isStatic);

  static std::unique_ptr<Inst_Base>
  makeRepInst(x86::x86_insn id, bool isStatic,
              const std::shared_ptr<DynamicFunction> &fun, uint32_t addr);
};

namespace inst_dyn_details {
std::shared_ptr<BitVector> two_operand(SEEngine *se, Inst_Base *inst, BVOper);
}

class UPDATE_SYMBOLS : public Inst_Base {
private:
  std::vector<std::tuple<uint32_t, uint32_t>> m_key_symbol;
  std::vector<uint8_t> m_key_value;

public:
  UPDATE_SYMBOLS(std::vector<std::tuple<uint32_t, uint32_t>> &key_symbol,
                 std::vector<uint8_t> &key_value)
      : Inst_Base(false) {
    m_key_symbol = key_symbol;
    m_key_value = key_value;
    this->is_updateSecret = true;
  }

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_NOP : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_PUSH : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_POP : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_NEG : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_NOT : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_INC : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_DEC : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MOVSX : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MOVZX : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVB : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MOV : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) override;
};

class INST_X86_INS_LEA : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_XCHG : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SBB : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_IMUL : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SHLD : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SHRD : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_ADC : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

// two operands

class INST_X86_INS_ADD : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SUB : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_AND : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_ROR : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_ROL : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_OR : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_XOR : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SHL : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SHR : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SAR : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_RET : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CALL : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_LEAVE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_ENTER : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_DIV : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_TEST : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMP : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JMP : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JA : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JAE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JB : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JBE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JC : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JG : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JGE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JL : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JLE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNA : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNAE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNB : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNBE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNC : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNG : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNGE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNL : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNLE : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNO : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNS : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JNZ : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JO : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JS : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JZ : public Inst_Base {
public:
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_REP_STOSD : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_STOSD : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_REP_STOSB : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_STOSB : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVZ : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETZ : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETNZ : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_BT : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MOVSB : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MUL : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MOVSD : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CLD : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVNS : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CPUID : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVNB : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CDQ : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVBE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SYSENTER : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JECXZ : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMPXCHG : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETB : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_JP : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_MOVSW : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_BSWAP : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVNZ : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVNBE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETNBE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_LODSD : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETBE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_BSF : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SSE : public Inst_Base {
public:
  static std::map<std::string, int> sse_map;

  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETLE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_RDTSC : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETNB : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVS : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETNLE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVLE : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVNL : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_CMOVL : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETNL : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_IDIV : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_SETL : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

class INST_X86_INS_PUSHAL : public Inst_Base {
  using Inst_Base::Inst_Base;

  bool symbolic_execution(SEEngine *se) final;
};

} // namespace tana