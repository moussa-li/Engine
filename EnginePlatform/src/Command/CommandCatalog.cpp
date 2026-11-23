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

        // ===== MeshQueryParam（GetMeshNodes / GetMeshElements 共用） =====
        const ParamFieldDesc kMeshQueryFields[] = {
            {"dataNodeId", "Data Node Id", ParamFieldType::UInt64, 0, "0 = resolve by name"},
            {"name", "Mesh Name", ParamFieldType::String, 64, "Mesh name when dataNodeId=0"},
        };

        // ===== MeshNodesQueryParam / MeshElementsQueryParam（按 ID 查询） =====
        const ParamFieldDesc kMeshIdsQueryFields[] = {
            {"dataNodeId", "Data Node Id", ParamFieldType::UInt64, 0, "0 = resolve by name"},
            {"name", "Mesh Name", ParamFieldType::String, 64, "Mesh name when dataNodeId=0"},
            {"ids", "Ids", ParamFieldType::String, 256, "1, 2, 3"},
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

        // ===== SetReferencePlaneParam =====
        const ParamFieldDesc kReferencePlaneFields[] = {
            {"enabled", "Enabled", ParamFieldType::Bool, 0, "true / false"},
            {"showGrid", "Show Grid", ParamFieldType::Bool, 0, "true / false"},
            {"placement", "Placement", ParamFieldType::UInt32, 0,
             "0=bbox center 1=bbox back faces"},
            {"scale", "Bounds Scale", ParamFieldType::Float, 0, "default 1.2"},
            {"colorX", "Color X (r,g,b,a)", ParamFieldType::String, 64, "1.0,0.3,0.3,0.1"},
            {"colorY", "Color Y (r,g,b,a)", ParamFieldType::String, 64, "0.3,1.0,0.3,0.1"},
            {"colorZ", "Color Z (r,g,b,a)", ParamFieldType::String, 64, "0.3,0.5,1.0,0.1"},
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
            {EventId::ListDataTreeNodes, "ListDataTreeNodes", "List Data Tree Nodes", "Mesh",
             false, 0, nullptr, 0},
            {EventId::GetMeshNodes, "GetMeshNodes", "Get Mesh Nodes", "Mesh", true,
             sizeof(MeshQueryParam), kMeshQueryFields,
             sizeof(kMeshQueryFields) / sizeof(kMeshQueryFields[0])},
            {EventId::GetMeshElements, "GetMeshElements", "Get Mesh Elements", "Mesh", true,
             sizeof(MeshQueryParam), kMeshQueryFields,
             sizeof(kMeshQueryFields) / sizeof(kMeshQueryFields[0])},
            {EventId::GetNodesXYZ, "GetNodesXYZ", "Get Nodes XYZ", "Mesh", true,
             sizeof(MeshNodesQueryParam), kMeshIdsQueryFields,
             sizeof(kMeshIdsQueryFields) / sizeof(kMeshIdsQueryFields[0])},
            {EventId::GetElementsTopology, "GetElementsTopology", "Get Elements Topology", "Mesh",
             true, sizeof(MeshElementsQueryParam), kMeshIdsQueryFields,
             sizeof(kMeshIdsQueryFields) / sizeof(kMeshIdsQueryFields[0])},
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
            {EventId::SetReferencePlane, "SetReferencePlane", "Set Reference Plane", "View", true,
             sizeof(SetReferencePlaneParam), kReferencePlaneFields,
             sizeof(kReferencePlaneFields) / sizeof(kReferencePlaneFields[0])},
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
