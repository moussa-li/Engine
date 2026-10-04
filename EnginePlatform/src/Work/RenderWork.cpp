#include "Work/RenderWork.hpp"

#include <chrono>
#include <cstring>

#include "Common/Log.hpp"
#include "Common/Performance.hpp"
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

    class RenderUpdate : public ICommand
    {
    public:
        RenderUpdate(RenderWork* worker, Common::SharedPtr<ME::Mesh> mesh)
            : _mesh(mesh), _worker(worker)
        {
        }

        Common::Return exec()
        {
            Common::Performance perf;
            EgLab::RE::MeshPrimitiveCreator creator(_mesh);
            perf.start();
            auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
            auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
            auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();
            std::lock_guard<std::mutex> lock(_worker->_renderUpdateMutex);
            RenderPacket packet = {nodePrimitive, linePrimitive, facePrimitive};
            _worker->_renderUpdateQueue.pushBack(packet);
            perf.stop();

            return Common::Return::Succeed;
        }

    private:
        Common::SharedPtr<ME::Mesh> _mesh;
        RenderWork* _worker;
    };

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

                drainRenderQueue();

                // UI frame draw and ImGui state update move into the render
                // worker thread where the GL/GLFW context is started and owned.
                float currentFrame = _window->getTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;

                _window->activeContext();
                updateMeshHighlight();
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

        std::lock_guard<std::mutex> lock(_meshUpdateMutex);
        for (auto it = _meshUpdateQueue.begin(); it.hasNext(); it.next())
        {
            delete[] it.data().data;
            it.data().data = nullptr;
            it.data().dataSize = 0;
        }
        _meshUpdateQueue.clear();
    }

    void RenderWork::subscribe(EventId eventId)
    {
        (void)eventId;
    }

    void RenderWork::queueMeshUpdate(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        if (mesh == nullptr)
        {
            LOG(WARNING) << "RenderWork::queueMeshUpdate() skipped null mesh";
            return;
        }

        const auto pod = EgLab::ME::MeshToPOD::serialize(mesh);
        if (pod.totalDataSize == 0 || pod.data == nullptr)
        {
            LOG(ERROR) << "RenderWork::queueMeshUpdate() failed to serialize mesh";
            return;
        }

        MeshPODWireHeader wire{};
        wire.nodeNumber = pod.nodeNumber;
        wire.elemNumber = pod.elemNumber;
        wire.nodeIdsOffset = pod.nodeIdsOffset;
        wire.nodeDataOffset = pod.nodeDataOffset;
        wire.elemIdsOffset = pod.elemIdsOffset;
        wire.elemTypesOffset = pod.elemTypesOffset;
        wire.elemDataOffset = pod.elemDataOffset;
        wire.totalDataSize = pod.totalDataSize;

        const uint32_t wireSize = static_cast<uint32_t>(sizeof(MeshPODWireHeader));
        UpdateMeshParam params{};
        params.dataSize = wireSize + pod.totalDataSize;
        params.data = new uint8_t[params.dataSize];
        std::memcpy(params.data, &wire, wireSize);
        std::memcpy(params.data + wireSize, pod.data, pod.totalDataSize);
        queueMeshUpdate(params);
        delete[] params.data;
    }

    void RenderWork::queueMeshUpdate(const UpdateMeshParam& params)
    {
        if (params.dataSize == 0 || params.data == nullptr) return;

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

    void RenderWork::drainRenderQueue()
    {
        Common::DynamicArray<RenderPacket> localPackets;
        {
            std::lock_guard<std::mutex> lock(_renderUpdateMutex);
            while (!_renderUpdateQueue.empty())
            {
                localPackets.pushBack(_renderUpdateQueue[0]);
                for (size_t i = 1; i < _renderUpdateQueue.size(); ++i)
                {
                    _renderUpdateQueue[i - 1] = _renderUpdateQueue[i];
                }
                _renderUpdateQueue.popBack();
            }
        }

        _window->activeContext();
        for (size_t i = 0; i < localPackets.size(); ++i)
        {
            auto nodePrimitive = localPackets[i].renderNode;
            auto linePrimitive = localPackets[i].renderLine;
            auto facePrimitive = localPackets[i].renderFace;
            nodePrimitive->setup();
            linePrimitive->setup();
            facePrimitive->setup();

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
            return;
        }

        auto updater = Common::SharedPtr<RenderUpdate>(this, mesh);
        _renderUpdateBus.dispatch(updater);
    }

    void RenderWork::fitView()
    {
        _camera->fitView(_scene->getBounds(), 2);
    }

    void RenderWork::onUpdateMesh(const UpdateMeshParam& params)
    {
        queueMeshUpdate(params);
    }

    void RenderWork::onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        queueMeshUpdate(mesh);
    }

    void RenderWork::setHighlightedMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        std::lock_guard<std::mutex> lock(_highlightMutex);
        _requestedHighlightedMesh = mesh;
    }

    void RenderWork::updateMeshHighlight()
    {
        Common::SharedPtr<ME::Mesh> requestedMesh;
        {
            std::lock_guard<std::mutex> lock(_highlightMutex);
            requestedMesh = _requestedHighlightedMesh;
        }

        if (requestedMesh == _activeHighlightedMesh)
        {
            return;
        }

        if (_highlightPrimitive)
        {
            const Common::Return result =
                _scene->removePrimitive(_highlightShader, _highlightPrimitive);
            if (result != Common::Return::Succeed)
            {
                LOG(WARNING) << "RenderWork::updateMeshHighlight() could not remove old highlight";
            }
            _highlightPrimitive = nullptr;
        }
        _activeHighlightedMesh = requestedMesh;

        if (!_activeHighlightedMesh)
        {
            return;
        }

        if (!_highlightShader)
        {
            Common::String source(
                "#shader vertex\n"
                "#version 330 core\n"
                "layout(location = 0) in vec3 position;\n"
                "uniform mat4 proj;\n"
                "uniform mat4 view;\n"
                "void main() { gl_Position = proj * view * vec4(position, 1.0); }\n"
                "#shader fragment\n"
                "#version 330 core\n"
                "out vec4 color;\n"
                "uniform vec4 u_Color;\n"
                "void main() { color = u_Color; }\n");
            _highlightShader = Common::makeShared<RE::Shader>(source);
        }

        RE::MeshPrimitiveCreator creator(_activeHighlightedMesh);
        auto primitive = creator.getPrimitive<RE::RenderLine>();
        auto line = Common::dynamicSharedPtrCast<RE::RenderLine>(primitive);
        line->setColor(Common::Vector4f(1.0f, 0.75f, 0.1f, 1.0f));
        line->setup();
        _highlightPrimitive = primitive;

        if (_scene->addPrimitive(_highlightShader, _highlightPrimitive) != Common::Return::Succeed)
        {
            LOG(ERROR) << "RenderWork::updateMeshHighlight() failed to add mesh highlight";
            _highlightPrimitive = nullptr;
        }
    }
} // namespace EgLab::Platform
