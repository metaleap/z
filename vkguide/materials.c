#include "./vkguide.h"
#include <vulkan/vulkan_core.h>


extern VulkanEngine vke;
extern VkDevice     vlkDevice;



void MatGltfMetallicRoughness_buildPipelines(MatGltfMetallicRoughness* this) {
  VkShaderModule shader_frag;
  VK_CHECK(vlkLoadShaderModule("../../vkguide/shaders/mesh.frag", vlkDevice, &shader_frag, VK_SHADER_STAGE_FRAGMENT_BIT));
  VkShaderModule shader_vert;
  VK_CHECK(vlkLoadShaderModule("../../vkguide/shaders/mesh.vert", vlkDevice, &shader_vert, VK_SHADER_STAGE_VERTEX_BIT));

  VlkDescriptorLayoutBuilder builder_layout = {};
  VlkDescriptorLayoutBuilder_addBinding(&builder_layout, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  VlkDescriptorLayoutBuilder_addBinding(&builder_layout, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  VlkDescriptorLayoutBuilder_addBinding(&builder_layout, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  this->materialLayout = VlkDescriptorLayoutBuilder_build(
      &builder_layout, vlkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr, 0);
  VkDescriptorSetLayout layouts[] = {vke.sceneDataDescriptorLayout, this->materialLayout};

  VkPushConstantRange matrix_range = {.size = sizeof(GpuDrawPushConstants), .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};
  VkPipelineLayoutCreateInfo mesh_layout_info = {.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                 .setLayoutCount         = ARR_LEN(layouts),
                                                 .pSetLayouts            = layouts,
                                                 .pushConstantRangeCount = 1,
                                                 .pPushConstantRanges    = &matrix_range};
  VkPipelineLayout           new_layout;
  VK_CHECK(vkCreatePipelineLayout(vlkDevice, &mesh_layout_info, nullptr, &new_layout));
  this->opaquePipeline.layout      = new_layout;
  this->transparentPipeline.layout = new_layout;

  // build the stage-create-info for both vertex and fragment stages.
  // This lets the pipeline know the shader modules per stage
  PipelineBuilder pipeline_builder = {};
  PipelineBuilder_setShaders(&pipeline_builder, shader_vert, shader_frag);
  PipelineBuilder_setInputTopology(&pipeline_builder, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  PipelineBuilder_setPolygonMode(&pipeline_builder, VK_POLYGON_MODE_FILL);
  PipelineBuilder_setCullMode(&pipeline_builder, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  PipelineBuilder_setMultisamplingNone(&pipeline_builder);
  PipelineBuilder_disableBlending(&pipeline_builder);
  PipelineBuilder_enableDepthTest(&pipeline_builder, true, VK_COMPARE_OP_GREATER_OR_EQUAL);
  PipelineBuilder_setColorAttachmentFormat(&pipeline_builder, vke.drawImage.format);
  PipelineBuilder_setDepthFormat(&pipeline_builder, vke.depthImage.format);
  pipeline_builder.pipelineLayout = new_layout;
  this->opaquePipeline.pipeline   = PipelineBuilder_build(&pipeline_builder, vlkDevice);
  PipelineBuilder_enableBlendingAdditive(&pipeline_builder);
  PipelineBuilder_enableDepthTest(&pipeline_builder, false, VK_COMPARE_OP_GREATER_OR_EQUAL);
  this->transparentPipeline.pipeline = PipelineBuilder_build(&pipeline_builder, vlkDevice);
  vkDestroyShaderModule(vlkDevice, shader_vert, nullptr);
  vkDestroyShaderModule(vlkDevice, shader_frag, nullptr);
}



MaterialInstance MatGltfMetallicRoughness_writeMaterial(MatGltfMetallicRoughness* this, MaterialPass pass,
                                                        MatGltfMetallicRoughnessMaterialResources* resources,
                                                        VlkDescriptorAllocatorGrowable*            descriptorAlloc) {
  MaterialInstance mat_data = {
      .passType    = pass,
      .pipeline    = ((pass == MaterialPass_Transparent) ? &this->transparentPipeline : &this->opaquePipeline),
      .materialSet = VlkDescriptorAllocatorGrowable_allocate(descriptorAlloc, vlkDevice, this->materialLayout, nullptr)};
  VlkDescriptorWriter_clear(&this->descriptorWriter);
  VlkDescriptorWriter_writeBuffer(&this->descriptorWriter, 0, resources->dataBuffer,
                                  sizeof(MatGltfMetallicRoughnessMaterialConstants), resources->dataBufferOffset,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  VlkDescriptorWriter_writeImage(&this->descriptorWriter, 1, resources->colorImage.defaultView, resources->colorSampler,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  VlkDescriptorWriter_writeImage(&this->descriptorWriter, 2, resources->metalRoughImage.defaultView,
                                 resources->metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  VlkDescriptorWriter_updateSet(&this->descriptorWriter, vlkDevice, mat_data.materialSet);
  return mat_data;
}
