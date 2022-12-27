#include "gamma_time.h"

#if _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

#include "gamma_agate.h"

namespace gma {

  struct TimeApi : TimeClass {

    static void now(AgateVM *vm) {
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

    static void from_seconds(AgateVM *vm) {
      int64_t nanoseconds = 0;

      switch (agateSlotType(vm, 1)) {
        case AGATE_TYPE_FLOAT:
          nanoseconds = (int64_t) (agateSlotGetFloat(vm, 1) * 1e9);
          break;
        case AGATE_TYPE_INT:
          nanoseconds = agateSlotGetInt(vm, 1) * INT64_C(1000000000);
          break;
        default:
          agateError(vm, "Float or Int parameter expected for `value`");
          break;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
    }

    static void from_milliseconds(AgateVM *vm) {
      int64_t nanoseconds = 0;

      switch (agateSlotType(vm, 1)) {
        case AGATE_TYPE_FLOAT:
          nanoseconds = (int64_t) (agateSlotGetFloat(vm, 1) * 1e6);
          break;
        case AGATE_TYPE_INT:
          nanoseconds = agateSlotGetInt(vm, 1) * INT64_C(1000000);
          break;
        default:
          agateError(vm, "Float or Int parameter expected for `value`");
          break;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
    }

    static void from_microseconds(AgateVM *vm) {
      int64_t nanoseconds = 0;

      switch (agateSlotType(vm, 1)) {
        case AGATE_TYPE_FLOAT:
          nanoseconds = (int64_t) (agateSlotGetFloat(vm, 1) * 1e3);
          break;
        case AGATE_TYPE_INT:
          nanoseconds = agateSlotGetInt(vm, 1) * INT64_C(1000);
          break;
        default:
          agateError(vm, "Float or Int parameter expected for `value`");
          break;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds);
    }

    static void to_seconds(AgateVM *vm) {
      int64_t nanoseconds;

      if (!agateCheck(vm, 1, nanoseconds)) {
        agateError(vm, "Int parameter expected for `value`");
        return;
      }

      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, nanoseconds * 1e-9);
    }

    static void to_milliseconds(AgateVM *vm) {
      int64_t nanoseconds;

      if (!agateCheck(vm, 1, nanoseconds)) {
        agateError(vm, "Int parameter expected for `value`");
        return;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds / 1000000);
    }

    static void to_microseconds(AgateVM *vm) {
      int64_t nanoseconds;

      if (!agateCheck(vm, 1, nanoseconds)) {
        agateError(vm, "Int parameter expected for `value`");
        return;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, nanoseconds / 1000);
    }

  };

  void TimeUnit::provide_support(Support & support) {
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "now", TimeApi::now);
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "from_seconds(_)", TimeApi::from_seconds);
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "from_milliseconds(_)", TimeApi::from_milliseconds);
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "from_microseconds(_)", TimeApi::from_microseconds);
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "to_seconds(_)", TimeApi::to_seconds);
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "to_milliseconds(_)", TimeApi::to_milliseconds);
    support.add_method(unit_name, TimeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "to_microseconds(_)", TimeApi::to_microseconds);
  }

}
