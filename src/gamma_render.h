#ifndef GAMMA_RENDER_H
#define GAMMA_RENDER_H

#include "agate.h"

AgateForeignClassHandler gammaRenderClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaRenderMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_RENDER_H
