cmake_minimum_required(VERSION 3.10)
project(Abacus)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


option(KOUZI_USE_EXTERNAL_JSON "Use an external JSON library" OFF)
add_subdirectory(thirdparty)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE INCLUDE_FILES ${CMAKE_SOURCE_DIR}/src/include/*.hpp)
file(GLOB_RECURSE SOURCE_FILES_NO_MAIN ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(SOURCE_FILES ${SOURCE_FILES_NO_MAIN})

include(${CMAKE_SOURCE_DIR}/cmake/clang_tools.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/cppcheck.cmake)

add_subdirectory(src)


add_subdirectory(test)
add_subdirectory(App)
