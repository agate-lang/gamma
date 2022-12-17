#include "gamma_check.h"

#include <assert.h>
#include <string.h>

#include "gamma_root.h"
#include "gamma_tags.h"

bool gammaCheckInt(AgateVM *vm, ptrdiff_t slot, int *result) {
  assert(result);

  if (agateSlotType(vm, slot) == AGATE_TYPE_INT) {
    *result = (int) agateSlotGetInt(vm, slot);
    return true;
  }

  *result = 0;
  return false;
}

bool gammaCheckInt64(AgateVM *vm, ptrdiff_t slot, int64_t *result) {
  assert(result);

  if (agateSlotType(vm, slot) == AGATE_TYPE_INT) {
    *result = agateSlotGetInt(vm, slot);
    return true;
  }

  *result = 0;
  return false;
}

bool gammaCheckFloat(AgateVM *vm, ptrdiff_t slot, float *result) {
  assert(result);

  if (agateSlotType(vm, slot) == AGATE_TYPE_FLOAT) {
    *result = (float) agateSlotGetFloat(vm, slot);
    return true;
  }

  if (agateSlotType(vm, slot) == AGATE_TYPE_INT) {
    *result = (float) agateSlotGetInt(vm, slot);
    return true;
  }

  *result = 0.0f;
  return false;
}

bool gammaCheckForeign(AgateVM *vm, ptrdiff_t slot, uint64_t tag) {
  return agateSlotType(vm, slot) == AGATE_TYPE_FOREIGN && agateSlotGetForeignTag(vm, slot) == tag;
}

