#pragma once

#include <SDL_stdinc.h>
#define VMA_DEDICATED_ALLOCATION 1

#include <assert.h>
#include <threads.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "../3rdparty/GPUOpen-LibrariesAndSDKs___VulkanMemoryAllocator/include/vk_mem_alloc.h"



void                        vlkImgTransition(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
VkImageSubresourceRange     vlkImgSubresourceRange(VkImageAspectFlags aspectMask);
VkCommandBufferBeginInfo    vlkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
VkFenceCreateInfo           vlkFenceCreateInfo(VkFenceCreateFlags flags);
VkSemaphoreCreateInfo       vlkSemaphoreCreateInfo(VkSemaphoreCreateFlags flags);
VkCommandPoolCreateInfo     vlkCommandPoolCreateInfo(Uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
VkCommandBufferAllocateInfo vlkCommandBufferAllocateInfo(VkCommandPool cmdPool, Uint32 cmdBufCount);
VkSemaphoreSubmitInfo       vlkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
VkCommandBufferSubmitInfo   vlkCommandBufferSubmitInfo(VkCommandBuffer cmdBuf);
VkSubmitInfo2               vlkSubmitInfo(VkCommandBufferSubmitInfo* cmdBuf, VkSemaphoreSubmitInfo* sig, VkSemaphoreSubmitInfo* wait);



#define ARR_LEN(_arr_) (sizeof((_arr_)) / sizeof((_arr_)[0]))
#define FRAME_OVERLAP  3


typedef void FnDispose(void*);
typedef struct DisposalQueue {
#define DISP_QUEUE_CAPACITY 1
  Uint64     count;
  FnDispose* funcs[DISP_QUEUE_CAPACITY];
  void*      args[DISP_QUEUE_CAPACITY];
} DisposalQueue;
void vke_del_push(DisposalQueue* queue, FnDispose fn, void* any);
void vke_del_flush(DisposalQueue* queue);


typedef struct FrameData {
  VkCommandPool   commandPool;
  VkCommandBuffer mainCommandBuffer;
  VkFence         fenceRender;
  VkSemaphore     semaPresent, semaRender;
  DisposalQueue   delQueue;
} FrameData;


typedef struct VulkanEngine {
  VmaAllocator  alloc;
  Uint64        frameNr;
  FrameData     frames[FRAME_OVERLAP];
  bool          paused;
  SDL_Window*   window;
  DisposalQueue delQueue;
} VulkanEngine;


extern bool         isDebug;
extern VulkanEngine vke;


void vkeInit();
void vkeRun();
void vkeDraw();
void vkeDispose();



#define VK_CHECK(x)                                           \
  do {                                                        \
    VkResult result = (x);                                    \
    if (result != VK_SUCCESS) {                               \
      SDL_Log("Vulkan error: %s\n", string_VkResult(result)); \
      exit(1);                                                \
    }                                                         \
  } while (false)
