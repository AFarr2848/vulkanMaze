#include <glm/ext/matrix_transform.hpp>
#include <vkMaze/Objects/Material.hpp>
#include <vkMaze/Objects/Shapes.hpp>
#include <vkMaze/Objects/UBOs.hpp>
#include <vkMaze/Components/Managers.hpp>

Material &MaterialManager::get(const std::string &name) {
  return materials.at(name);
}

Material &MaterialManager::create(const std::string &name, std::string albedo, std::string normal) {
  materials.emplace(name, Material(albedo, normal));
  return materials.at(name);
}
void MaterialManager::initMaterials(VulkanContext &cxt, Images &img, FrameData &frame, Descriptors &dsc, Buffers &buf) {
  for (auto i = materials.begin(); i != materials.end(); i++) {
    i->second.init(cxt, img, frame, dsc, buf);
  }
}

Material &MaterialManager::color(const std::string &name, glm::vec3 color) {
  materials.try_emplace(name, Material(color));
  return materials.at(name);
}

Shape &ShapeManager::get(const std::string &name) {
  return shapes.at(name);
}
Shape &ShapeManager::add(const std::string &name, Shape s, glm::vec3 pos, glm::vec3 rotation, glm::vec3 scale, Pipeline &pipeline, Material &mat) {
  s.pos = pos;
  s.rotation = rotation;
  s.scale = scale;
  s.transformIndex = shapes.size();
  s.pipeline = &pipeline;
  s.material = &mat;
  s.range = MeshRange({
      .vertexOffset = static_cast<uint32_t>(vertices.size()),
      .indexOffset = static_cast<uint32_t>(indices.size()),
      .indexCount = static_cast<uint32_t>(s.indices.size()),
  });

  shapes.emplace(name, s);

  transforms.push_back(glm::translate(glm::scale(glm::mat4(1.0f), s.scale), s.pos));
  vertices.insert(vertices.end(), s.vertices.begin(), s.vertices.end());
  indices.insert(indices.end(), s.indices.begin(), s.indices.end());

  return shapes.at(name);
}

void ShapeManager::updateTransform(const std::string &name) {
  Shape s = get(name);
  transforms[s.transformIndex] = glm::scale(glm::translate(glm::mat4(1.0f), s.pos), s.scale);
}

std::vector<Shape *> ShapeManager::getDrawOrder() {
  std::vector<Shape *> drawList;
  drawList.reserve(shapes.size());

  for (auto &[name, shape] : shapes)
    drawList.push_back(&shape);

  std::sort(drawList.begin(), drawList.end(),
            [](const Shape *a, const Shape *b) {
              if (a->pipeline != b->pipeline)
                return a->pipeline < b->pipeline;
              return a->material < b->material;
            });
  return drawList;
}

SSBOLight &LightManager::get(const std::string &name) {
  return lights.at(name);
}
SSBOLight &LightManager::add(const std::string &name, SSBOLight l, LightType type, glm::vec3 pos, glm::vec3 color, float brightness) {
  l.pos = pos;
  l.type = type;
  l.color = color;
  l.brightness = brightness;

  lights.emplace(name, l);

  return lights.at(name);
}

uint32_t LightManager::getSize() {
  return lights.size();
}
