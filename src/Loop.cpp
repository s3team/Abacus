
#include "Loop.hpp"
#include "Register.hpp"
#include "Tainter.hpp"
#include "VarMap.hpp"
#include "ins_types.hpp"
#include <fstream>
#include <sstream>

namespace tana {

namespace loop {
std::shared_ptr<std::map<std::string, tana_type::index>>
    inst_enum(new std::map<std::string, tana_type::index>);

tana_type::index getOpcIndex(
    std::string &opc_name,
    std::shared_ptr<std::map<std::string, tana_type::index>> &name_map) {
  auto it = name_map->find(opc_name);
  if (it != name_map->end())
    return it->second;
  else
    return 0; // Not Found
}

bool Loop::operator==(const Loop &loop) {
  if (loopSize != loop.loopSize) {
    return false;
  }

  auto loop1_begin = loop_begin;
  auto loop2_begin = loop.loop_begin;

  auto loop1_end = loop_end;
  auto loop2_end = loop.loop_end;

  while ((loop1_begin != loop1_end) || (loop2_begin != loop2_end)) {
    if ((loop1_begin->instruction_id) != (loop2_begin->instruction_id)) {
      return false;
    }

    if ((loop1_begin->oprs) != (loop2_begin->oprs)) {
      return false;
    }

    ++loop1_begin;
    ++loop2_begin;
  }

  if ((loop1_begin == loop1_end) && (loop2_begin == loop2_end)) {
    return true;
  }
  return false;
}

bool Loop::operator!=(const Loop &loop) { return !((*this) == loop); }

bool Loop::operator<(const Loop &loop) { return (loopSize < loop.loopSize); }

bool Loop::operator>(const Loop &loop) { return (loop.loopSize < loopSize); }

Loop::Loop(std::vector<Inst_Base>::const_iterator begin,
           std::vector<Inst_Base>::const_iterator end) {
  loop_begin = begin;
  loop_end = end;
  loopSize = static_cast<size_t>(std::distance(loop_begin, loop_end));
  loop_current = loop_begin;
  isGoodLoop = false;
}

bool Loop::isCheckedFinish() { return (next(loop_current, 1) == loop_end); }

bool Loop::checkLoopCurrent(
    std::vector<Inst_Base>::const_iterator inst_current) {
  ++loop_current;
  if (inst_current->instruction_id == loop_current->instruction_id)
    return true;
  return false;
}

std::list<Loop>
findPotentialLoop(std::vector<Inst_Base>::const_iterator current,
                  std::vector<Inst_Base>::const_iterator instruction_end,
                  uint32_t history_length, uint32_t min_loop_length) {
  std::list<Loop> loop_candidate;
  auto loop_end = current;
  auto loop_start = std::prev(current, current->id - 1);

  if (current->id > history_length) {
    loop_start = std::prev(current, history_length);
  }

  for (; loop_start != loop_end; ++loop_start) {
    if (loop_start->instruction_id != current->instruction_id) {
      continue;
    }

    if ((std::next(current, 1) == instruction_end) ||
        (std::next(current, 2) == instruction_end) ||
        (std::next(current, 3) == instruction_end) ||
        (std::next(current, 5) == instruction_end))
      continue;

    if ((std::next(loop_start, 1)->instruction_id ==
         std::next(current, 1)->instruction_id) &&
        (std::next(loop_start, 2)->instruction_id ==
         std::next(current, 2)->instruction_id) &&
        (std::next(loop_start, 3)->instruction_id ==
         std::next(current, 3)->instruction_id) &&
        (std::next(loop_start, 5)->instruction_id ==
         std::next(current, 5)->instruction_id)) {
      Loop potential_loop(loop_start, loop_end);
      if (potential_loop.getLoopLength() < min_loop_length) {
        continue;
      }
      loop_candidate.push_back(potential_loop);
    }
  }
  return std::move(loop_candidate);
}

std::vector<Loop> loopDetectionFast(std::vector<Inst_Base> *L, uint32_t id) {
  std::vector<Loop> confirmLoop;
  std::list<Loop> potential_loops;
  Loop current_loop = Loop(L->cend(), L->cend());
  std::cout << id << " instructions in total\n";
  InstructionMap inst_map;
  for (auto it = L->cbegin(); it != L->cend(); ++it) {
    // checkLoop
    if (it->id % LOOP_PRINT_FREQUENCY == 0) {
      std::cout << "Processing " << it->id << " Instruction "
                << "Progress: " << static_cast<float>(it->id) / id << "\n";
    }
    auto current_loop_size = current_loop.getLoopLength();
    if (current_loop_size > 0) {
      auto temp_loop_end = std::next(it, current_loop_size);
      Loop temp_loop = Loop(it, temp_loop_end);
      if (temp_loop == current_loop) {
        it = std::next(it, current_loop_size - 1);
        continue;
      } else {
        current_loop = Loop(L->cend(), L->cend());
        potential_loops.clear();
      }
    }

    auto each_potential_loop = potential_loops.begin();
    while (each_potential_loop != potential_loops.end()) {
      if (each_potential_loop->checkLoopCurrent(it)) {
        if (each_potential_loop->isCheckedFinish()) {
          auto exist_loop = std::find(confirmLoop.begin(), confirmLoop.end(),
                                      *each_potential_loop);
          if (exist_loop == confirmLoop.end()) {
            confirmLoop.push_back(*each_potential_loop);
            current_loop = *each_potential_loop;
          }
          each_potential_loop = potential_loops.erase(each_potential_loop);
        } else {
          ++each_potential_loop;
        }
      } else {
        each_potential_loop = potential_loops.erase(each_potential_loop);
      }
    }
    std::list<Loop> temp;
    temp = inst_map.findPotentialLoops(it, L->cend());
    potential_loops.splice(potential_loops.end(), temp);
    inst_map.updateMap(it);
  }
  return confirmLoop;
}

std::vector<Loop>
loopDetection(const std::vector<std::unique_ptr<Inst_Base>> &Lptr,
              uint32_t id) {
  std::vector<Loop> confirmLoop;
  std::list<Loop> potential_loops;
  std::vector<Inst_Base> Inst;
  for (const auto &ins : Lptr) {
    Inst.push_back(*ins);
  }

  std::vector<Inst_Base> *L = &Inst;

  Loop current_loop = Loop(L->cend(), L->cend());
  std::cout << id << " instructions in total\n";
  for (auto it = L->cbegin(); it != L->cend(); ++it) {

    // checkLoop

    if (it->id % LOOP_PRINT_FREQUENCY == 0) {
      std::cout << "Processing " << it->id << " Instruction "
                << "Progress: " << static_cast<float>(it->id) / id << "\n";
    }
    auto current_loop_size = current_loop.getLoopLength();
    if (current_loop_size > 0) {
      auto temp_loop_end = std::next(it, current_loop_size);
      Loop temp_loop = Loop(it, temp_loop_end);
      if (temp_loop == current_loop) {
        it = std::next(it, current_loop_size - 1);
        continue;
      } else {
        current_loop = Loop(L->cend(), L->cend());
        potential_loops.clear();
      }
    }
    auto each_potential_loop = potential_loops.begin();
    while (each_potential_loop != potential_loops.end()) {
      if (each_potential_loop->checkLoopCurrent(it)) {
        if (each_potential_loop->isCheckedFinish()) {
          auto exist_loop = std::find(confirmLoop.begin(), confirmLoop.end(),
                                      *each_potential_loop);
          if (exist_loop == confirmLoop.end()) {
            confirmLoop.push_back(*each_potential_loop);
            current_loop = *each_potential_loop;
          }
          each_potential_loop = potential_loops.erase(each_potential_loop);
        } else {
          ++each_potential_loop;
        }
      } else {
        each_potential_loop = potential_loops.erase(each_potential_loop);
      }
    }
    std::list<Loop> temp;
    temp = findPotentialLoop(it, L->cend(), MAX_LOOP_HISTORY, MIN_LOOP_LENGTH);
    potential_loops.splice(potential_loops.end(), temp);
  }
  return confirmLoop;
}

void outPrintLoops(std::vector<Loop> loops, std::string file_name) {
  int index = 0;
  for (auto it = loops.begin(); it != loops.end(); ++it) {
    std::string loopFile = file_name + std::to_string(index++) + ".ref";
    std::ofstream fp;
    fp.open(loopFile);

    for (auto ins = it->getStart(); ins != it->getEnd(); ++ins) {

      std::string opc_opr = ins->get_opcode_operand();
      fp << std::hex << ins->address << ";";

      fp << opc_opr << ";";
      // fp << ins->dissass << ";";
      for (uint32_t j = 0; j < GPR_NUM; ++j) {
        fp << ins->vcpu.gpr[j] << ",";
      }
      fp << ins->memory_address << std::dec << ",\n";
    }
    fp.close();
  }
}

std::list<Loop> InstructionMap::findPotentialLoops(
    std::vector<Inst_Base>::const_iterator current,
    std::vector<Inst_Base>::const_iterator instruction_end) {
  std::list<Loop> potential_loops;
  auto current_inst_id = current->id;
  auto current_inst = current->instruction_id;
  auto find_result = instMap.find(current_inst);
  if (find_result == instMap.end()) {
    return potential_loops;
  }

  auto &location_list = find_result->second;

  for (auto loc = location_list.crbegin(); loc != location_list.crend();
       ++loc) {
    auto loop_length = current_inst_id - *loc;
    if (loop_length < MIN_LOOP_LENGTH) {
      continue;
    }
    if (loop_length > MAX_LOOP_HISTORY) {
      break;
    }
    auto loop_start = std::prev(current, loop_length);
    auto loop_end = current;

    if ((std::next(current, 1) == instruction_end) ||
        (std::next(current, 2) == instruction_end) ||
        (std::next(current, 3) == instruction_end) ||
        (std::next(current, 5) == instruction_end))
      continue;

    if ((std::next(loop_start, 1)->instruction_id ==
         std::next(current, 1)->instruction_id) &&
        (std::next(loop_start, 2)->instruction_id ==
         std::next(current, 2)->instruction_id) &&
        (std::next(loop_start, 3)->instruction_id ==
         std::next(current, 3)->instruction_id) &&
        (std::next(loop_start, 5)->instruction_id ==
         std::next(current, 5)->instruction_id)) {
      Loop potential_loop(loop_start, loop_end);
      potential_loops.push_back(potential_loop);
    }
  }

  return potential_loops;
}

void InstructionMap::updateMap(std::vector<Inst_Base>::const_iterator current) {
  auto current_inst_id = current->id;
  auto current_inst = current->instruction_id;
  auto &inst_list = instMap[current_inst];
  inst_list.push_back(current_inst_id);
}

} // namespace loop
} // namespace tana
