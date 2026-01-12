#pragma once
#include "vkMaze/Vertex.hpp"
#include "vkMaze/UBOs.hpp"
#include "vkMaze/EngineConfig.hpp"
#include "vkMaze/Window.hpp"
#include <pch.hpp>
#include <cstdint>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.

class VulkanEngine {
public:
  void run();
  Window *window;

protected:
  virtual std::vector<Vertex> getVertices();
  virtual std::vector<uint16_t> getIndices();
  virtual void drawScreen();
  virtual void mouseMoved(float xoffset, float yoffset);
  virtual void updateCameraTransforms(GlobalUBO &ubo);
  virtual void updateUBOData(MaterialUBO &ubo);
  virtual void processInput(GLFWwindow *window);

  vk::Extent2D swapChainExtent;

private:
  uint32_t instanceCount;

  void initVulkan();

  void mainLoop();
  void createGlobalDescriptorSetLayout();
  ;
  void createDescriptorSets();
  void createDescriptorPool();
  void createUniformBuffers();
  void createVertexBuffer();

  void updateMaterialUniformBuffer(uint32_t currentImage);
  void updateGlobalUniformBuffer(uint32_t currentImage);
  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory);
  void createDepthResources();

  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
  vk::Format findDepthFormat();

  void createIndexBuffer();

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

  void cleanupSwapChain();

  void cleanup();

  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);

  void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory);

  vk::raii::ImageView createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlagBits flags);

  void recreateSwapChain();

  void createSurface();

  void createSwapChain();

  void createImageViews();

  void createGraphicsPipeline();

  void createCommandPool();

  void createCommandBuffers();

  void recordCommandBuffer(uint32_t imageIndex);

  void transition_image_layout(
      vk::Image image,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      vk::AccessFlags2 src_access_mask,
      vk::AccessFlags2 dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask,
      vk::ImageAspectFlags image_aspect_flags);

  void createSyncObjects();

  void drawFrame();

  void keepTime();

  [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

  static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

  static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

  static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  static std::vector<char> readFile(const std::string &filename);
};
