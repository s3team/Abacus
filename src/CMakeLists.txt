cmake_minimum_required(VERSION 3.10)
project(TraceLib)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(JSON_BuildTests OFF CACHE INTERNAL "")




add_library(TraceLib
        error.cpp
        x86.cpp
        ins_parser.cpp
        ins_types.cpp
        ins_semantics.cpp
        Register.cpp
        VarMap.cpp
        Tainter.cpp
        Loop.cpp
        Function.cpp
        Blocks.cpp
        BitVector.cpp
        Constrains.cpp
        MonteCarlo.cpp
        CallStack.cpp
        DebugInfo.cpp
        cmd.cpp
        ELFInfo.cpp
        Trace2ELF.cpp
        LibcModel.cpp
        external_api.cpp

        Engine/Engine.cpp
        Engine/DynSEEngine.cpp
        Engine/StaticSEEngine.cpp
        )


target_include_directories(TraceLib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(TraceLib PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/libelfio)



target_link_libraries(TraceLib PRIVATE dwarf)
target_link_libraries(TraceLib PRIVATE nlohmann_json::nlohmann_json)
