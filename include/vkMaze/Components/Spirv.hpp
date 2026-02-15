#pragma once
#include "spirv-reflect/spirv_reflect.h"
#include "vulkan/vulkan.hpp"
#include <filesystem>
#include <map>
#include <sys/types.h>
class VulkanContext;

class SpirvReflectPipeline {
public:
  // map to prevent sets used in different shader stages from making new sets
  // ik im gonna forget why I did this
  SpirvReflectPipeline(const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, vk::PipelineLayout &layout, VulkanContext &cxt) {
    this->cxt = &cxt;
    readSpirv(vertPath, vertModule);
    readSpirv(fragPath, fragModule);

    getBindingInfo(vertModule, vk::ShaderStageFlagBits::eVertex);
    getBindingInfo(fragModule, vk::ShaderStageFlagBits::eFragment);

    createSetLayouts();

    pcRange = vk::PushConstantRange{
        .offset = UINT32_MAX,
        .size = 0

    };
    getPushConstantInfo(fragModule, vk::ShaderStageFlagBits::eFragment);
    getPushConstantInfo(fragModule, vk::ShaderStageFlagBits::eFragment);

    makePipelineLayout(layout);
  }

  std::map<uint32_t, std::map<uint32_t, vk::DescriptorSetLayoutBinding>> bindingMaps;

  void getPushConstantInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag);
  void getBindingInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag);
  void readSpirv(const std::filesystem::path &path, SpvReflectShaderModule &module);
  void makePipelineLayout(vk::PipelineLayout &layout);
  void createSetLayouts();

private:
  std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings;
  std::vector<vk::DescriptorSetLayout> layouts;
  vk::PushConstantRange pcRange;
  SpvReflectShaderModule fragModule;
  SpvReflectShaderModule vertModule;
  VulkanContext *cxt;
};

class SpirvInfo {
public:
  uint32_t dscSetCount = 0;
  std::vector<SpvReflectDescriptorSet *> spvDscSets;
  std::vector<vk::DescriptorSetLayoutBinding *> vkDscSets;
};
