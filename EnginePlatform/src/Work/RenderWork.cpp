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
        RenderUpdate(RenderWork* worker, Common::SharedPtr<ME::Mesh> mesh,
                     Common::SharedPtr<ME::Mesh> sourceMesh)
            : _mesh(mesh), _sourceMesh(sourceMesh), _worker(worker)
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

            RenderWork::RenderEvent event{};
            event.type = RenderWork::RenderEventType::RenderPacket;
            event.packet = {nodePrimitive, linePrimitive, facePrimitive, _sourceMesh};
            _worker->enqueueRenderEvent(Common::move(event));
            perf.stop();

            return Common::Return::Succeed;
        }

    private:
        Common::SharedPtr<ME::Mesh> _mesh;
        Common::SharedPtr<ME::Mesh> _sourceMesh;
        RenderWork* _worker;
    };

    class RenderEventCommand : public ICommand
    {
    public:
        RenderEventCommand(RenderWork* worker, RenderWork::RenderEvent event)
            : _worker(worker), _event(Common::move(event))
        {
        }

        Common::Return exec() override
        {
            _worker->enqueueRenderEvent(Common::move(_event));
            return Common::Return::Succeed;
        }

    private:
        RenderWork* _worker;
        RenderWork::RenderEvent _event;
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
                // UI frame draw and ImGui state update move into the render
                // worker thread where the GL/GLFW context is started and owned.
                float currentFrame = _window->getTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;

                _window->activeContext();
                drainRenderEvents();
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
        RenderEvent event{};
        event.type = RenderEventType::MeshUpdate;
        event.mesh = mesh;
        event.payload.resize(wireSize + pod.totalDataSize);
        std::memcpy(event.payload.data(), &wire, wireSize);
        std::memcpy(event.payload.data() + wireSize, pod.data, pod.totalDataSize);
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::queueMeshUpdate(const UpdateMeshParam& params,
                                     const Common::SharedPtr<ME::Mesh>& sourceMesh)
    {
        if (params.dataSize == 0 || params.data == nullptr) return;

        RenderEvent event{};
        event.type = RenderEventType::MeshUpdate;
        event.mesh = sourceMesh;
        event.payload.assign(params.data, params.data + params.dataSize);
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::queueRenderEvent(RenderEvent event)
    {
        auto command = Common::SharedPtr<RenderEventCommand>(this, Common::move(event));
        _renderUpdateBus.dispatch(command);
    }

    void RenderWork::enqueueRenderEvent(RenderEvent event)
    {
        std::lock_guard<std::mutex> lock(_renderEventMutex);
        _renderEventQueue.push_back(Common::move(event));
    }

    void RenderWork::drainRenderEvents()
    {
        std::deque<RenderEvent> events;
        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            events.swap(_renderEventQueue);
        }

        for (auto& event : events)
        {
            switch (event.type)
            {
            case RenderEventType::MeshUpdate:
            {
                if (event.mesh && isMeshRemoved(event.mesh.get()))
                {
                    break;
                }
                UpdateMeshParam params{};
                params.dataSize = static_cast<uint32_t>(event.payload.size());
                params.data = event.payload.data();
                materializeMesh(params, event.mesh);
                break;
            }
            case RenderEventType::RenderPacket:
            {
                RenderPacket& packet = event.packet;
                if (packet.sourceMesh && isMeshRemoved(packet.sourceMesh.get()))
                {
                    break;
                }
                if (packet.sourceMesh)
                {
                    Common::Vector4f color(0.8f, 0.8f, 0.8f, 1.0f);
                    for (const auto& meshColor : _meshColors)
                    {
                        if (meshColor.mesh == packet.sourceMesh)
                        {
                            color = meshColor.color;
                            break;
                        }
                    }
                    auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(packet.renderFace);
                    if (face)
                    {
                        face->setColor(color);
                    }
                }
                packet.renderNode->setup();
                packet.renderLine->setup();
                packet.renderFace->setup();

                Common::SharedPtr<RE::Shader> nodeShader;
                Common::SharedPtr<RE::Shader> lineShader;
                Common::SharedPtr<RE::Shader> faceShader;
                RE::ShaderLib::instance().getNodeShader(nodeShader);
                RE::ShaderLib::instance().getLineShader(lineShader);
                RE::ShaderLib::instance().getFaceShader(faceShader);
                _scene->addPrimitive(nodeShader, packet.renderNode);
                _scene->addPrimitive(lineShader, packet.renderLine);
                _scene->addPrimitive(faceShader, packet.renderFace);
                if (packet.sourceMesh)
                {
                    _activeMeshPackets.pushBack(packet);
                }
                _camera->fitView(_scene->getBounds(), 2);
                break;
            }
            case RenderEventType::MeshColor:
            {
                if (!event.mesh || isMeshRemoved(event.mesh.get()))
                {
                    break;
                }
                bool found = false;
                for (auto& meshColor : _meshColors)
                {
                    if (meshColor.mesh == event.mesh)
                    {
                        meshColor.color = event.color;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    _meshColors.pushBack(MeshColor{event.mesh, event.color});
                }
                for (auto& packet : _activeMeshPackets)
                {
                    if (packet.sourceMesh != event.mesh)
                    {
                        continue;
                    }
                    auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(packet.renderFace);
                    if (face)
                    {
                        face->setColor(event.color);
                    }
                }
                break;
            }
            case RenderEventType::MeshRemoval:
            {
                if (!event.mesh || isMeshRemoved(event.mesh.get()))
                {
                    break;
                }
                _removedMeshes.pushBack(event.mesh.get());

                Common::SharedPtr<RE::Shader> nodeShader;
                Common::SharedPtr<RE::Shader> lineShader;
                Common::SharedPtr<RE::Shader> faceShader;
                RE::ShaderLib::instance().getNodeShader(nodeShader);
                RE::ShaderLib::instance().getLineShader(lineShader);
                RE::ShaderLib::instance().getFaceShader(faceShader);
                Common::DynamicArray<RenderPacket> remainingPackets;
                for (auto packet : _activeMeshPackets)
                {
                    if (packet.sourceMesh != event.mesh)
                    {
                        remainingPackets.pushBack(packet);
                        continue;
                    }
                    _scene->removePrimitive(nodeShader, packet.renderNode);
                    _scene->removePrimitive(lineShader, packet.renderLine);
                    _scene->removePrimitive(faceShader, packet.renderFace);
                }
                _activeMeshPackets = Common::move(remainingPackets);

                Common::DynamicArray<MeshColor> remainingColors;
                for (auto meshColor : _meshColors)
                {
                    if (meshColor.mesh != event.mesh)
                    {
                        remainingColors.pushBack(meshColor);
                    }
                }
                _meshColors = Common::move(remainingColors);
                if (_requestedHighlightedMesh == event.mesh)
                {
                    _requestedHighlightedMesh = nullptr;
                }
                break;
            }
            case RenderEventType::Highlight:
                _requestedHighlightedMesh =
                    event.mesh && !isMeshRemoved(event.mesh.get()) ? event.mesh : nullptr;
                break;
            }
        }
    }

    void RenderWork::materializeMesh(const UpdateMeshParam& params,
                                     const Common::SharedPtr<ME::Mesh>& sourceMesh)
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

        auto updater = Common::SharedPtr<RenderUpdate>(this, mesh, sourceMesh);
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
        RenderEvent event{};
        event.type = RenderEventType::Highlight;
        event.mesh = mesh;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::setMeshColor(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                  const Common::Vector4f& color)
    {
        if (!mesh)
        {
            LOG(WARNING) << "RenderWork::setMeshColor() skipped null mesh";
            return;
        }

        RenderEvent event{};
        event.type = RenderEventType::MeshColor;
        event.mesh = mesh;
        event.color = color;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::removeMesh(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        RenderEvent event{};
        event.type = RenderEventType::MeshRemoval;
        event.mesh = mesh;
        queueRenderEvent(Common::move(event));
    }

    bool RenderWork::isMeshRemoved(const ME::Mesh* mesh)
    {
        for (ME::Mesh* removedMesh : _removedMeshes)
        {
            if (removedMesh == mesh)
            {
                return true;
            }
        }
        return false;
    }

    void RenderWork::updateMeshHighlight()
    {
        Common::SharedPtr<ME::Mesh> requestedMesh = _requestedHighlightedMesh;
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
            RE::ShaderLib::instance().getHighlightShader(_highlightShader);
        }

        RE::MeshPrimitiveCreator creator(_activeHighlightedMesh);
        auto primitive = creator.getPrimitive<RE::RenderFace>();
        auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(primitive);
        face->setColor(Common::Vector4f(1.0f, 0.75f, 0.1f, 1.0f));
        face->setHighlightScale(1.05f);
        primitive->setup();
        _highlightPrimitive = primitive;

        if (_scene->addPrimitive(_highlightShader, _highlightPrimitive) != Common::Return::Succeed)
        {
            LOG(ERROR) << "RenderWork::updateMeshHighlight() failed to add mesh highlight";
            _highlightPrimitive = nullptr;
        }
    }
} // namespace EgLab::Platform
