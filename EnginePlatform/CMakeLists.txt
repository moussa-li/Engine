project(EnginePlatform)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/3rd/imgui)
set(IMGUIFILEDLG_DIR ${CMAKE_SOURCE_DIR}/3rd/ImGuiFileDialog)

set_common_variable()

file(GLOB_RECURSE  SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/*.cpp"
)

add_subdirectory(include)
add_subdirectory(src)

add_executable(EnginePlatform
    ${CPP_FILES}
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp                 # 可选：包含官方 Demo 窗口
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp   # GLFW 平台后端
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp# OpenGL3 渲染后端
    ${IMGUIFILEDLG_DIR}/ImGuiFileDialog.cpp
)

target_include_directories(EnginePlatform PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/EnginePlatform/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUIFILEDLG_DIR}
)

target_link_libraries(EnginePlatform PRIVATE
    Common
    RenderEngine
    MeshEngine
)