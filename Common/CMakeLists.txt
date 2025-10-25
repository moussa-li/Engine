cmake_minimum_required(VERSION 3.14)
project(Common)

set_common_variable()

if(BUILD_COMMON_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

file(GLOB_RECURSE  SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/*.cpp"
)

add_library(${PROJECT_NAME} ${LIB_TYPE}
    ${CPP_FILES}
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC pthread)

if(WIN32 AND BUILD_COMMON_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE EgLabCommon)
endif()

