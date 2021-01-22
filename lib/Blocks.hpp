/*************************************************************************
	> File Name: Blocks.h
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "ins_types.hpp"

namespace tana {

    class StaticBlock {
    public:
        std::vector<std::unique_ptr<Inst_Base>> inst_list;
        uint32_t addr;
        uint32_t end_addr;
        uint32_t inputs;
        uint32_t ninstr;
        uint32_t outputs;
        uint32_t size;
        int id = 0;
        bool traced;

        StaticBlock(uint32_t n_addr, uint32_t n_end_addr, uint32_t n_inputs, uint32_t n_ninstr, \
              uint32_t n_outputs, uint32_t n_size, uint32_t trace);

        bool init(std::vector<std::unique_ptr<Inst_Base>> &fun_inst);

        void print() const;

    };

    class DynamicBlock {
    private:
        uint32_t block_id;
        std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> m_inst_list_ptr;

    public:
        static uint32_t block_seed_id;
        uint32_t m_start_inst_index;
        uint32_t m_end_inst_index;

        bool operator==(const DynamicBlock &block);
        bool operator!=(const DynamicBlock &block);
        bool operator<(const DynamicBlock &block);
        bool operator>(const DynamicBlock &block);

        void print() const;

        uint32_t size() const ;

        DynamicBlock(uint32_t start_inst_index, uint32_t end_inst_index,
                     std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> m);

    };

}