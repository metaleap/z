#include <stdio.h>

#include <SDL3/SDL_main.h>
#include "./vkguide.h"


LIST_DEFINE_C(U32s, U32s, Uint32);
LIST_DEFINE_C(Verts, Verts, Vertex);
LIST_DEFINE_C(GeoSurfaces, GeoSurfaces, GeoSurface);
LIST_DEFINE_C(MeshAssets, MeshAssets, MeshAsset);
LIST_DEFINE_C(VlkDescriptorAllocatorSizeRatios, VlkDescriptorAllocatorSizeRatios, VlkDescriptorAllocatorSizeRatio);
LIST_DEFINE_C(VkDescriptorPoolSizes, VkDescriptorPoolSizes, VkDescriptorPoolSize);
LIST_DEFINE_C(VkDescriptorPools, VkDescriptorPools, VkDescriptorPool);
LIST_DEFINE_C(VkDescriptorImageInfos, VkDescriptorImageInfos, VkDescriptorImageInfo);
LIST_DEFINE_C(VkDescriptorBufferInfos, VkDescriptorBufferInfos, VkDescriptorBufferInfo);
LIST_DEFINE_C(VkWriteDescriptorSets, VkWriteDescriptorSets, VkWriteDescriptorSet);


VulkanEngine vke = {
    .windowExtent = {
                     .width  = 1600,
                     .height = 900,
                     }
};


int main(int argc, char* argv[]) {
  SDL_Log("—init...\n");
  vkeInit();
  vke.testMeshes = vkeLoadGlb("../../vkguide/assets/basicmesh.glb");
  vkeRun();
  vkeShutdown();
  SDL_Quit();
  return 0;
}


size_t utilMax(size_t s1, size_t s2) {
  return (s1 > s2) ? s1 : s2;
}
