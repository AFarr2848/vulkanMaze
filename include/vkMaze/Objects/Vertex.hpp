#include <glm/glm.hpp>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;

  static vk::VertexInputBindingDescription getBindingDescription();
  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
};
