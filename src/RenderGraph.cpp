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
#include "vkMaze/Records.hpp"
#include "vulkan/vulkan.hpp"
#include "vkMaze/Components/Imgui.hpp"
#include <cstdint>
#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vulkan/vulkan_raii.hpp>

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

  for (size_t passIndex = 0; passIndex < compiledPasses.size(); passIndex++) {
    RenderGraphPass &pass = compiledPasses[passIndex];
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
      recordMain(cmd, frameIndex, imageIndex, *this, pass);
      break;
    case POST_PASS:
      recordPost(cmd, frameIndex, imageIndex, *this, pass);
      break;
    case GUI_PASS:
      recordGui(cmd, frameIndex, imageIndex, *this, pass);
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

RenderGraphPass RenderGraph::createPassFromDsc(RenderGraphPassDsc &dsc) {
  RenderGraphPass pass = {
      .name = dsc.name,
      .type = dsc.type,

  };

  if (pass.type != MAIN_PASS) {
    pass.pipeline = Pipeline();
    pass.pipeline.init(*cxt, *swp, *img);
    pass.pipeline.createPipeline(dsc.pipelineDsc);
    for (int i = 0; i < pass.pipeline.shaderResources.size(); i++) {
      ShaderResource &res = pass.pipeline.shaderResources.at(i);
      if (res.type == vk::DescriptorType::eCombinedImageSampler ||
          res.type == vk::DescriptorType::eSampledImage ||
          res.type == vk::DescriptorType::eInputAttachment) {

        for (auto read : dsc.readOverrides) {
          if (res.name == read.shaderResource)
            res.name = read.rgResource;
        }

        // Set up ping-pong image numbers
        if (res.name.substr(0, 5) == "color") {
          res.name = "color:" + std::to_string(currentColor);
          pass.hasColorRead = true;
        }
        if (res.name.substr(0, 7) == "scratch") {
          res.name = "scratch:" + std::to_string(currentScratch);
          pass.hasScratchRead = true;
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

  for (auto write : dsc.writes) {
    if (write.resource == "color" && pass.hasColorRead) {
      currentColor = (currentColor + 1) % 2;
      write.resource = "color:" + std::to_string(currentColor);
    } else if (write.resource == "color") {
      write.resource = "color:" + std::to_string(currentColor);
    }
    if (write.resource == "scratch" && pass.hasScratchRead) {
      currentScratch = (currentScratch + 1) % 2;
      write.resource = "scratch:" + std::to_string(currentScratch);
    } else if (write.resource == "scratch") {
      write.resource = "scratch:" + std::to_string(currentScratch);
    }
    if (write.resource == "depth" && pass.hasDepthRead) {
      currentDepth = (currentDepth + 1) % 2;
      write.resource = "depth:" + std::to_string(currentDepth);
    } else if (write.resource == "depth") {
      write.resource = "depth:" + std::to_string(currentDepth);
    }
    pass.writes.push_back(write);
  }
  return std::move(pass);
}

RenderGraphPassDsc &RenderGraph::addPass(std::string name, const PipelineDsc dsc, std::vector<RenderGraphReadOverride> reads, std::vector<RenderGraphAccess> writes, PassType type) {
  RenderGraphPassDsc pass = {
      .name = name,
      .type = type,
      .pipelineDsc = dsc,
      .readOverrides = reads,
      .writes = writes,
  };
  auto insertIt = uncompiledPasses.end();
  if (type != GUI_PASS) {
    insertIt = std::find_if(uncompiledPasses.begin(), uncompiledPasses.end(), [](const RenderGraphPassDsc &existingPass) {
      return existingPass.type == GUI_PASS;
    });
  }

  size_t insertIndex = static_cast<size_t>(std::distance(uncompiledPasses.begin(), insertIt));
  uncompiledPasses.insert(insertIt, std::move(pass));
  return uncompiledPasses.at(insertIndex);
}

std::vector<std::vector<uint32_t>> RenderGraph::makePassDag(const std::vector<RenderGraphPass> &passes) const {
  std::vector<std::vector<uint32_t>> dag(passes.size());
  std::vector<std::unordered_set<uint32_t>> seenEdges(passes.size());

  std::unordered_map<std::string, uint32_t> lastWriter;
  std::unordered_map<std::string, std::vector<uint32_t>> readersSinceWrite;

  auto addEdge = [&](uint32_t from, uint32_t to) {
    if (from == to)
      return;
    if (seenEdges.at(from).insert(to).second)
      dag.at(from).push_back(to);
  };

  for (uint32_t passIndex = 0; passIndex < passes.size(); passIndex++) {
    const RenderGraphPass &pass = passes.at(passIndex);

    for (const RenderGraphAccess &read : pass.reads) {
      if (lastWriter.contains(read.resource))
        addEdge(lastWriter.at(read.resource), passIndex);
      readersSinceWrite[read.resource].push_back(passIndex);
    }

    for (const RenderGraphAccess &write : pass.writes) {
      const std::string &resource = write.resource;
      auto &resourceReaders = readersSinceWrite[resource];
      for (uint32_t reader : resourceReaders)
        addEdge(reader, passIndex);
      resourceReaders.clear();

      if (lastWriter.contains(resource))
        addEdge(lastWriter.at(resource), passIndex);
      lastWriter[resource] = passIndex;
    }
  }

  return dag;
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

std::vector<int> RenderGraph::flattenDAG(const std::vector<std::vector<uint32_t>> &dag) const {
  std::vector<int> inDegree(dag.size(), 0);
  for (const auto &edges : dag) {
    for (int to : edges) {
      if (to < 0 || static_cast<size_t>(to) >= dag.size())
        continue;
      inDegree.at(static_cast<size_t>(to))++;
    }
  }

  std::queue<int> zeroInDegree;
  for (size_t i = 0; i < inDegree.size(); i++) {
    if (inDegree.at(i) == 0)
      zeroInDegree.push(static_cast<int>(i));
  }

  std::vector<int> order;
  order.reserve(dag.size());

  while (!zeroInDegree.empty()) {
    int node = zeroInDegree.front();
    zeroInDegree.pop();
    order.push_back(node);

    for (int to : dag.at(static_cast<size_t>(node))) {
      if (to < 0 || static_cast<size_t>(to) >= dag.size())
        continue;
      int &deg = inDegree.at(static_cast<size_t>(to));
      deg--;
      if (deg == 0)
        zeroInDegree.push(to);
    }
  }

  // Cycle detected: no valid topological order.
  if (order.size() != dag.size())
    return {};
  return order;
}

void RenderGraph::compile() {
  resourceBarriers.clear();
  compiledPasses.clear();
  passDag.clear();
  currentColor = 0;
  currentDepth = 0;
  currentScratch = 0;
  std::vector<RenderGraphPass> pendingPasses;
  pendingPasses.reserve(uncompiledPasses.size());
  for (RenderGraphPassDsc &passDsc : uncompiledPasses)
    pendingPasses.push_back(createPassFromDsc(passDsc));
  passDag = makePassDag(pendingPasses);
  auto indices = flattenDAG(passDag);
  for (uint32_t i : indices) {
    compiledPasses.push_back(std::move(pendingPasses.at(i)));
  }

  for (auto &pair : resources) {
    RenderGraphResource &r = pair.second;
    r.desc.extent = swp->swapChainExtent;

    if (r.isExternal == false) {
      r.imageView = nullptr;
      r.image = nullptr;
      r.memory = nullptr;
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
  for (uint32_t i = 0; i < compiledPasses.size(); i++) {
    RenderGraphPass &pass = compiledPasses.at(i);

    for (RenderGraphAccess &access : pass.reads) {
      if (!lastAccess.contains(access.resource)) {

        auto &r = getResource(access.resource);
        if (access.layout != r.layout) {
          BarrierPoint barrier{
              .resource = access.resource,
              .oldLayout = r.layout,
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
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways};
  sampler = vk::raii::Sampler(cxt->device, samplerInfo);

  for (auto &pass : compiledPasses) {

    if (pass.type == POST_PASS || pass.type == GUI_PASS) {
      std::cout << "Making post img dsc sets" << std::endl;
      makePostImgDscSets(pass);
    }
  }
}

void RenderGraph::clear() {
  resources.clear();
  compiledPasses.clear();
  uncompiledPasses.clear();
  passDag.clear();
}

bool RenderGraph::removePass(size_t index) {
  if (index >= uncompiledPasses.size())
    return false;
  uncompiledPasses.erase(uncompiledPasses.begin() + static_cast<long>(index));
  return true;
}

std::vector<std::string> RenderGraph::listResourceIds() const {
  std::vector<std::string> names;
  names.reserve(resources.size());
  for (const auto &pair : resources)
    names.push_back(pair.first);
  std::sort(names.begin(), names.end());
  return names;
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

  // extra ping pong
  for (int i = 0; i < 2; i++) {
    RenderGraphResourceDesc dsc = {
        .name = "scratch:" + std::to_string(i),
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
}
RenderGraphResource &RenderGraph::getResource(std::string id, int imageIndex) {
  if (id == "swap") {
    assert(imageIndex >= 0);
    id = "swap:" + std::to_string(imageIndex);
  }
  if (!resources.contains(id)) {
    std::cerr << "RenderGraph resource not found: " << id << std::endl;
  }
  return resources.at(id);
}
