#include "RenderEngine/Core/Window.hpp"

#include <GL/glew.h>

#include <atomic>

#include "Common/Log.hpp"
#include "GLFW/glfw3.h"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/Renderer.hpp"

namespace EgLab::RE
{
    void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                    GLsizei length, const GLchar *message, const void *userParam)
    {
        // 忽略一些低优先级的通知，只关注错误和警告
        if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
        {
            fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
        }
    }

    void glfwErrorCallback(int error, const char *description)
    {
        LOG(ERROR) << "GLFW Error " << error << ": " << description;
    }

    namespace
    {
        uint32_t toCameraModifiers(int mods)
        {
            uint32_t result = CameraModifier_None;
            if (mods & GLFW_MOD_CONTROL)
            {
                result |= CameraModifier_Ctrl;
            }
            if (mods & GLFW_MOD_SHIFT)
            {
                result |= CameraModifier_Shift;
            }
            if (mods & GLFW_MOD_ALT)
            {
                result |= CameraModifier_Alt;
            }
            if (mods & GLFW_MOD_SUPER)
            {
                result |= CameraModifier_Super;
            }
            return result;
        }
    } // namespace

    class Window::Impl
    {
    public:
        void init()
        {
            if (!glfwInit()) return;
            _glfwInitialized = true;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SAMPLES, 4);

            glfwSetErrorCallback(glfwErrorCallback);

            _window = glfwCreateWindow(_width, _height, "Hello World", NULL, NULL);
            if (!_window)
            {
                glfwTerminate();
                _glfwInitialized = false;
                return;
            }
        }

        void start()
        {
            glfwSetWindowUserPointer(_window, this);

            glfwSetMouseButtonCallback(_window, mouseButtonCallback);
            glfwSetCursorPosCallback(_window, mouseButtonMoveCallback);
            glfwSetScrollCallback(_window, mouseScrollCallback);

            if (glewInit() != GLEW_OK)
            {
                LOG(ERROR) << "Error";
            }

            glfwSwapInterval(1);

            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(MessageCallback, 0);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glEnable(GL_MULTISAMPLE);

            LOG(INFO) << glGetString(GL_VERSION);
        }

        bool shouldClose() const
        {
            if (_window == nullptr) return true;
            return glfwWindowShouldClose(_window);
        }

        void deal() const
        {
            glfwSwapBuffers(_window);
            glfwPollEvents();
        }

        void swapBuffers() const
        {
            glfwSwapBuffers(_window);
        }

        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
        {
            Impl *impl = static_cast<Impl *>(glfwGetWindowUserPointer(window));
            if (impl)
            {
                if (impl->isCameraInputMasked()) return;

                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                impl->processMouseButton(button, action, (float)xpos, (float)ypos, mods);
            }
        }

        static void mouseButtonMoveCallback(GLFWwindow *window, double xpos, double ypos)
        {
            Impl *impl = static_cast<Impl *>(glfwGetWindowUserPointer(window));
            if (impl)
            {
                if (impl->isCameraInputMasked()) return;

                impl->processMouseMove((float)xpos, (float)ypos);
            }
        }

        static void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
        {
            Impl *impl = static_cast<Impl *>(glfwGetWindowUserPointer(window));
            if (impl)
            {
                if (impl->isCameraInputMasked()) return;

                impl->processMouseScroll((float)yoffset);
            }
        }

        bool isCameraInputMasked() const
        {
            return _maskCallBack || _shortcutCaptureActive.load();
        }

        // While the selection tool owns the left mouse button the remaining camera controls
        // (right/middle drag, wheel zoom) must keep working.
        bool isLeftButtonMasked() const
        {
            return _pointerInputMasked.load();
        }

        void updateCameraViewport()
        {
            if (_cameraController == nullptr || _window == nullptr) return;

            int width = 0;
            int height = 0;
            glfwGetWindowSize(_window, &width, &height);
            _cameraController->setViewportSize(static_cast<float>(width),
                                               static_cast<float>(height));
        }

        void processMouseButton(int button, int action, float xpos, float ypos, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && isLeftButtonMasked())
            {
                // The rubber-band tool consumes the left button; no camera rotation and no
                // hover pick must start from it.
                return;
            }

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                _selectDirty = true;
                _selectX = xpos;
                _selectY = ypos;
            }
            if (_cameraController == nullptr) return;

            updateCameraViewport();
            _cameraController->processMouseButton(button, action, xpos, ypos,
                                                  toCameraModifiers(mods));
        }

        void processMouseMove(float xpos, float ypos)
        {
            if (_cameraController == nullptr) return;

            updateCameraViewport();
            if (_cameraController->processMouseMove(xpos, ypos)) return;

            if (isLeftButtonMasked()) return;

            _selectDirty = true;
            _selectX = xpos;
            _selectY = ypos;
        }

        void processMouseScroll(float yoffset)
        {
            if (_cameraController == nullptr || _window == nullptr) return;

            double xpos = 0.0;
            double ypos = 0.0;
            glfwGetCursorPos(_window, &xpos, &ypos);

            updateCameraViewport();
            _cameraController->processMouseScroll(yoffset, static_cast<float>(xpos),
                                                  static_cast<float>(ypos));
        }

    public:
        GLFWwindow *_window{nullptr};
        bool _glfwInitialized{false};
        Common::SharedPtr<CameraController> _cameraController;

        unsigned int _width;
        unsigned int _height;

        bool _selectDirty{false};
        Scalar _selectX{0.0f};
        Scalar _selectY{0.0f};

        float lastFrame{0.0f};
        float deltaTime{0.0f};
        bool _maskCallBack{false};
        std::atomic<bool> _pointerInputMasked{false};
        std::atomic<bool> _shortcutCaptureActive{false};
        std::mutex _mutex;
    };

    Window::Window() : _impl(new Impl)
    {
    }

    Window::~Window()
    {
        if (_impl != nullptr)
        {
            terminate();
            delete _impl;
            _impl = nullptr;
        }
    }

    Window::Window(const unsigned int &width, const unsigned int &height) : _impl(new Impl)
    {
        _impl->_width = width;
        _impl->_height = height;
        _impl->init();
    }

    void Window::start()
    {
        activeContext();
        _impl->start();
        deactiveContext();
    }

    GLFWwindow *Window::getNative() const
    {
        return _impl->_window;
    }

    void Window::getWindowSize(unsigned int &width, unsigned int &height) const
    {
        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetWindowSize(_impl->_window, &windowWidth, &windowHeight);
        width = static_cast<unsigned int>(windowWidth);
        height = static_cast<unsigned int>(windowHeight);
    }

    void Window::getFramebufferSize(unsigned int &width, unsigned int &height) const
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(_impl->_window, &framebufferWidth, &framebufferHeight);
        width = static_cast<unsigned int>(framebufferWidth);
        height = static_cast<unsigned int>(framebufferHeight);
    }

    void Window::setMaximized(bool maximized)
    {
        if (_impl == nullptr || _impl->_window == nullptr)
        {
            return;
        }

        if (maximized)
        {
            glfwMaximizeWindow(_impl->_window);
        }
        else
        {
            glfwRestoreWindow(_impl->_window);
        }
    }

    bool Window::isMaximized() const
    {
        return _impl != nullptr && _impl->_window != nullptr &&
               glfwGetWindowAttrib(_impl->_window, GLFW_MAXIMIZED) == GLFW_TRUE;
    }

    bool Window::shouldClose() const
    {
        return _impl->shouldClose();
    }

    DeltaTime Window::getTime() const
    {
        // TODO: implete to _impl
        return static_cast<DeltaTime>(glfwGetTime());
    }

    void Window::deal() const
    {
        activeContext();
        _impl->deal();
        deactiveContext();
    }

    void Window::swapBuffers() const
    {
        activeContext();
        _impl->swapBuffers();
        deactiveContext();
    }

    void Window::setCameraController(Common::SharedPtr<CameraController> cmaeraController)
    {
        _impl->_cameraController = cmaeraController;
        if (cmaeraController == nullptr) return;
        if (cmaeraController->enableCursor())
            glfwSetInputMode(_impl->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(_impl->_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Window::terminate()
    {
        if (_impl != nullptr && _impl->_window != nullptr)
        {
            glfwDestroyWindow(_impl->_window);
            _impl->_window = nullptr;
        }
        if (_impl != nullptr && _impl->_glfwInitialized)
        {
            glfwTerminate();
            _impl->_glfwInitialized = false;
        }
    }

    void Window::maskEvent(bool mask)
    {
        _impl->_maskCallBack = mask;
    }

    void Window::setShortcutCaptureActive(bool active)
    {
        _impl->_shortcutCaptureActive.store(active);
    }

    void Window::setPointerInputMasked(bool masked)
    {
        _impl->_pointerInputMasked.store(masked);
    }

    void Window::activeContext() const
    {
        _impl->_mutex.lock();
        glfwMakeContextCurrent(_impl->_window);
    }

    void Window::deactiveContext() const
    {
        glfwMakeContextCurrent(nullptr);
        _impl->_mutex.unlock();
    }

    bool Window::trySelectedPos(Scalar &x, Scalar &y)
    {
        if (_impl->_selectDirty)
        {
            x = _impl->_selectX;
            y = _impl->_selectY;
            _impl->_selectDirty = false;
            return true;
        }
        return false;
    }

} // namespace EgLab::RE