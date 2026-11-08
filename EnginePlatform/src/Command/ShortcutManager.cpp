#include "Command/ShortcutManager.hpp"

#include "Command/CommandCatalog.hpp"
#include "Command/CommandLauncher.hpp"
#include "Common/Log.hpp"
#include "Core/IniManager.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        constexpr const char* kIniPrefix = "shortcut.";
        constexpr const char* kUnsetSentinel = "__unset__";

        char toLowerAscii(char ch)
        {
            return (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch - 'A' + 'a') : ch;
        }

        // Case-insensitive comparison between a fixed-length view and a null-terminated token.
        bool equalsIgnoreCase(const char* value, size_t length, const char* expected)
        {
            if (value == nullptr || expected == nullptr)
            {
                return false;
            }

            size_t index = 0;
            for (; index < length; ++index)
            {
                if (expected[index] == '\0')
                {
                    return false;
                }
                if (toLowerAscii(value[index]) != toLowerAscii(expected[index]))
                {
                    return false;
                }
            }
            return expected[index] == '\0';
        }

        // Resolve a key name persisted in the ini file ("F", "Delete", "Num 0", ...).
        int findKeyByName(const char* name, size_t length)
        {
            if (name == nullptr || length == 0)
            {
                return 0;
            }

            for (int keyValue = ImGuiKey_NamedKey_BEGIN; keyValue < ImGuiKey_NamedKey_END;
                 ++keyValue)
            {
                const char* keyName = ImGui::GetKeyName(static_cast<ImGuiKey>(keyValue));
                if (equalsIgnoreCase(name, length, keyName))
                {
                    return keyValue;
                }
            }
            return 0;
        }

        // Skip the modifier keys themselves while recording, wait for the real main key.
        bool isModifierKey(int keyValue)
        {
            return keyValue == ImGuiKey_LeftCtrl || keyValue == ImGuiKey_LeftShift ||
                   keyValue == ImGuiKey_LeftAlt || keyValue == ImGuiKey_LeftSuper ||
                   keyValue == ImGuiKey_RightCtrl || keyValue == ImGuiKey_RightShift ||
                   keyValue == ImGuiKey_RightAlt || keyValue == ImGuiKey_RightSuper;
        }
    } // namespace

    Common::String ShortcutChord::toString() const
    {
        if (!valid())
        {
            return Common::String();
        }

        Common::String text;
        if (mods & ImGuiMod_Ctrl)
        {
            text += Common::String("Ctrl+");
        }
        if (mods & ImGuiMod_Shift)
        {
            text += Common::String("Shift+");
        }
        if (mods & ImGuiMod_Alt)
        {
            text += Common::String("Alt+");
        }
        if (mods & ImGuiMod_Super)
        {
            text += Common::String("Super+");
        }

        const char* keyName = ImGui::GetKeyName(static_cast<ImGuiKey>(key));
        text += Common::String(keyName != nullptr ? keyName : "Unknown");
        return text;
    }

    ShortcutChord ShortcutChord::fromString(const Common::String& text)
    {
        ShortcutChord chord;
        if (text.size() == 0)
        {
            return chord;
        }

        const char* cursor = text.c_str();
        while (cursor != nullptr && *cursor != '\0')
        {
            const char* tokenBegin = cursor;
            while (*cursor != '\0' && *cursor != '+')
            {
                ++cursor;
            }

            const size_t tokenLength = static_cast<size_t>(cursor - tokenBegin);
            if (*cursor == '+')
            {
                ++cursor;
            }

            if (tokenLength == 0)
            {
                continue;
            }

            if (equalsIgnoreCase(tokenBegin, tokenLength, "Ctrl"))
            {
                chord.mods |= ImGuiMod_Ctrl;
            }
            else if (equalsIgnoreCase(tokenBegin, tokenLength, "Shift"))
            {
                chord.mods |= ImGuiMod_Shift;
            }
            else if (equalsIgnoreCase(tokenBegin, tokenLength, "Alt"))
            {
                chord.mods |= ImGuiMod_Alt;
            }
            else if (equalsIgnoreCase(tokenBegin, tokenLength, "Super") ||
                     equalsIgnoreCase(tokenBegin, tokenLength, "Win"))
            {
                chord.mods |= ImGuiMod_Super;
            }
            else
            {
                chord.key = findKeyByName(tokenBegin, tokenLength);
            }
        }

        // A chord made of modifiers only is treated as unbound.
        if (chord.key == 0)
        {
            chord.mods = 0;
        }
        return chord;
    }

    ShortcutManager::ShortcutManager()
    {
        ensureLoaded();
    }

    ShortcutManager::~ShortcutManager()
    {
    }

    void ShortcutManager::ensureLoaded()
    {
        if (_loaded)
        {
            return;
        }
        _loaded = true;

        applyDefaultBindings();
        loadFromIni();
    }

    void ShortcutManager::processInput()
    {
        ensureLoaded();

        if (isCapturing())
        {
            handleCapture();
            return;
        }

        // Stay quiet while text input is active so shortcuts do not swallow typed characters.
        if (ImGui::GetIO().WantTextInput)
        {
            return;
        }

        for (auto& entry : _bindings)
        {
            if (!entry.second.valid())
            {
                continue;
            }
            if (ImGui::Shortcut(entry.second.toImGuiChord(), ImGuiInputFlags_RouteGlobal))
            {
                trigger(entry.first);
            }
        }
    }

    bool ShortcutManager::trigger(EventId eventId)
    {
        return CommandLauncher::instance().invoke(eventId);
    }

    void ShortcutManager::handleCapture()
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        {
            cancelCapture();
            return;
        }

        // Only keyboard keys are scanned; mouse and gamepad keys are ignored.
        for (int keyValue = ImGuiKey_Tab; keyValue <= ImGuiKey_Oem102; ++keyValue)
        {
            if (isModifierKey(keyValue))
            {
                continue;
            }
            if (!ImGui::IsKeyPressed(static_cast<ImGuiKey>(keyValue), false))
            {
                continue;
            }

            ShortcutChord chord;
            chord.key = keyValue;
            chord.mods = ImGui::GetIO().KeyMods & ImGuiMod_Mask_;

            const EventId target = _captureTarget;
            _captureTarget = EventId::Unknow;
            bind(target, chord);
            return;
        }
    }

    void ShortcutManager::beginCapture(EventId eventId)
    {
        ensureLoaded();
        _captureTarget = eventId;
    }

    void ShortcutManager::cancelCapture()
    {
        _captureTarget = EventId::Unknow;
    }

    void ShortcutManager::bind(EventId eventId, const ShortcutChord& chord)
    {
        ensureLoaded();

        if (!chord.valid())
        {
            setBinding(eventId, ShortcutChord{});
            saveToIni();
            return;
        }

        // A shortcut stays unique: the new binding replaces the previous owner.
        const EventId owner = findOwner(chord);
        if (owner != EventId::Unknow && owner != eventId)
        {
            setBinding(owner, ShortcutChord{});
            if (const CommandDesc* desc = CommandCatalog::find(owner))
            {
                LOG(WARNING) << "Shortcut " << chord.toString() << " is moved from command ["
                             << desc->id << "] to the new owner.";
            }
        }

        setBinding(eventId, chord);
        saveToIni();
    }

    void ShortcutManager::unbind(EventId eventId)
    {
        ensureLoaded();
        setBinding(eventId, ShortcutChord{});
        saveToIni();
    }

    ShortcutChord ShortcutManager::getBinding(EventId eventId) const
    {
        auto it = _bindings.find(eventId);
        if (it == _bindings.end())
        {
            return ShortcutChord{};
        }
        return it.data().second;
    }

    bool ShortcutManager::isBound(EventId eventId) const
    {
        return getBinding(eventId).valid();
    }

    EventId ShortcutManager::findOwner(const ShortcutChord& chord) const
    {
        if (!chord.valid())
        {
            return EventId::Unknow;
        }

        for (auto& entry : _bindings)
        {
            if (entry.second == chord)
            {
                return entry.first;
            }
        }
        return EventId::Unknow;
    }

    void ShortcutManager::resetToDefaults()
    {
        ensureLoaded();
        applyDefaultBindings();
        saveToIni();
    }

    void ShortcutManager::setBinding(EventId eventId, const ShortcutChord& chord)
    {
        if (!chord.valid())
        {
            _bindings.erase(eventId);
            return;
        }
        _bindings[eventId] = chord;
    }

    void ShortcutManager::applyDefaultBindings()
    {
        size_t count = 0;
        const CommandDesc* commands = CommandCatalog::commands(count);
        for (size_t i = 0; i < count; ++i)
        {
            setBinding(commands[i].eventId, defaultChord(commands[i].eventId));
        }
    }

    ShortcutChord ShortcutManager::defaultChord(EventId eventId)
    {
        ShortcutChord chord;
        switch (eventId)
        {
        case EventId::FitView:
            chord.key = ImGuiKey_F;
            break;
        case EventId::MeshDelete:
            chord.key = ImGuiKey_Delete;
            chord.mods = ImGuiMod_Ctrl;
            break;
        default:
            break;
        }
        return chord;
    }

    void ShortcutManager::loadFromIni()
    {
        size_t count = 0;
        const CommandDesc* commands = CommandCatalog::commands(count);

        for (size_t i = 0; i < count; ++i)
        {
            const Common::String key = Common::String(kIniPrefix) + Common::String(commands[i].id);
            const Common::String value = IniManager::instance().getValue(key, kUnsetSentinel);
            if (value == kUnsetSentinel)
            {
                // Never saved before: keep the default binding.
                continue;
            }
            setBinding(commands[i].eventId, ShortcutChord::fromString(value));
        }
    }

    void ShortcutManager::saveToIni() const
    {
        size_t count = 0;
        const CommandDesc* commands = CommandCatalog::commands(count);

        for (size_t i = 0; i < count; ++i)
        {
            const Common::String key = Common::String(kIniPrefix) + Common::String(commands[i].id);
            IniManager::instance().setValue(key, getBinding(commands[i].eventId).toString());
        }
    }
} // namespace EgLab::Platform
