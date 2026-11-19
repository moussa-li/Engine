#include "Command/CommandCatalog.hpp"

namespace EgLab::Platform
{
    namespace
    {
        // ===== ShowPanelParam =====
        const ParamFieldDesc kShowPanelFields[] = {
            {"panelName", "Panel Name", ParamFieldType::String, sizeof(ShowPanelParam::panelName),
             "MeshImport / CameraSetting / DataTree ..."},
        };

        // ===== HidePanelParam =====
        const ParamFieldDesc kHidePanelFields[] = {
            {"panelName", "Panel Name", ParamFieldType::String, sizeof(HidePanelParam::panelName),
             "MeshImport / CameraSetting / DataTree ..."},
        };

        // ===== SetCameraParam =====
        const ParamFieldDesc kSetCameraFields[] = {
            {"mode", "Camera Mode", ParamFieldType::UInt32, 0, "0=orbit 1=fps"},
        };

        // ===== GetMeshDataParam =====
        const ParamFieldDesc kGetMeshDataFields[] = {
            {"dataNodeId", "Data Node Id", ParamFieldType::UInt64, 0, "0 = whole scene summary"},
        };

        // ===== ImportMeshParam =====
        const ParamFieldDesc kImportMeshFields[] = {
            {"fileDir", "Mesh File", ParamFieldType::String, sizeof(ImportMeshParam::fileDir),
             "C:/path/to/model.obj"},
        };

        // ===== DeleteMeshParam =====
        const ParamFieldDesc kDeleteMeshFields[] = {
            {"dataNodeId", "Data Node Id", ParamFieldType::UInt64, 0,
             "Id of the data node to delete"},
        };

        // ===== HighlightParam =====
        const ParamFieldDesc kHighlightFields[] = {
            {"ids", "Data Node Ids", ParamFieldType::String, 256, "1, 2, 3"},
        };

        // ===== LocateElementsParam =====
        const ParamFieldDesc kLocateFields[] = {
            {"nodeId", "Data Node Id", ParamFieldType::UInt64, 0, "Mesh data node id"},
            {"ids", "Element Ids", ParamFieldType::String, 256, "1, 2, 3"},
        };

        // ===== ElementsVisibilityParam =====
        const ParamFieldDesc kElementsVisibilityFields[] = {
            {"nodeId", "Data Node Id", ParamFieldType::UInt64, 0, "Mesh data node id"},
            {"mode", "Mode", ParamFieldType::UInt32, 0, "0=hide 1=show 2=show only 3=hide only"},
            {"ids", "Element Ids", ParamFieldType::String, 256, "1, 2, 3"},
        };

        // ===== NodeVisibilityParam =====
        const ParamFieldDesc kNodeVisibilityFields[] = {
            {"nodeId", "Data Node Id", ParamFieldType::UInt64, 0, "Mesh data node id"},
            {"mode", "Mode", ParamFieldType::UInt32, 0, "0=hide 1=show 2=show only 3=hide only"},
            {"ids", "Node Ids", ParamFieldType::String, 256, "1, 2, 3"},
        };

        // UpdateMesh carries a raw memory pointer (published directly by MeshEngine) and has no
        // fixed field layout, therefore it is not part of the command catalog.
        const CommandDesc kCommands[] = {
            {EventId::FitView, "FitView", "Fit View", "View", false, 0, nullptr, 0},
            {EventId::ShowPanel, "ShowPanel", "Show Panel", "Panel", true, sizeof(ShowPanelParam),
             kShowPanelFields, sizeof(kShowPanelFields) / sizeof(kShowPanelFields[0])},
            {EventId::HidePanel, "HidePanel", "Hide Panel", "Panel", true, sizeof(HidePanelParam),
             kHidePanelFields, sizeof(kHidePanelFields) / sizeof(kHidePanelFields[0])},
            {EventId::MeshImport, "MeshImport", "Import Mesh", "Mesh", true,
             sizeof(ImportMeshParam), kImportMeshFields,
             sizeof(kImportMeshFields) / sizeof(kImportMeshFields[0])},
            {EventId::MeshDelete, "MeshDelete", "Delete Mesh", "Mesh", true,
             sizeof(DeleteMeshParam), kDeleteMeshFields,
             sizeof(kDeleteMeshFields) / sizeof(kDeleteMeshFields[0])},
            {EventId::GetMeshData, "GetMeshData", "Get Mesh Data", "Mesh", true,
             sizeof(GetMeshDataParam), kGetMeshDataFields,
             sizeof(kGetMeshDataFields) / sizeof(kGetMeshDataFields[0])},
            {EventId::Highlight, "Highlight", "Highlight Meshes", "View", true,
             sizeof(HighlightParam), kHighlightFields,
             sizeof(kHighlightFields) / sizeof(kHighlightFields[0])},
            {EventId::LocateElements, "LocateElements", "Locate Elements", "Mesh", true,
             sizeof(LocateElementsParam), kLocateFields,
             sizeof(kLocateFields) / sizeof(kLocateFields[0])},
            {EventId::ElementsVisibility, "ElementsVisibility", "Elements Visibility",
             "Mesh", true, sizeof(ElementsVisibilityParam), kElementsVisibilityFields,
             sizeof(kElementsVisibilityFields) / sizeof(kElementsVisibilityFields[0])},
            {EventId::NodeVisibility, "NodeVisibility", "Node Visibility", "Mesh", true,
             sizeof(NodeVisibilityParam), kNodeVisibilityFields,
             sizeof(kNodeVisibilityFields) / sizeof(kNodeVisibilityFields[0])},
            {EventId::CameraPan, "CameraPan", "Pan Camera", "Camera", false, 0, nullptr, 0, true,
             true},
            {EventId::CameraRotate, "CameraRotate", "Rotate Camera", "Camera", false, 0, nullptr,
             0, true, true},
            {EventId::CameraRotateLocal, "CameraRotateLocal", "Rotate Camera (Local Axis)",
             "Camera", false, 0, nullptr, 0, true, true},
            {EventId::CameraZoom, "CameraZoom", "Zoom Camera", "Camera", false, 0, nullptr, 0,
             true, true},
            {EventId::SetCamera, "SetCamera", "Set Camera", "Camera", true,
             sizeof(SetCameraParam), kSetCameraFields,
             sizeof(kSetCameraFields) / sizeof(kSetCameraFields[0])},
        };

        constexpr size_t kCommandCount = sizeof(kCommands) / sizeof(kCommands[0]);
    } // namespace

    const CommandDesc* CommandCatalog::commands(size_t& count)
    {
        count = kCommandCount;
        return kCommands;
    }

    const CommandDesc* CommandCatalog::find(EventId eventId)
    {
        for (size_t i = 0; i < kCommandCount; ++i)
        {
            if (kCommands[i].eventId == eventId)
            {
                return &kCommands[i];
            }
        }
        return nullptr;
    }

    const CommandDesc* CommandCatalog::find(const Common::String& commandId)
    {
        for (size_t i = 0; i < kCommandCount; ++i)
        {
            if (commandId == kCommands[i].id)
            {
                return &kCommands[i];
            }
        }
        return nullptr;
    }
} // namespace EgLab::Platform
