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
#include <vulkan/vulkan_raii.hpp>

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

Window win;
VulkanContext cxt;
Swapchain swp;
FrameData frames;
Buffers buf;
Images img;
Descriptors dsc;
MaterialManager materials;

class VKMaze : public VulkanEngine {
public:
  void makeShapes() override {
    materials.create("default", "", "");
    materials.create("backpack", "textures/backpack_albedo.jpg", "");
    materials.create("earth", "textures/earth.png", "");
    materials.color("black", glm::vec3(0));
    materials.color("purple", glm::vec3(100, 20, 100));

    // floor
    shapes.add(
        "floor",
        Cube(),
        glm::vec3(0.0f, -3.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(300.0f, 0.5f, 300.0f),
        *&pipelinePhong,
        materials.get("default")

    );

    // cube
    shapes.add(
        "cube",
        Cube(),
        glm::vec3(0.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        *&pipelineWireframe,
        materials.get("default")

    );

    shapes.add(
        "backpack",
        Mesh("models/Survival_BackPack_2.fbx"),
        glm::vec3(0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.005f),
        *&pipelinePhong,
        materials.get("backpack")

    );

    shapes.add(
        "ico",
        Icosphere(3),
        glm::vec3(0.0f, 5.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        *&pipelinePhong,
        materials.get("earth")

    );

    shapes.add(
        "light_sphere",
        Icosphere(3),
        glm::vec3(0.0f, 5.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.2f),
        *&pipelineUnlit,
        materials.get("default")

    );

    // Cube lightCube(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)), glm::vec3(2.25f, 2.25f, 2.25f)));
  }

  void makeMaterials() override {
    materials.initMaterials(*cxt, *img, *frames, *dsc, *buf);
  }

private:
  ShapeManager shapes;
  Pipeline pipelinePhong;
  Pipeline pipelineUnlit;
  Pipeline pipelineWireframe;
  std::vector<Pipeline *> pipelines = {&pipelinePhong, &pipelineUnlit, &pipelineWireframe};

  void createPipelines() override {
    std::vector dscSetLayouts = {
        *dsc->descriptorSetLayout,
        *dsc->matSetLayout,
        *dsc->lightSetLayout,
        *dsc->transformSetLayout

    };

    pipelinePhong.init(*cxt, *dsc, *swp, *img);
    pipelinePhong.createPipeline({

        .shaderPath = "build/shaders/shader.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,
        .setLayouts = dscSetLayouts

    });
    pipelineUnlit.init(*cxt, *dsc, *swp, *img);
    pipelineUnlit.createPipeline({

        .shaderPath = "build/shaders/unlit.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,
        .setLayouts = dscSetLayouts

    });
    pipelineWireframe.init(*cxt, *dsc, *swp, *img);
    pipelineWireframe.createPipeline({

        .shaderPath = "build/shaders/unlit.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eLine,
        .cullModeFlags = vk::CullModeFlagBits::eBack,
        .setLayouts = dscSetLayouts

    });
  }

  std::vector<Vertex> getVertices() override {
    return shapes.vertices;
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

  void updateLights(std::vector<SSBOLight> &lights) override {
    lights.push_back(SSBOLight({

        .pos = shapes.get("light_sphere").pos,
        .color = glm::vec3(1.0f, 1.0f, 1.0f),
        .type = POINTLIGHT,
        .brightness = 0.5f

    }));
    static_assert(sizeof(SSBOLight) == 48, "Size must be 48");
    static_assert(offsetof(SSBOLight, type) == 32, "Type must start at byte 32");
  };

  void updateTransforms(std::vector<glm::mat4> &transforms) override {
    shapes.get("light_sphere").pos = {6 * sin(time), 8, 6 * cos(time)};
    shapes.updateTransform("light_sphere");

    transforms.insert(transforms.begin(), shapes.transforms.begin(), shapes.transforms.end());
  };

  std::vector<uint32_t> getIndices() override {
    return shapes.indices;
  }

  void drawScreen() override {
    vk::raii::CommandBuffer &buf = frames->commandBuffers[currentFrame];
    std::vector<Shape *> drawShapes = shapes.getDrawOrder();
    Material *currentMaterial = nullptr;
    Pipeline *currentPipeline = nullptr;

    for (Shape *s : drawShapes) {
      PushConstant pc = PushConstant({.transformIndex = s->transformIndex});
      buf.pushConstants(s->pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, vk::ArrayProxy<const PushConstant>(pc));
      if (s->pipeline != currentPipeline) {
        buf.bindPipeline(vk::PipelineBindPoint::eGraphics, s->pipeline->graphicsPipeline);
        buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s->pipeline->pipelineLayout, 0, *dsc->descriptorSets[currentFrame], nullptr);
        buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s->pipeline->pipelineLayout, 2, *dsc->lightDescriptorSets[currentFrame], nullptr);
        buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s->pipeline->pipelineLayout, 3, *dsc->transformDescriptorSets[currentFrame], nullptr);
        currentPipeline = s->pipeline;
      }
      if (s->material != currentMaterial) {
        buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s->pipeline->pipelineLayout, 1, *s->material->albedo.descriptorSet, nullptr);
        currentMaterial = s->material;
      }
      buf.drawIndexed(s->range.indexCount, 1, s->range.indexOffset, s->range.vertexOffset, 0);
    }

    /*
            for (const auto &pair : shapes.getShapes()) {
      vk::raii::CommandBuffer &buf = frames->commandBuffers[currentFrame];

      const Shape &s = pair.second;
      PushConstant pc = PushConstant({.transformIndex = s.transformIndex});

      buf.bindPipeline(vk::PipelineBindPoint::eGraphics, s.pipeline->graphicsPipeline);
      buf.pushConstants(s.pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, vk::ArrayProxy<const PushConstant>(pc));
      buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s.pipeline->pipelineLayout, 0, *dsc->descriptorSets[currentFrame], nullptr);
      buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s.pipeline->pipelineLayout, 1, *s.material->albedo.descriptorSet, nullptr);
      buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s.pipeline->pipelineLayout, 2, *dsc->lightDescriptorSets[currentFrame], nullptr);
      buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s.pipeline->pipelineLayout, 3, *dsc->transformDescriptorSets[currentFrame], nullptr);

      buf.drawIndexed(s.range.indexCount, 1, s.range.indexOffset, s.range.vertexOffset, 0);
    }

    */
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

    app.init(win, cxt, swp, frames, img, dsc, buf);
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
