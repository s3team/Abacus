#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include "ins_types.hpp"
#include "Register.hpp"
#include "Engine.hpp"
#include "BitVector.hpp"
#include "Constrains.hpp"
#include "CallStack.hpp"
#include "Function.hpp"

namespace tana {
    class BitVector;

    class Operation;


    class QIFSEEngine : public SEEngine {
    private:

        std::map<tana_type::T_ADDRESS, std::shared_ptr<BitVector> > m_memory;

        bool memory_find(uint32_t addr);

        std::map<std::string, std::shared_ptr<BitVector>> m_ctx;
        std::shared_ptr<BitVector> CF, OF, SF, ZF, AF, PF;
        std::vector<std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>> m_constrains;
        uint32_t eip;
        uint32_t mem_data;

        void checkOperand(const std::shared_ptr<Operand> &opr, Inst_Base *inst);

        std::shared_ptr<tana::Constrain> getMemoryAccessConstrain(std::shared_ptr<BitVector> mem_address_symbol, \
                                                                  std::string mem_address_concrete);

        std::unique_ptr<CallStack> stacks;
        std::vector<int> getInstSymbol(Inst_Base *inst);

        void updateStacks(Inst_Base *inst);
        std::shared_ptr<DynamicFunction> func;


    public:

        using SEEngine::SEEngine;


        void init(std::vector<std::unique_ptr<Inst_Base>>::iterator it1,
                  std::vector<std::unique_ptr<Inst_Base>>::iterator it2,
                  std::shared_ptr<DynamicFunction> function);

        void updateSecrets(const std::vector<std::tuple<uint32_t , uint32_t >> &key_symbol,
                           const std::vector<uint8_t> &key_value) override ;


        QIFSEEngine();

        static std::shared_ptr<BitVector> Extract(std::shared_ptr<BitVector> v, int low, int high);

        std::vector<std::shared_ptr<BitVector>> getAllOutput() override;

        std::shared_ptr<BitVector> readReg(x86::x86_reg reg) override;

        std::shared_ptr<BitVector> readReg(std::string reg) override;

        bool writeReg(x86::x86_reg reg, std::shared_ptr<BitVector> v) override;

        bool writeReg(std::string reg, std::shared_ptr<BitVector> v) override;

        std::shared_ptr<BitVector> readMem(std::string memory_address, tana_type::T_SIZE size) override;

        bool writeMem(std::string memory_address, tana_type::T_SIZE size, std::shared_ptr<BitVector> v) override;

        int run() override;

        void updateFlags(const std::string&, std::shared_ptr<BitVector>) override;

        void clearFlags(const std::string&) override;

        std::shared_ptr<tana::BitVector> getFlags(const std::string&) override;

        void updateCFConstrains(std::shared_ptr<Constrain> cons) override;  //Update the control-flow transfer constrain

        void updateDAConstrains(std::shared_ptr<Constrain> cons) override; //Update the data-access constrain

        void printMemory() override;

        void printConstrains();

        void reduceConstrains();

        void checkMemoryAccess(Inst_Base *inst);

        uint32_t eval_cache(const std::shared_ptr<BitVector> &v,
                      const std::map<int, uint32_t> &inmap);


        std::vector<std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>>
        getConstraints();

        std::map<int, uint32_t> return_key_value_map()
        {
            return key_value_map;
        }


    };

}