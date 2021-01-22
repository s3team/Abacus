cmake_minimum_required(VERSION 3.10)
project(taintTrace)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(JSON_BuildTests OFF CACHE INTERNAL "")

include_directories(lib)

include_directories(external)
add_subdirectory(external/nlohmann_json)


add_library(TraceLib
        lib/error.hpp
        lib/error.cpp
        lib/x86.cpp
        lib/x86.hpp
        lib/ins_parser.cpp
        lib/ins_parser.hpp
        lib/ins_types.hpp
        lib/ins_types.cpp
        lib/ins_semantics.hpp
        lib/ins_semantics.cpp
        lib/Engine.hpp
        lib/Engine.cpp
        lib/Register.cpp
        lib/Register.hpp
        lib/DynSEEngine.cpp
        lib/DynSEEngine.hpp
        lib/VarMap.hpp
        lib/VarMap.cpp
        lib/Tainter.cpp
        lib/Tainter.hpp
        lib/Loop.cpp
        lib/Loop.hpp
        lib/Function.cpp
        lib/Function.hpp
        lib/Blocks.cpp
        lib/Blocks.hpp
        lib/BitVector.hpp
        lib/BitVector.cpp
        lib/StaticSEEngine.cpp
        lib/StaticSEEngine.hpp
        lib/QIFSEEngine.hpp
        lib/QIFSEEngine.cpp
        lib/Constrains.hpp
        lib/Constrains.cpp
        lib/MonteCarlo.hpp
        lib/MonteCarlo.cpp
        lib/CallStack.hpp
        lib/CallStack.cpp lib/DebugInfo.cpp lib/DebugInfo.hpp
        lib/cmd.cpp lib/cmd.hpp
        lib/ELFInfo.cpp lib/ELFInfo.hpp
        lib/Trace2ELF.cpp lib/Trace2ELF.hpp
        lib/InputParser.hpp
        lib/LibcModel.hpp lib/LibcModel.cpp)


target_link_libraries(TraceLib PRIVATE nlohmann_json::nlohmann_json)


add_executable(QIF
        QIF/QIF.cpp)

add_executable(QIF_EXP
        QIF_EXP/QIF_EXP.cpp)



target_link_libraries(QIF TraceLib)
target_link_libraries(QIF_EXP TraceLib)







