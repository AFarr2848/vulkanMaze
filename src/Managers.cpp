#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <stdexcept>
#include <vkMaze/Objects/Material.hpp>
#include <vkMaze/Objects/Shapes.hpp>
#include <vkMaze/Objects/UBOs.hpp>
#include <vkMaze/Components/Managers.hpp>

ShaderResource &ShaderResourceManager::get(const std::string &name) {
  return resources.at(name);
}
ShaderResource &ShaderResourceManager::add(ShaderResource resource) {
  resources.emplace(resource.name, resource);
  return resources.at(resource.name);
}

std::unordered_map<std::string, ShaderResource> lights;

Material &MaterialManager::get(const std::string &name) {
  return materials.at(name);
}

Material &MaterialManager::create(const std::string &name, std::string albedo, std::string normal) {
  materials.emplace(name, Material(albedo, normal));
  return materials.at(name);
}

void MaterialManager::createColors() {
  std::cout << "creating colors" << std::endl;
  this->color("red", glm::vec3(100, 0, 0));
  this->color("green", glm::vec3(0, 100, 0));
  this->color("blue", glm::vec3(0, 0, 100));

  this->color("white", glm::vec3(100, 100, 100));
  this->color("black", glm::vec3(0, 0, 0));
  this->color("gray", glm::vec3(50, 50, 50));

  this->color("yellow", glm::vec3(100, 100, 0));
  this->color("cyan", glm::vec3(0, 100, 100));
  this->color("magenta", glm::vec3(100, 0, 100));

  this->color("orange", glm::vec3(100, 50, 0));
  this->color("purple", glm::vec3(50, 0, 100));
  this->color("pink", glm::vec3(100, 75, 80));

  this->color("lime", glm::vec3(75, 100, 0));
  this->color("teal", glm::vec3(0, 50, 50));
  this->color("navy", glm::vec3(0, 0, 50));
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

// there's not really any reason to use this lol
Material &MaterialManager::getRandomMaterial() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(0, materials.size() - 1);

  auto it = materials.begin();
  std::advance(it, distr(gen));
  return it->second;
}

Shape &ShapeManager::get(const std::string &name) {
  try {
    return shapes.at(name);
  } catch (std::exception e) {
    std::cout << "Error - Material \"" << name << "\" not found" << std::endl;
    throw e;
  }
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

  const glm::mat4 model = glm::translate(glm::mat4(1.0f), s.pos) *
                          glm::mat4_cast(glm::quat(s.rotation)) *
                          glm::scale(glm::mat4(1.0f), s.scale);
  transforms.push_back(model);
  vertices.insert(vertices.end(), s.vertices.begin(), s.vertices.end());
  indices.insert(indices.end(), s.indices.begin(), s.indices.end());

  return shapes.at(name);
}

void ShapeManager::updateTransform(const std::string &name) {
  Shape s = get(name);
  transforms[s.transformIndex] = glm::translate(glm::mat4(1.0f), s.pos) *
                                 glm::mat4_cast(glm::quat(s.rotation)) *
                                 glm::scale(glm::mat4(1.0f), s.scale);
}

void ShapeManager::updateShapes(float deltaTime) {
  for (auto &pair : shapes) {
    Shape &s = pair.second;
    s.pos += s.vel * deltaTime;
    s.rotation += s.rotVel * deltaTime;
    s.scale += s.scaleVel * deltaTime;
    transforms[s.transformIndex] = glm::translate(glm::mat4(1.0f), s.pos) *
                                   glm::mat4_cast(glm::quat(s.rotation)) *
                                   glm::scale(glm::mat4(1.0f), s.scale);
  }
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
SSBOLight &LightManager::addPointLight(const std::string &name, glm::vec3 pos, glm::vec3 color, float brightness) {
  SSBOLight l;
  l.pos = pos;
  l.type = POINTLIGHT;
  l.color = color;
  l.brightness = brightness;

  lights.emplace(name, l);

  return lights.at(name);
}

SSBOLight &LightManager::addDirLight(const std::string &name, glm::vec3 dir, glm::vec3 color, float brightness) {
  SSBOLight l;
  l.dir = dir;
  l.color = color;
  l.brightness = brightness;
  l.type = DIRLIGHT;

  lights.emplace(name, l);

  return lights.at(name);
}

SSBOLight &LightManager::addSpotLight(const std::string &name, glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float brightness, float cutoffIn, float cutoffOut) {
  SSBOLight l;
  l.pos = pos;
  l.dir = dir;
  l.color = color;
  l.brightness = brightness;
  l.type = SPOTLIGHT;
  l.param1 = cutoffIn;
  l.param2 = cutoffOut;

  lights.emplace(name, l);

  return lights.at(name);
}

glm::ivec3 LightManager::getLightNums() {
  int p = 0;
  int d = 0;
  int s = 0;
  for (auto &pair : lights) {
    switch (pair.second.type) {
    case POINTLIGHT:
      p++;
      break;
    case DIRLIGHT:
      d++;
      break;
    case SPOTLIGHT:
      s++;
      break;
    }
  }
  return glm::vec3(p, d, s);
}

uint32_t LightManager::getSize() {
  return lights.size();
}
