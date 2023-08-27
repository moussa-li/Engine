#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <eigen3/Eigen/Eigen>

#include <iostream>
#include <vector>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW			= -90.0f;
const float PITCH		= 0.0f;
const float SPEED		= 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM		= 45.0f;



class Camera
{
public:
	Eigen::Vector3f Position;
	Eigen::Vector3f Front;
	Eigen::Vector3f Up;
	Eigen::Vector3f Right;
	Eigen::Vector3f WorldUp;

	float Yaw;
	float Pitch;

	

	float lastX;
	float lastY;
	bool firstMouse = true;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(unsigned int width, unsigned int height, Eigen::Vector3f position = Eigen::Vector3f(0.0f, 0.0f, 3.0f), Eigen::Vector3f up = Eigen::Vector3f(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	
    Eigen::Matrix4f perspective(float fovy, float aspect, float zNear, float zFar);
    
    Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float zNear, float zFar);

    // set camera parameter
    void lookAt(Eigen::Vector3f const& position, Eigen::Vector3f const& front, Eigen::Vector3f const& up);

    Eigen::Matrix4f View();


    inline float radians(float degrees) const;

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch);
	
	void ProcessMouseScroll(float yoffset);

	void processInput(GLFWwindow* window, float deltaTime);

private:
	void updateCameraVectors();
};
