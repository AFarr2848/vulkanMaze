#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/VulkanEngine.hpp"
#include "vkMaze/Components/FrameData.hpp"
#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vkMaze/Objects/Vertex.hpp"

void Buffers::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory) {
  vk::BufferCreateInfo bufferInfo{
      .size = size,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive};

  buffer = vk::raii::Buffer(ctx->device, bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = ctx->findMemoryType(memRequirements.memoryTypeBits, properties),

  };
  bufferMemory = vk::raii::DeviceMemory(ctx->device, allocInfo);
  buffer.bindMemory(*bufferMemory, 0);
}

void Buffers::createVertexBuffer() {
  std::vector<Vertex> vertices = eng->getVertices();
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  vk::raii::Buffer stagingBuffer = nullptr;
  vk::raii::DeviceMemory stagingBufferMemory = nullptr;
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

  void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(dataStaging, vertices.data(), bufferSize);
  stagingBufferMemory.unmapMemory();

  createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertexBuffer, vertexBufferMemory);

  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void Buffers::createIndexBuffer() {
  std::vector<uint32_t> indices = eng->getIndices();
  vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  vk::raii::Buffer stagingBuffer = nullptr;
  vk::raii::DeviceMemory stagingBufferMemory = nullptr;
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

  void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(dataStaging, indices.data(), bufferSize);
  stagingBufferMemory.unmapMemory();

  createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexBuffer, indexBufferMemory);

  copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

void Buffers::copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = frame->commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1

  };

  vk::raii::CommandBuffer commandCopyBuffer = std::move(ctx->device.allocateCommandBuffers(allocInfo).front());
  commandCopyBuffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
  commandCopyBuffer.end();
  ctx->graphicsQueue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
  ctx->graphicsQueue.waitIdle();
}

void Buffers::createUniformBuffers() {
  globalUBOs.clear();
  globalUBOMemory.clear();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(GlobalUBO);
    vk::raii::Buffer buffer({});
    vk::raii::DeviceMemory bufferMem({});
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, buffer, bufferMem);
    globalUBOs.emplace_back(std::move(buffer));
    globalUBOMemory.emplace_back(std::move(bufferMem));
    globalUBOMapped.emplace_back(globalUBOMemory[i].mapMemory(0, bufferSize));
  }
}

void Buffers::createStorageBuffer() {
  SSBOs.clear();
  SSBOMemory.clear();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(SSBOLight) * MAX_LIGHTS + sizeof(glm::mat4) * MAX_TRANSFORMS;
    vk::raii::Buffer buffer({});
    vk::raii::DeviceMemory bufferMem({});
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, buffer, bufferMem);
    SSBOs.emplace_back(std::move(buffer));
    SSBOMemory.emplace_back(std::move(bufferMem));
    SSBOsMapped.emplace_back(SSBOMemory[i].mapMemory(0, bufferSize));
  }
}
