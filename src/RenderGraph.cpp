#include "vkMaze/Components/RenderGraph.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/Managers.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/Swapchain.hpp"

#include "vkMaze/Objects/Shapes.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vkMaze/Objects/Material.hpp"
#include "vulkan/vulkan.hpp"
#include <csignal>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

void RenderGraph::recordPost(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraphPass &pass) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo;
  vk::RenderingAttachmentInfo depthAttachmentInfo;
  for (auto &write : pass.writes) {
    auto &resource = getResource(write.resource, imageIndex);
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
      .renderArea = {.offset = {0, 0}, .extent = swp->swapChainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo

  };

  cmd.beginRendering(renderingInfo);

  cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swp->swapChainExtent.width), static_cast<float>(swp->swapChainExtent.height), 0.0f, 1.0f));
  cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swp->swapChainExtent));

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pass.pipeline.graphicsPipeline);
  if (pass.pipeline.usesSet(0))
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pass.pipeline.pipelineLayout, 0, *pass.postImgDscSets[frameIndex], nullptr);
  cmd.draw(3, 1, 0, 0);

  cmd.endRendering();
}

void RenderGraph::recordMain(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraphPass &pass) {

  vk::ClearValue clearColor = vk::ClearColorValue(0.05f, 0.03f, 0.05f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
  vk::RenderingAttachmentInfo attachmentInfo;
  vk::RenderingAttachmentInfo depthAttachmentInfo;
  for (auto &write : pass.writes) {
    auto &resource = getResource(write.resource, imageIndex);
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

  std::vector<Shape *> drawShapes = shapes->getDrawOrder();
  Material *currentMaterial = nullptr;
  Pipeline *currentPipeline = nullptr;

  for (Shape *s : drawShapes) {

    if (s->pipeline != currentPipeline) {
      currentPipeline = s->pipeline;
      currentMaterial = nullptr;
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, currentPipeline->graphicsPipeline);
      if (currentPipeline->usesSet(0))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 0, *globalDscSets[frameIndex], nullptr);
      if (currentPipeline->usesSet(2))
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentPipeline->pipelineLayout, 2, *lights->dscSets[frameIndex], nullptr);
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

  cmd.endRendering();
}

void RenderGraph::execute(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex) {
  img->transition_image_layout(
      cmd,
      swp->swapChainImages[imageIndex],
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},                                                 // srcAccessMask (no need to wait for previous operations)
      vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
      vk::ImageAspectFlagBits::eColor

  );

  for (size_t passIndex = 0; passIndex < passes.size(); passIndex++) {
    RenderGraphPass &pass = passes[passIndex];
    if (resourceBarriers.contains(passIndex)) {
      for (BarrierPoint &bp : resourceBarriers.at(passIndex)) {
        RenderGraphResource *res;
        if (bp.newAccess == vk::AccessFlagBits2::eColorAttachmentWrite || bp.newAccess == vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
          res = &getResource(bp.resource);
        else
          res = &getResource(bp.resource);
        // assert(bp.oldLayout == res.layout);
        img->transition_image_layout(cmd, res->getImage(), res->layout, bp.newLayout, bp.oldAccess, bp.newAccess, bp.oldStage, bp.newStage, bp.aspectFlags);
        res->layout = bp.newLayout;
      }
    }

    switch (pass.type) {
    case MAIN_PASS:
      recordMain(cmd, frameIndex, imageIndex, pass);
      break;
    case POST_PASS:
      recordPost(cmd, frameIndex, imageIndex, pass);
      break;
    }
  }
  img->transition_image_layout(
      cmd,
      swp->swapChainImages[imageIndex],
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
      {},                                                 // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eBottomOfPipe,          // dstStage
      vk::ImageAspectFlagBits::eColor);
}

RenderGraphPass &RenderGraph::addPass(std::string name, const PipelineDsc dsc, std::vector<RenderGraphAccess> writes, PassType type) {
  RenderGraphPass pass = {.name = name, .type = type};

  if (type != MAIN_PASS) {
    pass.pipeline = Pipeline();
    pass.pipeline.init(*cxt, *swp, *img);
    pass.pipeline.createPipeline(dsc);
    for (int i = 0; i < pass.pipeline.shaderResources.size(); i++) {
      ShaderResource &res = pass.pipeline.shaderResources.at(i);
      if (res.type == vk::DescriptorType::eCombinedImageSampler ||
          res.type == vk::DescriptorType::eSampledImage ||
          res.type == vk::DescriptorType::eInputAttachment) {

        // Set up ping-pong image numbers
        if (res.name.substr(0, 5) == "color") {
          res.name = "color:" + std::to_string(currentColor);
          pass.hasColorRead = true;
        }
        if (res.name.substr(0, 5) == "depth") {
          res.name = "depth:" + std::to_string(currentDepth);
          pass.hasDepthRead = true;
        }

        pass.reads.push_back({

            .resource = res.name,
            .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
            .access = vk::AccessFlagBits2::eShaderRead,
            .stages = vk::PipelineStageFlagBits2::eFragmentShader,

        });
      }

      // todo: storange images
    }
  }

  for (auto write : writes) {
    if (write.resource == "color" && pass.hasColorRead) {
      currentColor = (currentColor + 1) % 2;
      write.resource = "color:" + std::to_string(currentColor);
    } else if (write.resource == "color") {
      write.resource = "color:" + std::to_string(currentColor);
    }
    if (write.resource == "depth" && pass.hasDepthRead) {
      currentDepth = (currentDepth + 1) % 2;
      write.resource = "depth:" + std::to_string(currentDepth);
    } else if (write.resource == "depth") {
      write.resource = "depth:" + std::to_string(currentDepth);
    }
    pass.writes.push_back(write);
  }

  passes.push_back(std::move(pass));
  return passes.at(passes.size() - 1);
}

void RenderGraph::addImage(const RenderGraphResourceDesc &desc) {
  if (!resources.contains(desc.name)) {
    RenderGraphResource resource;
    resource.desc = desc;
    resources.emplace(desc.name, std::move(resource));
  }
}

void RenderGraph::addExternalImage(const RenderGraphResourceDesc &desc, vk::Image image, vk::raii::ImageView *view) {
  if (!resources.contains(desc.name)) {
    RenderGraphResource resource;
    resource.isExternal = true;
    resource.setExternalImage(image);
    resource.setExternalView(view);
    resource.desc = desc;
    resource.layout = desc.initialLayout;
    resources.emplace(resource.desc.name, std::move(resource));
  }
}

void RenderGraph::makePostImgDscSets(RenderGraphPass &pass) {
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pass.pipeline.setLayouts.at(0));
  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = *dsc->descriptorPool,
      .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts = layouts.data()};

  pass.postImgDscSets.clear();
  pass.postImgDscSets = cxt->device.allocateDescriptorSets(allocInfo);
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    std::vector<vk::DescriptorImageInfo> infos;
    std::vector<vk::WriteDescriptorSet> writes;
    infos.reserve(pass.pipeline.shaderResources.size());
    writes.reserve(pass.pipeline.shaderResources.size());

    for (auto &shaderResource : pass.pipeline.shaderResources) {
      if (shaderResource.set != 0 || shaderResource.type != vk::DescriptorType::eCombinedImageSampler)
        continue;

      auto &imageResource = getResource(shaderResource.name);
      infos.push_back({.sampler = sampler,
                       .imageView = imageResource.getImageView(),
                       .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal});

      writes.push_back({.dstSet = *pass.postImgDscSets[i],
                        .dstBinding = shaderResource.binding,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &infos.back()});
    }

    cxt->device.updateDescriptorSets(writes, nullptr);
  }
}

