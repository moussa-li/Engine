cmake_minimum_required(VERSION 3.14)

project(EngineLab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "build tests" ON)
option(BUILD_COMMON_SHARED "build the common library dynamic or static " ON)
option(BUILD_RENDER_SHARED "build the RenderEngine library dynamic or static" ON)


include(cmake/common.cmake)

# generate asserts include file to 
gen_assets()



if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

add_subdirectory(3rd/glfw-3.4)

add_subdirectory(Common)
add_subdirectory(RenderEngine)