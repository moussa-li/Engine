cmake_minimum_required(VERSION 3.21)

project(EngineLab VERSION 0.1.0)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()
if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "build tests" ON)
option(BUILD_COMMON_SHARED "Build Common as a shared library (OFF builds a static library)" OFF)
option(BUILD_RENDER_SHARED "Build RenderEngine as a shared library (OFF builds a static library)" OFF)
option(BUILD_DOC "generate the documentation with doxygen" OFF)


include(cmake/common.cmake)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set_common_variable()
find_package(Threads REQUIRED)

# generate asserts include file to 
gen_assets()

set(GLEW_DIR "${CMAKE_SOURCE_DIR}/3rd/glew")
if(NOT EXISTS "${GLEW_DIR}/include/GL/glew.h" OR NOT EXISTS "${GLEW_DIR}/src/glew.c")
    find_program(GNU_MAKE_EXECUTABLE NAMES make gmake mingw32-make REQUIRED)
    message(STATUS "Generating missing GLEW sources in ${GLEW_DIR}")
    execute_process(
        COMMAND "${GNU_MAKE_EXECUTABLE}" -C "${GLEW_DIR}" extensions
        RESULT_VARIABLE GLEW_GENERATE_RESULT
        OUTPUT_VARIABLE GLEW_GENERATE_OUTPUT
        ERROR_VARIABLE GLEW_GENERATE_OUTPUT
    )
    if(NOT GLEW_GENERATE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to generate GLEW sources: ${GLEW_GENERATE_OUTPUT}")
    endif()
endif()

set(BUILD_UTILS OFF CACHE BOOL "Disable GLEW utility executables" FORCE)

if(BUILD_TESTS)
    set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of bundled googletest" FORCE)
    enable_testing()
    add_subdirectory(test)
endif()

set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL ON)
add_subdirectory(3rd/glfw-3.4)
add_subdirectory(3rd/glew/build/cmake)

add_subdirectory(Common)
add_subdirectory(RenderEngine)
add_subdirectory(MeshEngine)
add_subdirectory(EnginePlatform)

install(TARGETS EnginePlatform
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS Common MeshEngine RenderEngine Assets
    EXPORT EngineLabTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY Common/include/Common/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Common
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(DIRECTORY MeshEngine/include/MeshEngine
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(DIRECTORY RenderEngine/include/RenderEngine/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/RenderEngine
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "GLWrapper.hpp" EXCLUDE
)

set(EngineLab_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/EngineLab")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EngineLabConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/EngineLabConfig.cmake"
    INSTALL_DESTINATION "${EngineLab_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/EngineLabConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)
install(EXPORT EngineLabTargets
    FILE EngineLabTargets.cmake
    NAMESPACE EngineLab::
    DESTINATION "${EngineLab_INSTALL_CMAKEDIR}"
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/EngineLabConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EngineLabConfigVersion.cmake"
    DESTINATION "${EngineLab_INSTALL_CMAKEDIR}"
)

#install(FILES "${CMAKE_SOURCE_DIR}/render_settings.ini" DESTINATION .)


if(BUILD_DOC)
    find_package(Doxygen REQUIRED)
    if(DOXYGEN_FOUND)
        set(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/Doxyfile.in)
        set(DOXYFILE_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

        add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
        )
    else()
        message(WARNING " Doxygen not found, documentation will not be generated")
    endif()

endif()