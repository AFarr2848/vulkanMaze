#include <cstdint>
class VulkanContext;
class Swapchain;
class Buffers;
class FrameData;

class Images {
public:
  void init(VulkanContext &cxt, Swapchain &swp, FrameData &frame) {
    this->cxt = &cxt;
    this->swp = &swp;
    this->frame = &frame;
  }

  vk::raii::Image depthImage = nullptr;
  vk::raii::DeviceMemory depthImageMemory = nullptr;
  vk::raii::ImageView depthImageView = nullptr;

  vk::raii::ImageView createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlagBits flags, uint32_t layer = 0, uint32_t layerCount = 1);

  void createDepthResources();
  vk::Format findDepthFormat();
  void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory, uint32_t layerCount = 1);
  void transition_image_layout(
      vk::Image image,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      vk::AccessFlags2 src_access_mask,
      vk::AccessFlags2 dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask,
      vk::ImageAspectFlags image_aspect_flags);

private:
  VulkanContext *cxt;
  Swapchain *swp;
  FrameData *frame;
};
