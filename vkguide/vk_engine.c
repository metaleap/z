#include "./vkguide.h"
#include <SDL_error.h>
#include <SDL_log.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan_core.h>


#ifdef DEVBUILD
bool isDebug = true;
#else
bool isDebug = false;
#endif

VkInstance       vlkInstance        = nullptr;
VkPhysicalDevice vlkGpu             = nullptr;
VkDevice         vlkDevice          = nullptr;
VkSurfaceKHR     vlkSurface         = nullptr;
VkSwapchainKHR   vlkSwapchain       = nullptr;
VkImage*         vlkSwapchainImages = nullptr;



void vlkInit() {
  Uint32 num_exts;
  SDL_Vulkan_GetInstanceExtensions(vke.window, &num_exts, nullptr);
  const char* inst_exts[num_exts];
  SDL_Vulkan_GetInstanceExtensions(vke.window, &num_exts, inst_exts);

  VkApplicationInfo    inst_app      = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3};
  const char*          inst_layers[] = {"VK_LAYER_KHRONOS_validation"};   // "VK_LAYER_KHRONOS_validation" MUST remain the last entry!
  VkInstanceCreateInfo inst_create   = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                        .enabledExtensionCount   = num_exts,
                                        .ppEnabledExtensionNames = inst_exts,
                                        .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
                                        .ppEnabledLayerNames     = inst_layers,
                                        .pApplicationInfo        = &inst_app};
  VK_CHECK(vkCreateInstance(&inst_create, nullptr, &vlkInstance));

  if (!SDL_Vulkan_CreateSurface(vke.window, vlkInstance, &vlkSurface)) {
    SDL_Log("%s\n", SDL_GetError());
    exit(1);
  }

  Uint32 num_gpus;
  vkEnumeratePhysicalDevices(vlkInstance, &num_gpus, nullptr);
  assert((num_gpus > 0) && "vkEnumeratePhysicalDevices");
  VkPhysicalDevice gpus[num_gpus];
  vkEnumeratePhysicalDevices(vlkInstance, &num_gpus, gpus);
  for (Uint32 i = 0; i < num_gpus; i++) {
    VkPhysicalDeviceProperties gpu_props = {};
    vkGetPhysicalDeviceProperties(gpus[i], &gpu_props);
    SDL_Log("GPU>>%d %d %s<<\n", gpu_props.deviceID, gpu_props.deviceType, gpu_props.deviceName);
    vlkGpu = gpus[i];
    break;
  }

  const char*             device_exts[] = {"VK_KHR_swapchain", "VK_KHR_maintenance1"};
  VkDeviceQueueCreateInfo queue_create
      = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueCount = 1, .queueFamilyIndex = 0, .pQueuePriorities = &(float) {1.0f}};
  VkDeviceCreateInfo device_create = {.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                      .queueCreateInfoCount    = 1,
                                      .pQueueCreateInfos       = &queue_create,
                                      .enabledExtensionCount   = 1,
                                      .ppEnabledExtensionNames = device_exts,
                                      .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
                                      .ppEnabledLayerNames     = inst_layers};
  VK_CHECK(vkCreateDevice(vlkGpu, &device_create, nullptr, &vlkDevice));
}



void vlkRecreateSwapchain() {
  if (vlkSwapchainImages != nullptr) {
    for (Uint32 i = 0; vlkSwapchainImages[i] != nullptr; i++)
      vkDestroyImage(vlkDevice, vlkSwapchainImages[i], nullptr);
    vlkSwapchainImages = nullptr;
  }

  VkSurfaceCapabilitiesKHR surface_caps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vlkGpu, vlkSurface, &surface_caps));
  VkSwapchainCreateInfoKHR create = {
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface          = vlkSurface,
      .minImageCount    = surface_caps.minImageCount + ((surface_caps.maxImageCount > surface_caps.minImageCount) ? 1 : 0),
      .imageExtent      = surface_caps.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageFormat      = VK_FORMAT_B8G8R8A8_SRGB,
      .imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
      .preTransform     = surface_caps.currentTransform,
      .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .clipped          = VK_TRUE,
  };
  VK_CHECK(vkCreateSwapchainKHR(vlkDevice, &create, nullptr, &vlkSwapchain));
  Uint32 num_images = 0;
  vkGetSwapchainImagesKHR(vlkDevice, vlkSwapchain, &num_images, nullptr);
  assert(num_images > 0);
  vlkSwapchainImages = calloc((1 + num_images), sizeof(VkImage));   // ensure trailing nullptr
  vkGetSwapchainImagesKHR(vlkDevice, vlkSwapchain, &num_images, vlkSwapchainImages);
}



void vlkInitCommands() {
}



void vlkInitSyncStructures() {
}



void vkeDispose() {
  SDL_DestroyWindow(vke.window);
  vkDestroySwapchainKHR(vlkDevice, vlkSwapchain, nullptr);
  if (vlkSwapchainImages != nullptr) {
    for (Uint32 i = 0; vlkSwapchainImages[i] != nullptr; i++)
      vkDestroyImage(vlkDevice, vlkSwapchainImages[i], nullptr);
    vlkSwapchainImages = nullptr;
  }
  vkDestroyDevice(vlkDevice, nullptr);
  vkDestroySurfaceKHR(vlkInstance, vlkSurface, nullptr);
  vkDestroyInstance(vlkInstance, nullptr);
}


void vkeInit() {
  SDL_Init(SDL_INIT_EVERYTHING);
  vke.window = SDL_CreateWindow("foo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600, 900,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
  if (vke.window == nullptr) {
    printf("%s\n", SDL_GetError());
    exit(1);
  }
  vlkInit();
  vlkRecreateSwapchain();
  vlkInitCommands();
  vlkInitSyncStructures();
}


void vkeRun() {
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
    vkeDraw();
  }
  vkDeviceWaitIdle(vlkDevice);
}


void vkeDraw() {
}
