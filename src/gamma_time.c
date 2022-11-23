#include "gamma_time.h"

#include <assert.h>
#include <stdint.h>

#if _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

#include "gamma_common.h"

static void gammaTimeNow(AgateVM *vm) {
  static const int64_t BILLION = INT64_C(1000000000);

#ifdef _MSC_VER
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);

  int64_t integral = time.QuadPart / frequency.QuadPart;
  int64_t fractional = time.QuadPart % frequency.QuadPart;
  int64_t nanoseconds = integral * BILLION + fractional * BILLION / frequency.QuadPart;

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
#else
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);

  int64_t nanoseconds = tp.tv_sec * BILLION + tp.tv_nsec;

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
#endif
}



AgateForeignMethodFunc gammaTimeMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/time"));

  if (gammaEquals(class_name, "Time")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      if (gammaEquals(signature, "now")) { return gammaTimeNow; }
    }
  }

  return NULL;
}
