#pragma once

#include <cstddef>
#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/GeometryData.hpp"

namespace EgLab::Platform
{
    /// Feature browser opened by double-clicking a GeometryData node in the data tree.
    ///
    /// A single window hosts a stack of pages (topology summary -> feature list -> feature
    /// detail). Double-clicking a row pushes the next page and the Back button pops it back to
    /// the previous page. Highlights are dispatched through CommandManager, so the browser never
    /// touches the render layer directly.
    class GeometryFeatureBrowser
    {
    public:
        GeometryFeatureBrowser() = default;
        ~GeometryFeatureBrowser() = default;

        /// Shows the summary page for data, replacing any previously browsed model.
        void open(const Common::SharedPtr<GeometryData>& data);

        /// Closes the window and clears the feature highlights.
        void close();

        /// True while a model is being browsed.
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
            FeatureList,
            Detail,
        };

        struct Page
        {
            PageKind kind{PageKind::Summary};
            uint32_t featureType{0};
            GeometryFeatureRef ref;
        };

        /// Activation reported by one feature row.
        struct FeatureRowResult
        {
            bool activated{false};
            bool doubleClicked{false};
        };

        void renderNavigationBar();
        void renderSummaryPage();
        void renderFeatureListPage();
        void renderDetailPage(const GeometryFeatureRef& ref);

        /// Draws one selectable feature row and reports how it was activated.
        FeatureRowResult renderFeatureRow(const GeometryFeatureRef& ref);

        /// Draws a clipped feature list and applies the click / modifier selection semantics.
        void renderFeatureRefList(const Common::DynamicArray<GeometryFeatureRef>& refs);

        bool isFeatureSelected(const GeometryFeatureRef& ref) const;
        bool tryAddSelectedFeature(const GeometryFeatureRef& ref);
        void applyPlainClick(const GeometryFeatureRef& ref);
        void applyToggleClick(const GeometryFeatureRef& ref);
        void applyRangeClick(const Common::DynamicArray<GeometryFeatureRef>& refs,
                             const GeometryFeatureRef& ref);
        void removeSelectedFeature(const GeometryFeatureRef& ref);

        /// Sends the whole selection to RenderWork through CommandManager.
        void commitSelectionHighlight();
        /// Highlights the feature of the top detail page with the focus colour, or clears it.
        void commitFocusHighlight();

        void pushFeatureListPage(uint32_t featureType);
        void pushDetailPage(const GeometryFeatureRef& ref);
        void popPage();
        const Page* currentPage() const;
        Common::String currentPageTitle() const;

        void clearSelection();

        static const char* featureTypeName(uint32_t featureType);

    private:
        Common::SharedPtr<GeometryData> _data;
        bool _show{false};
        Common::DynamicArray<Page> _pageStack;
        Common::DynamicArray<GeometryFeatureRef> _selectedFeatures;
        GeometryFeatureRef _selectionAnchor;
        GeometryFeatureRef _focusFeature;
        bool _focusActive{false};
        Common::String _selectionStatus;
    };
} // namespace EgLab::Platform
