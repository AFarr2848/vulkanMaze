
#include <assimp/matrix4x4.h>
#include <glm/detail/qualifier.hpp>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include <vkMaze/Vertex.hpp>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Pipeline;

class MeshRange {
public:
  uint32_t vertexOffset;
  uint32_t indexOffset;
  uint32_t indexCount;

  Pipeline *pipeline;
};

class Shape {
public:
  std::vector<Vertex> vertices = std::vector<Vertex>();
  std::vector<uint32_t> indices = std::vector<uint32_t>();
};

class Mesh : public Shape {
public:
  Mesh(std::string modelPath, std::string texPath, glm::mat4 transform) {
    this->modelPath = modelPath;
    this->texPath = texPath;
    this->trans = transform;

    loadModel();
  }

private:
  std::string modelPath;
  std::string texPath;
  glm::mat4 trans;
  void loadModel();

  void processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4 parentTransform);
  void processNode(aiNode *node, const aiScene *scene, glm::mat4 transform);
  glm::mat4 assimpToGlm(aiMatrix4x4 a);
};

class Cube : public Shape {
public:
  Cube(glm::mat4 transform) {
    vertices = {
        // Front face (Z+)
        {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0, 1}},

        // Back face (Z-)
        {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1}},

        // Left face (X-)
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1, 0}},
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},

        // Right face (X+)
        {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1}},
        {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0, 1}},

        // Top face (Y+)
        {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 1}},

        // Bottom face (Y-)
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 1}},
        {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1}},
    };

    indices = {
        // Front
        0, 1, 2, 0, 2, 3,
        // Back
        4, 5, 6, 4, 6, 7,
        // Left
        8, 9, 10, 8, 10, 11,
        // Right
        12, 13, 14, 12, 14, 15,
        // Top
        16, 17, 18, 16, 18, 19,
        // Bottom
        20, 21, 22, 20, 22, 23

    };
    for (Vertex &v : vertices) {
      glm::vec4 p = transform * glm::vec4(v.pos, 1.0);
      v.pos = p;
    }
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    for (Vertex &v : vertices) {
      v.normal = glm::normalize(normalMatrix * v.normal);
    }
  };
};
