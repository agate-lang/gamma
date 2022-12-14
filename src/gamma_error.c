#include "gamma_error.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

static void gammaErrorV(AgateVM *vm, const char *format, va_list args0) {
  va_list args1;
  va_copy(args1, args0);

  int size0 = vsnprintf(NULL, 0, format, args0);
  char *string = agateMemoryAllocate(vm, NULL, (size0 + 1) * sizeof(char));

  int size1 = vsnprintf(string, size0 + 1, format, args1);
  assert(size0 == size1);
  va_end(args1);

  ptrdiff_t string_slot = agateSlotAllocate(vm);
  agateSlotSetStringSize(vm, string_slot, string, size1);

  agateMemoryAllocate(vm, string, 0);

  agateAbort(vm, string_slot);
}

void gammaError(AgateVM *vm, const char *format, ...) {
  va_list args0;
  va_start(args0, format);
  gammaErrorV(vm, format, args0);
  va_end(args0);
}

bool gammaCheckError(AgateVM *vm, bool check, const char *format, ...) {
  if (check) {
    return check;
  }

  va_list args0;
  va_start(args0, format);
  gammaErrorV(vm, format, args0);
  va_end(args0);
  return check;
}
