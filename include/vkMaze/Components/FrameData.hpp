class VulkanContext;
class Swapchain;
class VulkanEngine;

class FrameData {
public:
  void init(VulkanContext &ctx, Swapchain &swp, VulkanEngine &eng) {
    this->ctx = &ctx;
    this->swp = &swp;
    this->eng = &eng;
  }

  int32_t currentFrame = 0;
  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffers;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphore;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphore;
  std::vector<vk::raii::Fence> inFlightFences;
  uint32_t semaphoreIndex = 0;
  std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();
  void endSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer);

  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();

private:
  VulkanContext *ctx;
  Swapchain *swp;
  VulkanEngine *eng;
};
