#include "vkMaze/Components/RenderGraph.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vulkan/vulkan.hpp"

// chatgpt did NOT do this stuff!

RenderGraphPass &RenderGraph::addPass(std::string name) {
  RenderGraphPass pass = {.name = name};
  passes.push_back(std::move(pass));
  return passes.at(passes.size() - 1);
}

uint32_t RenderGraph::addImage(const RenderGraphResourceDesc &desc) {
  RenderGraphResource resource;
  resource.desc = desc;
  resources.push_back(std::move(resource));
  return resources.size() - 1;
}

uint32_t RenderGraph::addExternalImage(const RenderGraphResourceDesc &desc, vk::raii::Image &image, vk::raii::ImageView &view) {
  RenderGraphResource resource;
  resource.isExternal = true;
  resource.extImage = &image;
  resource.extView = &view;
  resource.desc = desc;
  resource.layout = desc.initialLayout;
  resources.push_back(std::move(resource));
  return resources.size() - 1;
}

void RenderGraph::compile() {
  for (RenderGraphPass &pass : passes) {
  }
  for (RenderGraphResource &r : resources) {
    if (r.isExternal == false) {
      img->createImage(r.desc.format, vk::ImageTiling::eLinear, r.desc.usage, vk::MemoryPropertyFlagBits::eDeviceLocal, r.image, r.memory);
      img->createImageView(r.image, r.desc.format, r.desc.aspect);
      r.layout = r.desc.initialLayout;
    }
  }
}
