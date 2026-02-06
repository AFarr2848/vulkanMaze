#include "vkMaze/Util.hpp"
#include <iostream>
#include <vector>
#include <vkMaze/Objects/Shapes.hpp>

void Icosphere::subdivide(int divisions) {
  for (int d = 0; d < divisions; d++) {

    std::unordered_map<EdgePair, uint32_t> midpointCache;
    uint32_t size = indices.size();

    for (int j = 0; j < size; j += 3) {
      uint32_t i1 = indices[j];
      uint32_t i2 = indices[j + 1];
      uint32_t i3 = indices[j + 2];
      // between i1 and i2
      if (!midpointCache.contains(EdgePair(i1, i2))) {
        vertices.push_back({.pos = midpoint(vertices[i1].pos, vertices[i2].pos)});
        midpointCache.insert({EdgePair(i1, i2), static_cast<uint32_t>(vertices.size() - 1)});
      }

      // i1 and i3
      if (!midpointCache.contains(EdgePair(i1, i3))) {
        vertices.push_back({.pos = midpoint(vertices[i1].pos, vertices[i3].pos)});
        midpointCache.insert({EdgePair(i1, i3), static_cast<uint32_t>(vertices.size() - 1)});
      }
      // i2 and i3
      if (!midpointCache.contains(EdgePair(i2, i3))) {
        vertices.push_back({.pos = midpoint(vertices[i2].pos, vertices[i3].pos)});
        midpointCache.insert({EdgePair(i2, i3), static_cast<uint32_t>(vertices.size() - 1)});
      }

      indices.push_back(i1);
      indices.push_back(midpointCache.at(EdgePair(i1, i2)));
      indices.push_back(midpointCache.at(EdgePair(i1, i3)));

      indices.push_back(i2);
      indices.push_back(midpointCache.at(EdgePair(i2, i3)));
      indices.push_back(midpointCache.at(EdgePair(i1, i2)));

      indices.push_back(i3);
      indices.push_back(midpointCache.at(EdgePair(i1, i3)));
      indices.push_back(midpointCache.at(EdgePair(i2, i3)));

      indices[j] = midpointCache.at(EdgePair(i1, i2));
      indices[j + 1] = midpointCache.at(EdgePair(i2, i3));
      indices[j + 2] = midpointCache.at(EdgePair(i1, i3));
    }
  }
  // project to unit sphere
  for (Vertex &v : vertices) {
    v.pos = glm::normalize(v.pos);
  }
}

void Icosphere::makeNormals() {
  for (Vertex &v : vertices) {
    v.normal = normalize(v.pos);
  }
}

void Icosphere::makeTexCoords() {
  float pi = std::numbers::pi;
  for (Vertex &v : vertices) {
    v.uv = {atan2(v.pos.x, v.pos.z) / (2 * pi) + 0.5, -asin(v.pos.y) / pi + 0.5};
  }
}
