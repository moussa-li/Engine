#include "Reactor.h"

#include "Log.h"
extern const unsigned int WIDTH;
extern const unsigned int HEIGHT;

extern Camera *CAMERA;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (CAMERA == nullptr)
    {
        Log::Log(Log::Error, "Please bind camera !");
        return;
    }
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (CAMERA->firstMouse)
	{
		CAMERA->lastX = xpos;
		CAMERA->lastY = ypos;
		CAMERA->firstMouse = false;
	}

	float xoffset = xpos - CAMERA->lastX;
	float yoffset = CAMERA->lastY - ypos; // reversed since y-coordinates go from bottom to top

	CAMERA->lastX = xpos;
	CAMERA->lastY = ypos;

    //Log::Log(Log::Debug, "xoffset : " + std::to_string(xoffset) + " yoffset: " + std::to_string(yoffset));
    
	CAMERA->ProcessMouseMovement(xoffset, yoffset, true);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    CAMERA->Zoom -= static_cast<float>(yoffset);
	if (CAMERA->Zoom < 1.0f)
		CAMERA->Zoom = 1.0f;
	if (CAMERA->Zoom > 45.0f)
		CAMERA->Zoom = 45.0f;
}


