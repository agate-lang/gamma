#ifndef GAMMA_UTILS_H
#define GAMMA_UTILS_H

#include "agate.h"

void *gammaForeignAllocate(AgateVM *vm, ptrdiff_t slot, const char *unit_name, const char *class_name);

#endif // GAMMA_UTILS_H
