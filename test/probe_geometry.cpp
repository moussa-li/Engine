// Temporary diagnostic probe: reproduces the geometry render-prep path without OpenGL.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#pragma comment(lib, "dbghelp.lib")
#include <windows.h>
#include <dbghelp.h>

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <crtdbg.h>

int probeMain(int argc, char **argv);
#include <cstdint>

#include <crtdbg.h>

#include "Common/DynamicArray.hpp"
#include "Common/HashSet.hpp"
#include "Common/SharedPtr.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Model/GeometryFeature.hpp"
#include "GeometryEngine/Topology/Topology.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/Node.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"

namespace
{
    // Same conversion as GeometryMeshConverter (kept inline so the probe links nothing extra).
    EgLab::Common::SharedPtr<EgLab::ME::Mesh> convert(const EgLab::GE::GeometryDocument& document)
    {
        using namespace EgLab;
        auto mesh = Common::makeShared<ME::Mesh>();
        const std::size_t faceCount = document.getFeatureCount(GE::TopologyType::Face);
        std::printf("faces=%zu\n", faceCount);
        ME::IdType nextNodeId = 1;
        ME::IdType nextElemId = 1;
        for (std::size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            const GE::Geometry* geometry =
                document.getGeometry(GE::TopologyType::Face, static_cast<GE::IdxType>(faceIndex));
            if (geometry == nullptr) continue;
            const GE::Tessellation& tess = geometry->getTessellation();
            const auto& nodes = tess.getNodes();
            const auto& elementIndices = tess.getElemNodeIndices();
            const auto& elementSizes = tess.getElemSizes();
            const ME::IdType base = nextNodeId;
            for (std::size_t n = 0; n < nodes.size(); ++n)
            {
                ME::Node node;
                node.setId(nextNodeId++);
                ME::CoordType c(nodes[n][0], nodes[n][1], nodes[n][2]);
                node.setXYZ(c);
                mesh->addNode(Common::move(node));
            }
            std::size_t cursor = 0;
            for (std::size_t e = 0; e < elementSizes.size(); ++e)
            {
                const std::uint32_t nodeCount = elementSizes[e];
                if (nodeCount == 3)
                {
                    ME::Elem elem;
                    elem.setId(nextElemId++);
                    elem.setType(ME::ElemType::Tri3);
                    for (std::uint32_t corner = 0; corner < 3; ++corner)
                    {
                        elem.setNode(corner, static_cast<ME::IdType>(base + elementIndices[cursor + corner]));
                    }
                    mesh->addElem(Common::move(elem));
                }
                cursor += nodeCount;
            }
        }
        std::printf("mesh nodes=%u elems=%u\n", mesh->getNodeNumber(), mesh->getElemNumber());
        return mesh;
    }
} // namespace

namespace
{
    const char *g_baseDir = nullptr;

    LONG WINAPI vexHandler(EXCEPTION_POINTERS *info)
    {
        std::printf("\n!!! EXCEPTION code=0x%08lX addr=%p\n",
                    info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress);
        const HANDLE process = GetCurrentProcess();
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        if (g_baseDir)
        {
            SymInitialize(process, g_baseDir, TRUE);
        }
        else
        {
            SymInitialize(process, nullptr, TRUE);
        }
        void *frames[48];
        const USHORT count = CaptureStackBackTrace(0, 48, frames, nullptr);
        SYMBOL_INFO *symbol = (SYMBOL_INFO *)std::calloc(sizeof(SYMBOL_INFO) + 512, 1);
        symbol->MaxNameLen = 511;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        for (USHORT index = 0; index < count; ++index)
        {
            DWORD64 displacement = 0;
            const DWORD64 address = reinterpret_cast<DWORD64>(frames[index]);
            const BOOL ok = SymFromAddr(process, address, &displacement, symbol);
            IMAGEHLP_LINE64 line{};
            line.SizeOfStruct = sizeof(line);
            DWORD lineDisplacement = 0;
            const BOOL lineOk = SymGetLineFromAddr64(process, address, &lineDisplacement, &line);
            std::printf("  [%02u] %p %s + 0x%llx   %s:%lu\n", index, frames[index],
                        ok ? symbol->Name : "?", displacement,
                        lineOk ? line.FileName : "?", lineOk ? line.LineNumber : 0ul);
        }
        std::free(symbol);
        return EXCEPTION_EXECUTE_HANDLER;
    }
} // namespace

