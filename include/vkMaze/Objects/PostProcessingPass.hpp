#pragma once
class Descriptors;
class VulkanContext;
class Swapchain;
class Buffers;
class ShapeManager;
class LightManager;
class Images;
class Pipeline;

struct PPPassInfo {
  Pipeline &pipeline;
};

class PostProcessingPass {
public:
  void init(PPPassInfo info, VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Buffers &buf, Images &img) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->buf = &buf;
    this->img = &img;
    this->pipeline = &info.pipeline;
  }

  void drawScreen(vk::raii::CommandBuffer &cmd, uint32_t currentFrame);
  void createPPPDscSets(std::vector<vk::raii::ImageView> &imageViews, vk::raii::Sampler &sampler);
  void record(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, vk::raii::ImageView &colorView, vk::raii::ImageView &depthView);

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
  Pipeline *pipeline;
};
