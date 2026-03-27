#include "vkMaze/Components/EngineConfig.hpp"
#include "vkMaze/Components/RenderGraph.hpp"
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
#include <cstdint>
#include <vector>
#include <iostream>
#include <vkMaze/Objects/Shapes.hpp>
#include <vkMaze/Components/Swapchain.hpp>
#include <vkMaze/Objects/UBOs.hpp>
#include <vkMaze/Components/FrameData.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include <vkMaze/Components/Managers.hpp>
#include <vkMaze/Components/Spirv.hpp>

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

Window win;
VulkanContext cxt;
Swapchain swp;
FrameData frames;
Buffers buf;
Images img;
Descriptors dsc;

class VKMaze : public VulkanEngine {
public:
  RenderGraph renderGraph;

  void makeShapes() override {
    std::cout << "Light size:" << sizeof(SSBOLight) << std::endl;
    std::cout << "Offsets:" << std::endl;
    std::cout << "\tPos: " << offsetof(SSBOLight, pos) << std::endl;
    std::cout << "\tDir: " << offsetof(SSBOLight, dir) << std::endl;
    std::cout << "\tColor: " << offsetof(SSBOLight, color) << std::endl;
    std::cout << "\tType: " << offsetof(SSBOLight, type) << std::endl;
    std::cout << "\tBrightness: " << offsetof(SSBOLight, brightness) << std::endl;
    std::cout << "\tParam1: " << offsetof(SSBOLight, param1) << std::endl;
    std::cout << "\tParam2: " << offsetof(SSBOLight, param2) << std::endl;

    materials.create("default", "", "");
    materials.create("backpack", "textures/backpack_albedo.jpg", "");
    materials.create("earth", "textures/earth.png", "");
    materials.color("black", glm::vec3(0));
    materials.color("purple", glm::vec3(100, 20, 100));
    materials.color("blue", glm::vec3(0, 0, 100));
    materials.color("red", glm::vec3(100, 0, 0));
    materials.color("white", glm::vec3(1000, 1000, 1000));

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
        *&pipelinePhong,
        materials.get("white")

    );

    shapes.add(
        "light_sphere2",
        Icosphere(3),
        glm::vec3(0.0f, 5.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.2f),
        *&pipelinePhong,
        materials.get("white")

    );

    lights.addDirLight(
        "sun",
        glm::vec3(0.3, 0.3, 0.3),
        glm::vec3(0.0f),
        .0f

    );

    lights.addPointLight(
        "orbit_light",
        shapes.get("light_sphere").pos,
        glm::vec3(1.0f, 1.0f, 1.0f),
        6.0f

    );

    lights.addPointLight(
        "orbit_light_vertical",
        shapes.get("light_sphere2").pos,
        glm::vec3(1.0f, 1.0f, 1.0f),
        6.0f

    );

    /*
        lights.addSpotLight(
            "flashlight",
            camera.Position,
            camera.Front,
            glm::vec3(.3f),
            1.0f,
            glm::cos(glm::radians(8.0f)),
            glm::cos(glm::radians(12.5f))

        );
        */

    // Cube lightCube(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)), glm::vec3(2.25f, 2.25f, 2.25f)));
  }

  void drawScreen(uint32_t imageIndex) override {

    renderGraph.execute(frames->commandBuffers[currentFrame], currentFrame, imageIndex);
  }

  void makeMaterials() override {
    materials.initMaterials(*cxt, *img, *frames, *dsc, *buf);
  }

private:
  ShapeManager shapes;
  MaterialManager materials;
  LightManager lights;
  Pipeline pipelinePhong;
  Pipeline pipelineUnlit;
  Pipeline pipelineWireframe;
  std::vector<Pipeline *> pipelines = {&pipelinePhong, &pipelineUnlit, &pipelineWireframe};

  void createPipelines() override {

    renderGraph.init(shapes, lights, *cxt, *swp, *img, *dsc, *buf);

    renderGraph.addImage({
        .name = "brightness",
        .format = swp->swapChainSurfaceFormat.format,
        .extent = swp->swapChainExtent,
        .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .samples = vk::SampleCountFlagBits::e1,
        .initialLayout = vk::ImageLayout::eUndefined,
        .isExternal = false,
    });

    renderGraph.addPass("main", {}, {{.resource = "color", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput}, {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests}}, MAIN_PASS);

    PipelineDsc bloomDsc = {
        .fragPath = "build/shaders/fragBloom.spv",
        .vertPath = "build/shaders/vertPost.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eNone,
    };

    PipelineDsc brightnessDsc = {
        .fragPath = "build/shaders/fragSeperateBrightness.spv",
        .vertPath = "build/shaders/vertPost.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eNone};
    renderGraph.addPass("brightness", brightnessDsc, {{.resource = "brightness", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput}, {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests}}, POST_PASS);

    renderGraph.addPass("bloom", bloomDsc, {{.resource = "color", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput}, {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests}}, POST_PASS);

    PipelineDsc presentDsc = {
        .fragPath = "build/shaders/fragBasic.spv",
        .vertPath = "build/shaders/vertPost.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eNone,
    };
    renderGraph.addPass("present", presentDsc, {{.resource = "swap", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput}, {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests}}, POST_PASS);

    pipelinePhong.init(*cxt, *swp, *img);
    pipelinePhong.createPipeline({

        .fragPath = "build/shaders/fragPhong.spv",
        .vertPath = "build/shaders/vertBasic.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,

    });
    pipelineUnlit.init(*cxt, *swp, *img);
    pipelineUnlit.createPipeline({

        .fragPath = "build/shaders/fragUnlit.spv",
        .vertPath = "build/shaders/vertBasic.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,

    });
    pipelineWireframe.init(*cxt, *swp, *img);
    pipelineWireframe.createPipeline({

        .fragPath = "build/shaders/fragUnlit.spv",
        .vertPath = "build/shaders/vertBasic.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eLine,
        .cullModeFlags = vk::CullModeFlagBits::eBack,

    });
    std::cout << "\n\ncreating pp pipeline" << std::endl;
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

  void updateLights(std::vector<SSBOLight> &lightVec) override {
    lights.get("orbit_light").pos = shapes.get("light_sphere").pos;
    lights.get("orbit_light_vertical").pos = shapes.get("light_sphere2").pos;
    // lights.get("flashlight").pos = camera.Position;
    // lights.get("flashlight").dir =26AFarrar43 camera.Front;

    for (auto &pair : lights.lights) {
      lightVec.push_back(pair.second);
    }
  };

  void updateTransforms(std::vector<glm::mat4> &transforms) override {
    shapes.get("light_sphere").pos = {6 * sin(time), 8, 6 * cos(time)};
    shapes.get("light_sphere2").pos = {-6 * sin(time), 8, -6 * cos(time)};
    shapes.updateTransform("light_sphere");
    shapes.updateTransform("light_sphere2");

    transforms.insert(transforms.begin(), shapes.transforms.begin(), shapes.transforms.end());
  };

  std::vector<uint32_t> getIndices() override {
    return shapes.indices;
  }

  void createDescriptorSets() override {
    std::cout << "inittttttt" << std::endl;
    renderGraph.makeGlobalDscSets();
    renderGraph.compile();
    lights.dscSets = dsc->createLightDescriptorSets();
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
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  void changeResolution() override {
    swp->recreateSwapChain();
    renderGraph.compile();
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
