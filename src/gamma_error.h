#ifndef GAMMA_ERROR_H
#define GAMMA_ERROR_H

#include "agate.h"

#ifdef __GNUC__
  #define GAMMA_FORMAT(X, Y) __attribute__ ((format (printf, X, Y)))
#else
  #define GAMMA_FORMAT(X, Y)
#endif

void gammaError(AgateVM *vm, const char *format, ...) GAMMA_FORMAT(2, 3);

#endif // GAMMA_ERROR_H
