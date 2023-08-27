#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>

#include "Renderer/Renderer.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexBufferLayout.h"
#include "Renderer/Reactor.h"

#include "Renderer/Entity.h"
#include "Renderer/DirectionalLight.h"

#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"

#include "Renderer/Print.h"
#include "Renderer/Log.h"


const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

/* my renderer only have one camera ,maybe need more later , let it be */
Camera camera(WIDTH, HEIGHT, Eigen::Vector3f(0.0f, 0.0f, 1.0f));


// FPS output
void CalculateFrameRate()
{
    static int framesPerSecond    = 0;       // This will store our fps
    static float lastTime   = 0.0f;       // This will hold the time from the last frame
    float currentTime = static_cast<float>(glfwGetTime());

    ++framesPerSecond;
    if( currentTime - lastTime > 1.0f )
    {
        lastTime = currentTime;
       printf("FPS : %d\r\n", framesPerSecond);
       printf("camera Position : %f %f %f\r\n", camera.Position.x(), camera.Position.y(), camera.Position.z());
       printf("camera Front : %f %f %f\r\n", camera.Front.x(), camera.Front.y(), camera.Front.z());
       printf("camera Up : %f %f %f\r\n", camera.Up.x(), camera.Up.y(), camera.Up.z());
       printf("camera Right : %f %f %f\r\n", camera.Right.x(), camera.Right.y(), camera.Right.z());
       printf("camera WorldUp : %f %f %f\r\n", camera.WorldUp.x(), camera.WorldUp.y(), camera.WorldUp.z());
       printf("\033[1A");
       printf("\033[1A");
       printf("\033[1A");
       printf("\033[1A");
       printf("\033[1A");
       printf("\033[1A");
       framesPerSecond = 0;
    }
}




int main(int argc, char **argv)
 {
    /* interesting things about glfw , it needs a brace to make it close normal */
    {
        GLFWwindow* window;

        /* Initialize the library */
        if (!glfwInit())
            return -1;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);

        /* set user control */
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


        

        if (!window)
        {
            glfwTerminate();
            return -1;
        }

        /* Make the window's context current */

        /* disable cursor */
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

        /* user operator */
        glfwMakeContextCurrent(window);

        /* v-Sync */
        //glfwSwapInterval(1);

        if (glewInit() != GLEW_OK) {
            std::cout << "Error" << std::endl;
        }

        std::cout << glGetString(GL_VERSION) << std::endl;


        Entity entity(Eigen::Vector3f(0,0,0),Eigen::Vector3f(0,0,0),Eigen::Vector3f(0.1,0.1,0.1),"res/shaders/Basic.shader");
        entity.Load("res/models/nanosuit/nanosuit.obj");

        Entity entity2(Eigen::Vector3f(0,0,5),Eigen::Vector3f(0,0,0),Eigen::Vector3f(1,1,1),"res/shaders/Basic.shader");
        entity2.Load("res/models/bunny.obj");


        DirectionalLight light(
            Eigen::Vector3f(0.0f, -3.0f, 0.0f), //m_Direction
            Eigen::Vector3f(0.2f, 0.2f, 0.2f),  //m_Ambient
            Eigen::Vector3f(0.8f, 0.8f, 0.8f),  //m_Diffuse
            Eigen::Vector3f(1.0f, 1.0f, 1.0f)   //m_Specular
        );
        

        Renderer renderer(&camera);
        renderer.Insert_Entity(&entity);
        renderer.Insert_Entity(&entity2);
        renderer.Insert_Light(&light);
        renderer.SetBackGroundColor(Eigen::Vector4f(1, 1, 1, 1));

        // only render line (just mesh)
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            // depth test
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderer.Draw();

            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            /*input*/
            camera.processInput(window, deltaTime);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();

            CalculateFrameRate();
        }
    }
    glfwTerminate();
    return 0;
}