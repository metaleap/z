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

inline Mat3 Mat3_newRotX(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  return Mat3_newF(1, 0, 0, 0, c, -s, 0, s, c);
}

inline Mat3 Mat3_newRotY(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  return Mat3_newF(c, 0, s, 0, 1, 0, -s, 0, c);
}

inline Mat3 Mat3_newRotZ(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  return Mat3_newF(c, -s, 0, s, c, 0, 0, 0, 1);
}

inline Mat3 Mat3_newRot(Vec3* normAxis, float angle) {
  float c  = cosf(angle);
  float s  = sinf(angle);
  float d  = 1.0f - c;
  float x  = normAxis->x * d;
  float y  = normAxis->y * d;
  float z  = normAxis->z * d;
  float xy = x * normAxis->y;
  float xz = x * normAxis->z;
  float yz = y * normAxis->z;
  return Mat3_newF(c + (x * normAxis->x), xy - (s * normAxis->z), xz + (s * normAxis->y), xy + (s * normAxis->z), c + (y * normAxis->y),
                   yz - (s * normAxis->x), xz - (s * normAxis->y), yz + (s * normAxis->x), c + (z * normAxis->z));
}

inline Mat3 Mat3_newRefl(Vec3* normPlane) {
  float x  = normPlane->x * -2.0f;
  float y  = normPlane->y * -2.0f;
  float z  = normPlane->z * -2.0f;
  float xy = x * normPlane->y;
  float xz = x * normPlane->z;
  float yz = y * normPlane->z;
  return Mat3_newF((x * normPlane->x) + 1.0f, xy, xz, xy, (y * normPlane->y) + 1.0f, yz, xz, yz, (z * normPlane->z) + 1.0f);
}

inline Mat3 Mat3_newInvol(Vec3* norm) {
  float x  = norm->x * 2.0f;
  float y  = norm->y * 2.0f;
  float z  = norm->z * 2.0f;
  float xy = x * norm->y;
  float xz = x * norm->z;
  float yz = y * norm->z;
  return Mat3_newF((x * norm->x) - 1.0f, xy, xz, xy, (y * norm->y) - 1.0f, yz, xz, yz, (z * norm->z) - 1.0f);
}

inline Mat3 Mat3_newScaleF(float scaleX, float scaleY, float scaleZ) {
  return Mat3_newF(scaleX, 0, 0, 0, scaleY, 0, 0, 0, scaleZ);
}

inline Mat3 Mat3_newScaleV(float scale, Vec3* normDir) {
  scale    -= 1.0f;
  float x   = normDir->x * scale;
  float y   = normDir->y * scale;
  float z   = normDir->z * scale;
  float xy  = x * normDir->y;
  float xz  = x * normDir->z;
  float yz  = y * normDir->z;
  return Mat3_newF((x * normDir->x) + 1.0f, xy, xz, xy, (y * normDir->y) + 1.0f, yz, xz, yz, (z * normDir->z) + 1.0f);
}

inline Mat3 Mat3_newSkew(float t, Vec3* a, Vec3* b) {
  t       = tanf(t);
  float x = a->x * t;
  float y = a->y * t;
  float z = a->z * t;
  return Mat3_newF((x * b->x) + 1.0f, x * b->y, x * b->z, y * b->x, (y * b->y) + 1.0f, y * b->z, z * b->x, z * b->y, (z * b->z) + 1.0f);
}

inline Mat3 Mat3_newIdent() {
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
