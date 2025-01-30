#include "./vkguide.h"
#include <vulkan/vulkan_core.h>



void vlkImgTransition(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
  VkImageSubresourceRange srr = vlkImageSubresourceRange((newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                                                                                 : VK_IMAGE_ASPECT_COLOR_BIT);
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



void vlkImgCopy(VkCommandBuffer cmdBuf, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize) {
  VkImageBlit2 blit_region = {
      .sType          = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
      .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1}
  };
  blit_region.srcOffsets[1] = (VkOffset3D) {.x = srcSize.width, .y = srcSize.height, .z = 1};
  blit_region.dstOffsets[1] = (VkOffset3D) {.x = dstSize.width, .y = dstSize.height, .z = 1};

  vkCmdBlitImage2(cmdBuf, &(VkBlitImageInfo2) {.sType          = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                                               .dstImage       = dst,
                                               .srcImage       = src,
                                               .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                               .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                               .filter         = VK_FILTER_LINEAR,
                                               .regionCount    = 1,
                                               .pRegions       = &blit_region});
}
