#include "Panel/DataTreePanel.hpp"

#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        void renderDataTreeNode(const DataTree& node, bool defaultOpen = false)
        {
            const auto children = node.getChildrens();
            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
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
            if (isOpen && !children.empty())
            {
                for (const DataTree* child : children)
                {
                    renderDataTreeNode(*child);
                }
                ImGui::TreePop();
            }
        }
    } // namespace

    void DataTreePanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Data Tree", &_show))
        {
            renderDataTreeNode(Context::instance().getDataTree(), true);
        }
        ImGui::End();
    }
} // namespace EgLab::Platform
