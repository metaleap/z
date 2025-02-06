#include "./vkguide.h"


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
  auto   inst_exts = SDL_Vulkan_GetInstanceExtensions(&num_exts);

  VkApplicationInfo inst_app      = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_4};
  const char*       inst_layers[] = {
      "VK_LAYER_KHRONOS_validation",   // "VK_LAYER_KHRONOS_validation" MUST remain the last entry!
  };
  VkInstanceCreateInfo inst_create = {.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                      .enabledExtensionCount   = num_exts,
                                      .ppEnabledExtensionNames = inst_exts,
                                      .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
                                      .ppEnabledLayerNames     = inst_layers,
                                      .pApplicationInfo        = &inst_app};
  VK_CHECK(vkCreateInstance(&inst_create, nullptr, &vlkInstance));

  SDL_CHECK(SDL_Vulkan_CreateSurface(vke.window, vlkInstance, nullptr, &vlkSurface));

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

  const char* device_exts[] = {"VK_KHR_swapchain",           "VK_KHR_maintenance1",          "VK_KHR_synchronization2",
                               "VK_EXT_descriptor_indexing", "VK_KHR_buffer_device_address", "VK_KHR_dynamic_rendering"};
  VkDeviceQueueCreateInfo          queue_create = {.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                   .queueCount       = 1,
                                                   .queueFamilyIndex = 0,
                                                   .pQueuePriorities = &(float) {1.0f}};
  VkPhysicalDeviceVulkan12Features features     = {
          .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
          .bufferDeviceAddress = true,
          .descriptorIndexing  = true,
          .pNext = &(VkPhysicalDeviceVulkan13Features) {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                                                        .dynamicRendering = true,
                                                        .synchronization2 = true}
  };
  VkDeviceCreateInfo device_create = {
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount    = 1,
      .pQueueCreateInfos       = &queue_create,
      .enabledExtensionCount   = ARR_LEN(device_exts),
      .ppEnabledExtensionNames = device_exts,
      .enabledLayerCount       = ARR_LEN(inst_layers) - (isDebug ? 0 : 1),
      .ppEnabledLayerNames     = inst_layers,
      .pNext = &(VkPhysicalDeviceFeatures2) {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features}
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
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface          = vlkSurface,
      .minImageCount    = surface_caps.minImageCount + ((surface_caps.maxImageCount > surface_caps.minImageCount) ? 1 : 0),
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
  vlkSwapchainImages     = calloc((1 + num_images), sizeof(VkImage));       // ensuring trailing nullptr for iteration
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
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                         vke.drawImage.extent);
  VmaAllocationCreateInfo rimg_alloc = {.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
                                        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
  VK_CHECK(vmaCreateImage(vke.alloc, &rimg_create, &rimg_alloc, &vke.drawImage.image, &vke.drawImage.alloc, nullptr));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.drawImage.image, vke.drawImage.alloc);
  VkImageViewCreateInfo rview_create =
      vlkImageViewCreateInfo(vke.drawImage.format, vke.drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
  VK_CHECK(vkCreateImageView(vlkDevice, &rview_create, nullptr, &vke.drawImage.defaultView));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.drawImage.defaultView, nullptr);

  vke.depthImage.format = VK_FORMAT_D32_SFLOAT;
  vke.depthImage.extent = vke.drawImage.extent;
  VkImageCreateInfo dimg_create =
      vlkImageCreateInfo(vke.depthImage.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, vke.depthImage.extent);
  VK_CHECK(vmaCreateImage(vke.alloc, &dimg_create, &rimg_alloc, &vke.depthImage.image, &vke.depthImage.alloc, nullptr));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.depthImage.image, vke.depthImage.alloc);
  VkImageViewCreateInfo dview_create =
      vlkImageViewCreateInfo(vke.depthImage.format, vke.depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);
  VK_CHECK(vkCreateImageView(vlkDevice, &dview_create, nullptr, &vke.depthImage.defaultView));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.depthImage.defaultView, nullptr);
}



void vkeInitSwapchain() {
  vkeCreateSwapchain(vke.windowExtent.width, vke.windowExtent.height);
}



