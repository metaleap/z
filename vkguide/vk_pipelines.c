#include "./vkguide.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>



typedef struct SpirVBinary {
  uint32_t* words;   // SPIR-V words
  size_t    size;    // number of words in SPIR-V binary
} SpirVBinary;



SpirVBinary compileShaderToSPIRV_Vulkan(glslang_stage_t stage, const char* shaderSource, const char* fileName) {
  const glslang_input_t input = {
      .language                          = GLSLANG_SOURCE_GLSL,
      .stage                             = stage,
      .client                            = GLSLANG_CLIENT_VULKAN,
      .client_version                    = GLSLANG_TARGET_VULKAN_1_4,
      .target_language                   = GLSLANG_TARGET_SPV,
      .target_language_version           = GLSLANG_TARGET_SPV_1_6,
      .code                              = shaderSource,
      .default_version                   = 100,
      .default_profile                   = GLSLANG_NO_PROFILE,
      .force_default_version_and_profile = false,
      .forward_compatible                = false,
      .messages                          = GLSLANG_MSG_DEFAULT_BIT,
      .resource                          = glslang_default_resource(),
  };

  glslang_shader_t* shader = glslang_shader_create(&input);

  SpirVBinary bin = {
      .words = nullptr,
      .size  = 0,
  };
  if (!glslang_shader_preprocess(shader, &input)) {
    printf("GLSL preprocessing failed %s\n", fileName);
    printf("%s\n", glslang_shader_get_info_log(shader));
    printf("%s\n", glslang_shader_get_info_debug_log(shader));
    printf("%s\n", input.code);
    glslang_shader_delete(shader);
    return bin;
  }

  if (!glslang_shader_parse(shader, &input)) {
    printf("GLSL parsing failed %s\n", fileName);
    printf("%s\n", glslang_shader_get_info_log(shader));
    printf("%s\n", glslang_shader_get_info_debug_log(shader));
    printf("%s\n", glslang_shader_get_preprocessed_code(shader));
    glslang_shader_delete(shader);
    return bin;
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
    printf("GLSL linking failed %s\n", fileName);
    printf("%s\n", glslang_program_get_info_log(program));
    printf("%s\n", glslang_program_get_info_debug_log(program));
    glslang_program_delete(program);
    glslang_shader_delete(shader);
    return bin;
  }

  glslang_program_SPIRV_generate(program, stage);

  bin.size  = glslang_program_SPIRV_get_size(program);
  bin.words = malloc(bin.size * sizeof(uint32_t));
  glslang_program_SPIRV_get(program, bin.words);

  const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
  if (spirv_messages)
    printf("(%s) %s\b", fileName, spirv_messages);

  glslang_program_delete(program);
  glslang_shader_delete(shader);

  return bin;
}



VkResult vlkLoadShaderModule(char* filePath, VkDevice device, VkShaderModule* retShaderModule,
                             VkShaderStageFlagBits shaderStage) {
  size_t fileSize;
  void*  bytes = SDL_LoadFile(filePath, &fileSize);
  SDL_CHECK(bytes);
  glslang_stage_t shader_stage;
  switch (shaderStage) {
    case VK_SHADER_STAGE_COMPUTE_BIT:
      shader_stage = GLSLANG_STAGE_COMPUTE;
      break;
    case VK_SHADER_STAGE_VERTEX_BIT:
      shader_stage = GLSLANG_STAGE_VERTEX;
      break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      shader_stage = GLSLANG_STAGE_FRAGMENT;
      break;
    default:
      assert(false && "shaderStage");
  }
  SpirVBinary bin = compileShaderToSPIRV_Vulkan(shader_stage, bytes, filePath);
  if (bin.words == nullptr)
    exit(1);
  VkShaderModuleCreateInfo create = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .pCode = bin.words, .codeSize = (sizeof(Uint32) * bin.size)};
  return vkCreateShaderModule(device, &create, nullptr, retShaderModule);
}



