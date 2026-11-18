#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"

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
        FitView,
        Pick,
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
} // namespace EgLab::Platform
