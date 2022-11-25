#ifndef GAMMA_ROOT_H
#define GAMMA_ROOT_H

#include <stdint.h>

struct gammaVec2F {
  float x;
  float y;
};

struct gammaVec2I {
  int64_t x;
  int64_t y;
};

struct gammaColor {
  float r;
  float g;
  float b;
  float a;
};

struct gammaRectF {
  float x;
  float y;
  float w;
  float h;
};

struct gammaRectI {
  int64_t x;
  int64_t y;
  int64_t w;
  int64_t h;
};

#endif // GAMMA_ROOT_H
