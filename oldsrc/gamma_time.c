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

static void gammaTimeFromSeconds(AgateVM *vm) {
  int64_t nanoseconds = 0;

  switch (agateSlotType(vm, 1)) {
    case AGATE_TYPE_FLOAT:
      nanoseconds = (int64_t) (agateSlotGetFloat(vm, 1) * 1e9);
      break;
    case AGATE_TYPE_INT:
      nanoseconds = agateSlotGetInt(vm, 1) * INT64_C(1000000000);
      break;
    default:
      break;
  }

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
}

static void gammaTimeFromMilliseconds(AgateVM *vm) {
  int64_t nanoseconds = 0;

  switch (agateSlotType(vm, 1)) {
    case AGATE_TYPE_FLOAT:
      nanoseconds = (int64_t) (agateSlotGetFloat(vm, 1) * 1e6);
      break;
    case AGATE_TYPE_INT:
      nanoseconds = agateSlotGetInt(vm, 1) * INT64_C(1000000);
      break;
    default:
      break;
  }

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
}

static void gammaTimeFromMicroseconds(AgateVM *vm) {
  int64_t nanoseconds = 0;

  switch (agateSlotType(vm, 1)) {
    case AGATE_TYPE_FLOAT:
      nanoseconds = (int64_t) (agateSlotGetFloat(vm, 1) * 1e3);
      break;
    case AGATE_TYPE_INT:
      nanoseconds = agateSlotGetInt(vm, 1) * INT64_C(1000);
      break;
    default:
      break;
  }

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
}

static void gammaTimeToSeconds(AgateVM *vm) {
  assert(agateSlotType(vm, 1) == AGATE_TYPE_INT);
  int64_t nanoseconds = agateSlotGetInt(vm, 1);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, nanoseconds * 1e-9);
}

static void gammaTimeToMilliseconds(AgateVM *vm) {
  assert(agateSlotType(vm, 1) == AGATE_TYPE_INT);
  int64_t nanoseconds = agateSlotGetInt(vm, 1);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds / 1000000);
}

static void gammaTimeToMicroseconds(AgateVM *vm) {
  assert(agateSlotType(vm, 1) == AGATE_TYPE_INT);
  int64_t nanoseconds = agateSlotGetInt(vm, 1);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds / 1000);
}

AgateForeignMethodFunc gammaTimeMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/time"));

  if (gammaEquals(class_name, "Time")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      if (gammaEquals(signature, "now")) { return gammaTimeNow; }
      if (gammaEquals(signature, "from_seconds(_)")) { return gammaTimeFromSeconds; }
      if (gammaEquals(signature, "from_milliseconds(_)")) { return gammaTimeFromMilliseconds; }
      if (gammaEquals(signature, "from_microseconds(_)")) { return gammaTimeFromMicroseconds; }
      if (gammaEquals(signature, "to_seconds(_)")) { return gammaTimeToSeconds; }
      if (gammaEquals(signature, "to_milliseconds(_)")) { return gammaTimeToMilliseconds; }
      if (gammaEquals(signature, "to_microseconds(_)")) { return gammaTimeToMicroseconds; }
    }
  }

  return NULL;
}
