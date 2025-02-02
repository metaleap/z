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

  MeshAsset* ret   = calloc(data->meshes_count, sizeof(MeshAsset));
  List       verts = List_new(128);
  List       idxs  = List_new(128);
  for (size_t i = 0; i < data->meshes_count; i++) {
    auto mesh      = &data->meshes[i];
    auto new_mesh  = &ret[i];
    new_mesh->name = mesh->name;
    List_clear(&verts);
    List_clear(&idxs);
    for (size_t j = 0; j < mesh->primitives_count; j++) {
      auto       prim         = mesh->primitives[i];
      GeoSurface new_surface  = {.idxStart = idxs.len, .count = prim.indices->count};
      size_t     initial_vert = verts.len;
      {   // load indices
      }
    }
  }

  return ret;
}
