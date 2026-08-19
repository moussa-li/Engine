cmake_minimum_required(VERSION 3.14)
project(RenderEngine)

set_common_variable()

if((BUILD_RENDER_SHARED))
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

add_subdirectory(Core)
add_subdirectory(Objects)

add_library(${PROJECT_NAME} ${LIB_TYPE}
    ${CPP_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/RenderEngine
    ${CMAKE_SOURCE_DIR}/3rd/glew/include
    ${CMAKE_SOURCE_DIR}/3rd/glfw-3.4/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC Common)

target_link_libraries(${PROJECT_NAME} PUBLIC MeshEngine)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    opengl32
    glew32
    glfw
)

if(WIN32 AND BUILD_RENDER_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RenderEngine)
endif()