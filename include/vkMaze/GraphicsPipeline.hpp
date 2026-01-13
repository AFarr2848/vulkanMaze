#pragma once
class VulkanContext;
class Descriptors;
class Swapchain;
class Images;

class GraphicsPipeline {
public:
  void init(VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Images &img) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->img = &img;
  }

  void cleanup(VkDevice device);

  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;

  void createGraphicsPipeline();
  [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

private:
  VulkanContext *cxt;
  Descriptors *dsc;
  Swapchain *swp;
  Images *img;
};
