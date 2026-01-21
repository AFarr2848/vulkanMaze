#include <vulkan/vulkan_raii.hpp>
class VulkanContext;
class Buffers;
class Texture;
class Images;
class FrameData;
class Descriptors;

class Material {

public:
  Material(std::string albedo, std::string normal, VulkanContext &cxt, Images &img, FrameData &frame, Descriptors &dsc, Buffers &buf) {
    this->cxt = &cxt;
    this->img = &img;
    this->frame = &frame;
    this->dsc = &dsc;
    this->albedoPath = albedo;
    this->normalPath = normal;
    this->buf = &buf;

    createTextures();
  }

  Texture *albedo;
  Texture *normal;

private:
  VulkanContext *cxt;
  Images *img;
  FrameData *frame;
  Descriptors *dsc;
  Buffers *buf;

  std::string albedoPath;
  std::string normalPath;

  void createTextures();
  void createTextureImage(Texture *tex, std::string path);
  void transitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
  void copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);
  void createTextureImageView(Texture *tex);
  void createTextureSampler(Texture *tex);
};

class Texture {
public:
  vk::raii::Image image;
  vk::raii::DeviceMemory memory;
  vk::raii::ImageView view;
  vk::Format format;
  uint32_t mipLevels;
  vk::raii::Sampler sampler;
  vk::raii::DescriptorSet descriptorSet;

private:
  VulkanContext *cxt;
  Buffers *buf;
  Images *img;
  FrameData *frame;
};
