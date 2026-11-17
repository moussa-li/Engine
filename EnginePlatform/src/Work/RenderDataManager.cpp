#include "Work/RenderDataManager.hpp"

#include "Common/BBox.hpp"
#include "Common/HashMap.hpp"
#include "Common/Log.hpp"
#include "MeshEngine/Algorithm/ExtractSilhouette.hpp"
#include "MeshEngine/MeshData/HEMesh.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"

namespace EgLab::Platform
{
    RenderDataManager::RenderDataManager() = default;

    RenderDataManager::~RenderDataManager() = default;

    void RenderDataManager::bindScene(const Common::SharedPtr<RE::Scene>& scene)
    {
        _scene = scene;
    }

    void RenderDataManager::bindCamera(const Common::SharedPtr<RE::Camera>& camera)
    {
        _camera = camera;
    }

    void RenderDataManager::bindMeshSelector(const Common::SharedPtr<RE::MeshSelector>& selector)
    {
        _meshSelector = selector;
    }

    void RenderDataManager::releaseResources()
    {
        _activeMeshes.clear();
        _meshColors.clear();
        _elementVisibilityStates.clear();
        _removedMeshes.clear();
        _requestedHighlightedMeshes.clear();
        _activeHighlightedMeshes.clear();
        _highlightPrimitives.clear();
        _highlightContourPrimitives.clear();
        _heMeshHighlightPrimitives.clear();
        _locatePrimitives.clear();
        _locateMesh = nullptr;
        _locateTargetFace = nullptr;
        _locateTargetMesh = nullptr;
        _locateElementIds.clear();
        _highlightShader = nullptr;
        _heMeshHighlightShader = nullptr;
        _meshSelector = nullptr;
        _scene = nullptr;
        _camera = nullptr;
    }

    std::mutex& RenderDataManager::stateMutex() const
    {
        return _renderStateMutex;
    }

    bool RenderDataManager::recordMeshColor(const Common::SharedPtr<ME::Mesh>& mesh,
                                            const Common::Vector4f& color)
    {
        if (!mesh || isMeshRemoved(mesh.get()))
        {
            return false;
        }

        for (auto& meshColor : _meshColors)
        {
            if (meshColor.mesh == mesh)
            {
                meshColor.color = color;
                return true;
            }
        }
        _meshColors.pushBack(MeshColor{mesh, color});
        return true;
    }

    bool RenderDataManager::recordMeshRemoval(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh || isMeshRemoved(mesh.get()))
        {
            return false;
        }

