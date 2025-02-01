#include "./vkguide.h"
#include <vulkan/vulkan_core.h>


VulkanEngine vke = {
    .windowExtent = {.width = 1600, .height = 900}
};

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
VkQueue          vlkQueue;
Uint32           vlkQueueFamilyIndex;



void vkeInitVulkan() {
  Uint32 num_exts;
  SDL_Vulkan_GetInstanceExtensions(vke.window, &num_exts, nullptr);
  const char* inst_exts[num_exts];
  SDL_Vulkan_GetInstanceExtensions(vke.window, &num_exts, inst_exts);

  VkApplicationInfo inst_app = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_4};
  const char*       inst_layers[] = {
      "VK_LAYER_KHRONOS_validation"};   // "VK_LAYER_KHRONOS_validation" MUST remain the last entry!
  VkInstanceCreateInfo inst_create = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                      .enabledExtensionCount   = num_exts,
                                      .ppEnabledExtensionNames = inst_exts,
                                      .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
                                      .ppEnabledLayerNames     = inst_layers,
                                      .pApplicationInfo        = &inst_app};
  VK_CHECK(vkCreateInstance(&inst_create, nullptr, &vlkInstance));

  SDL_CHECK(SDL_Vulkan_CreateSurface(vke.window, vlkInstance, &vlkSurface));

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

  const char* device_exts[] = {
      "VK_KHR_swapchain",           "VK_KHR_maintenance1",          "VK_KHR_synchronization2",
      "VK_EXT_descriptor_indexing", "VK_KHR_buffer_device_address", "VK_KHR_dynamic_rendering"};
  VkDeviceQueueCreateInfo          queue_create = {.sType      = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                   .queueCount = 1,
                                                   .queueFamilyIndex = 0,
                                                   .pQueuePriorities = &(float) {1.0f}};
  VkPhysicalDeviceVulkan12Features features     = {
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
      .pNext = &(VkPhysicalDeviceFeatures2) {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                             .pNext = &features}
  };
  VK_CHECK(vkCreateDevice(vlkGpu, &device_create, nullptr, &vlkDevice));

  VmaAllocatorCreateInfo alloc_create = {.physicalDevice = vlkGpu,
                                         .device         = vlkDevice,
                                         .instance       = vlkInstance,
                                         .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT};
  VK_CHECK(vmaCreateAllocator(&alloc_create, &vke.alloc));
}



void vlkDisposeSwapchain() {
  if (vlkSwapchainImageViews != nullptr) {
    for (size_t i = 0; vlkSwapchainImageViews[i] != nullptr; i++)
      vkDestroyImageView(vlkDevice, vlkSwapchainImageViews[i], nullptr);
    vlkSwapchainImageViews = nullptr;
  }
  if (vlkSwapchain != nullptr) {
    vkDestroySwapchainKHR(vlkDevice, vlkSwapchain, nullptr);
    vlkSwapchain = nullptr;
  }
  vlkSwapchainImages = nullptr;   // they were disposed by the above vkDestroySwapchainKHR call
}



