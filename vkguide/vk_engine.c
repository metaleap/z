#include <threads.h>
#include <SDL.h>
#include <vulkan/vulkan_core.h>

#include "./vkguide.h"


#ifdef DEVBUILD
bool isDebug = true;
#else
bool isDebug = false;
#endif

VkInstance               vkInstance;
VkDebugUtilsMessengerEXT vkDebugMessenger;
VkPhysicalDevice         vkChosenGpu;
VkDevice                 vkDevice;
VkSurfaceKHR             vkSurface;


void init_vulkan() {
  VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_1};
  const char*       exts[]   = {"VK_KHR_surface", "VK_KHR_xcb_surface"};   //, "VK_KHR_maintenance1"};
  const char*       layers[] = {"VK_LAYER_KHRONOS_validation"};            // "VK_LAYER_KHRONOS_validation" MUST remain the last
                                                                           // entry in this array!

  VkInstanceCreateInfo create_info = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                      .enabledExtensionCount   = ARR_LEN(exts),
                                      .ppEnabledExtensionNames = exts,
                                      .enabledLayerCount       = ARR_LEN(layers) - (isDebug ? 0 : 1),
                                      .ppEnabledLayerNames     = layers,
                                      .pApplicationInfo        = &app_info};
  VK_CHECK(vkCreateInstance(&create_info, nullptr, &vkInstance));
}


void init_swapchain() {
}


void init_commands() {
}


void init_sync_structures() {
}


void vke_init() {
  SDL_Init(SDL_INIT_EVERYTHING);

  vke.window = SDL_CreateWindow("foo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, vke.window_extent.width, vke.window_extent.height,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
  if (vke.window == nullptr) {
    printf("%s\n", SDL_GetError());
    abort();
  }

  init_vulkan();
  init_swapchain();
  init_commands();
  init_sync_structures();
}


void vke_run() {
  SDL_Event evt;
  bool      quit = false;
  while (!quit) {
    if (vke.paused) {
      thrd_sleep(&(struct timespec) {.tv_sec = 1}, nullptr);
      continue;
    }

    while ((!quit) && (SDL_PollEvent(&evt) != 0)) {
      switch (evt.type) {
        case SDL_QUIT:
          quit = true;
          break;
        case SDL_WINDOWEVENT:
          switch (evt.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
              quit = true;
              break;
            case SDL_WINDOWEVENT_MINIMIZED:
              vke.paused = false;
              break;
            case SDL_WINDOWEVENT_RESTORED:
              vke.paused = true;
              break;
          }
          break;
      }
    }

    if (quit)
      break;
    vke_draw();
  }
}


void vke_draw() {
}

void vke_cleanup() {
  SDL_DestroyWindow(vke.window);
}
