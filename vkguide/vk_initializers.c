#include "./vkguide.h"



VkImageSubresourceRange vlkImgSubresourceRange(VkImageAspectFlags aspectMask) {
  return (VkImageSubresourceRange) {.aspectMask = aspectMask, .levelCount = VK_REMAINING_MIP_LEVELS, .layerCount = VK_REMAINING_ARRAY_LAYERS};
}


VkCommandBufferBeginInfo vlkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
  return (VkCommandBufferBeginInfo) {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = flags};
}


VkFenceCreateInfo vlkFenceCreateInfo(VkFenceCreateFlags flags) {
  return (VkFenceCreateInfo) {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = flags};
}


VkSemaphoreCreateInfo vlkSemaphoreCreateInfo(VkSemaphoreCreateFlags flags) {
  return (VkSemaphoreCreateInfo) {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .flags = flags};
}


VkCommandPoolCreateInfo vlkCommandPoolCreateInfo(Uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags) {
  return (VkCommandPoolCreateInfo) {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags = flags, .queueFamilyIndex = queueFamilyIndex};
}


VkCommandBufferAllocateInfo vlkCommandBufferAllocateInfo(VkCommandPool cmdPool, Uint32 cmdBufCount) {
  return (VkCommandBufferAllocateInfo) {.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                        .commandPool        = cmdPool,
                                        .commandBufferCount = cmdBufCount,
                                        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY};
}


VkSemaphoreSubmitInfo vlkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
  return (VkSemaphoreSubmitInfo) {
      .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = semaphore,
      .stageMask = stageMask,
      .value     = 1,
  };
}


VkCommandBufferSubmitInfo vlkCommandBufferSubmitInfo(VkCommandBuffer cmdBuf) {
  return (VkCommandBufferSubmitInfo) {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = cmdBuf};
}


VkSubmitInfo2 vlkSubmitInfo(VkCommandBufferSubmitInfo* cmdBufSubmitInfo, VkSemaphoreSubmitInfo* sigSemaInfo,
                            VkSemaphoreSubmitInfo* waitSemaInfo) {
  return (VkSubmitInfo2) {
      .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .waitSemaphoreInfoCount   = ((waitSemaInfo == nullptr) ? 0 : 1),
      .pWaitSemaphoreInfos      = waitSemaInfo,
      .signalSemaphoreInfoCount = ((sigSemaInfo == nullptr) ? 0 : 1),
      .pSignalSemaphoreInfos    = sigSemaInfo,
      .commandBufferInfoCount   = 1,
      .pCommandBufferInfos      = cmdBufSubmitInfo,
  };
}
