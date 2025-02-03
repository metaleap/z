#include <stdio.h>

#include "./vkguide.h"
#include <SDL3/SDL_main.h>


int main(int argc, char* argv[]) {
  SDL_Log("—init...\n");
  vke = (VulkanEngine) {
      .windowExtent = {.width = 1600, .height = 900}
  };
  // vkeLoadGlb("../../vkguide/assets/structure.glb");
  vkeLoadGlb("../../vkguide/assets/basicmesh.glb");
  vkeInit();
  vkeRun();
  vkeShutdown();
  SDL_Quit();
  return 0;
}


List List_new(Uint32 cap) {
  return (List) {.buf = (void**) malloc(sizeof(void*) * cap), .cap = cap};
}


void List_clear(List* self) {
  self->len = 0;
}


void List_append(List* self, void* item) {
  if (self->len == self->cap) {
    self->cap *= 2;
    self->buf  = (void**) realloc(self->buf, sizeof(void*) * self->cap);
  }
  self->buf[self->len] = item;
  self->len++;
}
