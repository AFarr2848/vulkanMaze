#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/VulkanEngine.hpp"
#include "vkMaze/Components/Window.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/FrameData.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Objects/Vertex.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include <iostream>

vk::VertexInputBindingDescription Vertex::getBindingDescription() {
  return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
  return {
      vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
      vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
      vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)),
  };
}

void VulkanEngine::run() {
  win->init(*this);
  std::cout << "Window init complete" << std::endl;
  cxt->init(*win);
  std::cout << "Context init complete" << std::endl;
  swp->init(*cxt, *win, *img);
  std::cout << "Swap init complete" << std::endl;
  frames->init(*cxt, *swp, *this);
  std::cout << "Frames init complete" << std::endl;
  dsc->init(*cxt, *buf);
  std::cout << "Descriptors init complete" << std::endl;
  buf->init(*this, *cxt, *frames);
  std::cout << "Buffer init complete" << std::endl;
  img->init(*cxt, *swp, *frames);
  std::cout << "Image init complete" << std::endl;

  win->initWindow();
  std::cout << "Window created" << std::endl;

  initVulkan();
  std::cout << "Vulkan init complete" << std::endl;
  mainLoop();
  cleanup();
}

void VulkanEngine::mainLoop() {
  while (!glfwWindowShouldClose(win->window)) {
    glfwPollEvents();
    keepTime();
    processInput(win->window);
    drawFrame();
  }
  std::cout << "should close" << std::endl;

  cxt->device.waitIdle();
}

void VulkanEngine::updateGlobalUniformBuffer(uint32_t currentImage) {
  GlobalUBO ubo{};
  updateCameraTransforms(ubo);

  memcpy(buf->globalUBOMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanEngine::updateStorageBuffer(uint32_t currentImage) {
  std::vector<SSBOLight> lights;
  updateLights(lights);

  memcpy(buf->SSBOsMapped[currentImage], lights.data(), lights.size() * sizeof(SSBOLight));
}

void VulkanEngine::drawFrame() {
  while (vk::Result::eTimeout == cxt->device.waitForFences(*frames->inFlightFences[currentFrame], vk::True, UINT64_MAX))
    ;
  auto [result, imageIndex] = swp->swapChain.acquireNextImage(UINT64_MAX, *frames->presentCompleteSemaphore[semaphoreIndex], nullptr);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    swp->recreateSwapChain();
    return;
  }
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  cxt->device.resetFences(*frames->inFlightFences[currentFrame]);
  frames->commandBuffers[currentFrame].reset();
  recordCommandBuffer(imageIndex);
  updateGlobalUniformBuffer(currentFrame);
  updateStorageBuffer(currentFrame);

  vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*frames->presentCompleteSemaphore[semaphoreIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*frames->commandBuffers[currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*frames->renderFinishedSemaphore[imageIndex]

  };
  cxt->graphicsQueue.submit(submitInfo, *frames->inFlightFences[currentFrame]);

  try {
    const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1, .pWaitSemaphores = &*frames->renderFinishedSemaphore[imageIndex], .swapchainCount = 1, .pSwapchains = &*swp->swapChain, .pImageIndices = &imageIndex};
    result = cxt->graphicsQueue.presentKHR(presentInfoKHR);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || win->framebufferResized) {
      win->framebufferResized = false;
      std::cout << "FRAMEBUFFER RESIZED" << std::endl;
      swp->recreateSwapChain();
    } else if (result != vk::Result::eSuccess) {
      throw std::runtime_error("failed to present swap chain image!");
    }
  } catch (const vk::SystemError &e) {
    if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR)) {
      swp->recreateSwapChain();
      return;
    } else {
      throw;
    }
  }
  semaphoreIndex = (semaphoreIndex + 1) % frames->presentCompleteSemaphore.size();
  frames->currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  currentFrame = frames->currentFrame;
}

