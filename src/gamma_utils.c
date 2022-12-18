#include "gamma_utils.h"

void *gammaForeignAllocate(AgateVM *vm, ptrdiff_t slot, const char *class_name) {
  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", class_name, class_slot);
  return agateSlotSetForeign(vm, slot, class_slot);
}
