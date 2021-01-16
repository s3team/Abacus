
#pragma once

#include "ins_types.hpp"
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace tana {

class BitVector;

class Operation;

enum class ValueType { SYMBOL, CONCRETE };

enum class BVOper {
  bvadd,
  bvsub,
  bvimul,
  bvshld,
  bvshrd,
  bvxor,
  bvxor3,
  bvand,
  bvand3,
  bvor,
  bvor3,
  bvshl,
  bvshr,
  bvsar,
  bvneg,
  bvnot,
  bvrol,
  bvror,
  bvquo,
  bvrem,
  bvbit,
  bvbsf,
  equal,
  noequal,
  greater,
  less,
  bvzeroext,
  bvextract,
  bvconcat,
  bvsignext,
  bvbitnot,
  bvmul32_h,
  bvmul32_l,
  bvmul,
  bvimul32_l,
  bvimul32_h,
  bvimul16_l,
  bvimul16_h,
  bvimul8_h,
  bvimul8_l,
  bvidiv32_quo,
  bvidiv32_rem,
  bvdiv32_quo,
  bvdiv32_rem
};

class BitVector {
private:

  std::string print() const;

  int formula_length = -1;
  bool formula_length_cache = false;

  bool input_symbol_cache = false;

  std::set<int> inputSymbols;

  void printV(std::stringstream &ss, uint32_t &length) const;

  int symbol_num_cache = -1; // -1 means cache miss

public:
  const int id;                  // a unique id for each value
  const ValueType val_type;      // value type: SYMBOL or CONCRETE
  const uint32_t concrete_value; // concrete value
  uint32_t low_bit = 1;
  uint32_t high_bit = REGISTER_SIZE;
  static int idseed;
  const std::string info;
  std::unique_ptr<Operation> opr = nullptr;

  uint32_t formula_cache_concrete_value;
  bool flag_formula_cache_concrete;

  BitVector() = delete;

  explicit BitVector(ValueType vty, const std::string &symbol_info);

  explicit BitVector(ValueType vty, uint32_t concrete,
                     bool Imm2SymState); // constructor for concrete value
  explicit BitVector(ValueType vty, uint32_t concrete, bool Imm2SymState,
                     uint32_t size); // constructor for concrete value
  explicit BitVector(ValueType vty, std::unique_ptr<Operation> oper);

  explicit BitVector(ValueType vty, uint32_t con);

  explicit BitVector(ValueType vty, const std::string &symbol_info,
                     uint32_t size);

  std::vector<int> getInputSymbolVector();

  std::set<int> getInputSymbolSet();

  static uint32_t arithmeticRightShift(uint32_t op1, uint32_t op2);

  static uint32_t rol32(uint32_t op1, uint32_t op2);

  static uint32_t ror32(uint32_t op1, uint32_t op2);

  static uint32_t shld32(uint32_t op0, uint32_t op1, uint32_t op2);

  static uint32_t shrd32(uint32_t op0, uint32_t op1, uint32_t op2);

  static uint32_t extract(uint32_t op1, uint32_t high, uint32_t low);

  static uint32_t concat(uint32_t op1, uint32_t op2, uint32_t op1_size,
                         uint32_t op2_size);

  static uint32_t zeroext(uint32_t op1);

  static uint32_t signext(uint32_t op1, uint32_t origin_size,
                          uint32_t new_size);

  static uint32_t bsf(uint32_t op);

  static uint32_t bvimul(uint32_t op1, uint32_t op2);

  static uint32_t bvimul32_l(uint32_t op1, uint32_t op2);

  static uint32_t bvimul32_h(uint32_t op1, uint32_t op2);

  static uint32_t bvimul16_l(uint32_t op1, uint32_t op2);

  static uint32_t bvimul16_h(uint32_t op1, uint32_t op2);

  static uint32_t bvimul8_l(uint32_t op1, uint32_t op2);

  static uint32_t bvimul8_h(uint32_t op1, uint32_t op2);

  static uint32_t bvmul32_h(uint32_t op1, uint32_t op2);

  static uint32_t bvmul32_l(uint32_t op1, uint32_t op2);

  static uint32_t bvmul16_8(uint32_t op1, uint32_t op2);

  static uint32_t bvidiv32_quo(uint32_t edx, uint32_t eax, uint32_t op);

  static uint32_t bvidiv32_rem(uint32_t edx, uint32_t eax, uint32_t op);

  static uint32_t bvdiv32_quo(uint32_t edx, uint32_t eax, uint32_t div);

  static uint32_t bvdiv32_rem(uint32_t edx, uint32_t eax, uint32_t div);

  static bool bit(uint32_t op0, uint32_t op1);

  bool isSymbol() const;

  uint32_t size() const;

  bool operator==(const BitVector &v1);

  uint32_t printV(std::stringstream &ss) const;

  uint32_t symbol_num();

  int length();

  friend std::ostream &operator<<(std::ostream &os, const BitVector &c);

  std::vector<uint32_t> get_bitvector_hash();

  std::vector<uint32_t> get_bitvector_hash_old();


  uint32_t eval(const std::map<int, uint32_t> &val_map);
};

// An operation taking several values to calculate a result value
class Operation {
public:
  BVOper opty;
  std::shared_ptr<BitVector> val[3] = {nullptr, nullptr, nullptr};

  Operation(BVOper opt, std::shared_ptr<BitVector> v1);

  Operation(BVOper opt, std::shared_ptr<BitVector> v1,
            std::shared_ptr<BitVector> v2);

  Operation(BVOper opt, std::shared_ptr<BitVector> v1,
            std::shared_ptr<BitVector> v2, std::shared_ptr<BitVector> v3);
};

std::shared_ptr<BitVector> buildop1(BVOper opty, std::shared_ptr<BitVector> v1);

std::shared_ptr<BitVector> buildop2(BVOper opty, std::shared_ptr<BitVector> v1,
                                    std::shared_ptr<BitVector> v2);

std::shared_ptr<BitVector> buildop2(BVOper opty, std::shared_ptr<BitVector> v1,
                                    uint32_t v2);

std::shared_ptr<BitVector> buildop3(BVOper opty, std::shared_ptr<BitVector> v1,
                                    std::shared_ptr<BitVector> v2,
                                    std::shared_ptr<BitVector> v3);

} // namespace tana