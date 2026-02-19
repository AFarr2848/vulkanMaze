#pragma once
#include <filesystem>
class VulkanContext;
class Descriptors;
class Swapchain;
class Images;
class ShaderResource;

struct PipelineDsc {
  const std::filesystem::path &fragPath;
  const std::filesystem::path &vertPath;
  vk::PrimitiveTopology topology;
  vk::PolygonMode polygonMode;
  vk::CullModeFlags cullModeFlags;
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
    this->swp = &swp;
    this->img = &img;
  }

  void createPipeline(const PipelineDsc &dsc);
  bool usesSet(uint32_t setNum);

  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;
  std::vector<ShaderResource> shaderResources;
  vk::PushConstantRange pcRange;
  bool hasPushConstants;

  void createGraphicsPipeline();
  [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

private:
  VulkanContext *cxt;
  Swapchain *swp;
  Images *img;
};
