

#pragma once

#include "BitVector.hpp"
#include "Constrains.hpp"
#include "Engine.hpp"
#include "Function.hpp"
#include "Trace2ELF.hpp"
#include <map>
#include <memory>
#include <random>
#include <vector>

namespace tana {
namespace MonteCarlo {

std::vector<uint8_t> getRandomVector(unsigned int size);

std::map<int, uint32_t> input2val(const std::vector<uint8_t> &input,
                                  std::vector<int> &bv);

std::vector<int> getAllKeys(
    const std::vector<std::tuple<uint32_t, std::shared_ptr<tana::Constrain>,
                                 LeakageType>> &constrains);

bool constrainSatisfy(
    const std::vector<std::tuple<uint32_t, std::shared_ptr<tana::Constrain>,
                                 LeakageType>> &constrains,
    const std::map<int, uint32_t> &input_map);

float calculateMonteCarlo(
    const std::vector<std::tuple<uint32_t, std::shared_ptr<tana::Constrain>,
                                 LeakageType>> &constrains,
    uint64_t sample_num);

} // namespace MonteCarlo

class FastMonteCarlo {
private:
  std::vector<std::unique_ptr<std::pair<std::vector<uint8_t>, bool>>> tests;
  std::vector<
      std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>>
      constrains;
  uint64_t num_sample;
  uint64_t num_satisfied;
  std::vector<int> input_vector;

  void testConstrain(const std::shared_ptr<tana::Constrain> &con);

  void testConstrain(
      const std::shared_ptr<tana::Constrain> &con,
      std::vector<std::unique_ptr<std::pair<std::vector<uint8_t>, bool>>>
          &cons_tests,
      std::vector<int> con_vector_int);

  std::vector<uint8_t> getRandomVector(unsigned int size);

  std::random_device rd;
  std::uniform_int_distribution<uint8_t> dist;
  std::minstd_rand engine{rd()};

  std::vector<uint8_t> input_seed;

  std::vector<std::vector<
      std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>>>
      constrains_group_addr;

  std::vector<
      std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>> *
  internal_find_constrain_group_by_addr(uint32_t addr);

  std::vector<std::tuple<uint32_t, uint64_t, LeakageType, uint64_t>>
      num_satisfied_group;

  void reset_tests();

  bool isFunctionInformationAvailable;

  std::shared_ptr<DynamicFunction> func;

  std::map<int, uint32_t> key_value_map;

  std::vector<std::unique_ptr<std::pair<std::vector<uint8_t>, bool>>>
  generate_random_tests(uint32_t dim, uint64_t size);

public:
  FastMonteCarlo(
      uint64_t sample_num,
      std::vector<
          std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>>
          constrains,
      const std::map<int, uint32_t> &key_value_map);

  FastMonteCarlo(
      uint64_t sample_num,
      std::vector<
          std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>>
          constrains,
      std::shared_ptr<DynamicFunction> func,
      const std::map<int, uint32_t> &key_value_map);

  void run();

  float getResult();

  bool verifyConstrain();

  void spiltConstrainsByAddress();

  void run_addr_group();

  void print_group_result(const std::string &resultFile,
                          std::shared_ptr<Trace2ELF> t2e);

  void infoConstrains(const std::string &result);
};
} // namespace tana