bool gammaCheckVec2F(AgateVM *vm, ptrdiff_t slot, struct GammaVec2F *result) {
  assert(result);
  memset(result, 0, sizeof(*result));

  AgateType type = agateSlotType(vm, slot);

  if (type == AGATE_TYPE_FOREIGN) {
    uint64_t tag = agateSlotGetForeignTag(vm, slot);

    if (tag == GAMMA_VEC2F_TAG) {
      struct GammaVec2F *vec = agateSlotGetForeign(vm, slot);
      *result = *vec;
      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_TUPLE) {
    if (agateSlotTupleSize(vm, slot) != 2) {
      return false;
    }

    ptrdiff_t component_slot = agateSlotAllocate(vm);

    for (int i = 0; i < 2; ++i) {
      agateSlotTupleGet(vm, slot, i, component_slot);

      if (!gammaCheckFloat(vm, component_slot, &result->v[i])) {
        return false;
      }
    }

    return true;
  }

  if (type == AGATE_TYPE_ARRAY) {
    if (agateSlotArraySize(vm, slot) != 2) {
      return false;
    }

    ptrdiff_t element_slot = agateSlotAllocate(vm);

    for (int i = 0; i < 2; ++i) {
      agateSlotArrayGet(vm, slot, i, element_slot);

      if (!gammaCheckFloat(vm, element_slot, &result->v[i])) {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool gammaCheckVec2I(AgateVM *vm, ptrdiff_t slot, struct GammaVec2I *result) {
  assert(result);
  memset(result, 0, sizeof(*result));

  AgateType type = agateSlotType(vm, slot);

  if (type == AGATE_TYPE_FOREIGN) {
    uint64_t tag = agateSlotGetForeignTag(vm, slot);

    if (tag == GAMMA_VEC2I_TAG) {
      struct GammaVec2I *vec = agateSlotGetForeign(vm, slot);
      *result = *vec;
      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_TUPLE) {
    if (agateSlotTupleSize(vm, slot) != 2) {
      return false;
    }

    ptrdiff_t component_slot = agateSlotAllocate(vm);

    for (int i = 0; i < 2; ++i) {
      agateSlotTupleGet(vm, slot, i, component_slot);

      if (!gammaCheckInt(vm, component_slot, &result->v[i])) {
        return false;
      }
    }

    return true;
  }

  if (type == AGATE_TYPE_ARRAY) {
    if (agateSlotArraySize(vm, slot) != 2) {
      return false;
    }

    ptrdiff_t element_slot = agateSlotAllocate(vm);

    for (int i = 0; i < 2; ++i) {
      agateSlotArrayGet(vm, slot, i, element_slot);

      if (!gammaCheckInt(vm, element_slot, &result->v[i])) {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool gammaCheckColor(AgateVM *vm, ptrdiff_t slot, struct GammaColor *result) {
  assert(result);
  memset(result, 0, sizeof(*result));

  AgateType type = agateSlotType(vm, slot);

  if (type == AGATE_TYPE_FOREIGN) {
    uint64_t tag = agateSlotGetForeignTag(vm, slot);

    if (tag == GAMMA_COLOR_TAG) {
      struct GammaColor *color = agateSlotGetForeign(vm, slot);
      *result = *color;
      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_INT) {
    int64_t rgba = agateSlotGetInt(vm, slot);
    gammaColorRawFromRgba(result, rgba);
    return false;
  }

  if (type == AGATE_TYPE_TUPLE) {
    ptrdiff_t component_count = agateSlotTupleSize(vm, slot);

    if (component_count != 3 && component_count != 4) {
      return false;
    }

    ptrdiff_t component_slot = agateSlotAllocate(vm);

    #define GAMMA_COLOR_CHECK_TUPLE(field, i)                   \
    agateSlotTupleGet(vm, slot, i, component_slot);             \
    if (!gammaCheckFloat(vm, component_slot, &result->field)) { \
      return false;                                             \
    }

    GAMMA_COLOR_CHECK_TUPLE(r, 0)
    GAMMA_COLOR_CHECK_TUPLE(g, 1)
    GAMMA_COLOR_CHECK_TUPLE(b, 2)

    if (component_count == 4) {
      GAMMA_COLOR_CHECK_TUPLE(a, 3)
    } else {
      result->a = 1.0f;
    }

    #undef GAMMA_COLOR_CHECK_TUPLE

    return true;
  }

  if (type == AGATE_TYPE_ARRAY) {
    ptrdiff_t element_count = agateSlotArraySize(vm, slot);

    if (element_count != 3 && element_count != 4) {
      return false;
    }

    ptrdiff_t element_slot = agateSlotAllocate(vm);

    #define GAMMA_COLOR_CHECK_ARRAY(field, i)                 \
    agateSlotArrayGet(vm, slot, i, element_slot);             \
    if (!gammaCheckFloat(vm, element_slot, &result->field)) { \
      return false;                                           \
    }

    GAMMA_COLOR_CHECK_ARRAY(r, 0)
    GAMMA_COLOR_CHECK_ARRAY(g, 1)
    GAMMA_COLOR_CHECK_ARRAY(b, 2)

    if (element_count == 4) {
      GAMMA_COLOR_CHECK_ARRAY(a, 3)
    } else {
      result->a = 1.0f;
    }

    #undef GAMMA_COLOR_CHECK_ARRAY

    return true;
  }

  return true;
}

bool gammaCheckRectF(AgateVM *vm, ptrdiff_t slot, struct GammaRectF *result) {
  assert(result);
  memset(result, 0, sizeof(*result));

  AgateType type = agateSlotType(vm, slot);

  if (type == AGATE_TYPE_FOREIGN) {
    uint64_t tag = agateSlotGetForeignTag(vm, slot);

    if (tag == GAMMA_RECTF_TAG) {
      struct GammaRectF *rect = agateSlotGetForeign(vm, slot);
      *result = *rect;
      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_TUPLE) {
    ptrdiff_t component_count = agateSlotTupleSize(vm, slot);

    if (component_count == 2) {
      ptrdiff_t component_slot = agateSlotAllocate(vm);

      agateSlotTupleGet(vm, slot, 0, component_slot);

      if (!gammaCheckVec2F(vm, component_slot, &result->position)) {
        return false;
      }

      agateSlotTupleGet(vm, slot, 1, component_slot);

      if (!gammaCheckVec2F(vm, component_slot, &result->size)) {
        return false;
      }
    }

    if (component_count == 4) {
      ptrdiff_t component_slot = agateSlotAllocate(vm);

      #define GAMMA_RECTF_CHECK_TUPLE(field, i)                   \
      agateSlotTupleGet(vm, slot, i, component_slot);             \
      if (!gammaCheckFloat(vm, component_slot, &result->field)) { \
        return false;                                             \
      }

      GAMMA_RECTF_CHECK_TUPLE(position.v[0], 0)
      GAMMA_RECTF_CHECK_TUPLE(position.v[1], 1)
      GAMMA_RECTF_CHECK_TUPLE(size.v[0],     2)
      GAMMA_RECTF_CHECK_TUPLE(size.v[1],     3)

      #undef GAMMA_RECTF_CHECK_TUPLE

      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_ARRAY) {
    ptrdiff_t element_count = agateSlotArraySize(vm, slot);

    if (element_count == 2) {
      ptrdiff_t element_slot = agateSlotAllocate(vm);

      agateSlotArrayGet(vm, slot, 0, element_count);

      if (!gammaCheckVec2F(vm, element_slot, &result->position)) {
        return false;
      }

      agateSlotTupleGet(vm, slot, 1, element_slot);

      if (!gammaCheckVec2F(vm, element_slot, &result->size)) {
        return false;
      }
    }

    if (element_count == 4) {
      ptrdiff_t element_slot = agateSlotAllocate(vm);

      #define GAMMA_RECTF_CHECK_ARRAY(field, i)                 \
      agateSlotArrayGet(vm, slot, i, element_slot);             \
      if (!gammaCheckFloat(vm, element_slot, &result->field)) { \
        return false;                                           \
      }

      GAMMA_RECTF_CHECK_ARRAY(position.v[0], 0)
      GAMMA_RECTF_CHECK_ARRAY(position.v[1], 1)
      GAMMA_RECTF_CHECK_ARRAY(size.v[0],     2)
      GAMMA_RECTF_CHECK_ARRAY(size.v[1],     3)

      #undef GAMMA_RECTF_CHECK_ARRAY

      return true;
    }

    return false;
  }

  return false;
}

bool gammaCheckRectI(AgateVM *vm, ptrdiff_t slot, struct GammaRectI *result) {
  assert(result);
  memset(result, 0, sizeof(*result));

  AgateType type = agateSlotType(vm, slot);

  if (type == AGATE_TYPE_FOREIGN) {
    uint64_t tag = agateSlotGetForeignTag(vm, slot);

    if (tag == GAMMA_RECTI_TAG) {
      struct GammaRectI *rect = agateSlotGetForeign(vm, slot);
      *result = *rect;
      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_TUPLE) {
    ptrdiff_t component_count = agateSlotTupleSize(vm, slot);

    if (component_count == 2) {
      ptrdiff_t component_slot = agateSlotAllocate(vm);

      agateSlotTupleGet(vm, slot, 0, component_slot);

      if (!gammaCheckVec2I(vm, component_slot, &result->position)) {
        return false;
      }

      agateSlotTupleGet(vm, slot, 1, component_slot);

      if (!gammaCheckVec2I(vm, component_slot, &result->size)) {
        return false;
      }
    }

    if (component_count == 4) {
      ptrdiff_t component_slot = agateSlotAllocate(vm);

      #define GAMMA_RECTI_CHECK_TUPLE(field, i)                 \
      agateSlotTupleGet(vm, slot, i, component_slot);           \
      if (!gammaCheckInt(vm, component_slot, &result->field)) { \
        return false;                                           \
      }

      GAMMA_RECTI_CHECK_TUPLE(position.v[0], 0)
      GAMMA_RECTI_CHECK_TUPLE(position.v[1], 1)
      GAMMA_RECTI_CHECK_TUPLE(size.v[0],     2)
      GAMMA_RECTI_CHECK_TUPLE(size.v[1],     3)

      #undef GAMMA_RECTI_CHECK_TUPLE

      return true;
    }

    return false;
  }

  if (type == AGATE_TYPE_ARRAY) {
    ptrdiff_t element_count = agateSlotArraySize(vm, slot);

    if (element_count == 2) {
      ptrdiff_t element_slot = agateSlotAllocate(vm);

      agateSlotArrayGet(vm, slot, 0, element_count);

      if (!gammaCheckVec2I(vm, element_slot, &result->position)) {
        return false;
      }

      agateSlotTupleGet(vm, slot, 1, element_slot);

      if (!gammaCheckVec2I(vm, element_slot, &result->size)) {
        return false;
      }
    }

    if (element_count == 4) {
      ptrdiff_t element_slot = agateSlotAllocate(vm);

      #define GAMMA_RECTI_CHECK_ARRAY(field, i)               \
      agateSlotArrayGet(vm, slot, i, element_slot);           \
      if (!gammaCheckInt(vm, element_slot, &result->field)) { \
        return false;                                         \
      }

      GAMMA_RECTI_CHECK_ARRAY(position.v[0], 0)
      GAMMA_RECTI_CHECK_ARRAY(position.v[1], 1)
      GAMMA_RECTI_CHECK_ARRAY(size.v[0],     2)
      GAMMA_RECTI_CHECK_ARRAY(size.v[1],     3)

      #undef GAMMA_RECTI_CHECK_ARRAY

      return true;
    }

    return false;
  }

  return false;
}
