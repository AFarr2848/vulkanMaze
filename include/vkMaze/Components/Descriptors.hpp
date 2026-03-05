#pragma once
#include <vulkan/vulkan_raii.hpp>
class VulkanContext;
class Buffers;

class Descriptors {

public:
  void init(VulkanContext &cxt, Buffers &buf) {
    this->cxt = &cxt;
    this->buf = &buf;
  }
  vk::raii::DescriptorPool descriptorPool = nullptr;
  vk::raii::DescriptorSetLayout globalDescriptorSetLayout = nullptr;
  vk::raii::DescriptorSetLayout lightSetLayout = nullptr;
  vk::raii::DescriptorSetLayout transformSetLayout = nullptr;

  void createGlobalDescriptorSetLayout();
  void createDescriptorPool();
  void createPostSetLayout();
  std::vector<vk::raii::DescriptorSet> createLightDescriptorSets();
  std::vector<vk::raii::DescriptorSet> createTransformDescriptorSets();
  vk::raii::DescriptorSet createMaterialDescriptorSet(vk::raii::DescriptorSetLayout layout, vk::ImageView imageView, vk::Sampler sampler);
  void createGlobalDescriptorSets();
  vk::raii::DescriptorSet &getSet(uint32_t setNum, uint32_t currentFrame);

private:
  VulkanContext *cxt;
  Buffers *buf;
};
