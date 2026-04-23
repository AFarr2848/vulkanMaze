#include "vkMaze/Components/Imgui.hpp"
#include "vkMaze/Components/RenderGraph.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Components/Window.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

struct PassDraft {
  std::string name = "new_pass";
  int type = POST_PASS;
  std::string fragPath = "build/shaders/fragBasic.spv";
  std::string vertPath = "build/shaders/vertPost.spv";
  bool writeColor = true;
  bool writeDepth = true;
  bool writeSwap = false;
  bool writeCustom = false;
  std::string customWrite = "color";
  std::vector<RenderGraphReadOverride> readOverrides;
};

void VulkanImgui::startImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  // createImguiDscPool();

  VkFormat colorFormat = static_cast<VkFormat>(swp->swapChainSurfaceFormat.format);
  ImGui_ImplVulkan_InitInfo initInfo = {
      .ApiVersion = VK_API_VERSION_1_3,
      .Instance = *cxt->instance,
      .PhysicalDevice = *cxt->physicalDevice,
      .Device = *cxt->device,
      .QueueFamily = cxt->queueIndex,
      .Queue = *cxt->graphicsQueue,
      //.DescriptorPool = *dscPool,
      .DescriptorPoolSize = 1000,
      .MinImageCount = (uint32_t)swp->swapChainImages.size(),
      .ImageCount = (uint32_t)swp->swapChainImages.size(),
      .PipelineInfoMain = {
          .PipelineRenderingCreateInfo = {
              .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
              .colorAttachmentCount = 1,
              .pColorAttachmentFormats = &colorFormat,
              .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT}},
      .UseDynamicRendering = true

  };
  ImGui_ImplVulkan_Init(&initInfo);
  ImGui_ImplGlfw_InitForVulkan(win->window, true);
}

