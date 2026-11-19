# AiClient/CMakeLists.txt
#
# 在构建 EngineLab 时同步把 AiClient（Electron 对话客户端）打包为可执行程序。
# 由根 CMakeLists.txt 在 BUILD_AI_CLIENT=ON 时通过 add_subdirectory 引入。
#
# 产物：
#   AiClient/electron/dist/win-unpacked/EgLabAIDialog.exe   （可直接运行，dist:dir）
#   如需生成 NSIS 安装包，手动执行:  npm run dist
#
# 依赖：Node.js / npm（以及可用的 Python 3，运行期由 Electron 主进程拉起 agent_server.py）。

unset(NPM_EXECUTABLE CACHE)
find_program(NPM_EXECUTABLE NAMES npm.cmd npm)

if(NOT NPM_EXECUTABLE)
    message(WARNING "未找到 npm，跳过 EgLab AI 对话客户端构建（安装 Node.js 后重新配置即可启用）")
    return()
endif()

# Electron/electron-builder 二进制默认从 GitHub 下载，国内网络通常不可达。
# 默认走 npmmirror 镜像，可通过 -DEGLAB_ELECTRON_MIRROR=... 覆盖。
set(EGLAB_ELECTRON_MIRROR "https://npmmirror.com/mirrors/electron/"
    CACHE STRING "Electron 下载镜像")
set(EGLAB_ELECTRON_BUILDER_MIRROR "https://npmmirror.com/mirrors/electron-builder-binaries/"
    CACHE STRING "electron-builder 二进制下载镜像")

set(EGLAB_AI_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/electron")
set(EGLAB_AI_STAMP "${CMAKE_CURRENT_BINARY_DIR}/ai_client.stamp")

add_custom_command(
    OUTPUT "${EGLAB_AI_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E env
            "ELECTRON_MIRROR=${EGLAB_ELECTRON_MIRROR}"
            "ELECTRON_BUILDER_BINARIES_MIRROR=${EGLAB_ELECTRON_BUILDER_MIRROR}"
            "CSC_IDENTITY_AUTO_DISCOVERY=false"
            "${NPM_EXECUTABLE}" install --no-audit --no-fund
    COMMAND "${CMAKE_COMMAND}" -E env
            "ELECTRON_MIRROR=${EGLAB_ELECTRON_MIRROR}"
            "ELECTRON_BUILDER_BINARIES_MIRROR=${EGLAB_ELECTRON_BUILDER_MIRROR}"
            "CSC_IDENTITY_AUTO_DISCOVERY=false"
            "${NPM_EXECUTABLE}" run dist:dir
    COMMAND "${CMAKE_COMMAND}" -E touch "${EGLAB_AI_STAMP}"
    WORKING_DIRECTORY "${EGLAB_AI_APP_DIR}"
    DEPENDS
        "${EGLAB_AI_APP_DIR}/package.json"
        "${EGLAB_AI_APP_DIR}/main.js"
        "${EGLAB_AI_APP_DIR}/preload.js"
        "${CMAKE_CURRENT_SOURCE_DIR}/agent_server.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/mcp_tools.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/command_schemas.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/index.html"
    COMMENT "Building EgLab AI dialog Electron client (electron-builder)"
    VERBATIM
)

add_custom_target(AiClient ALL DEPENDS "${EGLAB_AI_STAMP}")

# cmake --install 时把打包好的 AI 对话客户端（exe + resources，含 Python 运行时与 Agent/UI）
# 一并安装到 <prefix>/bin/AiClient/，双击 EgLabAIDialog.exe 即可直接启动。
set(EGLAB_AI_DIST_DIR "${EGLAB_AI_APP_DIR}/dist/win-unpacked")
install(CODE "
    set(_ai_src \"${EGLAB_AI_DIST_DIR}\")
    set(_ai_dst \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/AiClient\")
    if(NOT EXISTS \"\${_ai_src}/EgLabAIDialog.exe\")
        message(FATAL_ERROR \"EgLabAIDialog.exe 不存在于 \${_ai_src}，请先构建: cmake --build . --target AiClient\")
    endif()
    file(COPY \"\${_ai_src}/\" DESTINATION \"\${_ai_dst}\")
    message(STATUS \"已安装 EgLab AI 对话客户端到 \${_ai_dst}\")
" COMPONENT AiClient)

message(STATUS "EgLab AI 对话客户端：已启用，构建目标 AiClient（Electron -> dist/win-unpacked/EgLabAIDialog.exe）")
