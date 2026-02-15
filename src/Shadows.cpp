#include "vkMaze/Objects/Pipelines.hpp"
#include "vulkan/vulkan.hpp"
#include <vkMaze/Components/Shadows.hpp>
#include <vkMaze/Components/Images.hpp>
#include <vkMaze/Components/Swapchain.hpp>
#include <vkMaze/Components/FrameData.hpp>
#include <vulkan/vulkan_raii.hpp>

void MappedShadows::makeResources() {
  img.createImage(
      swp.swapChainExtent.width,
      swp.swapChainExtent.height,
      vk::Format::eD32Sfloat,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      imageArrayMap,
      imageArrayMemory,
      numShadowLights

  );

  imageArrayView = img.createImageView(
      imageArrayMap,
      vk::Format::eD32Sfloat,
      vk::ImageAspectFlagBits::eDepth,
      0,
      numShadowLights

  );
  shadowMappingPipeline.init(cxt, dsc, swp, img);
  shadowMappingPipeline.createPipeline({.shaderPath = "build/shaders/shadowMapping.spv"
                                                          .topology = vk::PrimitiveTopology::eTriangleList,
                                        .polygonMode = vk::PolygonMode::eFill,
                                        .cullModeFlags = vk::CullModeFlagBits::eBack,
                                        .setLayouts = dscSetLayouts})
}

void MappedShadows::renderShadows(vk::raii::CommandBuffer &buf) {
  vk::CommandBuffer cmd = frames.commandBuffers[frames.currentFrame];

  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo shadowsDepthInfo = {
      .imageView = imageArrayView,
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = vk::ClearDepthStencilValue(1.0f, 0)

  };

  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = swp.swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 0,
      .pColorAttachments = nullptr,
      .pDepthAttachment = &shadowsDepthInfo

  };

  for (int i = 0; i < lights; i++) {
    cmd.beginRendering(renderingInfo);
    cmd.bindPipeline(shadowMappingPipeline);
  }
}
