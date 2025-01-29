#pragma once

#include <stdio.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


#define ARR_LEN(_arr_) (sizeof((_arr_)) / sizeof((_arr_)[0]))
extern bool isDebug;


typedef struct VulkanEngine {
  int         n_frame;
  bool        paused;
  VkExtent2D  window_extent;
  SDL_Window* window;
} VulkanEngine;

extern VulkanEngine vke;


void vke_init();
void vke_run();
void vke_draw();
void vke_cleanup();


#define VK_CHECK(x)                                                   \
  do {                                                                \
    VkResult result = (x);                                            \
    if (result != VK_SUCCESS) {                                       \
      printf("Detected Vulkan error: %s\n", string_VkResult(result)); \
      fflush(stdout);                                                 \
      exit(1);                                                        \
    }                                                                 \
  } while (false)