void vkeCreateSwapchain(Uint32 width, Uint32 height) {
  VkSurfaceCapabilitiesKHR surface_caps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vlkGpu, vlkSurface, &surface_caps));
  VkSwapchainCreateInfoKHR create_swapchain = {
      .sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = vlkSurface,
      .minImageCount =
          surface_caps.minImageCount + ((surface_caps.maxImageCount > surface_caps.minImageCount) ? 1 : 0),
      .imageExtent      = {.width = width, .height = height}, // surface_caps.currentExtent,
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
  vlkSwapchainExtent = create_swapchain.imageExtent;
  Uint32 num_images  = 0;
  vkGetSwapchainImagesKHR(vlkDevice, vlkSwapchain, &num_images, nullptr);
  assert(num_images > 0);
  vlkSwapchainImages = calloc((1 + num_images), sizeof(VkImage));   // ensuring trailing nullptr for iteration
  vlkSwapchainImageViews = calloc((1 + num_images), sizeof(VkImageView));   // dito
  vkGetSwapchainImagesKHR(vlkDevice, vlkSwapchain, &num_images, vlkSwapchainImages);
  for (Uint32 i = 0; i < num_images; i++) {
    VkImageViewCreateInfo create_imageview = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = vlkSwapchainImages[i],
        .format           = vlkSwapchainImageFormat,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .subresourceRange = {.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel   = 0,
                             .levelCount     = 1,
                             .baseArrayLayer = 0,
                             .layerCount     = 1},
    };
    VK_CHECK(vkCreateImageView(vlkDevice, &create_imageview, nullptr, &vlkSwapchainImageViews[i]));
  }

  vke.drawImage.extent = (VkExtent3D) {vke.windowExtent.width, vke.windowExtent.height, 1};
  vke.drawImage.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  VkImageCreateInfo rimg_create =
      vlkImageCreateInfo(vke.drawImage.format,   // allocate the draw-image from gpu local memory
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                         vke.drawImage.extent);
  VmaAllocationCreateInfo rimg_alloc = {.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
                                        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
  VK_CHECK(vmaCreateImage(vke.alloc, &rimg_create, &rimg_alloc, &vke.drawImage.image, &vke.drawImage.alloc,
                          nullptr));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.drawImage.image,
                 vke.drawImage.alloc);

  VkImageViewCreateInfo rview_create =
      vlkImageViewCreateInfo(vke.drawImage.format, vke.drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
  VK_CHECK(vkCreateImageView(vlkDevice, &rview_create, nullptr, &vke.drawImage.defaultView));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.drawImage.defaultView, nullptr);
}



void vkeInitSwapchain() {
  vkeCreateSwapchain(vke.windowExtent.width, vke.windowExtent.height);
}



void vkeInitCommands() {
  VkCommandPoolCreateInfo create_pool =
      vlkCommandPoolCreateInfo(vlkQueueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(vlkDevice, &create_pool, nullptr, &vke.frames[i].commandPool));
    VkCommandBufferAllocateInfo buf_alloc = vlkCommandBufferAllocateInfo(vke.frames[i].commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(vlkDevice, &buf_alloc, &vke.frames[i].mainCommandBuffer));
  }
}



void vkeInitDescriptors() {
  VlkDescriptorAllocatorSizeRatio size_ratios[] = {
      {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 1}
  };
  VlkDescriptorAllocator_initPool(&vke.globalDescriptorAlloc, vlkDevice, 10, ARR_LEN(size_ratios),
                                  size_ratios);

  VlkDescriptorLayoutBuilder builder = {};
  VlkDescriptorLayoutBuilder_addBinding(&builder, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  vke.drawImageDescriptorLayout =
      VlkDescriptorLayoutBuilder_build(&builder, vlkDevice, VK_SHADER_STAGE_COMPUTE_BIT, nullptr, 0);

  vke.drawImageDescriptors =
      VlkDescriptorAllocator_allocate(&vke.globalDescriptorAlloc, vlkDevice, vke.drawImageDescriptorLayout);
  VkDescriptorImageInfo img = {.imageLayout = VK_IMAGE_LAYOUT_GENERAL, .imageView = vke.drawImage.defaultView};
  VkWriteDescriptorSet  draw_image_write = {.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                            .dstBinding      = 0,
                                            .dstSet          = vke.drawImageDescriptors,
                                            .descriptorCount = 1,
                                            .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            .pImageInfo      = &img};
  vkUpdateDescriptorSets(vlkDevice, 1, &draw_image_write, 0, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, &vke.globalDescriptorAlloc,
                 nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                 vke.drawImageDescriptorLayout, nullptr);
}



void vkeInitSyncStructures() {
  VkFenceCreateInfo create_fence = vlkFenceCreateInfo(
      VK_FENCE_CREATE_SIGNALED_BIT);   // start signalled so we can wait on it on the first frame
  VkSemaphoreCreateInfo create_sema = vlkSemaphoreCreateInfo(0);
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(vlkDevice, &create_fence, nullptr, &vke.frames[i].fenceRender));
    VK_CHECK(vkCreateSemaphore(vlkDevice, &create_sema, nullptr, &vke.frames[i].semaRender));
    VK_CHECK(vkCreateSemaphore(vlkDevice, &create_sema, nullptr, &vke.frames[i].semaPresent));
  }
}



