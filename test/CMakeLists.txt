# add gtest
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/gtest ${CMAKE_BINARY_DIR}/3rd/gtest_build) 

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/3rd/imgui)

set_common_variable()

add_executable(engine_tests
    test_main.cpp
    test_normal.cpp
    test_common.cpp
    test_render.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp                 # 可选：包含官方 Demo 窗口
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp   # GLFW 平台后端
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp# OpenGL3 渲染后端
)

add_executable(common_test
    test_common.cpp
    test_command_bus.cpp
    test_data_node.cpp
    test_main.cpp
    ${CMAKE_SOURCE_DIR}/EnginePlatform/src/Command/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/EnginePlatform/src/DataBase/DataTree.cpp
)

add_executable(mesh_test
    test_mesh.cpp
    test_mesh_visibility.cpp
    test_selection.cpp
    ${CMAKE_SOURCE_DIR}/EnginePlatform/src/Work/SelectionModel.cpp
    test_main.cpp
)

add_executable(geometry_test
    test_geometry.cpp
    test_main.cpp
)

# Converter test: links the few EnginePlatform data sources it needs instead of the whole
# platform target (which pulls in the renderer and the data tree).
add_executable(geometry_platform_test
    test_geometry_platform.cpp
    ${CMAKE_SOURCE_DIR}/EnginePlatform/src/Command/GeometryWire.cpp
    ${CMAKE_SOURCE_DIR}/EnginePlatform/src/DataBase/GeometryMeshConverter.cpp
    test_main.cpp
)

target_include_directories(geometry_platform_test PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/EnginePlatform/include
)

target_link_libraries(geometry_platform_test PRIVATE
    gtest
    gtest_main
    Common
    MeshEngine
    GeometryEngine
)

if(WIN32)
    add_executable(command_cross_process_test
        test_command_cross_process.cpp
    )

    target_include_directories(command_cross_process_test PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/EnginePlatform/include
    )

    target_link_libraries(command_cross_process_test PRIVATE
        Common
        MeshEngine
    )
endif()

if(NOT MSVC)
    target_compile_options(mesh_test PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(mesh_test PRIVATE -fsanitize=address)
endif()

if(MSVC)
    set_target_properties(engine_tests PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

if(WIN32)
    message(STATUS "Configuring for Windows")
    if(MSVC)
    # set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    endif()
elseif(UNIX)
    message(STATUS "Configuring for Linux")
endif()

target_include_directories(engine_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_include_directories(common_test PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/EnginePlatform/include
)

target_link_libraries(engine_tests PRIVATE
    gtest
    gtest_main
    Common
    RenderEngine
    glfw
    glew_s
)

target_include_directories(mesh_test PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/EnginePlatform/include
)

target_link_libraries(mesh_test PRIVATE
    gtest
    gtest_main
    Common
    MeshEngine
)

target_include_directories(geometry_test PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/gtest/googletest/include
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(geometry_test PRIVATE
    gtest
    gtest_main
    Common
    GeometryEngine
)

target_link_libraries(common_test PRIVATE
    gtest
    gtest_main
    Common
)

#add_dependencies(engine_tests embed_gen_files)

add_test(NAME engine_tests COMMAND engine_tests)

add_test(NAME common_test COMMAND common_test)

add_test(NAME geometry_test COMMAND geometry_test)

add_test(NAME geometry_platform_test COMMAND geometry_platform_test)