#pragma once
#include "vkMaze/Objects/Pipelines.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Images;
class Swapchain;
class Descriptors;
class VulkanContext;
class Buffers;
class ShapeManager;
class LightManager;

enum PassType {
  MAIN_PASS = 0,
  POST_PASS = 1
};

// Describes how a render graph image should be created or bound.
struct RenderGraphResourceDesc {
  std::string name;
  vk::Format format = vk::Format::eUndefined;
  vk::Extent2D extent = {};
  vk::ImageUsageFlags usage = {};
  vk::ImageAspectFlagBits aspect = vk::ImageAspectFlagBits::eColor;
  vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
  vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
  bool isExternal = false;
};

// Concrete resource instance tracked by the graph.
struct RenderGraphResource {
  RenderGraphResourceDesc desc;

  vk::raii::DeviceMemory memory = nullptr;
  vk::ImageLayout layout = vk::ImageLayout::eUndefined;

  bool isExternal = false;

  vk::Image getImage() { return isExternal ? extImage : image; }
  vk::ImageView getImageView() { return isExternal ? *extView : imageView; }
  void setExternalImage(vk::Image img) { extImage = img; }
  void setExternalView(vk::raii::ImageView *view) { extView = view; }

  vk::raii::Image image = nullptr;
  vk::raii::ImageView imageView = nullptr;

private:
  vk::Image extImage = nullptr;
  vk::raii::ImageView *extView = nullptr;
};

struct RenderGraphAccess {
  std::string resource;
  vk::ImageLayout layout = vk::ImageLayout::eUndefined;
  vk::AccessFlags2 access = {};
  vk::PipelineStageFlags2 stages = {};
  auto operator<=>(const RenderGraphAccess &) const = default;
};

struct RenderGraphPass {
  std::string name;
  PassType type;

  Pipeline pipeline;

  // Declared resource dependencies for barrier planning.
  std::vector<RenderGraphAccess> reads;
  std::vector<RenderGraphAccess> writes;

  std::vector<vk::raii::DescriptorSet> postImgDscSets;

  bool hasColorRead = false;
  bool hasDepthRead = false;
};

class RenderGraph {
public:
  void init(ShapeManager &shapes, LightManager &lights, VulkanContext &cxt, Swapchain &swp, Images &img, Descriptors &dsc, Buffers &buf) {
    this->cxt = &cxt;
    this->img = &img;
    this->swp = &swp;
    this->dsc = &dsc;
    this->buf = &buf;
    this->shapes = &shapes;
    this->lights = &lights;
    createPingPongResources();
  }

  RenderGraphPass &addPass(std::string name, const PipelineDsc dsc, std::vector<RenderGraphAccess> writes, PassType type);
  void addImage(const RenderGraphResourceDesc &desc);
  void addExternalImage(const RenderGraphResourceDesc &desc, vk::Image image, vk::raii::ImageView *view);

  RenderGraphResource &getResource(std::string id, int imageIndex = -1);

  void compile();

  void execute(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex);

  void clear();

  void makeGlobalDscSets();

  void makePostImgDscSets(RenderGraphPass &pass);

  void setFinalImage(vk::Image *img) { finalImage = img; }

private:
  struct BarrierPoint {
  public:
    std::string resource;
    vk::ImageLayout oldLayout;
    vk::ImageLayout newLayout;
    vk::AccessFlags2 oldAccess;
    vk::AccessFlags2 newAccess;
    vk::PipelineStageFlags2 oldStage;
    vk::PipelineStageFlags2 newStage;
    vk::ImageAspectFlags aspectFlags;
  };

  uint32_t currentColor = 0;
  uint32_t currentDepth = 0;

  Images *img = nullptr;
  Swapchain *swp = nullptr;
  VulkanContext *cxt = nullptr;
  Descriptors *dsc = nullptr;
  Buffers *buf = nullptr;
  std::unordered_map<std::string, RenderGraphResource> resources;
  std::vector<RenderGraphPass> passes;
  std::unordered_map<uint32_t, std::vector<BarrierPoint>> resourceBarriers;

  std::vector<vk::raii::DescriptorSet> globalDscSets;
  vk::raii::Sampler sampler = nullptr;
  ShapeManager *shapes = nullptr;
  LightManager *lights = nullptr;

  vk::Image *finalImage = nullptr;

  void makeDescriptorSets();

  void recordMain(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraphPass &pass);
  void recordPost(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex, RenderGraphPass &pass);

  void createPingPongResources();
};
