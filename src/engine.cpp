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

#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>

vk::VertexInputBindingDescription Vertex::getBindingDescription() {
  return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
}

std::array<vk::VertexInputAttributeDescription, 1> Vertex::getAttributeDescriptions() {
  return {
      vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
  };
}

void VulkanEngine::run() {
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

std::vector<Vertex> VulkanEngine::getVertices() {
  throw std::runtime_error("getVertices() is not overridden!");
}

std::vector<uint16_t> VulkanEngine::getIndices() {
  throw std::runtime_error("getIndices() is not overridden!");
}

void VulkanEngine::drawScreen() {
}

void VulkanEngine::updateMaterialUniformBuffer(uint32_t currentImage) {
  MaterialUBO ubo{};
  updateUBO(ubo);
  memcpy(materialUBOMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanEngine::mouseMoved(float xoffset, float yoffset) {
}

void VulkanEngine::updateTransforms(GlobalUBO &ubo) {
}

void VulkanEngine::updateUBO(MaterialUBO &ubo) {
}

void VulkanEngine::updateGlobalUniformBuffer(uint32_t currentImage) {
  GlobalUBO ubo{};
  updateTransforms(ubo);

  memcpy(globalUBOMapped[currentImage], &ubo, sizeof(ubo));
}

std::vector<const char *> requiredDeviceExtension = {
    vk::KHRSwapchainExtensionName,
    vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName

};

void VulkanEngine::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetCursorPosCallback(window, GLFWMouseCallback);
}

void VulkanEngine::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto app = reinterpret_cast<VulkanEngine *>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
}

void VulkanEngine::GLFWMouseCallback(GLFWwindow *window, double xposIn, double yposIn) {
  VulkanEngine *engine = static_cast<VulkanEngine *>(glfwGetWindowUserPointer(window));
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (engine->firstMouse) {
    engine->lastX = xpos;
    engine->lastY = ypos;
    engine->firstMouse = false;
  }

  float xoffset = xpos - engine->lastX;
  float yoffset = ypos - engine->lastY;

  engine->lastX = xpos;
  engine->lastY = ypos;

  engine->mouseMoved(xoffset, yoffset);
}

void VulkanEngine::initVulkan() {
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
  createGlobalDescriptorSetLayout();
  createGraphicsPipeline();
  createCommandPool();
  std::printf("Creating vertex buffer...\n");
  createVertexBuffer();
  std::printf("Creating index buffer...\n");
  createIndexBuffer();
  std::printf("Creating uniform buffers...\n");
  createUniformBuffers();
  std::printf("Creating descriptor pool...\n");
  createDescriptorPool();
  std::printf("Creating descriptor sets...\n");
  createDescriptorSets();
  std::printf("Creating command buffers...\n");
  createCommandBuffers();
  std::printf("Creating sync objects...\n");
  createSyncObjects();
}

void VulkanEngine::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    drawFrame();
  }

  device.waitIdle();
}

void VulkanEngine::createGlobalDescriptorSetLayout() {
  vk::DescriptorSetLayoutBinding uboLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };
  vk::DescriptorSetLayoutBinding uboMaterialLayoutBinding{
      .binding = 1,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };

  std::vector<vk::DescriptorSetLayoutBinding> layouts = {uboLayoutBinding, uboMaterialLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .flags = {},
      .bindingCount = 2,
      .pBindings = layouts.data()

  };

  descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

void VulkanEngine::createDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
  std::printf("created layouts\n");

  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()

  };
  std::printf("allocInfo complete\n");

  descriptorSets.clear();
  descriptorSets = device.allocateDescriptorSets(allocInfo);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo camInfo{
        .buffer = globalUBOs[i],
        .offset = 0,
        .range = sizeof(GlobalUBO)

    };
    std::cout << "CamInfo complete" << std::endl;
    std::cout << materialUBOs.size() << std::endl;

    vk::DescriptorBufferInfo matInfo{
        .buffer = materialUBOs[i],
        .offset = 0,
        .range = sizeof(MaterialUBO)

    };
    std::cout << "matInfo complete" << std::endl;

    vk::WriteDescriptorSet camWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &camInfo

    };
    std::cout << "CamWrite complete" << std::endl;

    vk::WriteDescriptorSet matWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &matInfo

    };
    std::cout << "MatWrite complete" << std::endl;

    std::vector<vk::WriteDescriptorSet> writes = {camWrite, matWrite};

    std::printf("updating descriptor sets\n");
    device.updateDescriptorSets(writes, {});
  }
}

