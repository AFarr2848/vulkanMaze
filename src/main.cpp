#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <pch.hpp>
#include <vkMaze/engine.hpp>
#include <vkMaze/Maze.hpp>
#include <vkMaze/Camera.hpp>
#include <vector>
#include <iostream>
#include <chrono>

std::vector<Vertex> vertices = {
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

std::vector<uint16_t> indices = {
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

std::vector<glm::mat4> transforms;

Camera camera(glm::vec3(0.0f, -3.0f, 3.0f));

class VKMaze : public VulkanEngine {
  std::vector<Vertex> getVertices() override {
    return vertices;
  }

  void updateTransforms(GlobalUBO &ubo) override {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = camera.GetViewMatrix();
    std::cout << glm::to_string(ubo.view) << std::endl;
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
  }

  void updateUBO(MaterialUBO &ubo) override {
    for (int i = 0; i < transforms.size(); i++) {
      ubo.transforms[i] = transforms[i];
    }
  }

  std::vector<uint16_t> getIndices() override {
    return indices;
  }

  void drawScreen() override {
    commandBuffers[currentFrame].drawIndexed(indices.size(), transforms.size(), 0, 0, 0);
  }

  void mouseMoved(float xoffset, float yoffset) override {
    camera.ProcessMouseMovement(xoffset, yoffset);
  }
};

int main() {
  try {
    VKMaze app;
    std::vector<std::vector<cell>> maze = makeMaze(8);
    std::vector<glm::mat4> transforms = mazeToTransforms(5, 1, 5, maze);

    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