void RenderGraph::makeGlobalDscSets() {
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *dsc->globalSetLayout);

  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = dsc->descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()

  };

  globalDscSets.clear();
  globalDscSets = cxt->device.allocateDescriptorSets(allocInfo);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo camInfo{
        .buffer = buf->globalUBOs[i],
        .offset = 0,
        .range = sizeof(GlobalUBO)

    };

    vk::WriteDescriptorSet camWrite{
        .dstSet = globalDscSets.at(i),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &camInfo

    };
    vk::DescriptorBufferInfo transformInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = sizeof(SSBOLight) * (MAX_DIR_LIGHTS + MAX_SPOT_LIGHTS + MAX_POINT_LIGHTS),
        .range = sizeof(glm::mat4) * MAX_TRANSFORMS};

    vk::WriteDescriptorSet transformWrite{
        .dstSet = globalDscSets.at(i),
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &transformInfo

    };

    std::vector<vk::WriteDescriptorSet> writes = {camWrite, transformWrite};

    cxt->device.updateDescriptorSets(writes, {});
  }
}

void RenderGraph::compile() {
  resourceBarriers.clear();
  for (auto &pair : resources) {
    RenderGraphResource &r = pair.second;
    r.desc.extent = swp->swapChainExtent;

    if (r.isExternal == false) {
      img->createImage(r.desc.extent.width, r.desc.extent.height, r.desc.format, vk::ImageTiling::eOptimal, r.desc.usage, vk::MemoryPropertyFlagBits::eDeviceLocal, r.image, r.memory);
      r.imageView = img->createImageView(r.getImage(), r.desc.format, r.desc.aspect);
    } else {
      std::cout << r.desc.name << std::endl;
      assert(r.getImage() != nullptr && r.getImageView() != nullptr);
    }
    r.layout = r.desc.initialLayout;
  }

  std::unordered_map<std::string, RenderGraphAccess> lastAccess;

  // assumes passes are in order
  for (uint32_t i = 0; i < passes.size(); i++) {
    RenderGraphPass &pass = passes.at(i);

    for (RenderGraphAccess &access : pass.reads) {
      if (!lastAccess.contains(access.resource)) {

        auto &r = getResource(access.resource);
        if (access.layout != r.layout) {
          BarrierPoint barrier{
              .resource = access.resource,
              .oldLayout = r.layout,
              .newLayout = access.layout,
              .oldAccess = {},
              .newAccess = access.access,
              .oldStage = vk::PipelineStageFlagBits2::eTopOfPipe,
              .newStage = access.stages,
              .aspectFlags = r.desc.aspect,
          };
          resourceBarriers.insert({i, {}});
          resourceBarriers.at(i).push_back(barrier);
        }
        lastAccess.emplace(access.resource, access);
      } else if (lastAccess.at(access.resource) != access) {
        RenderGraphAccess old = lastAccess.at(access.resource);
        auto &r = getResource(access.resource);
        BarrierPoint barrier{
            .resource = access.resource,
            .oldLayout = old.layout,
            .newLayout = access.layout,
            .oldAccess = old.access,
            .newAccess = access.access,
            .oldStage = old.stages,
            .newStage = access.stages,
            .aspectFlags = r.desc.aspect};
        resourceBarriers.insert({i, {}});
        resourceBarriers.at(i).push_back(barrier);
        lastAccess.at(access.resource) = access;
      } else {
        lastAccess.at(access.resource) = access;
      }
    }

    for (RenderGraphAccess &access : pass.writes) {
      if (access.resource == "swap")
        continue;
      if (!lastAccess.contains(access.resource)) {

        auto &r = getResource(access.resource);
        if (access.layout != r.layout) {
          BarrierPoint barrier{
              .resource = access.resource,
              .oldLayout = r.layout,
              .newLayout = access.layout,
              .oldAccess = {},
              .newAccess = access.access,
              .oldStage = vk::PipelineStageFlagBits2::eTopOfPipe,
              .newStage = access.stages,
              .aspectFlags = r.desc.aspect,
          };
          resourceBarriers.insert({i, {}});
          resourceBarriers.at(i).push_back(barrier);
        }
        lastAccess.emplace(access.resource, access);
      } else if (lastAccess.at(access.resource) != access) {
        RenderGraphAccess old = lastAccess.at(access.resource);
        auto &r = getResource(access.resource);
        BarrierPoint barrier{
            .resource = access.resource,
            .oldLayout = old.layout,
            .newLayout = access.layout,
            .oldAccess = old.access,
            .newAccess = access.access,
            .oldStage = old.stages,
            .newStage = access.stages,
            .aspectFlags = r.desc.aspect};
        resourceBarriers.insert({i, {}});
        resourceBarriers.at(i).push_back(barrier);
        lastAccess.at(access.resource) = access;
      } else {
        lastAccess.at(access.resource) = access;
      }
    }
  }

  vk::PhysicalDeviceProperties properties = cxt->physicalDevice.getProperties();
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways};
  sampler = vk::raii::Sampler(cxt->device, samplerInfo);

  for (auto &pass : passes) {

    if (pass.type == POST_PASS) {
      std::cout << "Making post img dsc sets" << std::endl;
      makePostImgDscSets(pass);
    }
  }
}

