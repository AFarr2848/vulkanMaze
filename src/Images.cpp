#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Components/FrameData.hpp"
#include "vulkan/vulkan.hpp"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

vk::raii::ImageView Images::createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlagBits flags, uint32_t layer, uint32_t layerCount) {
  vk::ImageViewCreateInfo viewInfo{.image = image, .viewType = vk::ImageViewType::e2D, .format = format, .subresourceRange = {flags, 0, 1, layer, layerCount}};
  return vk::raii::ImageView(cxt->device, viewInfo);
}

void Images::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  createImage(swp->swapChainExtent.width, swp->swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
  depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void Images::createColorResources() {
  postImages.clear();
  postImageMemory.clear();
  postImageViews.clear();
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory imageMem = nullptr;
    vk::Format colorFormat = swp->swapChainSurfaceFormat.format;
    createImage(swp->swapChainExtent.width, swp->swapChainExtent.height, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, image, imageMem);
    postImages.push_back(std::move(image));
    postImageMemory.push_back(std::move(imageMem));
    postImageViews.push_back(createImageView(postImages[i], colorFormat, vk::ImageAspectFlagBits::eColor));
  }

  vk::PhysicalDeviceProperties properties = cxt->physicalDevice.getProperties();
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways};

  postImageSampler = cxt->device.createSampler(samplerInfo);
}

vk::raii::ImageView &Images::getDataImageView(std::string name) {
  for (int i = 0; i < dataImages.size(); i++) {
    if (dataImages[i].name == name)
      return dataImages[i].view;
  }
  for (int i = 0; i < dataImages.size(); i++) {
    if (dataImages[i].name.empty())
      dataImages[i].name = name;
    return dataImages[i].view;
  }
  throw std::runtime_error("Out of data images!!!");
}

vk::raii::ImageView &Images::getNextPostView() {
  currentPostView %= 2;
  return postImageViews.at(currentPostView);
}
vk::raii::ImageView &Images::getCurrentPostView() {
  currentPostView %= 2;
  return postImageViews.at(currentPostView);
}

void Images::createPostDataResources() {
  dataImages.clear();
  for (int i = 0; i < MAX_DATA_IMAGES; i++) {
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory imageMem = nullptr;
    vk::Format colorFormat = swp->swapChainSurfaceFormat.format;
    createImage(swp->swapChainExtent.width, swp->swapChainExtent.height, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, image, imageMem);
    vk::raii::ImageView view = createImageView(image, colorFormat, vk::ImageAspectFlagBits::eColor);
    dataImages.push_back(

        {

            .image = std::move(image),
            .view = std::move(view),
            .imageMemory = std::move(imageMem),
            .name = ""});
  }

  vk::PhysicalDeviceProperties properties = cxt->physicalDevice.getProperties();
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways

  };
  dataImageSampler = cxt->device.createSampler(samplerInfo);
}

vk::Format Images::findDepthFormat() {
  return cxt->findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint

      },
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void Images::createImage(vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory, uint32_t layerCount) {
  createImage(swp->swapChainExtent.width, swp->swapChainExtent.height, format, tiling, usage, properties, image, imageMemory, layerCount);
}

void Images::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory, uint32_t layerCount) {
  vk::ImageCreateInfo imageInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = layerCount,
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
