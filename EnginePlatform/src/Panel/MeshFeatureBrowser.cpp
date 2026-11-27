#include "Panel/MeshFeatureBrowser.hpp"

#include <cstdio>

#include "Command/CommandManager.hpp"
#include "Command/CommandParam.hpp"
#include "Command/IdList.hpp"
#include "DataBase/MeshData.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    void MeshFeatureBrowser::open(const Common::SharedPtr<MeshData>& data)
    {
        if (!data)
        {
            return;
        }
        if (_data != data)
        {
            close();
            _data = data;
        }

        // Rebuilding the snapshot keeps a reopened browser in sync with the current mesh.
        _topology = Common::makeShared<MeshTopology>(data->getMesh());

        _pageStack.clear();
        Page summary;
        summary.kind = PageKind::Summary;
        _pageStack.pushBack(summary);
        _hasPendingPage = false;
        refreshPageRows();
        refreshPageHighlights();
        _show = true;
    }

    void MeshFeatureBrowser::close()
    {
        if (_focusActive && _data)
        {
            EventPacket focusPacket;
            focusPacket.setData("dataNodeId", static_cast<uint64_t>(_data->getId()));
            focusPacket.setData("nodeIds", Common::String());
            focusPacket.setData("elementIds", Common::String());
            CommandManager::instance().call(EventId::HighlightMeshEntitiesFocus,
                                            Common::move(focusPacket));
        }
        _focusActive = false;

        if (_highlightActive && _data)
        {
            EventPacket packet;
            packet.setData("dataNodeId", static_cast<uint64_t>(_data->getId()));
            packet.setData("nodeIds", Common::String());
            packet.setData("elementIds", Common::String());
            CommandManager::instance().call(EventId::HighlightMeshEntities, Common::move(packet));
        }

        _highlightActive = false;
        _data = nullptr;
        _topology = nullptr;
        _show = false;
        _pageStack.clear();
        _nodeRows.clear();
        _elementRows.clear();
        _selectedNodeIds.clear();
        _selectedElementIds.clear();
        _selectionAnchor = EntityRef();
        _selectionStatus = Common::String();
        _hasPendingPage = false;
    }

    void MeshFeatureBrowser::render()
    {
        if (!_data || !_topology || _pageStack.empty())
        {
            return;
        }

        char title[384];
        std::snprintf(title, sizeof(title), "%s - %s###MeshFeatureBrowser",
                      _data->getName().c_str(), currentPageTitle().c_str());

        ImGui::SetNextWindowSize(ImVec2(540.0f, 560.0f), ImGuiCond_FirstUseEver);
        bool open = _show;
        if (!ImGui::Begin(title, &open))
        {
            ImGui::End();
            _show = open;
            if (!_show)
            {
                close();
            }
            return;
        }

        renderNavigationBar();
        ImGui::Separator();

        const Page* page = currentPage();
        if (page != nullptr)
        {
            switch (page->kind)
            {
            case PageKind::Summary:
                renderSummaryPage();
                break;
            case PageKind::NodeList:
                renderNodeListPage();
                break;
            case PageKind::ElementList:
                renderElementListPage();
                break;
            case PageKind::NodeDetail:
                renderNodeDetailPage();
                break;
            case PageKind::ElementDetail:
                renderElementDetailPage();
                break;
            }
        }

        ImGui::End();
        _show = open;
        if (!_show)
        {
            close();
            return;
        }

        // Page switches are deferred until the current frame is fully drawn so the cached rows
        // a table is iterating over stay valid.
        applyPendingPage();
    }

    void MeshFeatureBrowser::renderNavigationBar()
    {
        ImGui::BeginDisabled(_pageStack.size() <= 1);
        if (ImGui::Button("< Back"))
        {
            popPage();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextUnformatted(currentPageTitle().c_str());
    }

    void MeshFeatureBrowser::renderSummaryPage()
    {
        ImGui::TextWrapped("Double-click a row to list the nodes or elements it stands for.");
        ImGui::Separator();

        if (ImGui::BeginTable("##meshSummaryTable", 2,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID("nodes");
            if (ImGui::Selectable(
                    "Nodes", false,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                Page page;
                page.kind = PageKind::NodeList;
                requestPage(page);
            }
            ImGui::PopID();
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", _topology->getNodeCount());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID("elements");
            if (ImGui::Selectable(
                    "Elements", false,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                Page page;
                page.kind = PageKind::ElementList;
                page.elementType = -1;
                requestPage(page);
            }
            ImGui::PopID();
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", _topology->getElementCount());

            // Element types are listed as indented children of the element row.
            for (const auto& entry : _topology->getElementTypeCounts())
            {
                char label[64];
                std::snprintf(label, sizeof(label), "    %s",
                              MeshTopology::elementTypeName(entry.type));

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(static_cast<int>(entry.type));
                if (ImGui::Selectable(label, false,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowDoubleClick) &&
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    Page page;
                    page.kind = PageKind::ElementList;
                    page.elementType = entry.type;
                    requestPage(page);
                }
                ImGui::PopID();
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", entry.count);
            }

            ImGui::EndTable();
        }
    }

    void MeshFeatureBrowser::renderNodeListPage()
    {
        ImGui::Text("%zu node(s)", _nodeRows.size());
        ImGui::TextDisabled(
            "Click highlights in orange, Ctrl/Shift multi-selects, double-click lists the "
            "node's elements.");
        // Always reserve the status line so the rows do not shift between the two clicks of a
        // double-click.
        ImGui::TextDisabled("%s", _selectionStatus.size() > 0 ? _selectionStatus.c_str() : " ");
        ImGui::Separator();

        renderNodeTable(_nodeRows);
    }

    void MeshFeatureBrowser::renderElementListPage()
    {
        const Page* page = currentPage();
        const int32_t elementType = page != nullptr ? page->elementType : -1;
        if (elementType < 0)
        {
            ImGui::Text("%zu element(s)", _elementRows.size());
        }
        else
        {
            ImGui::Text("%zu %s element(s)", _elementRows.size(),
                        MeshTopology::elementTypeName(elementType));
        }
        ImGui::TextDisabled(
            "Click highlights in orange, Ctrl/Shift multi-selects, double-click lists the "
            "element's nodes.");
        ImGui::TextDisabled("%s", _selectionStatus.size() > 0 ? _selectionStatus.c_str() : " ");
        ImGui::Separator();

        renderElementTable(_elementRows);
    }

    void MeshFeatureBrowser::renderNodeDetailPage()
    {
        const Page* page = currentPage();
        if (page == nullptr)
        {
            return;
        }

        ImGui::Text("Node #%u", page->entityId);
        ImGui::Text("%zu element(s) reference this node", _elementRows.size());
        ImGui::TextDisabled(
            "The inspected node is highlighted in green; the selected element row is orange.");
        ImGui::TextDisabled("%s", _selectionStatus.size() > 0 ? _selectionStatus.c_str() : " ");
        ImGui::Separator();

        if (_elementRows.empty())
        {
            ImGui::TextDisabled("None");
            return;
        }
        renderElementTable(_elementRows);
    }

    void MeshFeatureBrowser::renderElementDetailPage()
    {
        const Page* page = currentPage();
        if (page == nullptr)
        {
            return;
        }

        ImGui::Text("Element #%u   Type: %s", page->entityId,
                    MeshTopology::elementTypeName(page->elementType));
        ImGui::Text("%zu node(s) belong to this element", _nodeRows.size());
        ImGui::TextDisabled(
            "The element is highlighted in green; the selected node row is orange.");
        ImGui::TextDisabled("%s", _selectionStatus.size() > 0 ? _selectionStatus.c_str() : " ");
        ImGui::Separator();

        if (_nodeRows.empty())
        {
            ImGui::TextDisabled("None");
            return;
        }
        renderNodeTable(_nodeRows);
    }

    void MeshFeatureBrowser::renderNodeTable(const Common::DynamicArray<MeshNodeRecord>& nodes)
    {
        if (!ImGui::BeginTable("##meshNodeTable", 4,
                               ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                   ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY,
                               ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            return;
        }

        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(nodes.size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                const MeshNodeRecord& node = nodes[static_cast<std::size_t>(row)];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                const RowResult result = renderNodeRow(node);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.6g", node.x);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.6g", node.y);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.6g", node.z);

                if (result.activated)
                {
                    handleNodeRowActivation(node, result);
                }
            }
        }

        ImGui::EndTable();
    }

    void MeshFeatureBrowser::renderElementTable(
        const Common::DynamicArray<MeshElementRecord>& elements)
    {
        if (!ImGui::BeginTable("##meshElementTable", 2,
                               ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                   ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY,
                               ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            return;
        }

        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(elements.size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                const MeshElementRecord& element = elements[static_cast<std::size_t>(row)];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                const RowResult result = renderElementRow(element);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(MeshTopology::elementTypeName(element.type));

                if (result.activated)
                {
                    handleElementRowActivation(element, result);
                }
            }
        }

        ImGui::EndTable();
    }

    MeshFeatureBrowser::RowResult MeshFeatureBrowser::renderNodeRow(const MeshNodeRecord& node)
    {
        char label[32];
        std::snprintf(label, sizeof(label), "#%u", node.id);

        RowResult result;
        ImGui::PushID(static_cast<int>(node.id));
        if (ImGui::Selectable(
                label, isNodeSelected(node.id),
                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
        {
            result.activated = true;
            result.doubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
        }
        ImGui::PopID();
        return result;
    }

    MeshFeatureBrowser::RowResult MeshFeatureBrowser::renderElementRow(
        const MeshElementRecord& element)
    {
        char label[32];
        std::snprintf(label, sizeof(label), "#%u", element.id);

        RowResult result;
        ImGui::PushID(static_cast<int>(element.id));
        if (ImGui::Selectable(
                label, isElementSelected(element.id),
                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
        {
            result.activated = true;
            result.doubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
        }
        ImGui::PopID();
        return result;
    }

    void MeshFeatureBrowser::handleNodeRowActivation(const MeshNodeRecord& node,
                                                     const RowResult& result)
    {
        if (result.doubleClicked)
        {
            // The first click already updated the selection; only make sure the double-clicked
            // node stays selected before opening its element list.
            if (!isNodeSelected(node.id))
            {
                _selectedNodeIds.clear();
                _selectedElementIds.clear();
                addSelectedNode(node.id);
                commitHighlight();
            }
            _selectionAnchor = EntityRef{true, node.id, true};

            Page page;
            page.kind = PageKind::NodeDetail;
            page.entityId = node.id;
            requestPage(page);
            return;
        }

        const ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl)
        {
            applyNodeToggleClick(node.id);
        }
        else if (io.KeyShift)
        {
            applyNodeRangeClick(node.id);
        }
        else
        {
            applyNodePlainClick(node.id);
        }
    }

    void MeshFeatureBrowser::handleElementRowActivation(const MeshElementRecord& element,
                                                        const RowResult& result)
    {
        if (result.doubleClicked)
        {
            if (!isElementSelected(element.id))
            {
                _selectedNodeIds.clear();
                _selectedElementIds.clear();
                addSelectedElement(element.id);
                commitHighlight();
            }
            _selectionAnchor = EntityRef{false, element.id, true};

            Page page;
            page.kind = PageKind::ElementDetail;
            page.entityId = element.id;
            page.elementType = element.type;
            requestPage(page);
            return;
        }

        const ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl)
        {
            applyElementToggleClick(element.id);
        }
        else if (io.KeyShift)
        {
            applyElementRangeClick(element.id);
        }
        else
        {
            applyElementPlainClick(element.id);
        }
    }

    bool MeshFeatureBrowser::isNodeSelected(uint32_t id) const
    {
        for (const uint32_t selectedId : _selectedNodeIds)
        {
            if (selectedId == id)
            {
                return true;
            }
        }
        return false;
    }

    bool MeshFeatureBrowser::isElementSelected(uint32_t id) const
    {
        for (const uint32_t selectedId : _selectedElementIds)
        {
            if (selectedId == id)
            {
                return true;
            }
        }
        return false;
    }

    bool MeshFeatureBrowser::addSelectedNode(uint32_t id)
    {
        if (isNodeSelected(id))
        {
            return true;
        }
        if (_selectedNodeIds.size() >= MAX_HIGHLIGHT_MESH_ENTITY_COUNT)
        {
            return false;
        }
        _selectedNodeIds.pushBack(id);
        return true;
    }

    bool MeshFeatureBrowser::addSelectedElement(uint32_t id)
    {
        if (isElementSelected(id))
        {
            return true;
        }
        if (_selectedElementIds.size() >= MAX_HIGHLIGHT_MESH_ENTITY_COUNT)
        {
            return false;
        }
        _selectedElementIds.pushBack(id);
        return true;
    }

    void MeshFeatureBrowser::removeSelectedNode(uint32_t id)
    {
        Common::DynamicArray<uint32_t> remaining;
        remaining.reserve(_selectedNodeIds.size());
        for (const uint32_t selectedId : _selectedNodeIds)
        {
            if (selectedId != id)
            {
                remaining.pushBack(selectedId);
            }
        }
        _selectedNodeIds = Common::move(remaining);
    }

    void MeshFeatureBrowser::removeSelectedElement(uint32_t id)
    {
        Common::DynamicArray<uint32_t> remaining;
        remaining.reserve(_selectedElementIds.size());
        for (const uint32_t selectedId : _selectedElementIds)
        {
            if (selectedId != id)
            {
                remaining.pushBack(selectedId);
            }
        }
        _selectedElementIds = Common::move(remaining);
    }

    void MeshFeatureBrowser::applyNodePlainClick(uint32_t id)
    {
        // Clicking the only selected node deselects it; any other click replaces the selection.
        const bool deselect =
            isNodeSelected(id) && _selectedNodeIds.size() == 1 && _selectedElementIds.empty();
        _selectedNodeIds.clear();
        _selectedElementIds.clear();
        if (!deselect)
        {
            addSelectedNode(id);
        }
        _selectionAnchor = EntityRef{true, id, true};
        commitHighlight();
    }

    void MeshFeatureBrowser::applyElementPlainClick(uint32_t id)
    {
        const bool deselect =
            isElementSelected(id) && _selectedElementIds.size() == 1 && _selectedNodeIds.empty();
        _selectedNodeIds.clear();
        _selectedElementIds.clear();
        if (!deselect)
        {
            addSelectedElement(id);
        }
        _selectionAnchor = EntityRef{false, id, true};
        commitHighlight();
    }

    void MeshFeatureBrowser::applyNodeToggleClick(uint32_t id)
    {
        if (isNodeSelected(id))
        {
            removeSelectedNode(id);
        }
        else
        {
            addSelectedNode(id);
        }
        _selectionAnchor = EntityRef{true, id, true};
        commitHighlight();
    }

    void MeshFeatureBrowser::applyElementToggleClick(uint32_t id)
    {
        if (isElementSelected(id))
        {
            removeSelectedElement(id);
        }
        else
        {
            addSelectedElement(id);
        }
        _selectionAnchor = EntityRef{false, id, true};
        commitHighlight();
    }

    void MeshFeatureBrowser::applyNodeRangeClick(uint32_t id)
    {
        std::size_t anchorIndex = _nodeRows.size();
        std::size_t clickedIndex = _nodeRows.size();
        for (std::size_t index = 0; index < _nodeRows.size(); ++index)
        {
            if (_nodeRows[index].id == id)
            {
                clickedIndex = index;
            }
            if (_selectionAnchor.valid && _selectionAnchor.isNode &&
                _nodeRows[index].id == _selectionAnchor.id)
            {
                anchorIndex = index;
            }
        }

        // Shift extends only inside the list that owns the anchor; otherwise it falls back to
        // a plain click so the selection still follows the user's click.
        if (anchorIndex >= _nodeRows.size() || clickedIndex >= _nodeRows.size())
        {
            applyNodePlainClick(id);
            return;
        }

        const std::size_t first = anchorIndex < clickedIndex ? anchorIndex : clickedIndex;
        const std::size_t last = anchorIndex < clickedIndex ? clickedIndex : anchorIndex;
        _selectedNodeIds.clear();
        _selectedElementIds.clear();
        for (std::size_t index = first; index <= last; ++index)
        {
            if (!addSelectedNode(_nodeRows[index].id))
            {
                break;
            }
        }
        commitHighlight();
    }

    void MeshFeatureBrowser::applyElementRangeClick(uint32_t id)
    {
        std::size_t anchorIndex = _elementRows.size();
        std::size_t clickedIndex = _elementRows.size();
        for (std::size_t index = 0; index < _elementRows.size(); ++index)
        {
            if (_elementRows[index].id == id)
            {
                clickedIndex = index;
            }
            if (_selectionAnchor.valid && !_selectionAnchor.isNode &&
                _elementRows[index].id == _selectionAnchor.id)
            {
                anchorIndex = index;
            }
        }

        if (anchorIndex >= _elementRows.size() || clickedIndex >= _elementRows.size())
        {
            applyElementPlainClick(id);
            return;
        }

        const std::size_t first = anchorIndex < clickedIndex ? anchorIndex : clickedIndex;
        const std::size_t last = anchorIndex < clickedIndex ? clickedIndex : anchorIndex;
        _selectedNodeIds.clear();
        _selectedElementIds.clear();
        for (std::size_t index = first; index <= last; ++index)
        {
            if (!addSelectedElement(_elementRows[index].id))
            {
                break;
            }
        }
        commitHighlight();
    }

    void MeshFeatureBrowser::commitHighlight()
    {
        if (!_data)
        {
            return;
        }

        const Common::String nodeIds =
            formatIdList(_selectedNodeIds.data(), _selectedNodeIds.size());
        const Common::String elementIds =
            formatIdList(_selectedElementIds.data(), _selectedElementIds.size());

        char status[160];
        std::snprintf(status, sizeof(status), "%zu node(s) | %zu element(s) selected",
                      _selectedNodeIds.size(), _selectedElementIds.size());
        _selectionStatus = Common::String(status);

        EventPacket packet;
        packet.setData("dataNodeId", static_cast<uint64_t>(_data->getId()));
        packet.setData("nodeIds", nodeIds);
        packet.setData("elementIds", elementIds);
        CommandManager::instance().call(EventId::HighlightMeshEntities, Common::move(packet));

        _highlightActive = !(_selectedNodeIds.empty() && _selectedElementIds.empty());
    }

    void MeshFeatureBrowser::commitFocusHighlight()
    {
        if (!_data)
        {
            return;
        }

        // The context of a page is the entity it was opened for: a node page highlights that
        // node, an element page highlights that element. Neighbouring entities stay unlit until
        // the user selects their row, so only the row selection ever highlights them.
        Common::DynamicArray<uint32_t> focusNodeIds;
        Common::DynamicArray<uint32_t> focusElementIds;
        const Page* page = currentPage();
        if (page != nullptr && page->kind == PageKind::NodeDetail)
        {
            focusNodeIds.pushBack(page->entityId);
        }
        else if (page != nullptr && page->kind == PageKind::ElementDetail)
        {
            focusElementIds.pushBack(page->entityId);
        }

        const bool hasFocus = !focusNodeIds.empty() || !focusElementIds.empty();
        if (!hasFocus && !_focusActive)
        {
            return;
        }
        _focusActive = hasFocus;

        EventPacket packet;
        packet.setData("dataNodeId", static_cast<uint64_t>(_data->getId()));
        packet.setData("nodeIds", formatIdList(focusNodeIds.data(), focusNodeIds.size()));
        packet.setData("elementIds", formatIdList(focusElementIds.data(), focusElementIds.size()));
        CommandManager::instance().call(EventId::HighlightMeshEntitiesFocus, Common::move(packet));
    }

    void MeshFeatureBrowser::refreshPageHighlights()
    {
        commitFocusHighlight();

        // Re-send the selection after the page context so the selected entity is added to the
        // scene last and keeps its selection colour on top of the green context overlay.
        if (!_selectedNodeIds.empty() || !_selectedElementIds.empty())
        {
            commitHighlight();
        }
    }

    void MeshFeatureBrowser::clearSelection()
    {
        _selectionAnchor = EntityRef();
        _selectionStatus = Common::String();
        if (_selectedNodeIds.empty() && _selectedElementIds.empty())
        {
            return;
        }
        _selectedNodeIds.clear();
        _selectedElementIds.clear();
        commitHighlight();
    }

    void MeshFeatureBrowser::requestPage(const Page& page)
    {
        _pendingPage = page;
        _hasPendingPage = true;
    }

    void MeshFeatureBrowser::applyPendingPage()
    {
        if (!_hasPendingPage)
        {
            return;
        }
        _hasPendingPage = false;
        pushPage(_pendingPage);
    }

    void MeshFeatureBrowser::pushPage(const Page& page)
    {
        _pageStack.pushBack(page);
        refreshPageRows();
        refreshPageHighlights();
    }

    void MeshFeatureBrowser::popPage()
    {
        if (_pageStack.size() <= 1)
        {
            return;
        }
        _pageStack.popBack();
        refreshPageRows();

        // Going back leaves the entities selected on the child page behind. Clearing the
        // selection here removes their highlight instead of keeping it on a page that does not
        // list them, and restores the parent page's own context highlight.
        clearSelection();
        commitFocusHighlight();
    }

    const MeshFeatureBrowser::Page* MeshFeatureBrowser::currentPage() const
    {
        if (_pageStack.empty())
        {
            return nullptr;
        }
        return &_pageStack.back();
    }

    Common::String MeshFeatureBrowser::currentPageTitle() const
    {
        const Page* page = currentPage();
        if (page == nullptr)
        {
            return Common::String();
        }

        char title[128];
        switch (page->kind)
        {
        case PageKind::Summary:
            return Common::String("Topology");
        case PageKind::NodeList:
            return Common::String("Nodes");
        case PageKind::ElementList:
            if (page->elementType < 0)
            {
                return Common::String("Elements");
            }
            return Common::String(MeshTopology::elementTypeName(page->elementType));
        case PageKind::NodeDetail:
            std::snprintf(title, sizeof(title), "Node #%u", page->entityId);
            return Common::String(title);
        case PageKind::ElementDetail:
            std::snprintf(title, sizeof(title), "Element #%u", page->entityId);
            return Common::String(title);
        }
        return Common::String();
    }

    void MeshFeatureBrowser::refreshPageRows()
    {
        _nodeRows.clear();
        _elementRows.clear();
        if (!_topology)
        {
            return;
        }

        const Page* page = currentPage();
        if (page == nullptr)
        {
            return;
        }

        switch (page->kind)
        {
        case PageKind::Summary:
            break;
        case PageKind::NodeList:
            _topology->collectNodes(_nodeRows);
            break;
        case PageKind::ElementList:
            _topology->collectElements(page->elementType, _elementRows);
            break;
        case PageKind::NodeDetail:
            _topology->collectNodeElements(page->entityId, _elementRows);
            break;
        case PageKind::ElementDetail:
            _topology->collectElementNodes(page->entityId, _nodeRows);
            break;
        }
    }
} // namespace EgLab::Platform
