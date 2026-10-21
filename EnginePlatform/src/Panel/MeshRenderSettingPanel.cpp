#include "Panel/MeshRenderSettingPanel.hpp"

#include <sstream>

#include "DataBase/Context.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    MeshRenderSettingPanel::~MeshRenderSettingPanel()
    {
        if (_pickInfoSubscriptionId != 0)
        {
            RenderWork::instance().unsubscribePickInfo(_pickInfoSubscriptionId);
        }
    }

    void MeshRenderSettingPanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(320, 320), ImGuiCond_FirstUseEver);
        ImGui::Begin("Mesh Render Settings", &_show);

        auto renderer = Context::instance().getApplication().renderer;
        if (renderer)
        {
            if (_pickInfoSubscriptionId == 0)
            {
                _pickInfoSubscriptionId = RenderWork::instance().subscribePickInfo();
            }

            PickedMeshInfo pickedInfo;
            while (RenderWork::instance().tryReceivePickInfo(_pickInfoSubscriptionId, pickedInfo))
            {
                _pickedInfo = pickedInfo;
                _hasPickedInfo = pickedInfo.kind != PickedMeshKind::None;
            }

            bool renderNodes = renderer->isRenderNodesEnabled();
            bool renderEdges = renderer->isRenderEdgesEnabled();
            bool renderFaces = renderer->isRenderFacesEnabled();

            if (ImGui::Checkbox("Nodes", &renderNodes))
            {
                renderer->setRenderNodesEnabled(renderNodes);
            }
            if (ImGui::Checkbox("Edges", &renderEdges))
            {
                renderer->setRenderEdgesEnabled(renderEdges);
            }
            if (ImGui::Checkbox("Faces", &renderFaces))
            {
                renderer->setRenderFacesEnabled(renderFaces);
            }

            if (ImGui::ColorEdit4("Highlight Color", _highlightColor))
            {
                RenderWork::instance().setHighlightColor(
                    Common::Vector4f(_highlightColor[0], _highlightColor[1], _highlightColor[2],
                                     _highlightColor[3]));
            }

            ImGui::Separator();
            ImGui::Checkbox("Node Info", &_showNodeInfo);
            ImGui::Checkbox("Element Info", &_showElementInfo);
            ImGui::Checkbox("Element Edge Info", &_showElementEdgeInfo);
            ImGui::Checkbox("Element Face Info", &_showElementFaceInfo);
        }

        ImGui::End();

        const bool showNodeInfo = _showNodeInfo && _pickedInfo.kind == PickedMeshKind::Node;
        const bool showElementInfo =
            _showElementInfo && (_pickedInfo.kind == PickedMeshKind::ElementEdge ||
                                 _pickedInfo.kind == PickedMeshKind::ElementFace);
        const bool showElementEdgeInfo =
            _showElementEdgeInfo && _pickedInfo.kind == PickedMeshKind::ElementEdge;
        const bool showElementFaceInfo =
            _showElementFaceInfo && _pickedInfo.kind == PickedMeshKind::ElementFace;
        if (_hasPickedInfo &&
            (showNodeInfo || showElementInfo || showElementEdgeInfo || showElementFaceInfo))
        {
            ImGui::BeginTooltip();
            if (showNodeInfo)
            {
                ImGui::Text("Node ID: %u", _pickedInfo.entityId);
                ImGui::Text("XYZ: (%.6g, %.6g, %.6g)", _pickedInfo.x, _pickedInfo.y, _pickedInfo.z);
            }

            if (showElementInfo)
            {
                ImGui::Text("Element ID: %u", _pickedInfo.entityId);
                std::ostringstream nodeIds;
                for (size_t index = 0; index < _pickedInfo.topologyNodeCount; ++index)
                {
                    if (index != 0)
                    {
                        nodeIds << ", ";
                    }
                    nodeIds << _pickedInfo.topologyNodeIds[index];
                }
                ImGui::Text("Topology node IDs: %s", nodeIds.str().c_str());
            }

            if (showElementEdgeInfo)
            {
                ImGui::Text("Element ID: %u", _pickedInfo.entityId);
                ImGui::Text("Edge index: %u", _pickedInfo.edgeIndex);
            }

            if (showElementFaceInfo)
            {
                ImGui::Text("Element ID: %u", _pickedInfo.entityId);
                ImGui::Text("Face index: %u", _pickedInfo.faceIndex);
            }
            ImGui::EndTooltip();
        }
    }
} // namespace EgLab::Platform