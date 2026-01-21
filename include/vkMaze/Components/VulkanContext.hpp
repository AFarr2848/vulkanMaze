#pragma once

class Window;

class VulkanContext {
public:
  void init(Window &win) {
    this->win = &win;
  }

  void cleanup();
  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device device = nullptr;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  uint32_t queueIndex = ~0;

  void createInstance();
  void setupDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSurface();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

private:
  Window *win = nullptr;

  vk::raii::Context context;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  std::vector<const char *> getRequiredExtensions();
  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
};
