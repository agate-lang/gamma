#ifndef GAMMA_COMMON_H
#define GAMMA_COMMON_H

#include <stdbool.h>

bool gammaEquals(const char *lhs, const char *rhs);

static inline float gammaClampF(float value, float min, float max) {
  if (value < min) { value = min; }
  if (value > max) { value = max; }
  return value;
}

static inline float gammaMin2F(float a, float b) {
  return a < b ? a : b;
}

static inline float gammaMin3F(float a, float b, float c) {
  return gammaMin2F(gammaMin2F(a, b), c);
}

static inline float gammaMax2F(float a, float b) {
  return a < b ? b : a;
}

static inline float gammaMax3F(float a, float b, float c) {
  return gammaMax2F(gammaMax2F(a, b), c);
}

#endif // GAMMA_COMMON_H
