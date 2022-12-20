#ifndef GAMMA_MATH_H
#define GAMMA_MATH_H

#include "agate.h"

struct GammaVec2F {
  float v[2];
};

struct GammaVec2I {
  int v[2];
};

struct GammaRectF {
  struct GammaVec2F position;
  struct GammaVec2F size;
};

static inline
struct GammaVec2F gammaRectFRawCenter(const struct GammaRectF *rect) {
  struct GammaVec2F center = {{ rect->position.v[0] + rect->size.v[0] / 2.0f, rect->position.v[1] + rect->size.v[1] / 2.0f }};
  return center;
}

struct GammaRectI {
  struct GammaVec2I position;
  struct GammaVec2I size;
};

struct GammaMat3F {
  float m[3][3]; // column-major
};

void gammaMat3FRawTranslation(struct GammaMat3F *mat, struct GammaVec2F offset);
void gammaMat3FRawMul(struct GammaMat3F *result, struct GammaMat3F *lhs, struct GammaMat3F *rhs);
void gammaMat3FRawTransformPoint(struct GammaVec2F *result, struct GammaMat3F *lhs, struct GammaVec2F *rhs);
bool gammaMat3FRawInverse(struct GammaMat3F *result, const struct GammaMat3F *mat);

AgateForeignClassHandler gammaMathClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaMathMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_MATH_H
