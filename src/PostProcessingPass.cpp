
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/Managers.hpp"
#include "vkMaze/Objects/PostProcessingPass.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Objects/Shapes.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vkMaze/Objects/Material.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vkMaze/Components/RenderPass.hpp>
#include <vkMaze/Components/Swapchain.hpp>
#include <vulkan/vulkan_raii.hpp>

void PostProcessingPass::createPPPDscSets(vk::DescriptorSetLayout layout, std::vector<vk::raii::ImageView> &imageViews, vk::raii::Sampler &sampler) {
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, layout);
  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = *dsc->descriptorPool,
      .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts = layouts.data()};

  descriptorSets.clear();
  descriptorSets = cxt->device.allocateDescriptorSets(allocInfo);
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    std::vector<vk::DescriptorImageInfo> imageInfos;
    std::vector<vk::WriteDescriptorSet> writes;
    for (ShaderResource &r : pipeline.shaderResources) {
      imageInfos.push_back({

          .sampler = img->dataImageSampler,
          .imageView = img->getDataImageView(r.name),
          .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal

      });
      writes.push_back({

          .dstSet = *descriptorSets[i],
          .dstBinding = r.binding,
          .descriptorCount = 1,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo = &imageInfos.at(imageInfos.size() - 1)

      });
    }

    vk::DescriptorImageInfo imageInfo{
        .sampler = sampler,
        .imageView = imageViews[i],
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

    vk::WriteDescriptorSet write{
        .dstSet = *descriptorSets[i],
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo};

    cxt->device.updateDescriptorSets(write, nullptr);
  }
}

void PostProcessingPass::record(vk::raii::CommandBuffer &cmd, uint32_t frameIndex) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = img->getNextPostView(),
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
      .pDepthAttachment = nullptr

  };

  cmd.beginRendering(renderingInfo);

  cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swp->swapChainExtent.width), static_cast<float>(swp->swapChainExtent.height), 0.0f, 1.0f));
  cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swp->swapChainExtent));

  drawScreen(cmd, frameIndex);

  cmd.endRendering();
}

void PostProcessingPass::drawScreen(vk::raii::CommandBuffer &cmd, uint32_t currentFrame) {

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);
  for (ShaderResource &r : pipeline.shaderResources) {
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, r.set, *descriptorSets[currentFrame], nullptr);
  }

  cmd.draw(3, 1, 0, 0);
}
