#define CGLTF_IMPLEMENTATION
#include "./vkguide.h"


extern VulkanEngine vke;


MeshAsset* vkeLoadGlb(char* filePath) {
  cgltf_options options = {.type = cgltf_file_type_glb};
  cgltf_data*   data    = nullptr;
  cgltf_result  result  = cgltf_parse_file(&options, "scene.gltf", &data);
  if (result != cgltf_result_success) {
    SDL_Log("%d\n", result);
    exit(1);
  }

  MeshAsset* ret = calloc(data->meshes_count, sizeof(MeshAsset));
  for (size_t i = 0; i < data->meshes_count; i++) {
    auto mesh = &data->meshes[i];
    auto this = &ret[i];
    for (size_t j = 0; j < mesh->primitives_count; j++) {
      auto       prim         = mesh->primitives[i];
      GeoSurface this_surface = {};
    }
  }

  return ret;
}
