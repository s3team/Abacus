
#include "ins_types.hpp"
#include "BitVector.hpp"
#include "Register.hpp"
#include "error.hpp"
#include "ins_parser.hpp"
#include "x86.hpp"
#include <memory>
#include <string>

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {

std::string Inst_Base::get_opcode_operand() const {
  std::string instruction_operand;
  instruction_operand += x86::insn_id2string(instruction_id);
  instruction_operand += " ";
  for (const auto &opr : oprs) {
    instruction_operand += opr;
    instruction_operand += ",";
  }

  instruction_operand =
      instruction_operand.substr(0, instruction_operand.size() - 1);

  return instruction_operand;
}

std::ostream &operator<<(std::ostream &os, const Operand &opr) {
  if (opr.type == Operand::Mem) {
    os << "Mem: ";
    switch (opr.tag) {
    case 1: {
      // Imm Value
      os << opr.field[0];
      return os;
    }
    case 2: {
      // Register
      os << opr.field[0];
      return os;
    }
    case 3: {
      // eax*2
      os << opr.field[0] << "*" << opr.field[1];
      return os;
    }
    case 4: {
      // eax + Imm
      os << opr.field[0] << opr.field[1] << opr.field[2];
      return os;
    }
    case 5: {
      // eax + ebx*2
      os << opr.field[0] << "+" << opr.field[1] << "*" << opr.field[2];
      return os;
    }
    case 6: {
      // eax*2 + Imm
      os << opr.field[0] << "*" << opr.field[1] << opr.field[2] << opr.field[3];
      return os;
    }
    case 7: {
      // eax + ebx*2 + Imm
      os << opr.field[0] << "+" << opr.field[1] << "*" << opr.field[2]
         << opr.field[3] << opr.field[4];
      return os;
    }
    default:
      ERROR("Wrong Address Operand");
    }
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const Inst_Base &inst) {
  os << inst.id << " ";
  os << std::hex << inst.address << std::dec;
  os << " ";
  os << inst.get_opcode_operand();
  os << " ";
  if (!inst.is_static) {
    auto &v_register = inst.vcpu.gpr;
    os << "eax: " << std::hex << v_register[0] << std::dec << " ";
    os << "ebx: " << std::hex << v_register[1] << std::dec << " ";
    os << "ecx: " << std::hex << v_register[2] << std::dec << " ";
    os << "edx: " << std::hex << v_register[3] << std::dec << " ";
    os << "esi: " << std::hex << v_register[4] << std::dec << " ";
    os << "edi: " << std::hex << v_register[5] << std::dec << " ";
    os << "esp: " << std::hex << v_register[6] << std::dec << " ";
    os << "ebp: " << std::hex << v_register[7] << std::dec << " ";
  }
  if (inst.vcpu.eflags_state) {
    os << "CF: " << inst.vcpu.CF() << " ";
    os << "PF: " << inst.vcpu.PF() << " ";
    os << "AF: " << inst.vcpu.AF() << " ";
    os << "ZF: " << inst.vcpu.ZF() << " ";
    os << "SF: " << inst.vcpu.SF() << " ";
    os << "DF: " << inst.vcpu.DF() << " ";
    os << "OF: " << inst.vcpu.OF() << " ";
  }

  return os;
}

vcpu_ctx::vcpu_ctx() : eflags(0), eflags_state(false) {
  for (int i = 0; i < GPR_NUM; ++i) {
    gpr[i] = 0;
  }
};

bool vcpu_ctx::CF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x1u);
}

bool vcpu_ctx::PF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x4u);
}

bool vcpu_ctx::AF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x10u);
}

bool vcpu_ctx::ZF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x40u);
}

bool vcpu_ctx::SF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x80u);
}

bool vcpu_ctx::TF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x100u);
}

bool vcpu_ctx::DF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x400u);
}

bool vcpu_ctx::OF() const {
  assert(eflags >= 0);
  return static_cast<bool>(eflags & 0x800u);
}

Inst_Base::Inst_Base(bool inst_type)
    : id(0), address(0),
      instruction_id(x86::X86_INS_INVALID), oprd{nullptr, nullptr, nullptr},
      memory_address(0), is_static(inst_type), mem_data(0),
      mem_data_available(false) {}

uint32_t Inst_Base::get_operand_number() const {
  return static_cast<uint32_t>(this->oprs.size());
}

void Inst_Base::parseOperand() {
  if (is_static) {
    for (uint32_t i = 0; i < this->get_operand_number(); ++i) {
      this->oprd[i] = createOperandStatic(this->oprs[i], this->address);
    }
  } else {
    for (uint32_t i = 0; i < this->get_operand_number(); ++i) {
      this->oprd[i] = createOperand(this->oprs[i], this->address);
    }
  }
}

std::string Inst_Base::get_memory_address() {
  if (!is_static) {
    std::stringstream sstream;
    sstream << std::hex << memory_address << std::dec;
    std::string result = sstream.str();
    return result;
  } else {
    std::string reg_name;
    uint32_t num_opd = this->get_operand_number();
    for (uint32_t count = 0; count < num_opd; ++count) {
      reg_name = oprs[count];
      if (this->oprd[count]->type == Operand::Mem)
        return oprs[count];
    }
    return reg_name;
  }
}

void Inst_Base::set_mem_data(uint32_t data) {
  mem_data_available = true;
  this->mem_data = data;
}

uint32_t Inst_Base::read_mem_data() const {
  if (!mem_data_available) {
    ERROR("m_memory data not available");
    return 0;
  }

  if (this->instruction_id == x86::X86_INS_POP ||
      this->instruction_id == x86::X86_INS_PUSH ||
      this->instruction_id == x86::X86_INS_LEAVE)
    return mem_data;
  uint32_t mem_size = 0;
  for (int i = 0; i < 3; ++i) {
    if (oprd[i] == nullptr)
      continue;
    if (oprd[i]->type == Operand::Mem) {
      mem_size = oprd[i]->bit;
    }
  }

  switch (mem_size) {
  case 32:
  case 16:
  case 8:
    return mem_data;
  default:
    return 0;
  }
}

uint32_t Inst_Base::read_reg_data(std::string RegName) const {
  if (is_static) {
    ERROR("Regsiter data is not available");
    return 0;
  }
  x86::x86_reg regid = Registers::convert2RegID(RegName);
  RegType regType = Registers::getRegType(regid);
  uint32_t regIndex = Registers::getRegIndex(regid);
  uint32_t raw_reg_data = this->vcpu.gpr[regIndex];
  switch (regType) {
  case FULL:
    return raw_reg_data;
  case HALF:
    return BitVector::extract(raw_reg_data, 16, 1);
  case QLOW:
    return BitVector::extract(raw_reg_data, 8, 1);
  case QHIGH:
    return BitVector::extract(raw_reg_data, 16, 9);
  case INVALIDREG:
    ERROR("Invalid Register Type");
  }
  ERROR("Invalid Register Type");
  return 0;
}

} // namespace tana