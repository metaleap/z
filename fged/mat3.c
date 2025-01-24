#include "./fged.h"
#include <math.h>



inline Mat3 Mat3_newV(Vec3 col0, Vec3 col1, Vec3 col2) {
  return (Mat3) {
      .cols = {col0, col1, col2}
  };
}

inline Mat3 Mat3_newF(float n00, float n01, float n02, float n10, float n11, float n12, float n20, float n21, float n22) {
  Mat3 ret;
  ret.at[0][0] = n00;
  ret.at[0][1] = n10;
  ret.at[0][2] = n20;
  ret.at[1][0] = n01;
  ret.at[1][1] = n11;
  ret.at[1][2] = n21;
  ret.at[2][0] = n02;
  ret.at[2][1] = n12;
  ret.at[2][2] = n22;
  return ret;
}

inline Mat3 Mat3_rotX(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  return Mat3_newF(1, 0, 0, 0, c, -s, 0, s, c);
}

inline Mat3 Mat3_rotY(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  return Mat3_newF(c, 0, s, 0, 1, 0, -s, 0, c);
}

inline Mat3 Mat3_rotZ(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  return Mat3_newF(c, -s, 0, s, c, 0, 0, 0, 1);
}

inline Mat3 Mat3_identity() {
  return Mat3_newV(Vec3_new(1, 0, 0), Vec3_new(0, 1, 0), Vec3_new(0, 0, 1));
}

inline float Mat3_at(Mat3* m, size_t idxRow, size_t idxCol) {
  return m->at[idxCol][idxRow];
}

inline float Mat3_det(Mat3* m) {
  return ((Mat3_at(m, 0, 0) * Mat3_at(m, 1, 1) * Mat3_at(m, 2, 2)) - (Mat3_at(m, 1, 2) * Mat3_at(m, 2, 1)))
         + ((Mat3_at(m, 0, 1) * Mat3_at(m, 1, 2) * Mat3_at(m, 2, 0)) - (Mat3_at(m, 1, 0) * Mat3_at(m, 2, 2)))
         + ((Mat3_at(m, 0, 2) * Mat3_at(m, 1, 0) * Mat3_at(m, 2, 1)) - (Mat3_at(m, 1, 1) * Mat3_at(m, 2, 0)));
}

inline Mat3 Mat3_neg(Mat3* m) {
  return Mat3_newV(Vec3_neg(&m->cols[0]), Vec3_neg(&m->cols[1]), Vec3_neg(&m->cols[2]));
}

inline Mat3 Mat3_mulF(Mat3* m, float n) {
  return Mat3_newV(Vec3_mul(&m->cols[0], n), Vec3_mul(&m->cols[1], n), Vec3_mul(&m->cols[2], n));
}

inline Mat3 Mat3_div(Mat3* m, float n) {
  return Mat3_newV(Vec3_div(&m->cols[0], n), Vec3_div(&m->cols[1], n), Vec3_div(&m->cols[2], n));
}

inline Mat3 Mat3_add(Mat3* m1, Mat3* m2) {
  return Mat3_newV(Vec3_add(&m1->cols[0], &m2->cols[0]), Vec3_add(&m1->cols[1], &m2->cols[1]), Vec3_add(&m1->cols[2], &m2->cols[2]));
}

inline Mat3 Mat3_sub(Mat3* m1, Mat3* m2) {
  return Mat3_newV(Vec3_sub(&m1->cols[0], &m2->cols[0]), Vec3_sub(&m1->cols[1], &m2->cols[1]), Vec3_sub(&m1->cols[2], &m2->cols[2]));
}

inline Vec3 Mat3_mulV(Mat3* m, Vec3* v) {
  return (Vec3) {
      .xyz = {
              ((v->x * Mat3_at(m, 0, 0)) + (v->y * Mat3_at(m, 0, 1)) + (v->z * Mat3_at(m, 0, 2))),
              ((v->x * Mat3_at(m, 1, 0)) + (v->y * Mat3_at(m, 1, 1)) + (v->z * Mat3_at(m, 1, 2))),
              ((v->x * Mat3_at(m, 2, 0)) + (v->y * Mat3_at(m, 2, 1)) + (v->z * Mat3_at(m, 2, 2))),
              }
  };
}

inline Mat3 Mat3_mulM(Mat3* m1, Mat3* m2) {
  return Mat3_newF(
      ((Mat3_at(m1, 0, 0) * Mat3_at(m2, 0, 0)) + (Mat3_at(m1, 0, 1) * Mat3_at(m2, 1, 0)) + (Mat3_at(m1, 0, 2) * Mat3_at(m2, 2, 0))),
      ((Mat3_at(m1, 0, 0) * Mat3_at(m2, 0, 1)) + (Mat3_at(m1, 0, 1) * Mat3_at(m2, 1, 1)) + (Mat3_at(m1, 0, 2) * Mat3_at(m2, 2, 1))),
      ((Mat3_at(m1, 0, 0) * Mat3_at(m2, 0, 2)) + (Mat3_at(m1, 0, 1) * Mat3_at(m2, 1, 2)) + (Mat3_at(m1, 0, 2) * Mat3_at(m2, 2, 2))),
      ((Mat3_at(m1, 1, 0) * Mat3_at(m2, 0, 0)) + (Mat3_at(m1, 1, 1) * Mat3_at(m2, 1, 0)) + (Mat3_at(m1, 1, 2) * Mat3_at(m2, 2, 0))),
      ((Mat3_at(m1, 1, 0) * Mat3_at(m2, 0, 1)) + (Mat3_at(m1, 1, 1) * Mat3_at(m2, 1, 1)) + (Mat3_at(m1, 1, 2) * Mat3_at(m2, 2, 1))),
      ((Mat3_at(m1, 1, 0) * Mat3_at(m2, 0, 2)) + (Mat3_at(m1, 1, 1) * Mat3_at(m2, 1, 2)) + (Mat3_at(m1, 1, 2) * Mat3_at(m2, 2, 2))),
      ((Mat3_at(m1, 2, 0) * Mat3_at(m2, 0, 0)) + (Mat3_at(m1, 2, 1) * Mat3_at(m2, 1, 0)) + (Mat3_at(m1, 2, 2) * Mat3_at(m2, 2, 0))),
      ((Mat3_at(m1, 2, 0) * Mat3_at(m2, 0, 1)) + (Mat3_at(m1, 2, 1) * Mat3_at(m2, 1, 1)) + (Mat3_at(m1, 2, 2) * Mat3_at(m2, 2, 1))),
      ((Mat3_at(m1, 2, 0) * Mat3_at(m2, 0, 2)) + (Mat3_at(m1, 2, 1) * Mat3_at(m2, 1, 2)) + (Mat3_at(m1, 2, 2) * Mat3_at(m2, 2, 2))));
}

inline Mat3 Mat3_inverse(Mat3* m) {
  Vec3* a       = &m->cols[0];
  Vec3* b       = &m->cols[1];
  Vec3* c       = &m->cols[2];
  Vec3  r0      = Vec3_cross(b, c);
  Vec3  r1      = Vec3_cross(c, a);
  Vec3  r2      = Vec3_cross(a, b);
  float inv_det = 1.0f / Vec3_dot(&r2, c);
  return Mat3_newF(inv_det * r0.x, inv_det * r0.y, inv_det * r0.z, inv_det * r1.x, inv_det * r1.y, inv_det * r1.z, inv_det * r2.x,
                   inv_det * r2.y, inv_det * r2.z);
}
