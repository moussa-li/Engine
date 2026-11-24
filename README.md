# EngineLab

> A lightweight C++20 engine platform for mesh processing, real-time rendering, and interactive tools.

EngineLab is a modular engine platform designed to make graphics applications easier to build, extend, and maintain. It combines mesh data management, OpenGL rendering, scene organization, camera control, and an interactive desktop shell without forcing every application to carry a large general-purpose engine runtime.

The platform is organized as a small dependency stack: reusable data and utility components at the bottom, mesh and geometry services in the middle, and rendering plus application integration at the top. Each layer can evolve independently while keeping the public boundaries clear.

## What is included

| Module | Role |
| --- | --- |
| `Common` | Foundation utilities: vectors, matrices, smart pointers, containers, strings, logging, clocks, allocators, and `BBox`. |
| `MeshEngine` | Mesh data structures and mesh I/O. It currently includes `Mesh`, `Node`, `Elem`, iterators, and a `GmshImporter`. |
| `GeometryEngine` | B-Rep topology/geometry model with a kernel-agnostic adapter layer and an OpenCASCADE 7.5 STEP/BREP importer. |
| `RenderEngine` | OpenGL rendering layer: windows, cameras, shaders, vertex/index buffers, scenes, render primitives, and render orchestration. |
| `EnginePlatform` | Executable desktop application built with GLFW, GLEW, Dear ImGui, and ImGuiFileDialog. |

The dependency direction is deliberately one-way:

```text
Common
  |\
  | +-- MeshEngine
  | +-- GeometryEngine ----> (optional) OpenCASCADE 7.5
  |
  +-- RenderEngine ----> OpenGL / GLFW / GLEW
        |
        +----> EnginePlatform ----> Dear ImGui
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
- CMake 3.21 or newer.
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

The CMake configuration also exposes library targets such as `Common`, `MeshEngine`, `RenderEngine`, and `GeometryEngine`.

> **Localized MSVC + Ninja:** on a non-English Visual Studio installation (for example a
> Chinese-only language pack) `cl` prints a localized `/showIncludes` notice. CMake writes that
> prefix into `CMakeFiles/rules.ninja` and Ninja fails to match it, so **no header dependencies are
> recorded** and incremental builds silently reuse stale object files (mismatched class layouts can
> crash at runtime). Configure once with the console code page the compiler speaks, then build
> normally:
>
> ```powershell
> cmd /c "chcp 936 && cmake --preset win-ninja-relwithdebinfo-occt"
> cmake --build --preset win-ninja-relwithdebinfo-occt
> ```
>
> Verify with `ninja -t deps <some>.obj` — it must report a non-zero `#deps` count. Installing the
> English language pack for Visual Studio removes the problem entirely.

### Geometry kernel selection

`GeometryEngine` builds with a `Null` kernel adapter by default, so the module compiles and links
without any third-party geometry kernel. The `win-ninja-relwithdebinfo-occt` preset enables the
OpenCASCADE 7.5 adapter; it expects the OCCT 7.5 prebuilt package under `3rd/occt-7.5.0`:

```powershell
cmake --preset win-ninja-relwithdebinfo-occt
cmake --build --preset win-ninja-relwithdebinfo-occt --target EnginePlatform geometry_test
```

The preset pins `BUILD_GEOMETRY_KERNEL=OCCT`, `OpenCASCADE_DIR` and
`OpenCASCADE_THIRDPARTY_DLL_DIRS` (the bundled TBB `vc14` bin directory required by `TKernel.dll`).
If OCCT lives elsewhere, edit those two cache variables in
[CMakePresets.json](CMakePresets.json). `3rd/occt-7.5.0/` is git-ignored, so install the package
locally with the official `opencascade-7.5.0-vc14-64.exe` installer.

`BUILD_GEOMETRY_KERNEL` also accepts `AUTO` (default; probes OpenCASCADE and falls back to `NULL`)
and `NULL`. The adapter links only the OCCT toolkits it uses, and on Windows the required OCCT
runtime DLLs (15 toolkits plus the TBB DLLs, about 25 MB) are copied next to the build executables
and also installed next to the installed executables, so a packaged `EnginePlatform` needs no
separate OCCT/TBB installation on `PATH`. The OCCT import test is opt-in: set
`EGLAB_GEOMETRY_TEST_STEP` to a `.stp`/`.brep` file before running `geometry_test`.

OpenCASCADE is consumed as prebuilt libraries (LGPL-2.1 with exception, dynamically linked) and is
never modified. The validated baseline is OCCT 7.5.0; the STEP toolkit is resolved as `TKSTEP`
(7.5 and earlier) or `TKDESTEP` (later releases). Because the prebuilt packages are release-only,
build `GeometryEngine` with a release-like configuration (`RelWithDebInfo` or `Release`); a Debug
(`/MDd`) build would mix CRTs against the release OCCT binaries.

## Install and use from another CMake project

Install EngineLab to a prefix:

```powershell
cmake --preset clang-debug
cmake --build build/clang-debug
cmake --install build/clang-debug
```

In another project, set `EngineLab_DIR` to the installed `lib/cmake/EngineLab` directory (or add the install prefix to `CMAKE_PREFIX_PATH`), then link the exported targets:

```cmake
find_package(EngineLab CONFIG REQUIRED)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE EngineLab::RenderEngine)
```

