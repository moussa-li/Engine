#pragma once

#include <cstdint>

#include "Command/CommandParam.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "Work/GeometryDisplayMode.hpp"

namespace EgLab::ME
{
    class Mesh;
    class HEMesh;
} // namespace EgLab::ME

namespace EgLab::RE
{
    class RenderPrimitive;
    class Shader;
    class MeshToTriFace;
} // namespace EgLab::RE

namespace EgLab::Platform
{
    /// Render primitives and the shaders used to draw them. The association between a mesh
    /// and its RenderPacket is recorded by RenderDataManager, not carried by the packet.
    struct RenderPacket
    {
        Common::SharedPtr<RE::RenderPrimitive> renderNode;
        Common::SharedPtr<RE::RenderPrimitive> renderLine;
        Common::SharedPtr<RE::RenderPrimitive> renderFace;
        Common::SharedPtr<RE::Shader> nodeShader;
        Common::SharedPtr<RE::Shader> lineShader;
        Common::SharedPtr<RE::Shader> faceShader;
    };

    /// Kinds of rendering change requests accepted by RenderWork.
    enum class RenderEventType
    {
        MeshColor,
        MeshUpdate,
        MeshRemoval,
        Highlight,
        HighlightColor,
        NodePointSize,
        LineWidth,
        FaceShrink,
        HEMeshHighlight,
        Locate,
        ElementsVisibility,
        NodeVisibility,
        BoxSelection,
        PointSelection,
        ClearSelection,
        FitView,
        Pick,
        SetCamera,
        ReferencePlaneSettings,
        GeometryDisplayMode,
    };

    /// Polymorphic base for rendering change requests. Each concrete event type carries only
    /// the data it needs; RenderEvent itself just exposes the discriminator used by the
    /// RenderEventHandlerFactory.
    class RenderEvent
    {
    public:
        virtual ~RenderEvent() = default;

        RenderEventType getType() const
        {
            return _type;
        }

    protected:
        explicit RenderEvent(RenderEventType type) : _type(type)
        {
        }

    private:
        RenderEventType _type;
    };

    class MeshColorEvent : public RenderEvent
    {
    public:
        MeshColorEvent() : RenderEvent(RenderEventType::MeshColor)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
        Common::Vector4f color;
    };

    class MeshUpdateEvent : public RenderEvent
    {
    public:
        MeshUpdateEvent() : RenderEvent(RenderEventType::MeshUpdate)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
        Common::DynamicArray<uint8_t> payload;
        RenderPacket packet;
        // Transient topology translator produced while preparing the event. It is consumed
        // and recorded by RenderDataManager when the event is applied.
        Common::SharedPtr<RE::MeshToTriFace> translator;
    };

    class MeshRemovalEvent : public RenderEvent
    {
    public:
        MeshRemovalEvent() : RenderEvent(RenderEventType::MeshRemoval)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
    };

    class HighlightEvent : public RenderEvent
    {
    public:
        HighlightEvent() : RenderEvent(RenderEventType::Highlight)
        {
        }

        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> meshes;
    };

    class HighlightColorEvent : public RenderEvent
    {
    public:
        HighlightColorEvent() : RenderEvent(RenderEventType::HighlightColor)
        {
        }

        Common::Vector4f color;
    };

    class NodePointSizeEvent : public RenderEvent
    {
    public:
        NodePointSizeEvent() : RenderEvent(RenderEventType::NodePointSize)
        {
        }

        float pointSize{3.0f};
    };

    class LineWidthEvent : public RenderEvent
    {
    public:
        LineWidthEvent() : RenderEvent(RenderEventType::LineWidth)
        {
        }

        float lineWidth{2.0f};
    };

    class FaceShrinkEvent : public RenderEvent
    {
    public:
        FaceShrinkEvent() : RenderEvent(RenderEventType::FaceShrink)
        {
        }

        float faceShrink{0.0f};
    };

    class HEMeshHighlightEvent : public RenderEvent
    {
    public:
        HEMeshHighlightEvent() : RenderEvent(RenderEventType::HEMeshHighlight)
        {
        }

        Common::SharedPtr<ME::HEMesh> heMesh;
        uint32_t vertexIndex{UINT32_MAX};
        uint32_t halfEdgeIndex{UINT32_MAX};
    };

