#include "RenderLib.h"

unsigned int WIDTH = 800;
unsigned int HEIGHT = 600;
Camera *CAMERA;
GLFWwindow* window;
float deltaTime = 0.0f;
float lastFrame;

bool Windows_Init(unsigned int width, unsigned int height)
{
    WIDTH = width;
    HEIGHT = height;

    deltaTime = 0.0f;
    lastFrame = 0.0f;



    /* Initialize the library */
    if (!glfwInit())
        return 0;

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
        return 0;
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

    return 1;
}

void Bind_Camera(Camera* camera)
{
    CAMERA = camera;
}

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
       printf("FPS : %d\r", framesPerSecond);
       framesPerSecond = 0;
    }
}

void Windows_Start(Renderer* renderer)
{
/* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {
       // depth test
       glEnable(GL_DEPTH_TEST);
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       renderer->Update();

       renderer->Draw();

       float currentFrame = static_cast<float>(glfwGetTime());
       deltaTime = currentFrame - lastFrame;
       lastFrame = currentFrame;

       /*input*/
       CAMERA->processInput(window, deltaTime);

       /* Swap front and back buffers */
       glfwSwapBuffers(window);

       /* Poll for and process events */
       glfwPollEvents();

       CalculateFrameRate();
   }
}

void Windows_Terminate()
{
    glfwTerminate();
}
