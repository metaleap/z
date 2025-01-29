#pragma once

#include <assert.h>
#include <threads.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>


#define ARR_LEN(_arr_) (sizeof((_arr_)) / sizeof((_arr_)[0]))
#define FRAME_OVERLAP  2


extern bool isDebug;


typedef struct FrameData {
  VkCommandPool   commandPool;
  VkCommandBuffer mainCommandBuffer;
} FrameData;


typedef struct VulkanEngine {
  int         n_frame;
  FrameData   frames[FRAME_OVERLAP];
  bool        paused;
  SDL_Window* window;
  VkQueue     vlkQueue;
  Uint32      vlkQueueFamily;
} VulkanEngine;

extern VulkanEngine vke;


void vkeInit();
void vkeRun();
void vkeDraw();
void vkeDispose();


#define VK_CHECK(x)                                                    \
  do {                                                                 \
    VkResult result = (x);                                             \
    if (result != VK_SUCCESS) {                                        \
      SDL_Log("Detected Vulkan error: %s\n", string_VkResult(result)); \
      exit(1);                                                         \
    }                                                                  \
  } while (false)