void VulkanImgui::drawRenderGraphEditorContents(RenderGraph *graph) {
  if (graph == nullptr) {
    ImGui::TextUnformatted("No RenderGraph bound.");
    return;
  }

  static int selectedPass = -1;
  static PassDraft draft;

  if (ImGui::Button("Recompile Graph")) {
    win->framebufferResized = true;
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Passes");

  size_t passToRemove = static_cast<size_t>(-1);
  size_t passCount = graph->getUncompiledPassCount();

  for (size_t i = 0; i < passCount; i++) {
    ImGui::PushID(i);
    RenderGraphPassDsc *pass = graph->getUncompiledPass(i);
    if (pass == nullptr) {
      ImGui::PopID();
      continue;
    }
    ImGui::TextUnformatted(pass->name.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
      passToRemove = i;
    }
    ImGui::PopID();
  }

  if (passToRemove != static_cast<size_t>(-1)) {
    graph->removePass(passToRemove);
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Add Pass");
  for (size_t i = 0; i < passesToAdd.size(); i++) {
    ImGui::PushID(i);
    ImGui::TextUnformatted(passesToAdd.at(i).name.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Add")) {
      for (RenderGraphPassDsc &dsc : passesToAdd.at(i).passes)
        graph->addPass(dsc.name, dsc.pipelineDsc, dsc.readOverrides, dsc.writes, dsc.type);
    }
    ImGui::PopID();
  }
}

void VulkanImgui::makePassesToAdd() {

  passesToAdd.push_back({

      .name = "gaussianBlur",
      .passes = {
          {

              .name = "gaussV",
              .type = POST_PASS,
              .pipelineDsc = {
                  .fragPath = "build/shaders/fragGaussianBlurV.spv",
                  .vertPath = "build/shaders/vertPost.spv",
                  .topology = vk::PrimitiveTopology::eTriangleList,
                  .polygonMode = vk::PolygonMode::eFill,
                  .cullModeFlags = vk::CullModeFlagBits::eNone,
              },
              .readOverrides = {},
              .writes = {
                  {.resource = "color", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
                  {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests},
              },
          },
          {
              .name = "gaussH",
              .type = POST_PASS,
              .pipelineDsc = {
                  .fragPath = "build/shaders/fragGaussianBlurH.spv",
                  .vertPath = "build/shaders/vertPost.spv",
                  .topology = vk::PrimitiveTopology::eTriangleList,
                  .polygonMode = vk::PolygonMode::eFill,
                  .cullModeFlags = vk::CullModeFlagBits::eNone,
              },
              .writes = {
                  {.resource = "color", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
                  {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests},
              },
          },

      }

  });

  passesToAdd.push_back({

      .name = "bloom",
      .passes = {
          {

              .name = "reflection",
              .type = POST_PASS,
              .pipelineDsc = {
                  .fragPath = "build/shaders/fragSeperateBrightness.spv",
                  .vertPath = "build/shaders/vertPost.spv",
                  .topology = vk::PrimitiveTopology::eTriangleList,
                  .polygonMode = vk::PolygonMode::eFill,
                  .cullModeFlags = vk::CullModeFlagBits::eNone,
              },
              .readOverrides = {},
              .writes = {
                  {.resource = "brightness", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
                  {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests},
              },
          },

          {

              .name = "bloomGaussV",
              .type = POST_PASS,
              .pipelineDsc = {
                  .fragPath = "build/shaders/fragGaussianBlurV.spv",
                  .vertPath = "build/shaders/vertPost.spv",
                  .topology = vk::PrimitiveTopology::eTriangleList,
                  .polygonMode = vk::PolygonMode::eFill,
                  .cullModeFlags = vk::CullModeFlagBits::eNone,
              },
              .readOverrides = {{.shaderResource = "color", .rgResource = "brightness"}},
              .writes = {
                  {.resource = "scratch", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
                  {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests},
              },
          },
          {
              .name = "bloomGaussH",
              .type = POST_PASS,
              .pipelineDsc = {
                  .fragPath = "build/shaders/fragGaussianBlurH.spv",
                  .vertPath = "build/shaders/vertPost.spv",
                  .topology = vk::PrimitiveTopology::eTriangleList,
                  .polygonMode = vk::PolygonMode::eFill,
                  .cullModeFlags = vk::CullModeFlagBits::eNone,
              },
              .readOverrides = {{.shaderResource = "color", .rgResource = "scratch"}},
              .writes = {
                  {.resource = "scratch", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
                  {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests},
              },
          },
          {
              .name = "bloomGaussCombine",
              .type = POST_PASS,
              .pipelineDsc = {
                  .fragPath = "build/shaders/fragCombineImages.spv",
                  .vertPath = "build/shaders/vertPost.spv",
                  .topology = vk::PrimitiveTopology::eTriangleList,
                  .polygonMode = vk::PolygonMode::eFill,
                  .cullModeFlags = vk::CullModeFlagBits::eNone,
              },
              .readOverrides = {{.shaderResource = "image2", .rgResource = "scratch"}},
              .writes = {
                  {.resource = "color", .layout = vk::ImageLayout::eColorAttachmentOptimal, .access = vk::AccessFlagBits2::eColorAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput},
                  {.resource = "depth", .layout = vk::ImageLayout::eDepthAttachmentOptimal, .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite, .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests},
              },
          }

      }

  });
}

void VulkanImgui::drawRightSidePanel(RenderGraph *graph) {
  ImGuiIO &io = ImGui::GetIO();
  const float panelWidth = 350.0f; // Adjust as needed

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImVec2 viewportPos = viewport->Pos;
  ImVec2 viewportSize = viewport->Size;

  ImGui::SetNextWindowPos(
      ImVec2(viewportPos.x + viewportSize.x - panelWidth, viewportPos.y),
      ImGuiCond_Always);

  ImGui::SetNextWindowSize(
      ImVec2(panelWidth, viewportSize.y),
      ImGuiCond_Always);

  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoCollapse;

  ImGui::Begin("Inspector", nullptr, flags);
  ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

  ImGui::Separator();

  ImGui::BeginChild("RenderGraphEditor", ImVec2(0, 0), false);
  drawRenderGraphEditorContents(graph);
  ImGui::EndChild();

  ImGui::End();
}

void VulkanImgui::renderImgui(vk::raii::CommandBuffer &cmd) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Define UI
  drawRightSidePanel(renderGraph);

  // Finalize UI
  ImGui::Render();

  ImGui_ImplVulkan_RenderDrawData(
      ImGui::GetDrawData(),
      *cmd

  );
}
