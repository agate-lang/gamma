#ifndef GAMMA_CHECK_H
#define GAMMA_CHECK_H

#include "agate.h"

struct GammaVec2F;
struct GammaVec2I;
struct GammaColor;
struct GammaRectF;
struct GammaRectI;

bool gammaCheckInt(AgateVM *vm, ptrdiff_t slot, int *result);
bool gammaCheckInt64(AgateVM *vm, ptrdiff_t slot, int64_t *result);
bool gammaCheckFloat(AgateVM *vm, ptrdiff_t slot, float *result);

bool gammaCheckForeign(AgateVM *vm, ptrdiff_t slot, uint64_t tag);

bool gammaCheckVec2F(AgateVM *vm, ptrdiff_t slot, struct GammaVec2F *result);
bool gammaCheckVec2I(AgateVM *vm, ptrdiff_t slot, struct GammaVec2I *result);
bool gammaCheckColor(AgateVM *vm, ptrdiff_t slot, struct GammaColor *result);
bool gammaCheckRectF(AgateVM *vm, ptrdiff_t slot, struct GammaRectF *result);
bool gammaCheckRectI(AgateVM *vm, ptrdiff_t slot, struct GammaRectI *result);

#endif // GAMMA_CHECK_H