The primary targets are `EngineLab::Common`, `EngineLab::MeshEngine`, `EngineLab::RenderEngine`, and `EngineLab::GeometryEngine`; `EngineLab::Assets` is also exported for `Common`'s resource dependency. Consumers can link `EngineLab::RenderEngine` directly without separately finding or linking GLFW or GLEW.

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
cmake --build build/clang-debug --target common_test engine_tests mesh_test geometry_test
ctest --test-dir build/clang-debug --output-on-failure
```

The render tests require a usable OpenGL context. On headless machines or CI workers without graphics initialization, common, mesh, and geometry tests can be run independently from the platform executable.

The OCCT import test is opt-in: set `EGLAB_GEOMETRY_TEST_STEP` to a `.stp`/`.step`/`.brep` file to
import it and verify topology and tessellation counts.

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

### Import and inspect B-Rep geometry

`GeometryEngine` converts STEP/BREP files into its own neutral topology model. The third-party
kernel stays behind `IGeometryKernelFactory` / `IGeometryImporter` and is selected by name through
`GeometryKernelRegistry`, so supporting OCCT 8 or Parasolid later only requires a new adapter.

```cpp
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"

EgLab::GE::GeometryDocument document;
EgLab::GE::GeometryImporter importer("OCCT");
const auto report = importer.importFile("assets/models/bracket.stp", document);

const auto solid = document.getFeature(EgLab::GE::TopologyType::Solid, 0);
const auto &mesh = solid.getMesh();
for (std::size_t i = 0; i < mesh.getNodeCount(); ++i) {
    const auto &position = mesh.getNodes()[i]; // x / y / z
}
for (std::size_t e = 0; e < mesh.getElemCount(); ++e) {
    // getElemSizes()[e] node indices start at a running offset in getElemNodeIndices()
}
```

Each topology node owns its `id`, `name`, `children` (solid -> face -> edge -> vertex) and
`parents`, while the matching `Geometry` stores length/area/volume, a bounding box, and the
renderable tessellation exposed by `GeometryFeature::getMesh()`. Every OCCT header, type, and
namespace stays inside `GeometryEngine/src/Kernel/Occt`; nothing kernel-specific appears under
`GeometryEngine/include`. Consumers choose the kernel (for example from `EgPlatform.ini`) and pass
its name to `GeometryImporter`.

#### Geometry display modes

Imported B-Rep models are rendered through `ME::Mesh` (Tri3). Their display can be switched in
`Geometry` → `Geometry Render Settings` (persisted as `geometry.displayMode` in `EgPlatform.ini`)
or through the `SetGeometryRenderMode` command:

| Mode | Value | Appearance |
| --- | --- | --- |
| Mesh | 0 | Shaded faces with the dense triangle wireframe (the regular mesh look). |
| Face outline | 1 | Shaded faces, but only each face's outer frame is drawn; normals are smoothed per mesh node so curved faces shade smoothly while B-Rep face boundaries stay sharp. |
| Edges only | 2 | Faces are not rendered, but they stay pickable so hovering still resolves the B-Rep feature; only the geometry outline lines are drawn. |

The mode applies to imported geometry models only; regular meshes keep their existing appearance.
`Geometry Render Settings` dispatches `SetGeometryRenderMode` through `CommandManager`, so AI
clients can drive the same modes.

#### Geometry picking and selection

Geometry models are picked at B-Rep feature level, never at triangle level:

* A hit on a tessellated triangle is resolved to the feature that owns it (`GeometryElementMap`
  maps `elemId → FeatureRef` and back), and **all** triangles of that feature are highlighted, so
  hovering a face highlights the whole face.
* `Geometry Info` exposes the pick filter (persisted as `geometry.pickFilter`, dispatched as the
  `SetGeometryPickFilter` command, `mask` bit0=Vertex bit1=Edge bit2=Face bit3=Solid). A hit whose
  feature type is disabled is rejected, so the cursor falls through to whatever is behind it. With
  `Face` disabled and `Solid` enabled, a face hit is promoted to the solid that contains it and the
  whole solid is highlighted and reported.
* Highlights are depth-offset so they stay visible on the surface they duplicate: face highlights
  use `glPolygonOffset(-1, -4)`, line highlights squeeze the near depth range.
* Rectangle selection expands every covered triangle to its complete feature and honours the same
  filter, so a box selected across a face always selects whole faces.

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
GeometryEngine/  B-Rep topology/geometry model and kernel adapters (OCCT, Null)
RenderEngine/    OpenGL resources, camera, scene, shaders, and primitives
EnginePlatform/  Runnable GLFW + ImGui application
assets/          Models, shaders, and textures used by examples
3rd/             Third-party dependencies
test/            GoogleTest sources
cmake/           Shared CMake helpers
```

## Architecture path

1. Use `Common` for shared data structures, math, memory, and utility services.
2. Use `GeometryEngine` for B-Rep topology, per-feature geometry, and STEP/BREP import.
3. Use `MeshEngine/MeshData` and its importers to represent and traverse geometry.
4. Convert mesh data into `RenderEngine` primitives and register them in a `Scene`.
5. Use `Renderer`, `Camera`, and shader resources to draw the scene.
6. Embed the platform layer when an interactive desktop UI or tooling workflow is needed.

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
