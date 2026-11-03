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

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/RenderEngine>
)

target_link_libraries(${PROJECT_NAME} PUBLIC Common)

target_link_libraries(${PROJECT_NAME} PUBLIC MeshEngine)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

if(TARGET glew_s)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        $<BUILD_INTERFACE:glew_s>
        $<INSTALL_INTERFACE:GLEW::GLEW>
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

if(WIN32 AND BUILD_RENDER_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RenderEngine)
endif()