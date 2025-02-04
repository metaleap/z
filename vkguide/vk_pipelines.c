#include "./vkguide.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>
#include <vulkan/vulkan_core.h>



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



void PipelineBuilder_reset(PipelineBuilder* self) {
  self->inputAssembly =
      (VkPipelineInputAssemblyStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  self->rasterizer =
      (VkPipelineRasterizationStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  self->colorBlendAttachment = (VkPipelineColorBlendAttachmentState) {};
  self->multisampling =
      (VkPipelineMultisampleStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  self->pipelineLayout = nullptr;
  self->depthStencil =
      (VkPipelineDepthStencilStateCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  self->renderInfo = (VkPipelineRenderingCreateInfo) {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
}



void PipelineBuilder_setShaders(PipelineBuilder* self, VkShaderModule vertShader, VkShaderModule fragShader) {
  PipelineBuilder_reset(self);
  self->shaderStages[0] = vlkPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader, "main");
  self->shaderStages[1] = vlkPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader, "main");
}



void PipelineBuilder_setInputTopology(PipelineBuilder* self, VkPrimitiveTopology topo) {
  self->inputAssembly.topology = topo;
}



void PipelineBuilder_setPolygonMode(PipelineBuilder* self, VkPolygonMode mode) {
  self->rasterizer.polygonMode = mode;
  self->rasterizer.lineWidth   = 1.0f;
}



void PipelineBuilder_setCullMode(PipelineBuilder* self, VkCullModeFlags cullMode, VkFrontFace frontFace) {
  self->rasterizer.cullMode  = cullMode;
  self->rasterizer.frontFace = frontFace;
}



void PipelineBuilder_setMultisamplingNone(PipelineBuilder* self) {
  self->multisampling.sampleShadingEnable   = VK_FALSE;
  self->multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  self->multisampling.minSampleShading      = 1.0f;
  self->multisampling.pSampleMask           = nullptr;
  self->multisampling.alphaToCoverageEnable = VK_FALSE;
  self->multisampling.alphaToOneEnable      = VK_FALSE;
}



void PipelineBuilder_disableBlending(PipelineBuilder* self) {
  self->colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  self->colorBlendAttachment.blendEnable = VK_FALSE;
}



void PipelineBuilder_enableBlending(PipelineBuilder* self, VkBlendFactor dstColorBlendFactor) {
  self->colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  self->colorBlendAttachment.blendEnable         = VK_TRUE;
  self->colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  self->colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;
  self->colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
  self->colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  self->colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  self->colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
}
void PipelineBuilder_enableBlendingAdditive(PipelineBuilder* self) {
  PipelineBuilder_enableBlending(self, VK_BLEND_FACTOR_ONE);
}
void PipelineBuilder_enableBlendingAlphaBlend(PipelineBuilder* self) {
  PipelineBuilder_enableBlending(self, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
}



void PipelineBuilder_setColorAttachmentFormat(PipelineBuilder* self, VkFormat format) {
  self->colorAttachmentFormat              = format;
  self->renderInfo.colorAttachmentCount    = 1;
  self->renderInfo.pColorAttachmentFormats = &self->colorAttachmentFormat;
}



void PipelineBuilder_setDepthFormat(PipelineBuilder* self, VkFormat format) {
  self->renderInfo.depthAttachmentFormat = format;
}



void PipelineBuilder_disableDepthTest(PipelineBuilder* self) {
  self->depthStencil.depthTestEnable       = VK_FALSE;
  self->depthStencil.depthWriteEnable      = VK_FALSE;
  self->depthStencil.depthCompareOp        = VK_COMPARE_OP_NEVER;
  self->depthStencil.depthBoundsTestEnable = VK_FALSE;
  self->depthStencil.stencilTestEnable     = VK_FALSE;
  self->depthStencil.front                 = (VkStencilOpState) {};
  self->depthStencil.back                  = (VkStencilOpState) {};
  self->depthStencil.minDepthBounds        = 0;
  self->depthStencil.maxDepthBounds        = 1;
}



void PipelineBuilder_enableDepthTest(PipelineBuilder* self, bool depthWriteEnable, VkCompareOp opCmp) {
  self->depthStencil.depthTestEnable       = VK_TRUE;
  self->depthStencil.depthWriteEnable      = depthWriteEnable;
  self->depthStencil.depthCompareOp        = opCmp;
  self->depthStencil.depthBoundsTestEnable = VK_FALSE;
  self->depthStencil.stencilTestEnable     = VK_FALSE;
  self->depthStencil.front                 = (VkStencilOpState) {};
  self->depthStencil.back                  = (VkStencilOpState) {};
  self->depthStencil.minDepthBounds        = 0;
  self->depthStencil.maxDepthBounds        = 1;
}



VkPipeline PipelineBuilder_build(PipelineBuilder* self, VkDevice device) {
  VkPipelineViewportStateCreateInfo viewport = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};
  VkPipelineColorBlendStateCreateInfo  color_blending = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                         .logicOpEnable   = VK_FALSE,
                                                         .logicOp         = VK_LOGIC_OP_COPY,
                                                         .attachmentCount = 1,
                                                         .pAttachments    = &self->colorBlendAttachment};
  VkPipelineVertexInputStateCreateInfo vertex_input = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkDynamicState                   state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dyn     = {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .pDynamicStates = &state[0], .dynamicStateCount = 2};
  VkGraphicsPipelineCreateInfo pipeline = {.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                           .pNext               = &self->renderInfo,
                                           .stageCount          = ARR_LEN(self->shaderStages),
                                           .pStages             = self->shaderStages,
                                           .pVertexInputState   = &vertex_input,
                                           .pInputAssemblyState = &self->inputAssembly,
                                           .pViewportState      = &viewport,
                                           .pRasterizationState = &self->rasterizer,
                                           .pMultisampleState   = &self->multisampling,
                                           .pColorBlendState    = &color_blending,
                                           .pDepthStencilState  = &self->depthStencil,
                                           .layout              = self->pipelineLayout,
                                           .pDynamicState       = &dyn};
  VkPipeline                   ret;
  auto                         ok = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &ret);
  VK_CHECK(ok);
  if (VK_SUCCESS != ok)
    ret = VK_NULL_HANDLE;
  return ret;
}
