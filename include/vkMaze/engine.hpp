#pragma once
#include <pch.hpp>
#include <glm/fwd.hpp>
#include <cstdint>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct GlobalUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct MaterialUBO {
  std::array<glm::mat4, 10000> transforms;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;

  static vk::VertexInputBindingDescription getBindingDescription();
  static std::array<vk::VertexInputAttributeDescription, 1> getAttributeDescriptions();
};
class VulkanEngine {
public:
  bool framebufferResized = false;
  uint32_t currentFrame = 0;
  void run();

protected:
  virtual std::vector<Vertex> getVertices();
  virtual std::vector<uint16_t> getIndices();
  virtual void drawScreen();
  virtual void mouseMoved(float xoffset, float yoffset);
  virtual void updateTransforms(GlobalUBO &ubo);
  virtual void updateUBO(MaterialUBO &ubo);

  std::vector<vk::raii::CommandBuffer> commandBuffers;
  std::vector<void *> materialUBOMapped;
  vk::Extent2D swapChainExtent;

private:
  GLFWwindow *window = nullptr;
  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  uint32_t queueIndex = ~0;
  vk::raii::Queue queue = nullptr;
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  std::vector<vk::raii::ImageView> swapChainImageViews;

  vk::raii::DescriptorPool descriptorPool = nullptr;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;

  vk::raii::CommandPool commandPool = nullptr;

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  std::vector<vk::raii::Buffer> globalUBOs;
  std::vector<vk::raii::DeviceMemory> globalUBOMemory;
  std::vector<void *> globalUBOMapped;

  std::vector<vk::raii::Buffer> materialUBOs;
  std::vector<vk::raii::DeviceMemory> materialUBOMemory;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphore;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphore;
  std::vector<vk::raii::Fence> inFlightFences;
  uint32_t semaphoreIndex = 0;

  bool firstMouse = false;
  int lastX, lastY;

  std::vector<const char *> requiredDeviceExtension = {
      vk::KHRSwapchainExtensionName,
      vk::KHRSpirv14ExtensionName,
      vk::KHRSynchronization2ExtensionName,
      vk::KHRCreateRenderpass2ExtensionName

  };

  void initWindow();

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

  static void GLFWMouseCallback(GLFWwindow *window, double xposIn, double yposIn);

  void initVulkan();

  void mainLoop();
  void createGlobalDescriptorSetLayout();

  void createDescriptorSets();
  void createDescriptorPool();
  void createUniformBuffers();
  void createVertexBuffer();

  void updateMaterialUniformBuffer(uint32_t currentImage);
  void updateGlobalUniformBuffer(uint32_t currentImage);

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory);

  void createIndexBuffer();

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

  void cleanupSwapChain();

  void cleanup();

  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);

  void recreateSwapChain();

  void createInstance();

  void setupDebugMessenger();

  void createSurface();

  void pickPhysicalDevice();

  void createLogicalDevice();

  void createSwapChain();

  void createImageViews();

  void createGraphicsPipeline();

  void createCommandPool();

  void createCommandBuffers();

  void recordCommandBuffer(uint32_t imageIndex);

  void transition_image_layout(
      uint32_t imageIndex,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      vk::AccessFlags2 src_access_mask,
      vk::AccessFlags2 dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask);

  void createSyncObjects();

  void drawFrame();

  [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

  static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

  static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

  static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  std::vector<const char *> getRequiredExtensions();

  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);

  static std::vector<char> readFile(const std::string &filename);
};