void vkeShutdown() {
  vkDeviceWaitIdle(vlkDevice);
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(vlkDevice, vke.frames[i].commandPool, nullptr);
    vkDestroyFence(vlkDevice, vke.frames[i].fenceRender, nullptr);
    vkDestroySemaphore(vlkDevice, vke.frames[i].semaRender, nullptr);
    vkDestroySemaphore(vlkDevice, vke.frames[i].semaPresent, nullptr);
    disposals_flush(&vke.frames[i].disposals);
  }
  disposals_flush(&vke.disposals);
  vlkDisposeSwapchain();
  vkDestroySurfaceKHR(vlkInstance, vlkSurface, nullptr);
  vmaDestroyAllocator(vke.alloc);
  vkDestroyDevice(vlkDevice, nullptr);
  vkDestroyInstance(vlkInstance, nullptr);
  SDL_DestroyWindow(vke.window);
}



void vkeInitBackgroundPipelines() {
  VkPipelineLayoutCreateInfo create_layout = {.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                              .setLayoutCount = 1,
                                              .pSetLayouts    = &vke.drawImageDescriptorLayout};
  VK_CHECK(vkCreatePipelineLayout(vlkDevice, &create_layout, nullptr, &vke.gradientPipelineLayout));

  VkShaderModule drawing_compute_shader;
  VK_CHECK(vlkLoadShaderModule("../../vkguide/shaders/gradient.comp", vlkDevice, &drawing_compute_shader) &&
           "vlkLoadShaderModule");
  VkPipelineShaderStageCreateInfo create_shaderstage = {
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = drawing_compute_shader,
      .pName  = "main"};
  VkComputePipelineCreateInfo create_pipeline = {.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                                 .layout = vke.gradientPipelineLayout,
                                                 .stage  = create_shaderstage};
  VK_CHECK(vkCreateComputePipelines(vlkDevice, VK_NULL_HANDLE, 1, &create_pipeline, nullptr,
                                    &vke.gradientPipeline));
  vkDestroyShaderModule(vlkDevice, drawing_compute_shader, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, vke.gradientPipelineLayout,
                 nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, vke.gradientPipeline,
                 nullptr);
}



void vkeInitPipelines() {
  vkeInitBackgroundPipelines();
}



