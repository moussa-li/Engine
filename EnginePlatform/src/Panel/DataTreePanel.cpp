#include "Panel/DataTreePanel.hpp"

#include <cstdint>

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
            if (_selectedMeshes[index] == mesh)
            {
                selectedIndex = index;
                break;
            }
        }
        const bool isSelected = selectedIndex < _selectedMeshes.size();
        const bool isOpen = renderCommonNode(meshData, isSelected);
        const bool isClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

        if (isClicked)
        {
            const bool shiftDown = ImGui::GetIO().KeyShift;
            if (!shiftDown)
            {
                _selectedMeshes.clear();
                auto selectedMesh = mesh;
                _selectedMeshes.pushBack(selectedMesh);
            }
            else if (!isSelected)
            {
                auto selectedMesh = mesh;
                _selectedMeshes.pushBack(selectedMesh);
            }
            else
            {
                Common::DynamicArray<Common::SharedPtr<ME::Mesh>> remainingMeshes;
                for (size_t index = 0; index < _selectedMeshes.size(); ++index)
                {
                    if (index != selectedIndex)
                    {
                        auto selectedMesh = _selectedMeshes[index];
                        remainingMeshes.pushBack(selectedMesh);
                    }
                }
                _selectedMeshes = Common::move(remainingMeshes);
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
                Common::DynamicArray<Common::SharedPtr<ME::Mesh>> meshesToDelete;
                if (isSelected)
                {
                    for (size_t index = 0; index < _selectedMeshes.size(); ++index)
                    {
                        auto selectedMesh = _selectedMeshes[index];
                        meshesToDelete.pushBack(selectedMesh);
                    }
                }
                else
                {
                    auto meshToDelete = mesh;
                    meshesToDelete.pushBack(meshToDelete);
                }

                _selectedMeshes.clear();
                RenderWork::instance().setHighlightedMeshes(_selectedMeshes);
                for (size_t index = 0; index < meshesToDelete.size(); ++index)
                {
                    auto meshToDelete = reinterpret_cast<uintptr_t>(meshesToDelete[index].get());
                    EventPacket packet;
                    packet.setData("meshAddress", meshToDelete);
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

        return isOpen;
    }

    bool DataTreePanel::renderCommonNode(DataTree& node, bool isSelected)
    {
        const auto children = node.getChildrens();
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (isSelected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        const bool isOpen = ImGui::TreeNodeEx(&node, flags, "%s", node.getName().c_str());
        return isOpen;
    }

    bool DataTreePanel::renderNode(DataTree& node)
    {
        auto* meshData = dynamic_cast<MeshData*>(&node);
        bool isOpen = false;
        if (meshData != nullptr)
        {
            isOpen = renderMeshNode(*meshData);
        }
        else
        {
            isOpen = renderCommonNode(node);
        }

        const auto children = node.getChildrens();

        if (isOpen && (!children.empty()))
        {
            for (DataTree* child : children)
            {
                if (renderNode(*child))
                {
                    // ImGui::TreePop();
                }
            }
        }
        if (isOpen && (!children.empty()))
        {
            ImGui::TreePop();
        }
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
                RenderWork::instance().setHighlightedMeshes(_selectedMeshes);
            }
        }
        ImGui::End();
    }
} // namespace EgLab::Platform
