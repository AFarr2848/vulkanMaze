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
  vk::raii::DescriptorSetLayout lightSetLayout = nullptr;
  vk::raii::DescriptorSetLayout transformSetLayout = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  std::vector<vk::raii::DescriptorSet> lightDescriptorSets;
  std::vector<vk::raii::DescriptorSet> transformDescriptorSets;

  void createGlobalDescriptorSets();
  void createGlobalDescriptorSetLayout();
  void createDescriptorPool();
  void createMaterialDescriptorSetLayout();
  void createObjectDescriptorSetLayout();
  vk::raii::DescriptorSet createMaterialDescriptorSet(vk::ImageView imageView, vk::Sampler sampler);
  void createObjDescriptorSets();
  vk::raii::DescriptorSet &getSet(uint32_t setNum, uint32_t currentFrame);

private:
  VulkanContext *cxt;
  Buffers *buf;
};
