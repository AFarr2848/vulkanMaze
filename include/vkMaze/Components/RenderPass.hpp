#pragma once
class Descriptors;
class VulkanContext;
class Swapchain;
class Buffers;
class ShapeManager;
class LightManager;

struct RenderPassInfo {
  ShapeManager &shapes;
  LightManager &lights;
};

class RenderPass {
public:
  void init(RenderPassInfo info, VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Buffers &buf) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->buf = &buf;
    this->shapes = &info.shapes;
    this->lights = &info.lights;
  }

  void record(vk::raii::CommandBuffer &cmd, uint32_t frameIndex, vk::raii::ImageView &colorView, vk::raii::ImageView &depthView);

  void drawScreen(vk::raii::CommandBuffer &cmd, uint32_t currentFrame);
  void createGlobalDscSets();

private:
  void init();

  Descriptors *dsc;
  VulkanContext *cxt;
  Swapchain *swp;
  Buffers *buf;

  vk::DescriptorSetLayout globalLayout;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  vk::RenderingInfo renderingInfo;
  ShapeManager *shapes;
  LightManager *lights;
};
