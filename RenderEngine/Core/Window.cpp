#include "Window.hpp"

#include "Camera.hpp"
#include "Common/Log.hpp"
#include "GLFW/glfw3.h"
#include "Renderer.hpp"

namespace EgLab
{

    class Window::Impl
    {
    public:
        void init()
        {
            if (!glfwInit()) return;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

            /* user operator */
            glfwMakeContextCurrent(_window);

            /* v-Sync */
            // glfwSwapInterval(1);

            if (glewInit() != GLEW_OK)
            {
                LOG(ERROR) << "Error";
            }

            LOG(INFO) << glGetString(GL_VERSION);
        }

        bool shouldClose() const
        {
            if (_window == nullptr) return true;
            return glfwWindowShouldClose(_window);
        }

    public:
        GLFWwindow *_window;

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