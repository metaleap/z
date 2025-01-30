#include <SDL.h>
#include <stdio.h>

#include "./vkguide.h"





int main() {
  vkeInit();
  vkeRun();

  vkeShutdown();
  SDL_Quit();
  return 0;
}
