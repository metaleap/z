#include <stddef.h>



typedef union Vec3 {
  float xyz[3];
  struct {
    float x;
    float y;
    float z;
  };
} Vec3;

Vec3  Vec3_new(float x, float y, float z);
Vec3  Vec3_mul(Vec3* v, float n);
Vec3  Vec3_div(Vec3* v, float n);
Vec3  Vec3_neg(Vec3* v);
float Vec3_len(Vec3* v);
Vec3  Vec3_norm(Vec3* v);
Vec3  Vec3_add(Vec3* v1, Vec3* v2);
Vec3  Vec3_sub(Vec3* v1, Vec3* v2);
float Vec3_dot(Vec3* v1, Vec3* v2);
Vec3  Vec3_cross(Vec3* v1, Vec3* v2);
Vec3  Vec3_project(Vec3* a, Vec3* b);
Vec3  Vec3_reject(Vec3* a, Vec3* b);



typedef union Vec4 {
  float xyzw[4];
  struct {
    float x;
    float y;
    float z;
    float w;
  };
} Vec4;

Vec4  Vec4_new(float x, float y, float z, float w);
Vec4  Vec4_mul(Vec4* v, float n);
Vec4  Vec4_div(Vec4* v, float n);
Vec4  Vec4_neg(Vec4* v);
float Vec4_len(Vec4* v);
Vec4  Vec4_norm(Vec4* v);
Vec4  Vec4_add(Vec4* v1, Vec4* v2);
Vec4  Vec4_sub(Vec4* v1, Vec4* v2);
float Vec4_dot(Vec4* v1, Vec4* v2);
Vec4  Vec4_cross(Vec4* v1, Vec4* v2);
Vec4  Vec4_project(Vec4* a, Vec4* b);
Vec4  Vec4_reject(Vec4* a, Vec4* b);



typedef union Mat3 {
  Vec3  cols[3];
  float at[3][3];
} Mat3;

Mat3  Mat3_newV(Vec3 col0, Vec3 col1, Vec3 col2);
Mat3  Mat3_newF(float n00, float n01, float n02, float n10, float n11, float n12, float n20, float n21, float n22);
float Mat3_at(Mat3* mat, size_t idxCol, size_t idxRow);
Mat3  Mat3_neg(Mat3* m);
Mat3  Mat3_add(Mat3* m1, Mat3* m2);
Mat3  Mat3_sub(Mat3* m1, Mat3* m2);
Mat3  Mat3_div(Mat3* m, float n);
Mat3  Mat3_mulF(Mat3* m, float n);
Vec3  Mat3_mulV(Mat3* m, Vec3* v);
Mat3  Mat3_mulM(Mat3* m1, Mat3* m2);
Mat3  Mat3_inverse(Mat3* m);



typedef union Mat4 {
  Vec4  cols[4];
  float at[4][4];
} Mat4;

Mat4  Mat4_newV(Vec4 col0, Vec4 col1, Vec4 col2, Vec4 col3);
Mat4  Mat4_newF(float n00, float n01, float n02, float n03, float n10, float n11, float n12, float n13, float n20, float n21, float n22,
                float n23, float n30, float n31, float n32, float n33);
float Mat4_at(Mat4* mat, size_t idxCol, size_t idxRow);
Mat4  Mat4_neg(Mat4* m);
Mat4  Mat4_add(Mat4* m1, Mat4* m2);
Mat4  Mat4_sub(Mat4* m1, Mat4* m2);
Mat4  Mat4_div(Mat4* m, float n);
Mat4  Mat4_mulF(Mat4* m, float n);
Vec4  Mat4_mulV(Mat4* m, Vec4* v);
Mat4  Mat4_mulM(Mat4* m1, Mat4* m2);
Mat4  Mat4_inverse(Mat4* m);
