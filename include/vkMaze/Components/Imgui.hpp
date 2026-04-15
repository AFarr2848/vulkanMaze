

class VulkanContext;
class Descriptors;
class Swapchain;
class Window;
class RenderGraph;

class Imgui {
public:
  void init(VulkanContext &cxt, Descriptors &dsc, Swapchain &swp, Window &win) {
    this->cxt = &cxt;
    this->dsc = &dsc;
    this->swp = &swp;
    this->win = &win;
    startImgui();
  }

  void setRenderGraph(RenderGraph &graph) { renderGraph = &graph; }
  void createImguiDscPool();
  void renderImgui(vk::raii::CommandBuffer &cmd);

private:
  VulkanContext *cxt = nullptr;
  Descriptors *dsc = nullptr;
  Swapchain *swp = nullptr;
  Window *win = nullptr;
  RenderGraph *renderGraph = nullptr;
  vk::raii::DescriptorPool dscPool = nullptr;

  void startImgui();
  void drawRightSidePanel(RenderGraph *graph);
  void drawRenderGraphEditorContents(RenderGraph *graph);
};
