cmake_minimum_required(VERSION 3.14)

project(EngineLab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "build tests" ON)
option(BUILD_COMMON_SHARED "build the common libaray dynamic or static " ON)


include(cmake/common.cmake)

# generate asserts include file to 
gen_assets()



if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

add_subdirectory(Common)