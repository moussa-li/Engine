#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Print.h"
#include "Log.h"


Camera::Camera(unsigned int width, unsigned int height, Eigen::Vector3f position /*= Eigen::Vector3f(0.0f, 0.0f, 3.0f)*/, Eigen::Vector3f up /*= Eigen::Vector3f(0.0f, 1.0f, 0.0f)*/, float yaw /*= YAW*/, float pitch /*= PITCH*/)
:lastX(width / 2.0f), lastY(height / 2.0f), Front(Eigen::Vector3f(0.0f, 0.0f, -3.0f)), Position(position), WorldUp(up),
		Yaw(yaw), Pitch(pitch), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
		updateCameraVectors();

}

void Camera::updateCameraVectors()
	{
		Eigen::Vector3f front;
		front.x() = cos(radians(Yaw)) * cos(radians(Pitch));
		front.y() = sin(radians(Pitch));
		front.z() = sin(radians(Yaw)) * cos(radians(Pitch));
		front.normalize();
        Front = front;

		Right = Front.cross(WorldUp);
        Right.normalize();

        Up = Right.cross(Front);
        Up.normalize();
	}

Eigen::Matrix4f Camera::perspective(float fovy, float aspect, float zNear, float zFar)
{
    float const tanHalfFovy = tan(fovy / 2);

    Eigen::Matrix4f Result = Eigen::Matrix4f::Zero();
    Result(0, 0) = 1 / (aspect * tanHalfFovy);
    Result(1, 1) = 1 / (tanHalfFovy);
    Result(2, 2) = -(zFar + zNear) / (zFar - zNear);
    Result(3, 2) = -1.0f;
    Result(2, 3) = -(2 * zFar * zNear) / (zFar - zNear);
    return Result;
}

Eigen::Matrix4f Camera::ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    Eigen::Matrix4f  Result = Eigen::Matrix4f::Identity();
    Result(0, 0) = 2 / (right - left);
    Result(1, 1) = 2 / (top - bottom);
    Result(2, 2) = -2 / (zFar - zNear);
    Result(0, 3) = -(right + left) / (right - left);
    Result(1, 3) = -(top + bottom) / (top - bottom);
    Result(2, 3) = -(zFar + zNear) / (zFar - zNear);
    return Result;

}

void Camera::lookAt(Eigen::Vector3f const& position, Eigen::Vector3f const& front, Eigen::Vector3f const& up)
{
    Position = position;
    Front = front;
    Up = up;
}

Eigen::Matrix4f Camera::View()
{
    Eigen::Vector3f f(Front );
    Eigen::Vector3f s(f.cross(Up));
    Eigen::Vector3f  u(s.cross(f));
    f.normalize();
    s.normalize();
    u.normalize();


    Eigen::Matrix4f  Result = Eigen::Matrix4f::Identity();
    Result(0, 0) = s.x();
    Result(0, 1) = s.y();
    Result(0, 2) = s.z();
    Result(1, 0) = u.x();
    Result(1, 1) = u.y();
    Result(1, 2) = u.z();
    Result(2, 0) = -f.x();
    Result(2, 1) = -f.y();
    Result(2, 2) = -f.z();
    Result(0, 3) = -s.dot(Position);
    Result(1, 3) = -u.dot(Position);
    Result(2, 3) = f.dot(Position);
    Result(3, 3) = 1;
    return Result;
}

inline float Camera::radians(float degrees) const
{
    return degrees * static_cast<float>(0.01745329251994329576923690768489);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == Camera_Movement::FORWARD)
		Position += Front * velocity;
	else if (direction == Camera_Movement::BACKWARD)
		Position -= Front * velocity;
	else if (direction == Camera_Movement::LEFT)
		Position -= Right * velocity;
	else if (direction == Camera_Movement::RIGHT)
		Position += Right * velocity;
	else
		ASSERT(0);
}
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
	Zoom -= (float)yoffset;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
}



void Camera::processInput(GLFWwindow* window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		ProcessKeyboard(RIGHT, deltaTime);
	}
}
