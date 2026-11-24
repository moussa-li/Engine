cmake_minimum_required(VERSION 3.21)
project(GeometryEngine)

set_common_variable()

if(BUILD_GEOMETRY_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

if(WIN32)
    add_definitions(-DWNT)
endif()

# Resolve the kernel backend: AUTO probes for OpenCASCADE, otherwise OCCT or NULL.
set(GEOMETRY_KERNEL_BACKEND "${BUILD_GEOMETRY_KERNEL}")
if(NOT GEOMETRY_KERNEL_BACKEND)
    set(GEOMETRY_KERNEL_BACKEND "AUTO")
endif()
string(TOUPPER "${GEOMETRY_KERNEL_BACKEND}" GEOMETRY_KERNEL_BACKEND)

if(GEOMETRY_KERNEL_BACKEND STREQUAL "AUTO")
    find_package(OpenCASCADE QUIET)
    if(OpenCASCADE_FOUND)
        set(GEOMETRY_KERNEL_BACKEND "OCCT")
    else()
        set(GEOMETRY_KERNEL_BACKEND "NULL")
    endif()
elseif(GEOMETRY_KERNEL_BACKEND STREQUAL "OCCT")
    find_package(OpenCASCADE REQUIRED)
elseif(NOT GEOMETRY_KERNEL_BACKEND STREQUAL "NULL")
    message(FATAL_ERROR
        "Unsupported BUILD_GEOMETRY_KERNEL='${GEOMETRY_KERNEL_BACKEND}' (expected AUTO, OCCT or NULL)")
endif()

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

if(NOT GEOMETRY_KERNEL_BACKEND STREQUAL "OCCT")
    list(FILTER CPP_FILES EXCLUDE REGEX "/Kernel/Occt/")
endif()

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

if(GEOMETRY_KERNEL_BACKEND STREQUAL "OCCT")
    target_compile_definitions(${PROJECT_NAME} PUBLIC GEOMETRY_KERNEL_OCCT=1)
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCASCADE_INCLUDE_DIR})

    # Link only the toolkits this adapter uses. Linking OpenCASCADE_LIBRARIES pulls in
    # visualization modules that reference unrelated third-party libraries.
    set(OCCT_TOOLKITS
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKBRep
        TKTopAlgo
        TKMesh
        TKXSBase
    )
    if(TARGET TKDESTEP)
        list(APPEND OCCT_TOOLKITS TKDESTEP)
        if(TARGET TKDE)
            list(APPEND OCCT_TOOLKITS TKDE)
        endif()
    endif()
    if(TARGET TKSTEP)
        # OCCT 7.5 and earlier ship the STEP reader in TKSTEP.
        list(APPEND OCCT_TOOLKITS TKSTEPBase TKSTEPAttr TKSTEP209 TKSTEP)
    endif()

    set(OCCT_LINK_LIBRARIES)
    foreach(toolkit IN LISTS OCCT_TOOLKITS)
        # Resolve the import library by path: the imported OCCT targets embed absolute
        # third-party paths from the machine that built the binaries.
        find_library(OCCT_LIBRARY_${toolkit}
            NAMES ${toolkit}
            PATHS "${OpenCASCADE_LIBRARY_DIR}"
            NO_DEFAULT_PATH)
        if(OCCT_LIBRARY_${toolkit})
            list(APPEND OCCT_LINK_LIBRARIES "${OCCT_LIBRARY_${toolkit}}")
        elseif(TARGET ${toolkit})
            list(APPEND OCCT_LINK_LIBRARIES ${toolkit})
        endif()
    endforeach()

    if(NOT OCCT_LINK_LIBRARIES)
        message(FATAL_ERROR
            "GeometryEngine: OpenCASCADE was found but none of its toolkits could be linked")
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE ${OCCT_LINK_LIBRARIES})
    message(STATUS "GeometryEngine: kernel backend OCCT")

    # OCCT ships prebuilt DLLs, so its runtime must follow the executables. The optional
    # OpenCASCADE_THIRDPARTY_DLL_DIRS variable is a ';' separated list of extra runtime
    # directories (for example the bundled TBB vc14 bin dir).
    set(OCCT_RUNTIME_DIRS "")
    if(DEFINED OpenCASCADE_DLL_DIR)
        list(APPEND OCCT_RUNTIME_DIRS "${OpenCASCADE_DLL_DIR}")
    elseif(DEFINED OpenCASCADE_BINARY_DIR)
        list(APPEND OCCT_RUNTIME_DIRS "${OpenCASCADE_BINARY_DIR}")
    endif()
    if(DEFINED OpenCASCADE_THIRDPARTY_DLL_DIRS)
        foreach(dllDir IN LISTS OpenCASCADE_THIRDPARTY_DLL_DIRS)
            if(EXISTS "${dllDir}")
                list(APPEND OCCT_RUNTIME_DIRS "${dllDir}")
            endif()
        endforeach()
    endif()

    set(OCCT_RUNTIME_DLLS "")
    foreach(dllDir IN LISTS OCCT_RUNTIME_DIRS)
        file(GLOB dirDlls "${dllDir}/*.dll")
        list(APPEND OCCT_RUNTIME_DLLS ${dirDlls})
    endforeach()

    if(WIN32 AND OCCT_RUNTIME_DLLS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OCCT_RUNTIME_DLLS}
                    "${CMAKE_BINARY_DIR}/bin"
            COMMENT "Copying OpenCASCADE runtime DLLs")
    endif()
else()
    message(STATUS "GeometryEngine: kernel backend NULL (no third-party geometry kernel)")
endif()

if(WIN32)
    if(BUILD_GEOMETRY_SHARED)
        target_compile_definitions(${PROJECT_NAME} PRIVATE GeometryEngine)
    else()
        target_compile_definitions(${PROJECT_NAME} PUBLIC GEOMETRYENGINE_STATIC)
    endif()
endif()
