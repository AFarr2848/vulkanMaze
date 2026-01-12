class VulkanContext;
class Buffers;

class Descriptors {

public:
  void init(VulkanContext &cxt, Buffers &buf) {
    this->cxt = &cxt;
  }
  vk::raii::DescriptorPool descriptorPool = nullptr;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;

  void createDescriptorSets();
  void createGlobalDescriptorSetLayout();
  void createDescriptorPool();

private:
  VulkanContext *cxt;
  Buffers *buf;
};
