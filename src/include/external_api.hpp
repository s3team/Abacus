#include "BitVector.hpp"
#include "Blocks.hpp"
#include "Engine.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class EngineType { Static, Dynamic, QIF };

/*! \brief This function will parse the string into into the internal
 * instruction structure.
 *
 *  Parse the string into the internal instruction structure.
 *  e.g., "0x12341231    mov eax, eax"
 */
std::unique_ptr<tana::Inst_Base> ParseInst(const std::string &ins);

/*! \brief This function will parse the string into into the internal block
 *         structure.
 *
 *  Parse the string into block structure. A block is a series of
 *  instruction.
 */
tana::StaticBlock ParseStaticBlock(const std::string &block);

/*! \brief Create symbolic execution engine.
 *
 *
 *  Right now the library support three kinds of engines, Static, Dynamic and
 *  Dynamic with memory dump.
 */
tana::SEEngine *CreateEngine(EngineType type);

/*! \brief Symbolically execute each instruction.
 *
 *
 *  Instruction can be obtained from function ParseInst
 */
bool SymbolicOnIns(tana::Inst_Base *ins, tana::SEEngine *se);

/*! \brief Symbolically execute each basic block.
 *
 *
 *  Block can be obtained from function ParseBlock
 */
bool SymbolicOnBlock(const tana::StaticBlock &block, tana::SEEngine *se);

/*! \brief Get all the symbolic result from the symbolic execution engine.
 *
 *
 *  Right now the result will include all the formuala from eax, ebx, ecx
 *  edx register and memory.
 */
std::vector<std::shared_ptr<tana::BitVector>> GetResult(tana::SEEngine *se);

/*! \brief Check if two formulas are semantically equivalent or not.
 *
 *
 *  The function will also simplify the formula before checking if
 *  they are same or not.
 */
bool MatchFormula(const std::shared_ptr<tana::BitVector> &f1,
                  const std::shared_ptr<tana::BitVector> &f2,
                  tana::SEEngine *se1, tana::SEEngine *se2);