void PipelineBuilder_reset(PipelineBuilder* this) {
  this->inputAssembly =
      (VkPipelineInputAssemblyStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  this->rasterizer =
      (VkPipelineRasterizationStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  this->colorBlendAttachment = (VkPipelineColorBlendAttachmentState) {};
  this->multisampling =
      (VkPipelineMultisampleStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  this->pipelineLayout = nullptr;
  this->depthStencil =
      (VkPipelineDepthStencilStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  this->renderInfo = (VkPipelineRenderingCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
}



void PipelineBuilder_setShaders(PipelineBuilder* this, VkShaderModule vertShader, VkShaderModule fragShader) {
  PipelineBuilder_reset(this);
  this->shaderStages[0] = vlkPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader, "main");
  this->shaderStages[1] = vlkPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader, "main");
}



void PipelineBuilder_setInputTopology(PipelineBuilder* this, VkPrimitiveTopology topo) {
  this->inputAssembly.topology = topo;
}



void PipelineBuilder_setPolygonMode(PipelineBuilder* this, VkPolygonMode mode) {
  this->rasterizer.polygonMode = mode;
  this->rasterizer.lineWidth   = 1.0f;
}



void PipelineBuilder_setCullMode(PipelineBuilder* this, VkCullModeFlags cullMode, VkFrontFace frontFace) {
  this->rasterizer.cullMode  = cullMode;
  this->rasterizer.frontFace = frontFace;
}



void PipelineBuilder_setMultisamplingNone(PipelineBuilder* this) {
  this->multisampling.sampleShadingEnable   = VK_FALSE;
  this->multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  this->multisampling.minSampleShading      = 1.0f;
  this->multisampling.pSampleMask           = nullptr;
  this->multisampling.alphaToCoverageEnable = VK_FALSE;
  this->multisampling.alphaToOneEnable      = VK_FALSE;
}



void PipelineBuilder_disableBlending(PipelineBuilder* this) {
  this->colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  this->colorBlendAttachment.blendEnable = VK_FALSE;
}



void PipelineBuilder_enableBlending(PipelineBuilder* this, VkBlendFactor dstColorBlendFactor) {
  this->colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  this->colorBlendAttachment.blendEnable         = VK_TRUE;
  this->colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  this->colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;
  this->colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
  this->colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  this->colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  this->colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
}
void PipelineBuilder_enableBlendingAdditive(PipelineBuilder* this) {
  PipelineBuilder_enableBlending(this, VK_BLEND_FACTOR_ONE);
}
void PipelineBuilder_enableBlendingAlphaBlend(PipelineBuilder* this) {
  PipelineBuilder_enableBlending(this, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
}



void PipelineBuilder_setColorAttachmentFormat(PipelineBuilder* this, VkFormat format) {
  this->colorAttachmentFormat              = format;
  this->renderInfo.colorAttachmentCount    = 1;
  this->renderInfo.pColorAttachmentFormats = &this->colorAttachmentFormat;
}



void PipelineBuilder_setDepthFormat(PipelineBuilder* this, VkFormat format) {
  this->renderInfo.depthAttachmentFormat = format;
}



void PipelineBuilder_disableDepthTest(PipelineBuilder* this) {
  this->depthStencil.depthTestEnable       = VK_FALSE;
  this->depthStencil.depthWriteEnable      = VK_FALSE;
  this->depthStencil.depthCompareOp        = VK_COMPARE_OP_NEVER;
  this->depthStencil.depthBoundsTestEnable = VK_FALSE;
  this->depthStencil.stencilTestEnable     = VK_FALSE;
  this->depthStencil.front                 = (VkStencilOpState) {};
  this->depthStencil.back                  = (VkStencilOpState) {};
  this->depthStencil.minDepthBounds        = 0;
  this->depthStencil.maxDepthBounds        = 1;
}



void PipelineBuilder_enableDepthTest(PipelineBuilder* this, bool depthWriteEnable, VkCompareOp opCmp) {
  this->depthStencil.depthTestEnable       = VK_TRUE;
  this->depthStencil.depthWriteEnable      = depthWriteEnable;
  this->depthStencil.depthCompareOp        = opCmp;
  this->depthStencil.depthBoundsTestEnable = VK_FALSE;
  this->depthStencil.stencilTestEnable     = VK_FALSE;
  this->depthStencil.front                 = (VkStencilOpState) {};
  this->depthStencil.back                  = (VkStencilOpState) {};
  this->depthStencil.minDepthBounds        = 0;
  this->depthStencil.maxDepthBounds        = 1;
}



VkPipeline PipelineBuilder_build(PipelineBuilder* this, VkDevice device) {
  VkPipelineViewportStateCreateInfo viewport = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};
  VkPipelineColorBlendStateCreateInfo  color_blending = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                         .logicOpEnable   = VK_FALSE,
                                                         .logicOp         = VK_LOGIC_OP_COPY,
                                                         .attachmentCount = 1,
                                                         .pAttachments    = &this->colorBlendAttachment};
  VkPipelineVertexInputStateCreateInfo vertex_input = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkDynamicState                   state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dyn     = {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .pDynamicStates = &state[0], .dynamicStateCount = 2};
  VkGraphicsPipelineCreateInfo pipeline = {.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                           .pNext               = &this->renderInfo,
                                           .stageCount          = ARR_LEN(this->shaderStages),
                                           .pStages             = this->shaderStages,
                                           .pVertexInputState   = &vertex_input,
                                           .pInputAssemblyState = &this->inputAssembly,
                                           .pViewportState      = &viewport,
                                           .pRasterizationState = &this->rasterizer,
                                           .pMultisampleState   = &this->multisampling,
                                           .pColorBlendState    = &color_blending,
                                           .pDepthStencilState  = &this->depthStencil,
                                           .layout              = this->pipelineLayout,
                                           .pDynamicState       = &dyn};
  VkPipeline                   ret;
  auto                         ok = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &ret);
  VK_CHECK(ok);
  if (VK_SUCCESS != ok)
    ret = VK_NULL_HANDLE;
  return ret;
}
