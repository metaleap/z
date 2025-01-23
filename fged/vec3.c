#include <math.h>


#include "./fged.h"


Vec3 Vec3_new(float x, float y, float z) {
  return (Vec3) {.x = x, .y = y, .z = z};
}

Vec3 Vec3_neg(Vec3* v) {
  return (Vec3) {.x = (-v->x), .y = (-v->y), .z = (-v->z)};
}

Vec3 Vec3_mul(Vec3* v, float n) {
  return (Vec3) {.x = (v->x * n), .y = (v->y * n), .z = (v->z * n)};
}

Vec3 Vec3_div(Vec3* v, float n) {
  return (Vec3) {.x = (v->x / n), .y = (v->y / n), .z = (v->z / n)};
}

Vec3 Vec3_add(Vec3* v1, Vec3* v2) {
  return (Vec3) {.x = (v1->x + v2->x), .y = (v1->y + v2->y), .z = (v1->z + v2->z)};
}

Vec3 Vec3_sub(Vec3* v1, Vec3* v2) {
  return (Vec3) {.x = (v1->x - v2->x), .y = (v1->y - v2->y), .z = (v1->z - v2->z)};
}

float Vec3_len(Vec3* v) {
  return sqrtf((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

Vec3 Vec3_norm(Vec3* v) {
  return Vec3_div(v, Vec3_len(v));
}