void vkeResizeSwapchain() {
  VK_CHECK(vkDeviceWaitIdle(vlkDevice));
  vke.resizeRequested = false;
  vlkDisposeSwapchain();
  int w, h;
  assert(SDL_GetWindowSize(vke.window, &w, &h));
  if ((vke.windowExtent.width != (Uint32) w) || (vke.windowExtent.height != (Uint32) h)) {
    vke.windowExtent.width  = w;
    vke.windowExtent.height = h;
    vkeCreateSwapchain(vke.windowExtent.width, vke.windowExtent.height);
  }
}



void vkeInitCommands() {
  VkCommandPoolCreateInfo create_pool =
      vlkCommandPoolCreateInfo(vlkQueueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(vlkDevice, &create_pool, nullptr, &vke.frames[i].commandPool));
    VkCommandBufferAllocateInfo buf_alloc = vlkCommandBufferAllocateInfo(vke.frames[i].commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(vlkDevice, &buf_alloc, &vke.frames[i].mainCommandBuffer));
  }

  VK_CHECK(vkCreateCommandPool(vlkDevice, &create_pool, nullptr, &vke.immCommandPool));
  VkCommandBufferAllocateInfo create_immbuf = vlkCommandBufferAllocateInfo(vke.immCommandPool, 1);
  VK_CHECK(vkAllocateCommandBuffers(vlkDevice, &create_immbuf, &vke.immCommandBuffer));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, vke.immCommandPool, nullptr);
}



void vkeInitDescriptors() {
  VlkDescriptorAllocatorSizeRatio size_ratios[] = {
      {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 1}
  };
  VlkDescriptorAllocator_initPool(&vke.globalDescriptorAlloc, vlkDevice, 10, ARR_LEN(size_ratios), size_ratios);

  VlkDescriptorLayoutBuilder builder_compute = {};
  VlkDescriptorLayoutBuilder_addBinding(&builder_compute, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  vke.drawImageDescriptorLayout =
      VlkDescriptorLayoutBuilder_build(&builder_compute, vlkDevice, VK_SHADER_STAGE_COMPUTE_BIT, nullptr, 0);

  vke.drawImageDescriptors =
      VlkDescriptorAllocator_allocate(&vke.globalDescriptorAlloc, vlkDevice, vke.drawImageDescriptorLayout);

  VlkDescriptorWriter writer = {};
  VlkDescriptorWriter_writeImage(&writer, 0, vke.drawImage.defaultView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL,
                                 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  VlkDescriptorWriter_updateSet(&writer, vlkDevice, vke.drawImageDescriptors);

  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, &vke.globalDescriptorAlloc, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, vke.drawImageDescriptorLayout,
                 nullptr);

  static constexpr VlkDescriptorAllocatorSizeRatio frame_sizes[] = {
      {         .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 3},
      {        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 3},
      {        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 3},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .ratio = 4},
  };
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    vke.frames[i].frameDescriptors = (VlkDescriptorAllocatorGrowable) {};
    VlkDescriptorAllocatorGrowable_init(
        &vke.frames[i].frameDescriptors, vlkDevice, 1000,
        (VlkDescriptorAllocatorSizeRatios) {.buffer = frame_sizes, .count = 4, .capacity = 4});
  }

  vke.gpuSceneData                         = (GpuSceneData) {};
  VlkDescriptorLayoutBuilder builder_scene = {};
  VlkDescriptorLayoutBuilder_addBinding(&builder_scene, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  vke.gpuSceneDataDescriptorLayout = VlkDescriptorLayoutBuilder_build(
      &builder_scene, vlkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr, 0);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, vke.gpuSceneDataDescriptorLayout,
                 nullptr);

  VlkDescriptorLayoutBuilder builder_single_image = {};
  VlkDescriptorLayoutBuilder_addBinding(&builder_single_image, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  vke.singleTexDescriptorLayout =
      VlkDescriptorLayoutBuilder_build(&builder_single_image, vlkDevice, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr, 0);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, vke.singleTexDescriptorLayout,
                 nullptr);
}



void vkeInitSyncStructures() {
  VkFenceCreateInfo create_fence =
      vlkFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);   // start signalled so we can wait on it on the first frame
  VkSemaphoreCreateInfo create_sema = vlkSemaphoreCreateInfo(0);
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(vlkDevice, &create_fence, nullptr, &vke.frames[i].fenceRender));
    VK_CHECK(vkCreateSemaphore(vlkDevice, &create_sema, nullptr, &vke.frames[i].semaRender));
    VK_CHECK(vkCreateSemaphore(vlkDevice, &create_sema, nullptr, &vke.frames[i].semaPresent));
  }

  VK_CHECK(vkCreateFence(vlkDevice, &create_fence, nullptr, &vke.immFence));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, vke.immFence, nullptr);
}



