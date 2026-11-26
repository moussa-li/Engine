#pragma once

#include <cstdint>
#include <functional>
#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/HashSet.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "MeshEngine/Common/Definitions.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/ScenePicker.hpp"
#include "Work/RenderEvent.hpp"
#include "Work/GeometryZebra.hpp"
#include "Work/GeometryUv.hpp"
#include "Work/SelectionModel.hpp"

namespace EgLab::ME
{
    class Mesh;
    class HEMesh;
} // namespace EgLab::ME

namespace EgLab::RE
{
    class Scene;
    class Camera;
    class Renderer;
    class Shader;
    class MeshSelector;
    class RenderPrimitive;
    class MeshToTriFace;
} // namespace EgLab::RE

namespace EgLab::Platform
{
    /// Owns the rendering data produced by mesh updates and the per-mesh highlight,
    /// locate, visibility and color state. RenderWork forwards events to handlers,
    /// which in turn mutate this object; RenderWork itself does not manage render data.
    class RenderDataManager
    {
    public:
        RenderDataManager();
        ~RenderDataManager();

        void bindScene(const Common::SharedPtr<RE::Scene>& scene);
        void bindCamera(const Common::SharedPtr<RE::Camera>& camera);
        void bindRenderer(const Common::SharedPtr<RE::Renderer>& renderer);
        void bindMeshSelector(const Common::SharedPtr<RE::MeshSelector>& selector);
        // Visibility filter used while picking, typically the renderer's node/edge/face switches.
        void bindVisibilityFilter(RE::ScenePicker::VisibilityFilter filter);
        void releaseResources();

        /// Mutex protecting the render data managed by this object. Handlers are
        /// invoked while the caller holds this mutex.
        std::mutex& stateMutex() const;

        // ---- prepare phase: record requested state without mutating the scene ----
        bool recordMeshColor(const Common::SharedPtr<ME::Mesh>& mesh,
                             const Common::Vector4f& color);
        bool recordMeshRemoval(const Common::SharedPtr<ME::Mesh>& mesh);
        bool isMeshRemoved(const ME::Mesh* mesh) const;
        void replaceRequestedHighlightedMeshes(
            const Common::DynamicArray<Common::SharedPtr<ME::Mesh>>& meshes);
        void setHighlightColor(const Common::Vector4f& color);
        void setNodePointSize(float pointSize);
        void setLineWidth(float lineWidth);
        void setFaceShrink(float shrink);
        void setLocateState(const Common::SharedPtr<ME::Mesh>& mesh,
                            const Common::DynamicArray<uint32_t>& elementIds);
        void setCamera(const SetCameraEvent& event);
        void fitView(const Common::SharedPtr<ME::Mesh>& mesh);

        /// Returns the bounding-box center of the mesh's visible primitives, the same
        /// primitives fitView uses, so hidden element changes are respected.
        bool getMeshCenter(const Common::SharedPtr<ME::Mesh>& mesh, RE::CoordType& center) const;

        /// Returns the bounding-box center of the active mesh whose projected center lies
        /// nearest to the given cursor position. Coordinates are framebuffer pixels with a
        /// top-left origin. Does not require a geometry pick.
        bool getNearestMeshCenter(float cursorX, float cursorY, RE::CoordType& center) const;

        // ---- apply phase: mutate the scene ----
        Common::Return applyMeshUpdate(const MeshUpdateEvent& event);
        void applyMeshColor(const Common::SharedPtr<ME::Mesh>& mesh, const Common::Vector4f& color);
        void applyMeshRemoval(const Common::SharedPtr<ME::Mesh>& mesh);
        void applyHighlight();
        void applyHighlightColor();
        void applyNodePointSize(float pointSize);
        void applyLineWidth(float lineWidth);
        void applyFaceShrink(float shrink);
        void applyReferencePlaneSettings(const SetReferencePlaneParam& param);
        void applyLocate();
        void applyElementsVisibility(const ElementsVisibilityEvent& event);
        void applyNodeVisibility(const NodeVisibilityEvent& event);
        void applyBoxSelection(const BoxSelectEvent& event);
        void applyPointSelection(const PointSelectEvent& event);
        void applyClearSelection();
        void applyHEMeshHighlight(const HEMeshHighlightEvent& event);
        void updateViewDependentHighlights();

        /// Applies a geometry display mode to every imported B-Rep model in the scene.
        void applyGeometryDisplayMode(const GeometryDisplayModeEvent& event);

        /// Returns the geometry display mode currently applied to the scene.
        GeometryDisplayMode getGeometryDisplayMode() const;

        /// Shows or hides the B-Rep vertex markers of every geometry model.
        void applyGeometryVertexDisplay(const GeometryVertexDisplayEvent& event);

        /// Returns whether geometry vertex markers are shown.
        bool getGeometryVertexDisplay() const;

        /// Turns the zebra (isophote) stripe analysis of geometry faces on or off.
        void applyGeometryZebra(const GeometryZebraEvent& event);

        /// Turns the surface-UV colouring of geometry faces on or off.
        void applyGeometryUv(const GeometryUvEvent& event);

        // ---- pick phase: query the scene and expose the highlight as a scene primitive ----
        void applyPick(const PickEvent& event);

        /// @brief Copies the current selection together with aggregate counts.
        void getSelectionSnapshot(SelectionSnapshot& snapshot) const;
        /// @brief Returns the selection revision used to notify the UI only on changes.
        uint64_t getSelectionRevision() const;

    private:
        struct MeshColor
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::Vector4f color;
        };

