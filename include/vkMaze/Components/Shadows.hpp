#pragma once
class Images;
class Swapchain;
class FrameData;
class Pipeline;

class MappedShadows {
public:
  vk::raii::Image imageArrayMap;
  vk::raii::DeviceMemory imageArrayMemory;
  vk::raii::ImageView imageArrayView;
  Pipeline &shadowMappingPipeline;

  uint32_t numShadowLights;

  void makeResources();
  void renderShadows(vk::raii::CommandBuffer &buf);

private:
  Images &img;
  Swapchain &swp;
  FrameData &frames;
};