void vkeShutdown() {
  vkDeviceWaitIdle(vlkDevice);

  cppImguiShutdown();
  for (size_t i = 0; i < FRAME_OVERLAP; i++) {
    disposals_flush(&vke.frames[i].disposals);
    VlkDescriptorAllocatorGrowable_destroyPools(&vke.frames[i].frameDescriptors, vlkDevice);
    vkDestroyFence(vlkDevice, vke.frames[i].fenceRender, nullptr);
    vkDestroySemaphore(vlkDevice, vke.frames[i].semaRender, nullptr);
    vkDestroySemaphore(vlkDevice, vke.frames[i].semaPresent, nullptr);
    vkDestroyCommandPool(vlkDevice, vke.frames[i].commandPool, nullptr);
  }
  disposals_flush(&vke.disposals);
  vlkDisposeSwapchain();
  vkDestroySurfaceKHR(vlkInstance, vlkSurface, nullptr);
  vmaDestroyAllocator(vke.alloc);
  vkDestroyDevice(vlkDevice, nullptr);
  vkDestroyInstance(vlkInstance, nullptr);
  SDL_DestroyWindow(vke.window);
}



void vkeInitComputePipelines() {
  VkPushConstantRange pushes = {.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .size = sizeof(ComputeShaderPushConstants)};
  VkPipelineLayoutCreateInfo layout = {.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                       .setLayoutCount         = 1,
                                       .pSetLayouts            = &vke.drawImageDescriptorLayout,
                                       .pushConstantRangeCount = 1,
                                       .pPushConstantRanges    = &pushes};
  VK_CHECK(vkCreatePipelineLayout(vlkDevice, &layout, nullptr, &vke.computePipelineLayout));

  VkShaderModule shader_gradient;
  VK_CHECK(vlkLoadShaderModule("../../vkguide/shaders/gradient_color.comp", vlkDevice, &shader_gradient,
                               VK_SHADER_STAGE_COMPUTE_BIT));
  ComputeShaderEffect gradient = {
      .layout   = vke.computePipelineLayout,
      .name     = "gradient",
      .pushData = {.data1 = {.r = 1, .g = 0, .b = 0, .a = 1}, .data2 = {.r = 0, .g = 1, .b = 0, .a = 1}}
  };
  VkComputePipelineCreateInfo pipeline = {
      .sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .layout = vke.computePipelineLayout,
      .stage  = (VkPipelineShaderStageCreateInfo) {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                   .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
                                                   .module = shader_gradient,
                                                   .pName  = "main"}
  };
  VK_CHECK(vkCreateComputePipelines(vlkDevice, VK_NULL_HANDLE, 1, &pipeline, nullptr, &gradient.pipeline));
  vkDestroyShaderModule(vlkDevice, shader_gradient, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, vke.computePipelineLayout, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, gradient.pipeline, nullptr);

  VkShaderModule shader_sky;
  VK_CHECK(vlkLoadShaderModule("../../vkguide/shaders/sky.comp", vlkDevice, &shader_sky, VK_SHADER_STAGE_COMPUTE_BIT));
  pipeline.stage.module   = shader_sky;
  ComputeShaderEffect sky = {.layout   = vke.computePipelineLayout,
                             .name     = "sky",
                             .pushData = {.data1 = {.x = 0.1f, .y = 0.2f, .z = 0.4f, .w = 0.97f}}};
  VK_CHECK(vkCreateComputePipelines(vlkDevice, VK_NULL_HANDLE, 1, &pipeline, nullptr, &sky.pipeline));
  vkDestroyShaderModule(vlkDevice, shader_sky, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, sky.pipeline, nullptr);

  vke.bgEffects[0] = gradient;
  vke.bgEffects[1] = sky;
}



