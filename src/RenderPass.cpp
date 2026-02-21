#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/Managers.hpp"
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

void RenderPass::createGlobalDscSets() {

  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *dsc->descriptorSetLayout);

  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = dsc->descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()

  };

  descriptorSets.clear();
  descriptorSets = cxt->device.allocateDescriptorSets(allocInfo);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo camInfo{
        .buffer = buf->globalUBOs[i],
        .offset = 0,
        .range = sizeof(GlobalUBO)

    };

    vk::WriteDescriptorSet camWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &camInfo

    };

    std::vector<vk::WriteDescriptorSet> writes = {camWrite};

    cxt->device.updateDescriptorSets(writes, {});
  }
}

void RenderPass::record(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, vk::raii::ImageView &colorView, vk::raii::ImageView &depthView) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = colorView,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor

  };
  vk::RenderingAttachmentInfo depthAttachmentInfo = {
      .imageView = depthView,
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
  cmd.beginRendering(renderingInfo);

  cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swp->swapChainExtent.width), static_cast<float>(swp->swapChainExtent.height), 0.0f, 1.0f));
  cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swp->swapChainExtent));

  cmd.bindVertexBuffers(0, *buf->vertexBuffer, {0});
  cmd.bindIndexBuffer(*buf->indexBuffer, 0, vk::IndexType::eUint32);
  drawScreen(cmd, frameIndex);

  cmd.endRendering();
}

void RenderPass::drawScreen(vk::raii::CommandBuffer &cmd, uint32_t currentFrame) {
  std::vector<Shape *> drawShapes = shapes->getDrawOrder();
  Material *currentMaterial = nullptr;
  Pipeline *currentPipeline = nullptr;

  for (Shape *s : drawShapes) {

    if (s->pipeline != currentPipeline) {
      currentPipeline = s->pipeline;
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, currentPipeline->graphicsPipeline);
      if (currentPipeline->usesSet(0))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 0, *descriptorSets[currentFrame], nullptr);
      if (currentPipeline->usesSet(2))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 2, *dsc->getSet(2, currentFrame), nullptr);
      if (currentPipeline->usesSet(3))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 3, *dsc->getSet(3, currentFrame), nullptr);
    }
    if (s->material != currentMaterial && currentPipeline->usesSet(1)) {
      cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s->pipeline->pipelineLayout, 1, *s->material->albedo.descriptorSet, nullptr);
      currentMaterial = s->material;
    }

    if (s->pipeline->hasPushConstants) {
      PushConstant pc = PushConstant();
      pc.lightNums = lights->getLightNums();
      pc.transformIndex = s->transformIndex;
      const auto *pcBytes = reinterpret_cast<const uint8_t *>(&pc);
      cmd.pushConstants(s->pipeline->pipelineLayout,
                        s->pipeline->pcRange.stageFlags,
                        s->pipeline->pcRange.offset,
                        vk::ArrayProxy<const uint8_t>(s->pipeline->pcRange.size, pcBytes));
    }

    cmd.drawIndexed(s->range.indexCount, 1, s->range.indexOffset, s->range.vertexOffset, 0);
  }
}
