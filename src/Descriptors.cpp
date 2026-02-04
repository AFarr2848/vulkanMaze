#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vulkan/vulkan.hpp"
void Descriptors::createGlobalDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);

  vk::DescriptorSetAllocateInfo allocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data()

  };

  descriptorSets.clear();
  descriptorSets = cxt->device.allocateDescriptorSets(allocInfo);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo camInfo{
        .buffer = buf->globalUBOs[i],
        .offset = 0,
        .range = sizeof(GlobalUBO)

    };

    vk::WriteDescriptorSet camWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &camInfo

    };

    std::vector<vk::WriteDescriptorSet> writes = {camWrite};

    cxt->device.updateDescriptorSets(writes, {});
  }
}

void Descriptors::createObjDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> lightLayouts(MAX_FRAMES_IN_FLIGHT, *lightSetLayout);
  std::vector<vk::DescriptorSetLayout> transformLayouts(MAX_FRAMES_IN_FLIGHT, *transformSetLayout);
  vk::DescriptorSetAllocateInfo lightAllocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(lightLayouts.size()),
      .pSetLayouts = lightLayouts.data()

  };

  vk::DescriptorSetAllocateInfo transformAllocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(transformLayouts.size()),
      .pSetLayouts = transformLayouts.data()

  };

  lightDescriptorSets.clear();
  lightDescriptorSets = cxt->device.allocateDescriptorSets(lightAllocInfo);
  transformDescriptorSets.clear();
  transformDescriptorSets = cxt->device.allocateDescriptorSets(transformAllocInfo);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo lightsInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = 0,
        .range = sizeof(SSBOLight) * MAX_LIGHTS};

    vk::WriteDescriptorSet lightsWrite{
        .dstSet = lightDescriptorSets.at(i),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &lightsInfo};

    vk::DescriptorBufferInfo transformInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = sizeof(SSBOLight) * MAX_LIGHTS,
        .range = sizeof(glm::mat4) * MAX_TRANSFORMS};

    vk::WriteDescriptorSet transformWrite{
        .dstSet = transformDescriptorSets.at(i),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &transformInfo

    };

    std::vector<vk::WriteDescriptorSet> writes = {lightsWrite, transformWrite};

    cxt->device.updateDescriptorSets(writes, {});
  }
}

void Descriptors::createObjectDescriptorSetLayout() {
  vk::DescriptorSetLayoutBinding lightLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr

  };
  vk::DescriptorSetLayoutBinding transformLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };

  std::vector<vk::DescriptorSetLayoutBinding> lightLayouts = {lightLayoutBinding};
  std::vector<vk::DescriptorSetLayoutBinding> transformLayouts = {transformLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo lightLayoutInfo{
      .flags = {},
      .bindingCount = 1,
      .pBindings = lightLayouts.data()

  };
  vk::DescriptorSetLayoutCreateInfo transformLayoutInfo{
      .flags = {},
      .bindingCount = 1,
      .pBindings = transformLayouts.data()

  };

  lightSetLayout = vk::raii::DescriptorSetLayout(cxt->device, lightLayoutInfo);
  transformSetLayout = vk::raii::DescriptorSetLayout(cxt->device, transformLayoutInfo);
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
      .descriptorCount = 10

  };

  vk::DescriptorPoolSize poolSizeObj{
      .type = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT * 2

  };

  std::vector<vk::DescriptorPoolSize> poolSizes = {poolSizeUniform, poolSizeMat, poolSizeObj};

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT * 3 + 10,
      .poolSizeCount = 3,
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
