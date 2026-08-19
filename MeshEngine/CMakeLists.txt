cmake_minimum_required(VERSION 3.14)
project(MeshEngine)

set_common_variable()

if((BUILD_MESH_SHARED))
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

if(WIN32)
add_definitions(-DWNT)
add_definitions(-DGLEW_STATIC)
endif()

file(GLOB_RECURSE  SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/*.cpp"
)

add_subdirectory(include)
add_subdirectory(src)

add_library(${PROJECT_NAME} ${LIB_TYPE}
    ${CPP_FILES}
)


target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/MeshEngine/include
)
target_link_libraries(${PROJECT_NAME} PUBLIC Common)

if(WIN32 AND BUILD_MESH_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MeshEngine)
endif()