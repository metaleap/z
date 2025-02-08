#include "./vkguide.h"
#include "cglm/struct/quat.h"


LIST_DEFINE_C(RenderObjects, RenderObjects, RenderObject);
LIST_DEFINE_C(GeoSurfaces, GeoSurfaces, GeoSurface);
LIST_DEFINE_C(MeshAssets, MeshAssets, MeshAsset);
LIST_DEFINE_C(SceneNodes, SceneNodes, SceneNode);



void MeshAsset_draw(MeshAsset* this, mat4s* transform, DrawContext* ctx) {
  for (size_t i = 0; i < this->surfaces.count; i++) {
    GeoSurface*  surface = &this->surfaces.buffer[i];
    RenderObject draw    = {.indexCount          = surface->count,
                            .firstIndex          = surface->idxStart,
                            .indexBuffer         = this->meshBuffers.indexBuffer.buf,
                            .vertexBufferAddress = this->meshBuffers.vertexBufferAddress,
                            .material            = &surface->material.data,
                            .transform           = *transform};
    RenderObjects_addAligned(&ctx->opaqueSurfaces, draw);
  }
}



void SceneNode_draw(SceneNode* this, mat4s* topMatrix, DrawContext* ctx) {
  if (this->mesh != nullptr) {
    mat4s node_matrix = mat4_mul(*topMatrix, this->worldTransform);
    MeshAsset_draw(this->mesh, &node_matrix, ctx);
  }
  if (this->children != nullptr)
    for (size_t i = 0; i < this->children->count; i++)
      SceneNode_draw(&this->children->buffer[i], topMatrix, ctx);
}



void SceneNode_refreshTransform(SceneNode* this, mat4s* parentWorldTransform) {
  this->worldTransform = mat4_mul(*parentWorldTransform, this->localTransform);
  if (this->children != nullptr)
    for (size_t i = 0; i < this->children->count; i++)
      SceneNode_refreshTransform(&this->children->buffer[i], &this->worldTransform);
}
