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


typedef struct FrameData {
  VkCommandPool   commandPool;
  VkCommandBuffer mainCommandBuffer;
  VkFence         fenceRender;
  VkSemaphore     semaPresent, semaRender;
} FrameData;


#define DEL_QUEUE_CAPACITY 1
typedef void deleter(void*);
typedef struct DelQueue {
  deleter* funcs[DEL_QUEUE_CAPACITY];
  void*    args[DEL_QUEUE_CAPACITY];
  Uint32   count;
} DelQueue;
void vke_del_push(DelQueue* queue, deleter fn, void* any);
void vke_del_flush(DelQueue* queue);


typedef struct VulkanEngine {
  VmaAllocator alloc;
  int          frameNr;
  FrameData    frames[FRAME_OVERLAP];
  bool         paused;
  SDL_Window*  window;
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
