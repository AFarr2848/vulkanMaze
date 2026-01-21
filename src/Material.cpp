#include "vkMaze/Objects/Material.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/FrameData.hpp"
#include "vkMaze/Components/Descriptors.hpp"

#include "vulkan/vulkan.hpp"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void Material::createTextures() {
  std::cout << "Making textures" << std::endl;
  createTextureImage(albedo, albedoPath);
  std::cout << "Texture Image made" << std::endl;

  createTextureImageView(albedo);
  std::cout << "Texture ImageView made" << std::endl;

  createTextureImage(normal, normalPath);
  createTextureImageView(normal);

  albedo->descriptorSet = dsc->createMaterialDescriptorSet(albedo->view, albedo->sampler);
  std::cout << "Dsc set: " << *albedo->descriptorSet << std::endl;
}
void Material::createTextureSampler(Texture *tex) {
  vk::PhysicalDeviceProperties properties = cxt->physicalDevice.getProperties();
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways};
  tex->sampler = vk::raii::Sampler(cxt->device, samplerInfo);
}

void Material::createTextureImageView(Texture *tex) {
  tex->view = img->createImageView(tex->image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

void Material::createTextureImage(Texture *tex, std::string path) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels;
  if (!path.empty()) {
    std::cout << "Loading....." << std::endl;
    pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

  } else {
    texWidth = 1;
    texHeight = 1;
    texChannels = 1;
    pixels = new stbi_uc[4];
    pixels[0] = 255;
    pixels[1] = 255;
    pixels[2] = 255;
    pixels[3] = 255;
  }

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }
  vk::DeviceSize imageSize = texWidth * texHeight * 4;

  vk::raii::Buffer stagingBuffer = nullptr;
  vk::raii::DeviceMemory stagingBufferMemory = nullptr;
  std::cout << "Creating buffer..." << imageSize << std::endl;
  buf->createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);
  std::cout << "Created buffer" << std::endl;

  void *data = stagingBufferMemory.mapMemory(0, imageSize);
  std::cout << "Created data" << std::endl;
  memcpy(data, pixels, imageSize);
  std::cout << "memcpy" << std::endl;
  stagingBufferMemory.unmapMemory();

  stbi_image_free(pixels);

  std::cout << "creating imgge" << std::endl;
  img->createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, tex->image, tex->memory);
  std::cout << "Created Image" << std::endl;

  transitionImageLayout(tex->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(stagingBuffer, tex->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  transitionImageLayout(tex->image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Material::transitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
  auto commandBuffer = frame->beginSingleTimeCommands();

  vk::ImageMemoryBarrier barrier{
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .image = image,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }
  commandBuffer->pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
  frame->endSingleTimeCommands(*commandBuffer);
}

void Material::copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height) {
  std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = frame->beginSingleTimeCommands();
  vk::BufferImageCopy region{.bufferOffset = 0, .bufferRowLength = 0, .bufferImageHeight = 0, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageOffset = {0, 0, 0}, .imageExtent = {width, height, 1}};
  commandBuffer->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});
  frame->endSingleTimeCommands(*commandBuffer);
}
