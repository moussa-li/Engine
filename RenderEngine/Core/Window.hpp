#pragma once

#include "Common/SharedPtr.hpp"
#include "Core/Definites.hpp"
#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngineAPI.hpp"

struct GLFWwindow;
namespace EgLab::RE
{

    class CameraController;
    /// Owns a native window and exposes its event, context, and input operations.
    class RenderEngineAPI Window
    {
    public:
        /// @brief Creates a window with default dimensions.
        Window();
        /// @brief Destroys the window and releases its resources.
        ~Window();
        /// @brief Creates a window with the specified dimensions.
        /// @param width Window width in pixels.
        /// @param height Window height in pixels.
        Window(const unsigned int &width, const unsigned int &height);

        /// @brief Updates the window dimensions.
        /// @param width New width in pixels.
        /// @param height New height in pixels.
        void setSize(const unsigned int &width, const unsigned int &height);
        /// @brief Sets whether the window is maximized.
        /// @param maximized True to maximize the window.
        void setMaximized(bool maximized);
        /// @brief Reports whether the window is maximized.
        bool isMaximized() const;

        /// @brief Reports whether the window should close.
        bool shouldClose() const;

        /// @brief Enables or disables window event handling.
        /// @details Controls whether window events are masked.
        void maskEvent(bool);

        /// @brief Returns elapsed application time in seconds.
        DeltaTime getTime() const;

        /// @brief Starts the window event loop.
        void start();

        /// @brief Processes pending window events.
        void deal() const;

        /// @brief Swaps the front and back rendering buffers.
        void swapBuffers() const;

        /// @brief Terminates the window and its rendering context.
        void terminate();

        /// @brief Sets the camera controller associated with this window.
        /// @details The supplied controller receives window camera input.
        void setCameraController(Common::SharedPtr<CameraController>);

        /// @brief Returns the native GLFW window handle.
        GLFWwindow *getNative() const;

        /// @brief Retrieves the logical window dimensions.
        /// @param width Receives the window width in pixels.
        /// @param height Receives the window height in pixels.
        void getWindowSize(unsigned int &width, unsigned int &height) const;

        /// @brief Retrieves the framebuffer dimensions.
        /// @param width Receives the framebuffer width in pixels.
        /// @param height Receives the framebuffer height in pixels.
        void getFramebufferSize(unsigned int &width, unsigned int &height) const;

        /// @brief Makes this window's rendering context current.
        void activeContext() const;

        /// @brief Releases this window's current rendering context.
        void deactiveContext() const;

        /// @brief Retrieves the selected cursor position, if one is available.
        /// @param x Receives the cursor x-coordinate.
        /// @param y Receives the cursor y-coordinate.
        /// @return true if a selected position is available.
        bool trySelectedPos(Scalar &x, Scalar &y);

    private:
        class Impl;
        Impl *_impl;
    };

} // namespace EgLab::RE