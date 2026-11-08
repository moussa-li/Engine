#pragma once

#include <cstddef>

#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    // Stable panel names shared by the menu, the panels and the launcher.
    constexpr const char* kShortcutSettingPanelName = "ShortcutSetting";
    constexpr const char* kCommandParamWizardPanelName = "CommandParamWizard";

    // Editing widget the parameter wizard uses for one CommandParam field.
    enum class ParamFieldType
    {
        String, // char[N]
        Bool,
        UInt32,
        UInt64,
    };

    // Describes one field of a CommandParam structure.
    struct ParamFieldDesc
    {
        const char* name;          // Field name, also used as the EventPacket key
        const char* label;         // Label shown in the panel
        ParamFieldType type;       // Widget type
        size_t capacity{0};        // Buffer size of String fields (including the '\0')
        const char* hint{nullptr}; // Input hint
    };

    // Metadata of a single event: whether it carries a packet, and its CommandParam layout.
    struct CommandDesc
    {
        EventId eventId;                       // Event id
        const char* id;                        // Stable id, used to persist shortcut bindings
        const char* displayName;               // Name shown in the UI
        const char* category;                  // Group name
        bool needsPacket;                      // Whether an EventPacket is required
        size_t paramSize{0};                   // CommandParam structure size, 0 when unused
        const ParamFieldDesc* fields{nullptr}; // Field description of the parameters
        size_t fieldCount{0};                  // Number of fields
    };

    // Command catalog: associates each EventId with its CommandParam structure.
    // The shortcut system uses it to know whether an argument is needed, while the
    // parameter wizard uses it to collect the fields step by step.
    class CommandCatalog
    {
    public:
        // Returns every command description and writes the element count to count.
        static const CommandDesc* commands(size_t& count);
        static const CommandDesc* find(EventId eventId);
        static const CommandDesc* find(const Common::String& commandId);
    };
} // namespace EgLab::Platform
