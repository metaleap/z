#include "./vkg.h"


LIST_DEFINE_C(VlkDescriptorAllocatorSizeRatios, VlkDescriptorAllocatorSizeRatios, VlkDescriptorAllocatorSizeRatio);
LIST_DEFINE_C(VkDescriptorPoolSizes, VkDescriptorPoolSizes, VkDescriptorPoolSize);
LIST_DEFINE_C(VkDescriptorPools, VkDescriptorPools, VkDescriptorPool);
LIST_DEFINE_C(VkDescriptorImageInfos, VkDescriptorImageInfos, VkDescriptorImageInfo);
LIST_DEFINE_C(VkDescriptorBufferInfos, VkDescriptorBufferInfos, VkDescriptorBufferInfo);
LIST_DEFINE_C(VkWriteDescriptorSets, VkWriteDescriptorSets, VkWriteDescriptorSet);



void VlkDescriptorLayoutBuilder_clear(VlkDescriptorLayoutBuilder* this) {
  this->count = 0;
}



void VlkDescriptorLayoutBuilder_addBinding(VlkDescriptorLayoutBuilder* this, Uint32 binding, VkDescriptorType type) {
  assert((this->count < VDLB_CAP) && "VlkDescriptorLayoutBuilder_addBinding");
  this->bindings[this->count] =
      (VkDescriptorSetLayoutBinding) {.binding = binding, .descriptorCount = 1, .descriptorType = type};
  this->count++;
}



VkDescriptorSetLayout VlkDescriptorLayoutBuilder_build(VlkDescriptorLayoutBuilder* this, VkDevice device,
                                                       VkShaderStageFlags shaderStages, void* pNext,
                                                       VkDescriptorSetLayoutCreateFlags flags) {
  for (Uint8 i = 0; i < VDLB_CAP; i++)
    this->bindings[i].stageFlags |= shaderStages;
  VkDescriptorSetLayoutCreateInfo create = {.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                            .pBindings    = this->bindings,
                                            .bindingCount = this->count,
                                            .flags        = flags,
                                            .pNext        = pNext};
  VkDescriptorSetLayout           ret;
  VK_CHECK(vkCreateDescriptorSetLayout(device, &create, nullptr, &ret));
  return ret;
}



VkDescriptorPool VlkDescriptorAllocatorGrowable_createPool(VkDevice device, Uint32 numSets,
                                                           VlkDescriptorAllocatorSizeRatios poolRatios) {
  VkDescriptorPoolSizes pool_sizes = {};
  assert(VkDescriptorPoolSizes_init_capacity(&pool_sizes, poolRatios.count));
  for (size_t i = 0; i < poolRatios.count; i++)
    assert(VkDescriptorPoolSizes_add(
        &pool_sizes, (VkDescriptorPoolSize) {.type            = poolRatios.buffer[i].type,
                                             .descriptorCount = poolRatios.buffer[i].ratio * (float) numSets}));
  VkDescriptorPoolCreateInfo pool_create = {.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                            .maxSets       = numSets,
                                            .poolSizeCount = pool_sizes.count,
                                            .pPoolSizes    = pool_sizes.buffer};
  VkDescriptorPool           ret         = {};
  VK_CHECK(vkCreateDescriptorPool(device, &pool_create, nullptr, &ret));
  return ret;
}



VkDescriptorPool VlkDescriptorAllocatorGrowable_getOrCreateReadyPool(VlkDescriptorAllocatorGrowable* this,
                                                                     VkDevice device) {
  VkDescriptorPool ret = {};
  if (this->readyPools.count > 0) {
    ret = this->readyPools.buffer[this->readyPools.count - 1];
    this->readyPools.count--;
  } else {
    ret               = VlkDescriptorAllocatorGrowable_createPool(device, this->setsPerPool, this->ratios);
    this->setsPerPool = (1.5 * (float) this->setsPerPool);
    static constexpr Uint32 max_sets_per_pool = 4092;
    if (this->setsPerPool > max_sets_per_pool)
      this->setsPerPool = max_sets_per_pool;
  }
  return ret;
}



void VlkDescriptorAllocatorGrowable_init(VlkDescriptorAllocatorGrowable* this, VkDevice device, Uint32 maxSets,
                                         VlkDescriptorAllocatorSizeRatios poolRatios) {
  *this = (VlkDescriptorAllocatorGrowable) {
      .ratios = poolRatios, .setsPerPool = (1.5 * (float) maxSets), .fullPools = {}, .readyPools = {}};
  VkDescriptorPool new_pool = VlkDescriptorAllocatorGrowable_createPool(device, maxSets, poolRatios);
  assert(VkDescriptorPools_add(&this->readyPools, new_pool));
}



