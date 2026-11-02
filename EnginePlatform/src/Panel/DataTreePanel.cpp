#include "Panel/DataTreePanel.hpp"

#include "Command/CommandManager.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
#include "DataBase/MeshData.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    bool DataTreePanel::renderMeshNode(MeshData& meshData)
    {
        const auto& mesh = meshData.getMesh();
        size_t selectedIndex = _selectedMeshes.size();
        for (size_t index = 0; index < _selectedMeshes.size(); ++index)
        {
            if (_selectedDataNodeIds[index] == meshData.getId())
            {
                selectedIndex = index;
                break;
            }
        }
        const bool isSelected = selectedIndex < _selectedMeshes.size();
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (isSelected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        ImGui::TreeNodeEx(&meshData, flags, "%s", meshData.getName().c_str());
        const bool isClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

        if (isClicked)
        {
            const bool shiftDown = ImGui::GetIO().KeyShift;
            if (!shiftDown)
            {
                _selectedMeshes.clear();
                _selectedDataNodeIds.clear();
                auto selectedMesh = mesh;
                _selectedMeshes.pushBack(selectedMesh);
                _selectedDataNodeIds.pushBack(meshData.getId());
            }
            else if (!isSelected)
            {
                auto selectedMesh = mesh;
                _selectedMeshes.pushBack(selectedMesh);
                _selectedDataNodeIds.pushBack(meshData.getId());
            }
            else
            {
                Common::DynamicArray<Common::SharedPtr<ME::Mesh>> remainingMeshes;
                Common::DynamicArray<DataNodeId> remainingNodeIds;
                for (size_t index = 0; index < _selectedMeshes.size(); ++index)
                {
                    if (index != selectedIndex)
                    {
                        auto selectedMesh = _selectedMeshes[index];
                        remainingMeshes.pushBack(selectedMesh);
                        remainingNodeIds.pushBack(_selectedDataNodeIds[index]);
                    }
                }
                _selectedMeshes = Common::move(remainingMeshes);
                _selectedDataNodeIds = Common::move(remainingNodeIds);
            }
            RenderWork::instance().setHighlightedMeshes(_selectedMeshes);
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create Half-Edge Mesh"))
            {
                Context::instance().getPanelManager().openHEMeshPanel(mesh, meshData.getName());
            }

            if (ImGui::MenuItem("delete Mesh"))
            {
                Common::DynamicArray<DataNodeId> nodeIdsToDelete;
                if (isSelected)
                {
                    for (size_t index = 0; index < _selectedDataNodeIds.size(); ++index)
                    {
                        nodeIdsToDelete.pushBack(_selectedDataNodeIds[index]);
                    }
                }
                else
                {
                    nodeIdsToDelete.pushBack(meshData.getId());
                }

                _selectedMeshes.clear();
                _selectedDataNodeIds.clear();
                RenderWork::instance().setHighlightedMeshes(_selectedMeshes);
                for (size_t index = 0; index < nodeIdsToDelete.size(); ++index)
                {
                    EventPacket packet;
                    packet.setData("dataNodeId", nodeIdsToDelete[index]);
                    CommandManager::instance().call(EventId::MeshDelete, packet);
                }
            }

            ImGui::PushID(&meshData);
            const auto& color = meshData.getColor();
            float editableColor[] = {color[0], color[1], color[2], color[3]};
            if (ImGui::ColorEdit4("Color", editableColor,
                                  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
            {
                Common::Vector4f newColor;
                for (size_t index = 0; index < 4; ++index)
                {
                    newColor[index] = editableColor[index];
                }
                meshData.setColor(newColor);
                RenderWork::instance().setMeshColor(meshData.getMesh(), newColor);
            }
            ImGui::PopID();

            ImGui::EndPopup();
        }

        return false;
    }

    bool DataTreePanel::renderCommonNode(DataTree& node, bool isSelected)
    {
        const auto nodes = node.getDataNodes();
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (isSelected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (nodes.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        const bool isOpen = ImGui::TreeNodeEx(&node, flags, "%s", node.getName().c_str());
        return isOpen;
    }

    bool DataTreePanel::renderNode(DataTree& node)
    {
        const bool isOpen = renderCommonNode(node);
        if (!isOpen)
        {
            return false;
        }

        for (const auto& dataNode : node.getDataNodes())
        {
            if (auto* meshData = dynamic_cast<MeshData*>(dataNode.get()))
            {
                renderMeshNode(*meshData);
            }
            else
            {
                ImGui::TreeNodeEx(dataNode.get(),
                                  ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
                                  "%s", dataNode->getName().c_str());
            }
        }
        ImGui::TreePop();
        return isOpen;
    }

    DataTreePanel::DataTreePanel()
    {
    }

    DataTreePanel::~DataTreePanel()
    {
    }

    void DataTreePanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Data Tree", &_show))
        {
            // std::lock_guard<std::mutex> lock(Context::instance().getDataTreeMutex());
            renderNode(Context::instance().getDataTree());
            if (!_selectedMeshes.empty() && ImGui::IsWindowHovered() &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
            {
                _selectedMeshes.clear();
                _selectedDataNodeIds.clear();
                RenderWork::instance().setHighlightedMeshes(_selectedMeshes);
            }
        }
        ImGui::End();
    }
} // namespace EgLab::Platform