void RenderGraph::clear() {
  resources.clear();
  passes.clear();
}

void RenderGraph::createPingPongResources() {

  // swap
  for (int i = 0; i < swp->swapChainImages.size(); i++) {
    RenderGraphResourceDesc dsc = {
        .name = "swap:" + std::to_string(i),
        .format = swp->swapChainSurfaceFormat.format,
        .extent = swp->swapChainExtent,
        .usage = vk::ImageUsageFlagBits::eColorAttachment,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .samples = vk::SampleCountFlagBits::e1,
        .initialLayout = vk::ImageLayout::eUndefined,
        .isExternal = true,
    };
    addExternalImage(dsc, swp->swapChainImages.at(i), &swp->swapChainImageViews.at(i));
  }
  // color ping pong
  for (int i = 0; i < 2; i++) {
    RenderGraphResourceDesc dsc = {
        .name = "color:" + std::to_string(i),
        .format = swp->swapChainSurfaceFormat.format,
        .extent = swp->swapChainExtent,
        .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .samples = vk::SampleCountFlagBits::e1,
        .initialLayout = vk::ImageLayout::eUndefined,
        .isExternal = false,
    };
    addImage(dsc);
  }
  // depth ping pong
  for (int i = 0; i < 2; i++) {
    RenderGraphResourceDesc dsc = {
        .name = "depth:" + std::to_string(i),
        .format = img->findDepthFormat(),
        .extent = swp->swapChainExtent,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        .aspect = vk::ImageAspectFlagBits::eDepth,
        .samples = vk::SampleCountFlagBits::e1,
        .initialLayout = vk::ImageLayout::eUndefined,
        .isExternal = false,
    };
    addImage(dsc);
  }
}
RenderGraphResource &RenderGraph::getResource(std::string id, int imageIndex) {
  if (id == "swap") {
    assert(imageIndex >= 0);
    id = "swap:" + std::to_string(imageIndex);
  }
  return resources.at(id);
}
