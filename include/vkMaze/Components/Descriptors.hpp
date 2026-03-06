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
  vk::raii::DescriptorSetLayout globalSetLayout = nullptr;
  vk::raii::DescriptorSetLayout matSetLayout = nullptr;
  vk::raii::DescriptorSetLayout lightSetLayout = nullptr;
  vk::raii::DescriptorSetLayout ppSetLayout = nullptr;

  void createGlobalDescriptorSetLayout();
  void createDescriptorPool();
  void createMaterialDescriptorSetLayout();
  void createObjectDescriptorSetLayout();
  void createPostSetLayout();
  std::vector<vk::raii::DescriptorSet> createLightDescriptorSets();
  vk::raii::DescriptorSet createMaterialDescriptorSet(vk::ImageView imageView, vk::Sampler sampler);
  vk::raii::DescriptorSet &getSet(uint32_t setNum, uint32_t currentFrame);

private:
  VulkanContext *cxt;
  Buffers *buf;
};
