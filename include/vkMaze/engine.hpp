#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
struct GlobalUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct MaterialUBO {
  glm::vec3 idk;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;

  static vk::VertexInputBindingDescription getBindingDescription();
  static std::array<vk::VertexInputAttributeDescription, 1> getAttributeDescriptions();
};

class VulkanEngine {
public:
  void run();

protected:
  virtual std::vector<Vertex> getVertices();
  virtual std::vector<uint16_t> getIndices();
  virtual void updateMaterialUniformBuffer(uint32_t currentImage);
  virtual void updateGlobalUniformBuffer(uint32_t currentImage);
};
