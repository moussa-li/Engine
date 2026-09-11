#include "Work/RenderWork.hpp"

#include <GL/glew.h>

#include <chrono>
#include <cstring>

#include "Common/Log.hpp"
#include "GLFW/glfw3.h"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Window.hpp"

namespace EgLab::Platform
{
    namespace
    {
        struct MeshPODWireHeader
        {
            uint32_t nodeNumber{0};
            uint32_t elemNumber{0};
            uint32_t nodeIdsOffset{0};
            uint32_t nodeDataOffset{0};
            uint32_t elemIdsOffset{0};
            uint32_t elemTypesOffset{0};
            uint32_t elemDataOffset{0};
            uint32_t totalDataSize{0};
        };
    } // namespace

    RenderWork::RenderWork()
    {
    }

    RenderWork::~RenderWork()
    {
        stop();
    }

    void RenderWork::bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window)
    {
        _window = window;
    }

    void RenderWork::start()
    {
        if (_running.load())
        {
            return;
        }

        _running = true;
        _thread = std::thread([this]() {
            if (_window)
            {
                glfwMakeContextCurrent(_window->getNative());
            }

            while (_running.load())
            {
                // Dedicated render lane should stay married to UpdateMesh and other
                // render-only event sources. CommandManager remains the event source,
                // but RenderWork owns the refresh callback boundary.
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });
    }

    void RenderWork::stop()
    {
        _running = false;
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    void RenderWork::subscribe(EventId eventId)
    {
        (void)eventId;
    }

    void RenderWork::onUpdateMesh(const UpdateMeshParam& params)
    {
        if (_window)
        {
            glfwMakeContextCurrent(_window->getNative());
        }

        if (params.dataSize <= sizeof(MeshPODWireHeader) || params.data == nullptr)
        {
            LOG(WARNING) << "RenderWork::onUpdateMesh() skipped empty or invalid wire payload";
            return;
        }

        MeshPODWireHeader wire{};
        std::memcpy(&wire, params.data, sizeof(MeshPODWireHeader));

        if (wire.totalDataSize == 0 ||
            wire.totalDataSize > params.dataSize - sizeof(MeshPODWireHeader))
        {
            LOG(WARNING) << "RenderWork::onUpdateMesh() skipped malformed nested mesh packet";
            return;
        }

        EgLab::ME::MeshPOD pod{};
        pod.nodeNumber = wire.nodeNumber;
        pod.elemNumber = wire.elemNumber;
        pod.nodeIdsOffset = wire.nodeIdsOffset;
        pod.nodeDataOffset = wire.nodeDataOffset;
        pod.elemIdsOffset = wire.elemIdsOffset;
        pod.elemTypesOffset = wire.elemTypesOffset;
        pod.elemDataOffset = wire.elemDataOffset;
        pod.totalDataSize = wire.totalDataSize;

        pod.data = new uint8_t[pod.totalDataSize];
        std::memcpy(pod.data, params.data + sizeof(MeshPODWireHeader), pod.totalDataSize);

        auto mesh = EgLab::ME::MeshToPOD::deserialize(pod);
        if (!mesh)
        {
            LOG(ERROR) << "RenderWork::onUpdateMesh() failed to materialize mesh";
            return;
        }

        EgLab::RE::MeshPrimitiveCreator creator(mesh);
        auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
        auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
        auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();

        (void)nodePrimitive;
        (void)linePrimitive;
        (void)facePrimitive;

        LOG(INFO) << "RenderWork::onUpdateMesh() received MeshPOD and refreshed render primitives";
    }
} // namespace EgLab::Platform
