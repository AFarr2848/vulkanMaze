#include "vkMaze/FrameData.hpp"
#include "vkMaze/VulkanContext.hpp"
#include "vkMaze/EngineConfig.hpp"
#include "vkMaze/Swapchain.hpp"

void FrameData::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                     .queueFamilyIndex = ctx->queueIndex};
  commandPool = vk::raii::CommandPool(ctx->device, poolInfo);
}

void FrameData::createCommandBuffers() {
  commandBuffers.clear();
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
  commandBuffers = vk::raii::CommandBuffers(ctx->device, allocInfo);
}

void FrameData::createSyncObjects() {
  presentCompleteSemaphore.clear();
  renderFinishedSemaphore.clear();
  inFlightFences.clear();

  for (size_t i = 0; i < swp->swapChainImages.size(); i++) {
    presentCompleteSemaphore.emplace_back(ctx->device, vk::SemaphoreCreateInfo());
    renderFinishedSemaphore.emplace_back(ctx->device, vk::SemaphoreCreateInfo());
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    inFlightFences.emplace_back(ctx->device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  }
}
