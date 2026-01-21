
class VulkanContext;
class Window;
class Images;

class Swapchain {
public:
  void init(VulkanContext &ctx, Window &win, Images &img) {
    this->ctx = &ctx;
    this->win = &win;
    this->img = &img;
  }

  void createSwapChain();
  void recreate();
  void cleanup();

  vk::Extent2D extent() const;

  std::vector<vk::Image> swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::Extent2D swapChainExtent;
  vk::raii::SwapchainKHR swapChain = nullptr;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
  vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
  void recreateSwapChain();
  void cleanupSwapChain();
  void createImageViews();
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

private:
  VulkanContext *ctx;
  Window *win;
  Images *img;
};
