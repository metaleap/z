#include "cglm/types-struct.h"
#include <stddef.h>
#define CGLTF_IMPLEMENTATION
#include "./vkguide.h"


extern VulkanEngine vke;


MeshAsset* vkeLoadGlb(char* filePath) {
  {   // TEMP CUBE

    Vertex tlf = {
        .position = {.x = -1, .y = 1, .z = -1},
        .color    = {.r = 1, .g = 1, .b = 1, .a = 1},
        .normal   = {.x = 0, .y = 0, .z = -1},
        .uv_x     = 0,
        .uv_y     = 0
    };
    Vertex trf = {
        .position = {.x = 1, .y = 1, .z = -1},
        .color    = {.r = 0, .g = 1, .b = 1, .a = 1},
        .normal   = {.x = 0, .y = 0, .z = -1},
        .uv_x     = 0,
        .uv_y     = 0
    };
  }


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
      auto prim = &mesh->primitives[i_prim];

      cgltf_accessor* acc_indices      = prim->indices;
      cgltf_accessor* acc_positions    = nullptr;
      cgltf_accessor* acc_normals      = nullptr;
      cgltf_accessor* acc_colors       = nullptr;
      cgltf_accessor* acc_texcoords    = nullptr;
      float*          floats_positions = nullptr;
      float*          floats_normals   = nullptr;
      float*          floats_colors    = nullptr;
      float*          floats_texcoords = nullptr;
      size_t          count_positions  = 0;
      size_t          count_normals    = 0;
      size_t          count_colors     = 0;
      size_t          count_texcoords  = 0;
      size_t          stride_positions = 0;
      size_t          stride_normals   = 0;
      size_t          stride_colors    = 0;
      size_t          stride_texcoords = 0;
      for (size_t i_attr = 0; (i_attr < prim->attributes_count); i_attr++) {
        auto attr     = &prim->attributes[i_attr];
        auto acc      = attr->data;
        auto buf_view = acc->buffer_view;
        auto buf      = buf_view->buffer;
        auto off      = buf + acc->offset + buf_view->offset;
        switch (attr->type) {
          case cgltf_attribute_type_position:
            acc_positions    = acc;
            count_positions  = acc->count;
            stride_positions = utilMax(buf_view->stride, 3 * sizeof(float));
            break;
          case cgltf_attribute_type_normal:
            acc_normals    = acc;
            count_normals  = acc->count;
            stride_normals = utilMax(buf_view->stride, 3 * sizeof(float));
            break;
          case cgltf_attribute_type_texcoord:
            acc_texcoords    = acc;
            count_texcoords  = acc->count;
            stride_texcoords = utilMax(buf_view->stride, 2 * sizeof(float));
            break;
          case cgltf_attribute_type_color:
            acc_colors    = acc;
            count_colors  = acc->count;
            stride_colors = utilMax(buf_view->stride, 3 * sizeof(float));
            break;
        }
      }
      assert((acc_indices != nullptr) && (acc_positions != nullptr) && (acc_texcoords != nullptr) &&
             (acc_normals != nullptr));
      bool no_colors = ((acc_colors == nullptr) || (acc_colors->count == 0) || (count_colors == 0));

      size_t count_verts   = count_positions;
      size_t cur_positions = 0;
      size_t cur_normals   = 0;
      size_t cur_texcoords = 0;
      size_t cur_colors    = 0;
      for (size_t i_vtx = 0; i_vtx < count_verts; i_vtx++) {
        Vertex vert = {
            .position = {.x = floats_positions[cur_positions + 0],
                         .y = floats_positions[cur_positions + 1],
                         .z = floats_positions[cur_positions + 2]},
            .normal   = {.x = floats_normals[cur_normals + 0],
                         .y = floats_normals[cur_normals + 1],
                         .z = floats_normals[cur_normals + 2]},
            .uv_x     = floats_texcoords[cur_texcoords + 0],
            .uv_y     = floats_texcoords[cur_texcoords + 1],
            .color    = {.r = 1, .g = 1, .b = 1, .a = 1}
        };
        cur_positions += (stride_positions / sizeof(float));
        cur_normals   += (stride_normals / sizeof(float));
        cur_texcoords += (stride_texcoords / sizeof(float));
        if (!no_colors) {
          vert.color  = (vec4s) {.r = floats_colors[cur_colors + 0],
                                 .g = floats_colors[cur_colors + 1],
                                 .b = floats_colors[cur_colors + 2],
                                 .a = 1};
          cur_colors += (stride_colors / sizeof(float));
        }
        Verts_add(&vertices, vert);
      }
    }
    break;   // temporarily, only the cube
  }

  return ret;
}
