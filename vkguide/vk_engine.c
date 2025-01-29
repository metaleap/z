#include "./vkguide.h"
#include <SDL_error.h>
#include <SDL_log.h>
#include <SDL_vulkan.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>


#ifdef DEVBUILD
bool isDebug = true;
#else
bool isDebug = false;
#endif


VkFormat         vlkSwapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
VkInstance       vlkInstance             = nullptr;
VkPhysicalDevice vlkGpu                  = nullptr;
VkDevice         vlkDevice               = nullptr;
VkSurfaceKHR     vlkSurface              = nullptr;
VkSwapchainKHR   vlkSwapchain            = nullptr;
VkImage*         vlkSwapchainImages      = nullptr;
VkImageView*     vlkSwapchainImageViews  = nullptr;
VkExtent2D       vlkSwapchainExtent      = {};



void vlkInit() {
  Uint32 num_exts;
  SDL_Vulkan_GetInstanceExtensions(vke.window, &num_exts, nullptr);
  const char* inst_exts[num_exts];
  SDL_Vulkan_GetInstanceExtensions(vke.window, &num_exts, inst_exts);

  VkApplicationInfo    inst_app      = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_4};
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

  const char*             device_exts[] = {"VK_KHR_swapchain",           "VK_KHR_maintenance1",          "VK_KHR_synchronization2",
                                           "VK_EXT_descriptor_indexing", "VK_KHR_buffer_device_address", "VK_KHR_dynamic_rendering"};
  VkDeviceQueueCreateInfo queue_create
      = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueCount = 1, .queueFamilyIndex = 0, .pQueuePriorities = &(float) {1.0f}};
  VkPhysicalDeviceVulkan12Features features = {
      .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .bufferDeviceAddress = true,
      .descriptorIndexing  = true,
      .pNext               = &(VkPhysicalDeviceVulkan13Features) {
                                                                  .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                                                                  .dynamicRendering = true,
                                                                  .synchronization2 = true,
                                                                  }
  };
  VkDeviceCreateInfo device_create = {
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount    = 1,
      .pQueueCreateInfos       = &queue_create,
      .enabledExtensionCount   = ARR_LEN(device_exts),
      .ppEnabledExtensionNames = device_exts,
      .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
      .ppEnabledLayerNames     = inst_layers,
      .pNext                   = &(VkPhysicalDeviceFeatures2) {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features}
  };
  VK_CHECK(vkCreateDevice(vlkGpu, &device_create, nullptr, &vlkDevice));
}



void vlkDisposeSwapchain() {
  if (vlkSwapchainImageViews != nullptr) {
    for (Uint32 i = 0; vlkSwapchainImageViews[i] != nullptr; i++)
      vkDestroyImageView(vlkDevice, vlkSwapchainImageViews[i], nullptr);
    vlkSwapchainImageViews = nullptr;
  }
  if (vlkSwapchain != nullptr) {
    vkDestroySwapchainKHR(vlkDevice, vlkSwapchain, nullptr);
    vlkSwapchain = nullptr;
  }
  vlkSwapchainImages = nullptr;   // they were disposed by the above vkDestroySwapchainKHR call
}



void vlkRecreateSwapchain(VkExtent2D* windowSize) {
  vlkDisposeSwapchain();
  VkSurfaceCapabilitiesKHR surface_caps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vlkGpu, vlkSurface, &surface_caps));
  VkSwapchainCreateInfoKHR create_swapchain = {
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface          = vlkSurface,
      .minImageCount    = surface_caps.minImageCount + ((surface_caps.maxImageCount > surface_caps.minImageCount) ? 1 : 0),
      .imageExtent      = (windowSize == nullptr) ? surface_caps.currentExtent : *windowSize,
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .imageFormat      = vlkSwapchainImageFormat,
      .imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
      .preTransform     = surface_caps.currentTransform,
      .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .clipped          = VK_TRUE,
  };
  VK_CHECK(vkCreateSwapchainKHR(vlkDevice, &create_swapchain, nullptr, &vlkSwapchain));
  Uint32 num_images = 0;
  vkGetSwapchainImagesKHR(vlkDevice, vlkSwapchain, &num_images, nullptr);
  assert(num_images > 0);
  vlkSwapchainImages     = calloc((1 + num_images), sizeof(VkImage));       // ensuring trailing nullptr for iteration
  vlkSwapchainImageViews = calloc((1 + num_images), sizeof(VkImageView));   // dito
  vkGetSwapchainImagesKHR(vlkDevice, vlkSwapchain, &num_images, vlkSwapchainImages);
  for (Uint32 i = 0; i < num_images; i++) {
    VkImageViewCreateInfo create_imageview = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = vlkSwapchainImages[i],
        .format           = vlkSwapchainImageFormat,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
    };
    VK_CHECK(vkCreateImageView(vlkDevice, &create_imageview, nullptr, &vlkSwapchainImageViews[i]));
  }
}