void vkeInitMeshPipeline() {
  VkShaderModule shader_frag, shader_vert;
  VK_CHECK(
      vlkLoadShaderModule("../../vkguide/shaders/tex_image.frag", vlkDevice, &shader_frag, VK_SHADER_STAGE_FRAGMENT_BIT));
  VK_CHECK(vlkLoadShaderModule("../../vkguide/shaders/colored_triangle_mesh.vert", vlkDevice, &shader_vert,
                               VK_SHADER_STAGE_VERTEX_BIT));

  VkPushConstantRange push_range = {.size = sizeof(GpuDrawPushConstants), .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

  VkPipelineLayoutCreateInfo pipeline_layout = {.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                .setLayoutCount         = 1,
                                                .pSetLayouts            = &vke.singleTexDescriptorLayout,
                                                .pushConstantRangeCount = 1,
                                                .pPushConstantRanges    = &push_range};
  VK_CHECK(vkCreatePipelineLayout(vlkDevice, &pipeline_layout, nullptr, &vke.meshPipelineLayout));

  PipelineBuilder pb;
  PipelineBuilder_reset(&pb);
  PipelineBuilder_setShaders(&pb, shader_vert, shader_frag);
  pb.pipelineLayout = vke.meshPipelineLayout;
  PipelineBuilder_setInputTopology(&pb, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  PipelineBuilder_setPolygonMode(&pb, VK_POLYGON_MODE_FILL);
  PipelineBuilder_setCullMode(&pb, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  PipelineBuilder_setMultisamplingNone(&pb);
  PipelineBuilder_enableBlendingAlphaBlend(&pb);
  // PipelineBuilder_disableBlending(&pb);
  PipelineBuilder_enableDepthTest(&pb, true, VK_COMPARE_OP_GREATER_OR_EQUAL);
  PipelineBuilder_setColorAttachmentFormat(&pb, vke.drawImage.format);
  PipelineBuilder_setDepthFormat(&pb, vke.depthImage.format);
  vke.meshPipeline = PipelineBuilder_build(&pb, vlkDevice);
  vkDestroyShaderModule(vlkDevice, shader_frag, nullptr);
  vkDestroyShaderModule(vlkDevice, shader_vert, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, vke.meshPipelineLayout, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, vke.meshPipeline, nullptr);
}



void vkeInitPipelines() {
  // compute
  vkeInitComputePipelines();
  // graphics
  vkeInitMeshPipeline();
  MatGltfMetallicRoughness_buildPipelines(&vke.defaultMaterialMetalRough);
}




void vkeInitDefaultData() {
  Uint32 col_white   = 0xffffffff;
  Uint32 col_black   = 0xff000000;
  Uint32 col_gray    = 0xffaaaaaa;
  Uint32 col_magenta = 0xffff00ff;
  vke.texBlack = vkeUploadImage(&col_black, (VkExtent3D) {.depth = 1, .width = 1, .height = 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_USAGE_SAMPLED_BIT, false);
  vke.texGrey  = vkeUploadImage(&col_gray, (VkExtent3D) {.depth = 1, .width = 1, .height = 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_USAGE_SAMPLED_BIT, false);
  vke.texWhite = vkeUploadImage(&col_white, (VkExtent3D) {.depth = 1, .width = 1, .height = 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_USAGE_SAMPLED_BIT, false);
  Uint32 pix_checkerboard[16 * 16];
  for (int x = 0; x < 16; x++)
    for (int y = 0; y < 16; y++)
      pix_checkerboard[x + (y * 16)] = (((x % 2) ^ (y % 2)) ? col_magenta : col_black);
  vke.texCheckerboard = vkeUploadImage(pix_checkerboard, (VkExtent3D) {.depth = 1, .width = 16, .height = 16},
                                       VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

  VkSamplerCreateInfo sampl = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST};
  VK_CHECK(vkCreateSampler(vlkDevice, &sampl, nullptr, &vke.defaultSamplerNearest));
  sampl.magFilter = VK_FILTER_LINEAR;
  sampl.minFilter = VK_FILTER_LINEAR;
  VK_CHECK(vkCreateSampler(vlkDevice, &sampl, nullptr, &vke.defaultSamplerLinear));
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, vke.defaultSamplerLinear, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, vke.defaultSamplerNearest, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.texBlack.defaultView, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.texGrey.defaultView, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.texWhite.defaultView, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, vke.texCheckerboard.defaultView, nullptr);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.texBlack.image, vke.texBlack.alloc);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.texGrey.image, vke.texGrey.alloc);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.texWhite.image, vke.texWhite.alloc);
  disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, vke.texCheckerboard.image,
                 vke.texCheckerboard.alloc);

  MatGltfMetallicRoughnessMaterialResources mat_res = {.colorImage        = vke.texWhite,
                                                       .colorSampler      = vke.defaultSamplerLinear,
                                                       .metalRoughImage   = vke.texWhite,
                                                       .metalRoughSampler = vke.defaultSamplerLinear};
  // VlkBuffer
}



void vkeInitImgui() {
  // 1: create descriptor pool for IMGUI. the size of the pool is very oversize
  VkDescriptorPoolSize pool_sizes[] = {
      {               .type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1000},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000},
      {         .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1000},
      {         .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1000},
      {  .type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 1000},
      {  .type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 1000},
      {        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000},
      {        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1000},
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 1000},
      {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount = 1000},
      {      .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1000},
  };
  VkDescriptorPoolCreateInfo pool_create = {.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                            .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                            .maxSets       = 1000,
                                            .poolSizeCount = ARR_LEN(pool_sizes),
                                            .pPoolSizes    = pool_sizes};
  VkDescriptorPool           pool_imgui;
  VK_CHECK(vkCreateDescriptorPool(vlkDevice, &pool_create, nullptr, &pool_imgui));
  disposals_push(&vke.disposals, -VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, pool_imgui, nullptr);

  // 2: initialize imgui library
  cppImguiInit(vke.window, vlkInstance, vlkGpu, vlkDevice, vlkQueue, pool_imgui, vlkSwapchainImageFormat);
}