void VlkDescriptorAllocatorGrowable_clearPools(VlkDescriptorAllocatorGrowable* this, VkDevice device) {
  for (size_t i = 0; i < this->readyPools.count; i++)
    vkResetDescriptorPool(device, this->readyPools.buffer[i], 0);
  for (size_t i = 0; i < this->fullPools.count; i++) {
    vkResetDescriptorPool(device, this->fullPools.buffer[i], 0);
    assert(VkDescriptorPools_add(&this->readyPools, this->fullPools.buffer[i]));
  }
  VkDescriptorPools_clear(&this->fullPools);
}



void VlkDescriptorAllocatorGrowable_destroyPools(VlkDescriptorAllocatorGrowable* this, VkDevice device) {
  for (size_t i = 0; i < this->readyPools.count; i++)
    vkDestroyDescriptorPool(device, this->readyPools.buffer[i], nullptr);
  for (size_t i = 0; i < this->fullPools.count; i++)
    vkDestroyDescriptorPool(device, this->fullPools.buffer[i], nullptr);
  VkDescriptorPools_free_resources(&this->readyPools);
  VkDescriptorPools_free_resources(&this->fullPools);
}



VkDescriptorSet VlkDescriptorAllocatorGrowable_allocate(VlkDescriptorAllocatorGrowable* this, VkDevice device,
                                                        VkDescriptorSetLayout layout, void* pNext) {
  VkDescriptorPool            pool_to_use = VlkDescriptorAllocatorGrowable_getOrCreateReadyPool(this, device);
  VkDescriptorSetAllocateInfo alloc       = {.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                             .pNext              = pNext,
                                             .descriptorPool     = pool_to_use,
                                             .descriptorSetCount = 1,
                                             .pSetLayouts        = &layout};

  VkDescriptorSet ret;
  VkResult        err = vkAllocateDescriptorSets(device, &alloc, &ret);
  if ((err == VK_ERROR_OUT_OF_POOL_MEMORY) || (err == VK_ERROR_FRAGMENTED_POOL)) {
    assert(VkDescriptorPools_add(&this->fullPools, pool_to_use));
    pool_to_use          = VlkDescriptorAllocatorGrowable_getOrCreateReadyPool(this, device);
    alloc.descriptorPool = pool_to_use;
    VK_CHECK(vkAllocateDescriptorSets(device, &alloc, &ret));
  }
  assert(VkDescriptorPools_add(&this->readyPools, pool_to_use));
  return ret;
}



void VlkDescriptorWriter_writeBuffer(VlkDescriptorWriter* this, int binding, VkBuffer buffer, size_t size, size_t offset,
                                     VkDescriptorType type) {
  assert(VkDescriptorBufferInfos_add(&this->bufferInfos, (VkDescriptorBufferInfo) {
                                                             .buffer = buffer,
                                                             .offset = offset,
                                                             .range  = size,
                                                         }));
  assert(VkWriteDescriptorSets_add(
      &this->writes,
      (VkWriteDescriptorSet) {.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                              .dstBinding      = binding,
                              .pBufferInfo     = &this->bufferInfos.buffer[this->bufferInfos.count - 1],   // added above
                              .descriptorCount = 1,
                              .descriptorType  = type}));
}





void VlkDescriptorWriter_writeImage(VlkDescriptorWriter* this, int binding, VkImageView image, VkSampler sampler,
                                    VkImageLayout layout, VkDescriptorType type) {
  assert(VkDescriptorImageInfos_add(&this->imageInfos, (VkDescriptorImageInfo) {
                                                           .sampler     = sampler,
                                                           .imageView   = image,
                                                           .imageLayout = layout,
                                                       }));
  assert(VkWriteDescriptorSets_add(
      &this->writes,
      (VkWriteDescriptorSet) {.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                              .dstBinding      = binding,
                              .pImageInfo      = &this->imageInfos.buffer[this->imageInfos.count - 1],   // added above
                              .descriptorCount = 1,
                              .descriptorType  = type}));
}



void VlkDescriptorWriter_clear(VlkDescriptorWriter* this) {
  VkDescriptorBufferInfos_clear(&this->bufferInfos);
  VkDescriptorImageInfos_clear(&this->imageInfos);
  VkWriteDescriptorSets_clear(&this->writes);
}



void VlkDescriptorWriter_free(VlkDescriptorWriter* this) {
  VkDescriptorBufferInfos_free_resources(&this->bufferInfos);
  VkDescriptorImageInfos_free_resources(&this->imageInfos);
  VkWriteDescriptorSets_free_resources(&this->writes);
}



void VlkDescriptorWriter_updateSet(VlkDescriptorWriter* this, VkDevice device, VkDescriptorSet set) {
  for (size_t i = 0; i < this->writes.count; i++)
    this->writes.buffer[i].dstSet = set;
  vkUpdateDescriptorSets(device, this->writes.count, this->writes.buffer, 0, nullptr);
}
