#include "vkMaze/Objects/Pipelines.hpp"
#include "vkMaze/Objects/Vertex.hpp"
#include "vkMaze/Components/Descriptors.hpp"
#include "vkMaze/Components/Swapchain.hpp"
#include "vkMaze/Components/Images.hpp"
#include "vkMaze/Components/VulkanContext.hpp"
#include "vkMaze/Util.hpp"
#include "vkMaze/Objects/UBOs.hpp"
#include "vulkan/vulkan.hpp"

void Pipeline::createPipeline(const PipelineDsc &dsc) {
  vk::raii::ShaderModule shaderModule = createShaderModule(readFile(dsc.shaderPath));

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
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

  vk::PushConstantRange pushConstantRange{
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0,
      .size = sizeof(PushConstant),

  };

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(dsc.setLayouts.size()),
      .pSetLayouts = dsc.setLayouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange

  };

  pipelineLayout = vk::raii::PipelineLayout(cxt->device, pipelineLayoutInfo);

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
}

[[nodiscard]] vk::raii::ShaderModule Pipeline::createShaderModule(const std::vector<char> &code) const {
  vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())};
  vk::raii::ShaderModule shaderModule{cxt->device, createInfo};

  return shaderModule;
}
