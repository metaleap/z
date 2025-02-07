#include "./vkguide.h"


LIST_DEFINE_C(RenderObjects, RenderObjects, RenderObject);
LIST_DEFINE_C(GeoSurfaces, GeoSurfaces, GeoSurface);
LIST_DEFINE_C(MeshAssets, MeshAssets, MeshAsset);
LIST_DEFINE_C(SceneNodes, SceneNodes, SceneNode);



void SceneNode_draw(SceneNode* this, mat4s* topMatrix, DrawContext* ctx) {
  if (this->mesh != nullptr) {
    mat4s node_matrix = mat4_mul(*topMatrix, this->worldTransform);
    for (size_t i = 0; i < this->mesh->surfaces.count; i++) {
      GeoSurface*  surface = &this->mesh->surfaces.buffer[i];
      RenderObject draw    = {.indexCount          = surface->count,
                              .firstIndex          = surface->idxStart,
                              .indexBuffer         = this->mesh->meshBuffers.indexBuffer.buf,
                              .vertexBufferAddress = this->mesh->meshBuffers.vertexBufferAddress,
                              .material            = &surface->material.data,
                              .transform           = node_matrix};
      RenderObjects_addAligned(&ctx->opaqueSurfaces, draw);
    }
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
