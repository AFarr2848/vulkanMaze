#pragma once
#include <spirv_reflect.h>
#include <filesystem>

class VulkanContext;
struct ShaderResource;

class SpirvReflectPipeline {
public:
  SpirvReflectPipeline(const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, vk::raii::PipelineLayout &layout, VulkanContext &cxt);

  vk::raii::ShaderModule vkVertModule = nullptr;
  vk::raii::ShaderModule vkFragModule = nullptr;

  std::vector<ShaderResource> shaderResources;

  bool hasPushConstants = false;
  vk::PushConstantRange pcRange;
  std::vector<vk::raii::DescriptorSetLayout> layouts;

private:
  void getPushConstantInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag);
  void getBindingInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag);
  void readSpirv(const std::filesystem::path &path, SpvReflectShaderModule &module, vk::raii::ShaderModule &vkModule);
  void createSetLayouts();
  void makePipelineLayout(vk::raii::PipelineLayout &layout);
  void mergeStageFlags();
  void createShaderResources();

  std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings;
  std::vector<std::vector<std::string>> names;
  VulkanContext *cxt;
  SpvReflectShaderModule spvFragModule;
  SpvReflectShaderModule spvVertModule;
};