void vkeInit() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK);
  vke.window = SDL_CreateWindow("vkguide.dev", vke.windowExtent.width, vke.windowExtent.height,
                                SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);
  SDL_CHECK(vke.window);
  vkeInitVulkan();
  vkeInitSwapchain();
  vkGetDeviceQueue(vlkDevice, 0, 0, &vlkQueue);
  vkeInitCommands();
  vkeInitSyncStructures();
  vkeInitDescriptors();
  vkeInitPipelines();
  vkeInitDefaultData();
  vkeInitImgui();
}



void vkeRun() {
  SDL_Event evt;
  bool      quit = false;
  while (!quit) {
    if (vke.paused)
      thrd_sleep(&(struct timespec) {.tv_nsec = 321 * 1000000}, nullptr);

    while ((!quit) && (SDL_PollEvent(&evt) != 0)) {
      switch (evt.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_DESTROYED:
          quit = true;
          break;
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_OCCLUDED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
          vke.paused = true;
          break;
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          vke.paused = false;
          break;
        case SDL_EVENT_WINDOW_RESIZED:
          // case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
          vke.resizeRequested = true;
          break;
      }
      if (!(quit || vke.paused || vke.resizeRequested))
        cppImguiProcessEvent(&evt);
    }
    if (quit)
      break;
    if (vke.resizeRequested)
      vkeResizeSwapchain();
    if (!vke.paused) {
      cppImguiRender();
      vkeDraw();
    }
  }
}



VlkBuffer vkeCreateBufferMapped(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
  VkBufferCreateInfo      buf   = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = allocSize, .usage = usage};
  VmaAllocationCreateInfo alloc = {.usage = memoryUsage, .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT};
  VlkBuffer               ret   = {};
  VK_CHECK(vmaCreateBuffer(vke.alloc, &buf, &alloc, &ret.buf, &ret.alloc, &ret.allocInfo));
  return ret;
}



FrameData* vkeCurrentFrame() {
  return &vke.frames[vke.frameNr % FRAME_OVERLAP];
}



void vkeImmediateSubmitBegin() {
  VK_CHECK(vkResetFences(vlkDevice, 1, &vke.immFence));
  VK_CHECK(vkResetCommandBuffer(vke.immCommandBuffer, 0));
  VkCommandBufferBeginInfo cmdbuf_begin = vlkCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VK_CHECK(vkBeginCommandBuffer(vke.immCommandBuffer, &cmdbuf_begin));
}



