#include <glm/glm.hpp>

struct GlobalUBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec3 cameraPos;
  float _pad0;
};

struct PushConstant {
  int32_t transformIndex;           // 4 bytes at offset 0
  alignas(16) glm::ivec3 lightNums; // 12 bytes at offset 16
                                    // Total: 28 bytes
};

enum LightType : int32_t {
  POINTLIGHT,
  SPOTLIGHT,
  DIRLIGHT
};

struct alignas(16) SSBOLight {
  glm::vec3 pos;
  float _pad0 = 0;
  glm::vec3 dir;
  float _pad1;
  glm::vec3 color;
  int32_t type;
  float brightness;
  float param1 = 0;
  float param2 = 0;
};
