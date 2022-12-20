#ifndef GAMMA_RENDER_H
#define GAMMA_RENDER_H

#include "agate.h"

struct GammaColor {
  float r;
  float g;
  float b;
  float a;
};

void gammaColorRawFromRgba(struct GammaColor *color, int64_t rgba);

AgateForeignClassHandler gammaRenderClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaRenderMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_RENDER_H