void vkeImmediateSubmitEnd() {
  VK_CHECK(vkEndCommandBuffer(vke.immCommandBuffer));
  VkCommandBufferSubmitInfo submit  = vlkCommandBufferSubmitInfo(vke.immCommandBuffer);
  VkSubmitInfo2             submit2 = vlkSubmitInfo(&submit, nullptr, nullptr);
  VK_CHECK(vkQueueSubmit2(vlkQueue, 1, &submit2, vke.immFence));
  // vke.immFence will now block until the commands finish execution
  VK_CHECK(vkWaitForFences(vlkDevice, 1, &vke.immFence, true, VKE_VLK_TIMEOUTS_NS));
}



void vkeDraw_computeThreads(VkCommandBuffer cmdBuf) {
  ComputeShaderEffect* effect = &vke.bgEffects[vke.bgEffectCurIdx];
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, effect->pipeline);
  vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, vke.computePipelineLayout, 0, 1,
                          &vke.drawImageDescriptors, 0, nullptr);
  vkCmdPushConstants(cmdBuf, vke.computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputeShaderPushConstants),
                     &effect->pushData);
  vkCmdDispatch(cmdBuf, ceilf(((float) vke.drawExtent.width) / 16.0f), ceilf(((float) vke.drawExtent.height) / 16.0f), 1);
}



void vkeDraw_Imgui(VkCommandBuffer cmdBuf, VkImageView targetImageView) {
  VkRenderingAttachmentInfo attach =
      vlkRenderingAttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkRenderingInfo render = vlkRenderingInfo(vlkSwapchainExtent, &attach, nullptr);
  vkCmdBeginRendering(cmdBuf, &render);
  cppImguiDraw(cmdBuf);
  vkCmdEndRendering(cmdBuf);
}



void vkeDraw_Geometry(VkCommandBuffer cmdBuf) {
  FrameData* frame = vkeCurrentFrame();
  // {
  //   VlkBuffer buf_scene_data =
  //       vkeCreateBufferMapped(sizeof(GpuSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  //   disposals_push(&frame->disposals, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, buf_scene_data.buf, buf_scene_data.alloc);
  //   GpuSceneData* scene_data        = (GpuSceneData*) buf_scene_data.allocInfo.pMappedData;
  //   *scene_data                     = vke.gpuSceneData;
  //   VkDescriptorSet     global_desc = VlkDescriptorAllocatorGrowable_allocate(&frame->frameDescriptors, vlkDevice,
  //                                                                             vke.gpuSceneDataDescriptorLayout,
  //                                                                             nullptr);
  //   VlkDescriptorWriter writer_desc = {};
  //   VlkDescriptorWriter_writeBuffer(&writer_desc, 0, buf_scene_data.buf, sizeof(GpuSceneData), 0,
  //                                   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  //   VlkDescriptorWriter_updateSet(&writer_desc, vlkDevice, global_desc);
  // }

  VkRenderingAttachmentInfo color_attach =
      vlkRenderingAttachmentInfo(vke.drawImage.defaultView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkRenderingAttachmentInfo depth_attach =
      vlkRenderingAttachmentInfoDepth(vke.depthImage.defaultView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
  VkRenderingInfo render_info = vlkRenderingInfo(vke.drawExtent, &color_attach, &depth_attach);
  vkCmdBeginRendering(cmdBuf, &render_info);
  {
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vke.meshPipeline);
    // dynamic viewport & scissor
    VkViewport viewport = {
        .width    = (float) vke.drawExtent.width,
        .height   = -(float) vke.drawExtent.height,   // saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
        .x        = 0,
        .y        = (float) vke.drawExtent.height,   // saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
        .minDepth = 0,
        .maxDepth = 1};
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    VkRect2D scissor = {
        .extent = {.width = vke.drawExtent.width, .height = vke.drawExtent.height}
    };
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    VkDescriptorSet     descset_singleimg = VlkDescriptorAllocatorGrowable_allocate(&frame->frameDescriptors, vlkDevice,
                                                                                    vke.singleTexDescriptorLayout, nullptr);
    VlkDescriptorWriter writer_singleimg  = {};
    VlkDescriptorWriter_writeImage(&writer_singleimg, 0, vke.texCheckerboard.defaultView, vke.defaultSamplerNearest,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    VlkDescriptorWriter_updateSet(&writer_singleimg, vlkDevice, descset_singleimg);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, vke.meshPipelineLayout, 0, 1, &descset_singleimg, 0,
                            nullptr);

    mat4s view = glms_translate(mat4_identity(), (vec3s) {.x = 0, .y = 0, .z = -5});
    mat4s proj = glms_perspective(glm_rad(70), (float) vke.drawExtent.width / (float) vke.drawExtent.height, 10000, 0.1f);
    GpuDrawPushConstants push = {.worldMatrix  = glms_mul(proj, view),
                                 .vertexBuffer = vke.testMeshes.buffer[2].meshBuffers.vertexBufferAddress};
    vkCmdPushConstants(cmdBuf, vke.meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GpuDrawPushConstants), &push);
    vkCmdBindIndexBuffer(cmdBuf, vke.testMeshes.buffer[2].meshBuffers.indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdBuf, vke.testMeshes.buffer[2].surfaces.buffer[0].count, 1,
                     vke.testMeshes.buffer[2].surfaces.buffer[0].idxStart, 0, 0);
  }
  vkCmdEndRendering(cmdBuf);
}



