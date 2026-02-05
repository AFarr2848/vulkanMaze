#pragma once
#include <fstream>
#include <glm/glm.hpp>

static std::vector<char> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }
  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();
  return buffer;
};

static glm::vec3 midpoint(glm::vec3 vec1, glm::vec3 vec2) {
  glm::vec3 out = glm::vec3(0.0f);
  out.x = (vec1.x + vec2.x) / 2;
  out.y = (vec1.y + vec2.y) / 2;
  out.z = (vec1.z + vec2.z) / 2;
  return out;
}

static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
