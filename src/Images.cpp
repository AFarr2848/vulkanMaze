#include "vkMaze/Images.hpp"
#include "vkMaze/VulkanContext.hpp"
#include "vkMaze/Util.hpp"
#include "vkMaze/Swapchain.hpp"
void Images::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory) {
  vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D, .format = format, .extent = {width, height, 1}, .mipLevels = 1, .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1, .tiling = tiling, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  image = vk::raii::Image(cxt->device, imageInfo);

  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{.allocationSize = memRequirements.size,
                                   .memoryTypeIndex = cxt->findMemoryType(memRequirements.memoryTypeBits, properties)};
  imageMemory = vk::raii::DeviceMemory(cxt->device, allocInfo);
  image.bindMemory(imageMemory, 0);
}

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