void vkeDraw() {
  vke.drawExtent.width  = vke.drawImage.extent.width;
  vke.drawExtent.height = vke.drawImage.extent.height;

  FrameData* frame = vkeCurrentFrame();
  VK_CHECK(vkWaitForFences(vlkDevice, 1, &frame->fenceRender, true, VKE_VLK_TIMEOUTS_NS));
  disposals_flush(&frame->disposals);
  VlkDescriptorAllocatorGrowable_clearPools(&frame->frameDescriptors, vlkDevice);
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
    {
      vkeDraw_computeThreads(cmdbuf);
      vlkImgTransition(cmdbuf, vke.drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      vlkImgTransition(cmdbuf, vke.depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
      vkeDraw_Geometry(cmdbuf);
    }
    // transition the draw image and the swapchain image into their correct transfer layouts
    vlkImgTransition(cmdbuf, vke.drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkResult err = vkAcquireNextImageKHR(vlkDevice, vlkSwapchain, VKE_VLK_TIMEOUTS_NS, frame->semaPresent, nullptr,
                                         &idx_swapchain_image);
    if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR)) {
      vke.resizeRequested = true;
      return;
    }
    VK_CHECK(err);
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_swapchain_image], VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // execute a copy from the draw image into the swapchain
    vlkImgCopy(cmdbuf, vke.drawImage.image, vlkSwapchainImages[idx_swapchain_image], vke.drawExtent, vlkSwapchainExtent);
    // set swapchain image layout to Attachment Optimal so we can draw it
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_swapchain_image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    // draw imgui into the swapchain image
    vkeDraw_Imgui(cmdbuf, vlkSwapchainImageViews[idx_swapchain_image]);
    // swapchain image into presentable mode
    vlkImgTransition(cmdbuf, vlkSwapchainImages[idx_swapchain_image], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
    VkResult err = vkQueuePresentKHR(
        vlkQueue,
        &(VkPresentInfoKHR) {.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                             .swapchainCount     = 1,
                             .pSwapchains        = &vlkSwapchain,
                             .waitSemaphoreCount = 1,   // wait on render sema to only present after all draws done:
                             .pWaitSemaphores    = &frame->semaRender,
                             .pImageIndices      = &idx_swapchain_image});
    if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR)) {
      vke.resizeRequested = true;
      return;
    }
    VK_CHECK(err);
  }
  vke.frameNr++;
}



void disposals_push(DisposalQueue* this, VkStructureType type, void* arg, VmaAllocation alloc) {
  assert((this->count < DISP_QUEUE_CAPACITY) && (arg != nullptr) && "disposals_push");
  this->types[this->count]  = type;
  this->args[this->count]   = arg;
  this->allocs[this->count] = alloc;
  this->count++;
}



