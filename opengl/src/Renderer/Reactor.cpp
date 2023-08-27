#include "Reactor.h"

#include "Log.h"
extern const unsigned int WIDTH;
extern const unsigned int HEIGHT;

extern Camera camera;

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
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

    //cout << xpos << ypos << endl;
	if (camera.firstMouse)
	{
		camera.lastX = xpos;
		camera.lastY = ypos;
		camera.firstMouse = false;
	}

	float xoffset = xpos - camera.lastX;
	float yoffset = camera.lastY - ypos; // reversed since y-coordinates go from bottom to top

	camera.lastX = xpos;
	camera.lastY = ypos;
    
	camera.ProcessMouseMovement(xoffset, yoffset, true);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.Zoom -= static_cast<float>(yoffset);
	if (camera.Zoom < 1.0f)
		camera.Zoom = 1.0f;
	if (camera.Zoom > 45.0f)
		camera.Zoom = 45.0f;
}


