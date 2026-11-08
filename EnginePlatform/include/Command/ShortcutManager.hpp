#pragma once

#include <cstdint>

#include "Command/CommandIds.hpp"
#include "Common/HashMap.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    // A shortcut chord: main key plus modifier keys.
    // The key is stored as a raw ImGuiKey value so the header does not depend on imgui.h.
    struct ShortcutChord
    {
        int key{0};  // ImGuiKey value, 0 means unbound
        int mods{0}; // ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiMod_Super

        bool valid() const
        {
            return key != 0;
        }

        bool operator==(const ShortcutChord& other) const
        {
            return key == other.key && mods == other.mods;
        }

        bool operator!=(const ShortcutChord& other) const
        {
            return !(*this == other);
        }

        int toImGuiChord() const
        {
            return key | mods;
        }

        // Readable text such as "Ctrl+Shift+F", also used as the ini value.
        Common::String toString() const;
        static ShortcutChord fromString(const Common::String& text);
    };

    // Shortcut manager: keeps the EventId -> chord mapping.
    // Input is polled once per frame and routed through CommandLauncher into CommandManager,
    // so every CommandManager call can be triggered by a shortcut.
    class ShortcutManager : public Common::Singleton<ShortcutManager>
    {
    public:
        // Call once per frame, before MainMenuBar is rendered.
        void processInput();

        // Trigger one event directly; the settings panel "Run" button reuses this entry.
        bool trigger(EventId eventId);

        void bind(EventId eventId, const ShortcutChord& chord);
        void unbind(EventId eventId);
        ShortcutChord getBinding(EventId eventId) const;
        bool isBound(EventId eventId) const;

        // Returns the event currently owning the chord, or EventId::Unknow when free.
        EventId findOwner(const ShortcutChord& chord) const;

        // Record a chord: the first non-modifier key pressed wins, Esc cancels.
        void beginCapture(EventId eventId);
        void cancelCapture();
        bool isCapturing() const
        {
            return _captureTarget != EventId::Unknow;
        }
        EventId captureTarget() const
        {
            return _captureTarget;
        }

        void resetToDefaults();

    private:
        friend class Common::Singleton<ShortcutManager>;

        ShortcutManager();
        ~ShortcutManager();

        void ensureLoaded();
        void handleCapture();
        static ShortcutChord defaultChord(EventId eventId);
        void applyDefaultBindings();
        void loadFromIni();
        void saveToIni() const;
        void setBinding(EventId eventId, const ShortcutChord& chord);

        Common::HashMap<EventId, ShortcutChord> _bindings;
        EventId _captureTarget{EventId::Unknow};
        bool _loaded{false};
    };
} // namespace EgLab::Platform
