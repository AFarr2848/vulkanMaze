#include "vkMaze/Descriptors.hpp"
#include "vkMaze/EngineConfig.hpp"
#include "vkMaze/VulkanContext.hpp"
#include "vkMaze/UBOs.hpp"
#include "vkMaze/Buffers.hpp"
#include <iostream>
void Descriptors::createDescriptorSets() {
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
    std::cout << buf->materialUBOs.size() << std::endl;

    vk::DescriptorBufferInfo matInfo{
        .buffer = buf->materialUBOs[i],
        .offset = 0,
        .range = sizeof(MaterialUBO)

    };
    std::cout << "matInfo complete" << std::endl;

    vk::WriteDescriptorSet camWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &camInfo

    };
    std::cout << "CamWrite complete" << std::endl;

    vk::WriteDescriptorSet matWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &matInfo

    };
    std::cout << "MatWrite complete" << std::endl;

    std::vector<vk::WriteDescriptorSet> writes = {camWrite, matWrite};

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
  vk::DescriptorSetLayoutBinding uboMaterialLayoutBinding{
      .binding = 1,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };

  std::vector<vk::DescriptorSetLayoutBinding> layouts = {uboLayoutBinding, uboMaterialLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .flags = {},
      .bindingCount = 2,
      .pBindings = layouts.data()

  };

  descriptorSetLayout = vk::raii::DescriptorSetLayout(cxt->device, layoutInfo);
}

void Descriptors::createDescriptorPool() {
  vk::DescriptorPoolSize poolSizeUniform{
      .type = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT * 2

  };

  std::vector<vk::DescriptorPoolSize> poolSizes = {poolSizeUniform};

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes = poolSizes.data()

  };

  descriptorPool = vk::raii::DescriptorPool(cxt->device, poolInfo);
}
