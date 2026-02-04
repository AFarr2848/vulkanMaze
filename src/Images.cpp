#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Components/FrameData.hpp"

vk::raii::ImageView Images::createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlagBits flags) {
  vk::ImageViewCreateInfo viewInfo{.image = image, .viewType = vk::ImageViewType::e2D, .format = format, .subresourceRange = {flags, 0, 1, 0, 1}};
  return vk::raii::ImageView(cxt->device, viewInfo);
}

void Images::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  createImage(swp->swapChainExtent.width, swp->swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
  depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

vk::Format Images::findDepthFormat() {
  return cxt->findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint

      },
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void Images::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory) {
  vk::ImageCreateInfo imageInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive

  };

  image = vk::raii::Image(cxt->device, imageInfo);

  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{.allocationSize = memRequirements.size,
                                   .memoryTypeIndex = cxt->findMemoryType(memRequirements.memoryTypeBits, properties)};
  imageMemory = vk::raii::DeviceMemory(cxt->device, allocInfo);
  image.bindMemory(imageMemory, 0);
}

void Images::transition_image_layout(
    vk::Image image,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask,
    vk::ImageAspectFlags image_aspect_flags) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = src_stage_mask,
      .srcAccessMask = src_access_mask,
      .dstStageMask = dst_stage_mask,
      .dstAccessMask = dst_access_mask,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {
          .aspectMask = image_aspect_flags,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1}};
  vk::DependencyInfo dependency_info = {
      .dependencyFlags = {},
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier};
  frame->commandBuffers[frame->currentFrame].pipelineBarrier2(dependency_info);
}