void VulkanEngine::createDescriptorPool() {
  vk::DescriptorPoolSize poolSize{
      .type = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT * 2

  };
  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize

  };

  descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

void VulkanEngine::createUniformBuffers() {
  globalUBOs.clear();
  globalUBOMemory.clear();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(GlobalUBO);
    std::cout << bufferSize << std::endl;
    vk::raii::Buffer buffer({});
    vk::raii::DeviceMemory bufferMem({});
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, buffer, bufferMem);
    globalUBOs.emplace_back(std::move(buffer));
    globalUBOMemory.emplace_back(std::move(bufferMem));
    globalUBOMapped.emplace_back(globalUBOMemory[i].mapMemory(0, bufferSize));
  }

  materialUBOs.clear();
  materialUBOMemory.clear();
  materialUBOMapped.clear();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(MaterialUBO);
    std::cout << bufferSize << std::endl;
    vk::raii::Buffer buffer({});
    vk::raii::DeviceMemory bufferMem({});
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, buffer, bufferMem);
    materialUBOs.emplace_back(std::move(buffer));
    materialUBOMemory.emplace_back(std::move(bufferMem));
    materialUBOMapped.emplace_back(materialUBOMemory[i].mapMemory(0, bufferSize));
  }
}

void VulkanEngine::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory) {
  vk::BufferCreateInfo bufferInfo{
      .size = size,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive};

  buffer = vk::raii::Buffer(device, bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),

  };
  bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
  buffer.bindMemory(*bufferMemory, 0);
}

void VulkanEngine::createVertexBuffer() {
  std::vector<Vertex> vertices = getVertices();
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  vk::raii::Buffer stagingBuffer = nullptr;
  vk::raii::DeviceMemory stagingBufferMemory = nullptr;
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

  void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(dataStaging, vertices.data(), bufferSize);
  stagingBufferMemory.unmapMemory();

  createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertexBuffer, vertexBufferMemory);

  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void VulkanEngine::createIndexBuffer() {
  std::vector<uint16_t> indices = getIndices();
  vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  vk::raii::Buffer stagingBuffer = nullptr;
  vk::raii::DeviceMemory stagingBufferMemory = nullptr;
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

  void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(dataStaging, indices.data(), bufferSize);
  stagingBufferMemory.unmapMemory();

  createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexBuffer, indexBufferMemory);

  copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

void VulkanEngine::copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1

  };

  vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());
  commandCopyBuffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
  commandCopyBuffer.end();
  queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
  queue.waitIdle();
}

uint32_t VulkanEngine::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanEngine::cleanupSwapChain() {
  swapChainImageViews.clear();
  swapChain = nullptr;
}

void VulkanEngine::cleanup() {
  glfwDestroyWindow(window);

  glfwTerminate();
}

void VulkanEngine::recreateSwapChain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }

  device.waitIdle();

  cleanupSwapChain();
  createSwapChain();
  createImageViews();
}

void VulkanEngine::createInstance() {
  constexpr vk::ApplicationInfo appInfo{.pApplicationName = "Hello Triangle",
                                        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                        .pEngineName = "No Engine",
                                        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                        .apiVersion = vk::ApiVersion14};

  // Get the required layers
  std::vector<char const *> requiredLayers;
  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  // Check if the required layers are supported by the Vulkan implementation.
  auto layerProperties = context.enumerateInstanceLayerProperties();
  for (auto const &requiredLayer : requiredLayers) {
    if (std::ranges::none_of(layerProperties,
                             [requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; })) {
      throw std::runtime_error("Required layer not supported: " + std::string(requiredLayer));
    }
  }

  // Get the required extensions.
  auto requiredExtensions = getRequiredExtensions();

  // Check if the required extensions are supported by the Vulkan implementation.
  auto extensionProperties = context.enumerateInstanceExtensionProperties();
  for (auto const &requiredExtension : requiredExtensions) {
    if (std::ranges::none_of(extensionProperties,
                             [requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; })) {
      throw std::runtime_error("Required extension not supported: " + std::string(requiredExtension));
    }
  }

  vk::InstanceCreateInfo createInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};
  instance = vk::raii::Instance(context, createInfo);
}

void VulkanEngine::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;

  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
      .messageSeverity = severityFlags,
      .messageType = messageTypeFlags,
      .pfnUserCallback = &debugCallback};
  debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void VulkanEngine::createSurface() {
  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
    throw std::runtime_error("failed to create window surface!");
  }
  surface = vk::raii::SurfaceKHR(instance, _surface);
}

