#pragma once
#include <GLFW/glfw3.h>
#include <iostream>

class VulkanEngine;

class Window {
public:
  void init(VulkanEngine &e);

  GLFWwindow *window;
  bool framebufferResized = false;

  void initWindow();
  void pollEvents();
  bool shouldClose() const;
  void mouseMoved(float, float);

private:
  bool firstMouse = true;
  int lastX;
  int lastY;
  VulkanEngine *engine = nullptr;

  static void framebufferResizeCallback(GLFWwindow *, int, int);
  static void GLFWMouseCallback(GLFWwindow *, double, double);
};