int main(int argc, char** argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    if (argc >= 2 && std::strcmp(argv[1], "--dbg") == 0)
    {
        g_baseDir = argc >= 3 ? argv[2] : nullptr;
        AddVectoredExceptionHandler(1, vexHandler);
        std::printf("dbg: creating node\n");
        {
            auto node = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
            (void)node;
            std::printf("dbg: node created\n");
        }
        std::printf("dbg: node destroyed\n");
        return 0;
    }
    return probeMain(argc, argv);
}

int probeMain(int argc, char **argv)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#define PROBE_CHECK(tag)                                                        \
    do                                                                          \
    {                                                                           \
        const int ok = _CrtCheckMemory();                                        \
        std::printf("  [heap] %s -> %s\n", tag, ok ? "ok" : "CORRUPT");          \
    } while (0)

    if (argc < 2)
    {
        std::printf("usage: probe <file.stp>\n");
        return 2;
    }

    if (std::strcmp(argv[1], "--c1") == 0)
    {
        std::printf("only c1\n");
        {
            auto node = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
            (void)node;
            std::printf("c1 created\n");
        }
        std::printf("c1 destroyed\n");
        return 0;
    }

    if (std::strcmp(argv[1], "--p1") == 0)
    {
        struct ProbePrimitive : EgLab::RE::RenderPrimitive
        {
            void setup() override {}
            void draw(EgLab::Common::SharedPtr<EgLab::RE::Shader>) override {}
            EgLab::Common::BBox<EgLab::RE::Scalar, 3> getBounds() const override
            {
                return EgLab::Common::BBox<EgLab::RE::Scalar, 3>();
            }
            EgLab::Common::DynamicArray<EgLab::RE::CoordType> _values;
            EgLab::Common::HashSet<EgLab::RE::IdxType> _set;
        };
        std::printf("p1 start sizeof=%zu\n", sizeof(ProbePrimitive));
        {
            ProbePrimitive p;
            std::printf("p1 stack created\n");
        }
        std::printf("p1 stack destroyed\n");
        {
            ProbePrimitive *p = new ProbePrimitive();
            std::printf("p1 heap created\n");
            delete p;
            std::printf("p1 heap destroyed\n");
        }
        {
            auto p = EgLab::Common::makeShared<ProbePrimitive>();
            std::printf("p1 shared created\n");
        }
        std::printf("p1 shared destroyed\n");
        {
            auto p = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
            std::printf("node created sizeof=%zu\n", sizeof(EgLab::RE::RenderNode));
        }
        std::printf("node destroyed\n");
        {
            EgLab::RE::RenderNode p;
            std::printf("node stack created\n");
        }
        std::printf("node stack destroyed\n");
        return 0;
    }

    if (std::strcmp(argv[1], "--dyn") == 0)
    {
        std::printf("dyn start\n");
        {
            EgLab::Common::DynamicArray<double> a;
            std::printf("dyn a\n");
        }
        std::printf("dyn a done\n");
        {
            EgLab::Common::DynamicArray<float> a;
            std::printf("dyn f\n");
        }
        std::printf("dyn f done\n");
        {
            EgLab::Common::DynamicArray<unsigned int> a;
            std::printf("dyn u\n");
        }
        std::printf("dyn u done\n");
        {
            EgLab::Common::DynamicArray<double> a;
            a.pushBack(1.0);
            std::printf("dyn push %zu\n", a.size());
        }
        std::printf("dyn push done\n");
        return 0;
    }

    if (std::strcmp(argv[1], "--set") == 0)
    {
        std::printf("set start\n");
        {
            EgLab::Common::HashSet<unsigned int> h;
            std::printf("set empty\n");
        }
        std::printf("set empty done\n");
        {
            EgLab::Common::HashSet<unsigned int> h;
            h.insert(3u);
            h.insert(7u);
            std::printf("set size %zu\n", h.size());
        }
        std::printf("set size done\n");
        return 0;
    }

    if (std::strcmp(argv[1], "--line") == 0)
    {
        {
            auto line = EgLab::Common::makeShared<EgLab::RE::RenderLine>();
            std::printf("line created\n");
        }
        std::printf("line destroyed\n");
        {
            auto face = EgLab::Common::makeShared<EgLab::RE::RenderFace>();
            std::printf("face created\n");
        }
        std::printf("face destroyed\n");
        return 0;
    }

    if (std::strcmp(argv[1], "--raw") == 0)
    {
        std::printf("raw new/delete\n");
        {
            EgLab::RE::RenderNode *node = new EgLab::RE::RenderNode();
            std::printf("raw created\n");
            delete node;
        }
        std::printf("raw destroyed\n");
        {
            std::printf("sizeof(RenderNode)=%zu\n", sizeof(EgLab::RE::RenderNode));
            EgLab::RE::RenderNode *node = new EgLab::RE::RenderNode();
            std::printf("raw2 created\n");
            delete node;
            std::printf("raw2 destroyed\n");
        }
        return 0;
    }

    auto document = EgLab::Common::makeShared<EgLab::GE::GeometryDocument>();
    EgLab::GE::GeometryImporter importer;
    const EgLab::GE::GeometryImportReport report =
        importer.importFile(EgLab::Common::String(argv[1]), *document);
    std::printf("import status=%d msg=%s\n", static_cast<int>(report.getStatus()),
                report.getMessage().c_str());
    if (!report.isSucceed()) return 1;
    PROBE_CHECK("import");

    auto mesh = convert(*document);
    if (!mesh) return 1;
    PROBE_CHECK("convert");

    std::printf("step: translator\n");
    EgLab::RE::MeshPrimitiveCreator creator(mesh);
    PROBE_CHECK("translator ctor");

    std::printf("probe B: MeshToTriFace\n");
    std::printf("  line all=%zu\n", creator.getTranslator()->getBoundaryLineIdx().size());
    PROBE_CHECK("line all");
    std::printf("  boundary=%zu\n", creator.getTranslator()->getBoundaryElementEdgeIdx().size());
    PROBE_CHECK("boundary edges");
    std::printf("  faces idx=%zu nodes=%zu\n",
                creator.getTranslator()->getBoundaryFaceIdx().size(),
                creator.getTranslator()->getBoundaryFaceNodeIdx().size());
    PROBE_CHECK("face idx");

    std::printf("probe C1: makeShared<RenderNode> only\n");
    {
        auto node = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
        PROBE_CHECK("c1 created");
    }
    PROBE_CHECK("c1 destroyed");

    std::printf("probe C2: makeShared<RenderLine> only\n");
    {
        auto line = EgLab::Common::makeShared<EgLab::RE::RenderLine>();
        PROBE_CHECK("c2 created");
    }
    PROBE_CHECK("c2 destroyed");

    std::printf("probe C3: makeShared<RenderFace> only\n");
    {
        auto face = EgLab::Common::makeShared<EgLab::RE::RenderFace>();
        PROBE_CHECK("c3 created");
    }
    PROBE_CHECK("c3 destroyed");

    std::printf("probe C4: node + upcast\n");
    {
        auto node = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
        {
            EgLab::Common::SharedPtr<EgLab::RE::RenderPrimitive> base(node);
            PROBE_CHECK("c4 upcast");
        }
        PROBE_CHECK("c4 base destroyed");
    }
    PROBE_CHECK("c4 node destroyed");

    std::printf("probe C5: node + upcast + downcast\n");
    {
        auto node = EgLab::Common::makeShared<EgLab::RE::RenderNode>();
        {
            EgLab::Common::SharedPtr<EgLab::RE::RenderPrimitive> base(node);
            auto back = EgLab::Common::dynamicSharedPtrCast<EgLab::RE::RenderNode>(base);
            PROBE_CHECK("c5 downcast");
        }
        PROBE_CHECK("c5 base+back destroyed");
    }
    PROBE_CHECK("c5 node destroyed");

    std::printf("PROBE OK\n");
    return 0;
}