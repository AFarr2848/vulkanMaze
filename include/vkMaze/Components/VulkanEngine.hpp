#pragma once
#include <glm/fwd.hpp>
class VulkanContext;
class Swapchain;
class FrameData;
class Buffers;
class Images;
class Descriptors;
class Vertex;
class GlobalUBO;
class GLFWwindow;
class Window;
class SSBOLight;

class VulkanEngine {
public:
  float time;
  float deltaTime;
  float timeSinceFPS = 0;
  int frameCount = 0;
  uint32_t semaphoreIndex = 0;

  void init(Window &win, VulkanContext &cxt, Swapchain &swp, FrameData &frames, Images &img, Descriptors &dsc, Buffers &buf) {
    this->win = &win;
    this->cxt = &cxt;
    this->swp = &swp;
    this->frames = &frames;
    this->img = &img;
    this->dsc = &dsc;
    this->buf = &buf;
  }

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
  virtual std::vector<uint32_t> getIndices();

  virtual void createPipelines();
  virtual void drawScreen();
  virtual void mouseMoved(float xoffset, float yoffset);
  virtual void updateCameraTransforms(GlobalUBO &ubo);
  virtual void processInput(GLFWwindow *window);
  virtual void makeShapes();
  virtual void makeMaterials();
  virtual void updateLights(std::vector<SSBOLight> &);
  virtual void updateTransforms(std::vector<glm::mat4> &);

protected:
  void updateGlobalUniformBuffer(uint32_t currentFrame);
  void updateStorageBuffer(uint32_t currentImage);

  Window *win;
  VulkanContext *cxt;
  Swapchain *swp;
  FrameData *frames;
  Buffers *buf;
  Images *img;
  Descriptors *dsc;
  int32_t currentFrame = 0;
};
