#include "Engine.hpp"
#include "VarMap.hpp"
#include "error.hpp"
#include <algorithm>
#include <stack>

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

namespace tana {

bool SEEngine::isImmSym(uint32_t num) {
  if (!imm2sym) {
    return false;
  }
  if (num < MAX_IMM_NUMBER) {
    return false;
  }
  std::vector<uint32_t> common = {0xff,     0xfff,      0xffff, 0xfffff,
                                  0xffffff, 0xffffffff, 0xff00, 0xffff0000};
  auto find_result = std::find(common.begin(), common.end(), num);
  return !(find_result != common.end());
}

SEEngine::SEEngine(bool state_type) : next_eip(nullptr), current_eip(nullptr) {
  imm2sym = state_type;
  eflags = false;
}

int SEEngine::run() {
  if (start == end) {
    return true;
  }

  for (auto inst = start; std::next(inst) != end;) {
    auto it = inst->get();
    current_eip = it;

    ++inst;
    next_eip = inst->get();

    // if (x86::SymbolicExecutionNoEffect(it->instruction_id))
    //    continue;
    bool status = it->symbolic_execution(this);

    if (!status) {
      // ERROR("No recognized instruction");
      return false;
    }
  }
  return true;
}

void SEEngine::getFormulaLength(const std::shared_ptr<BitVector> &v,
                                uint32_t &len) {
  const std::unique_ptr<Operation> &op = v->opr;
  ++len;
  if (op == nullptr) {
    return;
  }

  if (op->val[0] != nullptr)
    getFormulaLength(op->val[0], len);
  if (op->val[1] != nullptr)
    getFormulaLength(op->val[1], len);
  if (op->val[2] != nullptr)
    getFormulaLength(op->val[2], len);
}

std::shared_ptr<BitVector> SEEngine::Concat(std::shared_ptr<BitVector> v1,
                                            std::shared_ptr<BitVector> v2) {
  std::shared_ptr<BitVector> low = nullptr, high = nullptr, res = nullptr;
  uint32_t size_res = v1->size() + v2->size();

  std::unique_ptr<Operation> oper =
      std::make_unique<Operation>(BVOper::bvconcat, v1, v2);

  if (v1->isSymbol() || v2->isSymbol())
    res = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
  else {
    uint32_t v1_value = eval(v1);
    uint32_t v2_value = eval(v2);
    uint32_t result_value =
        BitVector::concat(v1_value, v2_value, v1->size(), v2->size());
    res = std::make_shared<BitVector>(ValueType::CONCRETE, result_value);
  }

  res->low_bit = 1;
  res->high_bit = size_res;
  return res;
}

std::shared_ptr<BitVector> SEEngine::Concat(std::shared_ptr<BitVector> v1,
                                            std::shared_ptr<BitVector> v2,
                                            std::shared_ptr<BitVector> v3) {
  return Concat(Concat(v1, v2), v3);
}

std::shared_ptr<BitVector> SEEngine::Extract(std::shared_ptr<BitVector> v,
                                             int low, int high) {
  assert(high > low);

  std::unique_ptr<Operation> oper =
      std::make_unique<Operation>(BVOper::bvextract, v);
  std::shared_ptr<BitVector> res = nullptr;
  if (v->isSymbol()) {
    res = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
  } else {
    uint32_t result = eval(v);
    result = BitVector::extract(result, high, low);
    res = std::make_shared<BitVector>(ValueType::CONCRETE, result);
  }
  res->high_bit = high;
  res->low_bit = low;
  return res;
}

std::shared_ptr<BitVector> SEEngine::ZeroExt(std::shared_ptr<tana::BitVector> v,
                                             tana::tana_type::T_SIZE size_new) {
  assert(size_new >= v->size());
  std::unique_ptr<Operation> oper =
      std::make_unique<Operation>(BVOper::bvzeroext, v);
  std::shared_ptr<BitVector> res = nullptr;
  if (v->isSymbol()) {
    res = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
  } else {
    uint32_t result = eval(v);
    result = BitVector::extract(result, size_new, 1);
    res = std::make_shared<BitVector>(ValueType::CONCRETE, result);
  }
  res->high_bit = size_new;
  res->low_bit = 1;
  return res;
}

std::shared_ptr<BitVector> SEEngine::SignExt(std::shared_ptr<tana::BitVector> v,
                                             tana::tana_type::T_SIZE orgin_size,
                                             tana::tana_type::T_SIZE new_size) {
  assert(orgin_size < new_size);
  std::unique_ptr<Operation> oper =
      std::make_unique<Operation>(BVOper::bvsignext, v);
  std::shared_ptr<BitVector> res = nullptr;
  if (v->isSymbol()) {
    res = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
  } else {
    uint32_t result = eval(v);
    result = BitVector::signext(result, orgin_size, new_size);
    res = std::make_shared<BitVector>(ValueType::CONCRETE, result);
  }
  res->high_bit = new_size;
  res->low_bit = 1;
  return res;
}

void SEEngine::init(std::vector<std::unique_ptr<Inst_Base>>::iterator it1,
                    std::vector<std::unique_ptr<Inst_Base>>::iterator it2) {
  this->start = it1;
  this->end = it2;
}

void SEEngine::initAllRegSymol(
    std::vector<std::unique_ptr<Inst_Base>>::iterator it1,
    std::vector<std::unique_ptr<Inst_Base>>::iterator it2) {

  this->start = it1;
  this->end = it2;
}

uint32_t SEEngine::conexec(std::shared_ptr<BitVector> f,
                           const std::map<int, uint32_t> &input) {
  std::set<int> inmapkeys;
  std::set<int> inputsym = f->getInputSymbolSet();
  for (auto it = input.begin(); it != input.end(); ++it) {
    inmapkeys.insert(it->first);
  }

  if (inmapkeys != inputsym) {
    ERROR("Some inputs don't have parameters!");
    return 1;
  }

  return eval(f, input);
}

std::vector<std::shared_ptr<BitVector>>
SEEngine::reduceValues(std::vector<std::shared_ptr<BitVector>> values) {
  auto values_size = values.size();
  std::vector<bool> redundancy_table(values_size, false);
  uint32_t values_size_after_reduction = 0;
  for (uint32_t i = 0; i < values_size; ++i) {
    for (uint32_t j = 0; j < values_size; ++j) {
      if (i == j) {
        continue;
      }
      if (*values[i] == *values[j]) {
        // std::cout << "i = " << i << "j = " << j << std::endl;
        if (!redundancy_table[i]) {
          redundancy_table[j] = true;
        }
      }
    }
  }

  for (uint32_t i = 0; i < values_size; ++i) {
    if (!redundancy_table[i]) {
      ++values_size_after_reduction;
    }
  }

  std::vector<std::shared_ptr<BitVector>> values_reduced(
      values_size_after_reduction);
  uint32_t index = 0;
  for (uint32_t i = 0; i < values_size; ++i) {
    if (!redundancy_table[i]) {
      values_reduced[index] = values[i];
      ++index;
    }
  }

  // Remove formulas that are either too long or too short
  assert(index == values_size_after_reduction);

  auto iter = values_reduced.begin();

  while (iter != values_reduced.end()) {
    uint32_t formula_size = 0;
    SEEngine::getFormulaLength(*iter, formula_size);
    if ((formula_size < FORMULA_MIN_LENGTH) ||
        (formula_size > FORMULA_MAX_LENGTH)) {
      iter = values_reduced.erase(iter);
    } else {
      ++iter;
    }
  }

  return values_reduced;
}

uint32_t SEEngine::debugEval(const std::shared_ptr<BitVector> &v) {
  return SEEngine::eval(v, this->key_value_map);
}

uint32_t SEEngine::eval(const std::shared_ptr<BitVector> &v) { // for no input
  std::map<int, uint32_t> varm;
  assert(v->getInputSymbolSet().empty());
  return eval(v, varm);
}

uint32_t SEEngine::eval_fast(const std::shared_ptr<BitVector> &v,
                             const std::map<int, uint32_t> &inmap) {
  std::stack<std::shared_ptr<BitVector>> stack1, stack2, stack3;
  stack1.push(v);
  while (!stack1.empty()) {
    std::shared_ptr<BitVector> top_v = stack1.top();
    stack1.pop();
    stack2.push(top_v);

    if (top_v == nullptr) {
      continue;
    }

    std::unique_ptr<Operation> &op = top_v->opr;

    if (op == nullptr) {
      continue;
    }

    if (op->val[0] != nullptr) {
      stack1.push(op->val[0]);
    }

    if (op->val[1] != nullptr) {
      stack1.push(op->val[1]);
    }

    if (op->val[2] != nullptr) {
      stack1.push(op->val[2]);
    }
  }

  uint32_t res = 0;
  std::shared_ptr<BitVector> ptr = stack2.top();
  std::shared_ptr<BitVector> v0, v1, v2, v_n;
  do {
    while (ptr->opr == nullptr) {

      stack3.push(ptr);
      stack2.pop();
      ptr = stack2.top();
    }

    stack2.pop();
    switch (ptr->opr->opty) {
    case BVOper::bvzeroext: {
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::zeroext(v0->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }
    case BVOper::bvextract: {
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::extract(v0->concrete_value, ptr->high_bit, ptr->low_bit);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvconcat: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::concat(v0->concrete_value, v1->concrete_value,
                              v0->size(), v1->size());
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      v_n->high_bit = v0->size() + v1->size();
      stack3.push(v_n);
      break;
    }

    case BVOper::bvsignext: {
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::signext(v0->concrete_value, v0->size(), ptr->size());
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      v_n->high_bit = ptr->size();
      stack3.push(v_n);
      break;
    }

    case BVOper::bvadd: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v1->concrete_value + v0->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvsub: {

      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value - v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvshld: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::shld32(v0->concrete_value, v1->concrete_value,
                              v2->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvshrd: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::shrd32(v0->concrete_value, v1->concrete_value,
                              v2->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvxor: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value ^ v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvxor3: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value ^ v1->concrete_value ^ v2->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvand: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value & v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvand3: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value & v1->concrete_value & v2->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvor: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value | v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvor3: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value | v1->concrete_value | v2->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvshl: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value << v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvshr: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value >> v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvbit: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bit(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvsar: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::arithmeticRightShift(v0->concrete_value,
                                            v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvneg: {
      v0 = stack3.top();
      stack3.pop();
      res = ~v0->concrete_value + 1;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvnot: {
      v0 = stack3.top();
      stack3.pop();
      res = ~v0->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvrol: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::rol32(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvror: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::ror32(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvquo: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value / v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvrem: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value % v1->concrete_value;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::equal: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = (v0->concrete_value == v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::noequal: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = (v0->concrete_value != v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::greater: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = (v0->concrete_value > v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::less: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = (v0->concrete_value < v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvbitnot: {
      v0 = stack3.top();
      stack3.pop();
      res = v0->concrete_value ? 0 : 1;
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvbsf: {
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bsf(v0->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvmul32_h: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvmul32_h(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvmul32_l: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvmul32_l(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvmul: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvmul16_8(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul32_l: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul32_l(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul32_h: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul32_h(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul16_l: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul16_l(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul16_h: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul16_h(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul8_l: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul8_l(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvimul8_h: {
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvimul8_h(v0->concrete_value, v1->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvidiv32_quo: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvidiv32_quo(v0->concrete_value, v1->concrete_value,
                                    v2->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvidiv32_rem: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvidiv32_rem(v0->concrete_value, v1->concrete_value,
                                    v2->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvdiv32_quo: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvdiv32_quo(v0->concrete_value, v1->concrete_value,
                                   v2->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }

    case BVOper::bvdiv32_rem: {
      v2 = stack3.top();
      stack3.pop();
      v1 = stack3.top();
      stack3.pop();
      v0 = stack3.top();
      stack3.pop();
      res = BitVector::bvdiv32_rem(v0->concrete_value, v1->concrete_value,
                                   v2->concrete_value);
      v_n = std::make_shared<BitVector>(ValueType::CONCRETE, res);
      stack3.push(v_n);
      break;
    }
    }
    if (!stack2.empty()) {
      ptr = stack2.top();
    }
  } while (!stack2.empty());
  return stack3.top()->concrete_value;
}

uint32_t SEEngine::eval(const std::shared_ptr<BitVector> &v,
                        const std::map<int, uint32_t> &inmap) {
  const std::unique_ptr<Operation> &op = v->opr;

  if (op == nullptr) {
    if (v->val_type == ValueType::CONCRETE)
      return v->concrete_value;
    else
      return inmap.at(v->id);
  } else {
    uint32_t op0 = 0, op1 = 0;
    uint32_t op2 = 0;
    uint32_t op_num = 0;

    if (op->val[0] != nullptr) {
      ++op_num;
      op0 = eval(op->val[0], inmap);
    }
    if (op->val[1] != nullptr) {
      ++op_num;
      op1 = eval(op->val[1], inmap);
    }
    if (op->val[2] != nullptr) {
      ++op_num;
      op2 = eval(op->val[2], inmap);
    }

    switch (op->opty) {
    case BVOper::bvzeroext:
      return BitVector::zeroext(op0);

    case BVOper::bvextract:
      return BitVector::extract(op0, v->high_bit, v->low_bit);

    case BVOper::bvconcat:
      return BitVector::concat(op0, op1, op->val[0]->size(),
                               op->val[1]->size());

    case BVOper::bvsignext:
      return BitVector::signext(op0, op->val[0]->size(), v->size());

    case BVOper::bvadd:
      return op0 + op1;

    case BVOper::bvsub:
      return op0 - op1;

    case BVOper::bvimul:
      return BitVector::bvimul(op0, op1);

    case BVOper::bvshld:
      return BitVector::shld32(op0, op1, op2);

    case BVOper::bvshrd:
      return BitVector::shrd32(op0, op1, op2);

    case BVOper::bvxor:
      return op0 ^ op1;

    case BVOper::bvxor3:
      return op0 ^ op1 ^ op2;

    case BVOper::bvand:
      return op0 & op1;

    case BVOper::bvand3:
      return op0 & op1 & op2;

    case BVOper::bvor:
      return op0 | op1;

    case BVOper::bvor3:
      return op0 | op1 | op2;

    case BVOper::bvshl:
      return op0 << op1;

    case BVOper::bvshr:
      return op0 >> op1;

    case BVOper::bvbit:
      return BitVector::bit(op0, op1);

    case BVOper::bvsar:
      return BitVector::arithmeticRightShift(op0, op1);

    case BVOper::bvneg:
      return ~op0 + 1;

    case BVOper::bvnot:
      return ~op0;

    case BVOper::bvrol:
      return BitVector::rol32(op0, op1);

    case BVOper::bvror:
      return BitVector::ror32(op0, op1);

    case BVOper::bvquo: {
      if (op1 == 0) {
        return 0xffffffff;
      }
      return op0 / op1;
    }
    case BVOper::bvrem: {
      if (op1 == 0) {
        return 0xffffffff;
      }
      return op0 % op1;
    }
    case BVOper::equal:
      return op0 == op1;

    case BVOper::noequal:
      return op0 != op1;

    case BVOper::greater:
      return op0 > op1;

    case BVOper::less:
      return op0 < op1;

    case BVOper::bvbitnot:
      return op0 ? 0 : 1;

    case BVOper::bvbsf:
      return BitVector::bsf(op0);

    case BVOper::bvmul32_h:
      return BitVector::bvmul32_h(op0, op1);

    case BVOper::bvmul32_l:
      return BitVector::bvmul32_l(op0, op1);

    case BVOper::bvmul:
      return BitVector::bvmul16_8(op0, op1);

    case BVOper::bvimul32_l:
      return BitVector::bvimul32_l(op0, op1);

    case BVOper::bvimul32_h:
      return BitVector::bvimul32_h(op0, op1);

    case BVOper::bvimul16_l:
      return BitVector::bvimul16_l(op0, op1);

    case BVOper::bvimul16_h:
      return BitVector::bvimul16_h(op0, op1);

    case BVOper::bvimul8_l:
      return BitVector::bvimul8_l(op0, op1);

    case BVOper::bvimul8_h:
      return BitVector::bvimul8_h(op0, op1);

    case BVOper::bvidiv32_quo:
      return BitVector::bvidiv32_quo(op0, op1, op2);

    case BVOper::bvidiv32_rem:
      return BitVector::bvidiv32_rem(op0, op1, op2);

    case BVOper::bvdiv32_quo:
      return BitVector::bvdiv32_quo(op0, op1, op2);

    case BVOper::bvdiv32_rem:
      return BitVector::bvdiv32_rem(op0, op1, op2);
    }
  }
}

std::shared_ptr<BitVector>
SEEngine::formula_simplfy(std::shared_ptr<tana::BitVector> v) {
  const std::unique_ptr<Operation> &op = v->opr;
  if (op == nullptr) {
    return v;
  }
  uint32_t input_num = v->symbol_num();
  if (input_num == 0) {
    uint32_t res = eval(v);
    auto res_v = std::make_shared<BitVector>(ValueType::SYMBOL, res);
    return res_v;
  }

  if (op->val[0] != nullptr)
    op->val[0] = formula_simplfy(op->val[0]);
  if (op->val[1] != nullptr)
    op->val[1] = formula_simplfy(op->val[1]);
  if (op->val[2] != nullptr)
    op->val[2] = formula_simplfy(op->val[2]);

  return v;
}

uint32_t SEEngine::getRegisterConcreteValue(std::string reg_name) {
  x86::x86_reg reg_id = x86::reg_string2id(reg_name);
  uint32_t reg_index = Registers::getRegIndex(reg_id);
  return next_eip->vcpu.gpr[reg_index];
}

} // namespace tana