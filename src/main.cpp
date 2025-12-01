#include <vkMaze/engine.hpp>
#include <vkMaze/Maze.hpp>
#include <vector>
#include <iostream>

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

class VKMaze : public VulkanEngine {
  std::vector<Vertex> getVertices() override {
    return vertices;
  }

  std::vector<uint16_t> getIndices() override {
    return indices;
  }
};

int main() {
  try {
    VKMaze app;
    std::vector<std::vector<cell>> maze = makeMaze(8);

    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void initVulkan() {
}
