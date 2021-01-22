/*************************************************************************
	> File Name: Loop.h
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#pragma once

#include <list>
#include <unordered_map>
#include <deque>
#include "ins_types.hpp"
#include "ins_parser.hpp"

namespace tana {
    namespace loop {

        class Loop {
        private:
            bool isGoodLoop = false;
            std::vector<Inst_Base>::const_iterator loop_begin;
            std::vector<Inst_Base>::const_iterator loop_end;

            //Used for loop detection
            std::vector<Inst_Base>::const_iterator loop_current;
            size_t loopSize = 0;
        public:

            bool operator==(const Loop &loop);

            bool operator!=(const Loop &loop);

            bool operator<(const Loop &loop);

            bool operator>(const Loop &loop);

            Loop(std::vector<Inst_Base>::const_iterator loop_begin, std::vector<Inst_Base>::const_iterator loop_end);

            size_t getLoopLength() { return loopSize; }

            bool isCheckedFinish();
            bool checkLoopCurrent(std::vector<Inst_Base>::const_iterator inst_current);
            std::vector<Inst_Base>::const_iterator getStart(){ return loop_begin;}
            std::vector<Inst_Base>::const_iterator getEnd(){ return loop_end;}
        };

        class InstructionMap {
        private:
            std::unordered_map<tana_type::index, std::vector<tana_type::index>> instMap;

        public:

            std::list<Loop> findPotentialLoops(std::vector<Inst_Base>::const_iterator current,
                                               std::vector<Inst_Base>::const_iterator instruction_end);
            void updateMap(std::vector<Inst_Base>::const_iterator current);
        };

        std::list<Loop> findPotentialLoop(std::vector<Inst_Base>::const_iterator current,
                                          std::vector<Inst_Base>::const_iterator instruction_end,
                                          uint32_t history_length,
                                          uint32_t min_loop_length);

        std::vector<Loop> loopDetection(const std::vector<std::unique_ptr<Inst_Base>> &L, uint32_t id);

        std::vector<Loop> loopDetectionFast(std::vector<Inst_Base> *L, uint32_t id);

        void outPrintLoops(std::vector<Loop>, std::string file_name);
    }
}
