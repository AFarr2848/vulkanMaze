#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/Window.hpp"
#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vulkan/vulkan.hpp"
#include <vkMaze/Components/VulkanEngine.hpp>
#include <vkMaze/Objects/Camera.hpp>
#include <vkMaze/Objects/Material.hpp>
#include <vector>
#include <iostream>
#include <vkMaze/Objects/Shapes.hpp>
#include <vkMaze/Components/Swapchain.hpp>
#include <vkMaze/Objects/UBOs.hpp>
#include <vkMaze/Components/FrameData.hpp>
#include <GLFW/glfw3.h>

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

Window win;
VulkanContext cxt;
Swapchain swp;
FrameData frames;
Pipeline pipelinePhong;
Pipeline pipelineUnlit;
Buffers buf;
Images img;
Descriptors dsc;

class VKMaze : public VulkanEngine {
public:
  void makeShapes() override {
    // floor
    shapes.push_back(Cube(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 0.2f, 5.0f)), glm::vec3(0.0f, -8.0f, 0.0f))));
    shapes.at(shapes.size() - 1).pipeline = &pipelinePhong;

    // cube
    shapes.push_back(Cube(glm::mat4(1.0f)));
    shapes.at(shapes.size() - 1).pipeline = &pipelinePhong;

    // backpack
    shapes.push_back(Mesh("models/Survival_BackPack_2.fbx", glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f))));
    shapes.at(shapes.size() - 1).pipeline = &pipelinePhong;

    for (Shape s : shapes) {
      makeOffset(s);
    }

    // Cube lightCube(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)), glm::vec3(2.25f, 2.25f, 2.25f)));
  }

  void makeMaterials() override {
    materials.push_back(Material("", "", *cxt, *img, *frames, *dsc, *buf));
    materials.push_back(Material("", "", *cxt, *img, *frames, *dsc, *buf));
    materials.push_back(Material("textures/backpack_albedo.jpg", "", *cxt, *img, *frames, *dsc, *buf));

    for (int i = 0; i < shapes.size(); i++) {
      shapes.at(i).material = &materials.at(i);
    }
  }

private:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<MeshRange> offsets;
  std::vector<Shape> shapes;
  std::vector<Material> materials;

  void createPipelines() override {
    pipelinePhong.init(*cxt, *dsc, *swp, *img);
    pipelinePhong.createPipeline({

        .shaderPath = "build/shaders/shader.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,
        .descriptorSetLayout = dsc->descriptorSetLayout

    });
    pipelineUnlit.init(*cxt, *dsc, *swp, *img);
    pipelineUnlit.createPipeline({

        .shaderPath = "build/shaders/shader.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eLine,
        .cullModeFlags = vk::CullModeFlagBits::eBack,
        .descriptorSetLayout = dsc->descriptorSetLayout

    });
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
    ubo.cameraPos = camera.Position;
  }

  std::vector<uint32_t> getIndices() override {
    return indices;
  }

  void drawScreen() override {
    for (Shape s : shapes) {

      std::cout << s.pipeline << std::endl;
      std::cout << *s.material->albedo->descriptorSet << std::endl;
      frames->commandBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s.pipeline->pipelineLayout, 0, *s.material->albedo->descriptorSet, nullptr);

      frames->commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, s.pipeline->graphicsPipeline);
      frames->commandBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s.pipeline->pipelineLayout, 0, *dsc->descriptorSets[currentFrame], nullptr);
      frames->commandBuffers[currentFrame].drawIndexed(s.range->indexCount, 1, s.range->indexOffset, s.range->vertexOffset, 0);
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

  void makeOffset(Shape &s) {

    MeshRange offset{

        .vertexOffset = static_cast<uint32_t>(vertices.size()),
        .indexOffset = static_cast<uint32_t>(indices.size()),
        .indexCount = static_cast<uint32_t>(s.indices.size()),

    };

    offsets.push_back(offset);
    vertices.insert(vertices.end(), s.vertices.begin(), s.vertices.end());
    indices.insert(indices.end(), s.indices.begin(), s.indices.end());
    s.range = &offsets.at(offsets.size() - 1);
  }
};

int main() {
  try {
    VKMaze app;

    app.init(win, cxt, swp, frames, img, dsc, buf);
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
