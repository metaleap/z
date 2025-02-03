#define CGLTF_IMPLEMENTATION
#include "./vkguide.h"


extern VulkanEngine vke;


MeshAsset* vkeLoadGlb(char* filePath) {
  cgltf_options options = {.type = cgltf_file_type_glb};
  cgltf_data*   data    = nullptr;
  cgltf_result  result  = cgltf_parse_file(&options, filePath, &data);
  if (result != cgltf_result_success) {
    SDL_Log("%d\n", result);
    exit(1);
  }

  MeshAsset* ret   = calloc(data->meshes_count, sizeof(MeshAsset));
  List       verts = List_new(128);
  List       idxs  = List_new(128);
  for (size_t i_mesh = 0; i_mesh < data->meshes_count; i_mesh++) {
    auto mesh      = &data->meshes[i_mesh];
    auto new_mesh  = &ret[i_mesh];
    new_mesh->name = mesh->name;
    List_clear(&verts);
    List_clear(&idxs);
    for (size_t i_prim = 0; i_prim < mesh->primitives_count; i_prim++) {
      auto prim = &mesh->primitives[i_prim];
      if (prim->type != cgltf_primitive_type_triangles)
        continue;
      auto acc_idxs = prim->indices;
      for (size_t i_attr = 0; i_attr < prim->attributes_count; i_attr++) {
        auto attr = &prim->attributes[i_attr];
        switch (attr->type) {
          default:
            break;
        }
      }
    }
  }

  return ret;
}
