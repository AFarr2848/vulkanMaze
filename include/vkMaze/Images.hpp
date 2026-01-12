class VulkanContext;
class Swapchain;

class Images {
public:
  void init(VulkanContext &cxt, Swapchain &swp) {
    this->cxt = &cxt;
  }

  vk::raii::Image depthImage = nullptr;
  vk::raii::DeviceMemory depthImageMemory = nullptr;
  vk::raii::ImageView depthImageView = nullptr;
  void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory);
  vk::raii::ImageView createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlagBits flags);

  void createDepthResources();
  vk::Format findDepthFormat();

private:
  VulkanContext *cxt;
  Swapchain *swp;
};
