#include "./fged.h"



inline Mat4 Mat4_newV(Vec4 col0, Vec4 col1, Vec4 col2, Vec4 col3) {
  return (Mat4) {
      .cols = {col0, col1, col2, col3}
  };
}

inline Mat4 Mat4_newF(float n00, float n01, float n02, float n03, float n10, float n11, float n12, float n13, float n20, float n21, float n22,
                      float n23, float n30, float n31, float n32, float n33) {
  Mat4 ret;
  ret.at[0][0] = n00;
  ret.at[0][1] = n10;
  ret.at[0][2] = n20;
  ret.at[0][3] = n30;
  ret.at[1][0] = n01;
  ret.at[1][1] = n11;
  ret.at[1][2] = n21;
  ret.at[1][3] = n31;
  ret.at[2][0] = n02;
  ret.at[2][1] = n12;
  ret.at[2][2] = n22;
  ret.at[2][3] = n32;
  ret.at[3][0] = n03;
  ret.at[3][1] = n13;
  ret.at[3][2] = n23;
  ret.at[3][3] = n33;
  return ret;
}

inline Mat4 Mat4_identity() {
  return Mat4_newV(Vec4_new(1, 0, 0, 0), Vec4_new(0, 1, 0, 0), Vec4_new(0, 0, 1, 0), Vec4_new(0, 0, 0, 1));
}

inline float Mat4_at(Mat4* m, size_t idxRow, size_t idxCol) {
  return m->at[idxCol][idxRow];
}

inline Mat4 Mat4_neg(Mat4* m) {
  return Mat4_newV(Vec4_neg(&m->cols[0]), Vec4_neg(&m->cols[1]), Vec4_neg(&m->cols[2]), Vec4_neg(&m->cols[3]));
}

inline Mat4 Mat4_mulF(Mat4* m, float n) {
  return Mat4_newV(Vec4_mul(&m->cols[0], n), Vec4_mul(&m->cols[1], n), Vec4_mul(&m->cols[2], n), Vec4_mul(&m->cols[3], n));
}

inline Mat4 Mat4_div(Mat4* m, float n) {
  return Mat4_newV(Vec4_div(&m->cols[0], n), Vec4_div(&m->cols[1], n), Vec4_div(&m->cols[2], n), Vec4_div(&m->cols[3], n));
}

inline Mat4 Mat4_add(Mat4* m1, Mat4* m2) {
  return Mat4_newV(Vec4_add(&m1->cols[0], &m2->cols[0]), Vec4_add(&m1->cols[1], &m2->cols[1]), Vec4_add(&m1->cols[2], &m2->cols[2]),
                   Vec4_add(&m1->cols[3], &m2->cols[3]));
}

inline Mat4 Mat4_sub(Mat4* m1, Mat4* m2) {
  return Mat4_newV(Vec4_sub(&m1->cols[0], &m2->cols[0]), Vec4_sub(&m1->cols[1], &m2->cols[1]), Vec4_sub(&m1->cols[2], &m2->cols[2]),
                   Vec4_sub(&m1->cols[3], &m2->cols[3]));
}

inline Vec4 Mat4_mulV(Mat4* m, Vec4* v) {
  return (Vec4) {
      .xyzw = {
               ((v->x * Mat4_at(m, 0, 0)) + (v->y * Mat4_at(m, 0, 1)) + (v->z * Mat4_at(m, 0, 2)) + (v->z * Mat4_at(m, 0, 3))),
               ((v->x * Mat4_at(m, 1, 0)) + (v->y * Mat4_at(m, 1, 1)) + (v->z * Mat4_at(m, 1, 2)) + (v->z * Mat4_at(m, 1, 3))),
               ((v->x * Mat4_at(m, 2, 0)) + (v->y * Mat4_at(m, 2, 1)) + (v->z * Mat4_at(m, 2, 2)) + (v->z * Mat4_at(m, 2, 3))),
               ((v->x * Mat4_at(m, 3, 0)) + (v->y * Mat4_at(m, 3, 1)) + (v->z * Mat4_at(m, 3, 2)) + (v->z * Mat4_at(m, 3, 3))),
               }
  };
}

inline Mat4 Mat4_inverse(Mat4* m) {
  Vec3* a       = (Vec3*) (&m->cols[0]);
  Vec3* b       = (Vec3*) (&m->cols[1]);
  Vec3* c       = (Vec3*) (&m->cols[2]);
  Vec3* d       = (Vec3*) (&m->cols[3]);
  float x       = Mat4_at(m, 3, 0);
  float y       = Mat4_at(m, 3, 1);
  float z       = Mat4_at(m, 3, 2);
  float w       = Mat4_at(m, 3, 3);
  Vec3  s       = Vec3_cross(a, b);
  Vec3  t       = Vec3_cross(c, d);
  Vec3  ay      = Vec3_mul(a, y);
  Vec3  bx      = Vec3_mul(b, x);
  Vec3  u       = Vec3_sub(&ay, &bx);
  Vec3  cw      = Vec3_mul(c, w);
  Vec3  dz      = Vec3_mul(d, z);
  Vec3  v       = Vec3_sub(&cw, &dz);
  float inv_det = 1.0f / (Vec3_dot(&s, &v) + Vec3_dot(&t, &u));
  s             = Vec3_mul(&s, inv_det);
  t             = Vec3_mul(&t, inv_det);
  u             = Vec3_mul(&u, inv_det);
  v             = Vec3_mul(&v, inv_det);
  Vec3 bv       = Vec3_cross(b, &v);
  Vec3 ty       = Vec3_mul(&t, y);
  Vec3 r0       = Vec3_add(&bv, &ty);
  Vec3 va       = Vec3_cross(&v, a);
  Vec3 tx       = Vec3_mul(&t, x);
  Vec3 r1       = Vec3_add(&va, &tx);
  Vec3 du       = Vec3_cross(d, &u);
  Vec3 sw       = Vec3_mul(&s, w);
  Vec3 r2       = Vec3_add(&du, &sw);
  Vec3 uc       = Vec3_cross(&u, c);
  Vec3 sz       = Vec3_mul(&s, z);
  Vec3 r3       = Vec3_add(&uc, &sz);
}
