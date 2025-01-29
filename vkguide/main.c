#include <SDL.h>
#include <stdio.h>

#include "./vkguide.h"


VulkanEngine vke = {};



int main() {
  vkeInit();
  vkeRun();

  vkeDispose();
  SDL_Quit();
  return 0;
}
