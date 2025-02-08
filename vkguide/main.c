#include <stdio.h>

#include <SDL3/SDL_main.h>
#include "./vkguide.h"
#include "cglm/quat.h"
#include "cglm/struct/quat.h"
#include "cglm/struct/vec3.h"
#include "cglm/types.h"


LIST_DEFINE_C(U32s, U32s, Uint32);
LIST_DEFINE_C(Verts, Verts, Vertex);


VulkanEngine vke = {
    .windowExtent = {
                     .width  = 1600,
                     .height = 900,
                     }
};


int main(int argc, char* argv[]) {
  SDL_Log("—init...\n");
  vkeInit();
  vkeRun();
  vkeShutdown();
  SDL_Quit();
  return 0;
}


size_t utilMax(size_t s1, size_t s2) {
  return (s1 > s2) ? s1 : s2;
}
