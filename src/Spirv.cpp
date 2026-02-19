

#include "vkMaze/Components/VulkanContext.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spirv-reflect/spirv_reflect.h>
#include <vkMaze/Components/Spirv.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vkMaze/Objects/UBOs.hpp>
#include <vulkan/vulkan_to_string.hpp>

SpirvReflectPipeline::SpirvReflectPipeline(const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, vk::raii::PipelineLayout &layout, VulkanContext &cxt) {
  this->cxt = &cxt;
  readSpirv(vertPath, spvVertModule, vkVertModule);
  readSpirv(fragPath, spvFragModule, vkFragModule);
  getBindingInfo(spvVertModule, vk::ShaderStageFlagBits::eVertex);
  getBindingInfo(spvFragModule, vk::ShaderStageFlagBits::eFragment);
  mergeStageFlags();

  createSetLayouts();

  pcRange = vk::PushConstantRange{
      .stageFlags = vk::ShaderStageFlags{},
      .offset = UINT32_MAX,
      .size = 0

  };
  getPushConstantInfo(spvVertModule, vk::ShaderStageFlagBits::eVertex);
  getPushConstantInfo(spvFragModule, vk::ShaderStageFlagBits::eFragment);

  createShaderResources();

  makePipelineLayout(layout);

  spvReflectDestroyShaderModule(&spvFragModule);
  spvReflectDestroyShaderModule(&spvVertModule);
}
void SpirvReflectPipeline::createShaderResources() {
  for (int i = 0; i < names.size(); i++) {
    for (int j = 0; j < names.at(i).size(); j++) {
      shaderResources.push_back({

          .set = static_cast<uint32_t>(i),
          .binding = static_cast<uint32_t>(j),
          .name = names.at(i).at(j)

      });
    }
  }
}

void SpirvReflectPipeline::createSetLayouts() {
  layouts.clear();
  for (int i = 0; i < bindings.size(); i++) {
    vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = static_cast<uint32_t>(bindings.at(i).size()),
        .pBindings = bindings.at(i).data()};

    layouts.push_back(cxt->device.createDescriptorSetLayout(createInfo));
  }
}

void SpirvReflectPipeline::makePipelineLayout(vk::raii::PipelineLayout &layout) {
  std::vector<vk::DescriptorSetLayout> rawLayouts;
  rawLayouts.reserve(layouts.size());
  for (auto &l : layouts)
    rawLayouts.push_back(*l);

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(rawLayouts.size()),
      .pSetLayouts = rawLayouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pcRange

  };

  if (hasPushConstants == false) {
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
  }
  layout = vk::raii::PipelineLayout(cxt->device, pipelineLayoutInfo);
}

void SpirvReflectPipeline::readSpirv(const std::filesystem::path &path, SpvReflectShaderModule &module, vk::raii::ShaderModule &vkModule) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    throw std::runtime_error("Failed to open SPIR-V file");

  size_t size = file.tellg();
  file.seekg(0);

  std::vector<uint32_t> buffer(size / sizeof(uint32_t));
  file.read(reinterpret_cast<char *>(buffer.data()), size);

  SpvReflectResult result = spvReflectCreateShaderModule(
      buffer.size() * sizeof(uint32_t),
      buffer.data(),
      &module

  );

  if (result != SPV_REFLECT_RESULT_SUCCESS)
    throw std::runtime_error("spvReflectCreateShaderModule failed: " + std::to_string(result));

  vk::ShaderModuleCreateInfo vkModuleInfo{
      .codeSize = size,
      .pCode = buffer.data()};
  vkModule = vk::raii::ShaderModule{cxt->device, vkModuleInfo};
}

void SpirvReflectPipeline::getPushConstantInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag) {
  uint32_t pushCount = 0;
  spvReflectEnumeratePushConstantBlocks(&module, &pushCount, nullptr);
  std::cout << "  getPushConstantInfo called with stage " << static_cast<uint32_t>(stageFlag)
            << ", found " << pushCount << " push constant blocks" << std::endl;

  if (pushCount == 0) {
    std::cout << "No push constants" << std::endl;
    return;
  }

  std::vector<SpvReflectBlockVariable *> pushes(pushCount);
  spvReflectEnumeratePushConstantBlocks(&module, &pushCount, pushes.data());

  uint32_t pushMin = UINT32_MAX, pushMax = 0;
  for (auto push : pushes) {
    hasPushConstants = true;
    pushMin = std::min(pushMin, push->offset);
    pushMax = std::max(pushMax, push->offset + push->size);
    std::cout << "    Block: offset=" << push->offset << " size=" << push->size << std::endl;
  }

  uint32_t newMin = (pcRange.size == 0) ? pushMin : std::min(pushMin, pcRange.offset);
  uint32_t newMax = (pcRange.size == 0) ? pushMax : std::max(pushMax, pcRange.offset + pcRange.size);

  pcRange.offset = newMin;
  pcRange.size = newMax - newMin;
  std::cout << "  Before |=: pcRange.stageFlags.m_mask = " << vk::to_string(pcRange.stageFlags) << std::endl;
  pcRange.stageFlags |= stageFlag;
  std::cout << "  After |=: pcRange.stageFlags.m_mask = " << vk::to_string(pcRange.stageFlags) << std::endl;
}

void SpirvReflectPipeline::getBindingInfo(SpvReflectShaderModule &module, vk::ShaderStageFlagBits stageFlag) {
  uint32_t dscSetCount = 0;
  spvReflectEnumerateDescriptorSets(&module, &dscSetCount, nullptr);
  std::vector<SpvReflectDescriptorSet *> spvDscSets(dscSetCount);
  spvReflectEnumerateDescriptorSets(&module, &dscSetCount, spvDscSets.data());

  for (int i = 0; i < dscSetCount; i++) {
    const SpvReflectDescriptorSet *reflSet = spvDscSets.at(i);
    uint32_t setNumber = reflSet->set;

    if (bindings.size() <= setNumber) {
      bindings.resize(setNumber + 1);
      names.resize(setNumber + 1);
    }

    for (int j = 0; j < reflSet->binding_count; j++) {
      const SpvReflectDescriptorBinding *spvBinding = reflSet->bindings[j];
      vk::DescriptorSetLayoutBinding vkLayout{
          .binding = spvBinding->binding,
          .descriptorType = static_cast<vk::DescriptorType>(spvBinding->descriptor_type),
          .descriptorCount = spvBinding->count,
          .stageFlags = stageFlag,
          .pImmutableSamplers = nullptr};

      bindings.at(setNumber).push_back(vkLayout);
      names.at(setNumber).push_back(spvBinding->name);
    }
  }
}

// as of 2/15/26 this is the most evil function in this project
void SpirvReflectPipeline::mergeStageFlags() {
  for (int i = 0; i < bindings.size(); i++) {
    for (int j = 0; j < bindings.at(i).size(); j++) {
      for (int k = j + 1; k < bindings.at(i).size(); k++)
        // if there are two matching bindings in the same set, turn it into one binding for both shaders
        if (bindings.at(i).at(j).binding == bindings.at(i).at(k).binding) {
          bindings.at(i).at(j).stageFlags |= bindings.at(i).at(k).stageFlags;
          bindings.at(i).erase(bindings.at(i).begin() + k);
          names.at(i).erase(names.at(i).begin() + k);
        }
    }
  }
}