void VulkanEngine::recordCommandBuffer(uint32_t imageIndex) {
  std::vector<uint32_t> indices = getIndices();
  frames->commandBuffers[currentFrame].begin({});
  // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
  img->transition_image_layout(
      swp->swapChainImages[imageIndex],
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},                                                 // srcAccessMask (no need to wait for previous operations)
      vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::ImageAspectFlagBits::eColor // dstStage
  );
  img->transition_image_layout(
      *img->depthImage,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthAttachmentOptimal,
      vk::AccessFlagBits2::eDepthStencilAttachmentWrite,                                                // dstAccessMask
      vk::AccessFlagBits2::eDepthStencilAttachmentWrite,                                                // dstAccessMask
      vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, // dstAccessMask
      vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, // dstAccessMask
      vk::ImageAspectFlagBits::eDepth                                                                   // dstStage
  );
  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = swp->swapChainImageViews[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor

  };
  vk::RenderingAttachmentInfo depthAttachmentInfo = {
      .imageView = img->depthImageView,
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = clearDepth

  };
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = swp->swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo

  };
  frames->commandBuffers[currentFrame].beginRendering(renderingInfo);
  frames->commandBuffers[currentFrame].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swp->swapChainExtent.width), static_cast<float>(swp->swapChainExtent.height), 0.0f, 1.0f));
  frames->commandBuffers[currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swp->swapChainExtent));

  frames->commandBuffers[currentFrame].bindVertexBuffers(0, *buf->vertexBuffer, {0});
  frames->commandBuffers[currentFrame].bindIndexBuffer(*buf->indexBuffer, 0, vk::IndexType::eUint32);

  drawScreen();
  frames->commandBuffers[currentFrame].endRendering();
  // After rendering, transition the swapchain image to PRESENT_SRC
  img->transition_image_layout(
      swp->swapChainImages[imageIndex],
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
      {},                                                 // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eBottomOfPipe,          // dstStage
      vk::ImageAspectFlagBits::eColor);
  frames->commandBuffers[currentFrame].end();
}

void VulkanEngine::keepTime() {
  float currentTime = static_cast<float>(glfwGetTime());
  frameCount += 1;
  deltaTime = currentTime - time;
  time = currentTime;

  timeSinceFPS += deltaTime;

  if (timeSinceFPS >= 1) {
    std::cout << "FPS: " << frameCount << std::endl;
    timeSinceFPS = 0;
    frameCount = 0;
  }
}

void VulkanEngine::initVulkan() {

  cxt->createInstance();
  std::cout << "instance created" << std::endl;

  cxt->setupDebugMessenger();
  std::cout << "debug created" << std::endl;
  cxt->createSurface();
  std::cout << "Surface created" << std::endl;
  cxt->pickPhysicalDevice();
  cxt->createLogicalDevice();
  std::cout << "Logical device created" << std::endl;

  swp->createSwapChain();
  swp->createImageViews();
  dsc->createGlobalDescriptorSetLayout();
  dsc->createMaterialDescriptorSetLayout();
  dsc->createObjectDescriptorSetLayout();
  std::cout << "Created escriptor set layouts" << std::endl;
  createPipelines();
  std::cout << "Pipeline created" << std::endl;
  frames->createCommandPool();
  std::cout << "Command pool created" << std::endl;
  img->createDepthResources();
  std::cout << "Making shapes..." << std::endl;
  makeShapes();
  buf->createVertexBuffer();
  std::printf("Creating index buffer...\n");
  buf->createIndexBuffer();
  std::printf("Creating uniform buffers...\n");
  buf->createUniformBuffers();
  buf->createStorageBuffer();
  std::printf("Creating descriptor pool...\n");
  dsc->createDescriptorPool();
  std::printf("Creating descriptor sets...\n");
  dsc->createGlobalDescriptorSets();
  dsc->createObjDescriptorSets();
  std::printf("Creating command buffers...\n");
  frames->createCommandBuffers();
  std::printf("Creating sync objects...\n");
  frames->createSyncObjects();
  std::cout << "Making materials..." << std::endl;
  makeMaterials();
}

void VulkanEngine::cleanup() {
  glfwDestroyWindow(win->window);
  cxt = nullptr;
  win = nullptr;
  buf = nullptr;
  dsc = nullptr;
  frames = nullptr;
  swp = nullptr;

  glfwTerminate();
}

Window VulkanEngine::getWindow() {
  return *win;
}

void VulkanEngine::drawScreen() {};
void VulkanEngine::mouseMoved(float, float) {};
void VulkanEngine::updateCameraTransforms(GlobalUBO &) {};
void VulkanEngine::updateLights(std::vector<SSBOLight> &) {};
void VulkanEngine::processInput(GLFWwindow *) {};
void VulkanEngine::createPipelines() {};
void VulkanEngine::makeShapes() {};
void VulkanEngine::makeMaterials() {};
std::vector<Vertex> VulkanEngine::getVertices() { return {}; };
std::vector<uint32_t> VulkanEngine::getIndices() { return {}; };
