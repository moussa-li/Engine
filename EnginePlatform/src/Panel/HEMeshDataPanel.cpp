#include "Panel/HEMeshDataPanel.hpp"

#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
#include "DataBase/MeshData.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    void HEMeshDataPanel::renderMeshNode(MeshData& meshData)
    {
        const auto& mesh = meshData.getMesh();
        const bool isSelected = _selectedMesh == mesh;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (isSelected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        ImGui::TreeNodeEx(&meshData, flags, "%s", meshData.getName().c_str());
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            _selectedMesh = mesh;
        }

        if (ImGui::BeginPopupContextItem("HEMeshDataContext"))
        {
            if (ImGui::MenuItem("Create Half-Edge Mesh"))
            {
                Context::instance().getPanelManager().openHEMeshPanel(mesh, meshData.getName());
                _selectedMesh = mesh;
            }
            ImGui::EndPopup();
        }
    }

    void HEMeshDataPanel::renderNode(DataTree& node)
    {
        if (auto* meshData = dynamic_cast<MeshData*>(&node))
        {
            renderMeshNode(*meshData);
            return;
        }

        const auto children = node.getChildrens();
        const ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (ImGui::TreeNodeEx(&node, flags, "%s", node.getName().c_str()))
        {
            for (DataTree* child : children)
            {
                renderNode(*child);
            }
            ImGui::TreePop();
        }
    }

    void HEMeshDataPanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Half-Edge Mesh Data", &_show))
        {
            renderNode(Context::instance().getDataTree());
        }
        ImGui::End();
    }
} // namespace EgLab::Platform