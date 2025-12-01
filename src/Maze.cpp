
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <vector>
#include <array>
using namespace std;

struct cell {
  bool t = false, b = false, l = false, r = false;
  bool isCarved() { return t || b || l || r; }
  void carveDirection(vector<int> vec) {
    if (vec.at(0) == 1)
      b = true;
    if (vec.at(0) == -1)
      t = true;
    if (vec.at(1) == 1)
      r = true;
    if (vec.at(1) == -1)
      l = true;
  }
  void carveReverse(vector<int> vec) {
    carveDirection(vector<int>({vec.at(0) * -1, vec.at(1) * -1}));
  }
};

void carveFrom(int cx, int cy, vector<vector<cell>> &maze) {
  int nx, ny;
  vector<vector<int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

  std::random_device rd;
  std::mt19937 g(rd());
  shuffle(directions.begin(), directions.end(), g);

  for (vector<int> i : directions) {
    nx = cx + i.at(0);
    ny = cy + i.at(1);
    if (nx < maze.at(0).size() && nx >= 0 && ny < maze.size() && ny >= 0 &&
        !maze.at(ny).at(nx).isCarved()) {
      maze.at(cy).at(cx).carveDirection(i);
      maze.at(ny).at(nx).carveReverse(i);
      carveFrom(nx, ny, maze);
    }
  }
}

vector<vector<cell>> makeMaze(int mazeSize) {
  vector<vector<cell>> maze =
      vector<vector<cell>>(mazeSize, vector<cell>(mazeSize));

  vector<int> startingPoint = {0, 0};
  carveFrom(0, 0, maze);
  int height = maze.size();
  int width = maze[0].size();
  return maze;
}

vector<float> mazeToFloats(float width, float height, float wallHeight, vector<vector<cell>> maze) {
  float cx = width / maze.at(0).size();
  float cy = width / maze.size();
  vector<float> topLeft;

  vector<array<float, 3>> outVec;

  for (int r = 0; r < maze.size(); r++) {
    for (int c = 0; c < maze.at(0).size(); c++) {
      topLeft = {cx * c, cy * r};
      if (!maze.at(r).at(c).t) {
        // top left triangle
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1), wallHeight}});
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1), 0.0f}});
        outVec.push_back(array<float, 3>{{topLeft.at(0) + cx, topLeft.at(1), wallHeight}});
        // color
        //  bottom right triangle
        outVec.push_back(array<float, 3>{{topLeft.at(0) + cx, topLeft.at(1), wallHeight}});
        outVec.push_back(array<float, 3>{{topLeft.at(0) + cx, topLeft.at(1), 0.0f}});
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1), 0.0f}});
      }
      if (!maze.at(r).at(c).l) {
        // top left (?) triangle
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1), wallHeight}});
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1), 0.0f}});
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1) + cy, wallHeight}});
        // bottom left
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1) + cy, wallHeight}});
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1) + cy, 0.0f}});
        outVec.push_back(array<float, 3>{{topLeft.at(0), topLeft.at(1), 0.0f}});
      }
    }
  }

  vector<float> flatVec;
  flatVec.reserve(outVec.size() * 3);
  for (array<float, 3> array : outVec) {
    flatVec.insert(flatVec.end(), array.begin(), array.end());
  }
  return flatVec;
}

vector<glm::mat4> test(float xScale, float yScale, float zScale, vector<vector<cell>> maze) {
  vector<glm::mat4> outVec;
  outVec.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0)));
  outVec.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(-2, 0, 0)));
  return outVec;
}

vector<float> mazeToBounds(float xScale, float yScale, float zScale, vector<vector<cell>> maze) {
  float cx = xScale / maze.at(0).size();
  float cz = zScale / maze.size();
  float wallThickness = 0.1f;

  vector<float> bounds;

  for (int r = 0; r < maze.size(); r++) {
    for (int c = 0; c < maze.at(0).size(); c++) {
      if (!maze.at(r).at(c).t) {
        bounds.push_back(cx * c);                     // x1
        bounds.push_back(cz * r - wallThickness / 2); // z1
        bounds.push_back(cx * c + cx);                // x2
        bounds.push_back(cz * r - wallThickness / 2); // z2
      }
      if (!maze.at(r).at(c).l) {
        bounds.push_back(cx * c + wallThickness / 2); // x1
        bounds.push_back(cz * r);                     // z1
        bounds.push_back(cx * c + wallThickness / 2); // x2
        bounds.push_back(cz * r + cz);                // z2
      }
    }
  }
  return bounds;
}

vector<glm::mat4> mazeToTransforms(float xScale, float yScale, float zScale, vector<vector<cell>> const &maze) {
  float cx = xScale / maze.at(0).size();
  float cz = zScale / maze.size();
  glm::mat4 model;
  float wallThickness = 0.1f;

  vector<glm::mat4> transforms;

  for (int r = 0; r < maze.size(); r++) {
    for (int c = 0; c < maze.at(0).size(); c++) {
      if (!maze.at(r).at(c).t) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx * c, 0, cz * r - wallThickness / 2));
        model = glm::scale(model, glm::vec3(wallThickness, yScale, cx));
        transforms.push_back(model);
      }
      if (!maze.at(r).at(c).l) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx * c + wallThickness / 2, 0, cz * r));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(wallThickness, yScale, cz));
        transforms.push_back(model);
      }
    }
  }
  // bottom and right walls
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0, 0, zScale - wallThickness / 2));
  model = glm::scale(model, glm::vec3(xScale, yScale * 2, wallThickness));
  transforms.push_back(model);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(xScale + wallThickness / 2, 0, 0));
  model = glm::scale(model, glm::vec3(wallThickness, yScale, zScale));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  transforms.push_back(model);

  // and floor

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0, -0.1f, 0));
  model = glm::scale(model, glm::vec3(xScale, 0.1f, zScale));
  transforms.push_back(model);

  return transforms;
}
