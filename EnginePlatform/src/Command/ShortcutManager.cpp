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

        // Friendly name of a mouse-button key, or nullptr when the key is not a mouse button.
        const char* mouseKeyName(int keyValue);

        // Resolve a key name persisted in the ini file ("F", "Delete", "Num 0", ...).
        int findKeyByName(const char* name, size_t length)
        {
            if (name == nullptr || length == 0)
            {
                return 0;
            }

            for (int keyValue = ImGuiKey_MouseLeft; keyValue <= ImGuiKey_MouseX2; ++keyValue)
            {
                if (equalsIgnoreCase(name, length, mouseKeyName(keyValue)))
                {
                    return keyValue;
                }
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

        // Friendly name of a mouse-button key, or nullptr when the key is not a mouse button.
        const char* mouseKeyName(int keyValue)
        {
            switch (keyValue)
            {
            case ImGuiKey_MouseLeft:
                return "Mouse Left";
            case ImGuiKey_MouseRight:
                return "Mouse Right";
            case ImGuiKey_MouseMiddle:
                return "Mouse Middle";
            case ImGuiKey_MouseX1:
                return "Mouse X1";
            case ImGuiKey_MouseX2:
                return "Mouse X2";
            default:
                return nullptr;
            }
        }

        // Zero-based mouse button index used by the camera input bindings, -1 when not a mouse.
        int mouseButtonIndex(int keyValue)
        {
            switch (keyValue)
            {
            case ImGuiKey_MouseLeft:
                return 0;
            case ImGuiKey_MouseRight:
                return 1;
            case ImGuiKey_MouseMiddle:
                return 2;
            case ImGuiKey_MouseX1:
                return 3;
            case ImGuiKey_MouseX2:
                return 4;
            default:
                return -1;
            }
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

        const char* keyName = mouseKeyName(key);
        if (keyName == nullptr)
        {
            keyName = ImGui::GetKeyName(static_cast<ImGuiKey>(key));
        }
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
        refreshCameraBindings();
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

            // Held gestures (camera pan/rotate/zoom) are consumed by the camera controller,
            // which reacts to press and release instead of a one-shot chord.
            const CommandDesc* desc = CommandCatalog::find(entry.first);
            if (desc != nullptr && desc->holdInput)
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

        const EventId target = _captureTarget.load();
        const CommandDesc* desc = CommandCatalog::find(target);
        const bool mouseOnly = desc != nullptr && desc->mouseOnly;

        ShortcutChord chord;
        bool captured = false;

        // Camera gestures are recorded from mouse buttons so modifier keys can be held while
        // the button is clicked. Buttons already held when recording started are ignored.
        if (mouseOnly)
        {
            for (int index = 0; index < 5; ++index)
            {
                if (!ImGui::IsMouseDown(static_cast<ImGuiMouseButton>(index)))
                {
                    _captureIgnoreMouseButtons &= ~(1u << index);
                }
            }

            for (int index = 0; index < 5 && !captured; ++index)
            {
                if ((_captureIgnoreMouseButtons & (1u << index)) != 0)
                {
                    continue;
                }
                if (!ImGui::IsMouseClicked(static_cast<ImGuiMouseButton>(index)))
                {
                    continue;
                }

                chord.key = static_cast<int>(ImGuiKey_MouseLeft) + index;
                chord.mods = ImGui::GetIO().KeyMods & ImGuiMod_Mask_;
                captured = true;
            }
        }
        else
        {
            // Keyboard commands only record keyboard keys.
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

                chord.key = keyValue;
                chord.mods = ImGui::GetIO().KeyMods & ImGuiMod_Mask_;
                captured = true;
                break;
            }
        }

        if (!captured)
        {
            return;
        }

        _captureTarget.store(EventId::Unknow);
        bind(target, chord);
    }

    void ShortcutManager::beginCapture(EventId eventId)
    {
        ensureLoaded();

        _captureIgnoreMouseButtons = 0;
        for (int index = 0; index < 5; ++index)
        {
            if (ImGui::IsMouseDown(static_cast<ImGuiMouseButton>(index)))
            {
                _captureIgnoreMouseButtons |= (1u << index);
            }
        }

        _captureTarget.store(eventId);
    }

    void ShortcutManager::cancelCapture()
    {
        _captureTarget.store(EventId::Unknow);
        _captureIgnoreMouseButtons = 0;
    }

    void ShortcutManager::bind(EventId eventId, const ShortcutChord& chord)
    {
        ensureLoaded();

        if (!chord.valid())
        {
            setBinding(eventId, ShortcutChord{});
            saveToIni();
            refreshCameraBindings();
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
        refreshCameraBindings();
    }

    void ShortcutManager::unbind(EventId eventId)
    {
        ensureLoaded();
        setBinding(eventId, ShortcutChord{});
        saveToIni();
        refreshCameraBindings();
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
        refreshCameraBindings();
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
        case EventId::CameraPan:
            chord.key = ImGuiKey_MouseLeft;
            break;
        case EventId::CameraRotate:
            chord.key = ImGuiKey_MouseRight;
            break;
        case EventId::CameraRotateLocal:
            chord.key = ImGuiKey_MouseRight;
            chord.mods = ImGuiMod_Ctrl;
            break;
        case EventId::CameraZoom:
            chord.key = ImGuiKey_MouseMiddle;
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

    RE::CameraInputBindings ShortcutManager::cameraInputBindings() const
    {
        std::lock_guard<std::mutex> lock(_cameraBindingsMutex);
        return _cameraBindings;
    }

    void ShortcutManager::refreshCameraBindings()
    {
        RE::CameraInputBindings bindings;
        bindings.pan = toCameraChord(getBinding(EventId::CameraPan));
        bindings.rotate = toCameraChord(getBinding(EventId::CameraRotate));
        bindings.rotateLocal = toCameraChord(getBinding(EventId::CameraRotateLocal));
        bindings.zoom = toCameraChord(getBinding(EventId::CameraZoom));

        std::lock_guard<std::mutex> lock(_cameraBindingsMutex);
        _cameraBindings = bindings;
    }

    RE::CameraInputChord ShortcutManager::toCameraChord(const ShortcutChord& chord)
    {
        RE::CameraInputChord result;
        if (!chord.valid())
        {
            return result;
        }

        const int button = mouseButtonIndex(chord.key);
        if (button < 0)
        {
            return result;
        }

        result.button = button;
        if (chord.mods & ImGuiMod_Ctrl)
        {
            result.mods |= RE::CameraModifier_Ctrl;
        }
        if (chord.mods & ImGuiMod_Shift)
        {
            result.mods |= RE::CameraModifier_Shift;
        }
        if (chord.mods & ImGuiMod_Alt)
        {
            result.mods |= RE::CameraModifier_Alt;
        }
        if (chord.mods & ImGuiMod_Super)
        {
            result.mods |= RE::CameraModifier_Super;
        }
        return result;
    }
} // namespace EgLab::Platform
