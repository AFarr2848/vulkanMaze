#pragma once
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/Imgui.hpp"
#include "vkMaze/Components/Managers.hpp"
#include "vkMaze/Components/RenderGraph.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Objects/Shapes.hpp"
#include "vkMaze/Objects/Material.hpp"
inline void recordGui(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraph &graph, RenderGraphPass &pass) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo;
  vk::RenderingAttachmentInfo depthAttachmentInfo;
  for (auto &write : pass.writes) {
    auto &resource = graph.getResource(write.resource, imageIndex);
    if (write.layout == vk::ImageLayout::eColorAttachmentOptimal)

      attachmentInfo = {
          .imageView = resource.getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = clearColor

      };
    else if (write.layout == vk::ImageLayout::eDepthAttachmentOptimal || write.layout == vk::ImageLayout::eDepthAttachmentOptimalKHR)
      depthAttachmentInfo = {
          .imageView = resource.getImageView(),
          .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = clearDepth

      };
  }
  assert(attachmentInfo != nullptr);
  if (depthAttachmentInfo == nullptr) {
    depthAttachmentInfo = {
        .imageView = nullptr,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearDepth

    };
  }
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = graph.swp->swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo

  };

  cmd.beginRendering(renderingInfo);

  cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(graph.swp->swapChainExtent.width), static_cast<float>(graph.swp->swapChainExtent.height), 0.0f, 1.0f));
  cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), graph.swp->swapChainExtent));

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pass.pipeline.graphicsPipeline);
  if (pass.pipeline.usesSet(0))
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pass.pipeline.pipelineLayout, 0, *pass.postImgDscSets[frameIndex], nullptr);
  cmd.draw(3, 1, 0, 0);
  graph.gui->renderImgui(cmd);

  cmd.endRendering();
}

inline void recordPost(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraph &graph, RenderGraphPass &pass) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo;
  vk::RenderingAttachmentInfo depthAttachmentInfo;
  for (auto &write : pass.writes) {
    auto &resource = graph.getResource(write.resource, imageIndex);
    if (write.layout == vk::ImageLayout::eColorAttachmentOptimal)

      attachmentInfo = {
          .imageView = resource.getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = clearColor

      };
    else if (write.layout == vk::ImageLayout::eDepthAttachmentOptimal || write.layout == vk::ImageLayout::eDepthAttachmentOptimalKHR)
      depthAttachmentInfo = {
          .imageView = resource.getImageView(),
          .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = clearDepth

      };
  }
  assert(attachmentInfo != nullptr);
  if (depthAttachmentInfo == nullptr) {
    depthAttachmentInfo = {
        .imageView = nullptr,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearDepth

    };
  }
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = graph.swp->swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo

  };

  cmd.beginRendering(renderingInfo);

  cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(graph.swp->swapChainExtent.width), static_cast<float>(graph.swp->swapChainExtent.height), 0.0f, 1.0f));
  cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), graph.swp->swapChainExtent));

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pass.pipeline.graphicsPipeline);
  if (pass.pipeline.usesSet(0))
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pass.pipeline.pipelineLayout, 0, *pass.postImgDscSets[frameIndex], nullptr);
  cmd.draw(3, 1, 0, 0);

  cmd.endRendering();
}

inline void recordMain(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraph &graph, RenderGraphPass &pass) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo;
  vk::RenderingAttachmentInfo depthAttachmentInfo;
  for (auto &write : pass.writes) {
    auto &resource = graph.getResource(write.resource, imageIndex);
    assert(write.layout == resource.layout);
    if (write.layout == vk::ImageLayout::eColorAttachmentOptimal) {

      assert(resource.getImageView() != nullptr);
      attachmentInfo = {
          .imageView = resource.getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = clearColor

      };
    }
    if (write.layout == vk::ImageLayout::eDepthAttachmentOptimal || write.layout == vk::ImageLayout::eDepthAttachmentOptimalKHR) {

      assert(resource.getImageView() != nullptr);
      depthAttachmentInfo = {
          .imageView = resource.getImageView(),
          .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = clearDepth

      };
    }
  }
  assert(attachmentInfo.imageView && depthAttachmentInfo.imageView);
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = graph.swp->swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo

  };
  cmd.beginRendering(renderingInfo);

  cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(graph.swp->swapChainExtent.width), static_cast<float>(graph.swp->swapChainExtent.height), 0.0f, 1.0f));
  cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), graph.swp->swapChainExtent));

  cmd.bindVertexBuffers(0, *graph.buf->vertexBuffer, {0});
  cmd.bindIndexBuffer(*graph.buf->indexBuffer, 0, vk::IndexType::eUint32);

  std::vector<Shape *> drawShapes = graph.shapes->getDrawOrder();
  Material *currentMaterial = nullptr;
  Pipeline *currentPipeline = nullptr;

  for (Shape *s : drawShapes) {

    if (s->pipeline != currentPipeline) {
      currentPipeline = s->pipeline;
      currentMaterial = nullptr;
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, currentPipeline->graphicsPipeline);
      if (currentPipeline->usesSet(0))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 0, *graph.globalDscSets[frameIndex], nullptr);
      if (currentPipeline->usesSet(2))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 2, *graph.lights->dscSets[frameIndex], nullptr);
    }
    if (s->material != currentMaterial && currentPipeline->usesSet(1)) {
      cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s->pipeline->pipelineLayout, 1, *s->material->albedo.descriptorSet, nullptr);
      currentMaterial = s->material;
    }

    if (s->pipeline->hasPushConstants) {
      struct PushConstant {
        int32_t transformIndex;           // 4 bytes at offset 0
        alignas(16) glm::ivec3 lightNums; // 12 bytes at offset 16
      };
      PushConstant pc = PushConstant();
      pc.lightNums = graph.lights->getLightNums();
      pc.transformIndex = s->transformIndex;
      const auto *pcBytes = reinterpret_cast<const uint8_t *>(&pc);
      cmd.pushConstants(s->pipeline->pipelineLayout,
                        s->pipeline->pcRange.stageFlags,
                        s->pipeline->pcRange.offset,
                        vk::ArrayProxy<const uint8_t>(s->pipeline->pcRange.size, pcBytes));
    }

    cmd.drawIndexed(s->range.indexCount, 1, s->range.indexOffset, s->range.vertexOffset, 0);
  }

  cmd.endRendering();
}
