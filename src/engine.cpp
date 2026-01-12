#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "vulkan/vulkan.hpp"
#include <pch.hpp>
#include <cstring>
#include <glm/fwd.hpp>
#include <vkMaze/engine.hpp>
#include <cstdint>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

std::vector<Vertex> VulkanEngine::getVertices() {
  throw std::runtime_error("getVertices() is not overridden!");
}

std::vector<uint16_t> VulkanEngine::getIndices() {
  throw std::runtime_error("getIndices() is not overridden!");
}

void VulkanEngine::drawScreen() {
}

void VulkanEngine::mouseMoved(float xoffset, float yoffset) {
}

void VulkanEngine::processInput(GLFWwindow *window) {
}

void VulkanEngine::updateCameraTransforms(GlobalUBO &ubo) {
}

void VulkanEngine::updateUBOData(MaterialUBO &ubo) {
}
