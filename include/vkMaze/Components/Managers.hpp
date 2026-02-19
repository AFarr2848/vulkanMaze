#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <unordered_map>

class Material;
class VulkanContext;
class Images;
class FrameData;
class Descriptors;
class Pipeline;
class Buffers;
class Shape;
class Vertex;
class SSBOLight;
struct ShaderResource;

enum LightType : int32_t;

class MaterialManager {
public:
  Material &get(const std::string &name);
  Material &create(const std::string &name,
                   std::string albedo,
                   std::string normal);

  Material &color(const std::string &, glm::vec3);
  void initMaterials(VulkanContext &cxt, Images &img, FrameData &frame, Descriptors &dsc, Buffers &buf);

private:
  std::unordered_map<std::string, Material> materials;
};

class ShapeManager {
public:
  Shape &get(const std::string &name);
  Shape &add(const std::string &name, Shape s, glm::vec3 pos, glm::vec3 rotation, glm::vec3 scale, Pipeline &pipeline, Material &mat);

  size_t getSize() { return shapes.size(); }
  std::unordered_map<std::string, Shape> &getShapes() { return shapes; }
  void updateTransform(const std::string &name);
  std::vector<Shape *> getDrawOrder();

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<glm::mat4> transforms;

private:
  std::unordered_map<std::string, Shape> shapes;
};

class LightManager {
public:
  SSBOLight &get(const std::string &name);
  SSBOLight &addPointLight(const std::string &name, glm::vec3 pos, glm::vec3 color, float brightness);
  SSBOLight &addDirLight(const std::string &name, glm::vec3 dir, glm::vec3 color, float brightness);
  SSBOLight &addSpotLight(const std::string &name, glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float brightness, float cutoffIn, float cutoffOut);
  glm::ivec3 getLightNums();
  uint32_t getSize();
  std::unordered_map<std::string, SSBOLight> lights;
};

class ShaderResourceManager {
public:
  ShaderResource &get(const std::string &name);
  ShaderResource &add(ShaderResource resource);

  std::unordered_map<std::string, ShaderResource> resources;
};
