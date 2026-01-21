#include <glm/glm.hpp>

struct GlobalUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec3 cameraPos;
};
