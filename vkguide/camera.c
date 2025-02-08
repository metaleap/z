#include "./vkguide.h"
#include <SDL3/SDL_events.h>


mat4s Camera_getViewMatrix(Camera*) {
}



mat4s Camera_getRotationMatrix(Camera*) {
}



void Camera_processEvent(Camera* this, SDL_Event* evt) {
  if (evt->type == SDL_EVENT_MOUSE_MOTION) {
    this->yaw   += evt->motion.xrel / 200.0f;
    this->pitch -= evt->motion.yrel / 200.0f;

  } else if (evt->type == SDL_EVENT_KEY_DOWN) {
    if (evt->key.key == SDLK_W)
      this->velocity.z = -1;
    if (evt->key.key == SDLK_S)
      this->velocity.z = 1;
    if (evt->key.key == SDLK_A)
      this->velocity.x = -1;
    if (evt->key.key == SDLK_D)
      this->velocity.x = 1;

  } else if (evt->type == SDL_EVENT_KEY_UP) {
    if (evt->key.key == SDLK_W)
      this->velocity.z = 0;
    if (evt->key.key == SDLK_S)
      this->velocity.z = 0;
    if (evt->key.key == SDLK_A)
      this->velocity.x = 0;
    if (evt->key.key == SDLK_D)
      this->velocity.x = 0;
  }
}



void Camera_update(Camera* this) {
  mat4s rot     = Camera_getRotationMatrix(this);
  vec4s move_by = mat4_mulv(
      rot, (vec4s) {.x = this->velocity.x * 0.5f, .y = this->velocity.y * 0.5f, .z = this->velocity.z * 0.5f, 0});
  this->position = vec3_add(this->position, (vec3s) {.x = move_by.x, .y = move_by.y, .z = move_by.z});
}
