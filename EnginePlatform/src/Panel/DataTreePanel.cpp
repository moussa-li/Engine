#include "Panel/DataTreePanel.hpp"

#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
#include "DataBase/MeshData.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        bool renderDataTreeNode(DataTree& node, DataTree*& selectedNode,
                                bool defaultOpen = false)
        {
            const auto children = node.getChildrens();
            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (&node == selectedNode)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (children.empty())
            {
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }
            if (defaultOpen)
            {
                flags |= ImGuiTreeNodeFlags_DefaultOpen;
            }

            const bool isOpen =
                ImGui::TreeNodeEx(&node, flags, "%s", node.getName().c_str());
            const bool clicked = ImGui::IsItemClicked();
            bool selectionChanged = false;
            if (clicked)
            {
                selectedNode = dynamic_cast<MeshData*>(&node) != nullptr ? &node : nullptr;
                selectionChanged = true;
            }

            if (isOpen && !children.empty())
            {
                for (DataTree* child : children)
                {
                    selectionChanged |= renderDataTreeNode(*child, selectedNode);
                }
                ImGui::TreePop();
            }
            return selectionChanged;
        }
    } // namespace

    DataTreePanel::DataTreePanel()
    {}
    
    DataTreePanel::~DataTreePanel(){}

    void DataTreePanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Data Tree", &_show))
        {
            if (renderDataTreeNode(Context::instance().getDataTree(), _selectedNode, true))
            {
                auto* meshData = dynamic_cast<MeshData*>(_selectedNode);
                RenderWork::instance().setHighlightedMesh(
                    meshData == nullptr ? Common::SharedPtr<ME::Mesh>() : meshData->getMesh());
            }
        }
        ImGui::End();
    }
} // namespace EgLab::Platform
