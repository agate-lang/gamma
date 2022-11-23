#ifndef GAMMA_TIME_H
#define GAMMA_TIME_H

#include "agate.h"

AgateForeignMethodFunc gammaTimeMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_TIME_H