        struct MeshElementVisibilityState
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::HashSet<ME::IdType> hiddenElementIds;
        };

        struct MeshNodeVisibilityState
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::HashSet<ME::IdType> hiddenNodeIds;
        };

        // Records which RenderPacket belongs to which mesh, together with the topology
        // translator used for picking and surface rebuilding.
        struct MeshRenderEntry
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::SharedPtr<RE::MeshToTriFace> translator;
            RenderPacket packet;
        };

        Common::Return uploadRenderPacket(const MeshRenderEntry& entry);
        /// Shows or hides the B-Rep vertex markers of every geometry model.
        void applyGeometryNodeVisibility(const Common::SharedPtr<ME::Mesh>& mesh);

        void updateMeshHighlight();
        void clearPickHighlight();
        void notifyPickCleared();
        void updateLocateHighlight();
        void updateElementsVisibility(const ElementsVisibilityEvent& event);
        void updateNodeVisibility(const NodeVisibilityEvent& event);
        void applyNodeVisibilityToPrimitive(const Common::SharedPtr<ME::Mesh>& mesh);
        // Nodes hidden either explicitly or because every element referencing them is hidden.
        Common::HashSet<ME::IdType> collectHiddenNodeIds(
            const Common::SharedPtr<ME::Mesh>& mesh) const;
        void updateHEMeshHighlight(const HEMeshHighlightEvent& event);
        void updateSelectionHighlight();
        void clearSelectionHighlight();
        MeshElementVisibilityState* findElementVisibilityState(
            const Common::SharedPtr<ME::Mesh>& mesh);
        MeshNodeVisibilityState* findNodeVisibilityState(const Common::SharedPtr<ME::Mesh>& mesh);
        const MeshElementVisibilityState* findElementVisibilityState(
            const Common::SharedPtr<ME::Mesh>& mesh) const;
        const MeshNodeVisibilityState* findNodeVisibilityState(
            const Common::SharedPtr<ME::Mesh>& mesh) const;
        Common::DynamicArray<ME::IdType> collectHiddenElementIds(MeshElementVisibilityState& state);
        void rebuildMeshSurface(const Common::SharedPtr<ME::Mesh>& mesh,
                                const Common::DynamicArray<ME::IdType>& hiddenElementIds);
        /// True when the mesh belongs to an imported B-Rep model (GeometryData node).
        bool isGeometryMesh(const Common::SharedPtr<ME::Mesh>& mesh) const;
        /// Display mode in effect for one mesh: geometry meshes follow the global mode,
        /// regular meshes always use the regular mesh appearance.
        GeometryDisplayMode effectiveDisplayMode(const Common::SharedPtr<ME::Mesh>& mesh) const;
        /// Picks the face shader for a geometry mesh: UV display, then zebra, then the regular
        /// shaded face shader. Moves the primitive between shader buckets when it changes.
        void updateGeometryFaceShader(MeshRenderEntry& entry);
        Common::Vector4f getMeshBaseColor(const Common::SharedPtr<ME::Mesh>& mesh) const;
        MeshRenderEntry* findActiveMeshEntry(const Common::SharedPtr<ME::Mesh>& mesh);

    private:
        mutable std::mutex _renderStateMutex;

        Common::SharedPtr<RE::Scene> _scene;
        Common::SharedPtr<RE::Camera> _camera;
        Common::SharedPtr<RE::Renderer> _renderer;
        Common::SharedPtr<RE::MeshSelector> _meshSelector;
        RE::ScenePicker::VisibilityFilter _visibilityFilter;

        Common::DynamicArray<MeshColor> _meshColors;
        Common::DynamicArray<MeshRenderEntry> _activeMeshes;
        Common::DynamicArray<MeshElementVisibilityState> _elementVisibilityStates;
        Common::DynamicArray<MeshNodeVisibilityState> _nodeVisibilityStates;
        Common::DynamicArray<ME::Mesh*> _removedMeshes;

        SelectionModel _selection;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>>
            _selectionHighlightPrimitives;
        Common::SharedPtr<RE::Shader> _selectionHighlightShader;

        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> _requestedHighlightedMeshes;
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> _activeHighlightedMeshes;
        Common::SharedPtr<RE::Shader> _highlightShader;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _highlightPrimitives;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _highlightContourPrimitives;
        Common::Vector4f _highlightColor{1.0f, 0.75f, 0.7f, 0.3f};

        Common::SharedPtr<ME::Mesh> _locateMesh;
        Common::DynamicArray<uint32_t> _locateElementIds;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _locatePrimitives;
        Common::SharedPtr<RE::RenderPrimitive> _locateTargetFace;
        Common::SharedPtr<ME::Mesh> _locateTargetMesh;
        Common::Vector4f _locateColor{1.0f, 0.55f, 0.1f, 1.0f};

        float _nodePointSize{3.0f};
        float _lineWidth{2.0f};
        float _faceShrink{0.0f};
        GeometryDisplayMode _geometryDisplayMode{GeometryDisplayMode::Mesh};
        bool _geometryShowVertices{true};
        GeometryZebra _geometryZebra;
        GeometryUv _geometryUv;

        RE::CoordType _highlightViewPosition;
        RE::CoordType _highlightViewFront;
        bool _highlightViewValid{false};

        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _heMeshHighlightPrimitives;
        Common::SharedPtr<RE::Shader> _heMeshHighlightShader;

        Common::SharedPtr<RE::RenderPrimitive> _pickHighlightPrimitive;
    };
} // namespace EgLab::Platform
