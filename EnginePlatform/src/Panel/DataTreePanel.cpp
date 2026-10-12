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
    namespace
    {
        bool renderDataTreeNode(DataTree& node, DataTree*& selectedNode, uintptr_t& meshToDelete,
                                bool defaultOpen = false)
        {
            const auto children = node.getChildrens();
            auto* meshData = dynamic_cast<MeshData*>(&node);
            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (&node == selectedNode)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (children.empty() && meshData == nullptr)
            {
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }
            if (defaultOpen)
            {
                flags |= ImGuiTreeNodeFlags_DefaultOpen;
            }

            const bool isOpen = ImGui::TreeNodeEx(&node, flags, "%s", node.getName().c_str());
            const bool clicked = ImGui::IsItemClicked();
            bool selectionChanged = false;
            if (clicked)
            {
                selectedNode = meshData != nullptr ? &node : nullptr;
                selectionChanged = true;
            }

            if (meshData != nullptr && ImGui::BeginPopupContextItem("MeshContextMenu"))
            {
                if (ImGui::MenuItem("delete Mesh"))
                {
                    meshToDelete = reinterpret_cast<uintptr_t>(meshData->getMesh().get());
                    if (selectedNode == &node)
                    {
                        selectedNode = nullptr;
                        selectionChanged = true;
                    }
                }
                ImGui::EndPopup();
            }

            if (isOpen && (meshData != nullptr || !children.empty()))
            {
                if (meshData != nullptr)
                {
                    ImGui::PushID(meshData);
                    ImGui::TreeNodeEx(
                        "Color", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                    ImGui::SameLine();
                    const auto& color = meshData->getColor();
                    float editableColor[] = {color[0], color[1], color[2], color[3]};
                    if (ImGui::ColorEdit4(
                            "##MeshColor", editableColor,
                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
                    {
                        const Common::Vector4f newColor(editableColor[0], editableColor[1],
                                                        editableColor[2], editableColor[3]);
                        meshData->setColor(newColor);
                        RenderWork::instance().setMeshColor(meshData->getMesh(), newColor);
                    }
                    ImGui::PopID();
                }
                for (DataTree* child : children)
                {
                    selectionChanged |= renderDataTreeNode(*child, selectedNode, meshToDelete);
                }
                ImGui::TreePop();
            }
            return selectionChanged;
        }
    } // namespace

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
            uintptr_t meshToDelete = 0;
            if (renderDataTreeNode(Context::instance().getDataTree(), _selectedNode, meshToDelete,
                                   true))
            {
                auto* meshData = dynamic_cast<MeshData*>(_selectedNode);
                RenderWork::instance().setHighlightedMesh(
                    meshData == nullptr ? Common::SharedPtr<ME::Mesh>() : meshData->getMesh());
            }
            if (meshToDelete != 0)
            {
                EventPacket packet;
                packet.setData("meshAddress", meshToDelete);
                CommandManager::instance().call(EventId::MeshDelete, packet);
            }
        }
        ImGui::End();
    }
} // namespace EgLab::Platform