    class LocateEvent : public RenderEvent
    {
    public:
        LocateEvent() : RenderEvent(RenderEventType::Locate)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
        Common::DynamicArray<uint32_t> elementIds;
    };

    class ElementsVisibilityEvent : public RenderEvent
    {
    public:
        ElementsVisibilityEvent() : RenderEvent(RenderEventType::ElementsVisibility)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
        Common::DynamicArray<uint32_t> elementIds;
        uint32_t mode{0};
    };

    class NodeVisibilityEvent : public RenderEvent
    {
    public:
        NodeVisibilityEvent() : RenderEvent(RenderEventType::NodeVisibility)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
        Common::DynamicArray<uint32_t> nodeIds;
        uint32_t mode{0};
    };

    /// Requests a rubber-band selection over the scene.
    class BoxSelectEvent : public RenderEvent
    {
    public:
        BoxSelectEvent() : RenderEvent(RenderEventType::BoxSelection)
        {
        }

        /// Rectangle in framebuffer pixels; the origin is the top-left corner.
        float minX{0.0f};
        float minY{0.0f};
        float maxX{0.0f};
        float maxY{0.0f};
        unsigned int width{0};
        unsigned int height{0};
        /// True selects fully contained entities, false selects intersecting entities.
        bool containsOnly{true};
        /// True merges the result into the existing selection.
        bool additive{false};
        /// Pick kinds participating in the query.
        uint32_t kinds{0};
        /// Optional mesh scope; null means every visible mesh.
        Common::SharedPtr<ME::Mesh> scopeMesh;
    };

    /// Requests a single point selection at a cursor position. Used when the box tool is
    /// active but the user clicks without dragging, so point picks join the selection set and
    /// can be hidden or shown from the visibility panel.
    class PointSelectEvent : public RenderEvent
    {
    public:
        PointSelectEvent() : RenderEvent(RenderEventType::PointSelection)
        {
        }

        /// Cursor position in framebuffer pixels; the origin is the top-left corner.
        float x{0.0f};
        float y{0.0f};
        unsigned int width{0};
        unsigned int height{0};
        /// True merges the hit into the existing selection.
        bool additive{false};
        /// Pick kinds participating in the query.
        uint32_t kinds{0};
        /// Optional mesh scope; null means every visible mesh.
        Common::SharedPtr<ME::Mesh> scopeMesh;
    };

    /// Requests resetting the current selection set.
    class ClearSelectionEvent : public RenderEvent
    {
    public:
        ClearSelectionEvent() : RenderEvent(RenderEventType::ClearSelection)
        {
        }
    };

    class FitViewEvent : public RenderEvent
    {
    public:
        FitViewEvent() : RenderEvent(RenderEventType::FitView)
        {
        }

        Common::SharedPtr<ME::Mesh> mesh;
    };

    /// Requests one pick query at a cursor position.
    /// The handler inserts the resulting highlight primitive into the scene, so picking
    /// output is rendered like any other scene content.
    class PickEvent : public RenderEvent
    {
    public:
        PickEvent() : RenderEvent(RenderEventType::Pick)
        {
        }

        /// Cursor position in framebuffer pixels.
        float x{0.0f};
        float y{0.0f};
        /// Kinds accepted by this query; 0 clears the current pick highlight.
        uint32_t kinds{0};
    };

    class SetCameraEvent : public RenderEvent
    {
    public:
        SetCameraEvent() : RenderEvent(RenderEventType::SetCamera)
        {
        }

        SetCameraParam param;
    };

    /// Requests a partial update of the reference plane appearance. The update is applied on
    /// the render thread so the renderer state stays single-threaded.
    class ReferencePlaneSettingsEvent : public RenderEvent
    {
    public:
        ReferencePlaneSettingsEvent() : RenderEvent(RenderEventType::ReferencePlaneSettings)
        {
        }

        SetReferencePlaneParam param;
    };

    /// Requests changing how imported B-Rep geometry is drawn (mesh / face outline / edges).
    class GeometryDisplayModeEvent : public RenderEvent
    {
    public:
        GeometryDisplayModeEvent() : RenderEvent(RenderEventType::GeometryDisplayMode)
        {
        }

        uint32_t mode{0};
    };
} // namespace EgLab::Platform
