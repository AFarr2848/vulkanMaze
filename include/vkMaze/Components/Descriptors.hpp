#pragma once
class VulkanContext;
class Buffers;

class Descriptors {

public:
  void init(VulkanContext &cxt, Buffers &buf) {
    this->cxt = &cxt;
    this->buf = &buf;
  }
  vk::raii::DescriptorPool descriptorPool = nullptr;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  vk::raii::DescriptorSetLayout matSetLayout = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;

  void createGlobalDescriptorSets();
  void createGlobalDescriptorSetLayout();
  void createDescriptorPool();
  void createMaterialDescriptorSetLayout();
  vk::raii::DescriptorSet createMaterialDescriptorSet(vk::ImageView imageView, vk::Sampler sampler);

private:
  VulkanContext *cxt;
  Buffers *buf;
};
