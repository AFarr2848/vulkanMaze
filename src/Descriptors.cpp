#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

std::vector<vk::raii::DescriptorSet> Descriptors::createGlobalDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> lightLayouts(MAX_FRAMES_IN_FLIGHT, *lightSetLayout);
  vk::DescriptorSetAllocateInfo lightAllocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(lightLayouts.size()),
      .pSetLayouts = lightLayouts.data()

  };

  std::vector<vk::raii::DescriptorSet> dscSets;
  dscSets = cxt->device.allocateDescriptorSets(lightAllocInfo);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo pointInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = 0,
        .range = sizeof(SSBOLight) * MAX_POINT_LIGHTS};
    vk::DescriptorBufferInfo dirInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = pointInfo.range,
        .range = sizeof(SSBOLight) * MAX_DIR_LIGHTS};
    vk::DescriptorBufferInfo spotInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = dirInfo.range + pointInfo.range,
        .range = sizeof(SSBOLight) * MAX_SPOT_LIGHTS};

    vk::WriteDescriptorSet pointWrite{
        .dstSet = dscSets.at(i),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &pointInfo};

    vk::WriteDescriptorSet dirWrite{
        .dstSet = dscSets.at(i),
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &dirInfo};

    vk::WriteDescriptorSet spotWrite{
        .dstSet = dscSets.at(i),
        .dstBinding = 2,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &spotInfo};

    std::vector<vk::WriteDescriptorSet> writes = {pointWrite, dirWrite, spotWrite};

    cxt->device.updateDescriptorSets(writes, {});
  }
  return dscSets;
}

std::vector<vk::raii::DescriptorSet> Descriptors::createTransformDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> transformLayouts(MAX_FRAMES_IN_FLIGHT, *transformSetLayout);

  vk::DescriptorSetAllocateInfo transformAllocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(transformLayouts.size()),
      .pSetLayouts = transformLayouts.data()

  };

  std::vector<vk::raii::DescriptorSet> dscSets;
  dscSets = cxt->device.allocateDescriptorSets(transformAllocInfo);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DescriptorBufferInfo transformInfo{
        .buffer = buf->SSBOs.at(i),
        .offset = sizeof(SSBOLight) * (MAX_DIR_LIGHTS + MAX_SPOT_LIGHTS + MAX_POINT_LIGHTS),
        .range = sizeof(glm::mat4) * MAX_TRANSFORMS};

    vk::WriteDescriptorSet transformWrite{
        .dstSet = dscSets.at(i),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &transformInfo

    };

    std::vector<vk::WriteDescriptorSet> writes = {transformWrite};

    cxt->device.updateDescriptorSets(writes, {});
  }
  return dscSets;
}

void Descriptors::createGlobalDescriptorSetLayout() {
  vk::DescriptorSetLayoutBinding cameraLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };

  vk::DescriptorSetLayoutBinding pointLightLayoutBinding{
      .binding = 1,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr

  };

  vk::DescriptorSetLayoutBinding dirLightLayoutBinding{
      .binding = 2,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr

  };
  vk::DescriptorSetLayoutBinding spotLightLayoutBinding{
      .binding = 3,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr

  };
  vk::DescriptorSetLayoutBinding transformLayoutBinding{
      .binding = 4,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .pImmutableSamplers = nullptr

  };

  std::vector<vk::DescriptorSetLayoutBinding> layouts = {cameraLayoutBinding, dirLightLayoutBinding, pointLightLayoutBinding, spotLightLayoutBinding, transformLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{
      .flags = {},
      .bindingCount = static_cast<uint32_t>(layouts.size()),
      .pBindings = layouts.data()

  };

  globalDescriptorSetLayout = vk::raii::DescriptorSetLayout(cxt->device, layoutInfo);
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
      .descriptorCount = MAX_FRAMES_IN_FLIGHT + MAX_FRAMES_IN_FLIGHT * 3

  };

  vk::DescriptorPoolSize poolSizePP{
      .type = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT};

  std::vector<vk::DescriptorPoolSize> poolSizes = {poolSizeUniform, poolSizeMat, poolSizeObj, poolSizePP};

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT * 6 + 10,
      .poolSizeCount = 4,
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
