#include "./vkguide.h"


mat4s Camera_getViewMatrix(Camera* this) {
  // to create a correct model view, we need to move the world in opposite direction to the camera
  //  so we will create the camera model matrix and invert
  mat4s trans = glms_translate(mat4_identity(), this->position);
  mat4s rot   = Camera_getRotationMatrix(this);
  return mat4_inv(mat4_mul(trans, rot));
}


mat4s Camera_getRotationMatrix(Camera* this) {
  // fairly typical FPS style camera. we join the pitch and yaw rotations into the final rotation matrix

  // TODO: cglm makes this too verbose, the GLM equivalent code is:
  // glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3 { 1.f, 0.f, 0.f });
  // glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3 { 0.f, -1.f, 0.f });
  // return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);

  versor pitch_rot, yaw_rot;
  float  pitch_axis[3] = {1, 0, 0};
  float  yaw_axis[3]   = {0, -1, 0};
  glm_quatv(pitch_rot, this->pitch, pitch_axis);
  glm_quatv(yaw_rot, this->yaw, yaw_axis);
  mat4 pitch_mat, yaw_mat;
  glm_quat_mat4(yaw_rot, yaw_mat);
  glm_quat_mat4(pitch_rot, pitch_mat);
  mat4s ret;
  glm_mat4_mul(yaw_mat, pitch_mat, ret.raw);
  return ret;
}



void Camera_processEvent(Camera* this, SDL_Event* evt) {
  if (evt->type == SDL_EVENT_MOUSE_MOTION) {
    this->yaw   += evt->motion.xrel / 123.45f;
    this->pitch -= evt->motion.yrel / 123.45f;

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
