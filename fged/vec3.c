#include <assert.h>
#include <math.h>

#include "./fged.h"



inline Vec3 Vec3_new(float x, float y, float z) {
  return (Vec3) {
      .x = x,
      .y = y,
      .z = z,
  };
}

inline Vec3 Vec3_neg(Vec3* v) {
  return (Vec3) {
      .x = (-v->x),
      .y = (-v->y),
      .z = (-v->z),
  };
}

inline Vec3 Vec3_mul(Vec3* v, float n) {
  return (Vec3) {
      .x = (v->x * n),
      .y = (v->y * n),
      .z = (v->z * n),
  };
}

inline Vec3 Vec3_div(Vec3* v, float n) {
  assert(n != 0.0f);
  return (Vec3) {
      .x = (v->x / n),
      .y = (v->y / n),
      .z = (v->z / n),
  };
}

inline Vec3 Vec3_add(Vec3* v1, Vec3* v2) {
  return (Vec3) {
      .x = (v1->x + v2->x),
      .y = (v1->y + v2->y),
      .z = (v1->z + v2->z),
  };
}

inline Vec3 Vec3_sub(Vec3* v1, Vec3* v2) {
  return (Vec3) {
      .x = (v1->x - v2->x),
      .y = (v1->y - v2->y),
      .z = (v1->z - v2->z),
  };
}

inline float Vec3_len(Vec3* v) {
  return sqrtf((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

inline Vec3 Vec3_norm(Vec3* v) {
  return Vec3_div(v, Vec3_len(v));
}

inline float Vec3_dot(Vec3* v1, Vec3* v2) {
  return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z));
}

inline Vec3 Vec3_cross(Vec3* v1, Vec3* v2) {
  return (Vec3) {
      .x = ((v1->y * v2->z) - (v1->z * v2->y)),
      .y = ((v1->z * v2->x) - (v1->x * v2->z)),
      .z = ((v1->x * v2->y) - (v1->y * v2->x)),
  };
}

inline Vec3 Vec3_project(Vec3* a, Vec3* b) {
  return Vec3_mul(b, Vec3_dot(a, b) / Vec3_dot(b, b));
}

inline Vec3 Vec3_reject(Vec3* a, Vec3* b) {
  Vec3 proj = Vec3_project(a, b);
  return Vec3_sub(a, &proj);
}
