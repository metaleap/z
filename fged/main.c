#include <math.h>
#include <stdio.h>

#include "./fged.h"


int main() {
  Vec3 v = Vec3_new(1.23f, 3.21f, 7.89f);

  printf("%f\n", Vec3_len(&v));
  Vec3 norm = Vec3_norm(&v);
  printf("xyz=[%f,%f,%f]\t\t%f\n", norm.xyz[0], norm.xyz[1], norm.xyz[2], Vec3_len(&norm));
}