void vkeInit() {
  SDL_Init(SDL_INIT_EVERYTHING);
  vke.window =
      SDL_CreateWindow("vkguide.dev", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, vke.windowExtent.width,
                       vke.windowExtent.height,
                       SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
  SDL_CHECK(vke.window);
  vkeInitVulkan();
  vkeInitSwapchain();
  vkGetDeviceQueue(vlkDevice, 0, 0, &vlkQueue);
  vkeInitCommands();
  vkeInitSyncStructures();
  vkeInitDescriptors();
  vkeInitPipelines();
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
}


FrameData* vkeCurrentFrame() {
  return &vke.frames[vke.frameNr % FRAME_OVERLAP];
}


void vkeDrawCore(VkCommandBuffer cmdBuf) {
  VkClearColorValue clear_value = {
      .float32 = {fabsf(sinf(((float) vke.frameNr) / 44.0f)), 0.44f, 0.22f, 1.0f}
  };
  VkImageSubresourceRange clear_range = vlkImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
  vkCmdClearColorImage(cmdBuf, vke.drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);
}


void vkeDraw() {
  const Uint64 timeout_syncs = 11u * 1000000000;   // secs * nanosecs
  vke.drawExtent.width       = vke.drawImage.extent.width;
  vke.drawExtent.height      = vke.drawImage.extent.height;

  FrameData* frame = vkeCurrentFrame();
  VK_CHECK(vkWaitForFences(vlkDevice, 1, &frame->fenceRender, true, timeout_syncs));
  disposals_flush(&frame->disposals);
  VK_CHECK(vkResetFences(vlkDevice, 1, &frame->fenceRender));

  Uint32          idx_swapchain_image;
  // RECORD COMMANDS
  VkCommandBuffer cmdbuf = frame->mainCommandBuffer;
  VK_CHECK(vkResetCommandBuffer(cmdbuf, 0));
  VkCommandBufferBeginInfo cmdbuf_begin   // will use this command buffer exactly once:
      = vlkCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VK_CHECK(vkBeginCommandBuffer(cmdbuf, &cmdbuf_begin));
  {
    // transition our main draw image into general layout so we can write into it
    // we will overwrite it all so we dont care about what was the older layout
    vlkImgTransition(cmdbuf, vke.drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    // actual drawing:
    vkeDrawCore(cmdbuf);
    // transition the draw image and the swapchain image into their correct transfer layouts
    vlkImgTransition(cmdbuf, vke.drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VK_CHECK(vkAcquireNextImageKHR(vlkDevice, vlkSwapchain, timeout_syncs, frame->semaPresent, nullptr,
                                   &idx_swapchain_image));
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_swapchain_image], VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // execute a copy from the draw image into the swapchain
    vlkImgCopy(cmdbuf, vke.drawImage.image, vlkSwapchainImages[idx_swapchain_image], vke.drawExtent,
               vlkSwapchainExtent);
    // swapchain image into presentable mode
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_swapchain_image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }
  VK_CHECK(vkEndCommandBuffer(cmdbuf));

  {   // SUBMIT TO QUEUE
    VkCommandBufferSubmitInfo cmdbuf_submit = vlkCommandBufferSubmitInfo(cmdbuf);
    // wait on the present semaphore, as that semaphore is signaled when the swapchain is ready
    VkSemaphoreSubmitInfo     sema_wait_submit =
        vlkSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame->semaPresent);
    // signal the render semaphore to signal that rendering has finished
    VkSemaphoreSubmitInfo sema_sig_submit =
        vlkSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame->semaRender);

    VkSubmitInfo2 submit = vlkSubmitInfo(&cmdbuf_submit, &sema_sig_submit, &sema_wait_submit);
    // submit. fence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(vlkQueue, 1, &submit, frame->fenceRender));
  }

  {   // PRESENT TO WINDOW
    VK_CHECK(vkQueuePresentKHR(
        vlkQueue, &(VkPresentInfoKHR) {
                      .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                      .swapchainCount     = 1,
                      .pSwapchains        = &vlkSwapchain,
                      .waitSemaphoreCount = 1,   // wait on render sema to only present after all draws done:
                      .pWaitSemaphores    = &frame->semaRender,
                      .pImageIndices      = &idx_swapchain_image,
                  }));
  }
  vke.frameNr++;
}



void disposals_push(DisposalQueue* self, VkStructureType type, void* arg, VmaAllocation alloc) {
  assert((self->count < DISP_QUEUE_CAPACITY) && (arg != nullptr) && "disposals_push");
  self->types[self->count]  = type;
  self->args[self->count]   = arg;
  self->allocs[self->count] = alloc;
  self->count++;
}


void disposals_flush(DisposalQueue* self) {
  if (self->count > 0)
    for (int i = (self->count - 1); i >= 0; i--) {
      switch (self->types[i]) {
        case VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO:
          if (self->allocs[i] != nullptr)
            vmaDestroyImage(vke.alloc, (VkImage) self->args[i], self->allocs[i]);
          else
            vkDestroyImage(vlkDevice, (VkImage) self->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO:
          vkDestroyImageView(vlkDevice, (VkImageView) self->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO:
          VlkDescriptorAllocator_destroyPool(self->args[i], vlkDevice);
          break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO:
          vkDestroyDescriptorSetLayout(vlkDevice, self->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
          vkDestroyPipeline(vlkDevice, self->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO:
          vkDestroyPipelineLayout(vlkDevice, self->args[i], nullptr);
          break;
        default:
          SDL_Log(">>>%x<<<\n", self->types[i]);
          assert(false && "disposals_flush");
      }
    }
  self->count = 0;
}
