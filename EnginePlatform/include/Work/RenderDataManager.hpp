#pragma once

#include <cstdint>
#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/HashSet.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "MeshEngine/Common/Definitions.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "Work/RenderEvent.hpp"

namespace EgLab::ME
{
    class Mesh;
    class HEMesh;
} // namespace EgLab::ME

namespace EgLab::RE
{
    class Scene;
    class Camera;
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
        void bindMeshSelector(const Common::SharedPtr<RE::MeshSelector>& selector);
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
        void fitView(const Common::SharedPtr<ME::Mesh>& mesh);

        /// Returns the bounding-box center of the mesh's visible primitives, the same
        /// primitives fitView uses, so hidden element changes are respected.
        bool getMeshCenter(const Common::SharedPtr<ME::Mesh>& mesh, RE::CoordType& center) const;

        // ---- apply phase: mutate the scene ----
        Common::Return applyMeshUpdate(const MeshUpdateEvent& event);
        void applyMeshColor(const Common::SharedPtr<ME::Mesh>& mesh, const Common::Vector4f& color);
        void applyMeshRemoval(const Common::SharedPtr<ME::Mesh>& mesh);
        void applyHighlight();
        void applyHighlightColor();
        void applyNodePointSize(float pointSize);
        void applyLineWidth(float lineWidth);
        void applyFaceShrink(float shrink);
        void applyLocate();
        void applyElementsVisibility(const ElementsVisibilityEvent& event);
        void applyHEMeshHighlight(const HEMeshHighlightEvent& event);
        void updateViewDependentHighlights();

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

        // Records which RenderPacket belongs to which mesh, together with the topology
        // translator used for picking and surface rebuilding.
        struct MeshRenderEntry
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::SharedPtr<RE::MeshToTriFace> translator;
            RenderPacket packet;
        };

        Common::Return uploadRenderPacket(const MeshRenderEntry& entry);
        void updateMeshHighlight();
        void updateLocateHighlight();
        void updateElementsVisibility(const ElementsVisibilityEvent& event);
        void updateHEMeshHighlight(const HEMeshHighlightEvent& event);
        MeshElementVisibilityState* findElementVisibilityState(
            const Common::SharedPtr<ME::Mesh>& mesh);
        Common::DynamicArray<ME::IdType> collectHiddenElementIds(MeshElementVisibilityState& state);
        void rebuildMeshSurface(const Common::SharedPtr<ME::Mesh>& mesh,
                                const Common::DynamicArray<ME::IdType>& hiddenElementIds);
        Common::Vector4f getMeshBaseColor(const Common::SharedPtr<ME::Mesh>& mesh) const;

    private:
        mutable std::mutex _renderStateMutex;

        Common::SharedPtr<RE::Scene> _scene;
        Common::SharedPtr<RE::Camera> _camera;
        Common::SharedPtr<RE::MeshSelector> _meshSelector;

        Common::DynamicArray<MeshColor> _meshColors;
        Common::DynamicArray<MeshRenderEntry> _activeMeshes;
        Common::DynamicArray<MeshElementVisibilityState> _elementVisibilityStates;
        Common::DynamicArray<ME::Mesh*> _removedMeshes;

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

        RE::CoordType _highlightViewPosition;
        RE::CoordType _highlightViewFront;
        bool _highlightViewValid{false};

        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _heMeshHighlightPrimitives;
        Common::SharedPtr<RE::Shader> _heMeshHighlightShader;
    };
} // namespace EgLab::Platform
