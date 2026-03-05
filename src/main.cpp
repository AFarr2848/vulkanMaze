#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/Window.hpp"
#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Components/Buffers.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Objects/PostProcessingPass.hpp"
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
#include <vkMaze/Components/RenderPass.hpp>

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

Window win;
VulkanContext cxt;
Swapchain swp;
FrameData frames;
Buffers buf;
Images img;
Descriptors dsc;
RenderPass renderPass;

class VKMaze : public VulkanEngine {
public:
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
        materials.get("blue")

    );

    shapes.add(
        "light_sphere2",
        Icosphere(3),
        glm::vec3(0.0f, 5.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.2f),
        *&pipelinePhong,
        materials.get("red")

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
        glm::vec3(.0f, .0f, .5f),
        1.0f

    );

    lights.addPointLight(
        "orbit_light_vertical",
        shapes.get("light_sphere2").pos,
        glm::vec3(.5f, 0, 0),
        1.0f

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
    img->transition_image_layout(
        img->postImages[img->currentPostView],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eShaderRead,            // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,  // dstAccessMask
        vk::PipelineStageFlagBits2::eFragmentShader, // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor // dstStage
    );
    renderPass.record(frames->commandBuffers[currentFrame], currentFrame, img->postImageViews[img->currentPostView], img->depthImageView);

    img->transition_image_layout(

        img->postImages[img->currentPostView],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eShaderRead,                   // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eFragmentShader,
        vk::ImageAspectFlagBits::eColor // dstStage
    );

    for (PostProcessingPass &p : PPPasses) {
      p.record(frames->commandBuffers[currentFrame], currentFrame);
    }
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
  Pipeline ppPipeline;
  std::vector<Pipeline *> pipelines = {&pipelinePhong, &pipelineUnlit, &pipelineWireframe};
  std::vector<PostProcessingPass> PPPasses;

  void createPipelines() override {

    pipelinePhong.init(*cxt, *dsc, *swp, *img);
    pipelinePhong.createPipeline({

        .fragPath = "build/shaders/fragPhong.spv",
        .vertPath = "build/shaders/vertBasic.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,

    });
    pipelineUnlit.init(*cxt, *dsc, *swp, *img);
    pipelineUnlit.createPipeline({

        .fragPath = "build/shaders/fragUnlit.spv",
        .vertPath = "build/shaders/vertBasic.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eFill,
        .cullModeFlags = vk::CullModeFlagBits::eBack,

    });
    pipelineWireframe.init(*cxt, *dsc, *swp, *img);
    pipelineWireframe.createPipeline({

        .fragPath = "build/shaders/fragUnlit.spv",
        .vertPath = "build/shaders/vertBasic.spv",
        .topology = vk::PrimitiveTopology::eTriangleList,
        .polygonMode = vk::PolygonMode::eLine,
        .cullModeFlags = vk::CullModeFlagBits::eBack,

    });
    std::cout << "\n\ncreating pp pipeline" << std::endl;
    ppPipeline.init(*cxt, *dsc, *swp, *img);
    ppPipeline.createPipeline({.fragPath = "build/shaders/fragBoxBlur.spv",
                               .vertPath = "build/shaders/vertPost.spv",
                               .topology = vk::PrimitiveTopology::eTriangleList,
                               .polygonMode = vk::PolygonMode::eFill,
                               .cullModeFlags = vk::CullModeFlagBits::eNone});
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
    renderPass.init({.shapes = shapes, .lights = lights}, *cxt, *dsc, *swp, *buf);
    renderPass.createGlobalDscSets();
    std::cout << "renderPass init complete" << std::endl;
    std::cout << "renderPass dsc sets complete" << std::endl;
    PPPasses.push_back(PostProcessingPass({.fragPath = "build/shaders/fragBoxBlur.spv", .vertPath = "build/shaders/vertPost.spv"}, *cxt, *dsc, *swp, *buf, *img));

    std::cout << "ppp dsc sets complete" << std::endl;

    shapes.dscSets = dsc->createTransformDescriptorSets();
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
    for (PostProcessingPass &p : PPPasses)
      p.createPPPDscSets(img->postImageViews, img->postImageSampler);
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
