#include "Command/CommandLauncher.hpp"

#include "Command/CommandCatalog.hpp"
#include "Command/CommandManager.hpp"
#include "Common/Utils.hpp"

namespace EgLab::Platform
{
    namespace
    {
        void requestShowPanel(const Common::String& panelName)
        {
            EventPacket packet;
            packet.setData("panelName", panelName);
            CommandManager::instance().call(EventId::ShowPanel, Common::move(packet));
        }
    } // namespace

    CommandLauncher::CommandLauncher()
    {
    }

    CommandLauncher::~CommandLauncher()
    {
    }

    bool CommandLauncher::invoke(EventId eventId)
    {
        const CommandDesc* desc = CommandCatalog::find(eventId);
        if (desc == nullptr)
        {
            return false;
        }

        if (!desc->needsPacket)
        {
            // EventId-only command: dispatch to CommandManager directly.
            CommandManager::instance().call(eventId, EventPacket{});
            return true;
        }

        // Parameterised command: prepare one draft slot per field, then open the wizard.
        _eventId = eventId;
        _step = 0;
        _values.clear();
        _values.resize(desc->fieldCount);
        ++_revision;

        requestShowPanel(Common::String(kCommandParamWizardPanelName));
        return true;
    }

    bool CommandLauncher::hasDraft() const
    {
        return _eventId != EventId::Unknow;
    }

    EventId CommandLauncher::draftEventId() const
    {
        return _eventId;
    }

    const CommandDesc* CommandLauncher::draftDesc() const
    {
        if (_eventId == EventId::Unknow)
        {
            return nullptr;
        }
        return CommandCatalog::find(_eventId);
    }

    size_t CommandLauncher::draftStep() const
    {
        return _step;
    }

    void CommandLauncher::setDraftStep(size_t step)
    {
        const CommandDesc* desc = draftDesc();
        if (desc == nullptr || desc->fieldCount == 0)
        {
            _step = 0;
            return;
        }
        _step = step < desc->fieldCount ? step : desc->fieldCount - 1;
    }

    void CommandLauncher::discardDraft()
    {
        _eventId = EventId::Unknow;
        _step = 0;
        ++_revision;
        _values.clear();
    }

    Common::String CommandLauncher::getDraftString(size_t fieldIndex) const
    {
        if (fieldIndex >= _values.size())
        {
            return Common::String();
        }
        return _values[fieldIndex].text;
    }

    void CommandLauncher::setDraftString(size_t fieldIndex, const char* text)
    {
        if (fieldIndex >= _values.size())
        {
            return;
        }
        _values[fieldIndex].text = Common::String(text != nullptr ? text : "");
    }

    uint64_t CommandLauncher::getDraftUInt(size_t fieldIndex) const
    {
        if (fieldIndex >= _values.size())
        {
            return 0;
        }
        return _values[fieldIndex].number;
    }

    void CommandLauncher::setDraftUInt(size_t fieldIndex, uint64_t value)
    {
        if (fieldIndex >= _values.size())
        {
            return;
        }
        _values[fieldIndex].number = value;
    }

    bool CommandLauncher::submitDraft()
    {
        const CommandDesc* desc = draftDesc();
        if (desc == nullptr || !desc->needsPacket)
        {
            discardDraft();
            return false;
        }

        EventPacket packet;
        for (size_t i = 0; i < desc->fieldCount; ++i)
        {
            const ParamFieldDesc& field = desc->fields[i];
            const Common::String key(field.name);
            switch (field.type)
            {
            case ParamFieldType::String:
                packet.setData(key, getDraftString(i));
                break;
            case ParamFieldType::Bool:
                packet.setData(key, getDraftUInt(i) != 0);
                break;
            case ParamFieldType::UInt32:
                packet.setData(key, static_cast<uint32_t>(getDraftUInt(i)));
                break;
            case ParamFieldType::UInt64:
                packet.setData(key, getDraftUInt(i));
                break;
            }
        }

        CommandManager::instance().call(desc->eventId, Common::move(packet));
        discardDraft();
        return true;
    }
} // namespace EgLab::Platform
