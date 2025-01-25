#include <assert.h>
#include <math.h>

#include "./fged.h"



inline Vec4 Vec4_new(float x, float y, float z, float w) {
  return (Vec4) {
      .x = x,
      .y = y,
      .z = z,
      .w = w,
  };
}

inline Vec4 Vec4_neg(Vec4* v) {
  return (Vec4) {
      .x = (-v->x),
      .y = (-v->y),
      .z = (-v->z),
      .w = (-v->w),
  };
}

inline Vec4 Vec4_mul(Vec4* v, float n) {
  return (Vec4) {
      .x = (v->x * n),
      .y = (v->y * n),
      .z = (v->z * n),
      .w = (v->w * n),
  };
}

inline Vec4 Vec4_div(Vec4* v, float n) {
  assert(n != 0.0f);
  return (Vec4) {
      .x = (v->x / n),
      .y = (v->y / n),
      .z = (v->z / n),
      .w = (v->w / n),
  };
}

inline Vec4 Vec4_add(Vec4* v1, Vec4* v2) {
  return (Vec4) {
      .x = (v1->x + v2->x),
      .y = (v1->y + v2->y),
      .z = (v1->z + v2->z),
      .w = (v1->w + v2->w),
  };
}

inline Vec4 Vec4_sub(Vec4* v1, Vec4* v2) {
  return (Vec4) {
      .x = (v1->x - v2->x),
      .y = (v1->y - v2->y),
      .z = (v1->z - v2->z),
      .w = (v1->w - v2->w),
  };
}

inline float Vec4_len(Vec4* v) {
  return sqrtf((v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w));
}

inline Vec4 Vec4_norm(Vec4* v) {
  return Vec4_div(v, Vec4_len(v));
}

inline float Vec4_dot(Vec4* v1, Vec4* v2) {
  return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z) + (v1->w * v2->w));
}

inline Vec4 Vec4_project(Vec4* a, Vec4* b) {
  return Vec4_mul(b, Vec4_dot(a, b) / Vec4_dot(b, b));
}

inline Vec4 Vec4_reject(Vec4* a, Vec4* b) {
  Vec4 proj = Vec4_project(a, b);
  return Vec4_sub(a, &proj);
}
