

#include "Register.hpp"
#include "error.hpp"

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {
using namespace x86;

std::string Registers::getRegName(x86_reg reg) { return reg_id2string(reg); }

uint32_t Registers::getRegSize(x86_reg name) { return x86::get_reg_size(name); }

uint32_t Registers::getRegSize(const std::string &name) {
  auto reg = convert2RegID(name);
  return getRegSize(reg);
}

x86_reg Registers::convert2RegID(const std::string &reg_name) {
  return reg_string2id(reg_name);
}

void Registers::printTaintedRegs(
    bool taintedRegisters[GPR_NUM][REGISTER_SIZE]) {
  for (int i = 0; i < GPR_NUM; ++i)
    for (int j = 0; j < REGISTER_SIZE; ++j)
      if (taintedRegisters[i][j])
        std::cout << getRegName((x86_reg)i) << std::endl;
}

uint32_t Registers::getRegIndex(x86_reg name) {
  switch (name) {
  case X86_REG_EAX:
  case X86_REG_AX:
  case X86_REG_AH:
  case X86_REG_AL:
    return 0;

  case X86_REG_EBX:
  case X86_REG_BX:
  case X86_REG_BH:
  case X86_REG_BL:
    return 1;

  case X86_REG_ECX:
  case X86_REG_CX:
  case X86_REG_CH:
  case X86_REG_CL:
    return 2;

  case X86_REG_EDX:
  case X86_REG_DX:
  case X86_REG_DH:
  case X86_REG_DL:
    return 3;

  case X86_REG_ESI:
  case X86_REG_SI:
    return 4;

  case X86_REG_EDI:
  case X86_REG_DI:
    return 5;

  case X86_REG_ESP:
  case X86_REG_SP:
    return 6;

  case X86_REG_EBP:
  case X86_REG_BP:
    return 7;

  default:
    ERROR("Error getRegIndex:");
    return 8;
  }
}

std::string Registers::convertRegID2RegName(uint32_t id) {
  switch (id) {
  case 0:
    return "eax";
  case 1:
    return "ebx";
  case 2:
    return "ecx";
  case 3:
    return "edx";
  case 4:
    return "esi";
  case 5:
    return "edi";
  case 6:
    return "esp";
  case 7:
    return "ebp";
  default:
    ERROR("Error convertRegID2RegName:");
    return "null";
  }
}

RegType Registers::getRegType(x86_reg name) {
  switch (name) {
  case X86_REG_EAX:
  case X86_REG_EBX:
  case X86_REG_ECX:
  case X86_REG_EDX:
  case X86_REG_ESI:
  case X86_REG_EDI:
  case X86_REG_ESP:
  case X86_REG_EBP:
    return FULL;

  case X86_REG_AX:
  case X86_REG_BX:
  case X86_REG_CX:
  case X86_REG_DX:
  case X86_REG_SI:
  case X86_REG_DI:
  case X86_REG_BP:
  case X86_REG_SP:
    return HALF;

  case X86_REG_AH:
  case X86_REG_BH:
  case X86_REG_CH:
  case X86_REG_DH:
    return QHIGH;

  case X86_REG_AL:
  case X86_REG_BL:
  case X86_REG_CL:
  case X86_REG_DL:
    return QLOW;

  default:
    ERROR("Error getRegType: ");
    return INVALIDREG;
  }
}

RegType Registers::getRegType(const std::string &name) {

  auto reg = convert2RegID(name);
  return getRegType(reg);
}

} // namespace tana