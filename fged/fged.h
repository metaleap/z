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



typedef union Mat3 {
  Vec3  cols[3];
  float at[3][3];
} Mat3;

Mat3  Mat3_new(Vec3 col0, Vec3 col1, Vec3 col2);
float Mat3_at(Mat3* mat, size_t idxCol, size_t idxRow);
Mat3  Mat3_neg(Mat3* m);
Mat3  Mat3_mul(Mat3* m, float n);
Mat3  Mat3_div(Mat3* m, float n);
Mat3  Mat3_add(Mat3* m1, Mat3* m2);
Mat3  Mat3_sub(Mat3* m1, Mat3* m2);
Vec3  Mat3_mulVec(Mat3* m, Vec3* v);
Mat3  Mat3_mulMat(Mat3* m1, Mat3* m2);
