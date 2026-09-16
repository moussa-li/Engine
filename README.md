# EngineLab

> A lightweight C++20 engine platform for mesh processing, real-time rendering, and interactive tools.

EngineLab is a modular engine platform designed to make graphics applications easier to build, extend, and maintain. It combines mesh data management, OpenGL rendering, scene organization, camera control, and an interactive desktop shell without forcing every application to carry a large general-purpose engine runtime.

The platform is organized as a small dependency stack: reusable data and utility components at the bottom, mesh and geometry services in the middle, and rendering plus application integration at the top. Each layer can evolve independently while keeping the public boundaries clear.

## What is included

| Module | Role |
| --- | --- |
| `Common` | Foundation utilities: vectors, matrices, smart pointers, containers, strings, logging, clocks, allocators, and `BBox`. |
| `MeshEngine` | Mesh data structures and mesh I/O. It currently includes `Mesh`, `Node`, `Elem`, iterators, and a `GmshImporter`. |
| `RenderEngine` | OpenGL rendering layer: windows, cameras, shaders, vertex/index buffers, scenes, render primitives, and render orchestration. |
| `EnginePlatform` | Executable desktop application built with GLFW, GLEW, Dear ImGui, and ImGuiFileDialog. |

The dependency direction is deliberately one-way:

```text
Common
  |\
  | MeshEngine
  |     |
  +-- RenderEngine ----> OpenGL / GLFW / GLEW
     |
     +--------> EnginePlatform ----> Dear ImGui
```

## Platform capabilities

- C++20 modular project structure driven by CMake.
- A small, explicit math layer with vectors, matrices, quaternions, and axis-aligned bounding boxes.
- Scene-oriented rendering with shader buckets and render primitives for points, lines, and faces.
- Camera view/projection handling with scene bounds used to derive depth clipping ranges.
- Mesh ownership, traversal, and import separated from rendering code.
- A GLFW-based application shell with Dear ImGui and ImGuiFileDialog integration.
- Optional GoogleTest targets for common, mesh, and render-related components.
- Optional Doxygen generation for API documentation.

The design goal is a focused platform rather than a monolithic engine: applications can use the existing modules directly, replace individual services, or add domain-specific systems above them.

## Requirements

- Windows with an OpenGL-capable graphics driver.
- CMake 3.14 or newer.
- A C++20 compiler. The checked-in presets cover Clang/MinGW and MSVC/Ninja configurations.
- GLFW, GLEW, Dear ImGui, ImGuiFileDialog, and GoogleTest are included under `3rd/`.

## Build

From the repository root:

```powershell
cmake --preset clang-debug
cmake --build build/clang-debug --target EnginePlatform
```

The executable is generated under:

```text
build/clang-debug/bin/EnginePlatform.exe
```

For an MSVC/Ninja build:

```powershell
cmake --preset win-ninja-debug
cmake --build build/win-ninja-debug --target EnginePlatform
```

The CMake configuration also exposes library targets such as `Common`, `MeshEngine`, and `RenderEngine`.

## Run the engine platform

After building, launch the platform application from the matching build directory:

```powershell
./build/clang-debug/bin/EnginePlatform.exe
```

The application entry point is `EnginePlatform/src/main.cpp`. `EnginePlatform` provides the executable shell and editor-style UI integration, while reusable graphics code remains in `RenderEngine`.

## Tests

Configure with tests enabled, then build and run the test targets:

```powershell
cmake --preset clang-debug -DBUILD_TESTS=ON
cmake --build build/clang-debug --target common_test engine_tests mesh_test
ctest --test-dir build/clang-debug --output-on-failure
```

The render tests require a usable OpenGL context. On headless machines or CI workers without graphics initialization, common and mesh tests can be run independently from the platform executable.

## Integration overview

### Import mesh data

`MeshEngine` keeps mesh storage independent from OpenGL. Importers produce a `Mesh`, which can then be traversed or converted into application-specific render data:

```cpp
#include "MeshEngine/IO/GmshImporter.hpp"

EgLab::ME::GmshImporter importer("assets/models/example.msh");
auto mesh = importer.getMesh();

if (mesh != nullptr) {
 EgLab::ME::MeshIterator iterator(*mesh);
 while (iterator.nextNode()) {
  const auto &node = iterator.currentNode();
  // Consume node coordinates here.
 }
}
```

### Add render primitives to a scene

`RenderEngine` accepts a shader and a render primitive, then maintains scene-wide bounds for camera setup:

```cpp
auto scene = EgLab::Common::makeShared<EgLab::RE::Scene>();
scene->addPrimitive(shader, primitive);

const auto &bounds = scene->getBounds();
renderer.draw(scene, camera);
```

`RenderNode`, `RenderLine`, and `RenderFace` calculate their local vertex bounds. The scene merges those boxes, and the renderer passes the result to the camera when creating the projection matrix.

## Repository layout

```text
Common/          Core containers, math, memory, logging, and utilities
MeshEngine/      Mesh data model, iterators, importers, and geometry algorithms
RenderEngine/    OpenGL resources, camera, scene, shaders, and primitives
EnginePlatform/  Runnable GLFW + ImGui application
assets/          Models, shaders, and textures used by examples
3rd/             Third-party dependencies
test/            GoogleTest sources
cmake/           Shared CMake helpers
```

## Architecture path

1. Use `Common` for shared data structures, math, memory, and utility services.
2. Use `MeshEngine/MeshData` and its importers to represent and traverse geometry.
3. Convert mesh data into `RenderEngine` primitives and register them in a `Scene`.
4. Use `Renderer`, `Camera`, and shader resources to draw the scene.
5. Embed the platform layer when an interactive desktop UI or tooling workflow is needed.

## Roadmap

- Stabilize the mesh-to-render pipeline and add more mesh formats.
- Add transform-aware world-space bounds and frustum culling.
- Expand materials, lighting, textures, and resource lifetime management.
- Improve cross-platform build presets and headless test coverage.
- Add reusable scene, input, and editor services to the platform layer.
- Keep public module boundaries small so applications can adopt only what they need.

## Technical references

- [LearnOpenGL CN](https://learnopengl-cn.github.io/)
- [The Cherno: OpenGL](https://www.youtube.com/@TheCherno)
- [GLFW](https://www.glfw.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)

## License

See [LICENSE](LICENSE) for the project license.
