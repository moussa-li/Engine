#include "Work/RenderWork.hpp"

#include <chrono>
#include <cstring>

#include "Common/Log.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Window.hpp"
#include "Work/UIWork.hpp"

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

        if (_window)
        {
            _window->start();
        }

        _running = true;
        _thread = std::thread([this]() {
            UIWork::instance().start();

            while (_running.load())
            {
                // Drain the cross-thread mesh packet queue on the render lane.
                drainMeshUpdateQueue();

                // UI frame draw and ImGui state update move into the render
                // worker thread where the GL/GLFW context is started and owned.
                float currentFrame = _window->getTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;

                _window->activeContext();
                _scene->update(deltaTime);
                _renderer->update(deltaTime);
                _renderer->draw(_scene, _camera);
                _window->deactiveContext();

                UIWork::instance().renderFrame();

                if (_window)
                {
                    _window->deal();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });
    }

    void RenderWork::bindRenderer(const Common::SharedPtr<EgLab::RE::Renderer>& renderer)
    {
        _renderer = renderer;
    }

    void RenderWork::bindScene(const Common::SharedPtr<EgLab::RE::Scene>& scene)
    {
        _scene = scene;
    }

    void RenderWork::bindCamera(const Common::SharedPtr<EgLab::RE::Camera>& camera)
    {
        _camera = camera;
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

    void RenderWork::queueMeshUpdate(const UpdateMeshParam& params)
    {
        std::lock_guard<std::mutex> lock(_meshUpdateMutex);
        UpdateMeshParam copy = params;
        copy.data = new uint8_t[params.dataSize];
        std::memcpy(copy.data, params.data, params.dataSize);
        _meshUpdateQueue.pushBack(copy);
    }

    void RenderWork::drainMeshUpdateQueue()
    {
        Common::DynamicArray<UpdateMeshParam> localPackets;
        {
            std::lock_guard<std::mutex> lock(_meshUpdateMutex);
            while (!_meshUpdateQueue.empty())
            {
                localPackets.pushBack(_meshUpdateQueue[0]);
                for (size_t i = 1; i < _meshUpdateQueue.size(); ++i)
                {
                    _meshUpdateQueue[i - 1] = _meshUpdateQueue[i];
                }
                _meshUpdateQueue.popBack();
            }
        }

        _window->activeContext();
        for (size_t i = 0; i < localPackets.size(); ++i)
        {
            materializeMesh(localPackets[i]);
            delete[] localPackets[i].data;
        }
        _window->deactiveContext();
    }

    void RenderWork::materializeMesh(const UpdateMeshParam& params)
    {
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

        const uint32_t payloadSize = wire.totalDataSize;
        const uint32_t offsets[] = {
            wire.nodeIdsOffset,   wire.nodeDataOffset, wire.elemIdsOffset,
            wire.elemTypesOffset, wire.elemDataOffset,
        };
        for (uint32_t offset : offsets)
        {
            if (offset > payloadSize)
            {
                LOG(WARNING) << "RenderWork::onUpdateMesh() skipped invalid mesh offset";
                return;
            }
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
            delete[] pod.data;
            return;
        }

        EgLab::RE::MeshPrimitiveCreator creator(mesh);
        auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
        auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
        auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();

        EgLab::Common::SharedPtr<EgLab::RE::Shader> nodeShader;
        EgLab::RE::ShaderLib::instance().getNodeShader(nodeShader);
        EgLab::Common::SharedPtr<EgLab::RE::Shader> lineShader;
        EgLab::RE::ShaderLib::instance().getLineShader(lineShader);
        EgLab::Common::SharedPtr<EgLab::RE::Shader> faceShader;
        EgLab::RE::ShaderLib::instance().getFaceShader(faceShader);
        _scene->addPrimitive(nodeShader, nodePrimitive);
        _scene->addPrimitive(lineShader, linePrimitive);
        _scene->addPrimitive(faceShader, facePrimitive);
        _camera->fitView(_scene->getBounds(), 2);
    }

    void RenderWork::onUpdateMesh(const UpdateMeshParam& params)
    {
        queueMeshUpdate(params);
    }
} // namespace EgLab::Platform
