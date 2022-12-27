#include "gamma_agate.h"

#include <cassert>
#include <cstdio>
#include <cstdarg>

#include <vector>

namespace gma {

  static void agateErrorV(AgateVM *vm, const char *format, va_list args0) {
    va_list args1;
    va_copy(args1, args0);

    int size0 = std::vsnprintf(NULL, 0, format, args0);
    std::vector<char> string(size0 + 1);

    int size1 = std::vsnprintf(string.data(), string.size(), format, args1);
    assert(size0 == size1);
    va_end(args1);

    ptrdiff_t string_slot = agateSlotAllocate(vm);
    agateSlotSetStringSize(vm, string_slot, string.data(), size1);

    agateAbort(vm, string_slot);
  }

  void agateError(AgateVM *vm, const char *format, ...) {
    va_list args0;
    va_start(args0, format);
    agateErrorV(vm, format, args0);
    va_end(args0);
  }

  bool agateCheck(AgateVM * vm, ptrdiff_t slot, bool & result) {
    if (agateSlotType(vm, slot) == AGATE_TYPE_BOOL) {
      result = agateSlotGetBool(vm, slot);
      return true;
    }

    result = false;
    return false;
  }

  bool agateCheck(AgateVM * vm, ptrdiff_t slot, int64_t & result) {
    if (agateSlotType(vm, slot) == AGATE_TYPE_INT) {
      result = agateSlotGetInt(vm, slot);
      return true;
    }

    result = 0;
    return false;
  }

  bool agateCheck(AgateVM * vm, ptrdiff_t slot, int & result) {
    if (agateSlotType(vm, slot) == AGATE_TYPE_INT) {
      result = static_cast<int>(agateSlotGetInt(vm, slot));
      return true;
    }

    result = 0;
    return false;
  }

  bool agateCheck(AgateVM * vm, ptrdiff_t slot, float & result) {
    if (agateSlotType(vm, slot) == AGATE_TYPE_FLOAT) {
      result = static_cast<float>(agateSlotGetFloat(vm, slot));
      return true;
    }

    if (agateSlotType(vm, slot) == AGATE_TYPE_INT) {
      result = static_cast<float>(agateSlotGetInt(vm, slot));
      return true;
    }

    result = 0.0f;
    return false;
  }

  bool agateCheck(AgateVM * vm, ptrdiff_t slot, const char * & result) {
    if (agateSlotType(vm, slot) == AGATE_TYPE_STRING) {
      result = agateSlotGetString(vm, slot);
      return true;
    }

    result = "";
    return false;
  }


}