void disposals_flush(DisposalQueue* this) {
  if (this->count > 0)
    for (int i = (this->count - 1); i >= 0; i--) {
      switch (this->types[i]) {
        case VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO:
          if (this->allocs[i] != nullptr)
            vmaDestroyImage(vke.alloc, (VkImage) this->args[i], this->allocs[i]);
          else
            vkDestroyImage(vlkDevice, (VkImage) this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO:
          if (this->allocs[i] != nullptr)
            vmaDestroyBuffer(vke.alloc, (VkBuffer) this->args[i], this->allocs[i]);
          else
            vkDestroyBuffer(vlkDevice, (VkBuffer) this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO:
          vkDestroyImageView(vlkDevice, (VkImageView) this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO:
          vkDestroySampler(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO:
          VlkDescriptorAllocator_destroyPool(this->args[i], vlkDevice);
          break;
        case -VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO:
          vkDestroyDescriptorPool(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO:
          vkDestroyDescriptorSetLayout(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
          vkDestroyPipeline(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO:
          vkDestroyPipelineLayout(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
          vkDestroyPipeline(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO:
          vkDestroyCommandPool(vlkDevice, this->args[i], nullptr);
          break;
        case VK_STRUCTURE_TYPE_FENCE_CREATE_INFO:
          vkDestroyFence(vlkDevice, this->args[i], nullptr);
          break;
        default:
          SDL_Log(">>>%d<<<\n", this->types[i]);
          assert(false && "disposals_flush");
      }
    }
  this->count = 0;
}



GpuMeshBuffers vkeUploadMesh(size_t nVerts, Vertex verts[], size_t nIndices, Uint32 indices[]) {
  size_t         size_verts = nVerts * sizeof(Vertex);
  size_t         size_idxs  = nIndices * sizeof(Uint32);
  GpuMeshBuffers ret        = {
             .vertexBuffer = vkeCreateBufferMapped(size_verts,
                                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                   VMA_MEMORY_USAGE_GPU_ONLY),
             .indexBuffer  = vkeCreateBufferMapped(size_idxs, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                   VMA_MEMORY_USAGE_GPU_ONLY),
  };
  VkBufferDeviceAddressInfo deviceAdressInfo = {.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                                                .buffer = ret.vertexBuffer.buf};
  ret.vertexBufferAddress                    = vkGetBufferDeviceAddress(vlkDevice, &deviceAdressInfo);

  VlkBuffer staging =
      vkeCreateBufferMapped(size_verts + size_idxs, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
  void* data = staging.allocInfo.pMappedData;
  memcpy(data, verts, size_verts);
  memcpy(data + size_verts, indices, size_idxs);
  vkeImmediateSubmitBegin();
  {
    VkBufferCopy copy_verts = {.size = size_verts};
    vkCmdCopyBuffer(vke.immCommandBuffer, staging.buf, ret.vertexBuffer.buf, 1, &copy_verts);
    VkBufferCopy copy_idxs = {.size = size_idxs, .srcOffset = size_verts};
    vkCmdCopyBuffer(vke.immCommandBuffer, staging.buf, ret.indexBuffer.buf, 1, &copy_idxs);
  }
  vkeImmediateSubmitEnd();
  vmaDestroyBuffer(vke.alloc, staging.buf, staging.alloc);
  return ret;
}



VlkImage vkeUploadImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipMapped) {
  size_t    data_size  = 4 * size.depth * size.width * size.height;
  VlkBuffer buf_upload = vkeCreateBufferMapped(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  memcpy(buf_upload.allocInfo.pMappedData, data, data_size);
  VlkImage ret =
      vkeCreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipMapped);
  vkeImmediateSubmitBegin();
  {
    vlkImgTransition(vke.immCommandBuffer, ret.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copy_region = {
        .imageExtent = size, .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1}
    };
    vkCmdCopyBufferToImage(vke.immCommandBuffer, buf_upload.buf, ret.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &copy_region);
    vlkImgTransition(vke.immCommandBuffer, ret.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  vkeImmediateSubmitEnd();
  vmaDestroyBuffer(vke.alloc, buf_upload.buf, buf_upload.alloc);
  return ret;
}



VlkImage vkeCreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipMapped) {
  VlkImage          ret        = {.format = format, .extent = size};
  VkImageCreateInfo img_create = vlkImageCreateInfo(format, usage, size);
  if (mipMapped)
    img_create.mipLevels = 1 + floor(log2((double) utilMax(size.width, size.height)));
  VmaAllocationCreateInfo alloc = {.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
                                   .requiredFlags = (VkMemoryPropertyFlags) VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
  VK_CHECK(vmaCreateImage(vke.alloc, &img_create, &alloc, &ret.image, &ret.alloc, nullptr));
  VkImageViewCreateInfo view_create = vlkImageViewCreateInfo(
      format, ret.image, (format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
  view_create.subresourceRange.levelCount = img_create.mipLevels;
  VK_CHECK(vkCreateImageView(vlkDevice, &view_create, nullptr, &ret.defaultView));
  return ret;
}
