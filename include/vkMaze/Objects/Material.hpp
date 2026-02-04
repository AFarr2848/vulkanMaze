#include <iostream>
#include <unordered_map>
class VulkanContext;
class Buffers;
class Texture;
class Images;
class FrameData;
class Descriptors;

class Texture {
public:
  vk::raii::Image image = nullptr;
  vk::raii::DeviceMemory memory = nullptr;
  vk::raii::ImageView view = nullptr;
  vk::Format format;
  uint32_t mipLevels;
  vk::raii::Sampler sampler = nullptr;
  vk::raii::DescriptorSet descriptorSet = nullptr;

private:
  VulkanContext *cxt;
  Buffers *buf;
  Images *img;
  FrameData *frame;
};

class Material {

public:
  Material(std::string albedo, std::string normal) {
    this->albedoPath = albedo;
    this->normalPath = normal;
  }
  Material() {
    this->albedoPath = "";
    this->normalPath = "";
  }
  void init(VulkanContext &cxt, Images &img, FrameData &frame, Descriptors &dsc, Buffers &buf) {
    this->cxt = &cxt;
    this->img = &img;
    this->frame = &frame;
    this->dsc = &dsc;
    this->buf = &buf;
    createTextures();
  }

  Texture albedo;
  Texture normal;

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

class MaterialManager {
public:
  Material &get(const std::string &name);
  Material &create(const std::string &name,
                   std::string albedo,
                   std::string normal);
  void initMaterials(VulkanContext &cxt, Images &img, FrameData &frame, Descriptors &dsc, Buffers &buf);

private:
  std::unordered_map<std::string, Material> materials;
};
