#include "./vkguide.h"
#include <SDL_stdinc.h>
#include <math.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>


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
  VkApplicationInfo    inst_app      = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_1};
  const char*          inst_exts[]   = {"VK_KHR_surface", "VK_KHR_xcb_surface"};   //, "VK_KHR_maintenance1"};
  const char*          inst_layers[] = {"VK_LAYER_KHRONOS_validation"};            // "VK_LAYER_KHRONOS_validation" MUST remain the last
                                                                                   // entry in this array!
  VkInstanceCreateInfo inst_create   = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                        .enabledExtensionCount   = ARR_LEN(inst_exts),
                                        .ppEnabledExtensionNames = inst_exts,
                                        .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
                                        .ppEnabledLayerNames     = inst_layers,
                                        .pApplicationInfo        = &inst_app};
  VK_CHECK(vkCreateInstance(&inst_create, nullptr, &vkInstance));

  // Uint32 num_gpus;
  // vkEnumeratePhysicalDevices(vkInstance, &num_gpus, nullptr);
  // assert((num_gpus > 0) && "No Vulkan-compatible hardware found.");
  // VkPhysicalDevice gpus[num_gpus];
  // vkEnumeratePhysicalDevices(vkInstance, &num_gpus, gpus);
  // for (Uint32 i = 0; i < num_gpus; i++) {
  //   VkPhysicalDeviceProperties gpu_props = {};
  //   vkGetPhysicalDeviceProperties(gpus[i], &gpu_props);
  //   printf("GPU>>%d %d %s<<\n", gpu_props.deviceID, gpu_props.deviceType, gpu_props.deviceName);
  //   vkChosenGpu = gpus[i];
  //   break;
  // }

  // const char* device_exts[] = {"VK_KHR_swapchain"};   //, "VK_KHR_maintenance1"};

  // VkDeviceQueueCreateInfo queue_create
  //     = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueCount = 1, .queueFamilyIndex = 0, .pQueuePriorities = &(float) {1.0f}};
  // VkDeviceCreateInfo device_create = {.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
  //                                     .queueCreateInfoCount    = 1,
  //                                     .pQueueCreateInfos       = &queue_create,
  //                                     .enabledExtensionCount   = 1,
  //                                     .ppEnabledExtensionNames = device_exts,
  //                                     .enabledLayerCount       = 0,   // ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
  //                                     .ppEnabledLayerNames     = inst_layers};
  // VK_CHECK(vkCreateDevice(vkChosenGpu, &device_create, nullptr, &vkDevice));
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
