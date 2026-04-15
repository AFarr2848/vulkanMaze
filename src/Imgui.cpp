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

void Imgui::startImgui() {
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

void Imgui::drawRenderGraphEditorContents(RenderGraph *graph) {
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

  bool removedPass = false;

  /*
  for (size_t i = 0; i < passCount; i++) {
    ImGui::PushID(i);
    RenderGraphPass *pass = graph->getPass(i);
    ImGui::TextUnformatted(pass->name.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
      graph->removePass(i);
    }
    ImGui::PopID();
  }


  ImGui::Separator();
  ImGui::TextUnformatted("Selected Pass");
  passCount = graph->getPassCount();
  if (selectedPass >= 0 && static_cast<size_t>(selectedPass) < passCount) {
    RenderGraphPass *pass = graph->getPass(static_cast<size_t>(selectedPass));
    if (pass != nullptr) {
      ImGui::InputText("Name", &pass->name);
      const char *typeLabels[] = {"Main", "Post", "GUI"};
      int passType = static_cast<int>(pass->type);
      if (ImGui::Combo("Type", &passType, typeLabels, IM_ARRAYSIZE(typeLabels))) {
        pass->type = static_cast<PassType>(passType);
      }

      ImGui::TextUnformatted("Reads");
      for (const auto &read : pass->reads) {
        ImGui::BulletText("%s", read.resource.c_str());
      }
      ImGui::TextUnformatted("Writes");
      for (const auto &write : pass->writes) {
        ImGui::BulletText("%s", write.resource.c_str());
      }
    }
  } else {
    ImGui::TextUnformatted("Select a pass to edit.");
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Add Pass");
  ImGui::InputText("New Name", &draft.name);
  const char *typeLabels[] = {"Main", "Post", "GUI"};
  ImGui::Combo("New Type", &draft.type, typeLabels, IM_ARRAYSIZE(typeLabels));
  if (draft.type != MAIN_PASS) {
    ImGui::InputText("Frag Shader", &draft.fragPath);
    ImGui::InputText("Vert Shader", &draft.vertPath);
  }
  ImGui::Checkbox("Write Color", &draft.writeColor);
  ImGui::Checkbox("Write Depth", &draft.writeDepth);
  ImGui::Checkbox("Write Swap", &draft.writeSwap);
  ImGui::Checkbox("Write Custom", &draft.writeCustom);
  if (draft.writeCustom) {
    ImGui::InputText("Custom Target", &draft.customWrite);
  }

  ImGui::TextUnformatted("Read Overrides");
  for (size_t i = 0; i < draft.readOverrides.size(); i++) {
    ImGui::PushID(static_cast<int>(i));
    ImGui::InputText("Shader Resource", &draft.readOverrides[i].shaderResource);
    ImGui::InputText("RG Resource", &draft.readOverrides[i].rgResource);
    if (ImGui::SmallButton("Remove Override")) {
      draft.readOverrides.erase(draft.readOverrides.begin() + static_cast<long>(i));
      ImGui::PopID();
      break;
    }
    ImGui::Separator();
    ImGui::PopID();
  }
  if (ImGui::SmallButton("Add Override")) {
    draft.readOverrides.push_back({.shaderResource = "color", .rgResource = "color"});
  }

  if (ImGui::Button("Create Pass")) {
    std::vector<RenderGraphAccess> writes;
    writes.reserve(4);
    if (draft.writeColor) {
      writes.push_back({.resource = "color",
                        .layout = vk::ImageLayout::eColorAttachmentOptimal,
                        .access = vk::AccessFlagBits2::eColorAttachmentWrite,
                        .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput});
    }
    if (draft.writeDepth) {
      writes.push_back({.resource = "depth",
                        .layout = vk::ImageLayout::eDepthAttachmentOptimal,
                        .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                        .stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests});
    }
    if (draft.writeSwap) {
      writes.push_back({.resource = "swap",
                        .layout = vk::ImageLayout::eColorAttachmentOptimal,
                        .access = vk::AccessFlagBits2::eColorAttachmentWrite,
                        .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput});
    }
    if (draft.writeCustom && !draft.customWrite.empty()) {
      writes.push_back({.resource = draft.customWrite,
                        .layout = vk::ImageLayout::eColorAttachmentOptimal,
                        .access = vk::AccessFlagBits2::eColorAttachmentWrite,
                        .stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput});
    }

    PipelineDsc dsc = {};
    if (draft.type != MAIN_PASS) {
      dsc.fragPath = draft.fragPath;
      dsc.vertPath = draft.vertPath;
      dsc.topology = vk::PrimitiveTopology::eTriangleList;
      dsc.polygonMode = vk::PolygonMode::eFill;
      dsc.cullModeFlags = vk::CullModeFlagBits::eNone;
    }

    graph->addPass(draft.name, dsc, draft.readOverrides, writes, static_cast<PassType>(draft.type));
    graph->compile();
  }
  */
}

void Imgui::drawRightSidePanel(RenderGraph *graph) {
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

  ImGui::Text("Right-Side Panel");
  ImGui::Separator();

  ImGui::BeginChild("RenderGraphEditor", ImVec2(0, 0), false);
  drawRenderGraphEditorContents(graph);
  ImGui::EndChild();

  ImGui::End();
}

void Imgui::renderImgui(vk::raii::CommandBuffer &cmd) {
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
