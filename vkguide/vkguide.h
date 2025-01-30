#pragma once
#include <vulkan/vulkan_core.h>
#define VMA_DEDICATED_ALLOCATION 1

#include <assert.h>
#include <threads.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "../3rdparty/GPUOpen-LibrariesAndSDKs___VulkanMemoryAllocator/include/vk_mem_alloc.h"



VkImageSubresourceRange     vlkImageSubresourceRange(VkImageAspectFlags aspectMask);
VkCommandBufferBeginInfo    vlkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
VkFenceCreateInfo           vlkFenceCreateInfo(VkFenceCreateFlags flags);
VkSemaphoreCreateInfo       vlkSemaphoreCreateInfo(VkSemaphoreCreateFlags flags);
VkCommandPoolCreateInfo     vlkCommandPoolCreateInfo(Uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
VkCommandBufferAllocateInfo vlkCommandBufferAllocateInfo(VkCommandPool cmdPool, Uint32 cmdBufCount);
VkSemaphoreSubmitInfo       vlkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
VkCommandBufferSubmitInfo   vlkCommandBufferSubmitInfo(VkCommandBuffer cmdBuf);
VkSubmitInfo2               vlkSubmitInfo(VkCommandBufferSubmitInfo* cmdBuf, VkSemaphoreSubmitInfo* sig, VkSemaphoreSubmitInfo* wait);
VkImageCreateInfo           vlkImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
VkImageViewCreateInfo       vlkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
void                        vlkImgTransition(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
void                        vlkImgCopy(VkCommandBuffer cmdBuf, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);



#define ARR_LEN(_arr_) (sizeof((_arr_)) / sizeof((_arr_)[0]))
#define FRAME_OVERLAP  3


typedef struct DisposalQueue {
#define DISP_QUEUE_CAPACITY 11
  int             count;
  VkStructureType types[DISP_QUEUE_CAPACITY];
  void*           args[DISP_QUEUE_CAPACITY];
  VmaAllocation   allocs[DISP_QUEUE_CAPACITY];
} DisposalQueue;
void disposals_push(DisposalQueue* self, VkStructureType type, void* arg, VmaAllocation alloc);
void disposals_flush(DisposalQueue* self);


typedef struct FrameData {
  VkCommandPool   commandPool;
  VkCommandBuffer mainCommandBuffer;
  VkFence         fenceRender;
  VkSemaphore     semaPresent, semaRender;
  DisposalQueue   disposals;
} FrameData;


typedef struct VlkImage {
  VkImage       image;
  VkImageView   defaultView;
  VkExtent3D    extent;
  VkFormat      format;
  VmaAllocation alloc;
} VlkImage;


typedef struct VulkanEngine {
  VmaAllocator  alloc;
  size_t        frameNr;
  FrameData     frames[FRAME_OVERLAP];
  bool          paused;
  SDL_Window*   window;
  DisposalQueue disposals;
  VlkImage      drawImage;
  VkExtent2D    drawExtent;
} VulkanEngine;


extern bool         isDebug;
extern VulkanEngine vke;


void vkeInit();
void vkeRun();
void vkeDraw();
void vkeShutdown();



#define VK_CHECK(x)                                           \
  do {                                                        \
    VkResult result = (x);                                    \
    if (result != VK_SUCCESS) {                               \
      SDL_Log("Vulkan error: %s\n", string_VkResult(result)); \
      exit(1);                                                \
    }                                                         \
  } while (false)
