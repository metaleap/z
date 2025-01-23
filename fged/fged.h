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
