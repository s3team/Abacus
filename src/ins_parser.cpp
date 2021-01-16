
#include "ins_parser.hpp"
#include "LibcModel.hpp"
#include "ins_types.hpp"
#include <nlohmann/json.hpp>
#include <regex>

using namespace std;

namespace tana {

namespace count_num {
uint64_t FileRead(std::istream &is, std::vector<char> &buff) {
  is.read(&buff[0], buff.size());
  return is.gcount();
}

uint64_t CountLines(const std::vector<char> &buff, int sz) {
  int newlines = 0;
  const char *p = &buff[0];
  for (int i = 0; i < sz; i++) {
    if (p[i] == '\n') {
      newlines++;
    }
  }
  return newlines;
}
} // namespace count_num

uint64_t file_inst_num(std::ifstream &trace_file) {
  uint64_t total_line = 0;
  const int SZ = 1024 * 1024;
  std::vector<char> buff(SZ);

  while (int cc = count_num::FileRead(trace_file, buff)) {
    total_line += count_num::CountLines(buff, cc);
  }

  return total_line;
}

std::shared_ptr<Operand> createAddrOperandStatic(const std::string &s) {
  static std::regex addr1("0x[[:xdigit:]]+");
  static std::regex addr2("eax|ebx|ecx|edx|esi|edi|esp|ebp");
  static std::regex addr3("(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\*([[:digit:]])");

  static std::regex addr4(
      "(eax|ebx|ecx|edx|esi|edi|esp|ebp)(\\+|-)(0x[[:xdigit:]]+)");
  static std::regex addr5("(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\+(eax|ebx|ecx|"
                          "edx|esi|edi|esp|ebp)\\*([[:digit:]])");
  static std::regex addr6("(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\*([[:digit:]])("
                          "\\+|-)(0x[[:xdigit:]]+)");

  static std::regex addr7(
      "(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\+(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\*"
      "([[:digit:]])(\\+|-)(0x[[:xdigit:]]+)");

  static std::regex addr8("arg(.*)");
  static std::regex addr9("local(.*)");
  static std::regex addr10("var(.*)");
  static std::regex addr11("obj(.*)");

  std::shared_ptr<Operand> opr = std::make_shared<Operand>();
  std::smatch m;

  // pay attention to the matching order: long sequence should be matched first,
  // then the subsequence.
  if (regex_search(s, m, addr8)) { // addr8: arg_8h
    opr->type = Operand::Mem;
    opr->tag = 8;
    opr->field[0] = s;
    return opr;
  }

  if (regex_search(s, m, addr9)) { // addr9: local_1ch
    opr->type = Operand::Mem;
    opr->tag = 9;
    opr->field[0] = s;
    return opr;
  }

  if (regex_search(s, m, addr10)) { // addr10: var_1ch
    opr->type = Operand::Mem;
    opr->tag = 10;
    opr->field[0] = s;
    return opr;
  }

  if (regex_search(s, m, addr11)) { // addr11: obj_1ch
    opr->type = Operand::Mem;
    opr->tag = 11;
    opr->field[0] = s;
    return opr;
  }

  if (regex_search(s, m, addr7)) { // addr7: eax+ebx*2+0xfffff1
    opr->type = Operand::Mem;
    opr->tag = 7;
    opr->field[0] = m[1]; // eax
    opr->field[1] = m[2]; // ebx
    opr->field[2] = m[3]; // 2
    opr->field[3] = m[4]; // +
    opr->field[4] = m[5]; // 0xfffff1
    return opr;
  }
  if (regex_search(s, m, addr4)) { // addr4: eax+0xfffff1
    // cout << "addr 4: " << s << endl;
    opr->type = Operand::Mem;
    opr->tag = 4;
    opr->field[0] = m[1];
    opr->field[1] = m[2];
    opr->field[2] = m[3];
    return opr;
  }
  if (regex_search(s, m, addr5)) { // addr5: eax+ebx*2
    opr->type = Operand::Mem;
    opr->tag = 5;
    opr->field[0] = m[1]; // eax
    opr->field[1] = m[2]; // ebx
    opr->field[2] = m[3]; // 2
    return opr;
  }
  if (regex_search(s, m, addr6)) { // addr6: eax*2+0xfffff1
    opr->type = Operand::Mem;
    opr->tag = 6;
    opr->field[0] = m[1]; // eax
    opr->field[1] = m[2]; // 2
    opr->field[2] = m[3]; // +
    opr->field[3] = m[4]; // 0xfffff1
    return opr;
  }
  if (regex_search(s, m, addr3)) { // addr3: eax*2
    opr->type = Operand::Mem;
    opr->tag = 3;
    opr->field[0] = m[1];
    opr->field[1] = m[2];
    return opr;
  }
  if (regex_search(s, m, addr1)) { // addr1: Immdiate value address
    opr->type = Operand::Mem;
    opr->tag = 1;
    opr->field[0] = m[0];
    return opr;
  }
  if (regex_search(s, m, addr2)) { // addr2: 32 bit register address
    // cout << "addr 2: " << s << endl;
    opr->type = Operand::Mem;
    opr->tag = 2;
    opr->field[0] = m[0];
    return opr;
  }

  opr->type = Operand::Mem;
  opr->tag = 12;
  opr->field[0] = s;
  return opr;
}

std::shared_ptr<Operand> createAddrOperand(const std::string &s) {
  // regular expressions addresses
  static std::regex addr1("0x[[:xdigit:]]+");
  static std::regex addr2("eax|ebx|ecx|edx|esi|edi|esp|ebp");
  static std::regex addr3("(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\*([[:digit:]])");

  static std::regex addr4(
      "(eax|ebx|ecx|edx|esi|edi|esp|ebp)(\\+|-)(0x[[:xdigit:]]+)");
  static std::regex addr5("(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\+(eax|ebx|ecx|"
                          "edx|esi|edi|esp|ebp)\\*([[:digit:]])");
  static std::regex addr6("(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\*([[:digit:]])("
                          "\\+|-)(0x[[:xdigit:]]+)");

  static std::regex addr7(
      "(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\+(eax|ebx|ecx|edx|esi|edi|esp|ebp)\\*"
      "([[:digit:]])(\\+|-)(0x[[:xdigit:]]+)");

  std::shared_ptr<Operand> opr = make_shared<Operand>();
  std::smatch m;

  // pay attention to the matching order: long sequence should be matched first,
  // then the subsequence.
  if (regex_search(s, m, addr7)) { // addr7: eax+ebx*2+0xfffff1
    opr->type = Operand::Mem;
    opr->tag = 7;
    opr->field[0] = m[1]; // eax
    opr->field[1] = m[2]; // ebx
    opr->field[2] = m[3]; // 2
    opr->field[3] = m[4]; // +
    opr->field[4] = m[5]; // 0xfffff1
    return opr;
  } else if (regex_search(s, m, addr4)) { // addr4: eax+0xfffff1
    // cout << "addr 4: " << s << endl;
    opr->type = Operand::Mem;
    opr->tag = 4;
    opr->field[0] = m[1];
    opr->field[1] = m[2];
    opr->field[2] = m[3];
    return opr;
  } else if (regex_search(s, m, addr5)) { // addr5: eax+ebx*2
    opr->type = Operand::Mem;
    opr->tag = 5;
    opr->field[0] = m[1]; // eax
    opr->field[1] = m[2]; // ebx
    opr->field[2] = m[3]; // 2
    return opr;
  } else if (regex_search(s, m, addr6)) { // addr6: eax*2+0xfffff1
    opr->type = Operand::Mem;
    opr->tag = 6;
    opr->field[0] = m[1]; // eax
    opr->field[1] = m[2]; // 2
    opr->field[2] = m[3]; // +
    opr->field[3] = m[4]; // 0xfffff1
    return opr;
  } else if (regex_search(s, m, addr3)) { // addr3: eax*2
    opr->type = Operand::Mem;
    opr->tag = 3;
    opr->field[0] = m[1];
    opr->field[1] = m[2];
    return opr;
  } else if (regex_search(s, m, addr1)) { // addr1: Immdiate value address
    opr->type = Operand::Mem;
    opr->tag = 1;
    opr->field[0] = m[0];
    return opr;
  } else if (regex_search(s, m, addr2)) { // addr2: 32 bit register address
    // cout << "addr 2: " << s << endl;
    opr->type = Operand::Mem;
    opr->tag = 2;
    opr->field[0] = m[0];
    return opr;
  } else {
    std::cout << "Unknown addr operands: " << s << std::endl;
  }

  return opr;
}

std::shared_ptr<Operand> createDataOperand(const std::string &s,
                                           uint32_t addr) {
  // Regular expressions for Immvalue and Registers
  static std::regex immvalue("0x[[:xdigit:]]+");
  static std::regex immvalue1("[[:xdigit:]]+");

  static std::regex reg8("al|ah|bl|bh|cl|ch|dl|dh");
  static std::regex reg16("ax|bx|cx|dx|si|di|bp|cs|ds|es|fs|gs|ss");
  static std::regex reg32(
      "eax|ebx|ecx|edx|esi|edi|esp|ebp|st0|st1|st2|st3|st4|st5");

  std::shared_ptr<Operand> opr = make_shared<Operand>();
  std::smatch m;
  if (regex_search(s, m, reg32)) { // 32 bit register
    opr->type = Operand::Reg;
    opr->bit = 32;
    opr->field[0] = m[0];
    return opr;
  }
  if (regex_search(s, m, reg16)) { // 16 bit register
    opr->type = Operand::Reg;
    opr->bit = 16;
    opr->field[0] = m[0];
    return opr;
  }
  if (regex_search(s, m, reg8)) { // 8 bit register
    opr->type = Operand::Reg;
    opr->bit = 8;
    opr->field[0] = m[0];
    return opr;
  }
  if (regex_search(s, m, immvalue)) {
    opr->type = Operand::ImmValue;
    opr->bit = 32;
    opr->field[0] = m[0];
    return opr;
  }
  if (regex_search(s, m, immvalue1)) {
    opr->type = Operand::ImmValue;
    opr->bit = 32;
    opr->field[0] = m[0];
    return opr;
  }
  opr->type = Operand::Label;
  opr->bit = 32;
  opr->field[0] = s;
  return opr;
}

std::shared_ptr<Operand> createOperandStatic(const std::string &s,
                                             uint32_t addr) {
  static std::regex ptr("\\[(.*)\\]");
  static std::regex byteptr("byte \\[(.*)\\]");
  static std::regex wordptr("word \\[(.*)\\]");
  static std::regex dwordptr("dword \\[(.*)\\]");
  static std::regex segptr("dword (fs|gs):\\[(.*)\\]");
  static std::smatch m;

  std::shared_ptr<Operand> opr;

  if ((s.find("[") != std::string::npos) &&
      (s.find("]") != std::string::npos)) {
    if (regex_search(s, m, byteptr)) {
      opr = createAddrOperandStatic(m[1]);
      opr->bit = 8;
    } else if (regex_search(s, m, dwordptr)) {
      opr = createAddrOperandStatic(m[1]);
      opr->bit = 32;
    } else if (regex_search(s, m, segptr)) {
      opr = createAddrOperandStatic(m[2]);
      opr->issegaddr = true;
      opr->bit = 32;
      opr->segreg = m[1];
    } else if (regex_search(s, m, wordptr)) {
      opr = createAddrOperandStatic(m[1]);
      opr->bit = 16;
    } else if (regex_search(s, m, ptr)) {
      opr = createAddrOperandStatic(m[1]);
      opr->bit = 0;
    } else {
      std::cout << "Unkown addr: " << s << std::endl;
    }

  } else {
    opr = createDataOperand(s, addr);
  }

  return opr;
}

std::shared_ptr<Operand> createOperand(const std::string &s, uint32_t addr) {
  static std::regex ptr("ptr \\[(.*)\\]");
  static std::regex byteptr("byte ptr \\[(.*)\\]");
  static std::regex wordptr("word ptr \\[(.*)\\]");
  static std::regex dwordptr("dword ptr \\[(.*)\\]");
  static std::regex segptr("dword ptr (fs|gs):\\[(.*)\\]");
  static std::smatch m;

  std::shared_ptr<Operand> opr;

  if (s.find("ptr") != std::string::npos) { // Operand is a mem access addr
    if (regex_search(s, m, byteptr)) {
      opr = createAddrOperand(m[1]);
      opr->bit = 8;
      return opr;

    } else if (regex_search(s, m, dwordptr)) {
      opr = createAddrOperand(m[1]);
      opr->bit = 32;
      return opr;

    } else if (regex_search(s, m, segptr)) {
      opr = createAddrOperand(m[2]);
      opr->issegaddr = true;
      opr->bit = 32;
      opr->segreg = m[1];
      return opr;

    } else if (regex_search(s, m, wordptr)) {
      opr = createAddrOperand(m[1]);
      opr->bit = 16;
      return opr;

    } else if (regex_search(s, m, ptr)) {
      opr = createAddrOperand(m[1]);
      opr->bit = 0;
      return opr;

    } else {
      std::cout << "Unknown addr: " << s << std::endl;
    }
  } else { // Operand is data

    opr = createDataOperand(s, addr);
  }

  return opr;
}

std::unique_ptr<Inst_Base>
parseInst(const std::string &line, bool isStatic,
          const std::shared_ptr<DynamicFunction> &fun) {
  if (!isStatic) {
    std::istringstream strbuf(line);
    std::string temp, disasstr, temp_addr;

    // instruction address
    getline(strbuf, temp_addr, ';');
    auto ins_addrn = std::stoul(temp_addr, nullptr, 16);

    // get dissassemble string
    getline(strbuf, disasstr, ';');
    std::istringstream disasbuf(disasstr);

    std::string opcstr;
    getline(disasbuf, opcstr, ' ');
    auto ins_id = x86::insn_string2id(opcstr);

    std::unique_ptr<Inst_Base> ins = nullptr;

    // Remove prefix
    if (ins_id == x86::X86_INS_REP || ins_id == x86::X86_INS_DATA16) {
      getline(disasbuf, opcstr, ' ');
      ins_id = x86::insn_string2id(opcstr);
      ins = Inst_Factory::makeRepInst(ins_id, false, fun, ins_addrn);
    } else {
      ins = Inst_Factory::makeInst(ins_id, false, fun, ins_addrn);
    }

    ins->address = ins_addrn;
    ins->instruction_id = ins_id;

    while (disasbuf.good()) {
      getline(disasbuf, temp, ',');
      if (temp.find_first_not_of(' ') != std::string::npos)
        ins->oprs.push_back(temp);
    }

    // get 8 register value
    for (int i = 0; i < GPR_NUM; ++i) {
      getline(strbuf, temp, ',');
      ins->vcpu.gpr[i] = std::stoul(temp, nullptr, 16);
    }

    getline(strbuf, temp, ',');
    ins->memory_address = std::stoul(temp, nullptr, 16);

    // Get EPFLAGS
    getline(strbuf, temp, ',');
    if (!temp.empty()) {
      ins->vcpu.set_eflags(std::stoul(temp, nullptr, 16));
      ins->vcpu.eflags_state = true;
    }

    // Get Mem Data
    getline(strbuf, temp, ',');
    if (!temp.empty()) {
      ins->set_mem_data(std::stoul(temp, nullptr, 16));
    }

    ins->parseOperand();

    return std::move(ins);
  }

  std::string line_buf = line;
  std::string delimiter = " ";
  size_t pos = 0;
  std::string str_addr;

  pos = line_buf.find(delimiter);
  str_addr = line_buf.substr(0, pos);

  line_buf.erase(0, pos + delimiter.length());

  auto str_start = line_buf.find_first_not_of(' ');
  auto str_end = line_buf.find_last_not_of(' ');

  line_buf = line_buf.substr(str_start, str_end - str_start + 1);

  std::istringstream disasbuf(line_buf);

  std::string opcstr, temp;
  getline(disasbuf, opcstr, ' ');

  auto inst_instruction_id = x86::insn_string2id(opcstr);
  if (inst_instruction_id == x86::X86_INS_REP ||
      inst_instruction_id == x86::X86_INS_DATA16) {
    getline(disasbuf, opcstr, ' ');
    inst_instruction_id = x86::insn_string2id(opcstr);
  }

  auto inst_addrn = std::stoul(str_addr, nullptr, 16);

  std::unique_ptr<Inst_Base> inst =
      tana::Inst_Factory::makeInst(inst_instruction_id, true);

  inst->instruction_id = inst_instruction_id;
  inst->address = inst_addrn;

  while (disasbuf.good()) {
    getline(disasbuf, temp, ',');
    if (temp.find_first_not_of(' ') != std::string::npos)
      inst->oprs.push_back(temp);
  }

  inst->parseOperand();
  return std::move(inst);
}

bool parse_trace(std::ifstream &trace_file,
                 std::vector<std::unique_ptr<Inst_Base>> &L) {
  uint32_t batch_size = 0xffffffff;
  tana_type::T_ADDRESS addr_taint = 0;
  tana_type::T_SIZE size_taint = 0;
  uint32_t num = 0;
  bool finish_parse = parse_trace(trace_file, L, addr_taint, size_taint, num);
  return finish_parse;
}

bool parse_trace(std::ifstream &trace_file, tana_type::T_ADDRESS &addr_taint,
                 tana_type::T_SIZE &size_taint,
                 std::vector<std::unique_ptr<Inst_Base>> &L) {
  uint32_t batch_size = 1000;
  uint32_t id = 1;
  bool finish_parse = parse_trace(trace_file, L, addr_taint, size_taint, id);
  while (!finish_parse) {
    finish_parse = parse_trace(trace_file, L, addr_taint, size_taint, id);
    id = id + batch_size;
  }
  return finish_parse;
}

bool parse_trace(std::ifstream &trace_file,
                 std::vector<std::unique_ptr<Inst_Base>> &L,
                 tana_type::T_ADDRESS &addr_taint,
                 tana_type::T_SIZE &size_taint, uint32_t num) {
  std::string line;
  uint32_t id_count = 1;
  while (trace_file.good()) {
    getline(trace_file, line);
    if (line.empty()) {
      break;
    }

    if (line.find("Start") != std::string::npos) {
      std::istringstream fun_buf(line);
      std::string start_taint, taint_len, temp_str;
      getline(fun_buf, temp_str, ';');
      getline(fun_buf, start_taint, ';');
      getline(fun_buf, taint_len, ';');
      addr_taint = stoul(start_taint, nullptr, 16);
      size_taint = stoul(taint_len, nullptr, 10);
      getline(trace_file, line);

      return false;
    }

    if (line.find("END") != std::string::npos) {
      return true;
    }

    auto ins_index = num++;
    id_count++;

    std::unique_ptr<Inst_Base> ins = parseInst(line, false, nullptr);
    ins->id = ins_index;

    L.push_back(std::move(ins));
  }
  return !trace_file.good();
}

std::vector<std::unique_ptr<StaticBlock>>
parse_block(std::ifstream &json_file,
            std::vector<std::unique_ptr<Inst_Base>> &fun_inst) {
  nlohmann::json blocks_json = nlohmann::json::array();
  std::vector<std::unique_ptr<StaticBlock>> blocks;
  json_file >> blocks_json;
  int block_id = 0;
  for (auto &element : blocks_json) {
    std::string str_addr, str_end_addr, str_inputs, str_jump, str_ninstr,
        str_outputs, str_size, str_traced;
    str_addr = element["addr"];
    str_end_addr = element["end_addr"];
    str_inputs = element["inputs"];
    // str_jump = element["jump"];
    str_ninstr = element["ninstr"];
    str_outputs = element["outputs"];
    str_size = element["size"];
    str_traced = element["traced"];

    uint32_t block_addr = std::stoul(str_addr, nullptr, 16);
    uint32_t block_end_addr = std::stoul(str_end_addr, nullptr, 16);
    uint32_t block_inputs = std::stoul(str_inputs, nullptr, 16);
    // uint32_t block_jump = std::stoul(str_jump, nullptr, 16);
    uint32_t block_ninstr = std::stoul(str_ninstr, nullptr, 16);
    uint32_t block_outputs = std::stoul(str_outputs, nullptr, 16);
    uint32_t block_size = std::stoul(str_size, nullptr, 16);
    uint32_t block_traced = (str_traced == "true");

    auto block = std::make_unique<StaticBlock>(block_addr, block_end_addr,
                                               block_ninstr, block_size);

    block->id = block_id++;

    blocks.push_back(std::move(block));
  }

  for (auto &block : blocks) {
    bool res = block->init(fun_inst);
    if (!res) {
      // std::cout << "Block Parse Error" << std::endl;
    }
  }

  return std::move(blocks);
}

bool parse_static_trace(std::ifstream &trace_file, std::ifstream &json_file,
                        std::vector<std::unique_ptr<StaticBlock>> &blocks) {

  uint32_t ins_index = 0;

  std::vector<std::unique_ptr<Inst_Base>> fun_inst;

  std::string line;
  while (trace_file.good()) {

    getline(trace_file, line);

    if (line.find("----") != std::string::npos) {
      continue;
    }

    if (line.find("@") != std::string::npos) {
      continue;
    }

    if (line.find("0x") == std::string::npos) {
      continue;
    }

    // Remove comments
    size_t comment_pos = line.find(";");
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }

    char chars[] = "|\\";
    for (unsigned int i = 0; i < strlen(chars); ++i) {
      line.erase(std::remove(line.begin(), line.end(), chars[i]), line.end());
    }

    auto inst = parseInst(line, true, nullptr);
    inst->id = ins_index++;
    fun_inst.push_back(std::move(inst));
  }