void VulkanEngine::pickPhysicalDevice() {
  std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
  const auto devIter = std::ranges::find_if(
      devices,
      [&](auto const &device) {
        // Check if the device supports the Vulkan 1.3 API version
        bool supportsVulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;

        // Check if any of the queue families support graphics operations
        auto queueFamilies = device.getQueueFamilyProperties();
        bool supportsGraphics =
            std::ranges::any_of(queueFamilies, [](auto const &qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

        // Check if all required device extensions are available
        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions =
            std::ranges::all_of(requiredDeviceExtension,
                                [&availableDeviceExtensions](auto const &requiredDeviceExtension) {
                                  return std::ranges::any_of(availableDeviceExtensions,
                                                             [requiredDeviceExtension](auto const &availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
                                });

        auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                     vk::PhysicalDeviceVulkan11Features,
                                                     vk::PhysicalDeviceVulkan13Features,
                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                        features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                                        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
      });
  if (devIter != devices.end()) {
    physicalDevice = *devIter;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

void VulkanEngine::createLogicalDevice() {
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

  // get the first index into queueFamilyProperties which supports both graphics and present
  for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
    if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
        physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
      // found a queue family that supports both graphics and present
      queueIndex = qfpIndex;
      break;
    }
  }
  if (queueIndex == ~0) {
    throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
  }

  // query for Vulkan 1.3 features
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain = {
          {},                                                   // vk::PhysicalDeviceFeatures2
          {.shaderDrawParameters = true},                       // vk::PhysicalDeviceVulkan11Features
          {.synchronization2 = true, .dynamicRendering = true}, // vk::PhysicalDeviceVulkan13Features
          {.extendedDynamicState = true}                        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
      };

  // create a Device
  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
  vk::DeviceCreateInfo deviceCreateInfo{.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                        .queueCreateInfoCount = 1,
                                        .pQueueCreateInfos = &deviceQueueCreateInfo,
                                        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
                                        .ppEnabledExtensionNames = requiredDeviceExtension.data()};

  device = vk::raii::Device(physicalDevice, deviceCreateInfo);
  queue = vk::raii::Queue(device, queueIndex, 0);
}

void VulkanEngine::createSwapChain() {
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
  swapChainExtent = chooseSwapExtent(surfaceCapabilities);
  swapChainSurfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface = *surface,
                                                 .minImageCount = chooseSwapMinImageCount(surfaceCapabilities),
                                                 .imageFormat = swapChainSurfaceFormat.format,
                                                 .imageColorSpace = swapChainSurfaceFormat.colorSpace,
                                                 .imageExtent = swapChainExtent,
                                                 .imageArrayLayers = 1,
                                                 .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                                                 .imageSharingMode = vk::SharingMode::eExclusive,
                                                 .preTransform = surfaceCapabilities.currentTransform,
                                                 .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                 .presentMode = chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface)),
                                                 .clipped = true};

  swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
  swapChainImages = swapChain.getImages();
}

void VulkanEngine::createImageViews() {
  assert(swapChainImageViews.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{.viewType = vk::ImageViewType::e2D, .format = swapChainSurfaceFormat.format, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto image : swapChainImages) {
    imageViewCreateInfo.image = image;
    swapChainImageViews.emplace_back(device, imageViewCreateInfo);
  }
}

void VulkanEngine::createGraphicsPipeline() {
  vk::raii::ShaderModule shaderModule = createShaderModule(readFile("build/shaders/shader.spv"));

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount = attributeDescriptions.size(),
      .pVertexAttributeDescriptions = attributeDescriptions.data()

  };
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasSlopeFactor = 1.0f,
      .lineWidth = 1.0f

  };

  vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{.blendEnable = vk::False,
                                                             .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

  std::vector dynamicStates = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = 1,
      .pSetLayouts = &*descriptorSetLayout,
      .pushConstantRangeCount = 0

  };

  pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

  vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
      {.stageCount = 2,
       .pStages = shaderStages,
       .pVertexInputState = &vertexInputInfo,
       .pInputAssemblyState = &inputAssembly,
       .pViewportState = &viewportState,
       .pRasterizationState = &rasterizer,
       .pMultisampleState = &multisampling,
       .pColorBlendState = &colorBlending,
       .pDynamicState = &dynamicState,
       .layout = pipelineLayout,
       .renderPass = nullptr},
      {.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format}};

  graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

