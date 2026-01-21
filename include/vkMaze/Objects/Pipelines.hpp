#pragma once
class VulkanContext;
class Descriptors;
class Swapchain;
class Images;

struct PipelineDsc {
  std::string shaderPath;
  vk::PrimitiveTopology topology;
  vk::PolygonMode polygonMode;
  vk::CullModeFlags cullModeFlags;

  vk::DescriptorSetLayout descriptorSetLayout;
};

class Pipeline {
public:
  void init(VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Images &img) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->img = &img;
  }

  void createPipeline(const PipelineDsc &dsc);

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
