#ifndef MAZE_H
#define MAZE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct cell {
  bool t, b, l, r;
  bool isCarved();
  void carveDirection(std::vector<int> vec);
  void carveReverse(std::vector<int> vec);
};

std::vector<std::vector<cell>> makeMaze(int mazeSize);

std::vector<float> mazeToFloats(float width, float height, float wallHeight, std::vector<std::vector<cell>> maze);

void carveFrom(int cx, int cy, std::vector<std::vector<cell>> &maze);

std::vector<glm::mat4> mazeToTransforms(float xScale, float yScale, float zScale, std::vector<std::vector<cell>> const &maze);
std::vector<glm::mat4> test(float xScale, float yScale, float zScale, std::vector<std::vector<cell>> maze);
std::vector<float> mazeToBounds(float xScale, float yScale, float zScale, std::vector<std::vector<cell>> maze);

#endif
