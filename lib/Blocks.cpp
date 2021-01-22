/*************************************************************************
	> File Name: Blocks.cpp
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#include "Blocks.hpp"
#include "ins_types.hpp"


namespace tana {

    StaticBlock::StaticBlock(uint32_t n_addr, uint32_t n_end_addr, uint32_t n_inputs, uint32_t n_ninstr, \
              uint32_t n_outputs, uint32_t n_size, uint32_t n_trace) : addr(n_addr), end_addr(n_end_addr), \
                                                                       inputs(n_inputs), \
                                                                       ninstr(n_ninstr), outputs(n_outputs), \
                                                                       size(n_size), traced(n_trace) {}

    bool StaticBlock::init(std::vector<std::unique_ptr<Inst_Base>> &fun_inst) {
        bool copy_flag = false;
        bool finish_flag = false;
        for (auto &ins : fun_inst) {
            auto inst_addr = ins->addrn;
            if (inst_addr == addr) {
                copy_flag = true;
            }

            if (!copy_flag) {
                continue;
            }

            if (inst_addr >= end_addr) {
                if (inst_addr == end_addr) {
                    finish_flag = true;
                }

                break;
            }

            inst_list.push_back(std::move(ins));


        }
        auto start_pos = fun_inst.begin();
        while (start_pos != fun_inst.end()) {
            if (*start_pos == nullptr) {
                fun_inst.erase(start_pos);
            } else
                ++start_pos;
        }

        return finish_flag;
    }

    void StaticBlock::print() const {
        std::cout << "Block: " << this->id << "\n";
        std::cout << "Start Address: " << std::hex << addr << " End Address: " << end_addr << std::dec << "\n";
        std::cout << "Block Size: " << size << std::endl;
        for (auto const &inst : inst_list) {
            std::cout << *inst << "\n";
        }

        std::cout << "\n";
    }


    uint32_t DynamicBlock::block_seed_id = 0;

    DynamicBlock::DynamicBlock(uint32_t start_inst_index, uint32_t end_inst_index,
                               std::shared_ptr<std::vector<std::unique_ptr<Inst_Base>>> m_ptr):
                                                                                     m_end_inst_index(end_inst_index),
                                                                                     m_start_inst_index(start_inst_index),
                                                                                     m_inst_list_ptr(m_ptr)
    {
        this->block_id = ++block_seed_id;
    }

    bool DynamicBlock::operator==(const tana::DynamicBlock &block)
    {
        if(size() != block.size())
        {
            return false;
        }
        uint32_t inst_size = size();
        uint32_t inst_index = 0;

        while(inst_index < inst_size)
        {
            auto &inst1 = (*m_inst_list_ptr)[m_start_inst_index + inst_index]->instruction_id;
            auto &inst2 = (*(block.m_inst_list_ptr))[block.m_start_inst_index + inst_index]->instruction_id;
            if(inst1 != inst2)
            {
                return false;
            }
            ++inst_index;
        }
        return true;

    }

    bool DynamicBlock::operator!=(const DynamicBlock &block) {
        return !((*this) == block);
    }

    uint32_t DynamicBlock::size() const {
        return m_end_inst_index - m_start_inst_index;
    }


}