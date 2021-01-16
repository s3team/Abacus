cmake_minimum_required(VERSION 3.10)
project(QIF)

add_executable(QIF
        QIF.cpp
        src/QIFSEEngine.cpp)

add_executable(QIF-test
        QIF-test.cpp
        src/QIFSEEngine.cpp)

target_include_directories(QIF PRIVATE include)
target_include_directories(QIF-test PRIVATE include)

target_link_libraries(QIF TraceLib)
target_link_libraries(QIF-test TraceLib)