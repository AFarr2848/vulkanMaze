#pragma once
#include "vkMaze/Objects/Pipelines.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class Images;
class Pipeline;

// chatgpt did this header bc I didn't want to think about render passes anymore :(

// Describes how a render graph image should be created or bound.
struct RenderGraphResourceDesc {
  std::string name;
  vk::Format format = vk::Format::eUndefined;
  vk::Extent2D extent = {};
  vk::ImageUsageFlags usage = {};
  // Which aspects are relevant for view creation and transitions.
  vk::ImageAspectFlagBits aspect = vk::ImageAspectFlagBits::eColor;
  vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
  vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
  // True when the image/view are provided by the caller (not owned by the graph).
  bool isExternal = false;
};

// Concrete resource instance tracked by the graph.
struct RenderGraphResource {
  RenderGraphResourceDesc desc;
  // Owned by the graph when desc.external is false.
  vk::raii::Image image = nullptr;
  vk::raii::ImageView imageView = nullptr;
  vk::raii::DeviceMemory memory = nullptr;
  // Latest known layout for barrier planning.
  vk::ImageLayout layout = vk::ImageLayout::eUndefined;

  bool isExternal = false;
  vk::raii::Image *extImage = nullptr;
  vk::raii::ImageView *extView = nullptr;
};

// How a pass reads or writes a resource.
struct RenderGraphAccess {
  uint32_t resource = 0;
  vk::ImageLayout layout = vk::ImageLayout::eUndefined;
  vk::AccessFlags2 access = {};
  vk::PipelineStageFlags2 stages = {};
};

// A render graph pass and its declared resource usage.
struct RenderGraphPass {
  std::string name;

  // Optional pipeline data for consumers that want to store it here.
  Pipeline pipeline;
  vk::DescriptorSetLayout setLayout = nullptr;
  vk::DescriptorSet descriptorSet = nullptr;

  // Declared resource dependencies for barrier planning.
  std::vector<RenderGraphAccess> reads;
  std::vector<RenderGraphAccess> writes;

  // Recording callback invoked during execute().
  std::function<void(vk::CommandBuffer cmd, uint32_t frameIndex)> record;
};

// Minimal render graph: manages images, barriers, and pass recording order.
class RenderGraph {
public:
  void init(Images &img) {
    this->img = &img;
  }

  // Add a pass and return it for configuration.
  RenderGraphPass &addPass(std::string name);
  // Create an owned image resource.
  uint32_t addImage(const RenderGraphResourceDesc &desc);
  // Register a non-owned image resource (e.g. swapchain or external target).
  uint32_t addExternalImage(const RenderGraphResourceDesc &desc, vk::raii::Image &image, vk::raii::ImageView &view);

  const RenderGraphResource &getResource(uint32_t id) const;

  // Allocate resources and build/validate transitions.
  void compile();
  // Emit barriers and invoke pass callbacks in order.
  void execute(vk::CommandBuffer cmd, uint32_t frameIndex);
  // Clear all passes/resources for a fresh build.
  void clear();

private:
  Images *img = nullptr;
  std::vector<RenderGraphResource> resources;
  std::vector<RenderGraphPass> passes;
};
