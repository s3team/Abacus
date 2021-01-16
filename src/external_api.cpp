
#include "external_api.hpp"
#include "DynSEEngine.hpp"
#include "Engine.hpp"
#include "StaticSEEngine.hpp"
#include "VarMap.hpp"
#include "error.hpp"
#include "ins_parser.hpp"

using namespace std;
using namespace tana;

#define ERROR(MESSAGE) tana::default_error_handler(__FILE__, __LINE__, MESSAGE)

StaticBlock ParseStaticBlock(const std::string &block) {
  return tana::StaticBlock(0, 0, 0, 0);
}

bool SymbolicOnIns(tana::Inst_Base *ins, tana::SEEngine *se) {
  return ins->symbolic_execution(se);
}

bool SymbolicOnBlock(const tana::StaticBlock &block, tana::SEEngine *se) {
  bool status = true;
  for (auto &it : block.inst_list) {
    auto ins = it.get();
    status = ins->symbolic_execution(se) ^ status;
  }

  return status;
}

SEEngine *CreateEngine(EngineType eng_type) {
  if (eng_type == EngineType::Static) {
    auto se = new StaticSEEngine(false);
    std::vector<std::unique_ptr<Inst_Base>> inst;
    se->initAllRegSymol(inst.begin(), inst.end());
    return se;
  }

  if (eng_type == EngineType::Dynamic) {
    return new DynSEEngine(false);
  }

  ERROR("illegal engine type");
  return nullptr;
}

vector<std::shared_ptr<BitVector>> GetResult(SEEngine *se) {
  return se->getAllOutput();
}

bool MatchFormula(const shared_ptr<BitVector> &f1,
                  const shared_ptr<BitVector> &f2, SEEngine *se1,
                  SEEngine *se2) {
  return varmap::checkFormula(f1, f2, se1, se2);
}

unique_ptr<Inst_Base> ParseInst(const std::string &ins) {
  static uint32_t ins_index = 0;
  unique_ptr<Inst_Base> inst = parseInst(ins, true, nullptr);
  inst->id = ins_index++;
  return inst;
}
