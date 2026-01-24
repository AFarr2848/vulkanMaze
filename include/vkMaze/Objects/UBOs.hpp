#include <glm/glm.hpp>

struct GlobalUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec3 cameraPos;
  float _pad0;
};
enum LightType : int32_t {
  POINTLIGHT,
  SPOTLIGHT,
  DIRLIGHT
};

struct SSBOLight {
  glm::vec3 pos;
  float _pad0 = 0;
  glm::vec3 color;
  float _pad1 = 0;
  int32_t type;
  float brightness;
  glm::vec2 _pad2 = glm::vec2(0.0f);
};
