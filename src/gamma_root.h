#ifndef GAMMA_ROOT_H
#define GAMMA_ROOT_H

#include "agate.h"

struct GammaVec2F {
  float x;
  float y;
};

struct GammaVec2I {
  int x;
  int y;
};

struct GammaColor {
  float r;
  float g;
  float b;
  float a;
};

struct GammaRectF {
  float x;
  float y;
  float w;
  float h;
};

struct GammaRectI {
  int x;
  int y;
  int w;
  int h;
};

struct GammaMat3F {
  float data[3][3]; // column-major
};

AgateForeignClassHandler gammaRootClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaRootMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_ROOT_H
