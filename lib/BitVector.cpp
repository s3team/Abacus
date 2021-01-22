/*************************************************************************
	> File Name: BitVector.cpp
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#include <sstream>
#include <bitset>
#include <string>
#include <set>
#include <queue>
#include <cstdint>
#include <climits>
#include <utility>
#include "BitVector.hpp"


namespace tana {

    std::ostream &operator<<(std::ostream &os, BVOper bvop) {
        switch (bvop) {
            case BVOper::bvadd:
                return os << "bvadd";

            case BVOper::bvsub:
                return os << "bvsub";

            case BVOper::bvimul:
                return os << "bvimul";

            case BVOper::bvshld:
                return os << "bvshld";

            case BVOper::bvshrd:
                return os << "bvshrd";

            case BVOper::bvxor:
                return os << "bvxor";

            case BVOper::bvxor3:
                return os << "bvxor3";

            case BVOper::bvand:
                return os << "bvand";

            case BVOper::bvand3:
                return os << "bvand3";

            case BVOper::bvor3:
                return os << "bvor3";

            case BVOper::bvor:
                return os << "bvor";

            case BVOper::bvshl:
                return os << "bvshl";

            case BVOper::bvshr:
                return os << "bvshr";

            case BVOper::bvsar:
                return os << "bvsar";

            case BVOper::bvneg:
                return os << "bvneg";

            case BVOper::bvnot:
                return os << "bvnot";

            case BVOper::bvrol:
                return os << "bvrol";

            case BVOper::bvror:
                return os << "bvror";

            case BVOper::bvquo:
                return os << "bvquo";

            case BVOper::bvrem:
                return os << "bvrem";

            case BVOper::equal:
                return os << "equal";

            case BVOper::noequal:
                return os << "nonequal";

            case BVOper::greater:
                return os << "greater";

            case BVOper::less:
                return os << "less";

            case BVOper::bvzeroext:
                return os << "bvzeroext";

            case BVOper::bvextract:
                return os << "bvextract";

            case BVOper::bvconcat:
                return os << "bvconcat";

            case BVOper::bvsignext :
                return os << "bvsignext";

            case BVOper::bvbit:
                return os << "bvbit";

            case BVOper::bvbitnot:
                return os << "!";

            case BVOper::bvbsf:
                return os << "bsf";

            case BVOper::bvmul32_h:
                return os << "bvmul32_h";

            case BVOper::bvmul32_l:
                return os << "bvmul32_l";

            case BVOper::bvmul:
                return os << "bvmul";

            case BVOper::bvimul32_h:
                return os << "bvimul32_h";

            case BVOper::bvimul32_l:
                return os << "bvimul32_l";

            case BVOper::bvimul16_h:
                return os << "bvimul16_h";

            case BVOper::bvimul16_l:
                return os << "bvimul16_l";

            case BVOper::bvimul8_h:
                return os << "bvimul8_h";

            case BVOper::bvimul8_l:
                return os << "bvimul8_l";

            case BVOper::bvidiv32_quo:
                return os << "bvidiv32_quo";

            case BVOper::bvidiv32_rem:
                return os << "bvidiv32_rem";

            case BVOper::bvdiv32_quo:
                return os << "bvdiv32_quo";

            case BVOper::bvdiv32_rem:
                return os << "bvdiv32_rem";

                //omit default case to triger compiler warning for missing cases
        };

        return os << static_cast<uint32_t>(bvop);
    }

    Operation::Operation(BVOper opt, std::shared_ptr<BitVector> v1) {
        opty = opt;
        val[0] = std::move(v1);
        val[1] = nullptr;
        val[2] = nullptr;
    }

    Operation::Operation(BVOper opt, std::shared_ptr<BitVector> v1, std::shared_ptr<BitVector> v2) {
        opty = opt;
        val[0] = std::move(v1);
        val[1] = std::move(v2);
        val[2] = nullptr;
    }

    Operation::Operation(BVOper opt, std::shared_ptr<BitVector> v1, std::shared_ptr<BitVector> v2,
                         std::shared_ptr<BitVector> v3) {
        opty = opt;
        val[0] = std::move(v1);
        val[1] = std::move(v2);
        val[2] = std::move(v3);
    }

    int BitVector::idseed = 0;

    BitVector::BitVector(ValueType vty, const std::string &s_info) : opr(nullptr), val_type(vty), info(s_info),
                                                              concrete_value(0),
                                                              formula_cache_concrete_value(0),
                                                              flag_formula_cache_concrete(false){
        id = ++idseed;
    }

    BitVector::BitVector(ValueType vty, uint32_t con, bool Imm2SymState) : opr(nullptr), concrete_value(con),
                                                                           formula_cache_concrete_value(0),
                                                                           flag_formula_cache_concrete(false) {
        id = ++idseed;
        val_type = Imm2SymState ? ValueType::SYMBOL : vty;
    }

    BitVector::BitVector(ValueType vty, uint32_t con, bool Imm2SymState, uint32_t size) : opr(nullptr),
                                                                                          concrete_value(con),
                                                                                          low_bit(1), high_bit(size),
                                                                                          formula_cache_concrete_value(0),
                                                                                          flag_formula_cache_concrete(false){
        id = ++idseed;
        val_type = Imm2SymState ? ValueType::SYMBOL : vty;
    }

    BitVector::BitVector(ValueType vty, uint32_t con) : opr(nullptr), val_type(vty), concrete_value(con),
                                                        formula_cache_concrete_value(0),
                                                        flag_formula_cache_concrete(false){
        id = ++idseed;
    }

    BitVector::BitVector(ValueType vty, std::unique_ptr<Operation> oper) : concrete_value(0), val_type(vty),
                                                                           formula_cache_concrete_value(0),
                                                                           flag_formula_cache_concrete(false){
        id = ++idseed;
        opr = std::move(oper);
    }

    BitVector::BitVector(ValueType vty, const std::string &symbol_info, uint32_t size) : concrete_value(0), val_type(vty),
                                                                                  info(symbol_info),
                                                                                  low_bit(1), high_bit(size),
                                                                                  formula_cache_concrete_value(0),
                                                                                  flag_formula_cache_concrete(false){
        id = ++idseed;
        assert(vty == ValueType::SYMBOL);
    }

    std::set<int> BitVector::getInputSymbolSet() {
        std::queue<std::shared_ptr<BitVector> > que;
        std::set<int> input_set;


        if (this->opr == nullptr) {
            if (this->val_type == ValueType::SYMBOL) {
                input_set.insert(this->id);
            }
            return input_set;
        }

        for (int i = 0; i < 3; ++i) {
            const std::unique_ptr<Operation> &op = this->opr;
            if (op->val[i] != nullptr) que.push(op->val[i]);

        }

        while (!que.empty()) {
            std::shared_ptr<BitVector> v = que.front();
            const std::unique_ptr<Operation> &op = v->opr;
            que.pop();

            if (op == nullptr) {
                if (v->val_type == ValueType::SYMBOL)
                    input_set.insert(v->id);
            } else {
                for (int i = 0; i < 3; ++i) {
                    if (op->val[i] != nullptr) que.push(op->val[i]);
                }
            }
        }
        return input_set;
    }


    // Return the ID of each input symbol
    std::vector<int>
    BitVector::getInputSymbolVector() {
        std::set<int> input_set = this->getInputSymbolSet();

        std::vector<int> input_vector;
        for (auto it = input_set.begin(); it != input_set.end(); ++it) {
            input_vector.push_back(*it);
        }
        return input_vector;
    }


    uint32_t BitVector::arithmeticRightShift(uint32_t op1, uint32_t op2) {
        int32_t temp1 = op1;
        int32_t temp2 = op2;
        int32_t result = temp1 >> temp2;
        return uint32_t(result);
    }

    uint32_t BitVector::rol32(uint32_t op1, uint32_t op2) {
        const unsigned int mask = (CHAR_BIT * sizeof(op1) - 1);  // assumes width is a power of 2.
        // assert ( (c<=mask) &&"rotate by type width or more");
        op2 &= mask;
        return (op1 << op2) | (op1 >> ((-op2) & mask));

    }

    uint32_t BitVector::ror32(uint32_t op1, uint32_t op2) {
        const unsigned int mask = (CHAR_BIT * sizeof(op1) - 1);

        // assert ( (c<=mask) &&"rotate by type width or more");
        op2 &= mask;
        return (op1 >> op2) | (op1 << ((-op2) & mask));

    }

    uint32_t BitVector::shld32(uint32_t op1, uint32_t op2, uint32_t op3) {
        uint32_t cnt;
        uint32_t res;
        assert(op3 < REGISTER_SIZE);
        cnt = op3 % REGISTER_SIZE;
        res = (op1 << cnt) | (op2 >> (REGISTER_SIZE - cnt));
        return res;
    }

    uint32_t BitVector::shrd32(uint32_t op1, uint32_t op2, uint32_t op3) {
        uint32_t cnt;
        uint32_t res;
        assert(op3 < REGISTER_SIZE);
        cnt = op3 % REGISTER_SIZE;
        res = (op1 >> cnt) | (op2 << (REGISTER_SIZE - cnt));
        return res;
    }

    uint32_t BitVector::extract(uint32_t op1, uint32_t high, uint32_t low) {
        assert(high > low);
        //assert(high <= REGISTER_SIZE);
        assert(low > 0);
        uint32_t tmp = op1 << (REGISTER_SIZE - high);
        uint32_t res = tmp >> (REGISTER_SIZE - high + low - 1);
        return res;

    }


    uint32_t BitVector::bsf(uint32_t op)
    {
        if(op == 0)
            return 0;

        std::bitset<REGISTER_SIZE> bit(op);
        int index = 0;
        for(int i = REGISTER_SIZE-1; i>=0; --i)
        {
            if(bit[i])
            {
                return index;
            }
            ++index;
        }

        return 0;
    }

    uint32_t BitVector::bvimul(uint32_t op1, uint32_t op2)
    {
        int32_t t_op1, t_op2;
        int64_t res;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);

        res = (t_op1 * t_op2);

        return static_cast<uint32_t >(res);
    }

    uint32_t BitVector::bvimul32_l(uint32_t op1, uint32_t op2)
    {
        int32_t t_op1, t_op2;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);

        int64_t w_op1, w_op2, w_res;
        w_op1 = t_op1;
        w_op2 = t_op2;
        w_res = w_op1 * w_op2;

        auto u_res = static_cast<uint64_t >(w_res);

        return static_cast<uint32_t >(u_res);
    }

    uint32_t BitVector::bvimul32_h(uint32_t op1, uint32_t op2)
    {

        int32_t t_op1, t_op2;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);

        int64_t w_op1, w_op2, w_res;
        w_op1 = t_op1;
        w_op2 = t_op2;
        w_res = w_op1 * w_op2;

        auto u_res = static_cast<uint64_t >(w_res);
        u_res = u_res >> 32u;

        return static_cast<uint32_t >(u_res);
    }

    uint32_t BitVector::bvimul16_l(uint32_t op1, uint32_t op2)
    {
        int32_t t_op1, t_op2;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);
        int32_t res;
        res = t_op1 * t_op2;

        auto l_res = static_cast<int16_t >(res);
        return l_res;
    }

    uint32_t BitVector::bvimul16_h(uint32_t op1, uint32_t op2)
    {
        int32_t t_op1, t_op2;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);
        int32_t res;
        res = t_op1 * t_op2;
        uint32_t u_res = static_cast<uint32_t >(res);
        auto l_res = static_cast<int16_t >(u_res >> 16u);

        return l_res;
    }

    uint32_t BitVector::bvimul8_l(uint32_t op1, uint32_t op2)
    {
        int32_t t_op1, t_op2;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);
        int32_t res;
        res = t_op1 * t_op2;

        auto l_res = static_cast<int8_t >(res);
        return l_res;
    }

    uint32_t BitVector::bvimul8_h(uint32_t op1, uint32_t op2)
    {
        int32_t t_op1, t_op2;
        t_op1 = static_cast<int32_t >(op1);
        t_op2 = static_cast<int32_t >(op2);
        int32_t res;
        res = t_op1 * t_op2;
        uint32_t u_res = static_cast<int32_t >(res);

        auto l_res = static_cast<int8_t >(u_res >> 8u);
        return l_res;

    }




    uint32_t BitVector::bvmul32_l(uint32_t op1, uint32_t op2)
    {
        uint64_t t_op1 = op1;
        uint64_t t_op2 = op2;
        uint64_t res = t_op1 * t_op2;
        return static_cast<uint32_t>(res);
    }

    uint32_t BitVector::bvmul32_h(uint32_t op1, uint32_t op2)
    {
        uint64_t t_op1 = op1;
        uint64_t t_op2 = op2;
        uint64_t res = t_op1 * t_op2;
        uint64_t res_h = res >> 32u;
        return static_cast<uint32_t>(res_h);
    }

    uint32_t BitVector::bvmul16_8(uint32_t op1, uint32_t op2)
    {
        return op1*op2;
    }



    uint32_t BitVector::concat(uint32_t op1, uint32_t op2, uint32_t op1_size, uint32_t op2_size)
    {
        const static uint32_t op_mask[] = {0x1u,        0x3u,       0x7u,        0xFu,
                                           0x1Fu,       0x3Fu,      0x7Fu,       0xFFu,
                                           0x1FFu,      0x3FFu,     0x7FFu,      0xFFFu,
                                           0x1FFFu,     0x3FFFu,    0x7FFFu,     0xFFFFu,
                                           0x1FFFFu,    0x3FFFFu,   0x7FFFFu,    0xFFFFFu,
                                           0x1FFFFFu,   0x3FFFFFu,  0x7FFFFFu,   0xFFFFFFu,
                                           0x1FFFFFFu,  0x3FFFFFFu, 0x7FFFFFFu,  0xFFFFFFFu,
                                           0x1FFFFFFFu, 0x3FFFFFFFu,0x7FFFFFFFu, 0xFFFFFFFFu};
        uint32_t op1_mask = op1 & op_mask[op1_size];
        uint32_t op2_mask = op2 & op_mask[op2_size];

        return (op1_mask << op2_size) + op2_mask;
    }


    uint32_t BitVector::bvidiv32_quo(uint32_t edx, uint32_t eax, uint32_t op2)
    {
        uint64_t edx_w = edx;
        uint64_t eax_w = eax;

        uint64_t edx_eax = (edx_w<<REGISTER_SIZE) + eax_w;

        int64_t edx_eax_s = edx_eax;
        int64_t op = op2;

        int64_t quo = edx_eax_s / op;

        return static_cast<uint32_t >(quo);
    }

    uint32_t BitVector::bvidiv32_rem(uint32_t edx, uint32_t eax, uint32_t op2)
    {
        uint64_t edx_w = edx;
        uint64_t eax_w = eax;

        uint64_t edx_eax = (edx_w<<REGISTER_SIZE) + eax_w;

        int64_t edx_eax_s = edx_eax;
        int64_t op = op2;

        int64_t rem = edx_eax_s % op;

        return static_cast<uint32_t >(rem);

    }

    uint32_t BitVector::bvdiv32_quo(uint32_t edx, uint32_t eax, uint32_t div)
    {
        uint64_t edx_w = edx;
        uint64_t eax_w = eax;

        uint64_t edx_eax = (edx_w<<REGISTER_SIZE) + eax_w;
        uint64_t op = div;

        uint64_t quo = edx_eax / op;

        return static_cast<uint32_t >(quo);
    }

    uint32_t BitVector::bvdiv32_rem(uint32_t edx, uint32_t eax, uint32_t div)
    {
        uint64_t edx_w = edx;
        uint64_t eax_w = eax;

        uint64_t edx_eax = (edx_w<<REGISTER_SIZE) + eax_w;
        uint64_t op = div;

        uint64_t rem = edx_eax % op;

        return static_cast<uint32_t >(rem);
    }


    bool BitVector::bit(uint32_t op0, uint32_t op1) {
        std::bitset<REGISTER_SIZE> bit1(op0);
        uint32_t offset = op1 % REGISTER_SIZE;
        return bit1[offset];
    }

    uint32_t BitVector::zeroext(uint32_t op1) {
        return op1;
    }

    uint32_t BitVector::signext(uint32_t op1, uint32_t origin_size, uint32_t new_size) {
        std::bitset<REGISTER_SIZE> bit1(op1);
        std::string res;
        std::string bit1_str = bit1.to_string();
        auto bit1_str_size = bit1_str.size();
        auto sign_flag = bit1_str[bit1_str_size - origin_size];
        std::string str_ext(new_size - origin_size, sign_flag);
        res = str_ext + bit1_str.substr(bit1_str_size - origin_size, bit1_str_size);
        std::bitset<REGISTER_SIZE> res_bit(res);
        auto res_u = static_cast<uint32_t > (res_bit.to_ulong());
        return res_u;

    }

    bool BitVector::isSymbol() const {
        return val_type == ValueType::SYMBOL;

    }

    std::string BitVector::print() const {
        std::stringstream ss;
        if (val_type == ValueType::SYMBOL) {
            if (!info.empty()) {
                ss << "(SYM_ID" << id;
                ss << ", " << info << ")";
            } else {
                ss << "SYM_ID" << id;
            }
        }
        if (val_type == ValueType::CONCRETE) {
            if (info.empty()) {
                ss << "0x" << std::hex << concrete_value << std::dec;
            } else {
                ss << "(0x" << std::hex << concrete_value << std::dec;
                ss << ", " << info << ")";
            }
        }
        return ss.str();
    }

    uint32_t BitVector::size() const {
        return high_bit - low_bit + 1;
    }


    bool BitVector::operator==(const BitVector &v1) {
        const std::unique_ptr<Operation> &opr1 = opr;
        const std::unique_ptr<Operation> &opr2 = v1.opr;
        if ((opr == nullptr) && (v1.opr == nullptr)) {
            if (val_type != v1.val_type)
                return false;
            if (val_type == ValueType::SYMBOL)
                return (id == v1.id);
            if (val_type == ValueType::CONCRETE)
                return (concrete_value) == (v1.concrete_value);
        }
        if ((opr1 != nullptr) && (opr2 != nullptr)) {
            std::shared_ptr<BitVector> l1 = nullptr, l2 = nullptr, l3 = nullptr, r1 = nullptr, r2 = nullptr, r3 = nullptr;
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
            if (num_l == 1)
                return (*l1) == (*r1);
            if (num_l == 2)
                return ((*l1) == (*r1)) && ((*l2) == (*r2));
            if (num_l == 3)
                return ((*l1) == (*r1)) && ((*l2) == (*r2)) && ((*l3) == (*r3));
        }
        return false;
    }

    uint32_t BitVector::printV(std::stringstream &ss) const {
        uint32_t num = 0;
        printV(ss, num);
        return num;
    }

    std::ostream &operator<<(std::ostream &os, const BitVector &dt) {
        std::stringstream ss;
        dt.printV(ss);
        os << ss.str();
        return os;

    }


    void BitVector::printV(std::stringstream &ss, uint32_t &length) const {
        const std::unique_ptr<Operation> &op = this->opr;
        ++length;
        if (op == nullptr) {
            ss << this->print();
            return;
        }
        int num_opr = 0;
        if (op->val[0] != nullptr) ++num_opr;
        if (op->val[1] != nullptr) ++num_opr;
        if (op->val[2] != nullptr) ++num_opr;


        if (num_opr == 1) {
            ss << op->opty;
            if (op->opty == BVOper::bvextract) {
                ss << "(";
                (op->val[0])->printV(ss, length);
                ss << "," << this->low_bit << "," << this->high_bit << ")";
            } else {
                ss << "(";
                (op->val[0])->printV(ss, length);
                ss << ")";
            }

        }
        if (num_opr == 2) {
            ss << op->opty;
            ss << "(";
            (op->val[0])->printV(ss, length);
            ss << ",";
            (op->val[1])->printV(ss, length);
            ss << ")";
        }
        if (num_opr == 3) {
            ss << op->opty;
            ss << "(";
            (op->val[0])->printV(ss, length);
            ss << ",";
            (op->val[1])->printV(ss, length);
            ss << ",";
            (op->val[2])->printV(ss, length);
            ss << ")";
        }
    }

    void BitVector::symbol_num_internal(const std::shared_ptr<BitVector> &v, std::set<int> &input) const {
        const std::unique_ptr<Operation> &op = v->opr;
        if (op == nullptr) {
            if (v->val_type == ValueType::CONCRETE)
                return;
            else {
                input.insert(v->id);
                return;
            }
        }

        if (op->val[0] != nullptr) symbol_num_internal(op->val[0], input);
        if (op->val[1] != nullptr) symbol_num_internal(op->val[1], input);
        if (op->val[2] != nullptr) symbol_num_internal(op->val[2], input);

    }

    uint32_t BitVector::symbol_num() {
        if (symbol_num_cache != -1)
            return symbol_num_cache;

        std::set<int> input_num;
        const std::unique_ptr<Operation> &op = this->opr;

        if (op == nullptr) {
            if (this->val_type == ValueType::SYMBOL)
                input_num.insert(this->id);
            return input_num.size();
        }
        if (op->val[0] != nullptr) symbol_num_internal(op->val[0], input_num);
        if (op->val[1] != nullptr) symbol_num_internal(op->val[1], input_num);
        if (op->val[2] != nullptr) symbol_num_internal(op->val[2], input_num);

        symbol_num_cache = input_num.size();
        return symbol_num_cache;
    }


    std::shared_ptr<BitVector>
    buildop1(BVOper opty, std::shared_ptr<BitVector> v1) {
        std::unique_ptr<Operation> oper = std::make_unique<Operation>(opty, v1);
        std::shared_ptr<BitVector> result;
        if (v1->isSymbol())
            result = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
        else {
            result = std::make_shared<BitVector>(ValueType::CONCRETE, std::move(oper));
        }
        result->high_bit = v1->high_bit;
        result->low_bit = v1->low_bit;
        return result;
    }

    std::shared_ptr<BitVector>
    buildop2(BVOper opty, std::shared_ptr<BitVector> v1, std::shared_ptr<BitVector> v2) {
        std::unique_ptr<Operation> oper = std::make_unique<Operation>(opty, v1, v2);
        std::shared_ptr<BitVector> result;
        //assert(v1->size() == v2->size()|| !v2->isSymbol() || !v1->isSymbol());
        if (v1->isSymbol() || v2->isSymbol())
            result = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
        else
            result = std::make_shared<BitVector>(ValueType::CONCRETE, std::move(oper));
        result->low_bit = v1->low_bit;
        result->high_bit = v1->high_bit;
        return result;
    }

    std::shared_ptr<BitVector>
    buildop2(BVOper opty, std::shared_ptr<BitVector> v1, uint32_t con) {
        auto BV = std::make_shared<BitVector>(ValueType::CONCRETE, con);
        return buildop2(opty, v1, BV);
    }

    std::shared_ptr<BitVector>
    buildop3(BVOper opty, std::shared_ptr<BitVector> v1, std::shared_ptr<BitVector> v2, \
             std::shared_ptr<BitVector> v3) {
        std::unique_ptr<Operation> oper = std::make_unique<Operation>(opty, v1, v2, v3);
        std::shared_ptr<BitVector> result;

        if (v1->isSymbol() || v2->isSymbol() || v3->isSymbol())
            result = std::make_shared<BitVector>(ValueType::SYMBOL, std::move(oper));
        else
            result = std::make_shared<BitVector>(ValueType::CONCRETE, std::move(oper));

        return result;
    }

    int BitVector::length()
    {
        if(formula_length_cache)
            return formula_length;

        const std::unique_ptr<Operation> &op = this->opr;

        int length_temp = 0;
        if (op == nullptr) {
            formula_length = 1;
            formula_length_cache = true;
            return formula_length;
        }

        if (op->val[0] != nullptr) length_temp += (op->val[0])->length();
        if (op->val[1] != nullptr) length_temp += (op->val[1])->length();
        if (op->val[2] != nullptr) length_temp += (op->val[2])->length();
        formula_length = length_temp;
        formula_length_cache = true;
        return formula_length;

    }

}