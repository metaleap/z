#define CGLTF_IMPLEMENTATION
#include "./vkguide.h"


extern VulkanEngine vke;


MeshAsset* vkeLoadGlb(char* filePath) {
  cgltf_options options = {.type = cgltf_file_type_glb};
  cgltf_data*   data    = nullptr;
  cgltf_result  result  = cgltf_parse_file(&options, filePath, &data);
  if (result == cgltf_result_success)
    result = cgltf_load_buffers(&options, data, nullptr);
  if (result != cgltf_result_success) {
    SDL_Log("glTF load failure %d\n", result);
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
      auto            prim          = &mesh->primitives[i_prim];
      cgltf_accessor* acc_indices   = prim->indices;
      cgltf_accessor* acc_positions = nullptr;
      cgltf_accessor* acc_normals   = nullptr;
      cgltf_accessor* acc_colors    = nullptr;
      cgltf_accessor* acc_texcoords = nullptr;
      // 1: gather the above accessors first
      for (size_t i_attr = 0; (i_attr < prim->attributes_count); i_attr++) {
        auto attr = &prim->attributes[i_attr];
        SDL_Log("  p%zu[a%zu'%s']=%d (%zu)\n", i_prim, i_attr, attr->name, attr->type, attr->data->count);
        switch (attr->type) {
          case cgltf_attribute_type_position:
            acc_positions = attr->data;
            break;
          case cgltf_attribute_type_normal:
            acc_normals = attr->data;
            break;
          case cgltf_attribute_type_texcoord:
            acc_texcoords = attr->data;
            break;
          case cgltf_attribute_type_color:
            acc_colors = attr->data;
            break;
        }
      }
      assert((acc_indices != nullptr) && (acc_positions != nullptr) && (acc_texcoords != nullptr) &&
             (acc_normals != nullptr));
      // 2: draw the rest of the friggin owl
      int* indices = calloc(acc_indices->count, sizeof(int));
      cgltf_accessor_unpack_indices(acc_indices, indices, cgltf_component_size(acc_indices->component_type),
                                    acc_indices->count);
      float* positions = calloc(acc_positions->count, sizeof(float));
      cgltf_accessor_unpack_floats(acc_positions, positions, acc_positions->count);
      float* normals = calloc(acc_normals->count, sizeof(float));
      cgltf_accessor_unpack_floats(acc_normals, normals, acc_normals->count);
      float* texcoords = calloc(acc_texcoords->count, sizeof(float));
      cgltf_accessor_unpack_floats(acc_texcoords, texcoords, acc_texcoords->count);
      float* colors = (acc_colors == nullptr) ? nullptr : calloc(acc_colors->count, sizeof(float));
      (acc_colors == nullptr) ? 0 : cgltf_accessor_unpack_floats(acc_colors, colors, acc_colors->count);
      SDL_Log("P%zu | C%zu | N%zu | T%zu | I%zu", acc_positions->count, (acc_colors == nullptr) ? 0 : acc_colors->count,
              acc_normals->count, acc_texcoords->count, acc_indices->count);
      // if (index_accessor->component_type == cgltf_component_type_r_16u)
      // {
      //   for (int indice=0;indice <index_accessor->count ;indice  ++)
      //   {
      //     unsigned short t_indice = cgltf_accessor_read_index(index_accessor,indice);
      //      std::cout << "vertex1 " << *(position_data+t_indice) << " / " << *(position_data+t_indice+1) << " / " <<
      //      *(position_data+t_indice+2) << std::endl;
      //   }
      // }
    }
  }

  return ret;
}