  blocks = parse_block(json_file, fun_inst);
  return true;
}

bool parse_trace_qif(std::ifstream &trace_file,
                     std::vector<std::unique_ptr<Inst_Base>> &L,
                     uint64_t instsize,
                     const std::shared_ptr<DynamicFunction> &fun) {
  std::string line;
  uint32_t id_count = 1, num = 1;
  getline(trace_file, line);
  uint64_t size_inst_percent = instsize / 100;
  int percent = 0;

  while (trace_file.good()) {
    if (line.empty()) {
      break;
    }
    auto ins_index = num++;
    id_count++;

    if (size_inst_percent) {

      if ((id_count / size_inst_percent) > percent) {
        percent = id_count / size_inst_percent;
        if (percent % 5 == 0) {
          std::cout << percent << "% parsing" << std::endl;
        }
      }
    }

    std::unique_ptr<Inst_Base> ins = nullptr;

    if (line.find("Start") != std::string::npos) {
      std::vector<std::tuple<uint32_t, uint32_t>> key_symbol;
      std::vector<uint8_t> key_value;

      uint32_t addr_taint = 0, size_taint = 0;
      std::istringstream fun_buf(line);
      std::string start_taint, taint_len, temp_str;
      getline(fun_buf, temp_str, ';');
      getline(fun_buf, start_taint, ';');
      getline(fun_buf, taint_len, ';');
      addr_taint = stoul(start_taint, nullptr, 16);
      size_taint = stoul(taint_len, nullptr, 10);
      auto key_tuple = std::make_tuple(addr_taint, size_taint);
      key_symbol.push_back(key_tuple);

      // Get key value
      getline(trace_file, line);
      std::istringstream strbuf(line);
      std::string key_temp;
      uint8_t key_concrete;
      uint32_t index;

      for (index = 0; index < (size_taint / 8); ++index) {
        getline(strbuf, key_temp, ';');
        key_concrete = std::stoul(key_temp, nullptr, 16);
        key_value.push_back(key_concrete);
      }

      ins = make_unique<UPDATE_SYMBOLS>(key_symbol, key_value);
      ins->id = ins_index;
      L.push_back(std::move(ins));

      getline(trace_file, line);

      continue;
    }

    if (line.find("libc") != std::string::npos) {
      std::string line2;
      getline(trace_file, line2);
      ins = LibC_Factory::parseLibc(line + "; " + line2);
    } else {
      ins = parseInst(line, false, fun);
    }
    ins->id = ins_index;
    L.push_back(std::move(ins));

    getline(trace_file, line);
  }

  return trace_file.good();
}

} // namespace tana