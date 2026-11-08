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

        // UpdateMesh carries a raw memory pointer (published directly by MeshEngine) and has no
        // fixed field layout, therefore it is not part of the command catalog.
        const CommandDesc kCommands[] = {
            {EventId::FitView, "FitView", "Fit View", "View", false, 0, nullptr, 0},
            {EventId::ShowPanel, "ShowPanel", "Show Panel", "Panel", true, sizeof(ShowPanelParam),
             kShowPanelFields, sizeof(kShowPanelFields) / sizeof(kShowPanelFields[0])},
            {EventId::MeshImport, "MeshImport", "Import Mesh", "Mesh", true,
             sizeof(ImportMeshParam), kImportMeshFields,
             sizeof(kImportMeshFields) / sizeof(kImportMeshFields[0])},
            {EventId::MeshDelete, "MeshDelete", "Delete Mesh", "Mesh", true,
             sizeof(DeleteMeshParam), kDeleteMeshFields,
             sizeof(kDeleteMeshFields) / sizeof(kDeleteMeshFields[0])},
            {EventId::Highlight, "Highlight", "Highlight Meshes", "View", true,
             sizeof(HighlightParam), kHighlightFields,
             sizeof(kHighlightFields) / sizeof(kHighlightFields[0])},
            {EventId::LocateElements, "LocateElements", "Locate Elements", "Mesh", true,
             sizeof(LocateElementsParam), kLocateFields,
             sizeof(kLocateFields) / sizeof(kLocateFields[0])},
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
