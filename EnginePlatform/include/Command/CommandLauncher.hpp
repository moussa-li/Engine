#pragma once

#include <cstddef>
#include <cstdint>

#include "Command/CommandIds.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    struct CommandDesc;

    // Single entry point for every way of running a command (shortcut or panel button):
    //  - EventId-only commands: dispatched to CommandManager immediately;
    //  - commands that need an EventPacket: a draft is created from the CommandParam field
    //    description, the parameter wizard collects the values step by step, then dispatches.
    class CommandLauncher : public Common::Singleton<CommandLauncher>
    {
    public:
        // Trigger a command. Returns false when the event is not part of the command catalog.
        bool invoke(EventId eventId);

        // ===== Parameter wizard draft =====
        bool hasDraft() const;
        EventId draftEventId() const;
        const CommandDesc* draftDesc() const;

        size_t draftStep() const;
        void setDraftStep(size_t step);
        void discardDraft();

        // Draft revision: bumped whenever a draft is created or discarded, so the panel knows
        // when its edit widgets must resynchronise with the draft.
        uint64_t draftRevision() const
        {
            return _revision;
        }

        Common::String getDraftString(size_t fieldIndex) const;
        void setDraftString(size_t fieldIndex, const char* text);
        uint64_t getDraftUInt(size_t fieldIndex) const;
        void setDraftUInt(size_t fieldIndex, uint64_t value);

        // Build the EventPacket from the catalog field description and hand it to CommandManager.
        bool submitDraft();

    private:
        friend class Common::Singleton<CommandLauncher>;

        CommandLauncher();
        ~CommandLauncher();

        struct FieldValue
        {
            Common::String text;
            uint64_t number{0};
        };

        EventId _eventId{EventId::Unknow};
        size_t _step{0};
        uint64_t _revision{0};
        Common::DynamicArray<FieldValue> _values;
    };
} // namespace EgLab::Platform
