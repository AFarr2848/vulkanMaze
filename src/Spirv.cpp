

#include "vkMaze/Components/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <spirv-reflect/spirv_reflect.h>
#include <vkMaze/Components/Spirv.hpp>

void SpirvReflectPipeline::createSetLayouts() {
  layouts = std::vector<vk::DescriptorSetLayout>(bindings.size());
  for (int i = 0; i < bindings.size(); i++) {
    vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = static_cast<uint32_t>(bindings.at(i).size()),
        .pBindings = bindings.at(i).data()};
    layouts[i] = cxt->device.createDescriptorSetLayout(createInfo);
  }
}

void SpirvReflectPipeline::makePipelineLayout(vk::PipelineLayout &layout) {

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pcRange

  };
}

void SpirvReflectPipeline::readSpirv(const std::filesystem::path &path, SpvReflectShaderModule &module) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);

  size_t size = file.tellg();
  file.seekg(0);

  std::vector<uint32_t> buffer(size / sizeof(uint32_t));
  file.read(reinterpret_cast<char *>(buffer.data()), size);

  spvReflectCreateShaderModule(
      buffer.size() * sizeof(uint32_t),
      buffer.data(),
      &module);
}

void SpirvReflectPipeline::getPushConstantInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag) {
  uint32_t pushCount = 0;
  spvReflectEnumeratePushConstantBlocks(&module, &pushCount, nullptr);

  std::vector<SpvReflectBlockVariable *> pushes(pushCount);
  spvReflectEnumeratePushConstantBlocks(&module, &pushCount, pushes.data());

  uint32_t pushMin = UINT32_MAX;
  uint32_t pushMax = 0;
  for (auto push : pushes) {
    pushMin = std::min(pushMin, push->offset);
    pushMax = std::min(pushMax, push->offset + push->size);
  }
  vk::PushConstantRange range{
      .stageFlags = stageFlag | pcRange.stageFlags,
      .offset = std::min(pushMin, pcRange.offset),
      .size = std::max(pushMax - pushMin, pcRange.size)

  };
}

void SpirvReflectPipeline::getBindingInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag) {
  SpirvInfo info;

  spvReflectEnumerateDescriptorSets(&module, &info.dscSetCount, nullptr);
  info.spvDscSets = std::vector<SpvReflectDescriptorSet *>(info.dscSetCount);
  spvReflectEnumerateDescriptorSets(&module, &info.dscSetCount, info.spvDscSets.data());

  for (int i = 0; i < info.dscSetCount; i++) {
    const SpvReflectDescriptorSet *reflSet = info.spvDscSets.at(i);

    for (int j = 0; j < reflSet->binding_count; j++) {
      const SpvReflectDescriptorBinding *spvBinding = reflSet->bindings[j];
      vk::DescriptorSetLayoutBinding vkLayout{
          .binding = spvBinding->binding,
          .descriptorType = static_cast<vk::DescriptorType>(spvBinding->descriptor_type),
          .descriptorCount = spvBinding->count,
          .stageFlags = stageFlag,
          .pImmutableSamplers = nullptr

      };

      bindings.at(i).push_back(vkLayout);
    }
  }
}

// as of 2/15/26 this is the most evil function in this project
void mergeStageFlags(std::vector<std::vector<vk::DescriptorSetLayoutBinding>> &bindings) {
  for (int i = 0; i < bindings.size(); i++) {
    for (int j = 0; j < bindings.at(i).size(); j++) {
      for (int k = j + 1; k < bindings.at(i).size(); k++)
        // if there are two matching bindings in the same set, turn it into one binding for both shaders
        if (bindings.at(i).at(j).binding == bindings.at(i).at(k).binding) {
          bindings.at(i).at(j).stageFlags |= bindings.at(i).at(k).stageFlags;
          bindings.at(i).erase(bindings.at(i).begin() + k);
        }
    }
  }
}
