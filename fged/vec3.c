#include <assert.h>
#include <math.h>
#include <stddef.h>

#include "./fged.h"



inline Vec3 Vec3_new(float x, float y, float z) {
  return (Vec3) {.x = x, .y = y, .z = z};
}

inline Vec3 Vec3_neg(Vec3* v) {
  return (Vec3) {.x = (-v->x), .y = (-v->y), .z = (-v->z)};
}

inline Vec3 Vec3_mul(Vec3* v, float n) {
  return (Vec3) {.x = (v->x * n), .y = (v->y * n), .z = (v->z * n)};
}

inline Vec3 Vec3_div(Vec3* v, float n) {
  assert(n != 0.0f);
  return (Vec3) {.x = (v->x / n), .y = (v->y / n), .z = (v->z / n)};
}

inline Vec3 Vec3_add(Vec3* v1, Vec3* v2) {
  return (Vec3) {.x = (v1->x + v2->x), .y = (v1->y + v2->y), .z = (v1->z + v2->z)};
}

inline Vec3 Vec3_sub(Vec3* v1, Vec3* v2) {
  return (Vec3) {.x = (v1->x - v2->x), .y = (v1->y - v2->y), .z = (v1->z - v2->z)};
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



inline Mat3 Mat3_new(Vec3 col0, Vec3 col1, Vec3 col2) {
  return (Mat3) {
      .cols = {col0, col1, col2}
  };
}

inline float Mat3_at(Mat3* mat, size_t idxRow, size_t idxCol) {
  return mat->at[idxCol][idxRow];
}

inline Mat3 Mat3_neg(Mat3* m) {
  return Mat3_new(Vec3_neg(&m->cols[0]), Vec3_neg(&m->cols[1]), Vec3_neg(&m->cols[2]));
}

inline Mat3 Mat3_mul(Mat3* m, float n) {
  return Mat3_new(Vec3_mul(&m->cols[0], n), Vec3_mul(&m->cols[1], n), Vec3_mul(&m->cols[2], n));
}

inline Mat3 Mat3_div(Mat3* m, float n) {
  return Mat3_new(Vec3_div(&m->cols[0], n), Vec3_div(&m->cols[1], n), Vec3_div(&m->cols[2], n));
}

inline Mat3 Mat3_add(Mat3* m1, Mat3* m2) {
  return Mat3_new(Vec3_add(&m1->cols[0], &m2->cols[0]), Vec3_add(&m1->cols[1], &m2->cols[1]), Vec3_add(&m1->cols[2], &m2->cols[2]));
}

inline Mat3 Mat3_sub(Mat3* m1, Mat3* m2) {
  return Mat3_new(Vec3_sub(&m1->cols[0], &m2->cols[0]), Vec3_sub(&m1->cols[1], &m2->cols[1]), Vec3_sub(&m1->cols[2], &m2->cols[2]));
}

inline Vec3 Mat3_mulVec(Mat3* m, Vec3* v) {
  return (Vec3) {
      .xyz = {
              ((v->x * Mat3_at(m, 0, 0)) + (v->y * Mat3_at(m, 0, 1)) + (v->z * Mat3_at(m, 0, 2))),
              ((v->x * Mat3_at(m, 1, 0)) + (v->y * Mat3_at(m, 1, 1)) + (v->z * Mat3_at(m, 1, 2))),
              ((v->x * Mat3_at(m, 2, 0)) + (v->y * Mat3_at(m, 2, 1)) + (v->z * Mat3_at(m, 2, 2))),
              }
  };
}

inline Mat3 Mat3_mulMat(Mat3* m1, Mat3* m2) {
  return (Mat3) {
      .at = {
             {((Mat3_at(m1, 0, 0) * Mat3_at(m2, 0, 0)) + (Mat3_at(m1, 0, 1) * Mat3_at(m2, 1, 0)) + (Mat3_at(m1, 0, 2) * Mat3_at(m2, 2, 0))),
             ((Mat3_at(m1, 0, 0) * Mat3_at(m2, 0, 1)) + (Mat3_at(m1, 0, 1) * Mat3_at(m2, 1, 1)) + (Mat3_at(m1, 0, 2) * Mat3_at(m2, 2, 1))),
             ((Mat3_at(m1, 0, 0) * Mat3_at(m2, 0, 2)) + (Mat3_at(m1, 0, 1) * Mat3_at(m2, 1, 2)) + (Mat3_at(m1, 0, 2) * Mat3_at(m2, 2, 2)))},
             {((Mat3_at(m1, 1, 0) * Mat3_at(m2, 0, 0)) + (Mat3_at(m1, 1, 1) * Mat3_at(m2, 1, 0)) + (Mat3_at(m1, 1, 2) * Mat3_at(m2, 2, 0))),
             ((Mat3_at(m1, 1, 0) * Mat3_at(m2, 0, 1)) + (Mat3_at(m1, 1, 1) * Mat3_at(m2, 1, 1)) + (Mat3_at(m1, 1, 2) * Mat3_at(m2, 2, 1))),
             ((Mat3_at(m1, 1, 0) * Mat3_at(m2, 0, 2)) + (Mat3_at(m1, 1, 1) * Mat3_at(m2, 1, 2)) + (Mat3_at(m1, 1, 2) * Mat3_at(m2, 2, 2)))},
             {((Mat3_at(m1, 2, 0) * Mat3_at(m2, 0, 0)) + (Mat3_at(m1, 2, 1) * Mat3_at(m2, 1, 0)) + (Mat3_at(m1, 2, 2) * Mat3_at(m2, 2, 0))),
             ((Mat3_at(m1, 2, 0) * Mat3_at(m2, 0, 1)) + (Mat3_at(m1, 2, 1) * Mat3_at(m2, 1, 1)) + (Mat3_at(m1, 2, 2) * Mat3_at(m2, 2, 1))),
             ((Mat3_at(m1, 2, 0) * Mat3_at(m2, 0, 2)) + (Mat3_at(m1, 2, 1) * Mat3_at(m2, 1, 2)) + (Mat3_at(m1, 2, 2) * Mat3_at(m2, 2, 2)))},
             },
  };
}
