#include "Panel/GeometryInfoPanel.hpp"

#include <sstream>

#include "Command/CommandManager.hpp"
#include "Common/Log.hpp"
#include "Core/IniManager.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        constexpr const char* GEOMETRY_PICK_FILTER_KEY = "geometry.pickFilter";
    } // namespace

    GeometryInfoPanel::GeometryInfoPanel()
    {
        const int savedMask = IniManager::instance().getInt(
            GEOMETRY_PICK_FILTER_KEY, static_cast<int>(GEOMETRY_FEATURE_MASK_ALL));
        _pickFilter = static_cast<uint32_t>(savedMask) & GEOMETRY_FEATURE_MASK_ALL;
    }

    GeometryInfoPanel::~GeometryInfoPanel()
    {
        releasePickRequest();
        if (_pickInfoSubscriptionId != 0)
        {
            RenderWork::instance().unsubscribePickInfo(_pickInfoSubscriptionId);
            _pickInfoSubscriptionId = 0;
        }
    }

    Common::String GeometryInfoPanel::featureTypeName(uint32_t type)
    {
        switch (type)
        {
        case 0:
            return Common::String("Vertex");
        case 1:
            return Common::String("Edge");
        case 2:
            return Common::String("Face");
        case 3:
            return Common::String("Solid");
        default:
            return Common::String("Unknown");
        }
    }

    Common::String GeometryInfoPanel::geometryTypeName(uint32_t type)
    {
        switch (type)
        {
        case 0:
            return Common::String("Unknown");
        case 1:
            return Common::String("Point");
        case 2:
            return Common::String("Curve");
        case 3:
            return Common::String("Surface");
        case 4:
            return Common::String("Solid");
        default:
            return Common::String("Unknown");
        }
    }

    void GeometryInfoPanel::syncPickRequest()
    {
        RE::PickManager& picks = RenderWork::instance().pickManager();
        const bool wantsPicking = isShow() && _showHoverInfo;

        // Geometry features live on three primitives: vertices on the node layer, B-Rep edges
        // on the line layer and faces on the face layer, so all three kinds are requested.
        syncPickToken(_nodePickToken, wantsPicking, RE::PickKind::Node);
        syncPickToken(_edgePickToken, wantsPicking, RE::PickKind::Edge);
        syncPickToken(_facePickToken, wantsPicking, RE::PickKind::Face);
    }

    void GeometryInfoPanel::syncPickToken(RE::PickManager::PickToken& token, bool wanted,
                                          RE::PickKind kind)
    {
        RE::PickManager& picks = RenderWork::instance().pickManager();
        if (wanted && token == RE::PickManager::kInvalidToken)
        {
            token = picks.acquire(kind, Common::String("GeometryInfoPanel"));
        }
        else if (!wanted && token != RE::PickManager::kInvalidToken)
        {
            picks.release(token);
            token = RE::PickManager::kInvalidToken;
        }
    }

    void GeometryInfoPanel::releasePickRequest()
    {
        RE::PickManager& picks = RenderWork::instance().pickManager();
        if (_nodePickToken != RE::PickManager::kInvalidToken)
        {
            picks.release(_nodePickToken);
            _nodePickToken = RE::PickManager::kInvalidToken;
        }
        if (_edgePickToken != RE::PickManager::kInvalidToken)
        {
            picks.release(_edgePickToken);
            _edgePickToken = RE::PickManager::kInvalidToken;
        }
        if (_facePickToken != RE::PickManager::kInvalidToken)
        {
            picks.release(_facePickToken);
            _facePickToken = RE::PickManager::kInvalidToken;
        }
    }

    void GeometryInfoPanel::updatePickedInfo()
    {
        if (_pickInfoSubscriptionId == 0)
        {
            return;
        }

        // Picking only runs while the cursor moves (or the requested kinds change), so the
        // panel keeps the last result and only replaces it when a new one arrives. Clearing
        // the state on every frame made the info vanish as soon as the mouse stopped.
        PickedMeshInfo pickedInfo;
        while (RenderWork::instance().tryReceivePickInfo(_pickInfoSubscriptionId, pickedInfo))
        {
            _pickedInfo = pickedInfo;
            _hasPickedInfo = false;
            _hoverStatus = Common::String();

            // A cleared result arrives when the cursor leaves every pickable primitive.
            // Geometry features live on all three layers: vertex nodes, B-Rep edge segments
            // and faces.
            if (pickedInfo.kind != PickedMeshKind::ElementFace &&
                pickedInfo.kind != PickedMeshKind::ElementEdge &&
                pickedInfo.kind != PickedMeshKind::Node)
            {
                continue;
            }

            auto geometryData = GeometryData::findByMesh(pickedInfo.meshId);
            if (!geometryData)
            {
                _hoverStatus = Common::String("picked mesh is not a geometry model");
                continue;
            }

            // Resolve the same feature the pick selector reported, so a hit on a
            // filtered-out entity shows the feature that stands in for it.
            const GeometryFeatureRef feature = featureOfPick(*geometryData, pickedInfo);
            const GeometryFeatureRef resolved = geometryData->resolveByMask(feature, _pickFilter);
            _featureInfo = geometryData->getFeatureInfo(resolved);
            if (_featureInfo.ref.isValid())
            {
                _featureInfo.elemId = pickedInfo.entityId;
                _hasPickedInfo = true;

                // Log only when the hovered feature changes, which keeps hovering cheap and
                // makes pick problems diagnosable from the log.
                if (_featureInfo.featureId != _lastLoggedFeatureId ||
                    _featureInfo.ref != _lastLoggedRef)
                {
                    _lastLoggedFeatureId = _featureInfo.featureId;
                    _lastLoggedRef = _featureInfo.ref;
                    LOG(INFO) << "GeometryInfoPanel picked " << _featureInfo.name
                              << " (type=" << featureTypeName(_featureInfo.ref.featureType)
                              << ", picked kind=" << static_cast<uint32_t>(pickedInfo.kind)
                              << ", entity=" << pickedInfo.entityId << ")";
                }
            }
            else
            {
                _hoverStatus = Common::String("picked entity has no geometry feature");
            }
        }
    }

    GeometryFeatureRef GeometryInfoPanel::featureOfPick(const GeometryData& data,
                                                       const PickedMeshInfo& picked) const
    {
        switch (picked.kind)
        {
        case PickedMeshKind::Node:
            // Vertex features are visualised by single mesh nodes.
            return data.getFeatureByNodeId(picked.entityId);
        case PickedMeshKind::ElementEdge:
            if (picked.faceIndex == UINT32_MAX)
            {
                // B-Rep edge segments carry the segment index and no face index.
                return data.getFeatureByEdgeSegment(picked.entityId);
            }
            return data.getFeatureByElement(picked.entityId);
        case PickedMeshKind::ElementFace:
        default:
            return data.getFeatureByElement(picked.entityId);
        }
    }

    void GeometryInfoPanel::applyPickFilter(uint32_t mask)
    {
        _pickFilter = mask & GEOMETRY_FEATURE_MASK_ALL;
        IniManager::instance().setInt(GEOMETRY_PICK_FILTER_KEY, static_cast<int>(_pickFilter));

        EventPacket packet;
        packet.setData("mask", _pickFilter);
        CommandManager::instance().call(EventId::SetGeometryPickFilter, Common::move(packet));
    }

    void GeometryInfoPanel::renderPickFilter()
    {
        // Mirror the live mask so changes made through the command (for example by an AI
        // client) are reflected here.
        _pickFilter = RenderWork::instance().getGeometryPickFilter();

        ImGui::TextUnformatted("Pick filter (feature types)");
        const struct
        {
            uint32_t bit;
            const char* label;
            const char* hint;
        } options[] = {
            {GEOMETRY_FEATURE_MASK_VERTEX, "Vertex", "Vertex features"},
            {GEOMETRY_FEATURE_MASK_EDGE, "Edge", "Edge features"},
            {GEOMETRY_FEATURE_MASK_FACE, "Face",
             "Face features (the tessellated mesh resolves to faces)"},
            {GEOMETRY_FEATURE_MASK_SOLID, "Solid",
             "With Face disabled, a face hit is promoted to the solid that owns it"},
        };

        for (const auto& option : options)
        {
            bool enabled = (_pickFilter & option.bit) != 0;
            if (ImGui::Checkbox(option.label, &enabled))
            {
                applyPickFilter(enabled ? (_pickFilter | option.bit) : (_pickFilter & ~option.bit));
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", option.hint);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        if (_pickFilter == 0)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
                               "No feature type is enabled: geometry picking is disabled.");
        }
        else
        {
            ImGui::Text("Active: %s", GeometryFeatureMaskInfo::toString(_pickFilter).c_str());
        }
    }

    void GeometryInfoPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(360, 320), ImGuiCond_FirstUseEver);
    }

    void GeometryInfoPanel::renderImpl()
    {
        if (_pickInfoSubscriptionId == 0)
        {
            _pickInfoSubscriptionId = RenderWork::instance().subscribePickInfo();
        }

        ImGui::Checkbox("Hover feature info", &_showHoverInfo);
        ImGui::TextWrapped("Hover a rendered face to inspect its B-Rep feature. Picking always "
                           "resolves to a complete feature, never to a single triangle.");
        ImGui::Separator();

        renderPickFilter();
        ImGui::Separator();

        updatePickedInfo();

        if (!_hasPickedInfo)
        {
            ImGui::TextWrapped("%s", _hoverStatus.size() == 0 ? "No feature picked."
                                                              : _hoverStatus.c_str());
        }
        else
        {
            ImGui::Text("Feature: %s", _featureInfo.name.c_str());
            ImGui::Text("Type: %s (%s)", featureTypeName(_featureInfo.ref.featureType).c_str(),
                        geometryTypeName(_featureInfo.geometryType).c_str());
            ImGui::Text("Feature id: %u   Elem id: %u", _featureInfo.featureId,
                        _featureInfo.elemId);
            ImGui::Separator();
            ImGui::Text("Length: %.6g", _featureInfo.length);
            ImGui::Text("Area:   %.6g", _featureInfo.area);
            ImGui::Text("Volume: %.6g", _featureInfo.volume);
            ImGui::Text("Center: (%.6g, %.6g, %.6g)", _featureInfo.x, _featureInfo.y,
                        _featureInfo.z);
            ImGui::Text("BBox min: (%.4g, %.4g, %.4g)", _featureInfo.bboxMin[0],
                        _featureInfo.bboxMin[1], _featureInfo.bboxMin[2]);
            ImGui::Text("BBox max: (%.4g, %.4g, %.4g)", _featureInfo.bboxMax[0],
                        _featureInfo.bboxMax[1], _featureInfo.bboxMax[2]);
            ImGui::Text("Parents: %u   Children: %u", _featureInfo.parentCount,
                        _featureInfo.childCount);
            ImGui::Text("Picked point: (%.6g, %.6g, %.6g)", _pickedInfo.x, _pickedInfo.y,
                        _pickedInfo.z);
        }

        syncPickRequest();
    }

    void GeometryInfoPanel::afterRenderFrame()
    {
        syncPickRequest();

        if (!_showHoverInfo || !_hasPickedInfo)
        {
            return;
        }

        // 鼠标位于任意 ImGui 窗口（本面板、其他面板、菜单）上时抑制 tooltip，
        // 仅在 3D 视口上悬停时展示。
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        {
            return;
        }

        ImGui::BeginTooltip();
        ImGui::Text("%s", _featureInfo.name.c_str());
        ImGui::Text("%s", featureTypeName(_featureInfo.ref.featureType).c_str());
        ImGui::Text("Length: %.6g  Area: %.6g  Volume: %.6g", _featureInfo.length,
                    _featureInfo.area, _featureInfo.volume);
        ImGui::Text("Center: (%.6g, %.6g, %.6g)", _featureInfo.x, _featureInfo.y,
                    _featureInfo.z);
        ImGui::Text("Picked: (%.6g, %.6g, %.6g)", _pickedInfo.x, _pickedInfo.y, _pickedInfo.z);
        ImGui::EndTooltip();
    }

    void GeometryInfoPanel::abortClose()
    {
        releasePickRequest();
        if (_pickInfoSubscriptionId != 0)
        {
            RenderWork::instance().unsubscribePickInfo(_pickInfoSubscriptionId);
            _pickInfoSubscriptionId = 0;
        }
        _hasPickedInfo = false;
    }
} // namespace EgLab::Platform
