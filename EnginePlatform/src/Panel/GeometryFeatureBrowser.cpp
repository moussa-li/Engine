#include "Panel/GeometryFeatureBrowser.hpp"

#include <cstdio>

#include "Command/CommandManager.hpp"
#include "Command/CommandParam.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        // Summary rows are ordered from the highest dimension to the lowest, matching the
        // solid -> face -> edge -> vertex hierarchy shown in the detail windows.
        constexpr uint32_t FEATURE_TYPE_ORDER[] = {
            static_cast<uint32_t>(GE::TopologyType::Solid),
            static_cast<uint32_t>(GE::TopologyType::Face),
            static_cast<uint32_t>(GE::TopologyType::Edge),
            static_cast<uint32_t>(GE::TopologyType::Vertex),
        };
    } // namespace

    void GeometryFeatureBrowser::open(const Common::SharedPtr<GeometryData>& data)
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
        _showSummary = true;
    }

    void GeometryFeatureBrowser::close()
    {
        clearSelection();
        _data = nullptr;
        _showSummary = false;
        _showFeatureList = false;
        _detailWindows.clear();
    }

    void GeometryFeatureBrowser::render()
    {
        if (!_data)
        {
            return;
        }

        if (_showSummary)
        {
            renderSummaryWindow();
        }
        if (_showFeatureList)
        {
            renderFeatureListWindow();
        }

        // Detail windows may open further detail windows, so iterate by index and re-read the
        // entry every step instead of holding a reference across the draw call.
        for (std::size_t index = 0; index < _detailWindows.size(); ++index)
        {
            const GeometryFeatureRef ref = _detailWindows[index].ref;
            bool open = _detailWindows[index].open;
            renderDetailWindow(ref, open, index);
            _detailWindows[index].open = open;
        }

        Common::DynamicArray<DetailWindow> remainingWindows;
        for (const auto& window : _detailWindows)
        {
            if (window.open)
            {
                remainingWindows.pushBack(window);
            }
        }
        _detailWindows = Common::move(remainingWindows);

        if (!_showSummary && !_showFeatureList && _detailWindows.empty())
        {
            close();
        }
    }

    void GeometryFeatureBrowser::renderSummaryWindow()
    {
        char title[192];
        std::snprintf(title, sizeof(title), "Geometry Features - %s###GeometryFeatureSummary",
                      _data->getName().c_str());

        ImGui::SetNextWindowSize(ImVec2(320.0f, 210.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, &_showSummary))
        {
            ImGui::End();
            return;
        }

        ImGui::TextWrapped("Double-click a row to list every feature of that type.");
        ImGui::Separator();

        if (ImGui::BeginTable("##topologyTable", 2,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            for (const uint32_t featureType : FEATURE_TYPE_ORDER)
            {
                const std::size_t count =
                    _data->getFeatureCount(static_cast<GE::TopologyType>(featureType));

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(static_cast<int>(featureType));
                if (ImGui::Selectable(featureTypeName(featureType), false,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        openFeatureList(featureType);
                    }
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%zu", count);
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }

    void GeometryFeatureBrowser::renderFeatureListWindow()
    {
        char title[128];
        std::snprintf(title, sizeof(title), "%s###GeometryFeatureList",
                      featureTypeName(_featureListType));

        ImGui::SetNextWindowSize(ImVec2(380.0f, 400.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, &_showFeatureList))
        {
            ImGui::End();
            return;
        }

        const auto type = static_cast<GE::TopologyType>(_featureListType);
        const std::size_t count = _data->getFeatureCount(type);
        ImGui::Text("%zu %s feature(s)", count, featureTypeName(_featureListType));
        ImGui::TextDisabled(
            "Click highlights, Ctrl/Shift multi-selects, double-click inspects parents/children.");
        if (!_selectedFeatures.empty() && _selectionStatus.size() > 0)
        {
            ImGui::TextDisabled("%s", _selectionStatus.c_str());
        }
        ImGui::Separator();

        ImGui::BeginChild("##featureList");
        Common::DynamicArray<GeometryFeatureRef> refs;
        refs.reserve(count);
        for (std::size_t index = 0; index < count; ++index)
        {
            refs.pushBack(GeometryFeatureRef(type, static_cast<GE::IdxType>(index)));
        }
        renderFeatureRefList(refs);
        ImGui::EndChild();

        ImGui::End();
    }

    void GeometryFeatureBrowser::renderDetailWindow(const GeometryFeatureRef& ref, bool& open,
                                                    std::size_t index)
    {
        const GeometryFeatureInfo info = _data->getFeatureInfo(ref);

        // The visible title carries the geometry name plus the feature name and id; the part
        // after ### keeps the ImGui id stable and unique per feature.
        char title[320];
        std::snprintf(title, sizeof(title), "%s / %s (id %u)###GeometryFeatureDetail_%u_%u",
                      _data->getName().c_str(), info.name.c_str(), info.featureId,
                      static_cast<uint32_t>(ref.featureType),
                      static_cast<uint32_t>(ref.featureIndex));

        const float offset = 24.0f * static_cast<float>(index + 1);
        ImGui::SetNextWindowPos(ImVec2(160.0f + offset, 120.0f + offset), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340.0f, 360.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, &open))
        {
            ImGui::End();
            return;
        }

        ImGui::Text("Feature: %s", info.name.c_str());
        ImGui::Text("Id: %u   Type: %s", info.featureId, featureTypeName(ref.featureType));
        ImGui::TextDisabled(
            "Click highlights (Ctrl/Shift multi-select), double-click opens that feature.");
        if (!_selectedFeatures.empty() && _selectionStatus.size() > 0)
        {
            ImGui::TextDisabled("%s", _selectionStatus.c_str());
        }
        ImGui::Separator();

        Common::DynamicArray<GeometryFeatureRef> parents;
        Common::DynamicArray<GeometryFeatureRef> children;
        _data->getFeatureRelations(ref, parents, children);

        const float listHeight =
            (ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing()) * 0.5f;

        ImGui::TextUnformatted("Parents");
        ImGui::BeginChild("##parents", ImVec2(0.0f, listHeight), true);
        if (parents.empty())
        {
            ImGui::TextDisabled("None");
        }
        else
        {
            renderFeatureRefList(parents);
        }
        ImGui::EndChild();

        ImGui::TextUnformatted("Children");
        ImGui::BeginChild("##children", ImVec2(0.0f, 0.0f), true);
        if (children.empty())
        {
            ImGui::TextDisabled("None");
        }
        else
        {
            renderFeatureRefList(children);
        }
        ImGui::EndChild();

        ImGui::End();
    }

    GeometryFeatureBrowser::FeatureRowResult GeometryFeatureBrowser::renderFeatureRow(
        const GeometryFeatureRef& ref)
    {
        const GeometryFeatureInfo info = _data->getFeatureInfo(ref);
        char label[256];
        std::snprintf(label, sizeof(label), "#%u  %s", info.featureId, info.name.c_str());

        FeatureRowResult result;
        ImGui::PushID(static_cast<int>(ref.featureType));
        ImGui::PushID(static_cast<int>(ref.featureIndex));
        if (ImGui::Selectable(label, isFeatureSelected(ref),
                              ImGuiSelectableFlags_AllowDoubleClick))
        {
            result.activated = true;
            result.doubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
        }
        ImGui::PopID();
        ImGui::PopID();
        return result;
    }

    void GeometryFeatureBrowser::renderFeatureRefList(
        const Common::DynamicArray<GeometryFeatureRef>& refs)
    {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(refs.size()));
        while (clipper.Step())
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
            {
                const GeometryFeatureRef& ref = refs[static_cast<std::size_t>(index)];
                const FeatureRowResult result = renderFeatureRow(ref);
                if (!result.activated)
                {
                    continue;
                }

                if (result.doubleClicked)
                {
                    // The first click already updated the selection; only guarantee that the
                    // double-clicked feature stays selected before opening its window.
                    if (!isFeatureSelected(ref))
                    {
                        _selectedFeatures.clear();
                        _selectedFeatures.pushBack(ref);
                        commitSelectionHighlight();
                    }
                    _selectionAnchor = ref;
                    openDetailWindow(ref);
                    continue;
                }

                const ImGuiIO& io = ImGui::GetIO();
                if (io.KeyCtrl)
                {
                    applyToggleClick(ref);
                }
                else if (io.KeyShift)
                {
                    applyRangeClick(refs, ref);
                }
                else
                {
                    applyPlainClick(ref);
                }
            }
        }
    }

    bool GeometryFeatureBrowser::isFeatureSelected(const GeometryFeatureRef& ref) const
    {
        for (const auto& selected : _selectedFeatures)
        {
            if (selected == ref)
            {
                return true;
            }
        }
        return false;
    }

    bool GeometryFeatureBrowser::tryAddSelectedFeature(const GeometryFeatureRef& ref)
    {
        // The highlight command carries at most MAX_HIGHLIGHT_GEOMETRY_FEATURE_COUNT features.
        if (_selectedFeatures.size() >= MAX_HIGHLIGHT_GEOMETRY_FEATURE_COUNT)
        {
            return false;
        }
        _selectedFeatures.pushBack(ref);
        return true;
    }

    void GeometryFeatureBrowser::removeSelectedFeature(const GeometryFeatureRef& ref)
    {
        Common::DynamicArray<GeometryFeatureRef> remaining;
        remaining.reserve(_selectedFeatures.size());
        for (const auto& selected : _selectedFeatures)
        {
            if (selected != ref)
            {
                remaining.pushBack(selected);
            }
        }
        _selectedFeatures = Common::move(remaining);
    }

    void GeometryFeatureBrowser::applyPlainClick(const GeometryFeatureRef& ref)
    {
        // Clicking a selected row toggles it off; clicking an unselected row replaces the
        // selection with that single feature.
        if (isFeatureSelected(ref))
        {
            removeSelectedFeature(ref);
        }
        else
        {
            _selectedFeatures.clear();
            tryAddSelectedFeature(ref);
        }
        _selectionAnchor = ref;
        commitSelectionHighlight();
    }

    void GeometryFeatureBrowser::applyToggleClick(const GeometryFeatureRef& ref)
    {
        if (isFeatureSelected(ref))
        {
            removeSelectedFeature(ref);
        }
        else
        {
            tryAddSelectedFeature(ref);
        }
        _selectionAnchor = ref;
        commitSelectionHighlight();
    }

    void GeometryFeatureBrowser::applyRangeClick(
        const Common::DynamicArray<GeometryFeatureRef>& refs, const GeometryFeatureRef& ref)
    {
        std::size_t anchorIndex = refs.size();
        std::size_t clickedIndex = refs.size();
        for (std::size_t index = 0; index < refs.size(); ++index)
        {
            if (refs[index] == _selectionAnchor)
            {
                anchorIndex = index;
            }
            if (refs[index] == ref)
            {
                clickedIndex = index;
            }
        }

        // Shift extends only inside the list that owns the anchor; otherwise it falls back to
        // a plain click so the selection still follows the user's click.
        if (anchorIndex >= refs.size() || clickedIndex >= refs.size())
        {
            applyPlainClick(ref);
            return;
        }

        const std::size_t first = anchorIndex < clickedIndex ? anchorIndex : clickedIndex;
        const std::size_t last = anchorIndex < clickedIndex ? clickedIndex : anchorIndex;
        _selectedFeatures.clear();
        for (std::size_t index = first; index <= last; ++index)
        {
            if (!tryAddSelectedFeature(refs[index]))
            {
                break;
            }
        }
        // The anchor stays put so repeated shift-clicks extend or shrink the same range.
        commitSelectionHighlight();
    }

    void GeometryFeatureBrowser::commitSelectionHighlight()
    {
        if (!_data)
        {
            return;
        }

        // "type:index,type:index" keeps the command payload small and readable while the
        // render side merges the highlighted ids of every selected feature.
        Common::String features;
        char item[64];
        for (const auto& ref : _selectedFeatures)
        {
            std::snprintf(item, sizeof(item), "%u:%u", ref.featureType, ref.featureIndex);
            if (features.size() > 0)
            {
                features.append(",");
            }
            features.append(item);
        }

        std::size_t triangleCount = 0;
        std::size_t segmentCount = 0;
        std::size_t pointCount = 0;
        for (const auto& ref : _selectedFeatures)
        {
            GeometryFeatureRenderIds ids;
            _data->collectFeatureRenderIds(ref, ids);
            triangleCount += ids.elementIds.size();
            segmentCount += ids.edgeSegmentIndices.size();
            pointCount += ids.nodeIndices.size();
        }

        char status[160];
        std::snprintf(status, sizeof(status), "%zu selected | %zu tris | %zu segments | %zu points",
                      _selectedFeatures.size(), triangleCount, segmentCount, pointCount);
        _selectionStatus = Common::String(status);

        EventPacket packet;
        packet.setData("dataNodeId", static_cast<uint64_t>(_data->getId()));
        packet.setData("features", features);
        CommandManager::instance().call(EventId::HighlightGeometryFeature, Common::move(packet));
    }

    void GeometryFeatureBrowser::openFeatureList(uint32_t featureType)
    {
        _featureListType = featureType;
        _showFeatureList = true;
    }

    void GeometryFeatureBrowser::openDetailWindow(const GeometryFeatureRef& ref)
    {
        for (auto& window : _detailWindows)
        {
            if (window.ref == ref)
            {
                window.open = true;
                return;
            }
        }

        DetailWindow window;
        window.ref = ref;
        window.open = true;
        _detailWindows.pushBack(window);
    }

    void GeometryFeatureBrowser::clearSelection()
    {
        _selectionAnchor = GeometryFeatureRef();
        _selectionStatus = Common::String();
        if (_selectedFeatures.empty())
        {
            return;
        }
        _selectedFeatures.clear();
        commitSelectionHighlight();
    }

    const char* GeometryFeatureBrowser::featureTypeName(uint32_t featureType)
    {
        switch (featureType)
        {
        case static_cast<uint32_t>(GE::TopologyType::Vertex):
            return "Vertex";
        case static_cast<uint32_t>(GE::TopologyType::Edge):
            return "Edge";
        case static_cast<uint32_t>(GE::TopologyType::Face):
            return "Face";
        case static_cast<uint32_t>(GE::TopologyType::Solid):
            return "Solid";
        default:
            return "Unknown";
        }
    }
} // namespace EgLab::Platform
