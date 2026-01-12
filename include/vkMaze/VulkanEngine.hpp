class VulkanContext;
class Swapchain;
class FrameData;
class Window;
class GraphicsPipeline;
class Buffers;
class Images;
class Descriptors;
class Vertex;
class GlobalUBO;
class MaterialUBO;
class GLFWwindow;

class VulkanEngine {
public:
  uint32_t currentFrame = 0;
  float time;
  float deltaTime;
  uint32_t semaphoreIndex = 0;
  bool framebufferResized = false;

  void run();
  void initVulkan();
  void cleanup();
  void mainLoop();
  void keepTime();
  void drawFrame();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
  void recordCommandBuffer(uint32_t imageIndex);
  Window getWindow();
  virtual std::vector<Vertex> getVertices();
  virtual std::vector<uint16_t> getIndices();

  virtual void drawScreen();
  virtual void mouseMoved(float xoffset, float yoffset);
  virtual void updateCameraTransforms(GlobalUBO &ubo);
  virtual void updateUBOData(MaterialUBO &ubo);
  virtual void processInput(GLFWwindow *window);
  void transition_image_layout(
      vk::Image image,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      vk::AccessFlags2 src_access_mask,
      vk::AccessFlags2 dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask,
      vk::ImageAspectFlags image_aspect_flags);

protected:
  Window *win;
  VulkanContext *cxt;
  Swapchain *swp;
  FrameData *frames;
  GraphicsPipeline *pipeline;
  Buffers *buf;
  Images *img;
  Descriptors *dsc;

  void updateGlobalUniformBuffer(uint32_t currentFrame);
  void updateMaterialUniformBuffer(uint32_t currentFrame);
};
