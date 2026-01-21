#include "vkMaze/Components/Window.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/VulkanEngine.hpp"
#include <GLFW/glfw3.h>

void Window::init(VulkanEngine &e) {
  std::cout << "this: " << this << std::endl;
  std::cout << "&engine member: " << &engine << std::endl;
  engine = &e;
  std::cout << "assigned engine" << std::endl;
}
void Window::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetCursorPosCallback(window, GLFWMouseCallback);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  Window *thisWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  thisWindow->framebufferResized = true;
}

void Window::GLFWMouseCallback(GLFWwindow *window, double xposIn, double yposIn) {
  Window *thisWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (thisWindow->firstMouse) {
    thisWindow->lastX = xpos;
    thisWindow->lastY = ypos;
    thisWindow->firstMouse = false;
  }

  float xoffset = xpos - thisWindow->lastX;
  float yoffset = ypos - thisWindow->lastY;

  thisWindow->lastX = xpos;
  thisWindow->lastY = ypos;

  thisWindow->engine->mouseMoved(xoffset, yoffset);
}
