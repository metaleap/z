#include "./vkguide.h"
#include <vulkan/vulkan_core.h>


VkImageSubresourceRange vlkImgSubresourceRange(VkImageAspectFlags aspectMask) {
  return (VkImageSubresourceRange) {.aspectMask = aspectMask, .levelCount = VK_REMAINING_MIP_LEVELS, .layerCount = VK_REMAINING_ARRAY_LAYERS};
}


void vlkImgTransition(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
  VkImageSubresourceRange srr
      = vlkImgSubresourceRange((newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
  vkCmdPipelineBarrier2(
      cmdBuf, &(VkDependencyInfo) {
                  .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                  .imageMemoryBarrierCount = 1,
                  .pImageMemoryBarriers    = &(VkImageMemoryBarrier2) {.sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                                                       .srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                                                       .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                                       .dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                                                       .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
                                                                       .oldLayout     = currentLayout,
                                                                       .newLayout     = newLayout,
                                                                       .image         = image,
                                                                       .subresourceRange = srr}
  });
}