void vlkInitCommands() {
  VkCommandPoolCreateInfo create_pool = vlkCommandPoolCreateInfo(vke.vlkQueueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(vlkDevice, &create_pool, nullptr, &vke.frames[i].commandPool));
    VkCommandBufferAllocateInfo buf_alloc = vlkCommandBufferAllocateInfo(vke.frames[i].commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(vlkDevice, &buf_alloc, &vke.frames[i].mainCommandBuffer));
  }
}



void vlkInitSyncStructures() {
  VkFenceCreateInfo create_fence
      = vlkFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);   // start signalled so we can wait on it on the first frame
  VkSemaphoreCreateInfo create_sema = vlkSemaphoreCreateInfo(0);
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(vlkDevice, &create_fence, nullptr, &vke.frames[i].fenceRender));
    VK_CHECK(vkCreateSemaphore(vlkDevice, &create_sema, nullptr, &vke.frames[i].semaRender));
    VK_CHECK(vkCreateSemaphore(vlkDevice, &create_sema, nullptr, &vke.frames[i].semaPresent));
  }
}



void vkeDispose() {
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(vlkDevice, vke.frames[i].commandPool, nullptr);
    vkDestroyFence(vlkDevice, vke.frames[i].fenceRender, nullptr);
    vkDestroySemaphore(vlkDevice, vke.frames[i].semaRender, nullptr);
    vkDestroySemaphore(vlkDevice, vke.frames[i].semaPresent, nullptr);
  }
  vlkDisposeSwapchain();
  vkDestroySurfaceKHR(vlkInstance, vlkSurface, nullptr);
  vkDestroyDevice(vlkDevice, nullptr);
  vkDestroyInstance(vlkInstance, nullptr);
  SDL_DestroyWindow(vke.window);
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
  vlkRecreateSwapchain(nullptr);
  vkGetDeviceQueue(vlkDevice, 0, 0, &vke.vlkQueue);
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


FrameData* vkeCurrentFrame() {
  return &vke.frames[vke.n_frame % FRAME_OVERLAP];
}


void vkeDraw() {
  const Uint64 timeout_syncs = 22u * 1000000000;   // secs * nanosecs

  FrameData* frame = vkeCurrentFrame();
  VK_CHECK(vkWaitForFences(vlkDevice, 1, &frame->fenceRender, true, timeout_syncs));
  VK_CHECK(vkResetFences(vlkDevice, 1, &frame->fenceRender));

  // request image from the swapchain
  Uint32 idx_image;
  VK_CHECK(vkAcquireNextImageKHR(vlkDevice, vlkSwapchain, timeout_syncs, frame->semaPresent, nullptr, &idx_image));

  VkCommandBuffer cmdbuf = frame->mainCommandBuffer;
  VK_CHECK(vkResetCommandBuffer(cmdbuf, 0));

  VkCommandBufferBeginInfo cmdbuf_begin
      = vlkCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);   // will use this command buffer exactly once
  VK_CHECK(vkBeginCommandBuffer(cmdbuf, &cmdbuf_begin));
  {
    // swapchain image into writeable mode before rendering
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_image], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clear_value = {
        .float32 = {0, 0, fabsf(sinf(((float) vke.n_frame) / 120.0f)), 1}
    };
    VkImageSubresourceRange clear_range = vlkImgSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmdbuf, vlkSwapchainImages[idx_image], VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);

    // swapchain image into presentable mode
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_image], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }
  VK_CHECK(vkEndCommandBuffer(cmdbuf));

  {   // SUBMIT TO QUEUE
    VkCommandBufferSubmitInfo cmdbuf_submit = vlkCommandBufferSubmitInfo(cmdbuf);
    // wait on the present semaphore, as that semaphore is signaled when the swapchain is ready
    VkSemaphoreSubmitInfo sema_wait_submit  = vlkSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame->semaPresent);
    // signal the render semaphore to signal that rendering has finished
    VkSemaphoreSubmitInfo sema_sig_submit   = vlkSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame->semaRender);

    VkSubmitInfo2 submit = vlkSubmitInfo(&cmdbuf_submit, &sema_sig_submit, &sema_wait_submit);
    // submit. fence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(vke.vlkQueue, 1, &submit, frame->fenceRender));
  }

  // PRESENT TO WINDOW
  VK_CHECK(vkQueuePresentKHR(vke.vlkQueue, &(VkPresentInfoKHR) {
                                               .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                               .swapchainCount     = 1,
                                               .pSwapchains        = &vlkSwapchain,
                                               .waitSemaphoreCount = 1,   // wait on render sema to only present after all draws done:
                                               .pWaitSemaphores    = &frame->semaRender,
                                               .pImageIndices      = &idx_image,
                                           }));
  vke.n_frame++;
}
