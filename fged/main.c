#include <math.h>
#include <stdio.h>

#include "./fged.h"


int main() {
  Vec3 v = Vec3_new(1.23f, 3.21f, 7.89f);

  Vec3 norm = Vec3_norm(&v);
  printf("%f\t\t%f\n", Vec3_len(&v), Vec3_len(&norm));


  Mat3 m = Mat3_new(v, norm, Vec3_new(11, 22, 33));
  v      = m.cols[1];
  printf("%f,%f,%f\n", v.x, v.y, v.z);
  printf("%f,%f,%f\n", m.at[1][0], m.at[1][1], m.at[1][2]);
}
