#include "Work/RenderEventHandler.hpp"

#include <cstring>

#include "Command/MeshPODWire.hpp"
#include "Common/Log.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "Work/RenderDataManager.hpp"

namespace EgLab::Platform
{
    class MeshColorHandler : public IRenderEventHandler
    {
    public:
        explicit MeshColorHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<MeshColorHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const MeshColorEvent&>(event);
            return _data.recordMeshColor(e.mesh, e.color) ? Common::Return::Succeed
                                                           : Common::Return::Failed;
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const MeshColorEvent&>(event);
            _data.applyMeshColor(e.mesh, e.color);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class MeshUpdateHandler : public IRenderEventHandler
    {
    public:
        explicit MeshUpdateHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<MeshUpdateHandler>(data);
        }

        Common::Return prepare(RenderEvent& event) override
        {
            auto& e = static_cast<MeshUpdateEvent&>(event);
            Common::SharedPtr<ME::Mesh> renderMesh = e.mesh;
            if (!e.payload.empty())
            {
                if (e.payload.size() <= sizeof(MeshPODWireHeader))
                {
                    LOG(ERROR) << "MeshUpdateHandler::prepare() received truncated mesh data";
                    return Common::Return::Failed;
                }

                MeshPODWireHeader wire{};
                std::memcpy(&wire, e.payload.data(), sizeof(wire));
                if (wire.totalDataSize == 0 ||
                    wire.totalDataSize > e.payload.size() - sizeof(MeshPODWireHeader))
                {
                    LOG(ERROR) << "MeshUpdateHandler::prepare() received invalid mesh data size";
                    return Common::Return::Failed;
                }

                const uint32_t offsets[] = {
                    wire.nodeIdsOffset,   wire.nodeDataOffset, wire.elemIdsOffset,
                    wire.elemTypesOffset, wire.elemDataOffset,
                };
                for (uint32_t offset : offsets)
                {
                    if (offset > wire.totalDataSize)
                    {
                        LOG(ERROR) << "MeshUpdateHandler::prepare() received invalid mesh offset";
                        return Common::Return::Failed;
                    }
                }

                ME::MeshPOD pod{};
                pod.nodeNumber = wire.nodeNumber;
                pod.elemNumber = wire.elemNumber;
                pod.nodeIdsOffset = wire.nodeIdsOffset;
                pod.nodeDataOffset = wire.nodeDataOffset;
                pod.elemIdsOffset = wire.elemIdsOffset;
                pod.elemTypesOffset = wire.elemTypesOffset;
                pod.elemDataOffset = wire.elemDataOffset;
                pod.totalDataSize = wire.totalDataSize;
                pod.data = new uint8_t[pod.totalDataSize];
                std::memcpy(pod.data, e.payload.data() + sizeof(MeshPODWireHeader),
                            pod.totalDataSize);

                renderMesh = ME::MeshToPOD::deserialize(pod);
                if (!renderMesh)
                {
                    LOG(ERROR) << "MeshUpdateHandler::prepare() failed to deserialize mesh";
                    return Common::Return::Failed;
                }
            }
            if (!renderMesh)
            {
                LOG(ERROR) << "MeshUpdateHandler::prepare() cannot update a null mesh";
                return Common::Return::Failed;
            }

            RE::MeshPrimitiveCreator creator(renderMesh);
            e.packet.renderNode = creator.getPrimitive<RE::RenderNode>();
            e.packet.renderLine = creator.getPrimitive<RE::RenderLine>();
            e.packet.renderFace = creator.getPrimitive<RE::RenderFace>();
            e.translator = creator.getTranslator();
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent& event) override
        {
            return _data.applyMeshUpdate(static_cast<const MeshUpdateEvent&>(event));
        }

    private:
        RenderDataManager& _data;
    };

