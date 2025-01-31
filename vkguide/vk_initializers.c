#include "./vkguide.h"
#include <SDL_stdinc.h>
#include <vulkan/vulkan_core.h>



VkImageSubresourceRange vlkImageSubresourceRange(VkImageAspectFlags aspectMask) {
  return (VkImageSubresourceRange) {.aspectMask = aspectMask, .levelCount = VK_REMAINING_MIP_LEVELS, .layerCount = VK_REMAINING_ARRAY_LAYERS};
}


VkImageCreateInfo vlkImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
  return (VkImageCreateInfo) {.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                              .imageType   = VK_IMAGE_TYPE_2D,
                              .format      = format,
                              .extent      = extent,
                              .usage       = usageFlags,
                              .mipLevels   = 1,
                              .arrayLayers = 1,
                              .tiling      = VK_IMAGE_TILING_OPTIMAL,   // as long as no cpu readback
                              .samples     = VK_SAMPLE_COUNT_1_BIT};
}


VkImageViewCreateInfo vlkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
  return (VkImageViewCreateInfo) {
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .viewType         = VK_IMAGE_VIEW_TYPE_2D,
      .image            = image,
      .format           = format,
      .subresourceRange = {.aspectMask = aspectFlags, .levelCount = 1, .layerCount = 1}
  };
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
