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
  for (size_t i = 0; i < data->meshes_count; i++)
    GeoSurfaces_init_capacity(&ret[i].surfaces, 16);
  U32s indices = {};
  assert(U32s_init_capacity(&indices, 128));
  Verts vertices = {};
  assert(Verts_init_capacity(&vertices, 128));

  for (size_t i_mesh = 0; i_mesh < data->meshes_count; i_mesh++) {
    auto mesh      = &data->meshes[i_mesh];
    auto new_mesh  = &ret[i_mesh];
    new_mesh->name = mesh->name;
    U32s_clear(&indices);
    Verts_clear(&vertices);
    for (size_t i_prim = 0; i_prim < mesh->primitives_count; i_prim++) {
      auto            prim        = &mesh->primitives[i_prim];
      cgltf_accessor* acc_indices = prim->indices;
      assert(acc_indices != nullptr);
      GeoSurface new_surface = {.idxStart = (Uint32) indices.count, .count = (Uint64) acc_indices->count};
      size_t     initial_vtx = vertices.count;

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
        auto buf      = buf_view->buffer->data;
        auto off      = buf + acc->offset + buf_view->offset;
        switch (attr->type) {
          case cgltf_attribute_type_position:
            acc_positions    = acc;
            count_positions  = acc->count;
            stride_positions = utilMax(buf_view->stride, 3 * sizeof(float));
            floats_positions = (float*) off;
            break;
          case cgltf_attribute_type_normal:
            acc_normals    = acc;
            count_normals  = acc->count;
            stride_normals = utilMax(buf_view->stride, 3 * sizeof(float));
            floats_normals = (float*) off;
            break;
          case cgltf_attribute_type_texcoord:
            acc_texcoords    = acc;
            count_texcoords  = acc->count;
            stride_texcoords = utilMax(buf_view->stride, 2 * sizeof(float));
            floats_texcoords = (float*) off;
            break;
          case cgltf_attribute_type_color:
            acc_colors    = acc;
            count_colors  = acc->count;
            stride_colors = utilMax(buf_view->stride, 3 * sizeof(float));
            floats_colors = (float*) off;
            break;
        }
      }
      assert((acc_positions != nullptr) && (acc_texcoords != nullptr) && (acc_normals != nullptr) &&
             (floats_normals != nullptr) && (floats_positions != nullptr) && (floats_texcoords != nullptr) &&
             (count_normals != 0) && (count_positions != 0) && (count_texcoords != 0));
      bool no_colors =
          ((acc_colors == nullptr) || (acc_colors->count == 0) || (count_colors == 0) || (floats_colors == nullptr));

      // 1: vertex attrs
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
        assert(Verts_add(&vertices, vert));
      }

      // 2: indices
      auto buf_view = acc_indices->buffer_view;
      auto buf      = buf_view->buffer->data;
      switch (acc_indices->component_type) {
        case cgltf_component_type_r_8:
          {
            Sint8* idx = buf + acc_indices->offset + buf_view->offset;
            for (size_t i_idx = 0; i_idx < acc_indices->count; i_idx++)
              assert(U32s_add(&indices, (Uint32) initial_vtx + (Uint32) idx[i_idx]));
            break;
          }
        case cgltf_component_type_r_8u:
          {
            Uint8* idx = buf + acc_indices->offset + buf_view->offset;
            for (size_t i_idx = 0; i_idx < acc_indices->count; i_idx++)
              assert(U32s_add(&indices, (Uint32) initial_vtx + (Uint32) idx[i_idx]));
            break;
          }
        case cgltf_component_type_r_16:
          {
            Sint16* idx = buf + acc_indices->offset + buf_view->offset;
            for (size_t i_idx = 0; i_idx < acc_indices->count; i_idx++)
              assert(U32s_add(&indices, (Uint32) initial_vtx + (Uint32) idx[i_idx]));
            break;
          }
        case cgltf_component_type_r_16u:
          {
            Uint16* idx = buf + acc_indices->offset + buf_view->offset;
            for (size_t i_idx = 0; i_idx < acc_indices->count; i_idx++)
              assert(U32s_add(&indices, (Uint32) initial_vtx + (Uint32) idx[i_idx]));
            break;
          }
        case cgltf_component_type_r_32u:
          {
            Uint32* idx = buf + acc_indices->offset + buf_view->offset;
            for (size_t i_idx = 0; i_idx < acc_indices->count; i_idx++)
              assert(U32s_add(&indices, (Uint32) initial_vtx + (Uint32) idx[i_idx]));
            break;
          }
        case cgltf_component_type_r_32f:
          {
            float* idx = buf + acc_indices->offset + buf_view->offset;
            for (size_t i_idx = 0; i_idx < acc_indices->count; i_idx++)
              assert(U32s_add(&indices, (Uint32) initial_vtx + (Uint32) idx[i_idx]));
            break;
          }
      }
      assert(GeoSurfaces_add(&new_mesh->surfaces, new_surface));
    }

    if (true)
      for (size_t i_vtx = 0; i_vtx < vertices.count; i_vtx++) {
        auto vert   = &vertices.buffer[i_vtx];
        vert->color = (vec4s) {.r = vert->normal.x, .g = vert->normal.y, .b = vert->normal.z, .a = 1};
      }
    new_mesh->meshBuffers = vkeUploadMesh(vertices.count, vertices.buffer, indices.count, indices.buffer);
    disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, new_mesh->meshBuffers.indexBuffer.buf,
                   new_mesh->meshBuffers.indexBuffer.alloc);
    disposals_push(&vke.disposals, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, new_mesh->meshBuffers.vertexBuffer.buf,
                   new_mesh->meshBuffers.vertexBuffer.alloc);
  }

  return ret;
}
