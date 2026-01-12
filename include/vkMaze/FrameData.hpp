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
  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffers;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphore;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphore;
  std::vector<vk::raii::Fence> inFlightFences;
  uint32_t semaphoreIndex = 0;

  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();

private:
  VulkanContext *ctx;
  Swapchain *swp;
  VulkanEngine *eng;
};
