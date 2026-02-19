#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Components/Spirv.hpp"
#include "vkMaze/Objects/Vertex.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vulkan/vulkan.hpp"
#include <iostream>
#include <vulkan/vulkan_to_string.hpp>

void Pipeline::createPipeline(const PipelineDsc &dsc) {
  SpirvReflectPipeline reflected(dsc.vertPath, dsc.fragPath, pipelineLayout, *cxt);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = reflected.vkVertModule, .pName = "vertMain"};
  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = reflected.vkFragModule, .pName = "fragMain"};
  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount = attributeDescriptions.size(),
      .pVertexAttributeDescriptions = attributeDescriptions.data()

  };
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = dsc.topology};
  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = dsc.polygonMode,
      .cullMode = dsc.cullModeFlags,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasSlopeFactor = 1.0f,
      .lineWidth = 1.0f

  };

  vk::PipelineDepthStencilStateCreateInfo depthStencil{
      .depthTestEnable = vk::True,
      .depthWriteEnable = vk::True,
      .depthCompareOp = vk::CompareOp::eLess,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False

  };

  vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{.blendEnable = vk::False,
                                                             .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

  std::vector dynamicStates = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

  vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
      {.stageCount = 2,
       .pStages = shaderStages,
       .pVertexInputState = &vertexInputInfo,
       .pInputAssemblyState = &inputAssembly,
       .pViewportState = &viewportState,
       .pRasterizationState = &rasterizer,
       .pMultisampleState = &multisampling,
       .pDepthStencilState = &depthStencil,
       .pColorBlendState = &colorBlending,
       .pDynamicState = &dynamicState,
       .layout = pipelineLayout,
       .renderPass = nullptr},
      {.colorAttachmentCount = 1, .pColorAttachmentFormats = &swp->swapChainSurfaceFormat.format, .depthAttachmentFormat = img->findDepthFormat()}};

  graphicsPipeline = vk::raii::Pipeline(cxt->device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
  shaderResources = reflected.shaderResources;
  pcRange = reflected.pcRange;
  hasPushConstants = reflected.hasPushConstants;
}
bool Pipeline::usesSet(uint32_t setNum) {
  for (ShaderResource &s : shaderResources) {
    if (s.set == setNum)
      return true;
  }
  return false;
}

[[nodiscard]] vk::raii::ShaderModule Pipeline::createShaderModule(const std::vector<char> &code) const {
  vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())};
  vk::raii::ShaderModule shaderModule{cxt->device, createInfo};

  return shaderModule;
}
