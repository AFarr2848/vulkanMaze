#include "vkMaze/VulkanContext.hpp"
#include "vkMaze/Window.hpp"
#include "vkMaze/GraphicsPipeline.hpp"
#include "vkMaze/Buffers.hpp"
#include "vkMaze/Images.hpp"
#include "vkMaze/Descriptors.hpp"
#include <algorithm>
#include <vkMaze/VulkanEngine.hpp>
#include <vkMaze/Camera.hpp>
#include <vector>
#include <iostream>
#include <vkMaze/Shapes.hpp>
#include <vkMaze/Swapchain.hpp>
#include <vkMaze/UBOs.hpp>
#include <vkMaze/FrameData.hpp>
#include <GLFW/glfw3.h>

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

Window win;
VulkanContext cxt;
Swapchain swp;
FrameData frames;
GraphicsPipeline pipeline;
Buffers buf;
Images img;
Descriptors dsc;

class VKMaze : public VulkanEngine {
public:
  void makeShapes() {
    Cube floorCube(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 0.2f, 5.0f)), glm::vec3(0.0f, -8.0f, 0.0f)));
    addShape(floorCube);
    Cube cube(glm::mat4(1.0f));
    addShape(cube);
    std::cout << "Cube vertex count: " << cube.vertices.size() << std::endl;
  }

private:
  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;
  std::vector<MeshRange> offsets;

  void addShape(Shape shape) {
    MeshRange offset{

        .vertexOffset = static_cast<uint32_t>(vertices.size()),
        .indexOffset = static_cast<uint32_t>(indices.size()),
        .indexCount = static_cast<uint32_t>(shape.indices.size())

    };

    offsets.push_back(offset);
    vertices.insert(vertices.end(), shape.vertices.begin(), shape.vertices.end());
    indices.insert(indices.end(), shape.indices.begin(), shape.indices.end());
  }

  std::vector<Vertex> getVertices() override {
    std::cout << "Vertices size: " << vertices.size() << std::endl;
    return vertices;
  }

  void updateCameraTransforms(GlobalUBO &ubo) override {

    // ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::mat4(1.0f);
    // ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = camera.GetViewMatrix();
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swp->swapChainExtent.width) / static_cast<float>(swp->swapChainExtent.height), 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
  }

  void updateUBOData(MaterialUBO &ubo) override {
  }

  std::vector<uint16_t> getIndices() override {
    return indices;
  }

  void drawScreen() override {
    for (MeshRange m : offsets) {
      frames->commandBuffers[currentFrame].drawIndexed(m.indexCount, 1, m.indexOffset, m.vertexOffset, 0);
    }
  }

  void mouseMoved(float xoffset, float yoffset) override {
    camera.ProcessMouseMovement(xoffset, yoffset);
  }

  void processInput(GLFWwindow *window) override {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      camera.processKeyboard(DOWN, deltaTime);
  }
};

int main() {
  try {
    VKMaze app;
    app.makeShapes();
    std::cout << "Shapes made" << std::endl;

    app.init(win, cxt, swp, frames, img, dsc, pipeline, buf);
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
