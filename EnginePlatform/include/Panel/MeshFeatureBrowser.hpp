#pragma once

#include <cstddef>
#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/MeshTopology.hpp"

namespace EgLab::Platform
{
    class MeshData;

    /// Mesh browser opened by double-clicking a MeshData node in the data tree.
    ///
    /// The summary page lists the node and element counts together with the per-element-type
    /// counts. Double-clicking a row pushes the matching list page; from there double-clicking
    /// a node lists its elements and double-clicking an element lists its nodes. Clicking a row
    /// selects and highlights the corresponding entity in the viewport through CommandManager,
    /// in a different colour than the entity the current page was opened for (drawn green
    /// while selected rows are orange).
    class MeshFeatureBrowser
    {
    public:
        MeshFeatureBrowser() = default;
        ~MeshFeatureBrowser() = default;

        /// Shows the summary page for data, replacing any previously browsed mesh.
        void open(const Common::SharedPtr<MeshData>& data);

        /// Closes the window and clears the mesh highlight.
        void close();

        /// True while a mesh is being browsed.
        bool isOpen() const
        {
            return _data != nullptr;
        }

        /// Draws the browser window; call once per frame after the data tree window ended.
        void render();

    private:
        enum class PageKind
        {
            Summary,
            NodeList,
            ElementList,
            NodeDetail,
            ElementDetail,
        };

        struct Page
        {
            PageKind kind{PageKind::Summary};
            int32_t elementType{-1}; // ElementList filter, negative lists every type
            uint32_t entityId{0};    // node id for NodeDetail, element id for ElementDetail
        };

        /// Identifies the row that started a shift range selection.
        struct EntityRef
        {
            bool isNode{true};
            uint32_t id{0};
            bool valid{false};
        };

        struct RowResult
        {
            bool activated{false};
            bool doubleClicked{false};
        };

        void renderNavigationBar();
        void renderSummaryPage();
        void renderNodeListPage();
        void renderElementListPage();
        void renderNodeDetailPage();
        void renderElementDetailPage();

        /// Draws a clipped node table (id / x / y / z) and applies the click semantics.
        void renderNodeTable(const Common::DynamicArray<MeshNodeRecord>& nodes);
        /// Draws a clipped element table (id / type) and applies the click semantics.
        void renderElementTable(const Common::DynamicArray<MeshElementRecord>& elements);

        RowResult renderNodeRow(const MeshNodeRecord& node);
        RowResult renderElementRow(const MeshElementRecord& element);
        void handleNodeRowActivation(const MeshNodeRecord& node, const RowResult& result);
        void handleElementRowActivation(const MeshElementRecord& element, const RowResult& result);

        bool isNodeSelected(uint32_t id) const;
        bool isElementSelected(uint32_t id) const;
        bool addSelectedNode(uint32_t id);
        bool addSelectedElement(uint32_t id);
        void removeSelectedNode(uint32_t id);
        void removeSelectedElement(uint32_t id);
        void applyNodePlainClick(uint32_t id);
        void applyElementPlainClick(uint32_t id);
        void applyNodeToggleClick(uint32_t id);
        void applyElementToggleClick(uint32_t id);
        void applyNodeRangeClick(uint32_t id);
        void applyElementRangeClick(uint32_t id);

        /// Sends the whole selection to RenderWork through CommandManager.
        void commitHighlight();
        /// Highlights the entity the current page was opened for with the context colour, or
        /// clears that highlight when the page has no single context entity.
        void commitFocusHighlight();
        /// Refreshes the page context highlight and re-sends the selection so it stays on top.
        void refreshPageHighlights();
        void clearSelection();

        void requestPage(const Page& page);
        void applyPendingPage();
        void pushPage(const Page& page);
        void popPage();
        const Page* currentPage() const;
        Common::String currentPageTitle() const;
        /// Reloads the cached rows of the top page from the topology snapshot.
        void refreshPageRows();

    private:
        Common::SharedPtr<MeshData> _data;
        Common::SharedPtr<MeshTopology> _topology;
        bool _show{false};
        Common::DynamicArray<Page> _pageStack;
        Common::DynamicArray<MeshNodeRecord> _nodeRows;
        Common::DynamicArray<MeshElementRecord> _elementRows;
        Common::DynamicArray<uint32_t> _selectedNodeIds;
        Common::DynamicArray<uint32_t> _selectedElementIds;
        EntityRef _selectionAnchor;
        Common::String _selectionStatus;
        bool _highlightActive{false};
        bool _focusActive{false};

        Page _pendingPage;
        bool _hasPendingPage{false};
    };
} // namespace EgLab::Platform
