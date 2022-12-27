#ifndef GAMMA_EVENT_H
#define GAMMA_EVENT_H

#include "agate.h"

AgateForeignClassHandler gammaEventClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaEventMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_EVENT_H