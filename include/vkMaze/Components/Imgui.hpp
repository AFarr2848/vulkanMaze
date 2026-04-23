#pragma once
class VulkanContext;
class Descriptors;
class Swapchain;
class Window;
class RenderGraph;
class RenderGraphPassDsc;

class VulkanImgui {
public:
  void init(VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Window &win) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->win = &win;
    startImgui();
    makePassesToAdd();
  }

  void setRenderGraph(RenderGraph &graph) { renderGraph = &graph; }
  void createImguiDscPool();
  void renderImgui(vk::raii::CommandBuffer &cmd);
  void makePassesToAdd();

private:
  VulkanContext *cxt = nullptr;
  Descriptors *dsc = nullptr;
  Swapchain *swp = nullptr;
  Window *win = nullptr;
  RenderGraph *renderGraph = nullptr;

  vk::raii::DescriptorPool dscPool = nullptr;
  struct RenderGraphPassDscGroup {
    std::string name;
    std::vector<RenderGraphPassDsc> passes;
  };
  std::vector<RenderGraphPassDscGroup> passesToAdd;

  void startImgui();
  void drawRightSidePanel(RenderGraph *graph);
  void drawRenderGraphEditorContents(RenderGraph *graph);
};
