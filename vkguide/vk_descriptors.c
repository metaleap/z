#include "./vkguide.h"
#include <vulkan/vulkan_core.h>


void VlkDescriptorLayoutBuilder_clear(VlkDescriptorLayoutBuilder* self) {
  self->count = 0;
}


void VlkDescriptorLayoutBuilder_addBinding(VlkDescriptorLayoutBuilder* self, Uint32 binding,
                                           VkDescriptorType type) {
  assert(self->count < VDLB_CAP && "VlkDescriptorLayoutBuilder_addBinding");
  self->bindings[self->count] =
      (VkDescriptorSetLayoutBinding) {.binding = binding, .descriptorCount = 1, .descriptorType = type};
  self->count++;
}


VkDescriptorSetLayout VlkDescriptorLayoutBuilder_build(VlkDescriptorLayoutBuilder* self, VkDevice device,
                                                       VkShaderStageFlags shaderStages, void* pNext,
                                                       VkDescriptorSetLayoutCreateFlags flags) {
  for (Uint8 i = 0; i < VDLB_CAP; i++)
    self->bindings[i].stageFlags |= shaderStages;
  VkDescriptorSetLayoutCreateInfo create = {.sType     = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                            .pBindings = self->bindings,
                                            .bindingCount = self->count,
                                            .flags        = flags,
                                            .pNext        = pNext};
  VkDescriptorSetLayout           ret;
  VK_CHECK(vkCreateDescriptorSetLayout(device, &create, nullptr, &ret));
  return ret;
}


void VlkDescriptorAllocator_clearDescriptors(VlkDescriptorAllocator* self, VkDevice device) {
  VK_CHECK(vkResetDescriptorPool(device, self->pool, 0));
}


void VlkDescriptorAllocator_destroyPool(VlkDescriptorAllocator* self, VkDevice device) {
  vkDestroyDescriptorPool(device, self->pool, nullptr);
}


void VlkDescriptorAllocator_initPool(VlkDescriptorAllocator* self, VkDevice device, Uint32 maxSets,
                                     Uint8 ratiosCount, VlkDescriptorAllocatorSizeRatio ratios[]) {
  VkDescriptorPoolSize pool_sizes[ratiosCount];
  for (Uint8 i = 0; i < ratiosCount; i++)
    pool_sizes[i] = (VkDescriptorPoolSize) {.type            = ratios[i].type,
                                            .descriptorCount = (((float) maxSets) * ratios[i].ratio)};
  VkDescriptorPoolCreateInfo pool_create = {.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                            .pPoolSizes    = pool_sizes,
                                            .maxSets       = maxSets,
                                            .poolSizeCount = ratiosCount};
  VK_CHECK(vkCreateDescriptorPool(device, &pool_create, nullptr, &self->pool));
}


VkDescriptorSet VlkDescriptorAllocator_allocate(VlkDescriptorAllocator* self, VkDevice device,
                                                VkDescriptorSetLayout layout) {
  VkDescriptorSetAllocateInfo alloc = {.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                       .descriptorPool     = self->pool,
                                       .descriptorSetCount = 1,
                                       .pSetLayouts        = &layout};
  VkDescriptorSet             ret;
  VK_CHECK(vkAllocateDescriptorSets(device, &alloc, &ret));
  return ret;
}
