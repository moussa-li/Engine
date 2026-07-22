#include "RenderEngine/Core/Window.hpp"

#include "Common/Log.hpp"
#include "GLFW/glfw3.h"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/Renderer.hpp"

namespace EgLab
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

    class Window::Impl
    {
    public:
        void init()
        {
            if (!glfwInit()) return;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            glfwSetErrorCallback(glfwErrorCallback);

            /* Create a windowed mode window and its OpenGL context */
            _window = glfwCreateWindow(_width, _height, "Hello World", NULL, NULL);

            /* set user control */
            // glfwSetCursorPosCallback(_window, mouse_callback);
            // glfwSetScrollCallback(_window, scroll_callback);
            // glfwSetFramebufferSizeCallback(_window, framebuffer_size_callback);

            if (!_window)
            {
                glfwTerminate();
                return;
            }

            /* Make the window's context current */

            /* disable cursor */
            glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            glfwSetWindowUserPointer(_window, this);

            /* user operator */
            glfwMakeContextCurrent(_window);

            glfwSetMouseButtonCallback(_window, mouseButtonCallback);

            glfwSetCursorPosCallback(_window, mouseButtonMoveCallback);

            glfwSetScrollCallback(_window, mouseScrollCallback);

            /* v-Sync */
            // glfwSwapInterval(1);

            if (glewInit() != GLEW_OK)
            {
                LOG(ERROR) << "Error";
            }

            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(MessageCallback, 0);
            // 确保同步输出，这样错误发生时会立即触发回调（开发阶段推荐）
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

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

        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
        {
            Impl *impl = static_cast<Impl *>(glfwGetWindowUserPointer(window));
            if (impl)
            {
                // if (ImGui::GetIO().WantCaptureMouse) return;

                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                impl->processMouseButton(button, action, (float)xpos, (float)ypos);
            }
        }

        static void mouseButtonMoveCallback(GLFWwindow *window, double xpos, double ypos)
        {
            Impl *impl = static_cast<Impl *>(glfwGetWindowUserPointer(window));
            if (impl)
            {
                // if (ImGui::GetIO().WantCaptureMouse) return;

                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                impl->processMouseMove((float)xpos, (float)ypos);
            }
        }

        static void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
        {
            // ImGuiIO &io = ImGui::GetIO();
            // if (io.WantCaptureMouse) return; // 防止在 Imgui 窗口上滚轮时，场景也跟着缩放

            Impl *impl = static_cast<Impl *>(glfwGetWindowUserPointer(window));
            if (impl)
            {
                // if (ImGui::GetIO().WantCaptureMouse) return;
                impl->processMouseScroll((float)yoffset);
            }
        }

        void processMouseButton(int button, int action, float xpos, float ypos)
        {
            if (_cameraController == nullptr) return;
            _cameraController->processMouseButton(button, action, xpos, ypos);
        }

        void processMouseMove(float xpos, float ypos)
        {
            if (_cameraController == nullptr) return;
            _cameraController->processMouseMove(xpos, ypos);
        }

        void processMouseScroll(float yoffset)
        {
            if (_cameraController == nullptr) return;
            _cameraController->processMouseScroll(yoffset);
        }

    public:
        GLFWwindow *_window;
        SharedPtr<CameraController> _cameraController;

        unsigned int _width;
        unsigned int _height;

        float lastFrame{0.0f};
        float deltaTime{0.0f};
    };

    Window::Window() : _impl(new Impl)
    {
    }

    Window::~Window()
    {
        if (_impl != nullptr)
        {
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
        return _impl->deal();
    }

    void Window::setCameraController(SharedPtr<CameraController> cmaeraController)
    {
        _impl->_cameraController = cmaeraController;
        if (cmaeraController == nullptr) return;
        if (cmaeraController->enableCursor())
            glfwSetInputMode(_impl->_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(_impl->_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // void Window::exec()
    //{
    //     if (_renderer == nullptr) return;
    //     while (!glfwWindowShouldClose(_window))
    //     {
    //         glEnable(GL_DEPTH_TEST);
    //         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //         float currentFrame = static_cast<float>(glfwGetTime());
    //         deltaTime = currentFrame - lastFrame;
    //         lastFrame = currentFrame;

    //        _renderer->update(deltaTime);

    //        _renderer->draw();

    //        /*input*/
    //        // CAMERA->processInput(window, deltaTime);

    //        /* Swap front and back buffers */
    //        glfwSwapBuffers(_window);

    //        /* Poll for and process events */
    //        glfwPollEvents();

    //        // CalculateFrameRate();
    //    };
    //}

} // namespace EgLab