    class MeshRemovalHandler : public IRenderEventHandler
    {
    public:
        explicit MeshRemovalHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<MeshRemovalHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const MeshRemovalEvent&>(event);
            return _data.recordMeshRemoval(e.mesh) ? Common::Return::Succeed
                                                   : Common::Return::Failed;
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const MeshRemovalEvent&>(event);
            _data.applyMeshRemoval(e.mesh);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class HighlightHandler : public IRenderEventHandler
    {
    public:
        explicit HighlightHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<HighlightHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const HighlightEvent&>(event);
            _data.replaceRequestedHighlightedMeshes(e.meshes);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent&) override
        {
            _data.applyHighlight();
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class HighlightColorHandler : public IRenderEventHandler
    {
    public:
        explicit HighlightColorHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<HighlightColorHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const HighlightColorEvent&>(event);
            _data.setHighlightColor(e.color);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent&) override
        {
            _data.applyHighlightColor();
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class NodePointSizeHandler : public IRenderEventHandler
    {
    public:
        explicit NodePointSizeHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<NodePointSizeHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const NodePointSizeEvent&>(event);
            _data.setNodePointSize(e.pointSize);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const NodePointSizeEvent&>(event);
            _data.applyNodePointSize(e.pointSize);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class LineWidthHandler : public IRenderEventHandler
    {
    public:
        explicit LineWidthHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<LineWidthHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const LineWidthEvent&>(event);
            _data.setLineWidth(e.lineWidth);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const LineWidthEvent&>(event);
            _data.applyLineWidth(e.lineWidth);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class FaceShrinkHandler : public IRenderEventHandler
    {
    public:
        explicit FaceShrinkHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<FaceShrinkHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const FaceShrinkEvent&>(event);
            _data.setFaceShrink(e.faceShrink);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const FaceShrinkEvent&>(event);
            _data.applyFaceShrink(e.faceShrink);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class GeometryDisplayModeHandler : public IRenderEventHandler
    {
    public:
        explicit GeometryDisplayModeHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<GeometryDisplayModeHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const GeometryDisplayModeEvent&>(event);
            _data.applyGeometryDisplayMode(e);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class GeometryVertexDisplayHandler : public IRenderEventHandler
    {
    public:
        explicit GeometryVertexDisplayHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<GeometryVertexDisplayHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const GeometryVertexDisplayEvent&>(event);
            _data.applyGeometryVertexDisplay(e);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class GeometryZebraHandler : public IRenderEventHandler
    {
    public:
        explicit GeometryZebraHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<GeometryZebraHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const GeometryZebraEvent&>(event);
            _data.applyGeometryZebra(e);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class GeometryUvHandler : public IRenderEventHandler
    {
    public:
        explicit GeometryUvHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<GeometryUvHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            const auto& e = static_cast<const GeometryUvEvent&>(event);
            _data.applyGeometryUv(e);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class ReferencePlaneSettingsHandler : public IRenderEventHandler    {
    public:
        explicit ReferencePlaneSettingsHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<ReferencePlaneSettingsHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyReferencePlaneSettings(
                static_cast<const ReferencePlaneSettingsEvent&>(event).param);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class HEMeshHighlightHandler : public IRenderEventHandler
    {
    public:
        explicit HEMeshHighlightHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<HEMeshHighlightHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyHEMeshHighlight(static_cast<const HEMeshHighlightEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class LocateHandler : public IRenderEventHandler
    {
    public:
        explicit LocateHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<LocateHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const LocateEvent&>(event);
            _data.setLocateState(e.mesh, e.elementIds);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent&) override
        {
            _data.applyLocate();
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class GeometryFeatureHighlightHandler : public IRenderEventHandler
    {
    public:
        explicit GeometryFeatureHighlightHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<GeometryFeatureHighlightHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const GeometryFeatureHighlightEvent&>(event);
            _data.setGeometryFeatureHighlightState(e.mesh, e.elementIds, e.edgeSegmentNodeIndices,
                                                   e.nodeIndices);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent&) override
        {
            _data.applyGeometryFeatureHighlight();
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class GeometryFeatureFocusHandler : public IRenderEventHandler
    {
    public:
        explicit GeometryFeatureFocusHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<GeometryFeatureFocusHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const GeometryFeatureFocusEvent&>(event);
            _data.setGeometryFeatureFocusState(e.mesh, e.elementIds, e.edgeSegmentNodeIndices,
                                               e.nodeIndices);
            return Common::Return::Succeed;
        }

        Common::Return apply(const RenderEvent&) override
        {
            _data.applyGeometryFeatureFocus();
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class ElementsVisibilityHandler : public IRenderEventHandler
    {
    public:
        explicit ElementsVisibilityHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<ElementsVisibilityHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyElementsVisibility(static_cast<const ElementsVisibilityEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class NodeVisibilityHandler : public IRenderEventHandler
    {
    public:
        explicit NodeVisibilityHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<NodeVisibilityHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyNodeVisibility(static_cast<const NodeVisibilityEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class BoxSelectionHandler : public IRenderEventHandler
    {
    public:
        explicit BoxSelectionHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<BoxSelectionHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyBoxSelection(static_cast<const BoxSelectEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class PointSelectionHandler : public IRenderEventHandler
    {
    public:
        explicit PointSelectionHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<PointSelectionHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyPointSelection(static_cast<const PointSelectEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class ClearSelectionHandler : public IRenderEventHandler
    {
    public:
        explicit ClearSelectionHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<ClearSelectionHandler>(data);
        }

        Common::Return apply(const RenderEvent&) override
        {
            _data.applyClearSelection();
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class FitViewHandler : public IRenderEventHandler
    {
    public:
        explicit FitViewHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<FitViewHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            const auto& e = static_cast<const FitViewEvent&>(event);
            _data.fitView(e.mesh);
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class SetCameraHandler : public IRenderEventHandler
    {
    public:
        explicit SetCameraHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<SetCameraHandler>(data);
        }

        Common::Return record(const RenderEvent& event) override
        {
            _data.setCamera(static_cast<const SetCameraEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    class PickHandler : public IRenderEventHandler
    {
    public:
        explicit PickHandler(RenderDataManager& data) : _data(data)
        {
        }

        static Common::SharedPtr<IRenderEventHandler> create(RenderDataManager& data)
        {
            return Common::makeShared<PickHandler>(data);
        }

        Common::Return apply(const RenderEvent& event) override
        {
            _data.applyPick(static_cast<const PickEvent&>(event));
            return Common::Return::Succeed;
        }

    private:
        RenderDataManager& _data;
    };

    using CreateFunc = Common::SharedPtr<IRenderEventHandler> (*)(RenderDataManager&);

    struct HandlerEntry
    {
        RenderEventType type;
        CreateFunc create;
    };

    const HandlerEntry kHandlerEntries[] = {
        {RenderEventType::MeshColor, &MeshColorHandler::create},
        {RenderEventType::MeshUpdate, &MeshUpdateHandler::create},
        {RenderEventType::MeshRemoval, &MeshRemovalHandler::create},
        {RenderEventType::Highlight, &HighlightHandler::create},
        {RenderEventType::HighlightColor, &HighlightColorHandler::create},
        {RenderEventType::NodePointSize, &NodePointSizeHandler::create},
        {RenderEventType::LineWidth, &LineWidthHandler::create},
        {RenderEventType::FaceShrink, &FaceShrinkHandler::create},
        {RenderEventType::GeometryDisplayMode, &GeometryDisplayModeHandler::create},
        {RenderEventType::GeometryVertexDisplay, &GeometryVertexDisplayHandler::create},
        {RenderEventType::GeometryZebra, &GeometryZebraHandler::create},
        {RenderEventType::GeometryUv, &GeometryUvHandler::create},
        {RenderEventType::ReferencePlaneSettings, &ReferencePlaneSettingsHandler::create},
        {RenderEventType::HEMeshHighlight, &HEMeshHighlightHandler::create},
        {RenderEventType::Locate, &LocateHandler::create},
        {RenderEventType::GeometryFeatureHighlight, &GeometryFeatureHighlightHandler::create},
        {RenderEventType::GeometryFeatureFocus, &GeometryFeatureFocusHandler::create},
        {RenderEventType::ElementsVisibility, &ElementsVisibilityHandler::create},
        {RenderEventType::NodeVisibility, &NodeVisibilityHandler::create},
        {RenderEventType::BoxSelection, &BoxSelectionHandler::create},
        {RenderEventType::PointSelection, &PointSelectionHandler::create},
        {RenderEventType::ClearSelection, &ClearSelectionHandler::create},
        {RenderEventType::FitView, &FitViewHandler::create},
        {RenderEventType::Pick, &PickHandler::create},
        {RenderEventType::SetCamera, &SetCameraHandler::create},
    };

    RenderEventHandlerFactory::RenderEventHandlerFactory(RenderDataManager& data) : _data(data)
    {
    }

    Common::SharedPtr<IRenderEventHandler> RenderEventHandlerFactory::create(
        RenderEventType type) const
    {
        for (const auto& entry : kHandlerEntries)
        {
            if (entry.type == type)
            {
                return entry.create(_data);
            }
        }
        return Common::SharedPtr<IRenderEventHandler>(nullptr);
    }
} // namespace EgLab::Platform