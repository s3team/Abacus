cmake_minimum_required(VERSION 3.10)
project(Abacus_Examples)

find_package(PkgConfig REQUIRED)

add_executable(TestBitVector
        test_bitvector.cpp)

add_executable(TestX86
        test_x86.cpp)

add_executable(TestConstrain
        test_constrain.cpp)

add_executable(TestBit
        test_bit.cpp)

add_executable(TestQIF
        test_qif.cpp)

add_executable(TestQIF1
        test_qif1.c)

add_executable(TestQIF2
        test_qif2.c)

add_executable(TestRandom
        test_random_num.cpp)

add_executable(TestDebugInfo
        test_debuginfo.cpp)

add_executable(TestDebugInfoStr
        test_debuginfo_str.cpp)

add_executable(TestBvMul
        test_bvmul.cpp)

add_executable(TestCMD
        test_cmd.cpp)

add_executable(TestELF
        test_elf.cpp)

add_executable(TestEval
        test_eval.cpp)

add_executable(TestELFSym
        test_elf_sym.cpp)

add_executable(TestHashFormula
        test_formula_hash.cpp)

target_link_libraries(TestHashFormula TraceLib)
target_link_libraries(TestConstrain TraceLib)
target_link_libraries(TestRandom TraceLib)
target_link_libraries(TestDebugInfo TraceLib)
target_link_libraries(TestDebugInfoStr TraceLib)
target_link_libraries(TestCMD TraceLib)
target_link_libraries(TestELFSym TraceLib)
target_link_libraries(TestBit TraceLib)
target_link_libraries(TestBvMul TraceLib)
target_link_libraries(TestEval TraceLib)
target_link_libraries(TestBitVector TraceLib)
target_link_libraries(TestX86 TraceLib)
target_link_libraries(TestELF TraceLib)
