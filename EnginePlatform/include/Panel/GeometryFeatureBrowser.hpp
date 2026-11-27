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
    /// The windows form a hierarchy: a topology summary table, the feature list of one type,
    /// and one parent/child window per inspected feature. Highlights are dispatched through
    /// CommandManager, so the browser never touches the render layer directly.
    class GeometryFeatureBrowser
    {
    public:
        GeometryFeatureBrowser() = default;
        ~GeometryFeatureBrowser() = default;

        /// Shows the summary window for data, replacing any previously browsed model.
        void open(const Common::SharedPtr<GeometryData>& data);

        /// Closes every window and clears the feature highlight.
        void close();

        /// True while a model is being browsed.
        bool isOpen() const
        {
            return _data != nullptr;
        }

        /// Draws every open window; call once per frame after the data tree window ended.
        void render();

    private:
        struct DetailWindow
        {
            GeometryFeatureRef ref;
            bool open{true};
        };

        /// Activation reported by one feature row.
        struct FeatureRowResult
        {
            bool activated{false};
            bool doubleClicked{false};
        };

        void renderSummaryWindow();
        void renderFeatureListWindow();
        void renderDetailWindow(const GeometryFeatureRef& ref, bool& open, std::size_t index);

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

        void openFeatureList(uint32_t featureType);
        void openDetailWindow(const GeometryFeatureRef& ref);
        void clearSelection();

        static const char* featureTypeName(uint32_t featureType);

    private:
        Common::SharedPtr<GeometryData> _data;
        bool _showSummary{false};
        bool _showFeatureList{false};
        uint32_t _featureListType{0};
        Common::DynamicArray<DetailWindow> _detailWindows;
        Common::DynamicArray<GeometryFeatureRef> _selectedFeatures;
        GeometryFeatureRef _selectionAnchor;
        Common::String _selectionStatus;
    };
} // namespace EgLab::Platform
