#pragma once

#include "cglm/struct/vec4.h"
#include "cglm/types-struct.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VMA_DEDICATED_ALLOCATION 1
#include "../3rdparty/GPUOpen-LibrariesAndSDKs_VulkanMemoryAllocator/include/vk_mem_alloc.h"

#include "../3rdparty/recp_cglm/include/cglm/struct.h"



VkRenderingInfo           vlkRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment,
                                           VkRenderingAttachmentInfo* depthAttachment);
VkRenderingAttachmentInfo vlkRenderingAttachmentInfo(VkImageView view, VkClearValue* clear,
                                                     VkImageLayout layout);
VkImageSubresourceRange   vlkImageSubresourceRange(VkImageAspectFlags aspectMask);
VkCommandBufferBeginInfo  vlkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
VkFenceCreateInfo         vlkFenceCreateInfo(VkFenceCreateFlags flags);
VkSemaphoreCreateInfo     vlkSemaphoreCreateInfo(VkSemaphoreCreateFlags flags);
VkCommandPoolCreateInfo   vlkCommandPoolCreateInfo(Uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
VkCommandBufferAllocateInfo vlkCommandBufferAllocateInfo(VkCommandPool cmdPool, Uint32 cmdBufCount);
VkSemaphoreSubmitInfo       vlkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
VkSubmitInfo2               vlkSubmitInfo(VkCommandBufferSubmitInfo* cmdBuf, VkSemaphoreSubmitInfo* sig,
                                          VkSemaphoreSubmitInfo* wait);
VkCommandBufferSubmitInfo   vlkCommandBufferSubmitInfo(VkCommandBuffer cmdBuf);
VkImageCreateInfo     vlkImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
VkImageViewCreateInfo vlkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
void                  vlkImgTransition(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout currentLayout,
                                       VkImageLayout newLayout);
void     vlkImgCopy(VkCommandBuffer cmdBuf, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
VkResult vlkLoadShaderModule(char* filePath, VkDevice device, VkShaderModule* retShaderModule,
                             VkShaderStageFlagBits shaderStage);
VkPipelineShaderStageCreateInfo vlkPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                                                 VkShaderModule        shaderModule,
                                                                 const char*           entryPointName);



#define ARR_LEN(_arr_)      (sizeof((_arr_)) / sizeof((_arr_)[0]))
#define FRAME_OVERLAP       3
#define VKE_VLK_TIMEOUTS_NS (11u * 1000000000)


typedef struct DisposalQueue {
#define DISP_QUEUE_CAPACITY 22
  int             count;
  VkStructureType types[DISP_QUEUE_CAPACITY];
  void*           args[DISP_QUEUE_CAPACITY];
  VmaAllocation   allocs[DISP_QUEUE_CAPACITY];
} DisposalQueue;
void disposals_push(DisposalQueue* self, VkStructureType type, void* arg, VmaAllocation alloc);
void disposals_flush(DisposalQueue* self);


typedef struct FrameData {
  VkCommandPool   commandPool;
  VkCommandBuffer mainCommandBuffer;
  VkFence         fenceRender;
  VkSemaphore     semaPresent, semaRender;
  DisposalQueue   disposals;
} FrameData;


typedef struct VlkImage {
  VkImage       image;
  VkImageView   defaultView;
  VkExtent3D    extent;
  VkFormat      format;
  VmaAllocation alloc;
} VlkImage;


typedef struct VlkDescriptorLayoutBuilder {
#define VDLB_CAP 8
  VkDescriptorSetLayoutBinding bindings[VDLB_CAP];   // increase (above) as needed
  Uint8                        count;
} VlkDescriptorLayoutBuilder;
void                  VlkDescriptorLayoutBuilder_clear(VlkDescriptorLayoutBuilder* self);
void                  VlkDescriptorLayoutBuilder_addBinding(VlkDescriptorLayoutBuilder* self, Uint32 binding,
                                                            VkDescriptorType type);
VkDescriptorSetLayout VlkDescriptorLayoutBuilder_build(VlkDescriptorLayoutBuilder* self, VkDevice device,
                                                       VkShaderStageFlags shaderStages, void* pNext,
                                                       VkDescriptorSetLayoutCreateFlags flags);


typedef struct VlkDescriptorAllocatorSizeRatio {
  VkDescriptorType type;
  float            ratio;
} VlkDescriptorAllocatorSizeRatio;
typedef struct VlkDescriptorAllocator {
  VkDescriptorPool                pool;
  VlkDescriptorAllocatorSizeRatio ratios[VDLB_CAP];
  Uint8                           ratiosCount;
} VlkDescriptorAllocator;
void            VlkDescriptorAllocator_initPool(VlkDescriptorAllocator* self, VkDevice device, Uint32 maxSets,
                                                Uint8 ratiosCount, VlkDescriptorAllocatorSizeRatio ratios[]);
void            VlkDescriptorAllocator_clearDescriptors(VlkDescriptorAllocator* self, VkDevice device);
void            VlkDescriptorAllocator_destroyPool(VlkDescriptorAllocator* self, VkDevice device);
VkDescriptorSet VlkDescriptorAllocator_allocate(VlkDescriptorAllocator* self, VkDevice device,
                                                VkDescriptorSetLayout layout);


typedef struct PipelineBuilder {
  VkPipelineShaderStageCreateInfo        shaderStages[2];
  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineColorBlendAttachmentState    colorBlendAttachment;
  VkPipelineMultisampleStateCreateInfo   multisampling;
  VkPipelineLayout                       pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo  depthStencil;
  VkPipelineRenderingCreateInfo          renderInfo;
  VkFormat                               colorAttachmentFormat;
} PipelineBuilder;
void       PipelineBuilder_reset(PipelineBuilder* self);
VkPipeline PipelineBuilder_build(PipelineBuilder* self, VkDevice device);
void       PipelineBuilder_setColorAttachmentFormat(PipelineBuilder* self, VkFormat format);
void       PipelineBuilder_disableBlending(PipelineBuilder* self);
void       PipelineBuilder_setMultisamplingNone(PipelineBuilder* self);
void       PipelineBuilder_setInputTopology(PipelineBuilder* self, VkPrimitiveTopology topo);
void       PipelineBuilder_setPolygonMode(PipelineBuilder* self, VkPolygonMode mode);
void       PipelineBuilder_setCullMode(PipelineBuilder* self, VkCullModeFlags cullMode, VkFrontFace frontFace);
void       PipelineBuilder_setDepthFormat(PipelineBuilder* self, VkFormat format);
void       PipelineBuilder_disableDepthTest(PipelineBuilder* self);
void PipelineBuilder_setShaders(PipelineBuilder* self, VkShaderModule vertShader, VkShaderModule fragShader);



typedef struct ComputeShaderPushConstants {
  vec4s data1;
  vec4s data2;
  vec4s data3;
  vec4s data4;
} ComputeShaderPushConstants;


typedef struct ComputeShaderEffect {
  char*                      name;
  VkPipeline                 pipeline;
  VkPipelineLayout           layout;
  ComputeShaderPushConstants pushData;
} ComputeShaderEffect;


typedef struct VulkanEngine {
  VmaAllocator           alloc;
  size_t                 frameNr;
  FrameData              frames[FRAME_OVERLAP];
  bool                   paused;
  SDL_Window*            window;
  DisposalQueue          disposals;
  VkExtent2D             windowExtent;
  VkExtent2D             drawExtent;
  VlkImage               drawImage;
  VlkDescriptorAllocator globalDescriptorAlloc;
  VkDescriptorSet        drawImageDescriptors;
  VkDescriptorSetLayout  drawImageDescriptorLayout;
  VkPipelineLayout       computePipelineLayout;
  VkFence                immFence;
  VkCommandPool          immCommandPool;
  VkCommandBuffer        immCommandBuffer;
  ComputeShaderEffect    bgEffects[2];
  int                    bgEffectCurIdx;
  VkPipelineLayout       triPipelineLayout;
  VkPipeline             triPipeline;
} VulkanEngine;


extern bool         isDebug;
extern VulkanEngine vke;


void vkeInit();
void vkeRun();
void vkeDraw();
void vkeShutdown();
#ifdef __cplusplus
extern "C" {
#endif
void cppImguiShutdown();
void cppImguiProcessEvent(SDL_Event* evt);
void cppImguiRender();
void cppImguiDraw(VkCommandBuffer cmdBuf);
void cppImguiInit(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, VkDevice device,
                  VkQueue queue, VkDescriptorPool pool, VkFormat swapchainImageFormat);
#ifdef __cplusplus
}
#endif


#define SDL_CHECK(x)                              \
  do {                                            \
    if (!(x)) {                                   \
      SDL_Log("SDL error: %s\n", SDL_GetError()); \
      exit(1);                                    \
    }                                             \
  } while (false)



#define VK_CHECK(x)                                           \
  do {                                                        \
    VkResult result = (x);                                    \
    if (result != VK_SUCCESS) {                               \
      SDL_Log("Vulkan error: %s\n", string_VkResult(result)); \
      exit(1);                                                \
    }                                                         \
  } while (false)
