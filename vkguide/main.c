#include <SDL.h>
#include <stdio.h>

#include "./vkguide.h"



int main() {
  vke = (VulkanEngine) {
      .windowExtent = {.width = 1600, .height = 900}
  };
  vkeInit();
  vkeRun();
  vkeShutdown();
  SDL_Quit();
  return 0;
}
