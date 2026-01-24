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
  vk::raii::DescriptorSetLayout objSetLayout = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  std::vector<vk::raii::DescriptorSet> objDescriptorSets;

  void createGlobalDescriptorSets();
  void createGlobalDescriptorSetLayout();
  void createDescriptorPool();
  void createMaterialDescriptorSetLayout();
  void createObjectDescriptorSetLayout();
  vk::raii::DescriptorSet createMaterialDescriptorSet(vk::ImageView imageView, vk::Sampler sampler);
  void createObjDescriptorSets();

private:
  VulkanContext *cxt;
  Buffers *buf;
};
