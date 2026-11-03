cmake_minimum_required(VERSION 3.14)
project(MeshEngine)

set_common_variable()

if((BUILD_MESH_SHARED))
    set(LIB_TYPE SHARED)
    add_definitions(-DEXPORT_MESHENGINE)
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

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME} PUBLIC Common)

if(WIN32 AND BUILD_MESH_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MeshEngine)
endif()