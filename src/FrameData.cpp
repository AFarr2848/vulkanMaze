#include <vulkan/vulkan_raii.hpp>
#include "vkMaze/Components/FrameData.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/Swapchain.hpp"

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

std::unique_ptr<vk::raii::CommandBuffer> FrameData::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
  std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(vk::raii::CommandBuffers(ctx->device, allocInfo).front()));

  vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer->begin(beginInfo);

  return commandBuffer;
}

void FrameData::endSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer) {
  commandBuffer.end();

  vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};
  ctx->graphicsQueue.submit(submitInfo, nullptr);
  ctx->graphicsQueue.waitIdle();
}
