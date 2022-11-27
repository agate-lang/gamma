#ifndef GAMMA_GFX_H
#define GAMMA_GFX_H

#include "agate.h"

AgateForeignClassHandler gammaGfxClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaGfxMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_GFX_H
