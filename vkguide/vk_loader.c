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

  MeshAsset* ret = calloc(data->meshes_count, sizeof(MeshAsset));
  U32s       indices;
  U32s_init_capacity(&indices, 128);
  Verts vertices;
  Verts_init_capacity(&vertices, 128);

  for (size_t i_mesh = 0; i_mesh < data->meshes_count; i_mesh++) {
    auto mesh      = &data->meshes[i_mesh];
    auto new_mesh  = &ret[i_mesh];
    new_mesh->name = mesh->name;
    U32s_clear(&indices);
    Verts_clear(&vertices);
    for (size_t i_prim = 0; i_prim < mesh->primitives_count; i_prim++) {
      auto            prim          = &mesh->primitives[i_prim];
      // 1: gather all accessors first
      cgltf_accessor* acc_indices   = prim->indices;
      cgltf_accessor* acc_positions = nullptr;
      cgltf_accessor* acc_normals   = nullptr;
      cgltf_accessor* acc_colors    = nullptr;
      cgltf_accessor* acc_texcoords = nullptr;
      for (size_t i_attr = 0; (i_attr < prim->attributes_count); i_attr++) {
        auto attr = &prim->attributes[i_attr];
        SDL_Log("  p%zu[a%zu'%s'] = %d (%zu)\n", i_prim, i_attr, attr->name, attr->type, attr->data->count);
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
      bool no_colors   = ((acc_colors == nullptr) || (acc_colors->count <= 0));
      // 2: unpack accessors (load those float/int buffers)
      int* glb_indices = calloc(acc_indices->count, sizeof(int));
      auto len_indices = cgltf_accessor_unpack_indices(
          acc_indices, glb_indices, cgltf_component_size(acc_indices->component_type), acc_indices->count);
      float* glb_positions = calloc(acc_positions->count, sizeof(float));
      auto   len_positions = cgltf_accessor_unpack_floats(acc_positions, glb_positions, acc_positions->count);
      float* glb_normals   = calloc(acc_normals->count, sizeof(float));
      auto   len_normals   = cgltf_accessor_unpack_floats(acc_normals, glb_normals, acc_normals->count);
      float* glb_texcoords = calloc(acc_texcoords->count, sizeof(float));
      auto   len_texcoords = cgltf_accessor_unpack_floats(acc_texcoords, glb_texcoords, acc_texcoords->count);
      float* glb_colors    = no_colors ? nullptr : calloc(acc_colors->count, sizeof(float));
      auto   len_colors    = no_colors ? 0 : cgltf_accessor_unpack_floats(acc_colors, glb_colors, acc_colors->count);
      SDL_Log("P%zu | C%zu | N%zu | T%zu | I%zu", acc_positions->count, no_colors ? 0 : acc_colors->count,
              acc_normals->count, acc_texcoords->count, acc_indices->count);
      // 2: draw the rest of the friggin owl
      GeoSurface new_surface = {.idxStart = indices.count, .count = acc_indices->count};
      // 3: load indexes
      for (size_t i_idx = 0; i_idx < len_indices; i_idx++)
        U32s_add(&indices, glb_indices[i_idx]);
      // 4: load vertex positions, preset the other vert attrs
      for (size_t i_pos = 0; i_pos < len_positions; i_pos += 3)
        Verts_add(&vertices, (Vertex) {
                                 .position = (vec3s) {.x = glb_positions[i_pos + 0],
                                                      .y = glb_positions[i_pos + 1],
                                                      .z = glb_positions[i_pos + 2]},
                                 .normal   = (vec3s) {.x = 1, .y = 0, .z = 0},
                                 .color    = (vec4s) {.r = 1, .g = 1, .b = 1, .a = 1},
                                 .uv_x     = 0,
                                 .uv_y     = 0
        });

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