        _removedMeshes.pushBack(mesh.get());
        {
            Common::DynamicArray<MeshColor> remainingColors;
            for (const auto& meshColor : _meshColors)
            {
                if (meshColor.mesh != mesh)
                {
                    remainingColors.pushBack(meshColor);
                }
            }
            _meshColors = Common::move(remainingColors);

            Common::DynamicArray<Common::SharedPtr<ME::Mesh>> remainingMeshes;
            for (const auto& highlightedMesh : _requestedHighlightedMeshes)
            {
                if (highlightedMesh != mesh)
                {
                    remainingMeshes.pushBack(highlightedMesh);
                }
            }
            _requestedHighlightedMeshes = Common::move(remainingMeshes);

            Common::DynamicArray<MeshElementVisibilityState> remainingVisibilityStates;
            for (const auto& state : _elementVisibilityStates)
            {
                if (state.mesh != mesh)
                {
                    remainingVisibilityStates.pushBack(state);
                }
            }
            _elementVisibilityStates = Common::move(remainingVisibilityStates);
        }
        return true;
    }

    bool RenderDataManager::isMeshRemoved(const ME::Mesh* mesh) const
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

    void RenderDataManager::replaceRequestedHighlightedMeshes(
        const Common::DynamicArray<Common::SharedPtr<ME::Mesh>>& meshes)
    {
        _requestedHighlightedMeshes.clear();
        for (const auto& mesh : meshes)
        {
            if (mesh && !isMeshRemoved(mesh.get()))
            {
                _requestedHighlightedMeshes.pushBack(mesh);
            }
        }
    }

    void RenderDataManager::setHighlightColor(const Common::Vector4f& color)
    {
        _highlightColor = color;
    }

    void RenderDataManager::setNodePointSize(float pointSize)
    {
        _nodePointSize = pointSize;
    }

    void RenderDataManager::setLineWidth(float lineWidth)
    {
        _lineWidth = lineWidth;
    }

    void RenderDataManager::setFaceShrink(float shrink)
    {
        _faceShrink = shrink;
    }

    void RenderDataManager::setLocateState(const Common::SharedPtr<ME::Mesh>& mesh,
                                           const Common::DynamicArray<uint32_t>& elementIds)
    {
        _locateMesh = mesh;
        _locateElementIds = elementIds;
    }

    void RenderDataManager::fitView(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!_camera)
        {
            return;
        }

        if (mesh)
        {
            Common::BBox<RE::Scalar, 3> bounds;
            for (const auto& entry : _activeMeshes)
            {
                if (entry.mesh != mesh)
                {
                    continue;
                }
                if (entry.packet.renderNode)
                {
                    bounds.merge(entry.packet.renderNode->getBounds());
                }
                if (entry.packet.renderLine)
                {
                    bounds.merge(entry.packet.renderLine->getBounds());
                }
                if (entry.packet.renderFace)
                {
                    bounds.merge(entry.packet.renderFace->getBounds());
                }
                break;
            }
            if (!bounds.isEmpty())
            {
                _camera->fitView(bounds, 2);
            }
            return;
        }

        if (_scene)
        {
            _camera->fitView(_scene->getBounds(), 2);
        }
    }

    bool RenderDataManager::getMeshCenter(const Common::SharedPtr<ME::Mesh>& mesh,
                                          RE::CoordType& center) const
    {
        if (!mesh)
        {
            return false;
        }

        Common::BBox<RE::Scalar, 3> bounds;
        for (const auto& entry : _activeMeshes)
        {
            if (entry.mesh != mesh)
            {
                continue;
            }
            if (entry.packet.renderNode)
            {
                bounds.merge(entry.packet.renderNode->getBounds());
            }
            if (entry.packet.renderLine)
            {
                bounds.merge(entry.packet.renderLine->getBounds());
            }
            if (entry.packet.renderFace)
            {
                bounds.merge(entry.packet.renderFace->getBounds());
            }
            break;
        }

        if (bounds.isEmpty())
        {
            return false;
        }

        center = (bounds.min() + bounds.max()) * 0.5f;
        return true;
    }

    Common::Return RenderDataManager::applyMeshUpdate(const MeshUpdateEvent& event)
    {
        MeshRenderEntry entry;
        entry.mesh = event.mesh;
        entry.translator = event.translator;
        entry.packet = event.packet;
        RE::ShaderLib::instance().getNodeShader(entry.packet.nodeShader);
        RE::ShaderLib::instance().getLineShader(entry.packet.lineShader);
        RE::ShaderLib::instance().getFaceShader(entry.packet.faceShader);

        const Common::Return result = uploadRenderPacket(entry);
        if (result == Common::Return::Succeed && entry.mesh)
        {
            MeshElementVisibilityState* visibilityState = findElementVisibilityState(entry.mesh);
            if (visibilityState != nullptr)
            {
                rebuildMeshSurface(entry.mesh, collectHiddenElementIds(*visibilityState));
            }
            else if (_locateMesh && _locateMesh == entry.mesh)
            {
                updateLocateHighlight();
            }
        }
        return result;
    }

    void RenderDataManager::applyMeshColor(const Common::SharedPtr<ME::Mesh>& mesh,
                                           const Common::Vector4f& color)
    {
        if (!mesh)
        {
            return;
        }

        for (auto& entry : _activeMeshes)
        {
            if (entry.mesh != mesh)
            {
                continue;
            }
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(entry.packet.renderFace);
            if (face)
            {
                face->setColor(color);
            }
        }
        if (_locateMesh && _locateMesh == mesh)
        {
            updateLocateHighlight();
        }
    }

    void RenderDataManager::applyMeshRemoval(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        Common::DynamicArray<MeshRenderEntry> remainingEntries;
        for (auto& entry : _activeMeshes)
        {
            if (entry.mesh != mesh)
            {
                remainingEntries.pushBack(entry);
                continue;
            }
            if (_meshSelector && entry.mesh)
            {
                _meshSelector->unregisterMesh(entry.mesh.get());
            }
            _scene->removePrimitive(entry.packet.nodeShader, entry.packet.renderNode);
            _scene->removePrimitive(entry.packet.lineShader, entry.packet.renderLine);
            _scene->removePrimitive(entry.packet.faceShader, entry.packet.renderFace);
        }
        _activeMeshes = Common::move(remainingEntries);

        if (_locateMesh && _locateMesh == mesh)
        {
            Common::SharedPtr<RE::Shader> highlightShader;
            RE::ShaderLib::instance().getHighlightShader(highlightShader);
            for (const auto& primitive : _locatePrimitives)
            {
                _scene->removePrimitive(highlightShader, primitive);
            }
            _locatePrimitives.clear();
            _locateMesh = nullptr;
            _locateElementIds.clear();
            _locateTargetFace = nullptr;
            _locateTargetMesh = nullptr;
        }
        updateMeshHighlight();
    }

    void RenderDataManager::applyHighlight()
    {
        updateMeshHighlight();
    }

    void RenderDataManager::applyHighlightColor()
    {
        for (const auto& primitive : _highlightPrimitives)
        {
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(primitive);
            if (face)
            {
                face->setColor(_highlightColor);
            }
        }
    }

    void RenderDataManager::applyNodePointSize(float pointSize)
    {
        for (auto& entry : _activeMeshes)
        {
            auto node = Common::dynamicSharedPtrCast<RE::RenderNode>(entry.packet.renderNode);
            if (node)
            {
                node->setPointSize(pointSize);
            }
        }
    }

    void RenderDataManager::applyLineWidth(float lineWidth)
    {
        for (auto& entry : _activeMeshes)
        {
            auto line = Common::dynamicSharedPtrCast<RE::RenderLine>(entry.packet.renderLine);
            if (line)
            {
                line->setLineWidth(lineWidth);
            }
        }
    }

    void RenderDataManager::applyFaceShrink(float shrink)
    {
        for (auto& entry : _activeMeshes)
        {
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(entry.packet.renderFace);
            if (face)
            {
                face->setFaceShrink(shrink);
            }
        }
    }

    void RenderDataManager::applyLocate()
    {
        updateLocateHighlight();
    }

    void RenderDataManager::applyElementsVisibility(const ElementsVisibilityEvent& event)
    {
        updateElementsVisibility(event);
    }

    void RenderDataManager::applyHEMeshHighlight(const HEMeshHighlightEvent& event)
    {
        updateHEMeshHighlight(event);
    }

    Common::Return RenderDataManager::uploadRenderPacket(const MeshRenderEntry& entry)
    {
        if (!_scene || !_camera)
        {
            return Common::Return::Failed;
        }
        if (entry.mesh && isMeshRemoved(entry.mesh.get()))
        {
            return Common::Return::Succeed;
        }

        if (entry.mesh)
        {
            Common::Vector4f color(0.8f, 0.8f, 0.8f, 1.0f);
            for (const auto& meshColor : _meshColors)
            {
                if (meshColor.mesh == entry.mesh)
                {
                    color = meshColor.color;
                    break;
                }
            }
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(entry.packet.renderFace);
            if (face)
            {
                face->setColor(color);
            }
        }

        entry.packet.renderNode->setup();
        entry.packet.renderLine->setup();
        entry.packet.renderFace->setup();

        auto node = Common::dynamicSharedPtrCast<RE::RenderNode>(entry.packet.renderNode);
        if (node)
        {
            node->setPointSize(_nodePointSize);
        }
        auto line = Common::dynamicSharedPtrCast<RE::RenderLine>(entry.packet.renderLine);
        if (line)
        {
            line->setLineWidth(_lineWidth);
        }
        auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(entry.packet.renderFace);
        if (face)
        {
            face->setFaceShrink(_faceShrink);
        }

        if (_meshSelector && entry.mesh && entry.translator)
        {
            _meshSelector->registerMesh(entry.mesh, entry.translator, entry.packet.renderNode,
                                        entry.packet.renderLine, entry.packet.renderFace);
        }

        _scene->addPrimitive(entry.packet.nodeShader, entry.packet.renderNode);
        _scene->addPrimitive(entry.packet.lineShader, entry.packet.renderLine);
        _scene->addPrimitive(entry.packet.faceShader, entry.packet.renderFace);
        if (entry.mesh)
        {
            _activeMeshes.pushBack(entry);
        }
        return Common::Return::Succeed;
    }

    void RenderDataManager::updateMeshHighlight()
    {
        bool highlightsMatch =
            _requestedHighlightedMeshes.size() == _activeHighlightedMeshes.size();
        for (size_t index = 0; highlightsMatch && index < _requestedHighlightedMeshes.size();
             ++index)
        {
            highlightsMatch = _requestedHighlightedMeshes[index] == _activeHighlightedMeshes[index];
        }
        if (highlightsMatch)
        {
            return;
        }

        for (size_t index = 0; index < _highlightPrimitives.size(); ++index)
        {
            auto primitive = _highlightPrimitives[index];
            const Common::Return result = _scene->removePrimitive(_highlightShader, primitive);
            if (result != Common::Return::Succeed)
            {
                LOG(WARNING)
                    << "RenderDataManager::updateMeshHighlight() could not remove old highlight";
            }
        }
        _highlightPrimitives.clear();
        for (size_t index = 0; index < _highlightContourPrimitives.size(); ++index)
        {
            _scene->removePrimitive(_highlightShader, _highlightContourPrimitives[index]);
        }
        _highlightContourPrimitives.clear();
        _highlightViewValid = false;
        _activeHighlightedMeshes = _requestedHighlightedMeshes;

        if (_activeHighlightedMeshes.empty())
        {
            return;
        }

        if (!_highlightShader)
        {
            RE::ShaderLib::instance().getHighlightShader(_highlightShader);
        }

        for (size_t index = 0; index < _activeHighlightedMeshes.size(); ++index)
        {
            auto mesh = _activeHighlightedMeshes[index];
            RE::MeshPrimitiveCreator creator(mesh);
            auto primitive = creator.getPrimitive<RE::RenderFace>();
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(primitive);
            face->setColor(_highlightColor);
            face->setDepthOffset(-1.0f, -1.0f);
            primitive->setup();

            if (_scene->addPrimitive(_highlightShader, primitive) != Common::Return::Succeed)
            {
                LOG(ERROR)
                    << "RenderDataManager::updateMeshHighlight() failed to add mesh highlight";
                continue;
            }
            _highlightPrimitives.pushBack(primitive);

            auto linePrimitive = Common::dynamicSharedPtrCast<RE::RenderLine>(
                creator.getPrimitive<RE::RenderLine>());
            linePrimitive->setIndices(creator.getTranslator()->getBoundaryElementEdgeIdx());
            linePrimitive->setColor(Common::Vector4f(1.0f, 0.75f, 0.1f, 1.0f));
            linePrimitive->setLineWidth(3.0f);
            linePrimitive->setup();
            if (_scene->addPrimitive(_highlightShader, linePrimitive) == Common::Return::Succeed)
            {
                _highlightContourPrimitives.pushBack(linePrimitive);
            }
        }
    }

    Common::Vector4f RenderDataManager::getMeshBaseColor(
        const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        for (const auto& meshColor : _meshColors)
        {
            if (meshColor.mesh == mesh)
            {
                return meshColor.color;
            }
        }
        return Common::Vector4f(0.8f, 0.8f, 0.8f, 1.0f);
    }

    void RenderDataManager::updateLocateHighlight()
    {
        if (!_scene)
        {
            return;
        }

        Common::SharedPtr<RE::Shader> highlightShader;
        RE::ShaderLib::instance().getHighlightShader(highlightShader);

        for (const auto& primitive : _locatePrimitives)
        {
            _scene->removePrimitive(highlightShader, primitive);
        }
        _locatePrimitives.clear();

        if (_locateTargetFace)
        {
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(_locateTargetFace);
            if (face)
            {
                face->setColor(getMeshBaseColor(_locateTargetMesh));
            }
            _locateTargetFace = nullptr;
            _locateTargetMesh = nullptr;
        }

        if (!_locateMesh || _locateElementIds.empty())
        {
            return;
        }

        MeshRenderEntry* target = nullptr;
        for (auto& entry : _activeMeshes)
        {
            if (entry.mesh == _locateMesh)
            {
                target = &entry;
                break;
            }
        }
        if (target == nullptr || !target->translator || !target->packet.renderFace)
        {
            LOG(WARNING)
                << "RenderDataManager::updateLocateHighlight() could not find the mesh packet";
            return;
        }

        const Common::DynamicArray<RE::IdxType> faceIndices =
            target->translator->getBoundaryFaceIdx();
        const auto& locations = target->translator->getBoundaryFaceLocations();
        if (faceIndices.empty())
        {
            LOG(WARNING)
                << "RenderDataManager::updateLocateHighlight() the target mesh has no boundary "
                   "faces";
            return;
        }
        const size_t triangleCount = faceIndices.size() / 3;
        if (locations.size() < triangleCount)
        {
            LOG(WARNING) << "RenderDataManager::updateLocateHighlight() received incomplete face "
                            "locations";
            return;
        }

        Common::DynamicArray<RE::IdxType> subsetIndices;
        for (size_t triangle = 0; triangle < triangleCount; ++triangle)
        {
            bool requested = false;
            for (size_t index = 0; index < _locateElementIds.size(); ++index)
            {
                if (static_cast<uint64_t>(locations[triangle].elemId) ==
                    static_cast<uint64_t>(_locateElementIds[index]))
                {
                    requested = true;
                    break;
                }
            }
            if (!requested)
            {
                continue;
            }

            subsetIndices.pushBack(faceIndices[triangle * 3]);
            subsetIndices.pushBack(faceIndices[triangle * 3 + 1]);
            subsetIndices.pushBack(faceIndices[triangle * 3 + 2]);
        }

        if (subsetIndices.empty())
        {
            LOG(WARNING) << "RenderDataManager::updateLocateHighlight() found no boundary "
                            "triangles for the requested element ids";
            return;
        }

        RE::MeshPrimitiveCreator creator(_locateMesh);
        auto primitive = creator.getPrimitive<RE::RenderFace>();
        auto locateFace = Common::dynamicSharedPtrCast<RE::RenderFace>(primitive);
        if (!locateFace)
        {
            return;
        }
        locateFace->setIndices(Common::move(subsetIndices));
        locateFace->setColor(_locateColor);
        locateFace->setDepthOffset(-2.0f, -2.0f);
        primitive->setup();
        if (_scene->addPrimitive(highlightShader, primitive) != Common::Return::Succeed)
        {
            LOG(ERROR)
                << "RenderDataManager::updateLocateHighlight() failed to add the locate primitive";
            return;
        }
        _locatePrimitives.pushBack(primitive);

        auto targetFace = Common::dynamicSharedPtrCast<RE::RenderFace>(target->packet.renderFace);
        if (!targetFace)
        {
            return;
        }

        const Common::Vector4f baseColor = getMeshBaseColor(_locateMesh);
        const Common::Vector4f transparentColor(baseColor[0], baseColor[1], baseColor[2], 0.0f);
        Common::DynamicArray<Common::Vector4f> colors;
        colors.reserve(triangleCount);
        for (size_t triangle = 0; triangle < triangleCount; ++triangle)
        {
            colors.pushBack(transparentColor);
        }
        targetFace->setColors(Common::move(colors));
        _locateTargetFace = target->packet.renderFace;
        _locateTargetMesh = _locateMesh;
    }

    void RenderDataManager::updateElementsVisibility(const ElementsVisibilityEvent& event)
    {
        if (!event.mesh)
        {
            return;
        }

        MeshElementVisibilityState* state = findElementVisibilityState(event.mesh);
        if (state == nullptr)
        {
            MeshElementVisibilityState newState;
            newState.mesh = event.mesh;
            _elementVisibilityStates.pushBack(newState);
            state = &_elementVisibilityStates.back();
        }

        if (event.mode == 2)
        {
            if (event.elementIds.empty())
            {
                state->hiddenElementIds.clear();
            }
            else
            {
                Common::HashSet<ME::IdType> requested;
                for (size_t index = 0; index < event.elementIds.size(); ++index)
                {
                    requested.insert(static_cast<ME::IdType>(event.elementIds[index]));
                }

                Common::HashSet<ME::IdType> hidden;
                ME::MeshIterator iterator(*event.mesh);
                for (; iterator.hasMoreElem(); iterator.nextElem())
                {
                    const ME::IdType elementId = iterator.currentElem().getId();
                    if (requested.find(elementId) == requested.end())
                    {
                        hidden.insert(elementId);
                    }
                }
                state->hiddenElementIds = hidden;
            }
        }
        else if (event.mode == 3)
        {
            state->hiddenElementIds.clear();
            for (size_t index = 0; index < event.elementIds.size(); ++index)
            {
                state->hiddenElementIds.insert(static_cast<ME::IdType>(event.elementIds[index]));
            }
        }
        else if (event.mode == 1)
        {
            if (event.elementIds.empty())
            {
                state->hiddenElementIds.clear();
            }
            else if (!state->hiddenElementIds.empty())
            {
                Common::HashSet<ME::IdType> requested;
                for (size_t index = 0; index < event.elementIds.size(); ++index)
                {
                    requested.insert(static_cast<ME::IdType>(event.elementIds[index]));
                }

                Common::HashSet<ME::IdType> remaining;
                for (auto it = state->hiddenElementIds.begin(); it.hasNext(); it.next())
                {
                    const ME::IdType hiddenId = it.data();
                    if (requested.find(hiddenId) == requested.end())
                    {
                        remaining.insert(hiddenId);
                    }
                }
                state->hiddenElementIds = remaining;
            }
        }
        else
        {
            for (size_t index = 0; index < event.elementIds.size(); ++index)
            {
                state->hiddenElementIds.insert(static_cast<ME::IdType>(event.elementIds[index]));
            }
        }

        rebuildMeshSurface(event.mesh, collectHiddenElementIds(*state));
    }

    RenderDataManager::MeshElementVisibilityState* RenderDataManager::findElementVisibilityState(
        const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (auto& state : _elementVisibilityStates)
        {
            if (state.mesh == mesh)
            {
                return &state;
            }
        }
        return nullptr;
    }

    Common::DynamicArray<ME::IdType> RenderDataManager::collectHiddenElementIds(
        MeshElementVisibilityState& state)
    {
        Common::DynamicArray<ME::IdType> hiddenElementIds;
        hiddenElementIds.reserve(state.hiddenElementIds.size());
        for (auto it = state.hiddenElementIds.begin(); it.hasNext(); it.next())
        {
            hiddenElementIds.pushBack(it.data());
        }
        return hiddenElementIds;
    }

    void RenderDataManager::rebuildMeshSurface(
        const Common::SharedPtr<ME::Mesh>& mesh,
        const Common::DynamicArray<ME::IdType>& hiddenElementIds)
    {
        if (!_scene || !mesh)
        {
            return;
        }

        MeshRenderEntry* target = nullptr;
        for (auto& entry : _activeMeshes)
        {
            if (entry.mesh == mesh)
            {
                target = &entry;
                break;
            }
        }
        if (target == nullptr || !target->translator)
        {
            LOG(WARNING) << "RenderDataManager::rebuildMeshSurface() could not find the mesh entry";
            return;
        }

        if (_locateMesh == mesh)
        {
            Common::SharedPtr<RE::Shader> highlightShader;
            RE::ShaderLib::instance().getHighlightShader(highlightShader);
            for (const auto& primitive : _locatePrimitives)
            {
                _scene->removePrimitive(highlightShader, primitive);
            }
            _locatePrimitives.clear();
            _locateTargetFace = nullptr;
            _locateTargetMesh = nullptr;
        }

        target->translator->setHiddenElements(hiddenElementIds);

        RE::MeshPrimitiveCreator creator(mesh, target->translator);
        auto facePrimitive = creator.getPrimitive<RE::RenderFace>();
        auto linePrimitive = creator.getPrimitive<RE::RenderLine>();
        auto newFace = Common::dynamicSharedPtrCast<RE::RenderFace>(facePrimitive);
        auto newLine = Common::dynamicSharedPtrCast<RE::RenderLine>(linePrimitive);
        if (!newFace || !newLine)
        {
            LOG(ERROR)
                << "RenderDataManager::rebuildMeshSurface() failed to create rebuilt primitives";
            return;
        }

        newFace->setColor(getMeshBaseColor(mesh));
        newFace->setFaceShrink(_faceShrink);
        newLine->setLineWidth(_lineWidth);
        newFace->setup();
        newLine->setup();

        _scene->removePrimitive(target->packet.faceShader, target->packet.renderFace);
        _scene->removePrimitive(target->packet.lineShader, target->packet.renderLine);
        _scene->addPrimitive(target->packet.faceShader, facePrimitive);
        _scene->addPrimitive(target->packet.lineShader, linePrimitive);

        target->packet.renderFace = facePrimitive;
        target->packet.renderLine = linePrimitive;

        if (_meshSelector && target->mesh)
        {
            _meshSelector->registerMesh(target->mesh, target->translator, target->packet.renderNode,
                                        target->packet.renderLine, target->packet.renderFace);
        }

        if (_locateMesh == mesh)
        {
            updateLocateHighlight();
        }
    }

    void RenderDataManager::updateViewDependentHighlights()
    {
        if (!_scene || !_camera || _activeHighlightedMeshes.empty())
        {
            return;
        }

        const RE::CoordType cameraPosition = _camera->getPosition();
        const RE::CoordType cameraFront = _camera->getFront();
        if (_highlightViewValid && (cameraPosition - _highlightViewPosition).length2() < 1e-8f &&
            (cameraFront - _highlightViewFront).length2() < 1e-8f)
        {
            return;
        }

        for (const auto& primitive : _highlightContourPrimitives)
        {
            _scene->removePrimitive(_highlightShader, primitive);
        }
        _highlightContourPrimitives.clear();
        _highlightViewPosition = cameraPosition;
        _highlightViewFront = cameraFront;
        _highlightViewValid = true;

        for (const auto& mesh : _activeHighlightedMeshes)
        {
            ME::ExtractSilhouette extractor(mesh);
            const auto edges = extractor.getSilhouetteEdges(
                ME::CoordType(cameraPosition.x(), cameraPosition.y(), cameraPosition.z()));
            if (edges.empty())
            {
                continue;
            }

            Common::DynamicArray<RE::CoordType> nodeCoordinates;
            Common::HashMap<ME::IdType, RE::IdxType> nodeIndices;
            ME::MeshIterator meshIterator(*mesh);
            for (; meshIterator.hasMoreNode(); meshIterator.nextNode())
            {
                auto& node = meshIterator.currentNode();
                nodeIndices[node.getId()] = static_cast<RE::IdxType>(nodeCoordinates.size());
                nodeCoordinates.pushBack(RE::CoordType(node.x(), node.y(), node.z()));
            }

            Common::DynamicArray<RE::IdxType> indices;
            for (const auto& edge : edges)
            {
                const auto firstIndex = nodeIndices.find(edge.first);
                const auto secondIndex = nodeIndices.find(edge.second);
                if (firstIndex == nodeIndices.end() || secondIndex == nodeIndices.end())
                {
                    continue;
                }
                indices.pushBack((*firstIndex).second);
                indices.pushBack((*secondIndex).second);
            }
            if (indices.empty())
            {
                continue;
            }

            auto linePrimitive = Common::makeShared<RE::RenderLine>();
            linePrimitive->setNodes(Common::move(nodeCoordinates));
            linePrimitive->setIndices(Common::move(indices));
            linePrimitive->setColor(Common::Vector4f(1.0f, 0.75f, 0.1f, 1.0f));
            linePrimitive->setup();
            if (_scene->addPrimitive(_highlightShader, linePrimitive) == Common::Return::Succeed)
            {
                _highlightContourPrimitives.pushBack(linePrimitive);
            }
        }
    }

    void RenderDataManager::updateHEMeshHighlight(const HEMeshHighlightEvent& event)
    {
        if (!_scene)
        {
            return;
        }
        for (const auto& primitive : _heMeshHighlightPrimitives)
        {
            _scene->removePrimitive(_heMeshHighlightShader, primitive);
        }
        _heMeshHighlightPrimitives.clear();

        if (!event.heMesh || event.vertexIndex >= event.heMesh->getVertexCount())
        {
            return;
        }
        if (!_heMeshHighlightShader)
        {
            RE::ShaderLib::instance().getHighlightShader(_heMeshHighlightShader);
        }

        const Common::Vector4f highlightColor(1.0f, 0.28f, 0.04f, 1.0f);
        const auto selectedVertex = event.heMesh->getVertex(event.vertexIndex);
        Common::DynamicArray<RE::CoordType> nodeCoordinates;
        RE::CoordType nodeCoordinate(selectedVertex._coord.x(), selectedVertex._coord.y(),
                                     selectedVertex._coord.z());
        nodeCoordinates.pushBack(nodeCoordinate);
        auto nodePrimitive = Common::makeShared<RE::RenderNode>();
        nodePrimitive->setNodes(Common::move(nodeCoordinates));
        nodePrimitive->setColor(highlightColor);
        nodePrimitive->setPointSize(12.0f);
        nodePrimitive->setDepthTestEnabled(false);
        nodePrimitive->setup();
        if (_scene->addPrimitive(_heMeshHighlightShader, nodePrimitive) == Common::Return::Succeed)
        {
            _heMeshHighlightPrimitives.pushBack(nodePrimitive);
        }

        if (event.halfEdgeIndex >= event.heMesh->getHalfEdgeCount())
        {
            return;
        }
        const auto halfEdge = event.heMesh->getHalfEdge(event.halfEdgeIndex);
        if (halfEdge.vertex != event.vertexIndex ||
            halfEdge.next >= event.heMesh->getHalfEdgeCount())
        {
            return;
        }
        const auto nextHalfEdge = event.heMesh->getHalfEdge(halfEdge.next);
        const auto nextVertex = event.heMesh->getVertex(nextHalfEdge.vertex);
        Common::DynamicArray<RE::CoordType> edgeCoordinates;
        RE::CoordType edgeStart(selectedVertex._coord.x(), selectedVertex._coord.y(),
                                selectedVertex._coord.z());
        RE::CoordType edgeEnd(nextVertex._coord.x(), nextVertex._coord.y(), nextVertex._coord.z());
        edgeCoordinates.pushBack(edgeStart);
        edgeCoordinates.pushBack(edgeEnd);
        Common::DynamicArray<RE::IdxType> edgeIndices;
        RE::IdxType firstIndex = 0;
        RE::IdxType secondIndex = 1;
        edgeIndices.pushBack(firstIndex);
        edgeIndices.pushBack(secondIndex);
        auto linePrimitive = Common::makeShared<RE::RenderLine>();
        linePrimitive->setNodes(Common::move(edgeCoordinates));
        linePrimitive->setIndices(Common::move(edgeIndices));
        linePrimitive->setColor(highlightColor);
        linePrimitive->setLineWidth(5.0f);
        linePrimitive->setup();
        if (_scene->addPrimitive(_heMeshHighlightShader, linePrimitive) == Common::Return::Succeed)
        {
            _heMeshHighlightPrimitives.pushBack(linePrimitive);
        }
    }
} // namespace EgLab::Platform
