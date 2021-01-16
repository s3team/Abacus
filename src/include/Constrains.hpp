
#pragma once

#include "BitVector.hpp"
#include "CallStack.hpp"
#include "Function.hpp"
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace tana {

class Constrain {
private:
  uint32_t eip_addr;
  std::shared_ptr<BitVector> r;
  std::vector<std::shared_ptr<CallLeakageSites>> callsites;

public:
  Constrain() = default;

  Constrain(uint32_t, std::shared_ptr<BitVector> b, BVOper, uint32_t);

  Constrain(uint32_t, std::shared_ptr<BitVector> b1, BVOper,
            std::shared_ptr<BitVector> b2);

  void update(BVOper add_type, std::shared_ptr<BitVector> b, BVOper type,
              uint32_t num);

  friend std::ostream &operator<<(std::ostream &os, const Constrain &c);

  bool validate();

  bool validate(const std::map<int, uint32_t> &);

  uint32_t getNumSymbols();

  std::vector<int> getInputKeys();

  void
  updateCallSites(std::vector<std::shared_ptr<CallLeakageSites>> callsites);
};

} // namespace tana