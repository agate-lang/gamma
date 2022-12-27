#ifndef GAMMA_AGATE_H
#define GAMMA_AGATE_H

#include <cassert>

#include "agate.h"

namespace gma {

  template<typename Class>
  auto agateSlotNew(AgateVM * vm, ptrdiff_t slot) -> typename Class::type * {
    ptrdiff_t class_slot = agateSlotAllocate(vm);
    agateGetVariable(vm, Class::unit_name, Class::class_name, class_slot);
    return static_cast<typename Class::type *>(agateSlotSetForeign(vm, slot, class_slot));
  }

  template<typename Class>
  bool agateCheckTag(AgateVM * vm, ptrdiff_t slot) {
    return agateSlotGetForeignTag(vm, slot) == Class::tag;
  }

  template<typename Class>
  auto agateSlotGet(AgateVM *vm, ptrdiff_t slot) -> typename Class::type * {
    assert(agateCheckTag<Class>(vm, slot));
    return static_cast<typename Class::type *>(agateSlotGetForeign(vm, slot));
  }

  inline
  void agateSlotSet(AgateVM *vm, ptrdiff_t slot, float value) { agateSlotSetFloat(vm, slot, value); }
  inline
  void agateSlotSet(AgateVM *vm, ptrdiff_t slot, double value) { agateSlotSetFloat(vm, slot, value); }
  inline
  void agateSlotSet(AgateVM *vm, ptrdiff_t slot, int32_t value) { agateSlotSetInt(vm, slot, value); }
  inline
  void agateSlotSet(AgateVM *vm, ptrdiff_t slot, int64_t value) { agateSlotSetInt(vm, slot, value); }


  void agateError(AgateVM *vm, const char *format, ...);

  bool agateCheck(AgateVM * vm, ptrdiff_t slot, bool & result);
  bool agateCheck(AgateVM * vm, ptrdiff_t slot, int64_t & result);
  bool agateCheck(AgateVM * vm, ptrdiff_t slot, int & result);
  bool agateCheck(AgateVM * vm, ptrdiff_t slot, float & result);
  bool agateCheck(AgateVM * vm, ptrdiff_t slot, const char * & result);

}

#endif // GAMMA_AGATE_H
