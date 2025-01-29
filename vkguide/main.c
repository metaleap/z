#include <SDL.h>
#include <stdio.h>

#include "./vkguide.h"


VulkanEngine vke = {
    .window_extent = {.width = 1600, .height = 900}
};



int main() {
  vke_init();
  vke_run();

  vke_cleanup();
  SDL_Quit();
  return 0;
}
