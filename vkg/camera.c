#include "./vkg.h"


mat4s Camera_getViewMatrix(Camera* this) {
  // to create a correct model view, we need to move the world in opposite direction to the camera
  //  so we will create the camera model matrix and invert
  mat4s trans = glms_translate(mat4_identity(), this->position);
  mat4s rot   = Camera_getRotationMatrix(this);
  return mat4_inv(mat4_mul(trans, rot));
}


mat4s Camera_getRotationMatrix(Camera* this) {
  // fairly typical FPS style camera. we join the pitch and yaw rotations into the final rotation matrix
  versors pitch_rot = glms_quatv(this->pitch, GLMS_XUP);   // quatv aka GLM's angleAxis
  versors yaw_rot   = glms_quatv(this->yaw, (vec3s) {.x = 0, .y = -1, .z = 0});
  return mat4_mul(quat_mat4(yaw_rot), quat_mat4(pitch_rot));
}



void Camera_processEvent(Camera* this, SDL_Event* evt) {
  if (evt->type == SDL_EVENT_MOUSE_MOTION) {
    this->yaw   += evt->motion.xrel / 123.45f;
    this->pitch  = glm_clamp(this->pitch - (evt->motion.yrel / 123.45f), glm_rad(-77), glm_rad(77));

  } else if (evt->type == SDL_EVENT_KEY_DOWN) {
    if (evt->key.key == SDLK_W)
      this->velocity.z = -1;
    if (evt->key.key == SDLK_S)
      this->velocity.z = 1;
    if (evt->key.key == SDLK_Q)
      this->velocity.y = -1;
    if (evt->key.key == SDLK_E)
      this->velocity.y = 1;
    if (evt->key.key == SDLK_A)
      this->velocity.x = -1;
    if (evt->key.key == SDLK_D)
      this->velocity.x = 1;

  } else if (evt->type == SDL_EVENT_KEY_UP) {
    if (evt->key.key == SDLK_W)
      this->velocity.z = 0;
    if (evt->key.key == SDLK_S)
      this->velocity.z = 0;
    if (evt->key.key == SDLK_Q)
      this->velocity.y = 0;
    if (evt->key.key == SDLK_E)
      this->velocity.y = 0;
    if (evt->key.key == SDLK_A)
      this->velocity.x = 0;
    if (evt->key.key == SDLK_D)
      this->velocity.x = 0;
  }
}



void Camera_update(Camera* this) {
  mat4s cam_rot  = Camera_getRotationMatrix(this);
  vec4s move_by  = mat4_mulv(cam_rot, (vec4s) {.x = this->velocity.x * this->moveSpeed,
                                               .y = this->velocity.y * this->moveSpeed,
                                               .z = this->velocity.z * this->moveSpeed,
                                               0});
  this->position = vec3_add(this->position, (vec3s) {.x = move_by.x, .y = move_by.y, .z = move_by.z});
}
