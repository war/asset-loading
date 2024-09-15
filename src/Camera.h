#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

enum Camera_Modifiers {
    CTRL,
};

const float YAW = -90.0f;
const float PITCH = -35.0f;
const float SPEED_SLOW = 3.0f;
const float SPEED_FAST = 15.f;
const float SPEED = SPEED_FAST;
const float SENSITIVITY = 0.1f;

const float FOV_MIN = 50.0f;
const float FOV_MAX = 85.0f;
const float FOV = 80;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Fov;
    float FovRads;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    glm::mat4 GetViewMatrix();
		glm::vec3 GetPosition() const {	return Position;	}
		
		glm::vec3 GetFront() const { return Front; }
		glm::vec3 GetUp() const { return Up; }
		glm::vec3 GetRight() const { return Right; }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

private:
    void updateCameraVectors();
};
