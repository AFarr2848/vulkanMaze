#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <vector>
#include <iostream>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
  // camera Attributes
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  // euler Angles
  float Yaw;
  float Pitch;
  // camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  // constructor with vectors
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }
  // constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix() {
    updateCameraVectors();
    return glm::lookAt(Position, Position + Front, Up);
  }

  void lookAt(glm::vec3 pos) {
  }

  glm::vec3 moveCamera(Camera_Movement direction, float deltaTime) {
    glm::vec3 pos = glm::vec3(Position);

    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
      pos += Front * velocity;
    if (direction == BACKWARD)
      pos -= Front * velocity;
    if (direction == LEFT)
      pos -= Right * velocity;
    if (direction == RIGHT)
      pos += Right * velocity;
    if (direction == UP)
      pos.y += velocity;
    if (direction == DOWN)
      pos.y -= velocity;

    return pos;
  }

  // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
  void processKeyboard(Camera_Movement direction, float deltaTime) {
    Position = moveCamera(direction, deltaTime);
  }

  void moveWithCollision(Camera_Movement direction, float deltaTime, std::vector<float> bounds) {
    glm::vec3 newPos = moveCamera(direction, deltaTime);
    bool tooClose = false;

    for (int i = 0; i < bounds.size(); i += 4) {
      glm::vec2 p1 = glm::vec2(bounds.at(i), bounds.at(i + 1));
      glm::vec2 p2 = glm::vec2(bounds.at(i + 2), bounds.at(i + 3));
      std::cout << pointToSegmentDistance(glm::vec2(newPos.x, newPos.z), p1, p2) << std::endl;
      if (pointToSegmentDistance(glm::vec2(newPos.x, newPos.z), p1, p2) < 0.1f) {
        tooClose = true;
      }
    }
    if (!tooClose)
      Position = newPos;
  }

  // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
  void ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch -= yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (Pitch > 89.0f)
      Pitch = 89.0f;
    if (Pitch < -89.0f)
      Pitch = -89.0f;

    updateCameraVectors();
  }

  // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
  void ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
      Zoom = 1.0f;
    if (Zoom > 45.0f)
      Zoom = 45.0f;
  }

private:
  // calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors() {
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
  }

  float pointToSegmentDistance(glm::vec2 p, glm::vec2 a, glm::vec2 b) {
    glm::vec2 ab = b - a;
    glm::vec2 ap = p - a;

    float abLen2 = glm::dot(ab, ab); // squared length of AB
    if (abLen2 == 0.0f) {
      return glm::length(ap); // A and B are the same point
    }

    // Project AP onto AB to find t along AB
    float t = glm::dot(ap, ab) / abLen2;

    if (t < 0.0f)
      return glm::length(p - a); // before A
    if (t > 1.0f)
      return glm::length(p - b); // after B

    // perpendicular distance
    glm::vec2 projection = a + t * ab;
    return glm::length(p - projection);
  }
};
