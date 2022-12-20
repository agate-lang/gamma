#include "gamma_utils.h"

void *gammaForeignAllocate(AgateVM *vm, ptrdiff_t slot, const char *unit_name, const char *class_name) {
  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, unit_name, class_name, class_slot);
  return agateSlotSetForeign(vm, slot, class_slot);
}
