#pragma once

#include <cstdint>

namespace EgLab::RE
{
    /// Mouse button identifiers shared by the window backend and the camera controller.
    /// The values match GLFW's mouse button numbering.
    enum class CameraMouseButton : int
    {
        None = -1,
        Left = 0,
        Right = 1,
        Middle = 2,
        X1 = 3,
        X2 = 4,
    };

    /// Modifier-key flags captured when a camera gesture starts.
    enum CameraModifier : uint32_t
    {
        CameraModifier_None = 0,
        CameraModifier_Ctrl = 1u << 0,
        CameraModifier_Shift = 1u << 1,
        CameraModifier_Alt = 1u << 2,
        CameraModifier_Super = 1u << 3,
    };

    /// One mouse chord: a button plus the modifier keys held together with it.
    struct CameraInputChord
    {
        int button{static_cast<int>(CameraMouseButton::None)};
        uint32_t mods{CameraModifier_None};

        bool valid() const
        {
            return button >= 0;
        }

        bool matches(int candidateButton, uint32_t candidateMods) const
        {
            return valid() && button == candidateButton && mods == candidateMods;
        }
    };

    /// Gesture a camera mouse chord performs while the button stays held.
    enum class CameraAction
    {
        None,
        Pan,
        Rotate,
        RotateLocal,
        Zoom,
    };

    /// Point the view is anchored to while zooming.
    enum class CameraZoomAnchor
    {
        ScreenCenter,
        MouseCursor,
    };

    /// Mouse chords that start each camera gesture.
    struct CameraInputBindings
    {
        CameraInputChord pan{};
        CameraInputChord rotate{};
        CameraInputChord rotateLocal{};
        CameraInputChord zoom{};

        /// Left pans, right (and Ctrl+right) orbit around the mesh nearest the cursor,
        /// middle drags the zoom.
        static CameraInputBindings defaults()
        {
            CameraInputBindings bindings;
            bindings.pan = {static_cast<int>(CameraMouseButton::Left), CameraModifier_None};
            bindings.rotate = {static_cast<int>(CameraMouseButton::Right), CameraModifier_None};
            bindings.rotateLocal = {static_cast<int>(CameraMouseButton::Right),
                                    CameraModifier_Ctrl};
            bindings.zoom = {static_cast<int>(CameraMouseButton::Middle), CameraModifier_None};
            return bindings;
        }

        /// Resolve the gesture for a button press; the local orbit wins when chords overlap.
        CameraAction resolve(int button, uint32_t mods) const
        {
            if (rotateLocal.matches(button, mods))
            {
                return CameraAction::RotateLocal;
            }
            if (pan.matches(button, mods))
            {
                return CameraAction::Pan;
            }
            if (rotate.matches(button, mods))
            {
                return CameraAction::Rotate;
            }
            if (zoom.matches(button, mods))
            {
                return CameraAction::Zoom;
            }
            return CameraAction::None;
        }
    };
} // namespace EgLab::RE