void VulkanEngine::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                     .queueFamilyIndex = queueIndex};
  commandPool = vk::raii::CommandPool(device, poolInfo);
}

void VulkanEngine::createCommandBuffers() {
  commandBuffers.clear();
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
  commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

void VulkanEngine::recordCommandBuffer(uint32_t imageIndex) {
  std::vector<uint16_t> indices = getIndices();
  commandBuffers[currentFrame].begin({});
  // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
  transition_image_layout(
      imageIndex,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},                                                 // srcAccessMask (no need to wait for previous operations)
      vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eColorAttachmentOutput  // dstStage
  );
  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = swapChainImageViews[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor};
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo};
  commandBuffers[currentFrame].beginRendering(renderingInfo);
  commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
  commandBuffers[currentFrame].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
  commandBuffers[currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

  commandBuffers[currentFrame].bindVertexBuffers(0, *vertexBuffer, {0});
  commandBuffers[currentFrame].bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);

  commandBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[currentFrame], nullptr);
  commandBuffers[currentFrame].drawIndexed(indices.size(), 1, 0, 0, 0);
  drawScreen();
  commandBuffers[currentFrame].endRendering();
  // After rendering, transition the swapchain image to PRESENT_SRC
  transition_image_layout(
      imageIndex,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
      {},                                                 // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
  );
  commandBuffers[currentFrame].end();
}

void VulkanEngine::transition_image_layout(
    uint32_t imageIndex,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = src_stage_mask,
      .srcAccessMask = src_access_mask,
      .dstStageMask = dst_stage_mask,
      .dstAccessMask = dst_access_mask,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = swapChainImages[imageIndex],
      .subresourceRange = {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1}};
  vk::DependencyInfo dependency_info = {
      .dependencyFlags = {},
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier};
  commandBuffers[currentFrame].pipelineBarrier2(dependency_info);
}

void VulkanEngine::createSyncObjects() {
  presentCompleteSemaphore.clear();
  renderFinishedSemaphore.clear();
  inFlightFences.clear();

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    presentCompleteSemaphore.emplace_back(device, vk::SemaphoreCreateInfo());
    renderFinishedSemaphore.emplace_back(device, vk::SemaphoreCreateInfo());
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  }
}

void VulkanEngine::drawFrame() {
  while (vk::Result::eTimeout == device.waitForFences(*inFlightFences[currentFrame], vk::True, UINT64_MAX))
    ;
  auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore[semaphoreIndex], nullptr);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    return;
  }
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  device.resetFences(*inFlightFences[currentFrame]);
  commandBuffers[currentFrame].reset();
  recordCommandBuffer(imageIndex);
  updateGlobalUniformBuffer(currentFrame);
  updateMaterialUniformBuffer(currentFrame);

  vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*presentCompleteSemaphore[semaphoreIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*commandBuffers[currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*renderFinishedSemaphore[imageIndex]

  };
  queue.submit(submitInfo, *inFlightFences[currentFrame]);

  try {
    const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1, .pWaitSemaphores = &*renderFinishedSemaphore[imageIndex], .swapchainCount = 1, .pSwapchains = &*swapChain, .pImageIndices = &imageIndex};
    result = queue.presentKHR(presentInfoKHR);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
      framebufferResized = false;
      recreateSwapChain();
    } else if (result != vk::Result::eSuccess) {
      throw std::runtime_error("failed to present swap chain image!");
    }
  } catch (const vk::SystemError &e) {
    if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR)) {
      recreateSwapChain();
      return;
    } else {
      throw;
    }
  }
  semaphoreIndex = (semaphoreIndex + 1) % presentCompleteSemaphore.size();
  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

[[nodiscard]] vk::raii::ShaderModule VulkanEngine::createShaderModule(const std::vector<char> &code) const {
  vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())};
  vk::raii::ShaderModule shaderModule{device, createInfo};

  return shaderModule;
}

uint32_t VulkanEngine::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities) {
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

vk::SurfaceFormatKHR VulkanEngine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  assert(!availableFormats.empty());
  const auto formatIt = std::ranges::find_if(
      availableFormats,
      [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR VulkanEngine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
  return std::ranges::any_of(availablePresentModes,
                             [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; })
             ? vk::PresentModeKHR::eMailbox
             : vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanEngine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != 0xFFFFFFFF) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {
      std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
      std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
}

std::vector<const char *> VulkanEngine::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanEngine::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
  if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
    std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
  }

  return vk::False;
}

std::vector<char> VulkanEngine::readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }
  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();
  return buffer;
};
