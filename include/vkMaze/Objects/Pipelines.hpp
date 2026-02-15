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

  std::vector<vk::DescriptorSetLayout> setLayouts;
};
struct InputAssemblyDesc {
  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  bool primitiveRestart = false;
};

struct RasterizationDesc {
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
  vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
};

struct DepthStencilDesc {
  bool depthTest = true;
  bool depthWrite = true;
  vk::CompareOp compareOp = vk::CompareOp::eLess;
};

struct ColorDesc {
  int colorAttachmentCount = 1;
  vk::Format colorAttachmentFormats;
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
