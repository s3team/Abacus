

#include "Tainter.hpp"
#include "ins_types.hpp"
#include <cassert>

namespace tana {

std::list<tana_type::T_ADDRESS> Tainter::getTaintedAddress() {
  return taintedAdress;
}

Tainter::Tainter(tana_type::T_ADDRESS addr, tana_type::T_SIZE m_size) {
  // taintedAdress.empty();
  for (int i = 0; i < GPR_NUM; ++i) {
    for (int j = 0; j < REGISTER_SIZE; ++j)
      taintedRegisters[i][j] = false;
  }
  taint(addr, m_size);
}

bool Tainter::isTainted(tana_type::T_ADDRESS addr) {
  auto found = std::find(taintedAdress.begin(), taintedAdress.end(), addr);

  return !(found == taintedAdress.end());
}

bool Tainter::isTainted(x86::x86_reg reg) {
  tana_type::RegPart reg_size = Registers::getRegSize(reg);
  uint32_t reg_index = Registers::getRegIndex(reg);
  for (uint32_t i = 0; i < reg_size; ++i) {
    if (taintedRegisters[reg_index][i]) {
      return true;
    }
  }
  return false;
}

void Tainter::taint(tana_type::T_ADDRESS addr) {
  if (!isTainted(addr))
    taintedAdress.push_back(addr);
}

void Tainter::taint(x86::x86_reg reg) {
  tana_type::RegPart reg_size = Registers::getRegSize(reg);
  uint32_t reg_index = Registers::getRegIndex(reg);
  for (uint32_t i = 0; i < reg_size; ++i) {
    taintedRegisters[reg_index][i] = true;
  }
}

void Tainter::taint(tana_type::T_ADDRESS addr, tana_type::T_SIZE size) {
  for (tana_type::T_ADDRESS i = 0; i < size; ++i)
    taint(addr + i);
}

void Tainter::untaint(tana_type::T_ADDRESS addr) {
  if (isTainted(addr))
    taintedAdress.remove(addr);
}

void Tainter::untaint(x86::x86_reg reg) {
  tana_type::RegPart reg_size = Registers::getRegSize(reg);
  uint32_t reg_index = Registers::getRegIndex(reg);
  for (uint32_t i = 0; i < reg_size; ++i) {
    taintedRegisters[reg_index][i] = false;
  }
}

void Tainter::untaint(tana_type::T_ADDRESS addr, tana_type::T_SIZE size) {
  for (tana_type::T_ADDRESS i = addr; i < size; ++i)
    taintedAdress.remove(addr + i);
}

// Immediate to Memory
void Tainter::spreadTaintImediate2Memory(tana_type::T_ADDRESS addr,
                                         tana_type::T_SIZE m_size) {
  for (tana_type::T_ADDRESS i = addr; i < addr + m_size; ++i)
    untaint(i);
}

// Immediate to Register
void Tainter::spreadTaintImediate2Register(x86::x86_reg reg) { untaint(reg); }

// Register to Register
void Tainter::spreadTaintRegister2Register(x86::x86_reg dest,
                                           x86::x86_reg src) {
  tana_type::RegPart src_size = Registers::getRegSize(src);
  tana_type::RegPart dest_size = Registers::getRegSize(dest);
  uint32_t src_index = Registers::getRegIndex(src);
  uint32_t dest_index = Registers::getRegIndex(dest);
  for (uint32_t i = 0; (i < src_size) && (i < dest_size); ++i) {
    taintedRegisters[dest_index][i] = taintedRegisters[src_index][i];
  }
}

// Memory to m_memory
void Tainter::spreadTaintMemory2Memory(tana_type::T_ADDRESS ip_addr,
                                       tana_type::T_ADDRESS src,
                                       tana_type::T_ADDRESS dest,
                                       tana_type::T_SIZE m_size) {
  for (uint32_t i = 0; i < m_size; ++i) {
    if (isTainted(tana_type::T_ADDRESS(src + i))) {
      taint(tana_type::T_ADDRESS(dest + i));
      taint(ip_addr);
    } else {
      untaint(tana_type::T_ADDRESS(src + i));
    }
  }
}

// Register to m_memory
void Tainter::spreadTaintRegister2Memory(tana_type::T_ADDRESS ip_addr,
                                         x86::x86_reg reg,
                                         tana_type::T_ADDRESS addr) {
  tana_type::RegPart reg_size = Registers::getRegSize(reg);
  uint32_t reg_index = Registers::getRegIndex(reg);
  for (uint32_t i = 0; i < reg_size; ++i) {
    if (taintedRegisters[reg_index][i]) {

      // cout << "reg index: " << reg_index << " i: " << i << endl;
      taint((tana_type::T_ADDRESS)(addr + i / T_BYTE_SIZE));
      taint(ip_addr);
    } else {
      untaint((tana_type::T_ADDRESS)(i / T_BYTE_SIZE + addr));
    }
  }
}

// Memory to register
void Tainter::spreadTaintMemory2Register(tana_type::T_ADDRESS ip_addr,
                                         tana_type::T_ADDRESS addr,
                                         x86::x86_reg reg) {
  tana_type::RegPart reg_size = Registers::getRegSize(reg);
  uint32_t mem_size = reg_size / T_BYTE_SIZE;
  for (tana_type::T_SIZE i = 0; i < mem_size; ++i) {
    if (isTainted(addr + i)) {
      taint(reg);
      taint(ip_addr);
    }
  }
}

void Tainter::taintIns(Inst_Base &ins) {
  using namespace x86;
  auto opcode = ins.instruction_id;
  std::cout << std::hex << taintedAdress.front() << " ID: " << std::dec
            << ins.id << std::endl;
  uint32_t oprnum = ins.get_operand_number();
  switch (opcode) {
  case x86_insn::X86_INS_MOV:
    if (((ins.oprd[0])->type == Operand::Reg) &&
        ((ins.oprd[1])->type == Operand::Reg)) {
      std::cout << "REG REG" << std::endl;
      auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
      spreadTaintRegister2Register(reg0, reg1);
      break;
    }

    if (((ins.oprd[0])->type == Operand::Mem) &&
        ((ins.oprd[1])->type == Operand::Reg)) {
      std::cout << "MEM REG" << std::endl;
      auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
      spreadTaintRegister2Memory(ins.address, reg1, ins.memory_address);

      break;
    }

    if (((ins.oprd[0])->type == Operand::Reg) &&
        ((ins.oprd[1])->type == Operand::Mem)) {
      std::cout << "REG Mem" << std::endl;
      auto reg1 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      spreadTaintMemory2Register(ins.address, ins.memory_address, reg1);
      break;
    }

    if (((ins.oprd[0])->type == Operand::Reg) &&
        ((ins.oprd[1])->type == Operand::ImmValue)) {
      std::cout << "REG IMM" << std::endl;
      auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      spreadTaintImediate2Register(reg0);
      break;
    }

    if (((ins.oprd[0])->type == Operand::Mem) &&
        ((ins.oprd[1])->type == Operand::ImmValue)) {
      std::cout << "MEM IMM" << std::endl;
      spreadTaintImediate2Memory(ins.memory_address, 4);

      break;
    }

    if (((ins.oprd[0])->type == Operand::Mem) &&
        ((ins.oprd[1])->type == Operand::Mem)) {
      std::cout << "x86 has a Bug" << std::endl;
      // exit(0);
    }

    std::cout << "Some Thing Wrong" << std::endl;
    break;

  case x86_insn::X86_INS_ADD:
  case x86_insn::X86_INS_SUB:
    if (((ins.oprd[0])->type == Operand::Reg) &&
        ((ins.oprd[1])->type == Operand::ImmValue)) {
      std::cout << "REG IMM" << std::endl;
      break; // Doing nothing
    }
    if (((ins.oprd[0])->type == Operand::Reg) &&
        ((ins.oprd[1])->type == Operand::Reg)) {
      std::cout << "REG REG" << std::endl;
      auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
      spreadTaintRegister2Register(reg0, reg1);
      break;
    }
    std::cout << "Some thing wrong here" << std::endl;
    break;

  case x86_insn::X86_INS_OR:
  case x86_insn::X86_INS_AND:
  case x86_insn::X86_INS_XOR:
    assert(oprnum == 2);
    if ((ins.oprd[0]->type == Operand::Reg) &&
        (ins.oprd[1]->type == Operand::Reg)) {
      if (ins.oprd[0]->field[0] == ins.oprd[0]->field[1]) {
        auto reg = Registers::convert2RegID((ins.oprd[0]->field[0]));
        untaint(reg);
        break;
      }
      std::cout << "REG REG" << std::endl;
      auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
      spreadTaintRegister2Register(reg0, reg1);
      break;
    }
    std::cout << "Check here" << std::endl;
    break;

  case x86_insn::X86_INS_LEA:
    assert((ins.oprd[0])->type == Operand::Reg);
    assert((ins.oprd[1])->type == Operand::Mem);
    // TAG 4: eax+0xffffff1
    // TAG 6: eax*2+0xfffff1
    if (((ins.oprd[1])->tag == 4) || ((ins.oprd[1])->tag == 6)) {
      auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
      spreadTaintRegister2Register(reg0, reg1);
    }
    // TAG 5: eax+ebx*2
    // TAG 7: eax+ebx*2+0xfffff1
    if (((ins.oprd[1])->tag == 5) || ((ins.oprd[1])->tag == 7)) {
      auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
      auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
      auto reg2 = Registers::convert2RegID((ins.oprd[1])->field[1]);
      spreadTaintRegister2Register(reg0, reg1);
      spreadTaintRegister2Register(reg0, reg2);
    }

    break;
  case x86_insn::X86_INS_DIV:
    break; // do nothing EAX = EAX / src

  case x86_insn::X86_INS_POP:
    if ((oprnum == 1) && ((ins.oprd[0])->type == Operand::Reg)) {
      auto reg = Registers::convert2RegID((ins.oprd[0]->field[0]));
      spreadTaintRegister2Memory(ins.address, reg, ins.memory_address);
      break;
    }

    std::cout << "pop error" << std::endl;
    break;

  case x86_insn::X86_INS_PUSH:
    if ((oprnum == 1) && ((ins.oprd[0])->type == Operand::Reg)) {
      auto reg = Registers::convert2RegID((ins.oprd[0]->field[0]));
      spreadTaintRegister2Memory(ins.address, reg, ins.memory_address);
      break;
    }
    if ((oprnum == 1) && ((ins.oprd[0])->type == Operand::ImmValue)) {
      spreadTaintImediate2Memory(ins.memory_address, 4); // 32 bit = 4 bytes
      break;
    }
    if ((oprnum == 1) && ((ins.oprd[0])->type == Operand::ImmValue)) {
      auto reg = Registers::convert2RegID("esp");
      uint32_t esp_index = Registers::getRegIndex(reg);
      tana_type::T_ADDRESS esp_value = ins.vcpu.gpr[esp_index];
      tana_type::T_ADDRESS write_addr = esp_value - 4; // 32 bits = 4 bytes
      spreadTaintMemory2Memory(ins.address, ins.memory_address, write_addr, 4);
      break;
    }
    std::cout << "push error" << std::endl;
    break;

  case x86_insn::X86_INS_IMUL:
    if (oprnum == 1) {
      auto reg = Registers::convert2RegID("eax");
      // TODO
      break;
    }

    if ((oprnum == 2) || (oprnum == 3)) {
      if (((ins.oprd[0])->type == Operand::Reg) &&
          ((ins.oprd[1])->type == Operand::Reg)) {
        std::cout << "REG REG" << std::endl;
        auto reg0 = Registers::convert2RegID((ins.oprd[0])->field[0]);
        auto reg1 = Registers::convert2RegID((ins.oprd[1])->field[0]);
        spreadTaintRegister2Register(reg0, reg1);
        break;
      }
      if (((ins.oprd[0])->type == Operand::Reg) &&
          ((ins.oprd[1])->type == Operand::Mem)) {
        std::cout << "REG Mem" << std::endl;
        auto reg1 = Registers::convert2RegID((ins.oprd[0])->field[0]);
        spreadTaintMemory2Register(ins.address, ins.memory_address, reg1);
        break;
      }
    }

  default:
    std::cout << "ERROR at" << __FILE__ << ins.get_opcode_operand() << __LINE__
              << std::endl;
    break;
  }
}
}; // namespace tana
