#pragma once
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Objects/UBOs.hpp"
class Descriptors;
class VulkanContext;
class Swapchain;
class Buffers;
class ShapeManager;
class LightManager;
class Images;
class Pipeline;

struct PPPassInfo {
  std::string fragPath;
  std::string vertPath;
  std::string outputName;
};

class PostProcessingPass {
public:
  PostProcessingPass(PPPassInfo info, VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Buffers &buf, Images &img) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->buf = &buf;
    this->img = &img;
    this->outputView = &img.getDataImageView(info.outputName);
    this->pipeline.init(cxt, dsc, swp, img);
    this->pipeline.createPipeline({.fragPath = info.fragPath,
                                   .vertPath = info.vertPath,
                                   .topology = vk::PrimitiveTopology::eTriangleList,
                                   .polygonMode = vk::PolygonMode::eFill,
                                   .cullModeFlags = vk::CullModeFlagBits::eNone});
  }

  void drawScreen(vk::raii::CommandBuffer &cmd, uint32_t currentFrame);
  void createPPPDscSets(vk::DescriptorSetLayout layout, std::vector<vk::raii::ImageView> &imageViews, vk::raii::Sampler &sampler);
  void record(vk::raii::CommandBuffer &cmd, uint32_t frameIndex);

private:
  void init();

  Descriptors *dsc;
  VulkanContext *cxt;
  Swapchain *swp;
  Buffers *buf;
  Images *img;

  vk::DescriptorSetLayout globalLayout;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  vk::RenderingInfo renderingInfo;
  Pipeline pipeline = Pipeline();
  vk::raii::ImageView *outputView;
};
