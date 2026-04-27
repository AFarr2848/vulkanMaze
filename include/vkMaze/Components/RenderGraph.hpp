#pragma once
#include "vkMaze/Objects/Pipelines.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Images;
class Swapchain;
class Descriptors;
class VulkanContext;
class Buffers;
class ShapeManager;
class LightManager;
class VulkanImgui;

enum PassType {
  MAIN_PASS = 0,
  POST_PASS = 1,
  GUI_PASS = 2
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

struct RenderGraphReadOverride {
  std::string shaderResource;
  std::string rgResource;
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
  bool hasScratchRead = false;
  bool hasDepthRead = false;
};

struct RenderGraphPassDsc {
  std::string name;
  PassType type;
  PipelineDsc pipelineDsc;

  std::vector<RenderGraphReadOverride> readOverrides;
  std::vector<RenderGraphAccess> writes;
};

class RenderGraph {
public:
  void init(ShapeManager &shapes, LightManager &lights, VulkanContext &cxt, Swapchain &swp, Images &img, Descriptors &dsc, Buffers &buf, VulkanImgui &gui) {
    this->cxt = &cxt;
    this->img = &img;
    this->swp = &swp;
    this->dsc = &dsc;
    this->buf = &buf;
    this->shapes = &shapes;
    this->lights = &lights;
    this->gui = &gui;
    createPingPongResources();
  }

  RenderGraphPassDsc &addPass(std::string name, const PipelineDsc dsc, std::vector<RenderGraphReadOverride> reads, std::vector<RenderGraphAccess> writes, PassType type);
  void addImage(const RenderGraphResourceDesc &desc);
  void addExternalImage(const RenderGraphResourceDesc &desc, vk::Image image, vk::raii::ImageView *view);

  RenderGraphResource &getResource(std::string id, int imageIndex = -1);

  size_t getUncompiledPassCount() const { return uncompiledPasses.size(); }
  RenderGraphPass *getCompiledPass(size_t index) { return index < compiledPasses.size() ? &compiledPasses.at(index) : nullptr; }
  RenderGraphPassDsc *getUncompiledPass(size_t index) { return index < uncompiledPasses.size() ? &uncompiledPasses.at(index) : nullptr; }
  const std::vector<RenderGraphPass> &getCompiledPasses() const { return compiledPasses; }
  bool movePass(size_t from, size_t to);
  bool removePass(size_t index);
  std::vector<std::string> listResourceIds() const;
  const std::vector<std::vector<uint32_t>> &getPassDag() const { return passDag; }

  void compile();

  void execute(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, uint32_t imageIndex);

  void clear();

  void makeGlobalDscSets();

  void makePostImgDscSets(RenderGraphPass &pass);

  void setFinalImage(vk::Image *img) { finalImage = img; }

  VulkanImgui *gui = nullptr;
  Swapchain *swp = nullptr;
  Buffers *buf = nullptr;

  ShapeManager *shapes = nullptr;
  LightManager *lights = nullptr;

  std::vector<vk::raii::DescriptorSet> globalDscSets;

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
  uint32_t currentScratch = 0;
  uint32_t currentDepth = 0;

  Images *img = nullptr;
  VulkanContext *cxt = nullptr;
  Descriptors *dsc = nullptr;
  std::unordered_map<std::string, RenderGraphResource> resources;
  std::vector<RenderGraphPass> compiledPasses;
  std::vector<RenderGraphPassDsc> uncompiledPasses;
  std::vector<std::vector<uint32_t>> passDag;

  std::unordered_map<uint32_t, std::vector<BarrierPoint>> resourceBarriers;

  vk::raii::Sampler sampler = nullptr;

  vk::Image *finalImage = nullptr;

  void makeDescriptorSets();
  RenderGraphPass createPassFromDsc(RenderGraphPassDsc &dsc);
  std::vector<std::vector<uint32_t>> makePassDag(const std::vector<RenderGraphPass> &passes) const;
  std::vector<int> flattenDAG(const std::vector<std::vector<uint32_t>> &dag) const;

  void createPingPongResources();
};
