#include "Work/RenderDataManager.hpp"

#include <limits>

#include "Common/BBox.hpp"
#include "Common/HashMap.hpp"
#include "Common/Log.hpp"
#include "DataBase/GeometryData.hpp"
#include "MeshEngine/Algorithm/ExtractSilhouette.hpp"
#include "MeshEngine/MeshData/HEMesh.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshVisibility.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/RenderPickHighlight.hpp"
#include "RenderEngine/Core/ReferencePlane.hpp"
#include "RenderEngine/Core/Renderer.hpp"
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

    void RenderDataManager::bindRenderer(const Common::SharedPtr<RE::Renderer>& renderer)
    {
        _renderer = renderer;
    }

    void RenderDataManager::bindMeshSelector(const Common::SharedPtr<RE::MeshSelector>& selector)
    {
        _meshSelector = selector;
    }

    void RenderDataManager::bindVisibilityFilter(RE::ScenePicker::VisibilityFilter filter)
    {
        _visibilityFilter = Common::move(filter);
    }

    void RenderDataManager::releaseResources()
    {
        clearPickHighlight();
        clearSelectionHighlight();
        _visibilityFilter = nullptr;

        _activeMeshes.clear();
        _meshColors.clear();
        _elementVisibilityStates.clear();
        _nodeVisibilityStates.clear();
        _selection.clear();
        _selectionHighlightPrimitives.clear();
        _selectionHighlightShader = nullptr;
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
        _renderer = nullptr;
    }

    std::mutex& RenderDataManager::stateMutex() const
    {
        return _renderStateMutex;
    }

    void RenderDataManager::getSelectionSnapshot(SelectionSnapshot& snapshot) const
    {
        _selection.getSnapshot(snapshot);
    }

    uint64_t RenderDataManager::getSelectionRevision() const
    {
        return _selection.revision();
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

            Common::DynamicArray<MeshNodeVisibilityState> remainingNodeStates;
            for (const auto& state : _nodeVisibilityStates)
            {
                if (state.mesh != mesh)
                {
                    remainingNodeStates.pushBack(state);
                }
            }
            _nodeVisibilityStates = Common::move(remainingNodeStates);
        }

        _selection.removeMesh(mesh);
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

    void RenderDataManager::setCamera(const SetCameraEvent& event)
    {
        if (!_camera)
        {
            return;
        }

        const SetCameraParam& param = event.param;
        RE::CoordType position = _camera->getPosition();
        RE::CoordType target = _camera->getTarget();
        float fov = _camera->getZoom();
        float duration = 0.45f;

        if ((param.fieldMask & CAMERA_FIELD_POSITION) != 0)
        {
            position = RE::CoordType(param.position[0], param.position[1], param.position[2]);
        }
        if ((param.fieldMask & CAMERA_FIELD_TARGET) != 0)
        {
            target = RE::CoordType(param.target[0], param.target[1], param.target[2]);
        }
        if ((param.fieldMask & CAMERA_FIELD_FOV) != 0)
        {
            fov = param.fov;
        }
        if ((param.fieldMask & CAMERA_FIELD_DURATION) != 0)
        {
            duration = param.duration;
        }

        _camera->setCamera(position, target, fov, duration);
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

    bool RenderDataManager::getNearestMeshCenter(float cursorX, float cursorY,
                                                 RE::CoordType& center) const
    {
        if (!_camera)
        {
            return false;
        }

        const unsigned int width = _camera->getViewportWidth();
        const unsigned int height = _camera->getViewportHeight();
        if (width == 0 || height == 0)
        {
            return false;
        }

        const Common::Matrix4f view = _camera->view();
        Common::BBox<RE::Scalar, 3> sceneBounds;
        if (_scene)
        {
            sceneBounds = _scene->getBounds();
        }
        const Common::Matrix4f projection = _camera->perspective(sceneBounds);

        bool found = false;
        RE::Scalar bestDistanceSquared = std::numeric_limits<RE::Scalar>::max();
        RE::CoordType bestCenter;

        for (const auto& entry : _activeMeshes)
        {
            Common::BBox<RE::Scalar, 3> bounds;
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
            if (bounds.isEmpty())
            {
                continue;
            }

            const RE::CoordType meshCenter = (bounds.min() + bounds.max()) * 0.5f;
            RE::PickUtils::ScreenPoint screenPoint;
            if (!RE::PickUtils::projectPoint(meshCenter, view, projection, width, height,
                                             screenPoint))
            {
                continue;
            }

            const RE::Scalar deltaX = screenPoint.x - cursorX;
            const RE::Scalar deltaY = screenPoint.y - cursorY;
            const RE::Scalar distanceSquared = deltaX * deltaX + deltaY * deltaY;
            if (!found || distanceSquared < bestDistanceSquared)
            {
                found = true;
                bestDistanceSquared = distanceSquared;
                bestCenter = meshCenter;
            }
        }

        if (!found)
        {
            return false;
        }

        center = bestCenter;
        return true;
    }

    void RenderDataManager::applyPick(const PickEvent& event)
    {
        if (!_scene || !_camera)
        {
            return;
        }

        if (event.kinds == 0)
        {
            clearPickHighlight();
            notifyPickCleared();
            return;
        }

        const unsigned int width = _camera->getViewportWidth();
        const unsigned int height = _camera->getViewportHeight();
        if (width == 0 || height == 0)
        {
            return;
        }

        RE::PickContext context;
        context.x = event.x;
        context.y = event.y;
        context.width = width;
        context.height = height;
        context.view = _camera->view();
        context.projection = _camera->perspective(_scene->getBounds());

        RE::ScenePicker picker;
        const RE::PickOutcome outcome =
            picker.pick(_scene, context, event.kinds, _visibilityFilter);
        if (!outcome.valid())
        {
            clearPickHighlight();
            notifyPickCleared();
            return;
        }

        Common::DynamicArray<RE::IdxType> indices;
        if (_meshSelector)
        {
            indices = _meshSelector->getHighlightIndices(outcome.primitive, outcome.index);
        }
        else
        {
            indices.pushBack(outcome.index);
        }

        clearPickHighlight();

        Common::SharedPtr<RE::RenderPickHighlight> highlight =
            Common::makeShared<RE::RenderPickHighlight>();
        highlight->setSource(outcome.primitive);
        highlight->setSourceShader(outcome.shader);
        highlight->setIndices(Common::move(indices));

        Common::SharedPtr<RE::Shader> highlightShader;
        RE::ShaderLib::instance().getHighlightShader(highlightShader);
        if (highlightShader == nullptr ||
            _scene->addPrimitive(highlightShader, highlight) != Common::Return::Succeed)
        {
            LOG(ERROR) << "RenderDataManager::applyPick() failed to add the pick highlight";
            return;
        }
        _pickHighlightPrimitive = highlight;

        if (_meshSelector)
        {
            _meshSelector->processPick(outcome.primitive, outcome.index);
        }
    }

    void RenderDataManager::clearPickHighlight()
    {
        if (_scene && _pickHighlightPrimitive)
        {
            Common::SharedPtr<RE::Shader> highlightShader;
            RE::ShaderLib::instance().getHighlightShader(highlightShader);
            _scene->removePrimitive(highlightShader, _pickHighlightPrimitive);
        }
        _pickHighlightPrimitive = nullptr;
    }

    void RenderDataManager::notifyPickCleared()
    {
        if (_meshSelector)
        {
            _meshSelector->processPick(nullptr, static_cast<RE::IdxType>(-1));
        }
    }

    Common::Return RenderDataManager::applyMeshUpdate(const MeshUpdateEvent& event)
    {
        // A replaced primitive invalidates the current pick highlight.
        clearPickHighlight();

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
            if (visibilityState != nullptr || isGeometryMesh(entry.mesh))
            {
                // Rebuilding also normalises the line data and face visibility so a newly
                // uploaded geometry model immediately follows the current display mode.
                Common::DynamicArray<ME::IdType> hiddenElementIds;
                if (visibilityState != nullptr)
                {
                    hiddenElementIds = collectHiddenElementIds(*visibilityState);
                }
                rebuildMeshSurface(entry.mesh, hiddenElementIds);
            }
            else if (_locateMesh && _locateMesh == entry.mesh)
            {
                updateLocateHighlight();
            }

            applyNodeVisibilityToPrimitive(entry.mesh);
            _selection.pruneMesh(entry.mesh);
            updateSelectionHighlight();
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

        clearPickHighlight();

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

        _selection.removeMesh(mesh);
        MeshNodeVisibilityState* nodeState = findNodeVisibilityState(mesh);
        if (nodeState != nullptr)
        {
            Common::DynamicArray<MeshNodeVisibilityState> remainingNodeStates;
            for (const auto& state : _nodeVisibilityStates)
            {
                if (state.mesh != mesh)
                {
                    remainingNodeStates.pushBack(state);
                }
            }
            _nodeVisibilityStates = Common::move(remainingNodeStates);
        }

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
        updateSelectionHighlight();
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

    bool RenderDataManager::isGeometryMesh(const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        if (!mesh)
        {
            return false;
        }
        return GeometryData::findByMesh(reinterpret_cast<uint64_t>(mesh.get())) != nullptr;
    }

    GeometryDisplayMode RenderDataManager::effectiveDisplayMode(
        const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        // Regular meshes always use the regular mesh appearance; the geometry mode only
        // affects imported B-Rep models.
        return isGeometryMesh(mesh) ? _geometryDisplayMode : GeometryDisplayMode::Mesh;
    }

    void RenderDataManager::applyGeometryDisplayMode(const GeometryDisplayModeEvent& event)
    {
        if (!GeometryDisplayModeInfo::isValid(event.mode))
        {
            LOG(WARNING) << "RenderDataManager::applyGeometryDisplayMode() rejected mode "
                         << event.mode;
            return;
        }

        const auto mode = static_cast<GeometryDisplayMode>(event.mode);
        if (_geometryDisplayMode == mode)
        {
            return;
        }
        _geometryDisplayMode = mode;

        // Rebuild the geometry models so the line data and face visibility follow the mode.
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> geometryMeshes;
        for (const auto& entry : _activeMeshes)
        {
            if (isGeometryMesh(entry.mesh))
            {
                geometryMeshes.pushBack(entry.mesh);
            }
        }

        for (const auto& mesh : geometryMeshes)
        {
            Common::DynamicArray<ME::IdType> hiddenElementIds;
            MeshElementVisibilityState* state = findElementVisibilityState(mesh);
            if (state != nullptr)
            {
                hiddenElementIds = collectHiddenElementIds(*state);
            }
            rebuildMeshSurface(mesh, hiddenElementIds);
        }
    }

    GeometryDisplayMode RenderDataManager::getGeometryDisplayMode() const
    {
        return _geometryDisplayMode;
    }

    void RenderDataManager::applyReferencePlaneSettings(const SetReferencePlaneParam& param)
    {
        if (!_renderer)
        {
            return;
        }

        RE::ReferencePlane& referencePlane = _renderer->getReferencePlane();
        RE::ReferencePlaneSettings settings = referencePlane.getSettings();

        if ((param.fieldMask & REFERENCE_PLANE_FIELD_ENABLED) != 0)
        {
            settings.enabled = param.enabled != 0;
        }
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_SHOW_GRID) != 0)
        {
            settings.showGrid = param.showGrid != 0;
        }
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_PLACEMENT) != 0)
        {
            settings.placement =
                param.placement == REFERENCE_PLANE_PLACEMENT_BACK_FACES
                    ? RE::ReferencePlanePlacement::BoundingBoxBackFaces
                    : RE::ReferencePlanePlacement::BoundingBoxCenter;
        }
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_SCALE) != 0)
        {
            settings.scale = param.scale > 0.01f ? param.scale : 0.01f;
        }

        const uint32_t colorMasks[3] = {REFERENCE_PLANE_FIELD_COLOR_X,
                                        REFERENCE_PLANE_FIELD_COLOR_Y,
                                        REFERENCE_PLANE_FIELD_COLOR_Z};
        for (int plane = 0; plane < 3; ++plane)
        {
            if ((param.fieldMask & colorMasks[plane]) == 0)
            {
                continue;
            }
            for (int channel = 0; channel < 4; ++channel)
            {
                const float value = param.colors[plane][channel];
                settings.colors[plane][channel] =
                    value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
            }
        }

        referencePlane.setSettings(settings);
    }

    void RenderDataManager::applyLocate()
    {
        updateLocateHighlight();
    }

    void RenderDataManager::applyElementsVisibility(const ElementsVisibilityEvent& event)
    {
        updateElementsVisibility(event);
    }

    void RenderDataManager::applyNodeVisibility(const NodeVisibilityEvent& event)
    {
        updateNodeVisibility(event);
    }

    void RenderDataManager::applyBoxSelection(const BoxSelectEvent& event)
    {
        if (!_scene || !_camera)
        {
            return;
        }

        if (event.width == 0 || event.height == 0 || event.kinds == 0)
        {
            return;
        }

        RE::RectPickContext context;
        context.minX = event.minX < event.maxX ? event.minX : event.maxX;
        context.maxX = event.minX < event.maxX ? event.maxX : event.minX;
        context.minY = event.minY < event.maxY ? event.minY : event.maxY;
        context.maxY = event.minY < event.maxY ? event.maxY : event.minY;
        context.width = event.width;
        context.height = event.height;
        context.view = _camera->view();
        context.projection = _camera->perspective(_scene->getBounds());
        context.containsOnly = event.containsOnly;

        RE::ScenePicker picker;
        const Common::DynamicArray<RE::RectPickTarget> targets =
            picker.pickRect(_scene, context, event.kinds, _visibilityFilter);

        struct MeshSelectionAccumulator
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::HashSet<ME::IdType> nodeIds;
            Common::HashSet<ME::IdType> elementIds;
        };

        Common::DynamicArray<MeshSelectionAccumulator> accumulators;
        for (size_t targetIndex = 0; targetIndex < targets.size(); ++targetIndex)
        {
            const RE::RectPickTarget& target = targets[targetIndex];
            if (!_meshSelector)
            {
                continue;
            }

            RE::RectSelectionResolution resolution;
            if (!_meshSelector->resolveRectSelection(target.primitive, target.indices,
                                                     resolution))
            {
                continue;
            }
            if (!resolution.mesh)
            {
                continue;
            }
            if (event.scopeMesh && resolution.mesh != event.scopeMesh)
            {
                continue;
            }

            MeshSelectionAccumulator* accumulator = nullptr;
            for (auto& candidate : accumulators)
            {
                if (candidate.mesh == resolution.mesh)
                {
                    accumulator = &candidate;
                    break;
                }
            }
            if (accumulator == nullptr)
            {
                MeshSelectionAccumulator newAccumulator;
                newAccumulator.mesh = resolution.mesh;
                accumulators.pushBack(Common::move(newAccumulator));
                accumulator = &accumulators.back();
            }

            for (size_t index = 0; index < resolution.nodeIds.size(); ++index)
            {
                accumulator->nodeIds.insert(resolution.nodeIds[index]);
            }
            for (size_t index = 0; index < resolution.elementIds.size(); ++index)
            {
                accumulator->elementIds.insert(resolution.elementIds[index]);
            }
        }

        if (!event.additive)
        {
            _selection.clear();
        }

        for (auto& accumulator : accumulators)
        {
            Common::DynamicArray<uint32_t> nodeIds;
            Common::DynamicArray<uint32_t> elementIds;
            nodeIds.reserve(accumulator.nodeIds.size());
            elementIds.reserve(accumulator.elementIds.size());
            for (auto it = accumulator.nodeIds.begin(); it.hasNext(); it.next())
            {
                nodeIds.pushBack(static_cast<uint32_t>(it.data()));
            }
            for (auto it = accumulator.elementIds.begin(); it.hasNext(); it.next())
            {
                elementIds.pushBack(static_cast<uint32_t>(it.data()));
            }
            _selection.addMeshSelection(accumulator.mesh, nodeIds, elementIds);
        }

        updateSelectionHighlight();
        LOG(INFO) << "RenderDataManager::applyBoxSelection() resolved "
                  << accumulators.size() << " mesh selection group(s), containsOnly="
                  << (event.containsOnly ? "true" : "false");
    }

    void RenderDataManager::applyPointSelection(const PointSelectEvent& event)
    {
        if (!_scene || !_camera || event.width == 0 || event.height == 0 || event.kinds == 0)
        {
            return;
        }

        RE::PickContext context;
        context.x = event.x;
        context.y = event.y;
        context.width = event.width;
        context.height = event.height;
        context.view = _camera->view();
        context.projection = _camera->perspective(_scene->getBounds());

        RE::ScenePicker picker;
        const RE::PickOutcome outcome =
            picker.pick(_scene, context, event.kinds, _visibilityFilter);

        Common::SharedPtr<ME::Mesh> mesh;
        Common::DynamicArray<uint32_t> nodeIds;
        Common::DynamicArray<uint32_t> elementIds;
        if (outcome.valid() && _meshSelector)
        {
            // Reuse the highlight expansion so a face pick selects the whole element face.
            Common::DynamicArray<RE::IdxType> indices =
                _meshSelector->getHighlightIndices(outcome.primitive, outcome.index);
            RE::RectSelectionResolution resolution;
            if (_meshSelector->resolveRectSelection(outcome.primitive, indices, resolution) &&
                resolution.mesh &&
                (!event.scopeMesh || resolution.mesh == event.scopeMesh))
            {
                mesh = resolution.mesh;
                nodeIds.reserve(resolution.nodeIds.size());
                elementIds.reserve(resolution.elementIds.size());
                for (size_t index = 0; index < resolution.nodeIds.size(); ++index)
                {
                    nodeIds.pushBack(static_cast<uint32_t>(resolution.nodeIds[index]));
                }
                for (size_t index = 0; index < resolution.elementIds.size(); ++index)
                {
                    elementIds.pushBack(static_cast<uint32_t>(resolution.elementIds[index]));
                }
            }
        }

        if (!mesh)
        {
            // Clicking empty space resets the selection unless the click appends.
            if (!event.additive)
            {
                _selection.clear();
                updateSelectionHighlight();
            }
            return;
        }

        if (!event.additive)
        {
            _selection.clear();
        }
        _selection.addMeshSelection(mesh, nodeIds, elementIds);
        updateSelectionHighlight();
    }

    void RenderDataManager::applyClearSelection()
    {
        _selection.clear();
        updateSelectionHighlight();
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
            // A mesh has exactly one active packet: replacing it keeps every lookup that maps a
            // mesh to its primitives (visibility, selection highlight) pointing at live data.
            bool replaced = false;
            for (auto& existing : _activeMeshes)
            {
                if (existing.mesh != entry.mesh)
                {
                    continue;
                }
                if (existing.packet.renderNode && existing.packet.nodeShader)
                {
                    _scene->removePrimitive(existing.packet.nodeShader, existing.packet.renderNode);
                }
                if (existing.packet.renderLine && existing.packet.lineShader)
                {
                    _scene->removePrimitive(existing.packet.lineShader, existing.packet.renderLine);
                }
                if (existing.packet.renderFace && existing.packet.faceShader)
                {
                    _scene->removePrimitive(existing.packet.faceShader, existing.packet.renderFace);
                }
                existing = entry;
                replaced = true;
                break;
            }
            if (!replaced)
            {
                _activeMeshes.pushBack(entry);
            }
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
        if (event.mode > 3)
        {
            LOG(WARNING) << "RenderDataManager::updateElementsVisibility() ignored invalid mode "
                         << event.mode;
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
            Common::HashSet<ME::IdType> requested;
            for (size_t index = 0; index < event.elementIds.size(); ++index)
            {
                requested.insert(static_cast<ME::IdType>(event.elementIds[index]));
            }

            // "Show only" with an empty request means "show nothing", i.e. hide every element.
            Common::HashSet<ME::IdType> hidden;
            ME::MeshIterator iterator(*event.mesh);
            for (; iterator.hasMoreElem(); iterator.nextElem())
            {
                const ME::IdType elementId = iterator.currentElem().getId();
                if (requested.empty() || requested.find(elementId) == requested.end())
                {
                    hidden.insert(elementId);
                }
            }
            state->hiddenElementIds = hidden;
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
        // Hiding elements must hide their nodes too; the merged flag buffer keeps shared nodes
        // of still-visible elements drawable.
        applyNodeVisibilityToPrimitive(event.mesh);
    }

    void RenderDataManager::updateNodeVisibility(const NodeVisibilityEvent& event)
    {
        if (!event.mesh)
        {
            return;
        }
        if (event.mode > 3)
        {
            LOG(WARNING) << "RenderDataManager::updateNodeVisibility() ignored invalid mode "
                         << event.mode;
            return;
        }

        MeshNodeVisibilityState* state = findNodeVisibilityState(event.mesh);
        if (state == nullptr)
        {
            MeshNodeVisibilityState newState;
            newState.mesh = event.mesh;
            _nodeVisibilityStates.pushBack(Common::move(newState));
            state = &_nodeVisibilityStates.back();
        }

        if (event.mode == 2)
        {
            Common::HashSet<ME::IdType> requested;
            for (size_t index = 0; index < event.nodeIds.size(); ++index)
            {
                requested.insert(static_cast<ME::IdType>(event.nodeIds[index]));
            }

            // "Show only" with an empty request means "show nothing", i.e. hide every node.
            Common::HashSet<ME::IdType> hidden;
            ME::MeshIterator iterator(*event.mesh);
            for (; iterator.hasMoreNode(); iterator.nextNode())
            {
                const ME::IdType nodeId = iterator.currentNode().getId();
                if (requested.empty() || requested.find(nodeId) == requested.end())
                {
                    hidden.insert(nodeId);
                }
            }
            state->hiddenNodeIds = hidden;
        }
        else if (event.mode == 3)
        {
            state->hiddenNodeIds.clear();
            for (size_t index = 0; index < event.nodeIds.size(); ++index)
            {
                state->hiddenNodeIds.insert(static_cast<ME::IdType>(event.nodeIds[index]));
            }
        }
        else if (event.mode == 1)
        {
            if (event.nodeIds.empty())
            {
                state->hiddenNodeIds.clear();
            }
            else if (!state->hiddenNodeIds.empty())
            {
                Common::HashSet<ME::IdType> requested;
                for (size_t index = 0; index < event.nodeIds.size(); ++index)
                {
                    requested.insert(static_cast<ME::IdType>(event.nodeIds[index]));
                }

                Common::HashSet<ME::IdType> remaining;
                for (auto it = state->hiddenNodeIds.begin(); it.hasNext(); it.next())
                {
                    const ME::IdType hiddenId = it.data();
                    if (requested.find(hiddenId) == requested.end())
                    {
                        remaining.insert(hiddenId);
                    }
                }
                state->hiddenNodeIds = remaining;
            }
        }
        else
        {
            for (size_t index = 0; index < event.nodeIds.size(); ++index)
            {
                state->hiddenNodeIds.insert(static_cast<ME::IdType>(event.nodeIds[index]));
            }
        }

        applyNodeVisibilityToPrimitive(event.mesh);
        updateSelectionHighlight();
    }

    RenderDataManager::MeshNodeVisibilityState* RenderDataManager::findNodeVisibilityState(
        const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (auto& state : _nodeVisibilityStates)
        {
            if (state.mesh == mesh)
            {
                return &state;
            }
        }
        return nullptr;
    }

    void RenderDataManager::applyNodeVisibilityToPrimitive(
        const Common::SharedPtr<ME::Mesh>& mesh)
    {
        MeshRenderEntry* entry = findActiveMeshEntry(mesh);
        if (entry == nullptr)
        {
            return;
        }

        auto nodePrimitive = Common::dynamicSharedPtrCast<RE::RenderNode>(entry->packet.renderNode);
        if (!nodePrimitive)
        {
            return;
        }

        Common::HashSet<RE::IdxType> hiddenIndices;
        // Explicitly hidden nodes and nodes left without any visible element share one flag
        // buffer, because hiding an element must hide its nodes as well.
        const Common::HashSet<ME::IdType> hiddenNodeIds = collectHiddenNodeIds(mesh);
        if (!hiddenNodeIds.empty())
        {
            hiddenIndices.reserve(hiddenNodeIds.size());
        }

        RE::IdxType renderIndex = 0;
        ME::MeshIterator nodeIterator(*mesh);
        for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode(), ++renderIndex)
        {
            if (hiddenNodeIds.find(nodeIterator.currentNode().getId()) != hiddenNodeIds.end())
            {
                hiddenIndices.insert(renderIndex);
            }
        }
        nodePrimitive->setHiddenNodes(hiddenIndices);
    }

    Common::HashSet<ME::IdType> RenderDataManager::collectHiddenNodeIds(
        const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        Common::HashSet<ME::IdType> hiddenNodeIds;
        if (!mesh)
        {
            return hiddenNodeIds;
        }

        const MeshNodeVisibilityState* nodeState = findNodeVisibilityState(mesh);
        if (nodeState != nullptr)
        {
            for (auto it = nodeState->hiddenNodeIds.begin(); it.hasNext(); it.next())
            {
                hiddenNodeIds.insert(it.data());
            }
        }

        const MeshElementVisibilityState* elementState = findElementVisibilityState(mesh);
        if (elementState == nullptr || elementState->hiddenElementIds.empty())
        {
            return hiddenNodeIds;
        }

        // Nodes still referenced by a visible element stay drawable; the remaining nodes belong
        // only to hidden elements and disappear together with them.
        ME::MeshVisibility::collectNodeIdsHiddenByElements(*mesh, elementState->hiddenElementIds,
                                                           hiddenNodeIds);
        return hiddenNodeIds;
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

        // The rebuilt face/line primitives replace the ones a highlight may reference.
        clearPickHighlight();

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

        const GeometryDisplayMode displayMode = effectiveDisplayMode(mesh);

        RE::MeshPrimitiveCreator creator(mesh, target->translator);
        if (displayMode != GeometryDisplayMode::Mesh)
        {
            creator.setLineIndexSource(RE::MeshPrimitiveCreator::LineIndexSource::BoundaryOnly);
        }
        // Smooth shading is what makes the face frames readable on curved B-Rep faces.
        creator.setSmoothNormals(displayMode == GeometryDisplayMode::FaceOutline);
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

        // EdgeOnly hides the shaded faces; picking follows the same flag through the
        // renderer visibility filter, so faces are not picked while they are not drawn.
        newFace->setVisible(displayMode != GeometryDisplayMode::EdgeOnly);

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

        // The face primitive was replaced, so any selection highlight that referenced it must
        // be rebuilt from the new primitives.
        updateSelectionHighlight();
    }

    void RenderDataManager::updateSelectionHighlight()
    {
        clearSelectionHighlight();
        if (!_scene)
        {
            return;
        }

        if (!_selectionHighlightShader)
        {
            RE::ShaderLib::instance().getHighlightShader(_selectionHighlightShader);
        }
        if (!_selectionHighlightShader)
        {
            return;
        }

        for (const auto& state : _selection.getMeshStates())
        {
            if (!state.mesh)
            {
                continue;
            }

            MeshRenderEntry* entry = findActiveMeshEntry(state.mesh);
            if (entry == nullptr)
            {
                continue;
            }

            const auto addHighlight = [this](const Common::SharedPtr<RE::RenderPrimitive>& source,
                                             const Common::SharedPtr<RE::Shader>& sourceShader,
                                             Common::DynamicArray<RE::IdxType>&& indices) {
                if (!source || !sourceShader || indices.empty())
                {
                    return;
                }

                auto highlight = Common::makeShared<RE::RenderPickHighlight>();
                highlight->setSource(source);
                highlight->setSourceShader(sourceShader);
                highlight->setIndices(Common::move(indices));
                if (_scene->addPrimitive(_selectionHighlightShader, highlight) ==
                    Common::Return::Succeed)
                {
                    _selectionHighlightPrimitives.pushBack(highlight);
                }
            };

            // Node highlights keep the original point size enlargement, which makes selected
            // nodes visible even when faces and edges are dense.
            if (!state.nodeIds.empty())
            {
                const Common::HashSet<ME::IdType> hiddenNodeIds = collectHiddenNodeIds(state.mesh);
                Common::DynamicArray<RE::IdxType> nodeIndices;
                nodeIndices.reserve(state.nodeIds.size());
                RE::IdxType renderIndex = 0;
                ME::MeshIterator nodeIterator(*state.mesh);
                for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode(), ++renderIndex)
                {
                    const ME::IdType nodeId = nodeIterator.currentNode().getId();
                    if (state.nodeIds.find(nodeId) == state.nodeIds.end())
                    {
                        continue;
                    }
                    if (hiddenNodeIds.find(nodeId) != hiddenNodeIds.end())
                    {
                        continue;
                    }
                    nodeIndices.pushBack(renderIndex);
                }
                addHighlight(entry->packet.renderNode, entry->packet.nodeShader,
                             Common::move(nodeIndices));
            }

            // Only explicitly selected elements are highlighted. Elements reached through a
            // selected node stay part of the derived visibility data, not of the highlight.
            if (!state.elementIds.empty() && entry->translator && entry->packet.renderFace)
            {
                const MeshElementVisibilityState* elementState =
                    findElementVisibilityState(state.mesh);
                const auto& locations = entry->translator->getBoundaryFaceLocations();
                Common::DynamicArray<RE::IdxType> faceIndices;
                faceIndices.reserve(locations.size());
                for (size_t triangle = 0; triangle < locations.size(); ++triangle)
                {
                    const ME::IdType elementId = locations[triangle].elemId;
                    if (state.elementIds.find(elementId) == state.elementIds.end())
                    {
                        continue;
                    }
                    if (elementState != nullptr && !elementState->hiddenElementIds.empty() &&
                        elementState->hiddenElementIds.find(elementId) !=
                            elementState->hiddenElementIds.end())
                    {
                        continue;
                    }
                    faceIndices.pushBack(static_cast<RE::IdxType>(triangle));
                }
                addHighlight(entry->packet.renderFace, entry->packet.faceShader,
                             Common::move(faceIndices));
            }
        }
    }

    void RenderDataManager::clearSelectionHighlight()
    {
        if (_scene && _selectionHighlightShader)
        {
            for (const auto& primitive : _selectionHighlightPrimitives)
            {
                _scene->removePrimitive(_selectionHighlightShader, primitive);
            }
        }
        _selectionHighlightPrimitives.clear();
    }

    RenderDataManager::MeshRenderEntry* RenderDataManager::findActiveMeshEntry(
        const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (auto& entry : _activeMeshes)
        {
            if (entry.mesh == mesh)
            {
                return &entry;
            }
        }
        return nullptr;
    }

    const RenderDataManager::MeshNodeVisibilityState* RenderDataManager::findNodeVisibilityState(
        const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (const auto& state : _nodeVisibilityStates)
        {
            if (state.mesh == mesh)
            {
                return &state;
            }
        }
        return nullptr;
    }

    const RenderDataManager::MeshElementVisibilityState*
    RenderDataManager::findElementVisibilityState(const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (const auto& state : _elementVisibilityStates)
        {
            if (state.mesh == mesh)
            {
                return &state;
            }
        }
        return nullptr;
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
