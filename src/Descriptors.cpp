#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vulkan/vulkan.hpp"
#include <iostream>
void Descriptors::createGlobalDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
  std::printf("created layouts\n");

  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()

  };
  std::printf("allocInfo complete\n");

  descriptorSets.clear();
  descriptorSets = cxt->device.allocateDescriptorSets(allocInfo);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo camInfo{
        .buffer = buf->globalUBOs[i],
        .offset = 0,
        .range = sizeof(GlobalUBO)

    };
    std::cout << "CamInfo complete" << std::endl;

    vk::WriteDescriptorSet camWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &camInfo

    };
    std::cout << "CamWrite complete" << std::endl;

    std::vector<vk::WriteDescriptorSet> writes = {camWrite};

    std::printf("updating descriptor sets\n");
    cxt->device.updateDescriptorSets(writes, {});
  }
}
void Descriptors::createGlobalDescriptorSetLayout() {
  vk::DescriptorSetLayoutBinding uboLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };

  std::vector<vk::DescriptorSetLayoutBinding> layouts = {uboLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .flags = {},
      .bindingCount = 1,
      .pBindings = layouts.data()

  };

  descriptorSetLayout = vk::raii::DescriptorSetLayout(cxt->device, layoutInfo);
}

void Descriptors::createMaterialDescriptorSetLayout() {
  vk::DescriptorSetLayoutBinding samplerBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .bindingCount = 1,
      .pBindings = &samplerBinding};

  matSetLayout = vk::raii::DescriptorSetLayout(cxt->device, layoutInfo);
}

void Descriptors::createDescriptorPool() {
  vk::DescriptorPoolSize poolSizeUniform{
      .type = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT

  };

  vk::DescriptorPoolSize poolSizeMat{
      .type = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 10};

  std::vector<vk::DescriptorPoolSize> poolSizes = {poolSizeUniform, poolSizeMat};

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT + 10,
      .poolSizeCount = 2,
      .pPoolSizes = poolSizes.data()

  };

  descriptorPool = vk::raii::DescriptorPool(cxt->device, poolInfo);
}

vk::raii::DescriptorSet Descriptors::createMaterialDescriptorSet(vk::ImageView imageView, vk::Sampler sampler) {
  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = *descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &*matSetLayout};

  auto set = std::move(cxt->device.allocateDescriptorSets(allocInfo)[0]);

  vk::DescriptorImageInfo imageInfo{
      .sampler = sampler,
      .imageView = imageView,
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet write{
      .dstSet = *set,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  cxt->device.updateDescriptorSets(write, nullptr);
  return set;
}
