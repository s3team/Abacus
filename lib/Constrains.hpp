/*************************************************************************
	> File Name: Constrains.h
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#pragma once

#include <tuple>
#include <memory>
#include <vector>
#include <map>
#include "BitVector.hpp"
#include "Function.hpp"
#include "CallStack.hpp"


namespace tana {


    class Constrain {
    private:
        uint32_t inst_index;
        std::shared_ptr<BitVector> r;
        std::vector<std::shared_ptr<CallLeakageSites>> callsites;
    public:
        Constrain() = default;

        Constrain(uint32_t, std::shared_ptr<BitVector> b, BVOper, uint32_t);

        Constrain(uint32_t, std::shared_ptr<BitVector> b1, BVOper, std::shared_ptr<BitVector> b2);

        void update(BVOper add_type, std::shared_ptr<BitVector> b, BVOper type, uint32_t num);

        friend std::ostream &operator<<(std::ostream &os, const Constrain &c);

        bool validate();

        bool validate(const std::map<int, uint32_t> &);

        uint32_t getNumSymbols();

        std::vector<int> getInputKeys();

        void updateCallSites(std::vector<std::shared_ptr<CallLeakageSites>> callsites);
    };

}