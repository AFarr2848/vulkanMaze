#pragma once
class VulkanContext;
class VulkanEngine;
class FrameData;
class Vertex;

class Buffers {

public:
  void init(VulkanEngine &eng, VulkanContext &ctx, FrameData &frame) {
    this->eng = &eng;
    this->ctx = &ctx;
    this->frame = &frame;
  }

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  std::vector<vk::raii::Buffer> globalUBOs;
  std::vector<vk::raii::DeviceMemory> globalUBOMemory;
  std::vector<void *> globalUBOMapped;

  std::vector<vk::raii::Buffer> materialUBOs;
  std::vector<vk::raii::DeviceMemory> materialUBOMemory;
  std::vector<void *> materialUBOMapped;

  void createVertexBuffer();
  void createIndexBuffer();

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory);
  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
  void createUniformBuffers();

private:
  VulkanContext *ctx;
  VulkanEngine *eng;
  std::vector<Vertex> *vertices;
  std::vector<uint32_t> *indices;
  FrameData *frame;
};
