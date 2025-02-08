#pragma once

#include <SDL3/SDL_events.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <threads.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#define VMA_DEDICATED_ALLOCATION 1
#include <vk_mem_alloc.h>

#define CGLM_ALL_UNALIGNED           1
#define CGLM_OMIT_NS_FROM_STRUCT_API 1
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include <cglm/struct.h>
#include <cgltf.h>
#include <generic_list.h>


LIST_DEFINE_H(U32s, U32s, Uint32);



VkRenderingInfo             vlkRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment,
                                             VkRenderingAttachmentInfo* depthAttachment);
VkRenderingAttachmentInfo   vlkRenderingAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout);
VkRenderingAttachmentInfo   vlkRenderingAttachmentInfoDepth(VkImageView view, VkImageLayout layout);
VkImageSubresourceRange     vlkImageSubresourceRange(VkImageAspectFlags aspectMask);
VkCommandBufferBeginInfo    vlkCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);
VkFenceCreateInfo           vlkFenceCreateInfo(VkFenceCreateFlags flags);
VkSemaphoreCreateInfo       vlkSemaphoreCreateInfo(VkSemaphoreCreateFlags flags);
VkCommandPoolCreateInfo     vlkCommandPoolCreateInfo(Uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
VkCommandBufferAllocateInfo vlkCommandBufferAllocateInfo(VkCommandPool cmdPool, Uint32 cmdBufCount);
VkSemaphoreSubmitInfo       vlkSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
VkSubmitInfo2 vlkSubmitInfo(VkCommandBufferSubmitInfo* cmdBuf, VkSemaphoreSubmitInfo* sig, VkSemaphoreSubmitInfo* wait);
VkCommandBufferSubmitInfo vlkCommandBufferSubmitInfo(VkCommandBuffer cmdBuf);
VkImageCreateInfo         vlkImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
VkImageViewCreateInfo     vlkImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
void     vlkImgTransition(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
void     vlkImgCopy(VkCommandBuffer cmdBuf, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
VkResult vlkLoadShaderModule(char* filePath, VkDevice device, VkShaderModule* retShaderModule,
                             VkShaderStageFlagBits shaderStage);
VkPipelineShaderStageCreateInfo vlkPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule,
                                                                 const char* entryPointName);



#define ARR_LEN(_arr_)      (sizeof((_arr_)) / sizeof((_arr_)[0]))
#define FRAME_OVERLAP       3
#define VKE_VLK_TIMEOUTS_NS (12345u * 1000000)   // ms * ns


typedef struct DisposalQueue {
#define DISP_QUEUE_CAPACITY 44
  int             count;
  VkStructureType types[DISP_QUEUE_CAPACITY];
  void*           args[DISP_QUEUE_CAPACITY];
  VmaAllocation   allocs[DISP_QUEUE_CAPACITY];
} DisposalQueue;
void disposals_push(DisposalQueue*, VkStructureType type, void* arg, VmaAllocation alloc);
void disposals_flush(DisposalQueue*);


typedef struct VlkImage {
  VkImage       image;
  VkImageView   defaultView;
  VkExtent3D    extent;
  VkFormat      format;
  VmaAllocation alloc;
} VlkImage;


typedef struct VlkBuffer {
  VkBuffer          buf;
  VmaAllocation     alloc;
  VmaAllocationInfo allocInfo;
} VlkBuffer;


typedef struct Vertex {
  vec3s position;
  float uv_x;
  vec3s normal;
  float uv_y;
  vec4s color;
} Vertex;
LIST_DEFINE_H(Verts, Verts, Vertex);


typedef struct GpuMeshBuffers {
  VlkBuffer       indexBuffer;
  VlkBuffer       vertexBuffer;
  VkDeviceAddress vertexBufferAddress;
} GpuMeshBuffers;


typedef struct GpuDrawPushConstants {
  mat4s           worldMatrix;
  VkDeviceAddress vertexBuffer;
} GpuDrawPushConstants;


typedef struct GpuSceneData {
  mat4s view;
  mat4s proj;
  mat4s viewProj;
  vec4s ambientColor;
  vec4s sunlightDirectionAndPower;
  vec4s sunlightColor;
} GpuSceneData;


typedef struct VlkDescriptorLayoutBuilder {
#define VDLB_CAP 8
  VkDescriptorSetLayoutBinding bindings[VDLB_CAP];   // increase (above) as needed
  Uint8                        count;
} VlkDescriptorLayoutBuilder;
void VlkDescriptorLayoutBuilder_clear(VlkDescriptorLayoutBuilder*);
void VlkDescriptorLayoutBuilder_addBinding(VlkDescriptorLayoutBuilder*, Uint32 binding, VkDescriptorType type);
VkDescriptorSetLayout VlkDescriptorLayoutBuilder_build(VlkDescriptorLayoutBuilder*, VkDevice device,
                                                       VkShaderStageFlags shaderStages, void* pNext,
                                                       VkDescriptorSetLayoutCreateFlags flags);


typedef struct VlkDescriptorAllocatorSizeRatio {
  VkDescriptorType type;
  float            ratio;
} VlkDescriptorAllocatorSizeRatio;
LIST_DEFINE_H(VlkDescriptorAllocatorSizeRatios, VlkDescriptorAllocatorSizeRatios, VlkDescriptorAllocatorSizeRatio);
LIST_DEFINE_H(VkDescriptorPoolSizes, VkDescriptorPoolSizes, VkDescriptorPoolSize);

LIST_DEFINE_H(VkDescriptorPools, VkDescriptorPools, VkDescriptorPool);
typedef struct VlkDescriptorAllocatorGrowable {
  VkDescriptorPools                fullPools;
  VkDescriptorPools                readyPools;
  VlkDescriptorAllocatorSizeRatios ratios;
  Uint32                           setsPerPool;
} VlkDescriptorAllocatorGrowable;
void            VlkDescriptorAllocatorGrowable_init(VlkDescriptorAllocatorGrowable*, VkDevice device, Uint32 maxSets,
                                                    VlkDescriptorAllocatorSizeRatios poolRatios);
void            VlkDescriptorAllocatorGrowable_clearPools(VlkDescriptorAllocatorGrowable*, VkDevice device);
void            VlkDescriptorAllocatorGrowable_destroyPools(VlkDescriptorAllocatorGrowable*, VkDevice device);
VkDescriptorSet VlkDescriptorAllocatorGrowable_allocate(VlkDescriptorAllocatorGrowable*, VkDevice device,
                                                        VkDescriptorSetLayout layout, void* pNext);


LIST_DEFINE_H(VkDescriptorImageInfos, VkDescriptorImageInfos, VkDescriptorImageInfo);
LIST_DEFINE_H(VkDescriptorBufferInfos, VkDescriptorBufferInfos, VkDescriptorBufferInfo);
LIST_DEFINE_H(VkWriteDescriptorSets, VkWriteDescriptorSets, VkWriteDescriptorSet);
typedef struct VlkDescriptorWriter {
  VkDescriptorImageInfos  imageInfos;
  VkDescriptorBufferInfos bufferInfos;
  VkWriteDescriptorSets   writes;
} VlkDescriptorWriter;
void VlkDescriptorWriter_writeImage(VlkDescriptorWriter*, int binding, VkImageView image, VkSampler sampler,
                                    VkImageLayout layout, VkDescriptorType type);
void VlkDescriptorWriter_writeBuffer(VlkDescriptorWriter*, int binding, VkBuffer buffer, size_t size, size_t offset,
                                     VkDescriptorType type);
void VlkDescriptorWriter_clear(VlkDescriptorWriter*);
void VlkDescriptorWriter_free(VlkDescriptorWriter*);
void VlkDescriptorWriter_updateSet(VlkDescriptorWriter*, VkDevice device, VkDescriptorSet set);


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
void       PipelineBuilder_reset(PipelineBuilder*);
void       PipelineBuilder_setColorAttachmentFormat(PipelineBuilder*, VkFormat format);
void       PipelineBuilder_disableBlending(PipelineBuilder*);
void       PipelineBuilder_enableBlending(PipelineBuilder*, VkBlendFactor dstColorBlendFactor);
void       PipelineBuilder_enableBlendingAdditive(PipelineBuilder*);
void       PipelineBuilder_enableBlendingAlphaBlend(PipelineBuilder*);
void       PipelineBuilder_setMultisamplingNone(PipelineBuilder*);
void       PipelineBuilder_setInputTopology(PipelineBuilder*, VkPrimitiveTopology topo);
void       PipelineBuilder_setPolygonMode(PipelineBuilder*, VkPolygonMode mode);
void       PipelineBuilder_setCullMode(PipelineBuilder*, VkCullModeFlags cullMode, VkFrontFace frontFace);
void       PipelineBuilder_setDepthFormat(PipelineBuilder*, VkFormat format);
void       PipelineBuilder_disableDepthTest(PipelineBuilder*);
void       PipelineBuilder_enableDepthTest(PipelineBuilder*, bool depthWriteEnable, VkCompareOp opCmp);
void       PipelineBuilder_setShaders(PipelineBuilder*, VkShaderModule vertShader, VkShaderModule fragShader);
VkPipeline PipelineBuilder_build(PipelineBuilder*, VkDevice device);



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


typedef struct MaterialPipeline {
  VkPipeline       pipeline;
  VkPipelineLayout layout;
} MaterialPipeline;


typedef enum MaterialPass {
  MaterialPass_MainColor,
  MaterialPass_Transparent,
  MaterialPass_Other,
} MaterialPass;


typedef struct MaterialInstance {
  MaterialPipeline* pipeline;
  VkDescriptorSet   materialSet;
  MaterialPass      passType;
} MaterialInstance;


typedef struct MatGltf {
  MaterialInstance data;
} MatGltf;


typedef struct MatGltfMetallicRoughnessMaterialConstants {
  vec4s colorFactors;
  vec4s metalRoughFactors;
  vec4s _padding[14];
} MatGltfMetallicRoughnessMaterialConstants;
typedef struct MatGltfMetallicRoughnessMaterialResources {
  VlkImage  colorImage;
  VkSampler colorSampler;
  VlkImage  metalRoughImage;
  VkSampler metalRoughSampler;
  VkBuffer  dataBuffer;
  Uint32    dataBufferOffset;
} MatGltfMetallicRoughnessMaterialResources;
typedef struct MatGltfMetallicRoughness {
  MaterialPipeline                          opaquePipeline;
  MaterialPipeline                          transparentPipeline;
  VkDescriptorSetLayout                     materialLayout;
  MatGltfMetallicRoughnessMaterialConstants materialConstants;
  MatGltfMetallicRoughnessMaterialResources materialResources;
  VlkDescriptorWriter                       descriptorWriter;
} MatGltfMetallicRoughness;
void             MatGltfMetallicRoughness_buildPipelines(MatGltfMetallicRoughness*);
void             MatGltfMetallicRoughness_clearResources(MatGltfMetallicRoughness*);
MaterialInstance MatGltfMetallicRoughness_writeMaterial(MatGltfMetallicRoughness*, MaterialPass pass,
                                                        MatGltfMetallicRoughnessMaterialResources* resources,
                                                        VlkDescriptorAllocatorGrowable*            descriptorAlloc);


typedef struct GeoSurface {
  Uint32  idxStart;
  Uint32  count;
  MatGltf material;
} GeoSurface;
LIST_DEFINE_H(GeoSurfaces, GeoSurfaces, GeoSurface);


typedef struct MeshAsset {
  const char*    name;
  GeoSurfaces    surfaces;
  GpuMeshBuffers meshBuffers;
} MeshAsset;
LIST_DEFINE_H(MeshAssets, MeshAssets, MeshAsset);


typedef struct RenderObject {
  mat4s             transform;
  VkBuffer          indexBuffer;
  MaterialInstance* material;
  VkDeviceAddress   vertexBufferAddress;
  Uint32            indexCount;
  Uint32            firstIndex;
} RenderObject;
LIST_DEFINE_H(RenderObjects, RenderObjects, RenderObject);


typedef struct DrawContext {
  RenderObjects opaqueSurfaces;
} DrawContext;


typedef struct SceneNode {
  mat4s              worldTransform;
  mat4s              localTransform;
  struct SceneNodes* children;
  struct SceneNode*  parent;
  MeshAsset*         mesh;
} SceneNode;
LIST_DEFINE_H(SceneNodes, SceneNodes, SceneNode);
void SceneNode_refreshTransform(SceneNode*, mat4s* parentMatrix);
void SceneNode_draw(SceneNode*, mat4s* topMatrix, DrawContext* ctx);


typedef struct Camera {
  vec3s velocity;
  vec3s position;
  float pitch;   // vertical rot
  float yaw;     // horizontal rot
} Camera;
mat4s Camera_getViewMatrix(Camera*);
mat4s Camera_getRotationMatrix(Camera*);
void  Camera_processEvent(Camera*, SDL_Event*);
void  Camera_update(Camera*);


typedef struct FrameData {
  VkCommandPool                  commandPool;
  VkCommandBuffer                mainCommandBuffer;
  VkFence                        fenceRender;
  VkSemaphore                    semaPresent, semaRender;
  DisposalQueue                  disposals;
  VlkDescriptorAllocatorGrowable frameDescriptors;
  VlkDescriptorWriter            tmpWriter;
} FrameData;


typedef struct VulkanEngine {
  VmaAllocator                   alloc;
  size_t                         frameNr;
  FrameData                      frames[FRAME_OVERLAP];
  bool                           paused;
  SDL_Window*                    window;
  DisposalQueue                  disposals;
  VkExtent2D                     windowExtent;
  VkExtent2D                     drawExtent;
  VlkImage                       drawImage;
  VlkImage                       depthImage;
  VlkDescriptorAllocatorGrowable globalDescriptorAlloc;
  VkDescriptorSet                drawImageDescriptors;
  VkDescriptorSetLayout          drawImageDescriptorLayout;
  VkPipelineLayout               computePipelineLayout;
  VkFence                        immFence;
  VkCommandPool                  immCommandPool;
  VkCommandBuffer                immCommandBuffer;
  ComputeShaderEffect            bgEffects[2];
  int                            bgEffectCurIdx;
  VkPipelineLayout               meshPipelineLayout;
  VkPipeline                     meshPipeline;
  MeshAssets                     testMeshes;
  bool                           resizeRequested;
  GpuSceneData                   gpuSceneData;
  VkDescriptorSetLayout          gpuSceneDataDescriptorLayout;
  VlkImage                       texWhite;
  VlkImage                       texGrey;
  VlkImage                       texBlack;
  VlkImage                       texCheckerboard;
  VkSampler                      defaultSamplerLinear;
  VkSampler                      defaultSamplerNearest;
  VkDescriptorSetLayout          singleTexDescriptorLayout;
  MaterialInstance               defaultMaterialInstance;
  MatGltfMetallicRoughness       defaultMaterialMetalRough;
  DrawContext                    mainDrawContext;
  SceneNodes                     loadedNodes;
} VulkanEngine;



extern bool         isDebug;
extern VulkanEngine vke;
extern VkDevice     vlkDevice;



size_t         utilMax(size_t s1, size_t s2);
void           vkeInit();
void           vkeRun();
void           vkeDraw();
void           vkeShutdown();
MeshAssets     vkeLoadGlb(char* filePath);
GpuMeshBuffers vkeUploadMesh(size_t nVerts, Vertex verts[], size_t nIndices, Uint32 indices[]);
VlkImage       vkeUploadImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipMapped);
VlkImage       vkeCreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipMapped);
VlkBuffer      vkeCreateBufferMapped(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
#ifdef __cplusplus
extern "C" {
#endif
void cppImguiShutdown();
void cppImguiProcessEvent(SDL_Event* evt);
void cppImguiRender();
void cppImguiDraw(VkCommandBuffer cmdBuf);
void cppImguiInit(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, VkDevice device, VkQueue queue,
                  VkDescriptorPool pool, VkFormat swapchainImageFormat);
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


#define MAX(n1, n2) ((n2 > n1) ? n2 : n1)
