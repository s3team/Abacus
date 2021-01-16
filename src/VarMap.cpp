
#include "VarMap.hpp"
#include <algorithm>
#include <bitset>
#include <cassert>
#include <sstream>

namespace tana {

varmap::BitValue::BitValue() {
  symbol = nullptr;
  bit_index = 0;
}

varmap::BitValue::BitValue(std::shared_ptr<BitVector> v, tana_type::index n) {
  symbol = v;
  bit_index = n;
}

varmap::BitMatrix::BitMatrix(std::vector<std::vector<bool>> mx) { m = mx; }

varmap::BitMatrix::BitMatrix(uint32_t row_length, uint32_t col_length) {
  m.resize(row_length);
  for (uint32_t i = 0, rown = m.size(); i < rown; ++i)
    m[i].resize(col_length);
}

void varmap::BitMatrix::initIdentityMatrix(uint32_t m_size) {
  std::vector<bool> temp(m_size);

  for (uint32_t i = 0; i < m_size; ++i) {
    for (uint32_t j = 0; j < m_size; ++j) {
      if (i == j)
        temp[j] = 1;
      else
        temp[i] = 0;
    }
    m.push_back(temp);
  }
}

void varmap::BitMatrix::setCol(uint32_t col_index, bool b) {
  uint32_t num_row = m.size();
  for (uint32_t i = 0; i < num_row; ++i) {
    m[i][col_index] = b;
  }
}

void varmap::BitMatrix::setRow(std::vector<bool> bit_vector,
                               uint32_t row_index) {
  m[row_index] = bit_vector;
}

void varmap::BitMatrix::setmValue(uint32_t row, uint32_t col, bool v) {
  if (m.empty()) {
    std::cout << "M is empty" << std::endl;
    return;
  }
  m[row][col] = v;
}

void varmap::BitMatrix::show() const {
  uint32_t num_row = m.size();
  for (uint32_t i = 0; i < num_row; ++i) {
    for (uint32_t j = 0; j < m[i].size(); ++j) {
      std::cout << m[i][j] << " ";
    }
    std::cout << std::endl;
  }
}

std::vector<bool> varmap::BitMatrix::getRow(uint32_t row_index) const {
  assert(!m.empty());
  assert(row_index < m.size());
  std::vector<bool> temp_v(m.front().size());
  for (uint32_t i = 0; i < m.front().size(); ++i) {
    temp_v[i] = m[row_index][i];
  }

  return temp_v;
}

std::vector<uint32_t> varmap::BitMatrix::getRowVector() const {
  uint32_t num_row = m.size();
  uint32_t num_col = m.front().size();
  std::vector<uint32_t> row_vector;
  row_vector.reserve(num_row);
  for (uint32_t row_index = 0; row_index < num_row; ++row_index) {
    uint32_t row_sum = 0;

    for (uint32_t col_index = 0; col_index < num_col; ++col_index) {
      if (m[row_index][col_index])
        row_sum += 1;
    }

    row_vector.push_back(row_sum);
  }

  return row_vector;
}

std::vector<uint32_t> varmap::BitMatrix::getColVector() const {
  uint32_t num_row = m.size();
  uint32_t num_col = m.front().size();
  std::vector<uint32_t> col_vector;

  for (uint32_t col_index = 0; col_index < num_col; ++col_index) {
    uint32_t col_sum = 0;

    for (uint32_t row_index = 0; row_index < num_row; ++row_index) {
      if (m[row_index][col_index])
        col_sum += 1;
    }

    col_vector.push_back(col_sum);
  }
  return col_vector;
}

uint32_t varmap::BitMatrix::getColNum() const {
  return (m.empty()) ? 0 : m.front().size();
}

uint32_t varmap::BitMatrix::getRowNum() const { return m.size(); }

bool varmap::varmapAndoutputCVC(SEEngine *se1, std::shared_ptr<BitVector> v1,
                                SEEngine *se2, std::shared_ptr<BitVector> v2) {
  auto inv1 = v1->getInputSymbolVector();
  auto inv2 = v2->getInputSymbolVector();

  // skip variable mapping when the inputs have different number of bits
  if (inv1.size() != inv2.size()) {
    return false; // no mapping found
  }

  if (inv1.size() == 1)
    return false;

  if (inv1.empty()) {
    uint32_t res1 = se1->eval(v1);
    uint32_t res2 = se2->eval(v2);
    if ((res1 == 0) || (res1 == 1) ||
        (res1 == 0xff)) // some formulas can be simpified into a constant number
      return false;
    if (res1 == res2) {
      std::cout << "One possible match" << std::endl;
      return true;
    }
    return false;
  }

  std::set<uint32_t> inset1, inset2;
  for (uint32_t i = 0, max = inv1.size(); i < max; ++i) {
    for (uint32_t j = 0; j < REGISTER_SIZE; ++j) {
      inset1.insert(REGISTER_SIZE * i + j);
    }
  }
  inset2 = inset1;

  std::set<uint32_t> outset1, outset2;
  for (uint32_t i = 0; i < REGISTER_SIZE; ++i) {
    outset1.insert(i);
  }
  outset2 = outset1;

  PartMap pm1(inset1, inset2);
  PartMap pm2(outset1, outset2);

  std::vector<PartMap> vpm1 = {pm1};
  std::vector<PartMap> vpm2 = {pm2};

  std::map<uint32_t, uint32_t> input_map, output_map;
  std::list<FullMap> result;
  bitVarmap(v1, v2, se1, se2, inv1, inv2, vpm1, vpm2, input_map, output_map,
            &result);
  if (!result.empty()) {
    std::cout << "One possible match" << std::endl;
    return true;
  } else
    return false;
}

void varmap::setOutputMatrix(BitMatrix *inmput_m,
                             std::shared_ptr<BitVector> formula,
                             const std::vector<int> &input_v,
                             std::vector<std::shared_ptr<BitVector>> *output_v,
                             SEEngine *se, BitMatrix *output_m) {
  uint32_t row_size = inmput_m->getRowNum();
  for (uint32_t i = 0; i < row_size; ++i) {
    std::vector<bool> row = inmput_m->getRow(i);
    std::map<int, uint32_t> input = bv2var(row, input_v);
    uint32_t output = se->conexec(formula, input);
    std::map<int, uint32_t> outmap = {{(*output_v)[0]->id, output}};

    std::vector<int> output_v_int;

    for (auto v : *output_v) {
      output_v_int.push_back(v->id);
    }

    std::vector<bool> outbv = var2bv(outmap, output_v_int);
    output_m->setRow(outbv, i);
  }
}

std::vector<bool> varmap::var2bv(const std::map<int, uint32_t> &varm,
                                 const std::vector<int> &vv) {
  auto max = vv.size();
  std::vector<bool> bv(max * REGISTER_SIZE);

  if (varm.size() != vv.size()) {
    std::cout << "var2bv: varm and vv are not consistent!" << std::endl;
    return bv;
  }

  for (uint32_t i = 0; i < max; ++i) {
    std::bitset<REGISTER_SIZE> bs((varm).at(vv[i]));
    for (uint32_t j = 0; j < REGISTER_SIZE; ++j) {
      bv[i * REGISTER_SIZE + j] = bs[j];
    }
  }

  return bv;
}

std::map<int, uint32_t> varmap::bv2var(std::vector<bool> bv,
                                       const std::vector<int> &vv) {
  std::map<int, uint32_t> varm;
  if (bv.size() != REGISTER_SIZE * vv.size()) {
    std::cout << "bv2var: bv and vv are not consistent" << std::endl;
    return varm;
  }

  std::bitset<REGISTER_SIZE> bs;
  for (uint32_t i = 0, max = vv.size(); i < max; ++i) {
    for (uint32_t j = 0; j < REGISTER_SIZE; ++j) {
      bs[j] = bv[i * REGISTER_SIZE + j];
    }
    varm.insert(std::pair<int, uint32_t>(vv[i], bs.to_ulong()));
  }

  return varm;
}

void varmap::updateUnmap(std::vector<PartMap> *unmap,
                         std::vector<uint32_t> *vec1,
                         std::vector<uint32_t> *vec2,
                         std::map<uint32_t, uint32_t> *mappedvar) {
  std::set<uint32_t> mapped_var1;
  std::set<uint32_t> mapped_var2;
  std::list<uint32_t> unmapped_var1;
  std::list<uint32_t> unmapped_var2;

  for (auto it = mappedvar->begin(); it != mappedvar->end(); ++it) {
    mapped_var1.insert(it->first);
    mapped_var2.insert(it->second);
  }

  uint32_t num_v1 = vec1->size();
  uint32_t num_v2 = vec2->size();

  assert(num_v1 == num_v2);

  for (uint32_t i = 0; i < num_v1; ++i) {
    if (mapped_var1.find(i) == mapped_var1.end())
      unmapped_var1.push_back(i);
    if (mapped_var2.find(i) == mapped_var2.end())
      unmapped_var2.push_back(i);
  }

  while (unmapped_var1.size() > 0) {
    // assert(unmapped_var1.size() == unmapped_var2.size());
    uint32_t unmapped_head = unmapped_var1.front();
    std::set<uint32_t> s1 = {unmapped_head}, s2;

    uint32_t n = (*vec1)[unmapped_head];

    for (auto it = unmapped_var1.begin(); it != unmapped_var1.end(); ++it) {
      if ((*vec1)[*it] == n) {
        s1.insert(*it);
      }
    }
    for (auto it = unmapped_var1.begin();
         it != unmapped_var1.end();) { // erase all partial mapped vars in s1
      if (s1.find(*it) != s1.end()) {
        it = unmapped_var1.erase(it);
      } else {
        ++it;
      }
    }

    for (auto it = unmapped_var2.begin(); it != unmapped_var2.end(); ++it) {
      if ((*vec2)[*it] == n) {
        s2.insert(*it);
      }
    }
    for (auto it = unmapped_var2.begin();
         it != unmapped_var2.end();) { // erase all partial mapped vars in s2
      if (s2.find(*it) != s2.end()) {
        it = unmapped_var2.erase(it);
      } else {
        ++it;
      }
    }

    PartMap pm(s1, s2);
    unmap->push_back(pm);
  }
}

bool checkConsist(std::vector<varmap::PartMap> *m) {
  uint32_t i, mlen = m->size();

  if (mlen == 0) {
    return true;
  } else if (mlen == 1) {
    if ((*m)[0].first.size() != (*m)[0].second.size()) {
      return false;
    } else {
      return true;
    }
  } else {
    for (i = 0; i < mlen - 1; ++i) {
      if ((*m)[i].first.size() != (*m)[i].second.size()) {
        return false;
      }
      for (uint32_t j = i + 1; j < mlen; ++j) {
        std::vector<uint32_t> interset1, interset2;

        set_intersection((*m)[i].first.begin(), (*m)[i].first.end(),
                         (*m)[j].first.begin(), (*m)[j].first.end(),
                         back_inserter(interset1));
        set_intersection((*m)[i].second.begin(), (*m)[i].second.end(),
                         (*m)[j].second.begin(), (*m)[j].second.end(),
                         back_inserter(interset2));

        if (interset1.size() != interset2.size()) {
          return false;
        }
      }
    }

    // check the last element
    if ((*m)[mlen - 1].first.size() != (*m)[mlen - 1].second.size()) {
      return false;
    }
  }

  return true;
}

// reduce a partial mapping m by set intersection, return a map containing all
// mapped variables after reduce, no empty PartMap or one to one PartMap in m
std::map<uint32_t, uint32_t> reduce(std::vector<varmap::PartMap> *m) {
  std::map<uint32_t, uint32_t>
      mvar; // mapped variables identified in this reduce

  // erase empty PartMap in m
  for (uint32_t i = 0, max = m->size(); i < max;) {
    if ((*m)[i].first.empty()) { // second is also empty when first is empty
      m->erase(m->begin() + i);
      max = m->size();
    } else {
      ++i;
    }
  }

  for (int i = 0, mlen = m->size(); i < mlen - 1;) {
    for (int j = i + 1; j < mlen;) {
      std::set<uint32_t> interset1, interset2;

      set_intersection((*m)[i].first.begin(), (*m)[i].first.end(),
                       (*m)[j].first.begin(), (*m)[j].first.end(),
                       inserter(interset1, interset1.begin()));
      set_intersection((*m)[i].second.begin(), (*m)[i].second.end(),
                       (*m)[j].second.begin(), (*m)[j].second.end(),
                       inserter(interset2, interset2.begin()));

      if (interset1.size() != 0 && interset2.size() != 0) {

        // remove the common elements in m[i] and m[j]
        for (auto iter = interset1.begin(); iter != interset1.end(); ++iter) {
          (*m)[i].first.erase(*iter);
          (*m)[j].first.erase(*iter);
        }
        for (auto iter = interset2.begin(); iter != interset2.end(); ++iter) {
          (*m)[i].second.erase(*iter);
          (*m)[j].second.erase(*iter);
        }

        // push interset1 and interset2 m
        varmap::PartMap pm(interset1, interset2);
        m->push_back(pm);

        // erase empty PartMap in m
        for (uint32_t i = 0, max = m->size(); i < max;) {
          if ((*m)[i]
                  .first.empty()) { // second is also empty when first is empty
            m->erase(m->begin() + i);
            max = m->size();
          } else {
            ++i;
          }
        }

        // remove mapped variable from m and push it to mvar
        for (uint32_t i = 0, max = m->size(); i < max;) {
          if ((*m)[i].first.size() ==
              1) { // second size is also 1 when first is 1

            auto it1 = (*m)[i].first.begin();
            auto it2 = (*m)[i].second.begin();

            int mv1 = *it1;
            int mv2 = *it2;

            mvar.insert(std::pair<uint32_t, uint32_t>(*it1, *it2));
            m->erase(m->begin() + i);
            max = m->size();

            // remove the mapped variable from m
            for (uint32_t ii = 0; ii < max; ++ii) {
              std::set<uint32_t>::iterator it;
              if ((it = (*m)[ii].first.find(mv1)) != (*m)[ii].first.end())
                (*m)[ii].first.erase(it);
              if ((it = (*m)[ii].second.find(mv2)) != (*m)[ii].second.end())
                (*m)[ii].second.erase(it);
            }

          } else {
            ++i;
          }
        }

        // reset the iterator
        i = 0;
        j = 1;
        mlen = m->size();
        continue;
      } else {
        ++j;
      }
    }
    ++i; // put ++i ++j here so that continue can go around them
  }

  // erase empty PartMap in m
  for (uint32_t i = 0, max = m->size(); i < max;) {
    if ((*m)[i].first.empty()) { // second is also empty when first is empty
      m->erase(m->begin() + i);
      max = m->size();
    } else {
      ++i;
    }
  }

  return mvar;
}

void varmap::bitVarmap(std::shared_ptr<BitVector> formula1,
                       std::shared_ptr<BitVector> formula2, SEEngine *se1,
                       SEEngine *se2, const std::vector<int> &input_v1,
                       const std::vector<int> &input_v2,
                       std::vector<varmap::PartMap> unmap_input,
                       std::vector<varmap::PartMap> unmap_output,
                       std::map<uint32_t, uint32_t> map_input,
                       std::map<uint32_t, uint32_t> map_output,
                       std::list<varmap::FullMap> *result) {

  if (unmap_input.empty() && unmap_output.empty()) {
    varmap::FullMap fm(map_input, map_output);
    result->push_back(fm);
    return;
  }

  uint32_t num_input_value1 = input_v1.size();
  uint32_t num_input_value2 = input_v2.size();
  uint32_t num_input_bit_mapped = map_input.size();

  BitMatrix input_m1(REGISTER_SIZE * num_input_value1,
                     REGISTER_SIZE * num_input_value1);

  BitMatrix input_m2(REGISTER_SIZE * num_input_value2,
                     REGISTER_SIZE * num_input_value2);

  randomizeMappedVar(&input_m1, &input_m2, &map_input);
  setIdentityMatrix(&input_m1, &input_m2, &map_input);

  BitMatrix output_m1(REGISTER_SIZE * num_input_value1, REGISTER_SIZE);

  BitMatrix output_m2(REGISTER_SIZE * num_input_value2, REGISTER_SIZE);

  std::vector<std::shared_ptr<BitVector>> ouput_value1 = {formula1};
  std::vector<std::shared_ptr<BitVector>> ouput_value2 = {formula2};

  setOutputMatrix(&input_m1, formula1, input_v1, &ouput_value1, se1,
                  &output_m1);
  setOutputMatrix(&input_m2, formula2, input_v2, &ouput_value2, se2,
                  &output_m2);

  std::vector<uint32_t> rv1 = output_m1.getRowVector();
  std::vector<uint32_t> cv1 = output_m1.getColVector();
  std::vector<uint32_t> rv2 = output_m2.getRowVector();
  std::vector<uint32_t> cv2 = output_m2.getColVector();

  std::vector<uint32_t> rv1sort = rv1;
  std::sort(rv1sort.begin(), rv1sort.end());
  std::vector<uint32_t> cv1sort = cv1;
  std::sort(cv1sort.begin(), cv1sort.end());

  std::vector<uint32_t> rv2sort = rv2;
  std::sort(rv2sort.begin(), rv2sort.end());
  std::vector<uint32_t> cv2sort = cv2;
  std::sort(cv2sort.begin(), cv2sort.end());

  if (rv1sort != rv2sort || cv1sort != cv2sort)
    return;

  updateUnmap(&unmap_input, &rv1, &rv2, &map_input);
  updateUnmap(&unmap_output, &cv1, &cv2, &map_output);

  if (!checkConsist(&unmap_input))
    return;
  if (!checkConsist(&unmap_output))
    return;

  std::map<uint32_t, uint32_t> new_map_input = reduce(&unmap_input);
  std::map<uint32_t, uint32_t> new_map_output = reduce(&unmap_output);

  sort(unmap_input.begin(), unmap_input.end(),
       [](const PartMap &p1, const PartMap &p2) -> bool {
         return p1.first.size() < p2.first.size();
       });

  sort(unmap_output.begin(), unmap_output.end(),
       [](const PartMap &p1, const PartMap &p2) -> bool {
         return p1.first.size() < p2.first.size();
       });

  for (auto it = new_map_input.begin(); it != new_map_input.end(); ++it) {
    map_input[it->first] = it->second;
  }
  for (auto it = new_map_output.begin(); it != new_map_output.end(); ++it) {
    map_output[it->first] = it->second;
  }

  if (unmap_input.empty() && unmap_output.empty()) {
    FullMap fm(map_input, map_output);
    result->push_back(fm);
    return;
  } else if (!unmap_input.empty()) {
    PartMap p = unmap_input.front();
    auto i1 = p.first.begin();
    for (auto i2 = p.second.begin(); i2 != p.second.end(); ++i2) {
      std::set<uint32_t> s1 = {*i1};
      std::set<uint32_t> s2 = {*i2};
      PartMap pm(s1, s2);
      unmap_input.push_back(pm);
      bitVarmap(formula1, formula2, se1, se2, input_v1, input_v2, unmap_input,
                unmap_output, map_input, map_output, result);
    }
  } else {
    PartMap p = unmap_output.front();
    auto i1 = p.first.begin();
    for (auto i2 = p.second.begin(); i2 != p.second.end(); ++i2) {
      std::set<uint32_t> s1 = {*i1};
      std::set<uint32_t> s2 = {*i2};
      PartMap pm(s1, s2);
      unmap_output.push_back(pm);
      bitVarmap(formula1, formula2, se1, se2, input_v1, input_v2, unmap_input,
                unmap_output, map_input, map_output, result);
    }
  }
}

long long fac(uint32_t n) { return (n == 1 || n == 0) ? 1 : fac(n - 1) * n; }

std::vector<uint32_t> inline addVector(std::vector<uint32_t> preVec) {
  std::vector<uint32_t> newVec;
  for (auto it : preVec) {
    newVec.push_back(it + 1);
  }
  return newVec;
}

std::vector<uint32_t> mulVector(std::vector<uint32_t> preVec) {
  std::vector<uint32_t> newVec;
  for (auto it : preVec) {
    newVec.push_back(it * 0xf);
  }
  return newVec;
}

std::vector<uint32_t> extendVector(std::vector<uint32_t> preVec,
                                   uint32_t new_size) {
  std::vector<uint32_t> newVec = preVec;
  // uint32_t elem = preVec.back();
  while (newVec.size() < new_size) {
    newVec.push_back(0x1);
  }
  return newVec;
}

void inline addValue(std::map<int, uint32_t> &preVal) {
  for (auto it = preVal.begin(); it != preVal.end(); ++it) {
    preVal[it->first] = it->second + 1;
  }
}

void inline mulValue(std::map<int, uint32_t> &preVal) {
  for (auto it = preVal.begin(); it != preVal.end(); ++it) {
    preVal[it->first] = it->second * 0xf;
  }
}

bool varmap::fuzzFormula(const std::shared_ptr<BitVector> &v1,
                         const std::shared_ptr<BitVector> &v2, SEEngine *se1,
                         SEEngine *se2) {
  std::vector<int> inv1 = v1->getInputSymbolVector();

  std::vector<int> inv2 = v2->getInputSymbolVector();

  // skip variable mapping when the inputs have different number of bits
  if (inv1.size() != inv2.size()) {
    return false; // no mapping found
  }

  uint32_t inputSize = inv1.size();

  std::vector<uint32_t> sameInv(inputSize, 0xf);

  auto input_v1 = input2val(sameInv, inv1);
  auto input_v2 = input2val(sameInv, inv2);

  uint32_t output1 = se1->conexec(v1, input_v1);
  uint32_t output2 = se2->conexec(v2, input_v2);

  if (output1 != output2) {
    return false;
  }

  // 8! = 40320
  std::vector<std::vector<uint32_t>> input_test_set;

  uint32_t testSize = inputSize;
  if (inputSize > 5) {
    testSize = 5;
  }

  input_test_set.reserve(fac(testSize));
  std::vector<uint32_t> seed, ref_input;
  for (uint32_t i = 1; i < (testSize + 1); ++i) {
    seed.push_back(i);
    ref_input.push_back(i);
  }

  do {
    input_test_set.push_back(seed);
  } while (std::next_permutation(seed.begin(), seed.end()));

  bool add_flag = false;
  bool mul_flag = false;

  std::vector<uint32_t> input_vector2;
  std::vector<uint32_t> input1_add_test;
  std::vector<uint32_t> input2_add_test;

  seed = extendVector(seed, inputSize);
  input_v1 = input2val(seed, inv1);
  auto temp = input_v1;
  output1 = se1->conexec(v1, input_v1);
  for (const auto &it : input_test_set) {
    input_vector2 = extendVector(it, inputSize);

    input_v2 = input2val(input_vector2, inv2);
    output2 = se2->conexec(v2, input_v2);
    if (output2 == output1) {

      addValue(input_v1);
      addValue(input_v2);
      output1 = se1->conexec(v1, input_v1);
      output2 = se2->conexec(v2, input_v2);
      if (output1 == output2) {
        add_flag = true;
      }

      mulValue(input_v1);
      mulValue(input_v2);
      output1 = se1->conexec(v1, input_v1);
      output2 = se2->conexec(v2, input_v2);
      if (output1 == output2) {
        mul_flag = true;
      }

      if (mul_flag && add_flag) {
        return true;
      } else {
        mul_flag = false;
        add_flag = false;
        input_v1 = temp;
      }
    }
  }
  return false;
}

void varmap::randomizeMappedVar(varmap::BitMatrix *m1, varmap::BitMatrix *m2,
                                std::map<uint32_t, uint32_t> *mappedvar) {
  // srand(time(NULL));

  for (auto it = mappedvar->begin(); it != mappedvar->end(); ++it) {
    uint32_t input1 = it->first;
    uint32_t input2 = it->second;
    bool r = rand() % 2;

    // set column c1 in m1 and c2 in m2 to the same random number r (0/1)
    m1->setCol(input1, r);
    m2->setCol(input2, r);
  }
}

void varmap::setIdentityMatrix(BitMatrix *m1, BitMatrix *m2,
                               std::map<uint32_t, uint32_t> *mapped_var) {
  std::set<uint32_t> m1_mapped_bit_index;
  std::set<uint32_t> m2_mapped_bit_index;

  std::vector<uint32_t> m1_unmap_bit_index;
  std::vector<uint32_t> m2_unmap_bit_index;

  for (auto it = mapped_var->begin(); it != mapped_var->end(); ++it) {
    m1_mapped_bit_index.insert(it->first);
    m2_mapped_bit_index.insert(it->second);
  }

  uint32_t m1_col_size = m1->getColNum();
  uint32_t m1_row_size = m1->getRowNum();

  uint32_t m2_col_size = m2->getColNum();
  uint32_t m2_row_size = m2->getRowNum();
  assert(m1_col_size == m2_col_size);
  assert(m1_row_size == m2_row_size);

  for (uint32_t i = 0; i < m1_col_size; ++i) {
    if (m1_mapped_bit_index.find(i) == m1_mapped_bit_index.end()) {
      m1_unmap_bit_index.push_back(i);
    }
    if (m1_mapped_bit_index.find(i) == m1_mapped_bit_index.end()) {
      m2_unmap_bit_index.push_back(i);
    }
  }

  uint32_t num_unmap_bit = m1_unmap_bit_index.size();
  assert(m1_unmap_bit_index.size() == m2_unmap_bit_index.size());

  for (uint32_t i = 0; i < num_unmap_bit; ++i)
    for (uint32_t j = 0; j < num_unmap_bit; ++j) {
      if (i == j) {
        m1->setmValue(i, m1_unmap_bit_index[j], true);
        m2->setmValue(i, m1_unmap_bit_index[j], true);

      } else {
        m1->setmValue(i, m1_unmap_bit_index[j], false);
        m2->setmValue(i, m1_unmap_bit_index[j], false);
      }
    }
}

bool varmap::checkFormula(const std::shared_ptr<BitVector> &v1,
                          const std::shared_ptr<BitVector> &v2, SEEngine *se1,
                          SEEngine *se2) {

  const std::unique_ptr<Operation> &opr1 = v1->opr;
  const std::unique_ptr<Operation> &opr2 = v2->opr;
  ValueType type1 = v1->val_type;
  ValueType type2 = v2->val_type;
  if ((opr1 == nullptr) && (opr2 == nullptr)) {
    if (type1 != type2)
      return false;
    if (type1 == ValueType::SYMBOL)
      return true;
    if (type1 == ValueType::CONCRETE)
      return (v1->concrete_value) == (v2->concrete_value);
  }

  if ((opr1 != nullptr) && (opr2 != nullptr)) {
    std::shared_ptr<BitVector> l1 = nullptr, l2 = nullptr, l3 = nullptr,
                               r1 = nullptr, r2 = nullptr, r3 = nullptr;
    uint32_t num_l = 0;
    uint32_t num_r = 0;
    if (opr1->opty != opr2->opty)
      return varmap::varmapAndoutputCVC(se1, v1, se2, v2);
    if (opr1->val[0] != nullptr) {
      ++num_l;
      l1 = opr1->val[0];
    }
    if (opr1->val[1] != nullptr) {
      ++num_l;
      l2 = opr1->val[1];
    }
    if (opr1->val[2] != nullptr) {
      ++num_l;
      l3 = opr1->val[2];
    }
    if (opr2->val[0] != nullptr) {
      ++num_r;
      r1 = opr2->val[0];
    }
    if (opr2->val[1] != nullptr) {
      ++num_r;
      r2 = opr2->val[1];
    }
    if (opr2->val[2] != nullptr) {
      ++num_r;
      r3 = opr2->val[2];
    }
    if (num_l != num_r)
      return false;

    if (num_r == 3)
      return (checkFormula(l1, r1, se1, se2) &&
              checkFormula(l2, r2, se1, se2) && checkFormula(l3, r3, se1, se2));
    if (num_r == 2)
      return (checkFormula(l1, r1, se1, se2) && checkFormula(l2, r2, se1, se2));
    if (num_r == 1)
      return checkFormula(l1, r1, se1, se2);
  }

  return false;
}

bool varmap::checkSyn(const std::shared_ptr<BitVector> &v1,
                      const std::shared_ptr<BitVector> &v2, SEEngine *se1,
                      SEEngine *se2) {

  const std::unique_ptr<Operation> &opr1 = v1->opr;
  const std::unique_ptr<Operation> &opr2 = v2->opr;
  ValueType type1 = v1->val_type;
  ValueType type2 = v2->val_type;
  if ((opr1 == nullptr) && (opr2 == nullptr)) {
    if (type1 != type2)
      return false;
    if (type1 == ValueType::SYMBOL)
      return true;
    if (type1 == ValueType::CONCRETE)
      return (v1->concrete_value) == (v2->concrete_value);
  }

  if ((opr1 != nullptr) && (opr2 != nullptr)) {
    std::shared_ptr<BitVector> l1 = nullptr, l2 = nullptr, l3 = nullptr,
                               r1 = nullptr, r2 = nullptr, r3 = nullptr;
    uint32_t num_l = 0;
    uint32_t num_r = 0;
    if (opr1->opty != opr2->opty)
      return false;
    if (opr1->val[0] != nullptr) {
      ++num_l;
      l1 = opr1->val[0];
    }
    if (opr1->val[1] != nullptr) {
      ++num_l;
      l2 = opr1->val[1];
    }
    if (opr1->val[2] != nullptr) {
      ++num_l;
      l3 = opr1->val[2];
    }
    if (opr2->val[0] != nullptr) {
      ++num_r;
      r1 = opr2->val[0];
    }
    if (opr2->val[1] != nullptr) {
      ++num_r;
      r2 = opr2->val[1];
    }
    if (opr2->val[2] != nullptr) {
      ++num_r;
      r3 = opr2->val[2];
    }
    if (num_l != num_r)
      return false;

    if (num_r == 3)
      return (checkSyn(l1, r1, se1, se2) && checkSyn(l2, r2, se1, se2) &&
              checkSyn(l3, r3, se1, se2));
    if (num_r == 2)
      return (checkSyn(l1, r1, se1, se2) && checkSyn(l2, r2, se1, se2));
    if (num_r == 1)
      return checkSyn(l1, r1, se1, se2);
  }

  return false;
}

std::map<int, uint32_t> varmap::input2val(std::vector<uint32_t> input,
                                          const std::vector<int> &vv) {
  std::map<int, uint32_t> input_v;
  assert(input.size() == vv.size());

  for (uint32_t i = 0; i < input.size(); ++i) {
    input_v.insert(std::pair<int, uint32_t>((vv)[i], input[i]));
  }

  return input_v;
}

std::map<int, int>
varmap::matchFunction(SEEngine *se1, SEEngine *se2,
                      std::vector<std::shared_ptr<BitVector>> v1s,
                      std::vector<std::shared_ptr<BitVector>> v2s) {

  std::map<int, int> result;

  for (uint32_t i = 0; i < v1s.size(); ++i) {
    std::shared_ptr<BitVector> v1 = v1s[i];
    auto size_v1 = v1->getInputSymbolVector().size();
    if (size_v1 < MIN_NUM_INPUT) { // We think one crypto formula must have two
                                   // inputs symbols
      continue;
    }
    for (uint32_t j = 0; j < v2s.size(); ++j) {
      std::shared_ptr<BitVector> v2 = v2s[j];
      auto size_v2 = v2->getInputSymbolVector().size();
      if ((size_v2 < MIN_NUM_INPUT) || (size_v1 != size_v2)) {
        continue;
      }

      if (varmap::fuzzFormula(v1, v2, se1, se2)) {
        result[i] = j;
        break;
      }

      if (varmap::checkFormula(v1, v2, se1, se2)) {
        result[i] = j;
        break;
      }
    }
  }
  return result;
}

} // namespace tana